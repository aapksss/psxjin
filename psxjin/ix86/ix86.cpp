#include <stdio.h>
#include <string.h>
#include "ix86.h"

// Global variables

s8  x86ID[16];	// Vendor ID
u32 x86Family;	// Processor family
u32 x86Model;	// Processor model
u32 x86PType;	// Processor type
u32 x86StepID;	// Stepping ID
u32 x86Flags;	// Feature flags
u32 x86EFlags;	// Extended feature flags
s8  *x86Ptr;
u8  *j8Ptr[32];
u32 *j32Ptr[32];

static s32 iCpuId(s32 cmd, u32 *regs) {
	int flag;

	__asm {
		push ebx
		push edi

		pushfd;
		pop eax;
		mov edx, eax;
		xor eax, 1 << 21;
		push eax;
		popfd;
		pushfd;
		pop eax;
		xor eax, edx;
		mov flag, eax;
	}
	if (!flag) return -1;

	__asm {
		mov eax, cmd;
		cpuid;
		mov flag, edx;
		mov edi, [regs]
		mov [edi], eax;
		mov [edi+4], ebx;
		mov [edi+8], ecx;
		mov [edi+12], edx;

		pop edi
		pop ebx
	}

	return 0;

}

void x86Init() {
	u32 regs[4];
	u32 cmds;

	memset(x86ID, 0, sizeof(x86ID));
	x86Family = 0;
	x86Model  = 0;
	x86PType  = 0;
	x86StepID = 0;
	x86Flags  = 0;
	x86EFlags = 0;
	
	if (iCpuId(0, regs) != -1) {
		cmds = regs[0];
		((u32*)x86ID)[0] = regs[1];
		((u32*)x86ID)[1] = regs[3];
		((u32*)x86ID)[2] = regs[2];
		if (cmds >= 0x00000001) {
			if (iCpuId(0x00000001, regs) != -1) {
				x86StepID =  regs[0]        & 0xf;
				x86Model  = (regs[0] >>  4) & 0xf;
				x86Family = (regs[0] >>  8) & 0xf;
				x86PType  = (regs[0] >> 12) & 0x3;
				x86Flags = regs[3];
			}
		}

		if (iCpuId(0x80000000, regs) != -1) {
			cmds = regs[0];
			if (cmds >= 0x80000001) {
				if (iCpuId(0x80000001, regs) != -1) {
					x86EFlags = regs[3];
				}
			}
		}
	}
}

void x86SetPtr(char *ptr) {
	x86Ptr = ptr;
}

void x86Shutdown() {
}

void x86SetJ8(u8 *j8) {
	u32 jump = (x86Ptr - (s8*)j8) - 1;

	if (jump > 0x7f) printf("j8 greater than 0x7f!!\n");
	*j8 = (u8)jump;
}

void x86SetJ32(u32 *j32) {
	*j32 = (x86Ptr - (s8*)j32) - 4;
}

void x86Align(int bytes) {
	// Forward align
	x86Ptr = (s8*)(((u32)x86Ptr + bytes) & ~(bytes - 1));
}

#define SIB 4
#define DISP32 5

/* Macros helpers */

#define ModRM(mod, rm, reg) \
	write8((mod << 6) | (rm << 3) | (reg));

#define SibSB(ss, rm, index) \
	write8((ss << 6) | (rm << 3) | (index));

#define SET8R(cc, to) { \
	write8(0x0F); write8(cc); \
	write8((0xC0) | (to)); }

#define J8Rel(cc, to) { \
	write8(cc); write8(to); return (u8*)(x86Ptr - 1); }

#define J32Rel(cc, to) { \
	write8(0x0F); write8(cc); write32(to); return (u32*)(x86Ptr - 4); }

#define CMOV32RtoR(cc, to, from) { \
	write8(0x0F); write8(cc); \
	ModRM(3, to, from); }

#define CMOV32MtoR(cc, to, from) { \
	write8(0x0F); write8(cc); \
	ModRM(0, to, DISP32); \
	write32(from); }

// IX86 instructions

// MOV instructions

// MOV r32 to r32

void MOV32RtoR(int to, int from) {
	write8(0x89);
	ModRM(3, from, to);
}

// MOV r32 to m32

void MOV32RtoM(u32 to, int from) {
	write8(0x89);
	ModRM(0, from, DISP32);
	write32(to);
}

// MOV m32 to r32

void MOV32MtoR(int to, u32 from) {
	write8(0x8B);
	ModRM(0, to, DISP32);
	write32(from); 
}

// MOV [r32] to r32

void MOV32RmtoR(int to, int from) {
	write8(0x8B);
	ModRM(0, to, from);
}

