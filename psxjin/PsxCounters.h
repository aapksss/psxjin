#ifndef __PSXCOUNTERS_H__
#define __PSXCOUNTERS_H__

typedef struct {
	unsigned long count, mode, target;
	unsigned long sCycle, Cycle, rate, interrupt;
} psxCounter;

extern psxCounter psxCounters[5];

extern unsigned long psxNextCounter, psxNextsCounter;

void psxRcntInit();
void psxRcntUpdate();
void psxRcntWcount(unsigned long index, unsigned long value);
void psxRcntWmode(unsigned long index, unsigned long value);
void psxRcntWtarget(unsigned long index, unsigned long value);
unsigned long psxRcntRcount(unsigned long index);
int psxRcntFreeze(EMUFILE *f, int Mode);

void psxUpdateVSyncRate();

#endif /* __PSXCOUNTERS_H__ */
