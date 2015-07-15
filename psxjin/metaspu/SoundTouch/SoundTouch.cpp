#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdexcept>
#include <stdio.h>

#include "soundtouch.h"
#include "tdstretch.h"
#include "ratetransposer.h"
#include "cpudetect.h"

using namespace soundtouch;

// Print library version string

extern "C" void soundtouch_ac_test()
{
    printf("Sound touch Version: %s\n",SOUNDTOUCH_VERSION);
} 

SoundTouch::SoundTouch()
{
    // Initialize rate transposer and tempo changer instances

    pRateTransposer = RateTransposer::newInstance();
    pTDStretch = TDStretch::newInstance();

    setOutPipe(pTDStretch);

    rate = tempo = 0;

    virtualPitch = 
    virtualRate = 
    virtualTempo = 1.0;

    calcEffectiveRateAndTempo();

    channels = 0;
    bSrateSet = FALSE;
}

SoundTouch::~SoundTouch()
{
    delete pRateTransposer;
    delete pTDStretch;
}

/// Get Sound touch library version string

const char *SoundTouch::getVersionString()
{
    static const char *_version = SOUNDTOUCH_VERSION;

    return _version;
}

// Get Sound touch library version ID

uint SoundTouch::getVersionId()
{
    return SOUNDTOUCH_VERSION_ID;
}

// Sets the number of channels, 1 = mono, 2 = stereo

void SoundTouch::setChannels(uint numChannels)
{
    if (numChannels != 1 && numChannels != 2) 
    {
        throw std::runtime_error("Illegal number of channels");
    }
    channels = numChannels;
    pRateTransposer->setChannels(numChannels);
    pTDStretch->setChannels(numChannels);
}

// Sets new rate control value. Normal rate = 1.0, smaller values
// represent slower rate, larger faster rates

void SoundTouch::setRate(float newRate)
{
    virtualRate = newRate;
    calcEffectiveRateAndTempo();
}

// Sets new rate control value as a difference in percents compared
// to the original rate (-50 .. +100 %)

void SoundTouch::setRateChange(float newRate)
{
    virtualRate = 1.0f + 0.01f * newRate;
    calcEffectiveRateAndTempo();
}

// Sets new tempo control value. Normal tempo = 1.0, smaller values
// represent slower tempo, larger faster tempo

void SoundTouch::setTempo(float newTempo)
{
    virtualTempo = newTempo;
    calcEffectiveRateAndTempo();
}

// Sets new tempo control value as a difference in percents compared
// to the original tempo (-50 .. +100 %)

void SoundTouch::setTempoChange(float newTempo)
{
    virtualTempo = 1.0f + 0.01f * newTempo;
    calcEffectiveRateAndTempo();
}

// Sets new pitch control value. Original pitch = 1.0, smaller values
// represent lower pitches, larger values higher pitch

void SoundTouch::setPitch(float newPitch)
{
    virtualPitch = newPitch;
    calcEffectiveRateAndTempo();
}

// Sets pitch change in octaves compared to the original pitch
// (-1.00 .. +1.00)

void SoundTouch::setPitchOctaves(float newPitch)
{
    virtualPitch = (float)exp(0.69314718056f * newPitch);
    calcEffectiveRateAndTempo();
}

// Sets pitch change in semi-tones compared to the original pitch
// (-12 .. +12)

void SoundTouch::setPitchSemiTones(int newPitch)
{
    setPitchOctaves((float)newPitch / 12.0f);
}

void SoundTouch::setPitchSemiTones(float newPitch)
{
    setPitchOctaves(newPitch / 12.0f);
}

// Calculates 'effective' rate and tempo values from the
// nominal control values

void SoundTouch::calcEffectiveRateAndTempo()
{
    float oldTempo = tempo;
    float oldRate = rate;

    tempo = virtualTempo / virtualPitch;
    rate = virtualPitch * virtualRate;

    if (rate != oldRate) pRateTransposer->setRate(rate);
    if (tempo != oldTempo) pTDStretch->setTempo(tempo);

    if (rate > 1.0f) 
    {
        if (output != pRateTransposer) 
        {
            FIFOSamplePipe *transOut;

            assert(output == pTDStretch);
			
            // Move samples in the current output buffer to the output of pRateTransposer
			
            transOut = pRateTransposer->getOutput();
            transOut->moveSamples(*output);
			
            // Move samples in tempo changer's input to pitch transposer's input
			
            pRateTransposer->moveSamples(*pTDStretch->getInput());

            output = pRateTransposer;
        }
    } 
    else 
    {
        if (output != pTDStretch) 
        {
            FIFOSamplePipe *tempoOut;

            assert(output == pRateTransposer);
			
            // Move samples in the current output buffer to the output of pTDStretch
			
            tempoOut = pTDStretch->getOutput();
            tempoOut->moveSamples(*output);
			
            // Move samples in pitch transposer's store buffer to tempo changer's input
			
            pTDStretch->moveSamples(*pRateTransposer->getStore());

            output = pTDStretch;

        }
    }
}

// Sets sample rate

void SoundTouch::setSampleRate(uint srate)
{
    bSrateSet = TRUE;
    // Set sample rate, leave other tempo changer parameters as they are
    pTDStretch->setParameters(srate);
}

// Adds numSamples pcs of samples from the samples memory position into
// the input of the object

