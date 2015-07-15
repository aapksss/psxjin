#ifndef __IX86_H__
#define __IX86_H__

// Include basic types

#include "psxcommon.h"

// x86Flags defines
#define X86FLAG_FPU			0x00000001
#define X86FLAG_VME			0x00000002
#define X86FLAG_DEBUGEXT	0x00000004
#define X86FLAG_4MPAGE		0x00000008
#define X86FLAG_TSC			0x00000010
#define X86FLAG_MSR			0x00000020
#define X86FLAG_PAE			0x00000040
#define X86FLAG_MCHKXCP		0x00000080
#define X86FLAG_CMPXCHG8B	0x00000100
#define X86FLAG_APIC		0x00000200
#define X86FLAG_SYSENTER	0x00000800
#define X86FLAG_MTRR		0x00001000
#define X86FLAG_GPE			0x00002000
#define X86FLAG_MCHKARCH	0x00004000
#define X86FLAG_CMOV		0x00008000
#define X86FLAG_PAT			0x00010000
#define X86FLAG_PSE36		0x00020000
#define X86FLAG_PN			0x00040000
#define X86FLAG_MMX			0x00800000
#define X86FLAG_FXSAVE		0x01000000
#define X86FLAG_SSE			0x02000000

// x86EFlags defines

#define X86EFLAG_MMXEXT		0x00400000
#define X86EFLAG_3DNOWEXT	0x40000000
#define X86EFLAG_3DNOW		0x80000000

extern s8  x86ID[16];	// Vendor ID
extern u32 x86Family;	// Processor family
extern u32 x86Model;	// Processor model
extern u32 x86PType;	// Processor type
extern u32 x86StepID;	// Stepping ID
extern u32 x86Flags;	// Feature flags
extern u32 x86EFlags;	// Extended feature flags

// General defines

#define write8(val)  *(u8 *)x86Ptr = val; x86Ptr++;
#define write16(val) *(u16*)x86Ptr = val; x86Ptr+=2;
#define write32(val) *(u32*)x86Ptr = val; x86Ptr+=4;
#define write64(val) *(u64*)x86Ptr = val; x86Ptr+=8;

#define EAX 0
#define EBX 3
#define ECX 1
#define EDX 2
#define ESI 6
#define EDI 7
#define EBP 5
#define ESP 4

#define MM0 0
#define MM1 1
#define MM2 2
#define MM3 3
#define MM4 4
#define MM5 5
#define MM6 6
#define MM7 7

#define XMM0 0
#define XMM1 1
#define XMM2 2
#define XMM3 3
#define XMM4 4
#define XMM5 5
#define XMM6 6
#define XMM7 7


extern s8  *x86Ptr;
extern u8  *j8Ptr[32];
extern u32 *j32Ptr[32];

void x86Init();
void x86SetPtr(char *ptr);
void x86Shutdown();

void x86SetJ8(u8 *j8);
void x86SetJ32(u32 *j32);
void x86Align(int bytes);

// IX86 instructions

/*
 * scale values:
 *  0 - *1
 *  1 - *2
 *  2 - *4
 *  3 - *8
 */

// MOV instructions

// MOV r32 to r32

void MOV32RtoR(int to, int from);

// MOV r32 to m32

void MOV32RtoM(u32 to, int from);

// MOV m32 to r32

void MOV32MtoR(int to, u32 from);

// MOV [r32] to r32

void MOV32RmtoR(int to, int from);

// MOV [r32][r32*scale] to r32

void MOV32RmStoR(int to, int from, int from2, int scale);

// MOV r32 to [r32]

void MOV32RtoRm(int to, int from);

// MOV r32 to [r32][r32*scale]

void MOV32RtoRmS(int to, int to2, int scale, int from);

// MOV imm32 to r32

void MOV32ItoR(int to, u32 from);

// MOV imm32 to m32

void MOV32ItoM(u32 to, u32 from);

// MOV r16 to m16

void MOV16RtoM(u32 to, int from);

// MOV m16 to r16

void MOV16MtoR(int to, u32 from);

// MOV imm16 to m16

void MOV16ItoM(u32 to, u16 from);

// MOV r8 to m8

void MOV8RtoM(u32 to, int from);

// MOV m8 to r8

void MOV8MtoR(int to, u32 from);

// MOV imm8 to m8

void MOV8ItoM(u32 to, u8 from);

// MOVSX r8 to r32

void MOVSX32R8toR(int to, int from);

// MOVSX m8 to r32

void MOVSX32M8toR(int to, u32 from);

// MOVSX r16 to r32

void MOVSX32R16toR(int to, int from);

// MOVSX m16 to r32

void MOVSX32M16toR(int to, u32 from);

// MOVZX r8 to r32

void MOVZX32R8toR(int to, int from);

// MOVZX m8 to r32

