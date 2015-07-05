#ifndef __CDROM_H__
#define __CDROM_H__

#include "PsxCommon.h"
#include "Decode_XA.h"

struct cdrStruct{
	unsigned char OCUP;
	unsigned char Reg1Mode;
	unsigned char Reg2;
	unsigned char CmdProcess;
	unsigned char Ctrl;
	unsigned char Stat;

	unsigned char StatP;

	unsigned char Transfer[2352];
	unsigned char *pTransfer;

	unsigned char Prev[4];
	unsigned char Param[8];
	unsigned char Result[8];

	unsigned char ParamC;
	unsigned char ParamP;
	unsigned char ResultC;
	unsigned char ResultP;
	unsigned char ResultReady;
	unsigned char Cmd;
	unsigned char Readed;
	unsigned long Reading;

	unsigned char ResultTN[6];
#ifdef __DREAMCAST__
	unsigned char ResultTD[4] __attribute__ ((aligned (4)));
#else
	unsigned char ResultTD[4];
#endif
	unsigned char SetSector[4];
	unsigned char SetSectorSeek[4];
	unsigned char Track;
	int Play;
	int CurTrack;
	int Mode, File, Channel, Muted;
	int Reset;
	int RErr;
	int FirstSector;

	xa_decode_t Xa;

	int Init;

	unsigned char Irq;
	unsigned long eCycle;

	int Seeked;

	char Unused[4083];
};

extern cdrStruct cdr;

void cdrReset();
void cdrInterrupt();
void cdrReadInterrupt();
unsigned char cdrRead0(void);
unsigned char cdrRead1(void);
unsigned char cdrRead2(void);
unsigned char cdrRead3(void);
void cdrWrite0(unsigned char rt);
void cdrWrite1(unsigned char rt);
void cdrWrite2(unsigned char rt);
void cdrWrite3(unsigned char rt);
int cdrFreeze(EMUFILE *f, int Mode);

#endif /* __CDROM_H__ */
