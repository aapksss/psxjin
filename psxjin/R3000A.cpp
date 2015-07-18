/*  PSXjin - Pc Psx Emulator
 *  Copyright (C) 1999-2003  PSXjin Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psxcommon.h"
#include "cdrom.h"

// Global variables
R3000Acpu *psxCpu;
psxRegisters psxRegs;
ExceptionPatches exceptionPatches;

int psxInit() {

	psxCpu = &psxInt;
#if defined(__i386__) || defined(__sh__)
	//if (!Config.Cpu) psxCpu = &psxRec;
#endif
	Log=0;

	if (psxMemInit() == -1) return -1;

	return psxCpu->Init();
}

void psxReset() {
	psxCpu->Reset();

	psxMemReset();
	exceptionPatches.clear();

	memset(&psxRegs, 0, sizeof(psxRegs));

	psxRegs.pc = 0xbfc00000; // Start in bootstrap

	psxRegs.CP0.r[12] = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15] = 0x00000002; // PRevID = Revision ID, same as R3000A

	psxHwReset();
	psxBiosInit();

	psxExecuteBios();

#ifdef EMU_LOG
	EMU_LOG("*BIOS END*\n");
#endif
	Log=0;
}

void psxShutdown() {
	psxMemShutdown();
	psxBiosShutdown();

	psxCpu->Shutdown();
}

void psxException(u32 code, u32 bd) {
	// Set the Cause
	psxRegs.CP0.n.Cause = code;

	// Set the EPC & PC
	if (bd) {
#ifdef PSXCPU_LOG
		PSXCPU_LOG("bd set!!!\n");
#endif
		SysPrintf("bd set!!!\n");
		psxRegs.CP0.n.Cause|= 0x80000000;
		psxRegs.CP0.n.EPC = (psxRegs.pc - 4);
	} else
		psxRegs.CP0.n.EPC = (psxRegs.pc);

	if (psxRegs.CP0.n.Status & 0x400000)
		psxRegs.pc = 0xbfc00180;
	else
		psxRegs.pc = 0x80000080;

	// Set the Status
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status &~0x3f) |
						  ((psxRegs.CP0.n.Status & 0xf) << 2);

	// Ugly hack to prevent the BIOS from (correctly) skipping over GTE ops when an exception is
	// raised while one is due to be executed. On real hardware, the GTE op is executed in parallel
	// with the exception being raised, not after its return. See: "GTE opcodes are not unpatched on
	// exception return" in the bugtracker. Is this bug still on Google Code? We should check.
	if (!Config.HLE && (((PSXMu32(psxRegs.CP0.n.EPC) >> 24) & 0xfe) == 0x4a)) {
		if (exceptionPatches.size() >= 8) {
			// This should never happen; the official PSX kernel (BIOS) does not handle nested exceptions,
			// but just to be sure...
			GPUdisplayText("*PSXjin*: Too many exception op patches, forgetting earliest");
			exceptionPatches.erase(exceptionPatches.begin(), exceptionPatches.end() - 7);
		}

		// Store the patch information so it can be undone on exception return
		exceptionPatches.push_back(std::make_pair(psxRegs.CP0.n.EPC, PSXMu32ref(psxRegs.CP0.n.EPC)));

		// Mangle the GTE op so that it is not recognized by the BIOS for not-skipping
		PSXMu32ref(psxRegs.CP0.n.EPC) &= SWAPu32(~0x02000000);
	}

	if (Config.HLE) psxBiosException();
}

void psxBranchTest() {
	if ((psxRegs.cycle - psxNextsCounter) >= psxNextCounter)
		psxRcntUpdate();

	if (psxRegs.interrupt) {
		if ((psxRegs.interrupt & 0x80) && (!Config.Sio)) { // SIO
			if ((psxRegs.cycle - psxRegs.intCycle[7]) >= psxRegs.intCycle[7+1]) {
				psxRegs.interrupt&=~0x80;
				sioInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x04) { // CDR
			if ((psxRegs.cycle - psxRegs.intCycle[2]) >= psxRegs.intCycle[2+1]) {
				psxRegs.interrupt&=~0x04;
				cdrInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x040000) { // CDR read
			if ((psxRegs.cycle - psxRegs.intCycle[2+16]) >= psxRegs.intCycle[2+16+1]) {
				psxRegs.interrupt&=~0x040000;
				cdrReadInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x01000000) { // GPU DMA
			if ((psxRegs.cycle - psxRegs.intCycle[3+24]) >= psxRegs.intCycle[3+24+1]) {
				psxRegs.interrupt&=~0x01000000;
				gpuInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x02000000) { // MDEC out DMA
			if ((psxRegs.cycle - psxRegs.intCycle[5+24]) >= psxRegs.intCycle[5+24+1]) {
				psxRegs.interrupt&=~0x02000000;
				mdec1Interrupt();
			}
		}
	}

	if (psxHu32(0x1070) & psxHu32(0x1074)) {
		if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
#ifdef PSXCPU_LOG
//			PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
//			SysPrintf("Interrupt (%x): %x %x\n", psxRegs.cycle, psxHu32(0x1070), psxHu32(0x1074));
			psxException(0x400, 0);
		}
	}

	if (!Config.HLE && Config.PsxOut) {
		u32 call = psxRegs.GPR.n.t1 & 0xff;
		switch (psxRegs.pc & 0x1fffff) {
			case 0xa0:
#ifdef PSXBIOS_LOG
				if (call != 0x28 && call != 0xe) {
					PSXBIOS_LOG("BIOS call a0: %s (%x) %x,%x,%x,%x\n", biosA0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosA0[call])
					biosA0[call]();
				break;
			case 0xb0:
#ifdef PSXBIOS_LOG
				if (call != 0x17 && call != 0xb) {
					PSXBIOS_LOG("BIOS call b0: %s (%x) %x,%x,%x,%x\n", biosB0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosB0[call])
					biosB0[call]();
				break;
			case 0xc0:
#ifdef PSXBIOS_LOG
				PSXBIOS_LOG("BIOS call c0: %s (%x) %x,%x,%x,%x\n", biosC0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3);
#endif
				if (biosC0[call])
					biosC0[call]();
				break;
		}
	}
//	if (psxRegs.cycle > 0xd29c6500) Log=1;
}

void psxExecuteBios() {
	while (psxRegs.pc != 0x80030000)
		psxCpu->ExecuteBlock();
}