void MOVZX32M8toR(int to, u32 from);

// MOVZX r16 to r32

void MOVZX32R16toR(int to, int from);

// MOVZX m16 to r32

void MOVZX32M16toR(int to, u32 from);

// CMOVNE r32 to r32

void CMOVNE32RtoR(int to, int from);

// CMOVNE m32 to r32

void CMOVNE32MtoR(int to, u32 from);

// CMOVE r32 to r32

void CMOVE32RtoR(int to, int from);

// CMOVE m32 to r32

void CMOVE32MtoR(int to, u32 from);

// CMOVG r32 to r32

void CMOVG32RtoR(int to, int from);

// CMOVG m32 to r32

void CMOVG32MtoR(int to, u32 from);

// CMOVGE r32 to r32

void CMOVGE32RtoR(int to, int from);

// CMOVGE m32 to r32

void CMOVGE32MtoR(int to, u32 from);

// CMOVL r32 to r32

void CMOVL32RtoR(int to, int from);

// CMOVL m32 to r32

void CMOVL32MtoR(int to, u32 from);

// CMOVLE r32 to r32

void CMOVLE32RtoR(int to, int from);

// CMOVLE m32 to r32

void CMOVLE32MtoR(int to, u32 from);

// Arithmetic instructions

// Add imm32 to r32

void ADD32ItoR(int to, u32 from);

// Add imm32 to m32

void ADD32ItoM(u32 to, u32 from);

// Add r32 to r32

void ADD32RtoR(int to, int from);

// Add r32 to m32

void ADD32RtoM(u32 to, int from);

// Add m32 to r32

void ADD32MtoR(int to, u32 from);

// ADC imm32 to r32

void ADC32ItoR(int to, u32 from);

// ADC r32 to r32

void ADC32RtoR(int to, int from);

// ADC m32 to r32

void ADC32MtoR(int to, u32 from);

// INC r32

void INC32R(int to);

// INC m32

void INC32M(u32 to);

// Sub imm32 to r32

void SUB32ItoR(int to, u32 from);

// Sub r32 to r32

void SUB32RtoR(int to, int from);

// Sub m32 to r32

void SUB32MtoR(int to, u32 from);

// SBB imm32 to r32

void SBB32ItoR(int to, u32 from);

// SBB r32 to r32

void SBB32RtoR(int to, int from);

// SBB m32 to r32

void SBB32MtoR(int to, u32 from);

// DEC r32

void DEC32R(int to);

// DEC m32

void DEC32M(u32 to);

// MUL EAX by r32 to EDX:EAX

void MUL32R(int from);

// MUL EAX by m32 to EDX:EAX

void MUL32M(u32 from);

// IMUL EAX by r32 to EDX:EAX

void IMUL32R(int from);

// IMUL EAX by m32 to EDX:EAX

void IMUL32M(u32 from);

// IMUL r32 by r32 to r32

void IMUL32RtoR(int to, int from);

// DIV EAX by r32 to EDX:EAX

void DIV32R(int from);

// DIV EAX by m32 to EDX:EAX

void DIV32M(u32 from);

// IDIV EAX by r32 to EDX:EAX

void IDIV32R(int from);

// IDIV EAX by m32 to EDX:EAX

void IDIV32M(u32 from);

// Shifting instructions

// SHL imm8 to r32

void SHL32ItoR(int to, u8 from);

// SHL cl to r32

void SHL32CLtoR(int to);

// SHR imm8 to r32

void SHR32ItoR(int to, u8 from);

// SHR cl to r32

void SHR32CLtoR(int to);

// SAR imm8 to r32

void SAR32ItoR(int to, u8 from);

// SAR cl to r32

void SAR32CLtoR(int to);

// SAL imm8 to r32

#define SAL32ItoR SHL32ItoR

// SAL cl to r32

#define SAL32CLtoR SHL32CLtoR

// Logical instructions

// OR imm32 to r32

void OR32ItoR(int to, u32 from);

// OR imm32 to m32

void OR32ItoM(u32 to, u32 from);

// OR r32 to r32

void OR32RtoR(int to, int from);

// OR r32 to m32

void OR32RtoM(u32 to, int from);

// OR m32 to r32

void OR32MtoR(int to, u32 from);

// XOR imm32 to r32

void XOR32ItoR(int to, u32 from);

// XOR imm32 to m32

void XOR32ItoM(u32 to, u32 from);

// XOR r32 to r32

void XOR32RtoR(int to, int from);

// XOR r32 to m32

void XOR32RtoM(u32 to, int from);

// XOR m32 to r32

void XOR32MtoR(int to, u32 from);

// AND imm32 to r32

void AND32ItoR(int to, u32 from);

// AND imm32 to m32

void AND32ItoM(u32 to, u32 from);

// AND r32 to r32