// MOV [r32][r32*scale] to r32

void MOV32RmStoR(int to, int from, int from2, int scale) {
	write8(0x8B);
	ModRM(0, to, 0x4);
	SibSB(scale, from2, from);
}

// MOV r32 to [r32]

void MOV32RtoRm(int to, int from) {
	write8(0x89);
	ModRM(0, from, to);
}

// MOV r32 to [r32][r32*scale]

void MOV32RtoRmS(int to, int to2, int scale, int from) {
	write8(0x89);
	ModRM(0, from, 0x4);
	SibSB(scale, to2, to);
}

// MOV imm32 to r32 */

void MOV32ItoR(int to, u32 from) {
	write8(0xB8 | to); 
	write32(from);
}

// MOV imm32 to m32

void MOV32ItoM(u32 to, u32 from) {
	write8(0xC7);
	ModRM(0, 0, DISP32);
	write32(to);
	write32(from); 
}

// MOV r16 to m16

void MOV16RtoM(u32 to, int from) {
	write8(0x66);
	write8(0x89);
	ModRM(0, from, DISP32);
	write32(to);
}

// MOV m16 to r16

void MOV16MtoR(int to, u32 from) {
	write8(0x66);
	write8(0x8B);
	ModRM(0, to, DISP32);
	write32(from); 
}

// MOV imm16 to m16

void MOV16ItoM(u32 to, u16 from) {
	write8(0x66);
	write8(0xC7);
	ModRM(0, 0, DISP32);
	write32(to);
	write16(from); 
}

// MOV r8 to m8

void MOV8RtoM(u32 to, int from) {
	write8(0x88);
	ModRM(0, from, DISP32);
	write32(to);
}

// MOV m8 to r8

void MOV8MtoR(int to, u32 from) {
	write8(0x8A);
	ModRM(0, to, DISP32);
	write32(from); 
}

// MOV imm8 to m8

void MOV8ItoM(u32 to, u8 from) {
	write8(0xC6);
	ModRM(0, 0, DISP32);
	write32(to);
	write8(from); 
}

// MOVSX r8 to r32

void MOVSX32R8toR(int to, int from) {
	write16(0xBE0F); 
	ModRM(3, to, from); 
}

// MOVSX m8 to r32

void MOVSX32M8toR(int to, u32 from) {
	write16(0xBE0F); 
	ModRM(0, to, DISP32);
	write32(from);
}

// MOVSX r16 to r32

void MOVSX32R16toR(int to, int from) {
	write16(0xBF0F); 
	ModRM(3, to, from); 
}

// MOVSX m16 to r32

void MOVSX32M16toR(int to, u32 from) {
	write16(0xBF0F); 
	ModRM(0, to, DISP32);
	write32(from);
}

// MOVZX r8 to r32

void MOVZX32R8toR(int to, int from) {
	write16(0xB60F); 
	ModRM(3, to, from); 
}

// MOVZX m8 to r32

void MOVZX32M8toR(int to, u32 from) {
	write16(0xB60F); 
	ModRM(0, to, DISP32);
	write32(from);
}

// MOVZX r16 to r32

void MOVZX32R16toR(int to, int from) {
	write16(0xB70F); 
	ModRM(3, to, from); 
}

// MOVZX m16 to r32

void MOVZX32M16toR(int to, u32 from) {
	write16(0xB70F); 
	ModRM(0, to, DISP32);
	write32(from);
}

// CMOVNE r32 to r32

void CMOVNE32RtoR(int to, int from) {
	CMOV32RtoR(0x45, to, from);
}

// CMOVNE m32 to r32

void CMOVNE32MtoR(int to, u32 from) {
	CMOV32MtoR(0x45, to, from);
}

// CMOVE r32 to r32

void CMOVE32RtoR(int to, int from) {
	CMOV32RtoR(0x44, to, from);
}

// CMOVE m32 to r32

void CMOVE32MtoR(int to, u32 from) {
	CMOV32MtoR(0x44, to, from);
}

// CMOVG r32 to r32

void CMOVG32RtoR(int to, int from) {
	CMOV32RtoR(0x4F, to, from);
}

// CMOVG m32 to r32

void CMOVG32MtoR(int to, u32 from) {
	CMOV32MtoR(0x4F, to, from);
}

// CMOVGE r32 to r32

void CMOVGE32RtoR(int to, int from) {
	CMOV32RtoR(0x4D, to, from);
}

// CMOVGE m32 to r32

void CMOVGE32MtoR(int to, u32 from) {
	CMOV32MtoR(0x4D, to, from);
}

// CMOVL r32 to r32

void CMOVL32RtoR(int to, int from) {
	CMOV32RtoR(0x4C, to, from);
}

