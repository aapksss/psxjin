#ifndef FIRFilter_H
#define FIRFilter_H

#include "sttypes.h"

namespace soundtouch
{

class FIRFilter 

{
protected:

    // Number of FIR filter taps
	
    uint length;
	
    // Number of FIR filter taps divided by 8
	
    uint lengthDiv8;

    // Result divider factor in 2^k format
	
    uint resultDivFactor;

    // Result divider value
	
    SAMPLETYPE resultDivider;

    // Memory for filter coefficients
	
    SAMPLETYPE *filterCoeffs;

    virtual uint evaluateFilterStereo(SAMPLETYPE *dest, 
                                      const SAMPLETYPE *src, 
                                      uint numSamples) const;
    virtual uint evaluateFilterMono(SAMPLETYPE *dest, 
                                    const SAMPLETYPE *src, 
                                    uint numSamples) const;

public:
    FIRFilter();
    virtual ~FIRFilter();

    // Operator new is overloaded so that it automatically creates a suitable instance 
    // depending on if we have an MMX-capable CPU available or not
	
    void * operator new(size_t s);

    static FIRFilter *newInstance();

    // Applies the filter to the given sequence of samples
    // Note : The amount of outputted samples is by value of 'filter_length' 
    // smaller than the amount of input samples
    // \return Number of samples copied to destination
	
    uint evaluate(SAMPLETYPE *dest, 
                  const SAMPLETYPE *src, 
                  uint numSamples, 
                  uint numChannels) const;

    uint getLength() const;

    virtual void setCoefficients(const SAMPLETYPE *coeffs, 
                                 uint newLength, 
                                 uint uResultDivFactor);
};

// Optional sub-classes that implement CPU-specific optimizations

#ifdef ALLOW_MMX

    // Class that implements MMX-optimized functions exclusive for 16-bit integer samples type
	
    class FIRFilterMMX : public FIRFilter
    {
    protected:
        short *filterCoeffsUnalign;
        short *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(short *dest, const short *src, uint numSamples) const;
    public:
        FIRFilterMMX();
        ~FIRFilterMMX();

        virtual void setCoefficients(const short *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif // ALLOW_MMX


#ifdef ALLOW_3DNOW

    // Class that implements 3DNow!-optimized functions exclusive for floating point samples type
	
    class FIRFilter3DNow : public FIRFilter
    {
    protected:
        float *filterCoeffsUnalign;
        float *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(float *dest, const float *src, uint numSamples) const;
    public:
        FIRFilter3DNow();
        ~FIRFilter3DNow();
        virtual void setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif  // ALLOW_3DNOW


#ifdef ALLOW_SSE

    // Class that implements SSE-optimized functions exclusive for floating point samples type
	
    class FIRFilterSSE : public FIRFilter
    {
    protected:
        float *filterCoeffsUnalign;
        float *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(float *dest, const float *src, uint numSamples) const;
    public:
        FIRFilterSSE();
        ~FIRFilterSSE();

        virtual void setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif // ALLOW_SSE

}

#endif  // FIRFilter_H