void AND32RtoR(int to, int from);

// AND r32 to m32

void AND32RtoM(u32 to, int from);

// AND m32 to r32

void AND32MtoR(int to, u32 from);

// NOT r32

void NOT32R(int from);

// NEG r32

void NEG32R(int from);

// Jump instructions

// JMP rel8

u8*  JMP8(u8 to);

// JMP rel32

u32* JMP32(u32 to);

// JMP r32

void JMP32R(int to);

// JE rel8

u8*  JE8(u8 to);

// JZ rel8

u8*  JZ8(u8 to);

// JG rel8

u8*  JG8(u8 to);

// JGE rel8

u8*  JGE8(u8 to);

// JL rel8

u8*  JL8(u8 to);

// JLE rel8

u8*  JLE8(u8 to);

// JNE rel8

u8*  JNE8(u8 to);

// JNZ rel8

u8*  JNZ8(u8 to);

// JNG rel8

u8*  JNG8(u8 to);

// JNGE rel8

u8*  JNGE8(u8 to);

// JNL rel8

u8*  JNL8(u8 to);

// JNLE rel8

u8*  JNLE8(u8 to);

// JO rel8

u8*  JO8(u8 to);

// JNO rel8

u8*  JNO8(u8 to);

// JE rel32

u32* JE32(u32 to);

// JZ rel32

u32* JZ32(u32 to);

// JG rel32

u32* JG32(u32 to);

// JGE rel32

u32* JGE32(u32 to);

// JL rel32

u32* JL32(u32 to);

// JLE rel32

u32* JLE32(u32 to);

// JNE rel32

u32* JNE32(u32 to);

// JNZ rel32

u32* JNZ32(u32 to);

// JNG rel32

u32* JNG32(u32 to);

// JNGE rel32

u32* JNGE32(u32 to);

// JNL rel32

u32* JNL32(u32 to);

// JNLE rel32

u32* JNLE32(u32 to);

// JO rel32

u32* JO32(u32 to);

// JNO rel32

u32* JNO32(u32 to);

// Call function

void CALLFunc(u32 func); // Based on CALL32

// Call rel32

void CALL32(u32 to);

// Call r32

void CALL32R(int to);

// Call m32

void CALL32M(u32 to);

// Miscellaneous instructions

// CMP imm32 to r32

void CMP32ItoR(int to, u32 from);

// CMP imm32 to m32

void CMP32ItoM(u32 to, u32 from);

// CMP r32 to r32

void CMP32RtoR(int to, int from);

// CMP m32 to r32

void CMP32MtoR(int to, u32 from);

// TEST imm32 to r32

void TEST32ItoR(int to, u32 from);

// TEST r32 to r32

void TEST32RtoR(int to, int from);

// SETS r8

void SETS8R(int to);

// SETL r8

void SETL8R(int to);

// SETB r8

void SETB8R(int to);

// CBW

void CBW();

// CWD

void CWD();

// CDQ

void CDQ();

// PUSH r32

void PUSH32R(int from);

// PUSH m32

void PUSH32M(u32 from);

// PUSH imm32

void PUSH32I(u32 from);

// POP r32

void POP32R(int from);

// PUSHAD

void PUSHA32();

// POPAD

void POPA32();

// RET

void RET();

// FPU instructions

// FILD m32 to FPU registry stack

void FILD32(u32 from);

// FISTP m32 from FPU registry stack

void FISTP32(u32 from);

// FLD m32 to FPU registry stack

void FLD32(u32 from);

// FSTP m32 from FPU registry stack

void FSTP32(u32 to);

// FLDCW FPU control word from m16

void FLDCW(u32 from);

// FSTCW FPU control word to m16

void FNSTCW(u32 to);

// FADD m32 to FPU registry stack

void FADD32(u32 from);

// FSUB m32 to FPU registry stack

void FSUB32(u32 from);

// FMUL m32 to FPU registry stack

void FMUL32(u32 from);

// FDIV m32 to FPU registry stack

void FDIV32(u32 from);

// FABS FPU registry stack

void FABS();

// FSQRT FPU registry stack

void FSQRT();

// FCHS FPU registry stack

void FCHS();

// MMX instructions

// r64 = mm

// MOVQ m64 to r64

void MOVQMtoR(int to, u32 from);

// MOVQ r64 to m64

void MOVQRtoM(u32 to, int from);

// PAND r64 to r64

void PANDRtoR(int to, int from);

// PAND m64 to r64

void PANDMtoR(int to, u32 from);

// PANDN r64 to r64

void PANDNRtoR(int to, int from);

// PANDN r64 to r64

void PANDNMtoR(int to, u32 from);

// POR r64 to r64

void PORRtoR(int to, int from);

// POR m64 to r64

void PORMtoR(int to, u32 from);

// PXOR r64 to r64

