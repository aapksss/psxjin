/***************************************************************************
                            spu.h  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#ifndef _SPU_H_
#define _SPU_H_

#include "psxcommon.h"
#include "decodexa.h"
#include "emufile.h"
#include "xa.h"

#define NSSIZE 1

extern int iVolume;	// Volume control range: 1-5

void SPUplayBRRchannel(xa_decode_t *xap);

#define CHANSTATUS_STOPPED          0
#define CHANSTATUS_PLAY             1
#define CHANSTATUS_KEYOFF             2

struct REVERBInfo
{
	REVERBInfo()
	{
		memset(arr,0,sizeof(arr));
	}
	union
	{
		struct
		{
			s32 StartAddr;      // Reverb area start addr in samples
			s32 CurrAddr;       // Reverb area curr addr in samples

			s32 VolLeft;
			s32 VolRight;
			s32 iLastRVBLeft;
			s32 iLastRVBRight;
			s32 iRVBLeft;
			s32 iRVBRight;


			s32 FB_SRC_A;       // (offset)
			s32 FB_SRC_B;       // (offset)
			s32 IIR_ALPHA;      // (coefficient)
			s32 ACC_COEF_A;     // (coefficient)
			s32 ACC_COEF_B;     // (coefficient)
			s32 ACC_COEF_C;     // (coefficient)
			s32 ACC_COEF_D;     // (coefficient)
			s32 IIR_COEF;       // (coefficient)
			s32 FB_ALPHA;       // (coefficient)
			s32 FB_X;           // (coefficient)
			s32 IIR_DEST_A0;    // (offset)
			s32 IIR_DEST_A1;    // (offset)
			s32 ACC_SRC_A0;     // (offset)
			s32 ACC_SRC_A1;     // (offset)
			s32 ACC_SRC_B0;     // (offset)
			s32 ACC_SRC_B1;     // (offset)
			s32 IIR_SRC_A0;     // (offset)
			s32 IIR_SRC_A1;     // (offset)
			s32 IIR_DEST_B0;    // (offset)
			s32 IIR_DEST_B1;    // (offset)
			s32 ACC_SRC_C0;     // (offset)
			s32 ACC_SRC_C1;     // (offset)
			s32 ACC_SRC_D0;     // (offset)
			s32 ACC_SRC_D1;     // (offset)
			s32 IIR_SRC_B1;     // (offset)
			s32 IIR_SRC_B0;     // (offset)
			s32 MIX_DEST_A0;    // (offset)
			s32 MIX_DEST_A1;    // (offset)
			s32 MIX_DEST_B0;    // (offset)
			s32 MIX_DEST_B1;    // (offset)
			s32 IN_COEF_L;      // (coefficient)
			s32 IN_COEF_R;      // (coefficient)
		};

		s32 arr[41];
	};

	void save(EMUFILE* fp);
	void load(EMUFILE* fp);
};

struct _ADSRInfo
{
	_ADSRInfo();
	s32            State;
	s32            AttackModeExp;
	s32            AttackRate;
	s32            DecayRate;
	s32            SustainLevel;
	s32            SustainModeExp;
	s32            SustainIncrease;
	s32            SustainRate;
	s32            ReleaseModeExp;
	s32            ReleaseRate;
	s32            EnvelopeVol;
	s32           lVolume;
	void save(EMUFILE* fp);
	void load(EMUFILE* fp);
};

//typedef struct
//{
//	int            AttackModeExp;
//	long           AttackTime;
//	long           DecayTime;
//	long           SustainLevel;
//	int            SustainModeExp;
//	long           SustainModeDec;
//	long           SustainTime;
//	int            ReleaseModeExp;
//	unsigned long  ReleaseVal;
//	long           ReleaseTime;
//	long           ReleaseStartTime;
//	long           ReleaseVol;
//	long           lTime;
//	long           lVolume;
//} ;

class SPU_struct;

class SPU_chan
{
public:
	u32 ch;
	SPU_struct* spu;

	SPU_chan();

	void save(EMUFILE* fp);
	void load(EMUFILE* fp);

	void keyon();
	void keyoff();

	u8 status;
	s32               iLeftVolume;                        // Left volume
	s32               iLeftVolRaw;                        // Left PS1 volume value
	s32               iRightVolume;                        // Left volume
	s32               iRightVolRaw;                        // Left PS1 volume value

	// Call this to update smpinc from the provided pitch
	
	void updatePitch(u16 rawPitch);

	// Increment value 0000-3fff with 1000 being unit speed
	
	u16 rawPitch;
	
	u16 rawStartAddr; //= RealStartAddr = startAddr<<3, used as a blockAddress
	u32 loopStartAddr; // The loop start point, used as a blockAddress

	// Stores adsr values from the software and tracks them at runtime
	_ADSRInfo ADSR;

	// Runtime information never really exposed to CPU
	double smpinc; // Sample frequency stepper value IE 2.000 equals octave+1
	double smpcnt; // Current sample counter within the current block, kept under 28
	s32 blockAddress; // The current 16B encoded block address

	// Are these features enabled for this channel?
	
	u8 bFMod,bReverb;

	// This is set if the channel starts with bReverb enabled
	// So that changes to the reverb master enable don't affect currently running samples?
	// Is that even right?
	// Check if this is right
	
	u8 bRVBActive;

	// Noise state
	
	s32 iOldNoise;
	u8 bNoise;

	u32 pending;

	static const int NOISE_PENDING = 1;

	// BRR decoding and state
	// Note that the block is larger than the 28 sample block size
	// This is a clever hack to help keep previous samples available for interpolation
	// The final four samples of a block will get moved to 28,29,30,31 in order to wrap around
	// When interpolating using samples before sample 0 of the current block
	
	s16 block[32];
	s32 s_1,s_2;
	u8 flags;
	s32 decodeBRR(SPU_struct* spu);

};

class SPU_struct
{
public:
	SPU_struct(bool _isCore);
	~SPU_struct();
	
	void triggerIrqRange(u32 base, u32 size);

	SPU_chan channels[24];
	s16 outbuf[100000];
	void writeRegister(u32 r, u16 val);
	bool isCore;
	double mixtime;
	u32 dwNoiseVal;

	s32 iLeftXAVol, iRightXAVol;

	u8 iReverbCycle;
	u32 spuAddr;
	u16 spuCtrl;
	u16 spuStat;
	u16 spuIrq;
	u32 mixIrqCounter;
	u16 spuMem[256*1024];
	inline u8 readSpuMem(u32 addr) { return ((u8*)spuMem)[addr]; }

	xa_queue xaqueue;

	// Reverb
	s32 sRVBBuf[2];
	REVERBInfo rvb;
	void REVERB_initSample();
	void StoreREVERB(SPU_chan* pChannel,s32 left, s32 right);
	int MixREVERBLeft();
	int MixREVERBRight();
	int g_buffer(int iOff);
	void s_buffer(int iOff,int iVal);
	void s_buffer1(int iOff,int iVal);

	// DMA
	u16 SPUreadDMA(void);
	void SPUreadDMAMem(u16 * pusPSXMem,int iSize);
	void SPUwriteDMA(u16 val);
	void SPUwriteDMAMem(u16 * pusPSXMem,int iSize);

};

extern SPU_struct *SPU_core, *SPU_user;
extern int iUseReverb;

u16 SPUreadDMA();
void SPUreadDMAMem(u16 * pusPSXMem,int iSize);
void SPUwriteDMA(u16 val);
void SPUwriteDMAMem(u16 * pusPSXMem,int iSize);

void SPUfreeze_new(EMUFILE* fp);
bool SPUunfreeze_new(EMUFILE* fp);
void SPUcloneUser();

void SPUmute();
void SPUunMute();
void SPUReset();

class Lock {
public:
	Lock(); // Defaults to the critical section around NDS_exec (I think this has to do with Desmume, not PSXjin. I am going to leave it here until confirmed)
	Lock(CRITICAL_SECTION& cs);
	~Lock();
private:
	CRITICAL_SECTION* m_cs;
};

#endif
