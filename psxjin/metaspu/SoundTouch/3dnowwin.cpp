#include "cpudetect.h"
#include "sttypes.h"

#ifndef _WIN32
#error "wrong platform - this source code file is exclusively for Win32 platform"
#endif

using namespace soundtouch;

#ifdef ALLOW_3DNOW

// 3DNow! routines available only with float sample type    
// Implementation of 3DNow! optimized functions of class 'TDStretch3DNow'

#include "tdstretch.h"
#include <limits.h>

// These are declared in tdstretch.cpp

extern int scanOffsets[4][24];

// Calculates cross correlation of two buffers

double TDStretch3DNow::calcCrossCorrStereo(const float *pV1, const float *pV2) const
{
    uint overlapLengthLocal = overlapLength;
    float corr;

    // Calculates the cross-correlation value between 'pV1' and 'pV2' vectors
    /*
    c-pseudocode:

        corr = 0;
        for (i = 0; i < overlapLength / 4; i ++)
        {
            corr += pV1[0] * pV2[0];
                    pV1[1] * pV2[1];
                    pV1[2] * pV2[2];
                    pV1[3] * pV2[3];
                    pV1[4] * pV2[4];
                    pV1[5] * pV2[5];
                    pV1[6] * pV2[6];
                    pV1[7] * pV2[7];

            pV1 += 8;
            pV2 += 8;
        }
    */

    _asm 
    {
        // Give prefetch hints to CPU on what data is needed soon.
        // Give more aggressive hints on pV1 as that changes more between different calls 
        // while pV2 stays the same
        prefetch [pV1]
        prefetch [pV2]
        prefetch [pV1 + 32]

        mov     eax, dword ptr pV2
        mov     ebx, dword ptr pV1

        pxor    mm0, mm0

        mov     ecx, overlapLengthLocal
        shr     ecx, 2  // div by four

    loop1:
        movq    mm1, [eax]
        prefetch [eax + 32]     // Give a prefetch hint to CPU on what data is needed soon
        pfmul   mm1, [ebx]
        prefetch [ebx + 64]     // Give a prefetch hint to CPU on what data is needed soon

        movq    mm2, [eax + 8]
        pfadd   mm0, mm1
        pfmul   mm2, [ebx + 8]

        movq    mm3, [eax + 16]
        pfadd   mm0, mm2
        pfmul   mm3, [ebx + 16]

        movq    mm4, [eax + 24]
        pfadd   mm0, mm3
        pfmul   mm4, [ebx + 24]

        add     eax, 32
        pfadd   mm0, mm4
        add     ebx, 32

        dec     ecx
        jnz     loop1

        // Add halfs of mm0 together and return the result
        // Note: mm1 is used as a dummy parameter only, we actually don't care about it's value
		
        pfacc   mm0, mm1
        movd    corr, mm0
        femms
    }

    return corr;
}

// Implementation of 3DNow! optimized functions of class 'FIRFilter'

#include "firfilter.h"

FIRFilter3DNow::FIRFilter3DNow() : FIRFilter()
{
    filterCoeffsUnalign = NULL;
}


FIRFilter3DNow::~FIRFilter3DNow()
{
    delete[] filterCoeffsUnalign;
}

// (Overloaded) Calculates filter coefficients for 3DNow! routine

void FIRFilter3DNow::setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor)
{
    uint i;
    float fDivider;

    FIRFilter::setCoefficients(coeffs, newLength, uResultDivFactor);

    // Scale the filter coefficients so that it won't be necessary to scale the filtering result
    // Also rearrange coefficients suitably for 3DNow!
    // Ensure that filter coefficients array is aligned to 16-byte boundary
	
    delete[] filterCoeffsUnalign;
    filterCoeffsUnalign = new float[2 * newLength + 4];
    filterCoeffsAlign = (float *)(((uint)filterCoeffsUnalign + 15) & -16);

    fDivider = (float)resultDivider;

    // Re-arrange the filter coefficients for MMX routines
    for (i = 0; i < newLength; i ++)
    {
        filterCoeffsAlign[2 * i + 0] =
        filterCoeffsAlign[2 * i + 1] = coeffs[i + 0] / fDivider;
    }
}


// 3DNow!-optimized version of the filter routine for stereo sound

