#include "stdafx.h"

#define _IN_FREEZE

#include "externals.h"
#include "registers.h"
#include "spu.h"
#include "emufile.h"

void SPU_chan::save(EMUFILE* fp)
{
	fp->write32le((u32)1); // Version

	fp->write8le(status);

	fp->write32le(iLeftVolume);
	fp->write32le(iLeftVolRaw);
	fp->write32le(iRightVolume);
	fp->write32le(iRightVolRaw);

	fp->write16le(rawPitch);

	fp->write16le(rawStartAddr);
	fp->write32le(loopStartAddr);

	ADSR.save(fp);

	fp->writedouble(smpinc);
	fp->writedouble(smpcnt);
	fp->write32le(blockAddress);

	fp->write8le(bFMod);
	fp->write8le(bReverb);
	fp->write8le(bRVBActive);

	fp->write32le(iOldNoise);
	fp->write8le(bNoise);

	for(int i=0;i<32;i++)
		fp->write16le(block[i]);
	fp->write32le(s_1);
	fp->write32le(s_2);
	fp->write8le(flags);

	fp->write32le(pending);
	fp->write32le((u32)0);
	fp->write32le((u32)0);
}

void SPU_chan::load(EMUFILE* fp)
{
	u32 version = fp->read32le();

	fp->read8le(&status);

	fp->read32le(&iLeftVolume);
	fp->read32le(&iLeftVolRaw);
	fp->read32le(&iRightVolume);
	fp->read32le(&iRightVolRaw);

	fp->read16le(&rawPitch);

	fp->read16le(&rawStartAddr);
	fp->read32le(&loopStartAddr);

	ADSR.load(fp);

	fp->readdouble(&smpinc);
	fp->readdouble(&smpcnt);
	fp->read32le(&blockAddress);

	fp->read8le(&bFMod);
	fp->read8le(&bReverb);
	fp->read8le(&bRVBActive);

	fp->read32le(&iOldNoise);
	fp->read8le(&bNoise);

	for(int i=0;i<32;i++)
		fp->read16le(&block[i]);
	fp->read32le(&s_1);
	fp->read32le(&s_2);
	fp->read8le(&flags);

	s32 temp;
	fp->read32le(&pending);
	fp->read32le(&temp);
	fp->read32le(&temp);
}

void SPUfreeze_new(EMUFILE* fp)
{
	// To do - save noise seed
	// Need to implement this

	const u32 tag = 0xBEEFFACE;
	fp->write32le(tag);

	fp->write32le((u32)0); // Version

	CTASSERT(sizeof(SPU_core->spuMem)==0x80000);
	fp->fwrite(SPU_core->spuMem,0x80000);
	fp->fwrite(regArea,0x200);
	
	fp->writedouble(&SPU_core->mixtime);
	fp->write32le(&SPU_core->dwNoiseVal);
	fp->write32le(&SPU_core->iLeftXAVol);
	fp->write32le(&SPU_core->iRightXAVol);

	fp->write8le(&SPU_core->iReverbCycle);
	fp->write32le(&SPU_core->spuAddr);
	fp->write16le(&SPU_core->spuCtrl);
	fp->write16le(&SPU_core->spuStat);
	fp->write16le(&SPU_core->spuIrq);

	fp->write32le(&SPU_core->mixIrqCounter);

	fp->write32le(&SPU_core->sRVBBuf[0]);
	fp->write32le(&SPU_core->sRVBBuf[1]);
	SPU_core->rvb.save(fp);

	for(int i=0;i<MAXCHAN;i++) SPU_core->channels[i].save(fp);

	SPU_core->xaqueue.freeze(fp);

	// A bit weird to do this here, but I wanted to have a more solid XA state saver
	// and the CDR freeze sucks. I made sure this saves and loads after the CDR state
	
	cdr.Xa.save(fp);

	const u32 endtag = 0xBAADF00D;
	fp->write32le(endtag);
}
