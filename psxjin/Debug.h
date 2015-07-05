#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "PsxCommon.h"


extern char *disRNameCP0[];

char* disR3000AF(u32 code, u32 pc);

extern FILE *emuLog;

//#define GTE_DUMP

#ifdef GTE_DUMP
FILE *gteLog;
#endif

//#define LOG_STDOUT

//#define PAD_LOG  __Log
//#define GTE_LOG  __Log
//#define CDR_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log

//#define PSXHW_LOG   __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXBIOS_LOG __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXDMA_LOG  __Log
//#define PSXMEM_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXCPU_LOG  __Log

//#define CDRCMD_DEBUG

#if defined (PSXCPU_LOG) || defined(PSXDMA_LOG) || defined(CDR_LOG) || defined(PSXHW_LOG) || \
	defined(PSXBIOS_LOG) || defined(PSXMEM_LOG) || defined(GTE_LOG)    || defined(PAD_LOG)
#define EMU_LOG __Log
#endif

#endif /* __DEBUG_H__ */
