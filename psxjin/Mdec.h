#ifndef __MDEC_H__
#define __MDEC_H__

void mdecInit();
void mdecWrite0(u32 data);
void mdecWrite1(u32 data);
u32  mdecRead0();
u32  mdecRead1();
void psxDma0(u32 madr, u32 bcr, u32 chcr);
void psxDma1(u32 madr, u32 bcr, u32 chcr);
void mdec1Interrupt();
int  mdecFreeze(EMUFILE *f, int Mode);

#endif /* __MDEC_H__ */
