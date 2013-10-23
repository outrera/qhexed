/*
 * QHexEd - Simple Qt Based Hex Editor
 *
 * Copyright (C) 2007 Nikita Sadkov
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This code is free software.  You may redistribute copies of it under the terms of
 * the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.
 * There is NO WARRANTY, to the extent permitted by law.
*/

#ifndef X86_INTERNAL_H
#define X86_INTERNAL_H

#include <HexPlugin.h>

// list of all recognized x86 commands
enum {
	X86_FIRST=-1, // start of mnemonizable tokens
	X86_UNUSED,

	// General Purpose Opcodes
	X86_AAD,
	X86_AAM,
	X86_ADC,
	X86_ADD,
	X86_AND,
	X86_ARPL,
	X86_BOUND,
	X86_BSF,
	X86_BSR,
	X86_BSWAP,
	X86_BT,
	X86_BTC,
	X86_BTR,
	X86_BTS,
	X86_CALL,
	X86_CBW,
	X86_CFLUSH,
	X86_CLC,
	X86_CLD,
	X86_CLI,
	X86_CLTS,
	X86_CMC,
	X86_CMOVL,
	X86_CMOVGE,
	X86_CMOVLE,
	X86_CMOVG,
	X86_CMOVO,
	X86_CMOVNO,
	X86_CMOVB,
	X86_CMOVAE,
	X86_CMOVS,
	X86_CMOVNS,
	X86_CMOVP,
	X86_CMOVNP,
	X86_CMOVZ,
	X86_CMOVNZ,
	X86_CMOVBE,
	X86_CMOVA,
	X86_CMP,
	X86_CMPS,
	X86_CMPXCH8B,
	X86_CMPXCHG,
	X86_CPUID,
	X86_CWD,
	X86_AAS,
	X86_AAA,
	X86_DAA,
	X86_DAS,
	X86_DEC,
	X86_DIV,
	X86_ENTER,
	X86_HLT,
	X86_IDIV,
	X86_IMUL,
	X86_IN,
	X86_INC,
	X86_INSB,
	X86_INSV,
	X86_INT,
	X86_INTO,
	X86_INVD,
	X86_INVLPG,
	X86_IRET,
	X86_JCXZ,
	X86_LAHF,
	X86_LAR,
	X86_LDMXCSR,
	X86_LDS,
	X86_LEA,
	X86_LEAVE,
	X86_LES,
	X86_LFS,
	X86_LGDT,
	X86_LGS,
	X86_LIDT,
	X86_LLDT,
	X86_LMSW,
	X86_LODS,
	X86_LODSB,
	X86_LOOP,
	X86_LOOPNZ,
	X86_LOOPZ,
	X86_LSL,
	X86_LSS,
	X86_LTR,
	X86_MOV,
	X86_MOVNTI,
	X86_MOVS,
	X86_MOVSB,
	X86_MOVSX,
	X86_MOVZX,
	X86_MUL,
	X86_NEG,
	X86_NOP,
	X86_NOT,
	X86_OR,
	X86_OUT,
	X86_OUTS,
	X86_POP,
	X86_POPA,
	X86_POPF,
	X86_PUSH,
	X86_PUSHA,
	X86_PUSHF,
	X86_RCL,
	X86_RCR,
	X86_RDMSR,
	X86_RDPMC,
	X86_RDTSC,
	X86_RET,
	X86_RETF,
	X86_ROL,
	X86_ROR,
	X86_RSM,
	X86_SAHF,
	X86_SAR,
	X86_SBB,
	X86_SCAS,
	X86_SCASB,
	X86_SETO,
	X86_SETNO,
	X86_SETB,
	X86_SETAE,
	X86_SETZ,
	X86_SETNZ,
	X86_SETBE,
	X86_SETA,
	X86_SETS,
	X86_SETNS,
	X86_SETP,
	X86_SETNP,
	X86_SETL,
	X86_SETGE,
	X86_SETLE,
	X86_SETG,
	X86_SGDT,
	X86_SHL,
	X86_SHLD,
	X86_SHR,
	X86_SHRD,
	X86_SIDT,
	X86_SLDT,
	X86_SMSW,
	X86_STC,
	X86_STD,
	X86_STI,
	X86_STMXCSR,
	X86_STOS,
	X86_STOSB,
	X86_STR,
	X86_SUB,
	X86_SYSENTER,
	X86_SYSEXIT,
	X86_TEST,
	X86_UD2,
	X86_VERR,
	X86_VERW,
	X86_WAIT,
	X86_WBINVD,
	X86_WRMSR,
	X86_XADD,
	X86_XCHG,
	X86_XLAT,
	X86_XOR,
	X86_JMP,
	X86_JL,
	X86_JGE,
	X86_JLE,
	X86_JG,
	X86_JO,
	X86_JNO,
	X86_JB,
	X86_JAE,
	X86_JS,
	X86_JNS,
	X86_JP,
	X86_JNP,
	X86_JZ,
	X86_JNZ,
	X86_JBE,
	X86_JA,

