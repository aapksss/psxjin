#ifndef _REVERB_H_
#define _REVERB_H_

#include "spu.h"

void StartREVERB(SPU_chan* pChannel);
void StoreREVERB(SPU_chan* pChannel,s32 left, s32 right);
s32 MixREVERBLeft(SPU_struct* spu);
s32 MixREVERBRight(SPU_struct* spu);
void REVERB_initSample();

#endif //_REVERB_H_
