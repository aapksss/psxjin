#ifndef RateTransposer_H
#define RateTransposer_H

#include "aafilter.h"
#include "fifosamplepipe.h"
#include "fifosamplebuffer.h"
#include "sttypes.h"

namespace soundtouch
{

// A common linear sample rate transposer class.
// Note: Use function "RateTransposer::newInstance()" to create a new class 
// instance instead of the "new" operator; that function automatically 
// chooses a correct implementation depending on if integer or floating 
// arithmetics are to be used.
class RateTransposer : public FIFOProcessor
{
protected:

    // Anti-aliasing filter object
    AAFilter *pAAFilter;

    float fRate;

    uint uChannels;

    // Buffer for collecting samples to feed the anti-aliasing filter between
    // two batches
	
    FIFOSampleBuffer storeBuffer;

    // Buffer for keeping samples between transposing and the anti-aliasing filter
	
    FIFOSampleBuffer tempBuffer;

    // Output sample buffer
	
    FIFOSampleBuffer outputBuffer;

    BOOL bUseAAFilter;

    void init();

    virtual void resetRegisters() = 0;

    virtual uint transposeStereo(SAMPLETYPE *dest, 
                         const SAMPLETYPE *src, 
                         uint numSamples) = 0;
    virtual uint transposeMono(SAMPLETYPE *dest, 
                       const SAMPLETYPE *src, 
                       uint numSamples) = 0;
    uint transpose(SAMPLETYPE *dest, 
                   const SAMPLETYPE *src, 
                   uint numSamples);

    void flushStoreBuffer();

    void downsample(const SAMPLETYPE *src, 
                    uint numSamples);
    void upsample(const SAMPLETYPE *src, 
                 uint numSamples);

    // Transposes sample rate by applying anti-aliasing filter to prevent folding. 
    // Returns amount of samples returned in the destination buffer.
    // The maximum amount of samples that can be returned at a time is set by
    // the 'set_returnBuffer_size' function
	
    void processSamples(const SAMPLETYPE *src, 
                        uint numSamples);


public:
    RateTransposer();
    virtual ~RateTransposer();

    // Operator 'new' is overloaded so that it automatically creates a suitable instance 
    // depending on if we're to use integer or floating point arithmetics
	
    void *operator new(size_t s);

    // Use this function instead of "new" operator to create a new instance of this class. 
    // This function automatically chooses a correct implementation, depending on if 
    // integer or floating point arithmetics are to be used
	
    static RateTransposer *newInstance();

    // Returns the output buffer object
	
    FIFOSamplePipe *getOutput() { return &outputBuffer; };

    // Returns the store buffer object
	
    FIFOSamplePipe *getStore() { return &storeBuffer; };

    // Return anti-aliasing filter object
	
    AAFilter *getAAFilter() const;

    // Enables/disables the anti-aliasing filter. Zero to disable, non-zero to enable
	
    void enableAAFilter(BOOL newMode);

    // Returns non-zero if anti-alias filter is enabled
	
    BOOL isAAFilterEnabled() const;

    // Sets new target rate. Normal rate = 1.0, smaller values represent slower 
    // rate, larger faster rates
	
    virtual void setRate(float newRate);

    // Sets the number of channels, 1 = mono, 2 = stereo
	
    void setChannels(uint channels);

    // Adds 'numSamples' pcs of samples from the samples memory position into
    // the input of the object
	
    void putSamples(const SAMPLETYPE *samples, uint numSamples);

    // Clears all the samples in the object
	
    void clear();

    // Returns non-zero if there aren't any samples available for outputting
	
    uint isEmpty();
};

}

#endif
