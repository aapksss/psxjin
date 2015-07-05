#ifndef __PSXBIOS_H__
#define __PSXBIOS_H__

extern char *biosA0n[256];
extern char *biosB0n[256];
extern char *biosC0n[256];

void psxBiosInit();
void psxBiosShutdown();
void psxBiosException();
void psxBiosFreeze(int Mode);

extern void (*biosA0[256])();
extern void (*biosB0[256])();
extern void (*biosC0[256])();

#endif /* __PSXBIOS_H__ */