void SoundTouch::putSamples(const SAMPLETYPE *samples, uint numSamples)
{
    if (bSrateSet == FALSE) 
    {
        throw std::runtime_error("Sound touch : Sample rate not defined");
    } 
    else if (channels == 0) 
    {
        throw std::runtime_error("Sound touch : Number of channels not defined");
    }

    // Transpose the rate of the new samples if necessary
    /* Bypass the nominal setting - can introduce a click in sound when tempo/pitch control crosses the nominal value
    if (rate == 1.0f) 
    {
        // The rate value is same as the original, simply evaluate the tempo changer
        assert(output == pTDStretch);
        if (pRateTransposer->isEmpty() == 0) 
        {
            // Yet flush the last samples in the pitch transposer buffer
            // (may happen if 'rate' changes from a non-zero value to zero)
            pTDStretch->moveSamples(*pRateTransposer);
        }
        pTDStretch->putSamples(samples, numSamples);
    } 
    */
    else if (rate <= 1.0f) 
    {
        // Transpose the rate down, output the transposed sound to tempo changer buffer
		
        assert(output == pTDStretch);
        pRateTransposer->putSamples(samples, numSamples);
        pTDStretch->moveSamples(*pRateTransposer);
    } 
    else 
    {
        assert(rate > 1.0f);
		
        // Evaluate the tempo changer, then transpose the rate up
		
        assert(output == pRateTransposer);
        pTDStretch->putSamples(samples, numSamples);
        pRateTransposer->moveSamples(*pTDStretch);
    }
}

// Flushes the last samples from the processing pipeline to the output.
// Clears also the internal processing buffers
// Note: This function is meant for extracting the last samples of a sound
// stream. This function may introduce additional blank samples in the end
// of the sound stream, and thus it's not recommended to call this function
// in the middle of a sound stream

void SoundTouch::flush()
{
    int i;
    uint nOut;
    SAMPLETYPE buff[128];

    nOut = numSamples();

    memset(buff, 0, 128 * sizeof(SAMPLETYPE));
	
    // "Push" the last active samples out from the processing pipeline by
    // feeding blank samples into the processing pipeline until new, 
    // processed samples appear in the output (not however, more than 
    // 8k samples in any case)
	
    for (i = 0; i < 128; i ++) 
    {
        putSamples(buff, 64);
        if (numSamples() != nOut) break;  // New samples have appeared in the output
    }

    // Clear working buffers
	
    pRateTransposer->clear();
    pTDStretch->clearInput();
	
    // Yet leave the tempoChanger output untouched as that's where the
    // flushed samples are
}

// Changes a setting controlling the processing system behavior. See the
// 'SETTING_...' defines for available setting ID's

BOOL SoundTouch::setSetting(uint settingId, uint value)
{
    uint sampleRate, sequenceMs, seekWindowMs, overlapMs;

    // Read current tdstretch routine parameters
	
    pTDStretch->getParameters(&sampleRate, &sequenceMs, &seekWindowMs, &overlapMs);

    switch (settingId) 
    {
        case SETTING_USE_AA_FILTER:
		
            // Enables / disables anti-alias filter
			
            pRateTransposer->enableAAFilter((value != 0) ? TRUE : FALSE);
            return TRUE;

        case SETTING_AA_FILTER_LENGTH:
		
            // Sets anti-alias filter length
			
            pRateTransposer->getAAFilter()->setLength(value);
            return TRUE;

        case SETTING_USE_QUICKSEEK:
		
            // Enables / disables tempo routine quick seeking algorithm
			
            pTDStretch->enableQuickSeek((value != 0) ? TRUE : FALSE);
            return TRUE;

        case SETTING_SEQUENCE_MS:
		
            // Change time stretch sequence duration parameter
			
            pTDStretch->setParameters(sampleRate, value, seekWindowMs, overlapMs);
            return TRUE;

        case SETTING_SEEKWINDOW_MS:
		
            // Change time stretch seek window length parameter
			
            pTDStretch->setParameters(sampleRate, sequenceMs, value, overlapMs);
            return TRUE;

        case SETTING_OVERLAP_MS:
		
            // Change time stretch overlap length parameter
			
            pTDStretch->setParameters(sampleRate, sequenceMs, seekWindowMs, value);
            return TRUE;

        default :
            return FALSE;
    }
}

// Reads a setting controlling the processing system behavior. See the
// 'SETTING_...' defines for available setting ID's

// Returns the setting value

uint SoundTouch::getSetting(uint settingId) const
{
    uint temp;

    switch (settingId) 
    {
        case SETTING_USE_AA_FILTER :
            return pRateTransposer->isAAFilterEnabled();

        case SETTING_AA_FILTER_LENGTH :
            return pRateTransposer->getAAFilter()->getLength();

        case SETTING_USE_QUICKSEEK :
            return pTDStretch->isQuickSeekEnabled();

        case SETTING_SEQUENCE_MS:
            pTDStretch->getParameters(NULL, &temp, NULL, NULL);
            return temp;

        case SETTING_SEEKWINDOW_MS:
            pTDStretch->getParameters(NULL, NULL, &temp, NULL);
            return temp;

        case SETTING_OVERLAP_MS:
            pTDStretch->getParameters(NULL, NULL, NULL, &temp);
            return temp;

        default :
            return 0;
    }
}

// Clears all the samples in the object's output and internal processing
// buffers

void SoundTouch::clear()
{
    pRateTransposer->clear();
    pTDStretch->clear();
}

/// Returns number of samples currently unprocessed

uint SoundTouch::numUnprocessedSamples() const
{
    FIFOSamplePipe * psp;
    if (pTDStretch)
    {
        psp = pTDStretch->getInput();
        if (psp)
        {
            return psp->numSamples();
        }
    }
    return 0;
}
