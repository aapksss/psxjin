#include <stdio.h>
#include "dsound.h"
#include "dxerr8.h"
#include "psxcommon.h"

int SNDDXInit(int buffersize);
void SNDDXDeInit();
void SNDDXUpdateAudio(s16 *buffer, u32 num_samples);
u32 SNDDXGetAudioSpace();
void SNDDXMuteAudio();
void SNDDXUnMuteAudio();
void SNDDXSetVolume(int volume);

LPDIRECTSOUND8 lpDS8 = NULL;
LPDIRECTSOUNDBUFFER lpDSB, lpDSB2;

static s16 *stereodata16;
static u32 soundoffset=0;
static u32 soundbufsize;
static LONG soundvolume;
static int issoundmuted;

extern HWND    hWMain;

static volatile bool doterminate;
static volatile bool terminated;

volatile int win_sound_samplecounter = 0;
static bool insilence;
static int samplecounter_fakecontribution = 0;

//DWORD WINAPI SNDDXThread( LPVOID )
//{
//	for(;;) {
//		if(doterminate) break;
//		{
//			Lock lock;
//			SPU_Emulate_user();
//		}
//		Sleep(10);
//	}
//	terminated = true;
//	return 0;
//}

void SNDDXSetWindow(HWND hwnd)
{
	if(!lpDS8) return;
	if ((IDirectSound8_SetCooperativeLevel(lpDS8, hwnd, DSSCL_PRIORITY)) != DS_OK)
	{
	}
}