	// FPU instructions
	X86_F2XM1,
	X86_FABS,
	X86_FADD,
	X86_FADDP,
	X86_FBLD,
	X86_FBSTP,
	X86_FCHS,
	X86_FCLEX,
	X86_FCMOVB,
	X86_FCMOVBE,
	X86_FCMOVE,
	X86_FCMOVNB,
	X86_FCMOVNBE,
	X86_FCMOVNE,
	X86_FCMOVNU,
	X86_FCMOVU,
	X86_FCOM,
	X86_FCOMI,
	X86_FCOMIP,
	X86_FCOMP,
	X86_FCOMPP,
	X86_FCOS,
	X86_FDECSTP,
	X86_FDISI,
	X86_FDIV,
	X86_FDIVP,
	X86_FDIVRP,
	X86_FENI,
	X86_FFREE,
	X86_FIADD,
	X86_FICOM,
	X86_FICOMP,
	X86_FIDIV,
	X86_FIDIVR,
	X86_FILD,
	X86_FIMUL,
	X86_FINCSTP,
	X86_FINIT,
	X86_FIST,
	X86_FISTP,
	X86_FISUB,
	X86_FISUBR,
	X86_FLD,
	X86_FLD1,
	X86_FLDCW,
	X86_FLDENV,
	X86_FLDL2E,
	X86_FLDL2T,
	X86_FLDLG2,
	X86_FLDLN2,
	X86_FLDPI,
	X86_FLDZ,
	X86_FMUL,
	X86_FMULP,
	X86_FNOP,
	X86_FPATAN,
	X86_FPREM,
	X86_FPREM1,
	X86_FPTAN,
	X86_FRNDINT,
	X86_FRSTOR,
	X86_FSAVE,
	X86_FSCALE,
	X86_FSIN,
	X86_FSINCOS,
	X86_FSQRT,
	X86_FST,
	X86_FSTCW,
	X86_FSTENV,
	X86_FSTP,
	X86_FSTSW,
	X86_FSUB,
	X86_FSUBP,
	X86_FSUBRP,
	X86_FTST,
	X86_FUCOM,
	X86_FUCOMI,
	X86_FUCOMIP,
	X86_FUCOMP,
	X86_FUCOMPP,
	X86_FXAM,
	X86_FXCH,
	X86_FXRSTOR,
	X86_FXSAVE,
	X86_FXTRACT,
	X86_FYL2X,
	X86_FYL2XP1,

