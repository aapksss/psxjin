#ifndef AAFilter_H
#define AAFilter_H

#include "sttypes.h"

namespace soundtouch
{

class AAFilter
{
protected:
    class FIRFilter *pFIR;

    // Low-pass filter cut-off frequency, negative = invalid
	
    double cutoffFreq;

    // Number of filter taps
    uint length;

    // Calculate the FIR coefficients realizing the given cutoff frequency
	
    void calculateCoeffs();
public:
    AAFilter(uint length);

    ~AAFilter();

    // Sets new anti-alias filter cut-off edge frequency, scaled to sampling 
    // frequency (Nyquist frequency = 0.5). The filter will cut off the 
    // frequencies than that. (This is an incomplete sentence, but I can't understand it. I won't change it until I do)
	
    void setCutoffFreq(double newCutoffFreq);

    /// Sets number of FIR filter taps, IE filter complexity
	
    void setLength(uint newLength);

    uint getLength() const;

    /// Applies the filter to the given sequence of samples. 
    /// Note : The amount of samples output is by value of 'filter length' 
    /// smaller than the amount of input samples.
    uint evaluate(SAMPLETYPE *dest, 
                  const SAMPLETYPE *src, 
                  uint numSamples, 
                  uint numChannels) const;
};

}

#endif
