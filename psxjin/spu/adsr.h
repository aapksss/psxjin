#ifndef _ADSR_H_
#define _ADSR_H_

#include "spu.h"
#include "psxcommon.h"

void StaticInitADSR();
void StartADSR(SPU_chan * pChannel);
s32  MixADSR(SPU_chan * pChannel);

#endif //_ADSR_H_