int SNDDXInit(int buffersize)
{
	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfx;
	HRESULT ret;
	char tempstr[512];

	if ((ret = DirectSoundCreate8(NULL, &lpDS8, NULL)) != DS_OK)
	{
		sprintf(tempstr, "DirectSound8Create error: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
		MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
		return -1;
	}

	if ((ret = IDirectSound8_SetCooperativeLevel(lpDS8, hWMain, DSSCL_PRIORITY)) != DS_OK)
	{
		sprintf(tempstr, "IDirectSound8_SetCooperativeLevel error: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
		MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
		return -1;
	}

	memset(&dsbdesc, 0, sizeof(dsbdesc));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL;

	if ((ret = IDirectSound8_CreateSoundBuffer(lpDS8, &dsbdesc, &lpDSB, NULL)) != DS_OK)
	{
		sprintf(tempstr, "Error when creating primary sound buffer: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
		MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
		return -1;
	}

	soundbufsize = buffersize * 2; // Caller already multiplies buffersize by 2

	memset(&wfx, 0, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if ((ret = IDirectSoundBuffer8_SetFormat(lpDSB, &wfx)) != DS_OK)
	{
		sprintf(tempstr, "IDirectSoundBuffer8_SetFormat error: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
		MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
		return -1;
	}

	memset(&dsbdesc, 0, sizeof(dsbdesc));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_STICKYFOCUS |
		DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 |
		DSBCAPS_LOCHARDWARE;
	dsbdesc.dwBufferBytes = soundbufsize;
	dsbdesc.lpwfxFormat = &wfx;

	if ((ret = IDirectSound8_CreateSoundBuffer(lpDS8, &dsbdesc, &lpDSB2, NULL)) != DS_OK)
	{
		if (ret == DSERR_CONTROLUNAVAIL ||
			ret == DSERR_INVALIDCALL ||
			ret == E_FAIL || 
			ret == E_NOTIMPL)
		{
			// Try using a software buffer instead
			
			dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_STICKYFOCUS |
				DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 |
				DSBCAPS_LOCSOFTWARE;

			if ((ret = IDirectSound8_CreateSoundBuffer(lpDS8, &dsbdesc, &lpDSB2, NULL)) != DS_OK)
			{
				sprintf(tempstr, "Error when creating secondary sound buffer: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
				MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
				return -1;
			}
		}
		else
		{
			sprintf(tempstr, "Error when creating secondary sound buffer: %s - %s", DXGetErrorString8(ret), DXGetErrorDescription8(ret));
			MessageBox (NULL, tempstr, "Error",  MB_OK | MB_ICONINFORMATION);
			return -1;
		}
	}

	IDirectSoundBuffer8_Play(lpDSB2, 0, 0, DSBPLAY_LOOPING);

	if ((stereodata16 = new s16[soundbufsize / sizeof(s16)]) == NULL)
		return -1;

	memset(stereodata16, 0, soundbufsize);

	soundvolume = DSBVOLUME_MAX;
	issoundmuted = 0;

	doterminate = false;
	terminated = false;

	return 0;
}

void SNDDXDeInit()
{
	DWORD status=0;

	doterminate = true;
	while(!terminated) {
		Sleep(1);
	}

	if (lpDSB2)
	{
		IDirectSoundBuffer8_GetStatus(lpDSB2, &status);

		if(status == DSBSTATUS_PLAYING)
			IDirectSoundBuffer8_Stop(lpDSB2);

		IDirectSoundBuffer8_Release(lpDSB2);
		lpDSB2 = NULL;
	}

	if (lpDSB)
	{
		IDirectSoundBuffer8_Release(lpDSB);
		lpDSB = NULL;
	}

	if (lpDS8)
	{
		IDirectSound8_Release(lpDS8);
		lpDS8 = NULL;
	}
}

void SNDDXClearAudioBuffer()
{
	// We shouldn't need to provide 2 buffers since it's 1 contiguous range
	// But maybe newer DirectSound implementations have issues
	
	LPVOID buffer1;
	LPVOID buffer2;
	DWORD buffer1_size, buffer2_size;
	HRESULT hr = lpDSB2->Lock(0, 0, &buffer1, &buffer1_size, &buffer2, &buffer2_size, DSBLOCK_ENTIREBUFFER);
	if(FAILED(hr))
		return;
	memset(buffer1, 0, buffer1_size);
	if(buffer2)
		memset(buffer2, 0, buffer2_size);
	lpDSB2->Unlock(buffer1, buffer1_size, buffer2, buffer2_size);
}

void SNDDXUpdateAudio(s16 *buffer, u32 num_samples)
{
	int samplecounter;
	{
		Lock lock;
		if(num_samples)
		{
			samplecounter = win_sound_samplecounter -= num_samples - samplecounter_fakecontribution;
			samplecounter_fakecontribution = 0;
		}
		else
		{
			samplecounter = win_sound_samplecounter -= 44100/180;
			samplecounter_fakecontribution += 44100/180;
		}
	}

	//printf("%d\n",num_samples);

	bool silence = (samplecounter<-44100*15/60); // If you are behind by more than a quarter second then start silence

	if(insilence)
	{
		if(silence)
		{
			return;
		}
		else
			insilence = false;
	}
	else
	{
		if(silence)
		{
//#ifndef PUBLIC_RELEASE
//			extern volatile bool execute;
//			if(execute)
//				printf("snddx: emergency cleared sound buffer. (%d, %d, %d)\n", win_sound_samplecounter, num_samples, samplecounter_fakecontribution);
//#endif
			samplecounter_fakecontribution = 0;
			insilence = true;
			SNDDXClearAudioBuffer();
			printf("clearing\n");
			return;
		}
	}

	LPVOID buffer1;
	LPVOID buffer2;
	DWORD buffer1_size, buffer2_size;

	HRESULT hr = lpDSB2->Lock(soundoffset, num_samples * sizeof(s16) * 2,
	                          &buffer1, &buffer1_size, &buffer2, &buffer2_size, 0);
	if(FAILED(hr))
	{
		if(hr == DSBSTATUS_BUFFERLOST)
			lpDSB2->Restore();
		return;
	}

	memcpy(buffer1, buffer, buffer1_size);
	if(buffer2)
		memcpy(buffer2, ((u8 *)buffer)+buffer1_size, buffer2_size);

	soundoffset += buffer1_size + buffer2_size;
	soundoffset %= soundbufsize;

	lpDSB2->Unlock(buffer1, buffer1_size, buffer2, buffer2_size);
}

static inline u32 circularDist(u32 from, u32 to, u32 size)
{
	if(size == 0)
		return 0;
	s32 diff = (s32)(to - from);
	while(diff < 0)
		diff += size;
	return (u32)diff;
}

u32 SNDDXGetAudioSpace()
{
	DWORD playcursor, writecursor;
	if(FAILED(lpDSB2->GetCurrentPosition(&playcursor, &writecursor)))
		return 0;

	u32 curToWrite = circularDist(soundoffset, writecursor, soundbufsize);
	u32 curToPlay = circularDist(soundoffset, playcursor, soundbufsize);

	if(curToWrite < curToPlay)
		return 0; // In between the two cursors. we shouldn't write anything during this time

	//printf("[%012d] SNDDXGetAudioSpace returns %d\n",timeGetTime(), curToPlay / (sizeof(s16) * 2));
	return curToPlay / (sizeof(s16) * 2);
}

void SNDDXMuteAudio()
{
	issoundmuted = 1;
	IDirectSoundBuffer8_SetVolume (lpDSB2, DSBVOLUME_MIN);
}

void SNDDXUnMuteAudio()
{
	issoundmuted = 0;
	IDirectSoundBuffer8_SetVolume (lpDSB2, soundvolume);
}

void SNDDXSetVolume(int volume)
{
	if (!lpDSB2) return ;     // Might happen when changing sound devices on the fly, caused a GPF (What is a GPF?)
	soundvolume = (((LONG)volume) - 100) * 100;
	if (!issoundmuted)
		IDirectSoundBuffer8_SetVolume (lpDSB2, soundvolume);
}

void SetupSound(void)
{
	const int sndbuffersize=11760;
	SNDDXInit(sndbuffersize);
}

void RemoveSound(void)
{
	SNDDXDeInit();
}