// CMOVL m32 to r32

void CMOVL32MtoR(int to, u32 from) {
	CMOV32MtoR(0x4C, to, from);
}

// CMOVLE r32 to r32

void CMOVLE32RtoR(int to, int from) {
	CMOV32RtoR(0x4E, to, from);
}

// CMOVLE m32 to r32

void CMOVLE32MtoR(int to, u32 from) {
	CMOV32MtoR(0x4E, to, from);
}

// Arithmetic instructions

// ADD imm32 to r32

void ADD32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x05); 
	} else {
		write8(0x81); 
		ModRM(3, 0, to);
	}
	write32(from);
}

// ADD imm32 to m32

void ADD32ItoM(u32 to, u32 from) {
	write8(0x81); 
	ModRM(0, 0, DISP32);
	write32(to);
	write32(from);
}

// ADD r32 to r32

void ADD32RtoR(int to, int from) {
	write8(0x01); 
	ModRM(3, from, to);
}

// ADD r32 to m32

void ADD32RtoM(u32 to, int from) {
	write8(0x01); 
	ModRM(0, from, DISP32);
	write32(to);
}

// ADD m32 to r32

void ADD32MtoR(int to, u32 from) {
	write8(0x03); 
	ModRM(0, to, DISP32);
	write32(from);
}

// ADC imm32 to r32

void ADC32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x15);
	} else {
		write8(0x81);
		ModRM(3, 2, to);
	}
	write32(from); 
}

// ADC r32 to r32

void ADC32RtoR(int to, int from) {
	write8(0x11); 
	ModRM(3, from, to);
}

// ADC m32 to r32

void ADC32MtoR(int to, u32 from) {
	write8(0x13); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// INC r32

void INC32R(int to) {
	write8(0x40 + to);
}

// INC m32

void INC32M(u32 to) {
	write8(0xFF);
	ModRM(0, 0, DISP32);
	write32(to);
}

// SUB imm32 to r32

void SUB32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x2D); 
	} else {
		write8(0x81); 
		ModRM(3, 5, to);
	}
	write32(from); 
}

// SUB r32 to r32

void SUB32RtoR(int to, int from) {
	write8(0x29); 
	ModRM(3, from, to);
}

// SUB m32 to r32

void SUB32MtoR(int to, u32 from) {
	write8(0x2B); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// SBB imm32 to r32

void SBB32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x1D);
	} else {
		write8(0x81);
		ModRM(3, 3, to);
	}
	write32(from); 
}

// SBB r32 to r32

void SBB32RtoR(int to, int from) {
	write8(0x19); 
	ModRM(3, from, to);
}

// SBB m32 to r32

void SBB32MtoR(int to, u32 from) {
	write8(0x1B); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// DEC r32

void DEC32R(int to) {
	write8(0x48 + to);
}

// DEC m32

void DEC32M(u32 to) {
	write8(0xFF);
	ModRM(0, 1, DISP32);
	write32(to);
}

// MUL EAX by r32 to EDX:EAX

void MUL32R(int from) {
	write8(0xF7); 
	ModRM(3, 4, from);
}

// IMUL EAX by r32 to EDX:EAX

void IMUL32R(int from) {
	write8(0xF7); 
	ModRM(3, 5, from);
}

// MUL EAX by m32 to EDX:EAX

void MUL32M(u32 from) {
	write8(0xF7); 
	ModRM(0, 4, DISP32);
	write32(from); 
}

// IMUL EAX by m32 to EDX:EAX

void IMUL32M(u32 from) {
	write8(0xF7); 
	ModRM(0, 5, DISP32);
	write32(from); 
}

// IMUL r32 by r32 to r32

void IMUL32RtoR(int to, int from) {
	write16(0xAF0F); 
	ModRM(3, to, from);
}

// DIV EAX by r32 to EDX:EAX

void DIV32R(int from) {
	write8(0xF7); 
	ModRM(3, 6, from);
}

// IDIV EAX by r32 to EDX:EAX

void IDIV32R(int from) {
	write8(0xF7); 
	ModRM(3, 7, from);
}

// DIV EAX by m32 to EDX:EAX

void DIV32M(u32 from) {
	write8(0xF7); 
	ModRM(0, 6, DISP32);
	write32(from); 
}

// IDIV EAX by m32 to EDX:EAX

void IDIV32M(u32 from) {
	write8(0xF7); 
	ModRM(0, 7, DISP32);
	write32(from); 
}

// Shifting instructions

void RCR32ItoR(int to,int from)
{
	if (from==1)
	{
		write8(0xd1);
		write8(0xd8 | to);
	}
	else
	{
		write8(0xc1);
		write8(0xd8 | to);
		write8(from);
	}
}

// SHL imm8 to r32

void SHL32ItoR(int to, u8 from) {
	if (from==1)
	{
		write8(0xd1);
		write8(0xe0 | to);
		return;
	}
	write8(0xC1); 
	ModRM(3, 4, to);
	write8(from); 
}

// SHL CL to r32

void SHL32CLtoR(int to) {
	write8(0xD3); 
	ModRM(3, 4, to);
}

// SHR imm8 to r32

void SHR32ItoR(int to, u8 from) {
	if (from==1)
	{
		write8(0xd1);
		write8(0xe8 | to);
		return;
	}
	write8(0xC1); 
	ModRM(3, 5, to);
	write8(from); 
}

// SHR CL to r32

void SHR32CLtoR(int to) {
	write8(0xD3); 
	ModRM(3, 5, to);
}

// SAR imm8 to r32

void SAR32ItoR(int to, u8 from) {
	write8(0xC1); 
	ModRM(3, 7, to);
	write8(from); 
}

// SAR CL to r32

void SAR32CLtoR(int to) {
	write8(0xD3); 
	ModRM(3, 7, to);
}

// Logical instructions

// OR imm32 to r32

void OR32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x0D); 
	} else {
		write8(0x81); 
		ModRM(3, 1, to);
	}
	write32(from); 
}