	// AMD 3DNow! (including Athlon/PIII extentions)
	X86_PAVGUSB,
	X86_PFADD,
	X86_PFSUB,
	X86_PFSUBR,
	X86_PFACC,
	X86_PFCMPGE,
	X86_PFCMPGT,
	X86_PFCMPEQ,
	X86_PFMIN,
	X86_PFMAX,
	X86_PI2FD,
	X86_PF2ID,
	X86_PFRCP,
	X86_PFRSQRT,
	X86_PFMUL,
	X86_PFRCPIT1,
	X86_PFRSQIT1,
	X86_PFRCPIT2,
	X86_PMULHRW,
	X86_PF2IW,
	X86_PFNACC,
	X86_PFPNACC,
	X86_PI2FW,
	X86_PSWAPD,

 	// MMX
	X86_EMMS,
	X86_MOVD,
	X86_MOVQ,
	X86_PACKSSDW,
	X86_PACKSSWB,
	X86_PACKUSWB,
	X86_PADDB,
	X86_PADDD,
	X86_PADDSB,
	X86_PADDSW,
	X86_PADDUSB,
	X86_PADDUSW,
	X86_PADDW,
	X86_PAND,
	X86_PANDN,
	X86_PCMPEQB,
	X86_PCMPEQD,
	X86_PCMPEQW,
	X86_PCMPGTB,
	X86_PCMPGTD,
	X86_PCMPGTW,
	X86_PMADDWD,
	X86_PMULHW,
	X86_PMULLW,
	X86_POR,
	X86_PSLLD,
	X86_PSLLQ,
	X86_PSLLW,
	X86_PSRAD,
	X86_PSRAW,
	X86_PSRLD,
	X86_PSRLQ,
	X86_PSRLW,
	X86_PSUBB,
	X86_PSUBD,
	X86_PSUBSB,
	X86_PSUBSW,
	X86_PSUBUSB,
	X86_PSUBUSW,
	X86_PSUBW,
	X86_PUNPCKHBW,
	X86_PUNPCKHDQ,
	X86_PUNPCKHWD,
	X86_PUNPCKLBW,
	X86_PUNPCKLDQ,
	X86_PUNPCKLWD,
	X86_PXOR,

	// AMD extentions to MMX command set (including Athlon/PIII extentions).
	X86_FEMMS,
	X86_PREFETCH,
	X86_PREFETCHW,
	X86_MASKMOVQ,
	X86_MOVNTQ,
	X86_PAVGB,
	X86_PAVGW,
	X86_PEXTRW,
	X86_PINSRW,
	X86_PMAXSW,
	X86_PMAXUB,
	X86_PMINSW,
	X86_PMINUB,
	X86_PMOVMSKB,
	X86_PMULHUW,
	X86_PREFETCHNTA,
	X86_PREFETCHT0,
	X86_PREFETCHT1,
	X86_PREFETCHT2,
	X86_PSADBW,
	X86_PSHUFW,
	X86_SFENCE,

	// SSE
	X86_LFENCE,
	X86_MFENCE,

	// special
	X86_SSE,
	X86_NH,

	X86_LAST	// end of mnemonizable tokens
};


// enumeration of all x86 registers
typedef enum {
	// NOTE: don't change order of elements within the line
	X86_AH, X86_CH, X86_DH, X86_BH,
	X86_ES, X86_CS, X86_SS, X86_DS, X86_FS, X86_GS, X86_FLAGS,

	X86_EAX, X86_ECX, X86_EDX, X86_EBX, X86_ESP, X86_EBP, X86_ESI, X86_EDI,
	X86_CR0, X86_CR1, X86_CR2, X86_CR3, X86_CR4, X86_CR5, X86_CR6, X86_CR7,
	X86_TR0, X86_TR1, X86_TR2, X86_TR3, X86_TR4, X86_TR5, X86_TR6, X86_TR7,
	X86_DR0, X86_DR1, X86_DR2, X86_DR3, X86_DR4, X86_DR5, X86_DR6, X86_DR7,
	X86_MM0, X86_MM1, X86_MM2, X86_MM3, X86_MM4, X86_MM5, X86_MM6, X86_MM7,
	X86_XMM0, X86_XMM1, X86_XMM2, X86_XMM3, X86_XMM4, X86_XMM5, X86_XMM6, X86_XMM7,
} X86_Reg;

