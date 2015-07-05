#ifndef __PSXDMA_H__
#define __PSXDMA_H__

#define GPUDMA_INT(eCycle) { \
	psxRegs.interrupt |= 0x01000000; \
	psxRegs.intCycle[3+24+1] = eCycle; \
	psxRegs.intCycle[3+24] = psxRegs.cycle; \
}

#define MDECOUTDMA_INT(eCycle) { \
	psxRegs.interrupt |= 0x02000000; \
	psxRegs.intCycle[5+24+1] = eCycle; \
	psxRegs.intCycle[5+24] = psxRegs.cycle; \
}

void psxDma2(u32 madr, u32 bcr, u32 chcr);
void psxDma3(u32 madr, u32 bcr, u32 chcr);
void psxDma4(u32 madr, u32 bcr, u32 chcr);
void psxDma6(u32 madr, u32 bcr, u32 chcr);
void gpuInterrupt();

#endif /* __PSXDMA_H__ */