// OR imm32 to m32

void OR32ItoM(u32 to, u32 from) {
	write8(0x81); 
	ModRM(0, 1, DISP32);
	write32(to);
	write32(from); 
}

// OR r32 to r32

void OR32RtoR(int to, int from) {
	write8(0x09); 
	ModRM(3, from, to);
}

// OR r32 to m32

void OR32RtoM(u32 to, int from) {
	write8(0x09); 
	ModRM(0, from, DISP32);
	write32(to);
}

// OR m32 to r32

void OR32MtoR(int to, u32 from) {
	write8(0x0B); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// XOR imm32 to r32

void XOR32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x35); 
	} else {
		write8(0x81); 
		ModRM(3, 6, to);
	}
	write32(from); 
}

// XOR imm32 to m32

void XOR32ItoM(u32 to, u32 from) {
	write8(0x81); 
	ModRM(0, 6, DISP32);
	write32(to); 
	write32(from); 
}

// XOR r32 to r32

void XOR32RtoR(int to, int from) {
	write8(0x31); 
	ModRM(3, from, to);
}

// XOR r32 to m32

void XOR32RtoM(u32 to, int from) {
	write8(0x31); 
	ModRM(0, from, DISP32);
	write32(to);
}

// XOR m32 to r32

void XOR32MtoR(int to, u32 from) {
	write8(0x33); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// AND imm32 to r32

void AND32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x25); 
	} else {
		write8(0x81); 
		ModRM(3, 0x4, to);
	}
	write32(from); 
}

// AND imm32 to m32

void AND32ItoM(u32 to, u32 from) {
	write8(0x81); 
	ModRM(0, 0x4, DISP32);
	write32(to);
	write32(from); 
}

// AND r32 to r32

void AND32RtoR(int to, int from) {
	write8(0x21); 
	ModRM(3, from, to);
}

// AND r32 to m32

void AND32RtoM(u32 to, int from) {
	write8(0x21); 
	ModRM(0, from, DISP32);
	write32(to); 
}

// AND m32 to r32

void AND32MtoR(int to, u32 from) {
	write8(0x23); 
	ModRM(0, to, DISP32);
	write32(from); 
}

// NOT r32

void NOT32R(int from) {
	write8(0xF7); 
	ModRM(3, 2, from);
}

// NEG r32

void NEG32R(int from) {
	write8(0xF7); 
	ModRM(3, 3, from);
}

// Jump instructions

// JMP rel8

u8*  JMP8(u8 to) {
	write8(0xEB); 
	write8(to);
	return (u8*)(x86Ptr - 1);
}

// JMP rel32

u32* JMP32(u32 to) {
	write8(0xE9); 
	write32(to); 
	return (u32*)(x86Ptr - 4);
}

// JMP r32

void JMP32R(int to) {
	write8(0xFF); 
	ModRM(3, 4, to);
}

// JE rel8

u8*  JE8(u8 to) {
	J8Rel(0x74, to);
}

// JZ rel8

u8*  JZ8(u8 to) {
	J8Rel(0x74, to); 
}

// JG rel8

u8*  JG8(u8 to) { 
	J8Rel(0x7F, to);
}

// JGE rel8

u8*  JGE8(u8 to) { 
	J8Rel(0x7D, to); 
}

// JL rel8