uint FIRFilter3DNow::evaluateFilterStereo(float *dest, const float *src, const uint numSamples) const
{
    float *filterCoeffsLocal = filterCoeffsAlign;
    uint count = (numSamples - length) & -2;
    uint lengthLocal = length / 4;

    assert(length != 0);
    assert(count % 2 == 0);

    /* original code:

    double suml1, suml2;
    double sumr1, sumr2;
    uint i, j;

    for (j = 0; j < count; j += 2)
    {
        const float *ptr;

        suml1 = sumr1 = 0.0;
        suml2 = sumr2 = 0.0;
        ptr = src;
        filterCoeffsLocal = filterCoeffs;
        for (i = 0; i < lengthLocal; i ++) 
        {
            // unroll loop for efficiency.

            suml1 += ptr[0] * filterCoeffsLocal[0] + 
                     ptr[2] * filterCoeffsLocal[2] +
                     ptr[4] * filterCoeffsLocal[4] +
                     ptr[6] * filterCoeffsLocal[6];

            sumr1 += ptr[1] * filterCoeffsLocal[1] + 
                     ptr[3] * filterCoeffsLocal[3] +
                     ptr[5] * filterCoeffsLocal[5] +
                     ptr[7] * filterCoeffsLocal[7];

            suml2 += ptr[8] * filterCoeffsLocal[0] + 
                     ptr[10] * filterCoeffsLocal[2] +
                     ptr[12] * filterCoeffsLocal[4] +
                     ptr[14] * filterCoeffsLocal[6];

            sumr2 += ptr[9] * filterCoeffsLocal[1] + 
                     ptr[11] * filterCoeffsLocal[3] +
                     ptr[13] * filterCoeffsLocal[5] +
                     ptr[15] * filterCoeffsLocal[7];

            ptr += 16;
            filterCoeffsLocal += 8;
        }
        dest[0] = (float)suml1;
        dest[1] = (float)sumr1;
        dest[2] = (float)suml2;
        dest[3] = (float)sumr2;

        src += 4;
        dest += 4;
    }
    */
    _asm
    {
        mov     eax, dword ptr dest
        mov     ebx, dword ptr src
        mov     edx, count
        shr     edx, 1

    loop1:
        // "Outer loop": during each round 2*2 output samples are calculated
        prefetch  [ebx]                 // Give a prefetch hint to the CPU as to what data is going to be needed soon
        prefetch  [filterCoeffsLocal]   // Give a prefetch hint to the CPU as to what data is going to be needed soon

        mov     esi, ebx
        mov     edi, filterCoeffsLocal
        pxor    mm0, mm0
        pxor    mm1, mm1
        mov     ecx, lengthLocal

    loop2:
        // "Inner loop": during each round four FIR filter taps are evaluated for 2*2 output samples
        movq    mm2, [edi]
        movq    mm3, mm2
        prefetch  [edi + 32]     // Give a prefetch hint to the CPU as to what data is going to be needed soon
        pfmul   mm2, [esi]
        prefetch  [esi + 32]     // Give a prefetch hint to the CPU as to what data is going to be needed soon
        pfmul   mm3, [esi + 8]

        movq    mm4, [edi + 8]
        movq    mm5, mm4
        pfadd   mm0, mm2
        pfmul   mm4, [esi + 8]
        pfadd   mm1, mm3
        pfmul   mm5, [esi + 16]

        movq    mm2, [edi + 16]
        movq    mm6, mm2
        pfadd   mm0, mm4
        pfmul   mm2, [esi + 16]
        pfadd   mm1, mm5
        pfmul   mm6, [esi + 24]

        movq    mm3, [edi + 24]
        movq    mm7, mm3
        pfadd   mm0, mm2
        pfmul   mm3, [esi + 24]
        pfadd   mm1, mm6
        pfmul   mm7, [esi + 32]
        add     esi, 32
        pfadd   mm0, mm3
        add     edi, 32
        pfadd   mm1, mm7

        dec     ecx
        jnz     loop2

        movq    [eax], mm0
        add     ebx, 16
        movq    [eax + 8], mm1
        add     eax, 16

        dec     edx
        jnz     loop1

        femms
    }

    return count;
}

#endif  // ALLOW_3DNOW
