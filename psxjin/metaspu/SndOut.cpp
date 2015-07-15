//#include "Global.h"
//#include "types.h" //desmume
#include "psxcommon.h"
#include "sndout.h"
#include <assert.h>

// Check the above for what is needed for the emulator

int SndOutLatencyMS = 160;
bool timeStretchDisabled = false;

StereoOut32 StereoOut32::Empty( 0, 0 );

StereoOut32::StereoOut32( const StereoOut16& src ) :
	Left( src.Left ),
	Right( src.Right )
{
}

StereoOut32::StereoOut32( const StereoOutFloat& src ) :
	Left( (s32)(src.Left * 2147483647.0f) ),
	Right( (s32)(src.Right * 2147483647.0f) )
{
}

StereoOut16 StereoOut32::DownSample() const
{
	return StereoOut16(
		Left >> SndOutVolumeShift,
		Right >> SndOutVolumeShift
	);
}

StereoOut32 StereoOut16::UpSample() const
{
	return StereoOut32(
		Left << SndOutVolumeShift,
		Right << SndOutVolumeShift
	);

}

//class NullOutModule: public SndOutModule
//{
//public:
//	s32  Init()  { return 0; }
//	void Close() { }
//	s32  Test() const { return 0; }
//	void Configure(uptr parent)  { }
//	bool Is51Out() const { return false; }
//	int GetEmptySampleCount() const { return 0; }
//	
//	const wchar_t* GetIdent() const
//	{
//		return L"nullout";
//	}
//
//	const wchar_t* GetLongName() const
//	{
//		return L"No Sound (Emulate SPU2 only)";
//	}
//
//	void ReadSettings()
//	{
//	}
//
//	void WriteSettings() const
//	{
//	}
//
//} NullOut;
//
//SndOutModule* mods[]=
//{
//	&NullOut,
//#ifdef _MSC_VER
//	XAudio2Out,
//	DSoundOut,
//	WaveOut,
//#endif
//	NULL		// signals the end of our list
//};
//
//int FindOutputModuleById( const wchar_t* omodid )
//{
//	int modcnt = 0;
//	while( mods[modcnt] != NULL )
//	{
//		if( wcscmp( mods[modcnt]->GetIdent(), omodid ) == 0 )
//			break;
//		++modcnt;
//	}
//	return modcnt;
//}

StereoOut32 *SndBuffer::m_buffer;
s32 SndBuffer::m_size;
s32 SndBuffer::m_rpos;
s32 SndBuffer::m_wpos;
s32 SndBuffer::m_data;

bool SndBuffer::m_underrun_freeze;
StereoOut32* SndBuffer::sndTempBuffer = NULL;
StereoOut16* SndBuffer::sndTempBuffer16 = NULL;
int SndBuffer::sndTempProgress = 0;

int GetAlignedBufferSize( int comp )
{
	return (comp + SndOutPacketSize-1) & ~(SndOutPacketSize-1);
}

// Returns TRUE if there is data to be output, or false if no data
// is available to be copied.

bool SndBuffer::CheckUnderrunStatus( int& nSamples, int& quietSampleCount )
{
	quietSampleCount = 0;
	if( m_underrun_freeze )
	{			
		int toFill = (int)(m_size * ( timeStretchDisabled ? 0.50f : 0.1f ) );
		toFill = GetAlignedBufferSize( toFill );

		// toFill is now aligned to a SndOutPacket

		if( m_data < toFill )
		{
			quietSampleCount = nSamples;
			return false;
		}

		m_underrun_freeze = false;
		// To do
		// Check this code for completeness
		//if( MsgOverruns() )
			printf(" * SPU2 > Underrun compensation (%d packets buffered)\n", toFill / SndOutPacketSize );
		lastPct = 0.0;		// Normalize time stretcher
	}
	else if( m_data < nSamples )
	{
		nSamples = m_data;
		quietSampleCount = SndOutPacketSize - m_data;
		m_underrun_freeze = true;

		if( !timeStretchDisabled )
			timeStretchUnderrun();

		return nSamples != 0;
	}

	return true;
}

void SndBuffer::_InitFail()
{
	// If a failure occurs, just initialize the NoSound driver.  This'll allow
	// the game to emulate properly (hopefully), albeit without sound.
	//OutputModule = FindOutputModuleById( NullOut.GetIdent() );
	//mods[OutputModule]->Init();
}