u8*  JL8(u8 to) { 
	J8Rel(0x7C, to); 
}

// JLE rel8

u8*  JLE8(u8 to) { 
	J8Rel(0x7E, to); 
}

// JNE rel8

u8*  JNE8(u8 to) { 
	J8Rel(0x75, to); 
}

// JNZ rel8

u8*  JNZ8(u8 to) { 
	J8Rel(0x75, to); 
}

// JNG rel8

u8*  JNG8(u8 to) { 
	J8Rel(0x7E, to); 
}

// JNGE rel8

u8*  JNGE8(u8 to) { 
	J8Rel(0x7C, to); 
}

// JNL rel8

u8*  JNL8(u8 to) { 
	J8Rel(0x7D, to); 
}

// JNLE rel8

u8*  JNLE8(u8 to) { 
	J8Rel(0x7F, to); 
}

// JO rel8

u8*  JO8(u8 to) { 
	J8Rel(0x70, to); 
}

// JNO rel8

u8*  JNO8(u8 to) { 
	J8Rel(0x71, to); 
}

// JE rel32

u32* JE32(u32 to) {
	J32Rel(0x84, to);
}

// JZ rel32

u32* JZ32(u32 to) {
	J32Rel(0x84, to); 
}

// JG rel32

u32* JG32(u32 to) { 
	J32Rel(0x8F, to);
}

// JGE rel32

u32* JGE32(u32 to) { 
	J32Rel(0x8D, to); 
}

// JL rel32

u32* JL32(u32 to) { 
	J32Rel(0x8C, to); 
}

// JLE rel32

u32* JLE32(u32 to) { 
	J32Rel(0x8E, to); 
}

// JNE rel32

u32* JNE32(u32 to) { 
	J32Rel(0x85, to); 
}

// JNZ rel32

u32* JNZ32(u32 to) { 
	J32Rel(0x85, to); 
}

// JNG rel32

u32* JNG32(u32 to) { 
	J32Rel(0x8E, to); 
}

// JNGE rel32

u32* JNGE32(u32 to) { 
	J32Rel(0x8C, to); 
}

// JNL rel32

u32* JNL32(u32 to) { 
	J32Rel(0x8D, to); 
}

// JNLE rel32

u32*  JNLE32(u32 to) { 
	J32Rel(0x8F, to); 
}

// JO rel32

u32*  JO32(u32 to) { 
	J32Rel(0x80, to); 
}

// JNO rel32

u32*  JNO32(u32 to) { 
	J32Rel(0x81, to); 
}

// Call function

void CALLFunc(u32 func) {
	CALL32(func - ((u32)x86Ptr + 5));
}

// Call rel32

void CALL32(u32 to) {
	write8(0xE8); 
	write32(to); 
}

// Call r32

void CALL32R(int to) {
	write8(0xFF);
	ModRM(3, 2, to);
}

// Call m32

void CALL32M(u32 to) {
	write8(0xFF);
	ModRM(0, 2, DISP32);
	write32(to);
}

// Miscellaneous instructions

// CMP imm32 to r32

void CMP32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0x3D);
	} else {
		write8(0x81);
		ModRM(3, 7, to);
	}
	write32(from); 
}

// CMP imm32 to m32

void CMP32ItoM(u32 to, u32 from) {
	write8(0x81); 
	ModRM(0, 7, DISP32);
	write32(to); 
	write32(from); 
}

// CMP r32 to r32

void CMP32RtoR(int to, int from) {
	write8(0x39);
	ModRM(3, from, to);
}

// CMP m32 to r32

void CMP32MtoR(int to, u32 from) {
	write8(0x3B);
	ModRM(0, to, DISP32);
	write32(from); 
}

// TEST imm32 to r32

void TEST32ItoR(int to, u32 from) {
	if (to == EAX) {
		write8(0xA9);
	} else {
		write8(0xF7);
		ModRM(3, 0, to);
	}
	write32(from); 
}

// TEST r32 to r32

void TEST32RtoR(int to, int from) {
	write8(0x85);
	ModRM(3, from, to);
}

void BT32ItoR(int to,int from)
{
	write16(0xba0f);
	write8(0xe0 | to);
	write8(from);
}

// SETS r8

void SETS8R(int to) { 
        SET8R(0x98, to); 
}

// SETL r8

void SETL8R(int to) { 
	SET8R(0x9C, to); 
}

// SETB r8

void SETB8R(int to) { 
	SET8R(0x92, to); 
}

// SETNZ r8

void SETNZ8R(int to) { 
	SET8R(0x95,to); 
}

// CBW

void CBW() {
	write16(0x9866); 
}

// CWD

void CWD() {
	write8(0x98);
}