void PXORRtoR(int to, int from);

// PXOR m64 to r64

void PXORMtoR(int to, u32 from);

// PSLLQ r64 to r64

void PSLLQRtoR(int to, int from);

// PSLLQ m64 to r64

void PSLLQMtoR(int to, u32 from);

// PSLLQ imm8 to r64

void PSLLQItoR(int to, u8 from);

// PSRLQ r64 to r64

void PSRLQRtoR(int to, int from);

// PSRLQ m64 to r64

void PSRLQMtoR(int to, u32 from);

// PSRLQ imm8 to r64

void PSRLQItoR(int to, u8 from);

// PADDUSB r64 to r64

void PADDUSBRtoR(int to, int from);

// PADDUSB m64 to r64

void PADDUSBMtoR(int to, u32 from);

// PADDUSW r64 to r64

void PADDUSWRtoR(int to, int from);

// PADDUSW m64 to r64

void PADDUSWMtoR(int to, u32 from);

// PADDB r64 to r64

void PADDBRtoR(int to, int from);

// PADDB m64 to r64

void PADDBMtoR(int to, u32 from);

// PADDW r64 to r64

void PADDWRtoR(int to, int from);

// PADDW m64 to r64

void PADDWMtoR(int to, u32 from);

// PADDD r64 to r64

void PADDDRtoR(int to, int from);

// PADDD m64 to r64

void PADDDMtoR(int to, u32 from);

// EMMS

void EMMS();
void FEMMS();
void BT32ItoR(int to,int from);
void RCR32ItoR(int to,int from);

void PADDSBRtoR(int to, int from);
void PADDSWRtoR(int to, int from);
void PADDSDRtoR(int to, int from);
void PSUBSBRtoR(int to, int from); 
void PSUBSWRtoR(int to, int from);
void PSUBSDRtoR(int to, int from);

void PSUBBRtoR(int to, int from);
void PSUBWRtoR(int to, int from);
void PSUBDRtoR(int to, int from);

void MOVQ64ItoR(int reg,u64 i); // Prototype. Need to add all constants to end of block. Not after jr $+8 (should check this out)

void PMAXSWRtoR(int to,int from);
void PMINSWRtoR(int to,int from);

void PCMPEQBRtoR(int to,int from);
void PCMPEQWRtoR(int to,int from);
void PCMPEQDRtoR(int to,int from);

void PCMPGTBRtoR(int to,int from);
void PCMPGTWRtoR(int to,int from);
void PCMPGTDRtoR(int to,int from);

void PSRLWItoR(int to,int from);
void PSRLDItoR(int to,int from);
void PSLLWItoR(int to,int from);
void PSLLDItoR(int to,int from);
void PSRAWItoR(int to,int from);
void PSRADItoR(int to,int from);

void FCOMP32(u32 from);
void FNSTSWtoAX();
void SETNZ8R(int to);

void PFCMPEQMtoR(int to,int from);
void PFCMPGTMtoR(int to,int from);
void PFCMPGEMtoR(int to,int from);

void PFADDMtoR(int to,int from);
void PFADDRtoR(int to,int from);

void PFSUBMtoR(int to,int from);
void PFSUBRtoR(int to,int from);

void PFMULMtoR(int to,int from);
void PFMULRtoR(int to,int from);

void PFRCPMtoR(int to,int from);
void PFRCPRtoR(int to,int from);
void PFRCPIT1RtoR(int to,int from);
void PFRCPIT2RtoR(int to,int from);

void PFRSQRTRtoR(int to,int from);
void PFRSQIT1RtoR(int to,int from);

void PF2IDMtoR(int to,int from);
void PF2IDRtoR(int to,int from);
void PI2FDMtoR(int to,int from);
void PI2FDRtoR(int to,int from);

void PFMAXMtoR(int to,int from);
void PFMAXRtoR(int to,int from);
void PFMINMtoR(int to,int from);
void PFMINRtoR(int to,int from);

void MOVDMtoR(int to, u32 from);
void MOVDRtoM(u32 to, int from);
void MOVD32RtoR(int to, int from);
void MOVD64RtoR(int to, int from);

void MOVQRtoR(int to,int from);

// if to==from MMLO=MMHI

void PUNPCKHDQRtoR(int to,int from);

//if to==from MMHI=MMLO

void PUNPCKLDQRtoR(int to,int from);

/*
	SSE	instructions
*/

void MOVAPSMtoR(int to,int from);
void MOVAPSRtoM(int to,int from);
void MOVAPSRtoR(int to,int from);

void ORPSMtoR(int to,int from);
void ORPSRtoR(int to,int from);

void XORPSMtoR(int to,int from);
void XORPSRtoR(int to,int from);

void ANDPSMtoR(int to,int from);
void ANDPSRtoR(int to,int from);


#endif /* __IX86_H__ */
