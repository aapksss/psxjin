#ifndef _SIO_H_
#define _SIO_H_

#include "emufile.h"

#define MCD_SIZE	(1024 * 8 * 16)

// Status Flags
#define TX_RDY		0x0001
#define RX_RDY		0x0002
#define TX_EMPTY	0x0004
#define PARITY_ERR	0x0008
#define RX_OVERRUN	0x0010
#define FRAMING_ERR	0x0020
#define SYNC_DETECT	0x0040
#define DSR			0x0080
#define CTS			0x0100
#define IRQ			0x0200

// Control Flags
#define TX_PERM		0x0001
#define DTR			0x0002
#define RX_PERM		0x0004
#define BREAK		0x0008
#define RESET_ERR	0x0010
#define RTS			0x0020
#define SIO_RESET	0x0040

extern unsigned short StatReg;
extern unsigned short ModeReg;
extern unsigned short CtrlReg;
extern unsigned short BaudReg;

extern char Mcd1Data[MCD_SIZE], Mcd2Data[MCD_SIZE];

unsigned char sioRead8();
void sioWrite8(unsigned char value);
void sioWriteCtrl16(unsigned short value);
void sioInterrupt();
int sioFreeze(EMUFILE *f, int Mode);

void LoadMcd(int mcd, char *str);
void LoadMcds(char *mcd1, char *mcd2);
void SaveMcd(char *mcd, char *data, unsigned long adr, int size);
void CreateMcd(char *mcd);
void ConvertMcd(char *mcd, char *data);

typedef struct {
	char Title[48];
	short sTitle[48];
	char ID[14];
	char Name[16];
	int IconCount;
	short Icon[16*16*3];
	unsigned char Flags;
} McdBlock;

void GetMcdBlockInfo(int mcd, int block, McdBlock *info);

void SIO_SaveMemoryCardsEmbed(char *file,char slot);
void SIO_LoadMemoryCardsEmbed(char *file);
void SIO_ClearMemoryCardsEmbed();
void SIO_UnsetTempMemoryCards();

#endif