// type of reference, used by X86_Ref
typedef enum {
	X86_REF_NNN=0,	// not used
	X86_REF_REG,	// register
	X86_REF_SIB,	// scale/index/base evaluates to address of operand
	X86_REF_IMM,	// operand is immediate
	X86_REF_OFF,	// operand is referenced unsigned DS and offset, that is immediate
	X86_REF_JMP,	// operand is signed immediate, that is added to IP+instruction_size
} X86_RefType;

// type of data, referenced by X86_Ref
typedef enum {
	X86_REF_DESC=1,		// 6-byte pseudo descriptor for GDT and IDR operations
	X86_REF_BOUND,		// two 2-byte or two 4-byte values for BOUND
	X86_REF_FAR_PTR,	// pointer for calls and segment load/store operations
	X86_REF_BYTE,		// 1-byte
	X86_REF_WORD,		// 2-byte
	X86_REF_DWORD,		// 4-byte
	X86_REF_QWORD,		// 8-byte
	X86_REF_QDWORD,		// 16-byte
} X86_RefWhat;

typedef u4 X86_Val;	// u4 is enough for now..
typedef s4 X86_SignedVal; // sometimes we will need its signed version


// keeps reference to memory from instruction [seg:base+index*scale+disp]
typedef struct {
	int seg;		// base segment
	int base;		// base register keeps start of data array
	int index;		// this register keeps index in data array
	int scale;		// scale for index (2, 4, 8, or 16) (array element size)
	X86_Val disp;	// signed displacement
} X86_Sib;
// reference for instruction argument
typedef struct {
	X86_RefType how;	// how to access 'what'
	X86_RefWhat what;	// what is referenced

	union {
		X86_Sib sib;	// values for SIB equation
		X86_Val imm;	// immediate value
		X86_Reg reg;	// register address
	};
} X86_Ref;

#define X86_MAXCMDSIZE		16		/// maximum size of x86 command

#define X86_MODE_IA16		0x1		// expect code to be 16-bit
#define X86_MODE_IA32		0x2		// expect code to be 32-bit

#define X86_DIS_NH			-1

// structure for keeping decoded instruction (we can produce mnemo code from this one)
typedef struct {
	int cmd;				// opcode token (pointer into mnemonic table)
	int argc;				// number of operands to this instruction
	X86_Ref argv[3];		// instruction operands
	char dump[X86_MAXCMDSIZE*2];
	char *error;			// error, if X86_Dis returned -1
	int size;				// instruction size

	// prefix bytes
	struct {
		int x0f;			// 0x0f prefix
		int seg;			// segment override prefix
		bool lock;			// has lock prefix
		bool repe;
		bool repne;
		bool opr;			// address override
		bool adr;			// address override
	} pre;

	// for decoder
	struct {
		u4 a1;
		u4 a2;
		u4 a3;
		u4 f;
		bool arg32;
		bool adr32;
		int mod;
		int reg;
		int rm;
		u1 *pc;
		u1 *pq;
		char *pd;
	} dec;

	// user specified parameters for disassembler/assembler
	struct {
		u4 ip;				// instruction pointer of this instruction
		int mode;			// disassembler mode
		int flags;
	} usr;
} X86_Instr;

// decodes instruction, and returns its size or one of X86_DIS_*
bool X86_Dec(X86_Instr *out, u1 *in);

// disassembles instruction to mnemonic
void X86_Dis(char *out, X86_Instr *in);

// assembles mnemonic to instruction
//bool X86_Asm(X86_Instr *out, char *in);

// encodes instruction and returns number of bytes written
//int X86_Enc(u1 *out, X86_Instr *out);

// returns mnemonic for command token
char *X86_TokenToMnemo(int cmd);


#endif

