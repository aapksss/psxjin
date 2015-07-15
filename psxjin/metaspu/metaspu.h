#ifndef _METASPU_H_
#define _METASPU_H_

#include "metasputypes.h"

#include <algorithm>

template< typename T >
static FORCEINLINE void Clampify( T& src, T min, T max )
{
	src = std::min( std::max( src, min ), max );
}

template< typename T >
static FORCEINLINE T GetClamped( T src, T min, T max )
{
	return std::min( std::max( src, min ), max );
}

class ISynchronizingAudioBuffer
{
public:
	virtual void enqueue_samples(s16* buf, int samples_provided) = 0;

	// Returns the number of samples actually supplied, which may not match the number requested
	
	virtual int output_samples(s16* buf, int samples_requested) = 0;
};

enum ESynchMode
{
	ESynchMode_DualSynchAsynch,
	ESynchMode_Synchronous
};

enum ESynchMethod
{
	ESynchMethod_N,
	ESynchMethod_Z,
	ESynchMethod_P,
};

ISynchronizingAudioBuffer* metaspu_construct(ESynchMethod method);

#endif
