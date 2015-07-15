#ifndef FIFOSampleBuffer_H
#define FIFOSampleBuffer_H

#include "fifosamplepipe.h"

namespace soundtouch
{

// Sample buffer working in FIFO (first-in-first-out) principle. The class takes
// care of storage size adjustment and data moving during input/output operations.
// Notice that in case of stereo audio, one sample is considered to consist of 
// both channel data

class FIFOSampleBuffer : public FIFOSamplePipe
{
private:

    // Sample buffer
    SAMPLETYPE *buffer;

    // Raw unaligned buffer memory. Buffer is made aligned by pointing it to first
    // 16-byte aligned location of this buffer
	
    SAMPLETYPE *bufferUnaligned;

    // Sample buffer size in bytes
	
    uint sizeInBytes;

    // How many samples are currently in buffer
	
    uint samplesInBuffer;

    // Channels, 1=mono, 2=stereo
	
    uint channels;

    // Current position pointer to the buffer. This pointer is increased when samples are 
    // removed from the pipe so that it's necessary to actually rewind buffer (move data)
    // only new data when is put to the pipe
	
    uint bufferPos;

    // Rewind the buffer by moving data from position pointed by bufferPos to real
    // beginning of the buffer
	
    void rewind();

    // Ensures that the buffer has capacity for at least this many samples
	
    void ensureCapacity(const uint capacityRequirement);

    // Returns current capacity
	
    uint getCapacity() const;
 
public:

    // Constructor
	
    FIFOSampleBuffer(uint numChannels = 2     // Number of channels, 1=mono, 2=stereo
                                              // Default is stereo
                     );

    // Destructor
	
    ~FIFOSampleBuffer();

    // Returns a pointer to the beginning of the output samples
    // This function is provided for accessing the output samples directly
    // Please be careful not to corrupt the book keeping
    // When using this function to output samples, also remember to remove the
    // output samples from the buffer by calling the 
    // receiveSamples(numSamples) function
	
    virtual SAMPLETYPE *ptrBegin() const;

    // Returns a pointer to the end of the used part of the sample buffer (IE
    // where the new samples are to be inserted). This function may be used for 
    // inserting new samples into the sample buffer directly. Please be careful
    // not to corrupt the book keeping
    // When using this function as means for inserting new samples, also remember 
    // to increase the sample count afterwards, by calling the
    // putSamples(numSamples) function
	
    SAMPLETYPE *ptrEnd(
                uint slackCapacity   // How much free capacity (in samples) there at least
                                     // should be so that the caller can successfully insert the 
                                     // desired samples to the buffer. If necessary, the function 
                                     // grows the buffer size to comply with this requirement
                );

    // Adds numSamples pcs of samples from the samples memory position to
    // the sample buffer
	
    virtual void putSamples(const SAMPLETYPE *samples,  // Pointer to samples
                            uint numSamples                         // Number of samples to insert
                            );

    // Adjusts the book keeping to increase number of samples in the buffer without 
    // copying any actual samples
    // This function is used to update the number of samples in the sample buffer
    // when accessing the buffer directly with ptrEnd function. Please be 
    // careful though
	
    virtual void putSamples(uint numSamples   // Number of samples been inserted
                            );

    // Output samples from the beginning of the sample buffer. Copies requested samples to the
    // output buffer and removes them from the sample buffer. If there are less than
    // numsample samples in the buffer, returns all that are available
    // \return Number of samples returned
	
    virtual uint receiveSamples(SAMPLETYPE *output, // Buffer where to copy output samples
                                uint maxSamples                 // How many samples to receive at max
                                );

    // Adjusts book keeping so that given number of samples are removed from beginning of the 
    // sample buffer without copying them anywhere
    // Used to reduce the number of samples in the buffer when accessing the sample buffer directly
    // with ptrBegin function
	
    virtual uint receiveSamples(uint maxSamples   // Remove this many samples from the beginning of pipe
                                );

    // Returns number of samples currently available
	
    virtual uint numSamples() const;

    // Sets number of channels, 1 = mono, 2 = stereo
	
    void setChannels(uint numChannels);

    // Returns non-zero if there aren't any samples available for output
	
    virtual int isEmpty() const;

    // Clears all the samples
	
    virtual void clear();
};

}

#endif