// CDQ

void CDQ() {
	write8(0x99); 
}

// PUSH r32

void PUSH32R(int from) {
	write8(0x50 | from); 
}

// PUSH m32

void PUSH32M(u32 from) {
	write8(0xFF);
	ModRM(0, 6, DISP32);
	write32(from); 
}

// PUSH imm32

void PUSH32I(u32 from) {
	write8(0x68); write32(from); 
}

// POP r32

void POP32R(int from) {
	write8(0x58 | from); 
}

// PUSHAD

void PUSHA32() {
	write8(0x60); 
}

// POPAD

void POPA32() {
	write8(0x61); 
}

// RET

void RET() {
	write8(0xC3); 
}

// FPU instructions

// Compare m32 to FPU registry stack

void FCOMP32(u32 from) {
	write8(0xD8);
	ModRM(0, 0x3, DISP32);
	write32(from); 
}

void FNSTSWtoAX() {
	write16(0xE0DF);
}

// FILD m32 to FPU registry stack

void FILD32(u32 from) {
	write8(0xDB);
	ModRM(0, 0x0, DISP32);
	write32(from); 
}

// FISTP m32 from FPU registry stack

void FISTP32(u32 from) {
	write8(0xDB);
	ModRM(0, 0x3, DISP32);
	write32(from); 
}

// FLD m32 to FPU registry stack

void FLD32(u32 from) {
	write8(0xD9);
	ModRM(0, 0x0, DISP32);
	write32(from); 
}

// FSTP m32 from FPU registry stack

void FSTP32(u32 to) {
	write8(0xD9);
	ModRM(0, 0x3, DISP32);
	write32(to); 
}

// FLDCW FPU control word from m16

void FLDCW(u32 from) {
	write8(0xD9);
	ModRM(0, 0x5, DISP32);
	write32(from); 
}

// FNSTCW FPU control word to m16

void FNSTCW(u32 to) {
	write8(0xD9);
	ModRM(0, 0x7, DISP32);
	write32(to); 
}

// FADD m32 to FPU registry stack

void FADD32(u32 from) {
	write8(0xD8);
	ModRM(0, 0x0, DISP32);
	write32(from); 
}

// FSUB m32 to FPU registry stack

void FSUB32(u32 from) {
	write8(0xD8);
	ModRM(0, 0x4, DISP32);
	write32(from); 
}

// FMUL m32 to FPU registry stack

void FMUL32(u32 from) {
	write8(0xD8);
	ModRM(0, 0x1, DISP32);
	write32(from); 
}

// FDIV m32 to FPU registry stack

void FDIV32(u32 from) {
	write8(0xD8);
	ModRM(0, 0x6, DISP32);
	write32(from); 
}

// FABS FPU registry stack

void FABS() {
	write16(0xE1D9);
}

// FSQRT FPU registry stack

void FSQRT() {
	write16(0xFAD9);
}

// FCHS FPU registry stack

void FCHS() {
	write16(0xE0D9);
}

// MMX instructions

// r64 = mm

// MOVQ m64 to r64

void MOVQMtoR(int to, u32 from) {
	write16(0x6F0F);
	ModRM(0, to, DISP32);
	write32(from); 
}

// MOVQ r64 to m64

void MOVQRtoM(u32 to, int from) {
	write16(0x7F0F);
	ModRM(0, from, DISP32);
	write32(to); 
}

// PAND r64 to r64

void PANDRtoR(int to, int from) {
	write16(0xDB0F);
	ModRM(3, to, from); 
}

// PAND r64 to r64

void PANDNRtoR(int to, int from) {
	write16(0xDF0F);
	ModRM(3, to, from); 
}

// POR r64 to r64

void PORRtoR(int to, int from) {
	write16(0xEB0F);
	ModRM(3, to, from); 
}

// PXOR r64 to r64

void PXORRtoR(int to, int from) {
	write16(0xEF0F);
	ModRM(3, to, from); 
}

// PSLLQ r64 to r64

void PSLLQRtoR(int to, int from) {
	write16(0xF30F);
	ModRM(3, to, from); 
}

// PSLLQ m64 to r64

void PSLLQMtoR(int to, u32 from) {
	write16(0xF30F); 
	ModRM(0, to, DISP32); 
	write32(from);
}

// PSLLQ imm8 to r64

void PSLLQItoR(int to, u8 from) {
	write16(0x730F); 
	ModRM(3, 6, to); 
	write8(from); 
}

// PSRLQ r64 to r64

void PSRLQRtoR(int to, int from) {
	write16(0xD30F); 
	ModRM(3, to, from); 
}

// PSRLQ m64 to r64

