#include "psxcommon.h"

// DMA 0/1 in mdec.c
// DMA 3   in cdrom.c

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
	u16 *ptr;
	u32 size;

	switch (chcr) {
		case 0x01000201: // CPU to SPU transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - mem2spu *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - mem2spu *** NULL Pointer!!!\n");
#endif
				break;
			}
			SPUwriteDMAMem(ptr, (bcr >> 16) * (bcr & 0xffff) * 2);
			break;

		case 0x01000200: // SPU to CPU transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - spu2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - spu2mem *** NULL Pointer!!!\n");
#endif
				break;
			}
			size = (bcr >> 16) * (bcr & 0xffff) * 2;
    		SPUreadDMAMem(ptr, size);
			psxCpu->Clear(madr, size);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA4 SPU - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}

	HW_DMA4_CHCR &= ~0x01000000;
	DMA_INTERRUPT(4);
}

void psxDma2(u32 madr, u32 bcr, u32 chcr) { // GPU
	u32 *ptr;
	u32 size;

	switch(chcr) {
		case 0x01000200: // vram2mem
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA2 GPU - vram2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA2 GPU - vram2mem *** NULL Pointer!!!\n");
#endif
				break;
			}
			size = (bcr >> 16) * (bcr & 0xffff);
			GPUreadDataMem(ptr, size);
			psxCpu->Clear(madr, size);
			break;

		case 0x01000201: // mem2vram
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU mem2vram *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA2 GPU - mem2vram *** NULL Pointer!!!\n");
#endif
				break;
			}
			size = (bcr >> 16) * (bcr & 0xffff);
			GPUwriteDataMem(ptr, size);
			GPUDMA_INT((size / 4) / BIAS);
			return;
//			break;

		case 0x01000401: // DMA chain
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU dma chain *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			iGpuHasUpdated=1;
			GPUdmaChain((u32 *)psxM, madr & 0x1fffff);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA 2 - GPU unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}

	HW_DMA2_CHCR &= ~0x01000000;
	DMA_INTERRUPT(2);
}

void gpuInterrupt() {
	HW_DMA2_CHCR &= ~0x01000000;
	DMA_INTERRUPT(2);
}

void psxDma6(u32 madr, u32 bcr, u32 chcr) {
	u32 *mem = (u32 *)PSXM(madr);

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA6 OT *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		if (mem == NULL) {
#ifdef CPU_LOG
			CPU_LOG("*** DMA6 OT *** NULL Pointer!!!\n");
#endif
			HW_DMA6_CHCR &= ~0x01000000;
			DMA_INTERRUPT(6);
			return;
		}

		while (bcr--) {
			*mem-- = (madr - 4) & 0xffffff;
			madr -= 4;
		}
		mem++; *mem = 0xffffff;
	}
#ifdef PSXDMA_LOG
	else {
		// Unknown option - we should investigate
		PSXDMA_LOG("*** DMA6 OT - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
	}
#endif

	HW_DMA6_CHCR &= ~0x01000000;
	DMA_INTERRUPT(6);
}
