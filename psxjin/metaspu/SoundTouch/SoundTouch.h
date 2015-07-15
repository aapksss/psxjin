#ifndef SoundTouch_H
#define SoundTouch_H

#include "fifosamplepipe.h"
#include "sttypes.h"

namespace soundtouch
{

// Sound touch library version string

#define SOUNDTOUCH_VERSION          "1.3.1"

// Sound touch library version ID

#define SOUNDTOUCH_VERSION_ID       010301

// Available setting IDs for the setSetting and get_setting functions

// Enable/disable anti-alias filter in pitch transposer (0 = disable)

#define SETTING_USE_AA_FILTER       0

// Pitch transposer anti-alias filter length (8 .. 128 taps, default = 32)

#define SETTING_AA_FILTER_LENGTH    1

// Enable/disable quick seeking algorithm in tempo changer routine
// (enabling quick seeking lowers CPU utilization, but causes a minor sound
//  quality compromise)

#define SETTING_USE_QUICKSEEK       2

// Time-stretch algorithm single processing sequence length in milliseconds. This determines 
// to how long sequences the original sound is chopped in the time-stretch algorithm. 
// See sttypes.h for more information

#define SETTING_SEQUENCE_MS         3

// Time-stretch algorithm seeking window length in milliseconds for algorithm that finds the 
// best possible overlapping location. This determines from how wide window the algorithm 
// may look for an optimal joining location when mixing the sound sequences back together. 
// See sttypes.h for more information

#define SETTING_SEEKWINDOW_MS       4

// Time-stretch algorithm overlap length in milliseconds. When the chopped sound sequences 
// are mixed back together, to form a continuous sound stream, this parameter defines over 
// how long period the two consecutive sequences are let to overlap each other. 
// See sttypes.h for more information

#define SETTING_OVERLAP_MS          5

class SoundTouch : public FIFOProcessor
{
private:

    // Rate transposer class instance
	
    class RateTransposer *pRateTransposer;

    // Time-stretch class instance
	
    class TDStretch *pTDStretch;

    // Virtual pitch parameter. Effective rate and tempo are calculated from these parameters
	
    float virtualRate;

    // Virtual pitch parameter. Effective rate and tempo are calculated from these parameters
	
    float virtualTempo;

    // Virtual pitch parameter. Effective rate and tempo are calculated from these parameters
	
    float virtualPitch;

    // Flag: has sample rate been set?
	
    BOOL  bSrateSet;

    // Calculates effective rate and tempo values from virtualRate, virtualTempo and 
    // virtualPitch parameters
	
    void calcEffectiveRateAndTempo();

protected:

    // Number of channels
	
    uint  channels;

    // Effective rate value calculated from virtualRate, virtualTempo and virtualPitch
	
    float rate;

    // Effective tempo value calculated from virtualRate, virtualTempo and virtualPitch
	
    float tempo;

public:
    SoundTouch();
    virtual ~SoundTouch();

    // Get Sound touch library version string
	
    static const char *getVersionString();

    // Get Sound touch library version ID
	
    static uint getVersionId();

    // Sets new rate control value. Normal rate = 1.0, smaller values
    // represent slower rate, larger faster rates
	
    void setRate(float newRate);

    // Sets new tempo control value. Normal tempo = 1.0, smaller values
    // represent slower tempo, larger faster tempo
	
    void setTempo(float newTempo);

    // Sets new rate control value as a difference in percents compared
    // to the original rate (-50 .. +100 %)
	
    void setRateChange(float newRate);

    // Sets new tempo control value as a difference in percents compared
    // to the original tempo (-50 .. +100 %)
	
    void setTempoChange(float newTempo);

    // Sets new pitch control value. Original pitch = 1.0, smaller values
    // represent lower pitches, larger values higher pitch
	
    void setPitch(float newPitch);

    // Sets pitch change in octaves compared to the original pitch  
    // (-1.00 .. +1.00)
	
    void setPitchOctaves(float newPitch);

    // Sets pitch change in semi-tones compared to the original pitch
    // (-12 .. +12)
	
    void setPitchSemiTones(int newPitch);
    void setPitchSemiTones(float newPitch);

    // Sets the number of channels, 1 = mono, 2 = stereo
	
    void setChannels(uint numChannels);

    // Sets sample rate
	
    void setSampleRate(uint srate);

    // Flushes the last samples from the processing pipeline to the output.
    // Clears the internal processing buffers as well
    // Note: This function is meant for extracting the last samples of a sound
    // stream. This function may introduce additional blank samples in the end
    // of the sound stream, and thus it's not recommended to call this function
    // in the middle of a sound stream
	
    void flush();

    // Adds numSamples pcs of samples from the samples memory position into
    // the input of the object. Notice that sample rate has to be set before
    // calling this function, otherwise it throws a runtime_error exception
	
    virtual void putSamples(
            const SAMPLETYPE *samples,  // Pointer to sample buffer
            uint numSamples                         // Number of samples in buffer. Notice
                                                    // that in case of stereo sound a single sample
                                                    // contains data for both channels
            );

    // Clears all the samples in the object's output and internal processing
    // buffers
	
    virtual void clear();

    // Changes a setting controlling the processing system behavior. See the
    // 'SETTING_...' defines for available setting ID's

    // \return 'TRUE' if the setting was successfully changed
    BOOL setSetting(uint settingId,   // Setting ID number. see SETTING_... defines
                    uint value        // New setting value
                    );

    // Reads a setting controlling the processing system behavior. See the
    // 'SETTING_...' defines for available setting ID's
	
    // \return the setting value
	
    uint getSetting(uint settingId    // Setting ID number, see SETTING_... defines
                    ) const;

    // Returns number of samples currently unprocessed
	
    virtual uint numUnprocessedSamples() const;

    // Other handy functions that are implemented in the ancestor classes (see
    // classes 'FIFOProcessor' and 'FIFOSamplePipe')

    // - receiveSamples(): Use this function to receive 'ready' processed samples from Sound touch
    // - numSamples()    : Get number of 'ready' samples that can be received with 
    //                      function 'receiveSamples()'
    // - isEmpty()        : Returns non-zero if there aren't any 'ready' samples
    // - clear()          : Clears all samples from ready/processing buffers
};

}

#endif