void PSRLQMtoR(int to, u32 from) {
	write16(0xD30F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// PSRLQ imm8 to r64

void PSRLQItoR(int to, u8 from) {
	write16(0x730F);
	ModRM(3, 2, to); 
	write8(from); 
}

// PADDUSB r64 to r64

void PADDUSBRtoR(int to, int from) {
	write16(0xDC0F); 
	ModRM(3, to, from); 
}

// PADDUSB m64 to r64

void PADDUSBMtoR(int to, u32 from) {
	write16(0xDC0F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// PADDUSW r64 to r64

void PADDUSWRtoR(int to, int from) {
	write16(0xDD0F); 
	ModRM(3, to, from); 
}

// PADDUSW m64 to r64

void PADDUSWMtoR(int to, u32 from) {
	write16(0xDD0F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// PADDB r64 to r64

void PADDBRtoR(int to, int from) {
	write16(0xFC0F); 
	ModRM(3, to, from); 
}

// PADDB m64 to r64

void PADDBMtoR(int to, u32 from) {
	write16(0xFC0F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// PADDW r64 to r64

void PADDWRtoR(int to, int from) {
	write16(0xFD0F); 
	ModRM(3, to, from); 
}

// PADDW m64 to r64

void PADDWMtoR(int to, u32 from) {
	write16(0xFD0F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// PADDD r64 to r64

void PADDDRtoR(int to, int from) {
	write16(0xFE0F); 
	ModRM(3, to, from); 
}

// PADDD m64 to r64

void PADDDMtoR(int to, u32 from) {
	write16(0xFE0F); 
	ModRM(0, to, DISP32); 
	write32(from); 
}

// EMMS

void EMMS() {
	
	// Use FEMMS if we have 3Dnow!
	write16(0x0e0f);
	return;
}

/* FEMMS */

void FEMMS() {
	write16(0x770F);
	return;
}

void PADDSBRtoR(int to, int from) {
	write16(0xEC0F); 
	ModRM(3, to, from); 
}

void PADDSWRtoR(int to, int from) {
	write16(0xED0F);
	ModRM(3, to, from); 
}

void PADDSDRtoR(int to, int from) {
	write16(0xEE0F); 
	ModRM(3, to, from); 
}

void PSUBSBRtoR(int to, int from) {
	write16(0xE80F); 
	ModRM(3, to, from); 
}

void PSUBSWRtoR(int to, int from) {
	write16(0xE90F);
	ModRM(3, to, from); 
}

void PSUBSDRtoR(int to, int from) {
	write16(0xEA0F); 
	ModRM(3, to, from); 
}

void PSUBBRtoR(int to, int from) {
	write16(0xF80F); 
	ModRM(3, to, from); 
}

void PSUBWRtoR(int to, int from) {
	write16(0xF90F); 
	ModRM(3, to, from); 
}

void PSUBDRtoR(int to, int from) {
	write16(0xFA0F); 
	ModRM(3, to, from); 
}

void MOVQ64ItoR(int reg,u64 i)
{
	MOVQMtoR(reg,(u32)(x86Ptr)+2+7);
	JMP8(8);
	write64(i);
}

void PSUBUSBRtoR(int to, int from) {
	write16(0xD80F); 
	ModRM(3, to, from); 
}

void PSUBUSWRtoR(int to, int from) {
	write16(0xD90F); 
	ModRM(3, to, from); 
}

void PMAXSWRtoR(int to,int from)
{
	write16(0xEE0F); 
	ModRM(3, to, from); 
}

void PMINSWRtoR(int to,int from)
{
	write16(0xEA0F); 
	ModRM(3, to, from); 
}

void PCMPEQBRtoR(int to,int from)
{
	write16(0x740F); 
	ModRM(3, to, from); 
}

void PCMPEQWRtoR(int to,int from)
{
	write16(0x750F); 
	ModRM(3, to, from); 
}

void PCMPEQDRtoR(int to,int from)
{
	write16(0x760F); 
	ModRM(3, to, from); 
}

void PCMPGTBRtoR(int to,int from)
{
	write16(0x640F); 
	ModRM(3, to, from); 
}

void PCMPGTWRtoR(int to,int from)
{
	write16(0x650F); 
	ModRM(3, to, from); 
}

void PCMPGTDRtoR(int to,int from)
{
	write16(0x660F); 
	ModRM(3, to, from); 
}

void PSRLWItoR(int to,int from)
{
	write16(0x710f);
	ModRM(2, 2 , to); 
	write8(from);
}
void PSRLDItoR(int to,int from)
{
	write16(0x720f);
	ModRM(2, 2 , to); 
	write8(from);
}

void PSLLWItoR(int to,int from)
{
	write16(0x710f);
	ModRM(3, 6 , to); 
	write8(from);
}

void PSLLDItoR(int to,int from)
{
	write16(0x720f);
	ModRM(3, 6 , to); 
	write8(from);
}

void PSRAWItoR(int to,int from)
{
	write16(0x710f);
	ModRM(3, 4 , to); 
	write8(from);
}

void PSRADItoR(int to,int from)
{
	write16(0x720f);
	ModRM(3, 4 , to); 
	write8(from);
}

// POR m64 to r64

void PORMtoR(int to, u32 from) {
	write16(0xEB0F);
	ModRM(0, to, DISP32); 
	write32(from);
}

// PXOR m64 to r64

void PXORMtoR(int to, u32 from) {
	write16(0xEF0F);
	ModRM(0, to, DISP32); 
	write32(from);
}

// PAND m64 to r64

void PANDMtoR(int to, u32 from) {
	write16(0xDB0F);
	ModRM(0, to, DISP32); 
	write32(from);
}

// PANDN m64 to r64

void PANDNMtoR(int to, u32 from) {
	write16(0xDF0F);
	ModRM(0, to, DISP32); 
	write32(from);
}

// MOVD m32 to r64

void MOVDMtoR(int to, u32 from) {
	write16(0x6E0F);
	ModRM(0, to, DISP32);
	write32(from); 
}

// MOVQ r64 to m32

void MOVDRtoM(u32 to, int from) {
	write16(0x7E0F);
	ModRM(0, from, DISP32);
	write32(to); 
}

// MOVD r32 to r64

void MOVD32RtoR(int to, int from) {
	write16(0x6E0F);
	ModRM(3, to,from);
}

// MOVQ r64 to r32

void MOVD64RtoR(int to, int from) {
	write16(0x7E0F);
	ModRM(3, from,to);
}

void MOVQRtoR(int to,int from)
{
	write16(0x6F0F);
	ModRM(3, to,from);
}

void PUNPCKHDQRtoR(int to,int from)
{
	write16(0x6A0F);
	ModRM(3, to,from);
}

void PUNPCKLDQRtoR(int to,int from)
{
	write16(0x620F);
	ModRM(3, to,from);
}

// SSE instructions

void MOVAPSMtoR(int to,int from)
{
	write16(0x280f);
	ModRM(0, to, DISP32);
	write32(from);
}

void MOVAPSRtoM(int to,int from)
{
	write16(0x2b0f);
	ModRM(0, from, DISP32);
	write32(to);
}

void MOVAPSRtoR(int to,int from)
{
	write16(0x290f);
	ModRM(3, to,from);
}

void ORPSMtoR(int to,int from)
{
	write16(0x560f);
	ModRM(0, to, DISP32);
	write32(from);
}

void ORPSRtoR(int to,int from)
{
	write16(0x560f);
	ModRM(3, to,from);
}

void XORPSMtoR(int to,int from)
{
	write16(0x570f);
	ModRM(0, to, DISP32);
	write32(from);
}

void XORPSRtoR(int to,int from)
{
	write16(0x570f);
	ModRM(3, to,from);
}

void ANDPSMtoR(int to,int from)
{
	write16(0x540f);
	ModRM(0, to, DISP32);
	write32(from);
}

void ANDPSRtoR(int to,int from)
{
	write16(0x540f);
	ModRM(3, to,from);
}

// 3Dnow! instructions

void PFCMPEQMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0xb0);
}

void PFCMPGTMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0xa0);
}

void PFCMPGEMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x90);
}

void PFADDMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x9e);
}

void PFADDRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from);
	write8(0x9e);
}

void PFSUBMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x9a);
}

void PFSUBRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from); 
	write8(0x9a);
}

void PFMULMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0xb4);
}

void PFMULRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0xb4);
}

void PFRCPMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x96);
}

void PFRCPRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0x96);
}

void PFRCPIT1RtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0xa6);
}

void PFRCPIT2RtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0xb6);
}

void PFRSQRTRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0x97);
}

void PFRSQIT1RtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to,from); 
	write8(0xa7);
}

void PF2IDMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x1d);
}

void PF2IDRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from); 
	write8(0x1d);
}

void PI2FDMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x0d);
}

void PI2FDRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from); 
	write8(0x0d);
}

// 3Dnow! extension instructions

void PFMAXMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0xa4);
}

void PFMAXRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from); 
	write8(0xa4);
}

void PFMINMtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(0, to, DISP32); 
	write32(from); 
	write8(0x94);
}

void PFMINRtoR(int to,int from)
{
	write16(0x0f0f);
	ModRM(3, to, from);
	write8(0x94);
}