void SndBuffer::_WriteSamples(StereoOut32 *bData, int nSamples)
{
	int free = m_size-m_data;
	m_predictData = 0;

	assert( m_data <= m_size );

	// Problem:
	//  If the SPU2 gets out of sync with the SndOut device, the writepos of the
	//  circular buffer will overtake the readpos, leading to a prolonged period
	//  of hopscotching read/write accesses (IE, lots of crappy sound for
	//  several seconds).
	//
	// Compromise:
	//  When an overrun occurs, we adapt by discarding a portion of the buffer.
	//  The older portion of the buffer is discarded rather than incoming data,
	//  so that the overall audio synchronization is better.

	if( free < nSamples )
	{
		// Buffer overrun!
		// Dump samples from the read portion of the buffer instead of dropping
		// the newly written stuff.

		s32 comp;

		if( !timeStretchDisabled )
		{
			comp = timeStretchOverrun();
		}
		else
		{
			// Toss half the buffer plus whatever is being written anew:
			comp = GetAlignedBufferSize( (m_size + nSamples ) / 2 );
			if( comp > (m_size-SndOutPacketSize) ) comp = m_size-SndOutPacketSize;
		}

		m_data -= comp;
		m_rpos = (m_rpos+comp) % m_size;
		//TODO
		//if( MsgOverruns() )
			printf(" * SPU2 > Overrun compensation (%d packets tossed)\n", comp / SndOutPacketSize );
		lastPct = 0.0;		// Normalize the time stretcher
	}

	// Copy in two phases, since there's a chance the packet
	// wraps around the buffer (it'd be nice to deal in packets only, but
	// the time stretcher and DSP options require flexibility).

	const int endPos = m_wpos + nSamples;
	const int secondCopyLen = endPos - m_size;
	StereoOut32* wposbuffer = &m_buffer[m_wpos];

	m_data += nSamples;
	if( secondCopyLen > 0 )
	{
		nSamples -= secondCopyLen;
		memcpy( m_buffer, &bData[nSamples], secondCopyLen * sizeof( *bData ) );
		m_wpos = secondCopyLen;
	}
	else
		m_wpos += nSamples;

	memcpy( wposbuffer, bData, nSamples * sizeof( *bData ) );
}

void SndBuffer::Init()
{
	//if( mods[OutputModule] == NULL )
	//{
	//	_InitFail();
	//	return;
	//}

	// Initialize sound buffer
	// Buffer actually attempts to run ~50%, so allocate near double what
	// the requested latency is:

	m_rpos = 0;
	m_wpos = 0;
	m_data = 0;

	try
	{
		const float latencyMS = SndOutLatencyMS * (timeStretchDisabled ? 1.5f : 2.0f );
		m_size = GetAlignedBufferSize( (int)(latencyMS * SampleRate / 1000.0f ) );
		m_buffer = new StereoOut32[m_size];
		m_underrun_freeze = false;

		sndTempBuffer = new StereoOut32[SndOutPacketSize];
		sndTempBuffer16 = new StereoOut16[SndOutPacketSize];
	}
	catch( std::bad_alloc& )
	{
		// Out of memory exception (most likely)

		printf( "Out of memory error occurred while initializing SPU2." );
		_InitFail();
		return;
	}

	// Clear buffers!
	// Fixes loopy sounds on emulator resets
	
	memset( sndTempBuffer, 0, sizeof(StereoOut32) * SndOutPacketSize );
	memset( sndTempBuffer16, 0, sizeof(StereoOut16) * SndOutPacketSize );

	sndTempProgress = 0;

	soundtouchInit();		// Initializes the time stretching

	// Some stuff
	//spdif_set51(mods[OutputModule]->Is51Out());

	// Initialize module
	//if( mods[OutputModule]->Init() == -1 ) _InitFail();
}

void SndBuffer::Cleanup()
{
	//mods[OutputModule]->Close();

	soundtouchCleanup();

	//safe_delete_array( m_buffer );
	//safe_delete_array( sndTempBuffer );
	//safe_delete_array( sndTempBuffer16 );
	delete[] m_buffer;
	delete[] sndTempBuffer;
	delete[] sndTempBuffer16;
}

int SndBuffer::m_dsp_progress = 0;

int SndBuffer::m_timestretch_progress = 0;
int SndBuffer::ssFreeze = 0;

void SndBuffer::ClearContents()
{
	SndBuffer::soundtouchClearContents();
	SndBuffer::ssFreeze = 30; // Delays sound output for about half a second
}

void SndBuffer::Write( const StereoOut32& Sample )
{
	// Log final output to wav file
	//WaveDump::WriteCore( 1, CoreSrc_External, Sample.DownSample() );

	//RecordWrite( Sample.DownSample() );

	//if(mods[OutputModule] == &NullOut) // null output doesn't need buffering or stretching! :p
	//	return;

	sndTempBuffer[sndTempProgress++] = Sample;

	// If we haven't accumulated a full packet yet, do nothing more:
	
	if(sndTempProgress < SndOutPacketSize) return;
	sndTempProgress = 0;

	// Don't play anything directly after loading a savestate, avoids static killing your speakers
	
//	if ( ssFreeze > 0 )
//	{	
//		ssFreeze--;
//		return;
//	}

//	else
	{
		if( !timeStretchDisabled )
			timeStretchWrite();
		else
			_WriteSamples(sndTempBuffer, SndOutPacketSize);
	}
}

s32 SndBuffer::Test()
{
	//if( mods[OutputModule] == NULL )
	//	return -1;

	//return mods[OutputModule]->Test();
	return 0;
}
