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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "x86.h"

// intermediate tokens for prefixes, and escapes
enum {
	X86_ADROVR=X86_LAST,
	X86_OPROVR,
	X86_REPE,
	X86_REPNE,
	X86_SEGOVR,
	X86_0F,
	X86_LOCK,
	X86_GRP01,
	X86_GRP02,
	X86_GRP03,
	X86_GRP04,
	X86_GRP05,
	X86_GRP06,
	X86_GRP07,
	X86_GRP08,
	X86_GRP09,
	X86_GRP10,
	X86_GRP11,
	X86_GRP12,
	X86_GRP13,
	X86_GRP14,
	X86_GRP15,
	X86_GRP16,
	X86_FPU0,
	X86_FPU1,
	X86_FPU2,
	X86_FPU3,
	X86_FPU4,
	X86_FPU5,
	X86_FPU6,
	X86_FPU7,
	X86_MMX_UD,
	X86_MMX,
	X86_SEE,
	X86_SEE2,
	X86_LAST_CMD // useful in some cases
};


// we will use higher token bits to keep info, like affected flags
#define PREFIX_BITS 3
#define PREFIX_OFF (32-PREFIX_BITS)
#define PREFIX_MASK	(MASKN(PREFIX_BITS)<<PREFIX_OFF)

// format of opcode flags descriptor:
// ---------------------------
// ssss suuu uuuu uuuu ffff
// f - flags
// u - unused
// s - transfer size (BYTE/WORD/DWORD/VAR), for immediate
#define OT(a)		((a)<<(32-SIZE_BITS)) // create operand type everride
#define GETTYPE(a)	((a)>>(32-SIZE_BITS)) // get operand type everride

// opcode flags
#define IMM			0x01 // instruction has immediate value
#define MODRM		0x02 // instruction has modrm
#define SSE			0x04 // instruction requires SSE support
#define NDOS		0x08 // name depends on size
#define SPEC		0x10 // instructions behaves in non-standard way
#define EXT			0x20 // instruction is extended by modrm

// format of operand descriptor:
// ---------------------------
// ssss ssuu uuuu uumt tttt
// m - register or mode (REGOP)
// u - unused
// s - transfer size (BYTE/WORD/DWORD), if not set, default to current size
#define OPRTOK_BITS		8							// operand token bits

// A direct address, as immediate value (depends on address size mode)
// C modrm::reg selects control register
// D modrm::reg selects debug register
// E modrm::rm specifies operand - gpr or memory(sib)
// G modrm::reg selects gp register
// I operand is immediate, that follows opcode
// J operand is signed immediate offset, that is to be added to IP
// M modrm::rm may only refer to memory
// O absolute offset of operand follows instruction
// P modrm::reg selects MMX register
// Q modrm::rm specifies operand - MMX register or memory(sib)
// R modrm::rm may only refer to gpr
// S modrm::reg selects segment register
// T modrm::reg selects test register
// V modrm::reg selects XMM register
// W modrm::rm specifies operand - XMM register or memory(sib)
// X memory adressed by DS:SI pair
// Y memory adressed by ES:DI pair

// addressing methods
enum {
	eAX = 1, eCX, eDX, eBX, eSP, eBP, eSI, eDI,
	ES, CS, SS, DS, FS, GS,
	FLG,
	AH, CH, DH, BH, // these require special treatment
	MODRM_TYPES_START, // following ops use modrm bytes
	GGG, SSS, DDD, CCC, TTT, PPP, VVV, MMM, RRR, EEE, QQQ, WWW,
	MODRM_TYPES_END,
	AAA, III, JJJ, OOO, XXX, YYY,
	ONE, TWO, TRI
};
#define ISREG(a)		((a)<GGG) // register operand
#define ISMODRM(a)		((a)<AAA) // operand is determined by rm
#define ISMODRM_REG(a)	((a)<MMM) // operand is register adresed by reg field of modrm

// operand types
enum {
	b_=X86_REF_BYTE<<16,	// 1-byte, regardless of mode
	w_=X86_REF_WORD<<16,	// 2-byte, regardless of size
	d_=X86_REF_DWORD<<16,	// 4-byte, regardless of mode
	q_=X86_REF_QWORD<<16,	// 8-byte, regardless of mode
	dq=X86_REF_QDWORD<<16,	// 16-byte, regardless of mode
	si=X86_REF_DWORD<<16,	// 4-byte integer register

	a_=X86_REF_BOUND<<16,	// two 4-byte or two 2-byte operands (for BOUND), based on mode
	p_=X86_REF_FAR_PTR<<16,	// 4-byte or 6-byte pointer, depending on mode
	s_=X86_REF_DESC<<16,	// 6-byte pseudo descriptor (for GDT/IDT operations)

	c_=0x02000000,			// 1-byte or 2-byte, depending on mode
	pi=0x04000000,			// 8-byte MMX regiter
	ps=0x05000000,			// 16-byte, packed 4-byte floating point data
	ss=0x06000000,			// scalar element of 16-byte packed floating point data
	v_=0x07000000,			// 2-byte or 4-byte, depending on mode
};

char *reglut[] = {
	"",
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "es", "cs", "ss", "ds", "gs","fs","eflags",
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
	"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",
	"tr0", "tr1", "tr2", "tr3", "tr4", "tr5", "tr6", "tr7",
	"dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7",
	"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
	"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
};

// possible flags
#define IMM		0x01 // instruction has immediate value
#define MODRM	0x02 // instruction has modrm
#define SIB		0x04 // instruction has sib

typedef struct {
	int c;				// command token
	u4 a1;				// 1st operand descriptor
	u4 a2;				// 2nd operand descriptor
	u4 a3;				// 3rd operand descriptor
	u4 f;				// flags
} X86_Opcode; // opcode descriptor

#define NNN		0
#define nn		0

static X86_Opcode map[] = {
	// 00-0F
	{X86_ADD		, EEE|b_, GGG|b_, NNN|nn},
	{X86_ADD		, EEE|v_, GGG|v_, NNN|nn},
	{X86_ADD		, GGG|b_, EEE|b_, NNN|nn},
	{X86_ADD		, GGG|v_, EEE|v_, NNN|nn},
	{X86_ADD		, eAX|b_, III|b_, NNN|nn},
	{X86_ADD		, eAX|v_, III|v_, NNN|nn},
	{X86_PUSH		, ES |v_, NNN|nn, NNN|nn},
	{X86_POP		, ES |v_, NNN|nn, NNN|nn},

	{X86_OR			, EEE|b_, GGG|b_, NNN|nn},
	{X86_OR			, EEE|v_, GGG|v_, NNN|nn},
	{X86_OR			, GGG|b_, EEE|b_, NNN|nn},
	{X86_OR			, GGG|v_, EEE|v_, NNN|nn},
	{X86_OR			, eAX|b_, III|b_, NNN|nn},
	{X86_OR			, eAX|v_, III|v_, NNN|nn},
	{X86_PUSH		, CS |v_, NNN|nn, NNN|nn},
	{X86_0F			, ES |v_, NNN|nn, NNN|nn},

	// 10-1F
	{X86_ADC		, EEE|b_, GGG|b_, NNN|nn},
	{X86_ADC		, EEE|v_, GGG|v_, NNN|nn},
	{X86_ADC		, GGG|b_, EEE|b_, NNN|nn},
	{X86_ADC		, GGG|v_, EEE|v_, NNN|nn},
	{X86_ADC		, eAX|b_, III|b_, NNN|nn},
	{X86_ADC		, eAX|v_, III|v_, NNN|nn},
	{X86_PUSH		, SS |v_, NNN|nn, NNN|nn},
	{X86_POP		, SS |v_, NNN|nn, NNN|nn},

	{X86_SBB		, EEE|b_, GGG|b_, NNN|nn},
	{X86_SBB		, EEE|v_, GGG|v_, NNN|nn},
	{X86_SBB		, GGG|b_, EEE|b_, NNN|nn},
	{X86_SBB		, GGG|v_, EEE|v_, NNN|nn},
	{X86_SBB		, eAX|b_, III|b_, NNN|nn},
	{X86_SBB		, eAX|v_, III|v_, NNN|nn},
	{X86_PUSH		, DS |v_, NNN|nn, NNN|nn},
	{X86_POP		, DS |v_, NNN|nn, NNN|nn},


	// 20-2F
	{X86_AND		, EEE|b_, GGG|b_, NNN|nn},
	{X86_AND		, EEE|v_, GGG|v_, NNN|nn},
	{X86_AND		, GGG|b_, EEE|b_, NNN|nn},
	{X86_AND		, GGG|v_, EEE|v_, NNN|nn},
	{X86_AND		, eAX|b_, III|b_, NNN|nn},
	{X86_AND		, eAX|v_, III|v_, NNN|nn},
	{X86_SEGOVR		, ES |v_, NNN|nn, NNN|nn},
	{X86_DAA		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SUB		, EEE|b_, GGG|b_, NNN|nn},
	{X86_SUB		, EEE|v_, GGG|v_, NNN|nn},
	{X86_SUB		, GGG|b_, EEE|b_, NNN|nn},
	{X86_SUB		, GGG|v_, EEE|v_, NNN|nn},
	{X86_SUB		, eAX|b_, III|b_, NNN|nn},
	{X86_SUB		, eAX|v_, III|v_, NNN|nn},
	{X86_SEGOVR		, CS |nn, NNN|nn, NNN|nn},
	{X86_DAS		, NNN|nn, NNN|nn, NNN|nn},


	// 30-3F
	{X86_XOR		, EEE|b_, GGG|b_, NNN|nn},
	{X86_XOR		, EEE|v_, GGG|v_, NNN|nn},
	{X86_XOR		, GGG|b_, EEE|b_, NNN|nn},
	{X86_XOR		, GGG|v_, EEE|v_, NNN|nn},
	{X86_XOR		, eAX|b_, III|b_, NNN|nn},
	{X86_XOR		, eAX|v_, III|v_, NNN|nn},
	{X86_SEGOVR		, SS |v_, NNN|nn, NNN|nn},
	{X86_AAA		, NNN|nn, NNN|nn, NNN|nn},

	{X86_CMP		, EEE|b_, GGG|b_, NNN|nn},
	{X86_CMP		, EEE|v_, GGG|v_, NNN|nn},
	{X86_CMP		, GGG|b_, EEE|b_, NNN|nn},
	{X86_CMP		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMP		, eAX|b_, III|b_, NNN|nn},
	{X86_CMP		, eAX|v_, III|v_, NNN|nn},
	{X86_SEGOVR		, DS |v_, NNN|nn, NNN|nn},
	{X86_AAS		, NNN|nn, NNN|nn, NNN|nn},


	// 40-4f
	{X86_INC		, eAX|v_, NNN|nn, NNN|nn},
	{X86_INC		, eCX|v_, NNN|nn, NNN|nn},
	{X86_INC		, eDX|v_, NNN|nn, NNN|nn},
	{X86_INC		, eBX|v_, NNN|nn, NNN|nn},
	{X86_INC		, eSP|v_, NNN|nn, NNN|nn},
	{X86_INC		, eBP|v_, NNN|nn, NNN|nn},
	{X86_INC		, eSI|v_, NNN|nn, NNN|nn},
	{X86_INC		, eDI|v_, NNN|nn, NNN|nn},

	{X86_DEC		, eAX|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eCX|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eDX|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eBX|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eSP|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eBP|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eSI|v_, NNN|nn, NNN|nn},
	{X86_DEC		, eDI|v_, NNN|nn, NNN|nn},

	// 50-5f
	{X86_PUSH		, eAX|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eCX|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eDX|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eBX|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eSP|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eBP|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eSI|v_, NNN|nn, NNN|nn},
	{X86_PUSH		, eDI|v_, NNN|nn, NNN|nn},

	{X86_POP		, eAX|v_, NNN|nn, NNN|nn},
	{X86_POP		, eCX|v_, NNN|nn, NNN|nn},
	{X86_POP		, eDX|v_, NNN|nn, NNN|nn},
	{X86_POP		, eBX|v_, NNN|nn, NNN|nn},
	{X86_POP		, eSP|v_, NNN|nn, NNN|nn},
	{X86_POP		, eBP|v_, NNN|nn, NNN|nn},
	{X86_POP		, eSI|v_, NNN|nn, NNN|nn},
	{X86_POP		, eDI|v_, NNN|nn, NNN|nn},

	// 60-6F
	{X86_PUSHA		, NNN|nn, NNN|nn, NNN|nn},
	{X86_POPA		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BOUND		, GGG|v_, MMM|a_, NNN|nn},
	{X86_ARPL		, EEE|w_, GGG|w_, NNN|nn},
	{X86_SEGOVR		, FS |v_, NNN|nn, NNN|nn},
	{X86_SEGOVR		, GS |v_, NNN|nn, NNN|nn},
	{X86_OPROVR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_ADROVR		, NNN|nn, NNN|nn, NNN|nn},

	{X86_PUSH		, III|v_, NNN|nn, NNN|nn},
	{X86_IMUL		, GGG|v_, EEE|v_, III|v_},
	{X86_PUSH		, III|b_, NNN|nn, NNN|nn},
	{X86_IMUL		, GGG|v_, EEE|v_, III|b_},
	{X86_INSB		, YYY|b_, eDX|b_, NNN|nn},
	{X86_INSV		, YYY|v_, eDX|v_, NNN|nn},
	{X86_OUTS		, eDX|b_, XXX|b_, NNN|nn},
	{X86_OUTS		, eDX|v_, XXX|v_, NNN|nn},

	// 70-7F
	{X86_JO			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JNO		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JB			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JAE		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JZ			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JNZ		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JBE		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JA			, JJJ|b_, NNN|nn, NNN|nn},

	{X86_JS			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JNS		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JP			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JNP		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JL			, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JGE		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JLE		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JG			, JJJ|b_, NNN|nn, NNN|nn},

	// 80-8F
	{X86_GRP01		, EEE|b_, III|b_, NNN|nn},
	{X86_GRP01		, EEE|v_, III|v_, NNN|nn},
	{X86_GRP01		, EEE|b_, III|b_, NNN|nn},
	{X86_GRP01		, EEE|v_, III|b_, NNN|nn},
	{X86_TEST		, EEE|b_, GGG|b_, NNN|nn},
	{X86_TEST		, EEE|v_, GGG|v_, NNN|nn},
	{X86_XCHG		, EEE|b_, GGG|b_, NNN|nn},
	{X86_XCHG		, EEE|v_, GGG|v_, NNN|nn},

	{X86_MOV		, EEE|b_, GGG|b_, NNN|nn},
	{X86_MOV		, EEE|v_, GGG|v_, NNN|nn},
	{X86_MOV		, GGG|b_, EEE|b_, NNN|nn},
	{X86_MOV		, GGG|v_, EEE|v_, NNN|nn},
	{X86_MOV		, EEE|w_, SSS|w_, NNN|nn},
	{X86_LEA		, GGG|v_, MMM|nn, NNN|nn},
	{X86_MOV		, SSS|w_, EEE|w_, NNN|nn},
	{X86_POP		, EEE|v_, NNN|nn, NNN|nn},

	// 90-9F
	{X86_NOP		, NNN|nn, NNN|nn, NNN|nn},
	{X86_XCHG		, eAX|v_, eCX|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eDX|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eBX|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eSP|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eBP|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eSI|v_, NNN|nn},
	{X86_XCHG		, eAX|v_, eDI|v_, NNN|nn},

	{X86_CBW		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CWD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CALL		, AAA|p_, NNN|nn, NNN|nn}, // far call
	{X86_WAIT		, NNN|nn, NNN|nn, NNN|nn},
	{X86_PUSHF		, FLG|v_, NNN|nn, NNN|nn},
	{X86_POPF		, FLG|v_, NNN|nn, NNN|nn},
	{X86_SAHF		, NNN|nn, NNN|nn, NNN|nn},
	{X86_LAHF		, NNN|nn, NNN|nn, NNN|nn},

	// A0-AF
	{X86_MOV		, eAX|b_, OOO|b_, NNN|nn},
	{X86_MOV		, eAX|v_, OOO|v_, NNN|nn},
	{X86_MOV		, OOO|b_, eAX|b_, NNN|nn},
	{X86_MOV		, OOO|v_, eAX|v_, NNN|nn},
	{X86_MOVS		, YYY|b_, XXX|b_, NNN|nn},
	{X86_MOVS		, YYY|v_, XXX|v_, NNN|nn},
	{X86_CMPS		, YYY|b_, XXX|b_, NNN|nn},
	{X86_CMPS		, XXX|v_, YYY|v_, NNN|nn},

	{X86_TEST		, eAX|b_, III|b_, NNN|nn},
	{X86_TEST		, eAX|v_, III|v_, NNN|nn},
	{X86_STOS		, YYY|b_, eAX|b_, NNN|nn},
	{X86_STOS		, YYY|v_, eAX|v_, NNN|nn},
	{X86_LODS		, eAX|b_, XXX|b_, NNN|nn},
	{X86_LODS		, eAX|v_, XXX|v_, NNN|nn},
	{X86_SCAS		, eAX|b_, YYY|b_, NNN|nn},
	{X86_SCAS		, eAX|v_, YYY|v_, NNN|nn},

	// B0-BF
	{X86_MOV		, eAX|b_, III|b_, NNN|nn},
	{X86_MOV		, eAX|b_, III|b_, NNN|nn},
	{X86_MOV		, eAX|b_, III|b_, NNN|nn},
	{X86_MOV		, eAX|b_, III|b_, NNN|nn},
	{X86_MOV		, AH |b_, III|b_, NNN|nn},
	{X86_MOV		, CH |b_, III|b_, NNN|nn},
	{X86_MOV		, DH |b_, III|b_, NNN|nn},
	{X86_MOV		, BH |b_, III|b_, NNN|nn},

	{X86_MOV		, eAX|v_, III|v_, NNN|nn},
	{X86_MOV		, eCX|v_, III|v_, NNN|nn},
	{X86_MOV		, eDX|v_, III|v_, NNN|nn},
	{X86_MOV		, eBX|v_, III|v_, NNN|nn},
	{X86_MOV		, eSP|v_, III|v_, NNN|nn},
	{X86_MOV		, eBP|v_, III|v_, NNN|nn},
	{X86_MOV		, eSI|v_, III|v_, NNN|nn},
	{X86_MOV		, eDI|v_, III|v_, NNN|nn},

	// C0-CF
	{X86_GRP02		, EEE|b_, III|b_, NNN|nn},
	{X86_GRP02		, EEE|v_, III|b_, NNN|nn},
	{X86_RET		, III|w_, NNN|nn, NNN|nn},
	{X86_RET		, NNN|nn, NNN|nn, NNN|nn},
	{X86_LES		, GGG|v_, MMM|p_, NNN|nn},
	{X86_LDS		, GGG|v_, MMM|p_, NNN|nn},
	{X86_GRP11		, EEE|b_, III|b_, NNN|nn},
	{X86_GRP11		, EEE|v_, III|v_, NNN|nn},

	{X86_ENTER		, III|w_, III|b_, NNN|nn},
	{X86_LEAVE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RETF		, III|w_, NNN|nn, NNN|nn},
	{X86_RETF		, NNN|nn, NNN|nn, NNN|nn},
	{X86_INT		, TRI|nn, NNN|nn, NNN|nn},
	{X86_INT		, III|b_, NNN|nn, NNN|nn},
	{X86_INTO		, NNN|nn, NNN|nn, NNN|nn},
	{X86_IRET		, NNN|nn, NNN|nn, NNN|nn},

	// D0-DF
	{X86_GRP02		, EEE|b_, ONE|b_, NNN|nn},
	{X86_GRP02		, EEE|v_, ONE|b_, NNN|nn},
	{X86_GRP02		, EEE|b_, eCX |b_, NNN|nn},
	{X86_GRP02		, EEE|v_, eCX |b_, NNN|nn},
	{X86_AAM		, III|b_, NNN|nn, NNN|nn},
	{X86_AAD		, III|b_, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_XLAT		, NNN|nn, NNN|nn, NNN|nn},

	{X86_FPU0		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU1		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU2		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU3		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU4		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU5		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU6		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FPU7		, NNN|nn, NNN|nn, NNN|nn},

	// E0-EF
	{X86_LOOPNZ		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_LOOPZ		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_LOOP		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_JCXZ		, JJJ|b_, NNN|nn, NNN|nn},
	{X86_IN			, eAX|b_, III|b_, NNN|nn},
	{X86_IN			, eAX|v_, III|b_, NNN|nn},
	{X86_OUT		, III|b_, eAX|b_, NNN|nn},
	{X86_OUT		, III|b_, eAX|v_, NNN|nn},

	{X86_CALL		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JMP		, JJJ|v_, NNN|nn, NNN|nn},	// near
	{X86_JMP		, AAA|p_, NNN|nn, NNN|nn},	// far
	{X86_JMP		, JJJ|b_, NNN|nn, NNN|nn},	// shor
	{X86_IN			, eAX|b_, eDX|w_, NNN|nn},
	{X86_IN			, eAX|v_, eDX|w_, NNN|nn},
	{X86_OUT		, eDX|w_, eAX|b_, NNN|nn},
	{X86_OUT		, eDX|w_, eAX|v_, NNN|nn},

	// F0-F7
	{X86_LOCK		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_REPNE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_REPE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_HLT		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CMC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP03		, EEE|b_, NNN|nn, NNN|nn},
	{X86_GRP03		, EEE|v_, NNN|nn, NNN|nn},

	{X86_CLC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_STC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CLI		, NNN|nn, NNN|nn, NNN|nn},
	{X86_STI		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CLD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_STD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP04		, NNN|nn, NNN|nn, NNN|nn}, // inc/dec
	{X86_GRP05		, NNN|nn, NNN|nn, NNN|nn}, // inc/dec
};

/**********************************************************
 *********************************************************/

// Secondary map for when first opcode is 0F
// NOTE: SSE instructions use prefixes and mod/rm to extend itselves
static X86_Opcode map0f[] = {
	// 00-0F
	{X86_GRP06		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP07		, NNN|nn, NNN|nn, NNN|nn},
	{X86_LAR		, GGG|v_, EEE|w_, NNN|nn},
	{X86_LSL		, GGG|v_, EEE|w_, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CLTS		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	{X86_INVD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_WBINVD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UD2		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// 10-1F
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_GRP16		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// 20-2F
	{X86_MOV		, RRR|d_, CCC|d_, NNN|nn},
	{X86_MOV		, RRR|d_, DDD|d_, NNN|nn},
	{X86_MOV		, CCC|d_, RRR|d_, NNN|nn},
	{X86_MOV		, DDD|d_, RRR|d_, NNN|nn},
	{X86_MOV		, RRR|d_, TTT|d_, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MOV		, TTT|d_, RRR|d_, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// 30-3F
	{X86_WRMSR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RDTSC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RDMSR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RDPMC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SYSENTER	, NNN|nn, NNN|nn, NNN|nn},
	{X86_SYSEXIT	, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MOVNTI		, GGG|v_, EEE|v_, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// 40-4F
	{X86_CMOVO		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVNO		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVB		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVAE		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVZ		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVNZ		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVBE		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVA		, GGG|v_, EEE|v_, NNN|nn},

	{X86_CMOVS		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVNS		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVP		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVNP		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVL		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVGE		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVLE		, GGG|v_, EEE|v_, NNN|nn},
	{X86_CMOVG		, GGG|v_, EEE|v_, NNN|nn},

	// 50-5F
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// 60-6F
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// 70-7F
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP12		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP13		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP14		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MMX_UD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// 80-8F
	{X86_JO			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JNO		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JB			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JAE		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JZ			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JNZ		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JBE		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JA			, JJJ|v_, NNN|nn, NNN|nn},

	{X86_JS			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JNS		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JP			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JNP		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JL			, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JGE		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JLE		, JJJ|v_, NNN|nn, NNN|nn},
	{X86_JG			, JJJ|v_, NNN|nn, NNN|nn},

	// 90-9F
	{X86_SETO		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETNO		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETB		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETAE		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETZ		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETNZ		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETBE		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETA		, EEE|b_, NNN|nn, NNN|nn},

	{X86_SETS		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETNS		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETP		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETNP		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETL		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETGE		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETLE		, EEE|b_, NNN|nn, NNN|nn},
	{X86_SETG		, EEE|b_, NNN|nn, NNN|nn},

	// A0-AF
	{X86_PUSH		, FS |v_, NNN|nn, NNN|nn},
	{X86_POP		, FS |v_, NNN|nn, NNN|nn},
	{X86_CPUID		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BT			, EEE|v_, GGG|v_, NNN|nn},
	{X86_SHLD		, EEE|v_, GGG|v_, III|b_},
	{X86_SHLD		, EEE|v_, GGG|v_, eCX|b_},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	{X86_PUSH		, GS |v_, NNN|nn, NNN|nn},
	{X86_POP		, GS |v_, NNN|nn, NNN|nn},
	{X86_RSM		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BTS		, EEE|v_, GGG|v_, NNN|nn},
	{X86_SHRD		, EEE|v_, GGG|v_, III|b_},
	{X86_SHRD		, EEE|v_, GGG|v_, eCX|b_},
	{X86_GRP15		, NNN|nn, NNN|nn, NNN|nn},
	{X86_IMUL		, GGG|v_, EEE|v_, NNN|nn},
	// SHLD(a,b,c) == a<<c|(b>>(NBITSIN(b)-c))

	// B0-BF
	{X86_CMPXCHG	, EEE|b_, GGG|b_, NNN|nn},
	{X86_CMPXCHG	, EEE|v_, GGG|v_, NNN|nn},
	{X86_LSS		, MMM|p_, NNN|nn, NNN|nn},
	{X86_BTR		, EEE|v_, GGG|v_, NNN|nn},
	{X86_LFS		, MMM|p_, NNN|nn, NNN|nn},
	{X86_LGS		, MMM|p_, NNN|nn, NNN|nn},
	{X86_MOVZX		, GGG|v_, EEE|b_, NNN|nn}, // zero eXtended
	{X86_MOVZX		, GGG|v_, EEE|w_, NNN|nn},

	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP10		, NNN|nn, NNN|nn, NNN|nn}, // leads to nowhere
	{X86_GRP08		, EEE|v_, III|b_, NNN|nn},
	{X86_BTC		, EEE|v_, GGG|v_, NNN|nn},
	{X86_BSF		, GGG|v_, EEE|v_, NNN|nn},
	{X86_BSR		, GGG|v_, EEE|v_, NNN|nn},
	{X86_MOVSX		, GGG|v_, EEE|b_, NNN|nn}, // sign eXtended
	{X86_MOVSX		, GGG|v_, EEE|w_, NNN|nn},

	// C0-CF
	{X86_XADD		, EEE|b_, GGG|b_, NNN|nn},
	{X86_XADD		, EEE|v_, GGG|v_, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MOVNTI		, EEE|d_, GGG|d_, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_GRP09		, NNN|nn, NNN|nn, NNN|nn},

	{X86_BSWAP		, eAX|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eCX|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eDX|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eBX|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eSP|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eBP|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eSI|v_, NNN|nn, NNN|nn},
	{X86_BSWAP		, eDI|v_, NNN|nn, NNN|nn},

	// D0-DF
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// E0-EF
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	// F0-FF
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
};

// opcode map for instructions extended by modrm; note
static X86_Opcode mapext[] = {
	// Group 1
	{X86_ADD		, NNN|nn, NNN|nn, NNN|nn},
	{X86_OR			, NNN|nn, NNN|nn, NNN|nn},
	{X86_ADC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SBB		, NNN|nn, NNN|nn, NNN|nn},
	{X86_AND		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SUB		, NNN|nn, NNN|nn, NNN|nn},
	{X86_XOR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CMP		, NNN|nn, NNN|nn, NNN|nn},

	// Group 2
	{X86_ROL		, NNN|nn, NNN|nn, NNN|nn},
	{X86_ROR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RCL		, NNN|nn, NNN|nn, NNN|nn},
	{X86_RCR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SHL		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SHR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SAR		, NNN|nn, NNN|nn, NNN|nn},

	// Group 3
	{X86_TEST		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_NOT		, NNN|nn, NNN|nn, NNN|nn},
	{X86_NEG		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MUL		, eAX|nn, NNN|nn, NNN|nn},
	{X86_IMUL		, eAX|nn, NNN|nn, NNN|nn},
	{X86_DIV		, eAX|nn, NNN|nn, NNN|nn},
	{X86_IDIV		, eAX|nn, NNN|nn, NNN|nn},

	// Group 4
	{X86_INC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_DEC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 5
	{X86_INC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_DEC		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CALL		, EEE|v_, NNN|nn, NNN|nn},
	{X86_CALL		, EEE|p_, NNN|nn, NNN|nn},
	{X86_JMP		, EEE|v_, NNN|nn, NNN|nn},
	{X86_JMP		, EEE|p_, NNN|nn, NNN|nn},
	{X86_PUSH		, EEE|v_, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 6
	{X86_SLDT		, EEE|w_, NNN|nn, NNN|nn},
	{X86_STR		, EEE|v_, NNN|nn, NNN|nn},
	{X86_LLDT		, EEE|w_, NNN|nn, NNN|nn},
	{X86_LTR		, EEE|w_, NNN|nn, NNN|nn},
	{X86_VERR		, EEE|w_, NNN|nn, NNN|nn},
	{X86_VERW		, EEE|w_, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 7
	{X86_SGDT		, MMM|s_, NNN|nn, NNN|nn},	// store GDRR
	{X86_SIDT		, MMM|s_, NNN|nn, NNN|nn},	// store IDTR
	{X86_LGDT		, MMM|s_, NNN|nn, NNN|nn},
	{X86_LIDT		, MMM|s_, NNN|nn, NNN|nn},
	{X86_SMSW		, EEE|w_, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_LMSW		, EEE|w_, NNN|nn, NNN|nn},
	{X86_INVLPG		, MMM|b_, NNN|nn, NNN|nn},

	// Group 8
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BT			, NNN|nn, NNN|nn, NNN|nn},
	{X86_BTS		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BTR		, NNN|nn, NNN|nn, NNN|nn},
	{X86_BTC		, NNN|nn, NNN|nn, NNN|nn},

	// Group 9
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CMPXCH8B	, MMM|q_, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 10
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 11
	{X86_MOV		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 12
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 13
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 14
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 15
	{X86_FXSAVE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_FXRSTOR	, NNN|nn, NNN|nn, NNN|nn},
	{X86_LDMXCSR	, NNN|nn, NNN|nn, NNN|nn},
	{X86_STMXCSR	, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_CFLUSH		, NNN|nn, NNN|nn, NNN|nn},

	// Group 16
	{X86_PREFETCHNTA, NNN|nn, NNN|nn, NNN|nn},
	{X86_PREFETCHT0	, NNN|nn, NNN|nn, NNN|nn},
	{X86_PREFETCHT1	, NNN|nn, NNN|nn, NNN|nn},
	{X86_PREFETCHT2	, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
};

static X86_Opcode mapext2nd[] = {
	// Group 9
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 10
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 11
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 12
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 13
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},

	// Group 14
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SSE		, NNN|nn, NNN|nn, NNN|nn},

	// Group 15
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_LFENCE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_MFENCE		, NNN|nn, NNN|nn, NNN|nn},
	{X86_SFENCE		, NNN|nn, NNN|nn, NNN|nn},

	// Group 16
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
	{X86_UNUSED		, NNN|nn, NNN|nn, NNN|nn},
};

#define ARGTYPE(a) ((a)&0x0000ffff)
#define ARGSIZE(a) ((a)&0xffff0000)
#define IMMSZ(a) ((ARGSIZE(a) == b_) ? 1 : ((ARGSIZE(a) == w_) ? 2 : ((ARGSIZE(a) == d_) ? 4 : 0)))

#define ISIMM(a) (ARGTYPE(a)==III || ARGTYPE(a)==JJJ)
#define HASMODRM(a) (MODRM_TYPES_START < ARGTYPE(a) && ARGTYPE(a) < MODRM_TYPES_END)

#define MOD(a) (((a)&0xc0)>>6)
#define REG(a) (((a)&0x38)>>3)
#define RM(a) ((a)&0x07)

#define SCALE(a) (((a)&0xc0)>>6)
#define INDEX(a) (((a)&0x38)>>3)
#define BASE(a) ((a)&0x07)

#define d (i->dec)

static void SetupOpcode(X86_Opcode *o) {
	if(HASMODRM(o->a1) || HASMODRM(o->a2) || HASMODRM(o->a3) ||
			(X86_GRP01 <= o->c && o->c <= X86_GRP16) || o->c == X86_SSE)
		o->f |= MODRM;
}

static void SetupTables() {
	int i;
	for(i = 0; i < 0x100; i++) {
		SetupOpcode(map+i);
		SetupOpcode(map0f+i);
		if(i < 0x80) SetupOpcode(mapext+i);
		if(i < 0x40) SetupOpcode(mapext2nd+i);
	}
	//fprintf(stderr, "%d\n", sizeof(map)/sizeof(map[0]));
}

static int BinToHex(int b) {
	return b < 10 ? b+'0' : b-10+'A';
}

static u4 UpdateOpcode(u4 a, u4 b) {
	u4 r = a;
	if(b&0xffff) r = (r&0xffff0000) | (b&0xffff);
	if(b&0xffff0000) r |= (r&0xffff) | (b&0xffff0000);
	return r;
}

static void DecodeSib(X86_Instr *i, X86_Sib *o) {
	o->seg = DS; // DS is default segment (if SP isn't used for index)
	o->scale = 0;
	o->index = 0;
	o->base = 0;
	o->disp = 0;

	if(d.arg32) { // 32-bit case
		if(d.rm == 4) { // has sib byte
			u1 sib = *d.pc;
			if(d.mod == 0 && BASE(sib) == 5) { // ds:flat_offset_32bit
				o->disp = (X86_SignedVal)(s4)LSB32(d.pc);
				d.pc += 4;
			}
			if(INDEX(sib) != 4) { // has scaled index
				o->index = INDEX(sib);
				o->scale = 2<<SCALE(sib);
			}
			o->base = BASE(sib)+X86_EAX;
		} else if(d.mod || d.rm != 5)
			o->base = d.reg+X86_EAX;
	

		if(d.mod == 1) {
			o->disp = (X86_SignedVal)(s1)*d.pc;
			d.pc += 1;
		} else if(d.mod == 2) {
			o->disp = (X86_SignedVal)(s4)LSB32(d.pc);
			d.pc += 4;
		} else if(d.mod == 0 && d.rm == 5) { // ds:flat_offset_32bit
			o->disp = (X86_SignedVal)(s4)LSB32(d.pc);
			d.pc += 4;
		}
	} else { // 16-bit case
		// 16-bit code uses somewhat different addressing scheme
		o->scale = 0;
		if(d.mod == 0 && d.rm == 6) { // ds:flat_offset_16bit
			o->disp = (X86_SignedVal)(s2)LSB16(d.pc);
			d.pc += 2;
		} else {
			if(d.mod == 1) {
				o->disp = (X86_SignedVal)(s1)*d.pc;
				d.pc += 1;
			} else if(d.mod == 2) {
				o->disp = (X86_SignedVal)(s2)LSB16(d.pc);
				d.pc += 2;
			}
			switch(d.rm) {
			case 0: // [BX+SI]
				o->base = X86_EBX;
				o->index = X86_ESI;
				break;
			case 1: // [BX+SI]
				o->base = X86_EBX;
				o->index = X86_EDI;
				break;
			case 2: // [BP+SI]
				o->base = X86_EBP;
				o->index = X86_ESI;
				break;
			case 3: // [BP+DI]
				o->base = X86_EBP;
				o->index = X86_EDI;
				break;
			case 4: // [SI]
				o->base = X86_ESI;
				break;
			case 5: // [DI]
				o->base = X86_EDI;
				break;
			case 6: // [SS:BP]
				o->base = X86_EBP;
				o->seg = X86_SS; // BP defaults to SS
				break;
			case 7: // [BX]
				o->base = X86_EBX;
				break;
			default:
				break;
			}
			if(i->pre.seg) o->seg = i->pre.seg;
		}
	}
}

static void DecodeImm(X86_Instr *i, X86_Ref *r) {
	switch(r->what) {
	case X86_REF_BYTE:
		r->imm = *d.pc++;
		break;

	case X86_REF_WORD:
word:
		r->imm = LSB16(d.pc);
		d.pc += 2;
		break;

	case X86_REF_DWORD:
dword:
		r->imm = LSB32(d.pc);
		d.pc += 4;
		break;

	case X86_REF_QWORD:
		if(d.arg32) goto dword;
		goto word;
		break;

	default:
		fprintf(stderr, "DecodeImm(): invalid size");
		abort();
	}
}

static X86_RefWhat GetRefWhat(X86_Instr *i, u4 x) {
	X86_RefWhat r;

	if((x&0xffff0000) == v_) {
		if(d.arg32) r = X86_REF_DWORD;
		else r = X86_REF_WORD;
	} else r = (X86_RefWhat)(x>>16);
	return r;
}

// decodes operand
static void DecodeArg(X86_Instr *i, X86_Ref *out, u4 a) {
	u4 t = ARGTYPE(a);
	out->what = GetRefWhat(i, a);

	*d.pd++ = ' '; // separate args from command and each other
	
	switch(t) {
	case eAX: case eCX: case eDX: case eBX: case eSP: case eBP: case eSI: case eDI:
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(t-eAX+X86_EAX);
		break;

	case ES: case CS: case SS: case DS: case GS: case FS: case FLG:
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(t-ES+X86_ES);
		break;

	case AH: case CH: case DH: case BH:
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(t-AH+X86_AH);
		break;

	case GGG: // modrm.reg selects gpr
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_EAX);
		break;

	case RRR: // modrm.rm selects GPR
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.rm+X86_EAX);
		break;
		
	case SSS: // modrm.reg selects segment register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_ES);
		break;
		
	case DDD: // modrm.reg selects debug register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_DR0);
		break;

	case CCC: // modrm.reg selects control register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_CR0);
		break;

	case PPP: // modrm.reg selects MMX register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_MM0);
		break;

	case TTT: // modrm.reg selects test register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_TR0);
		break;

	case VVV: // modrm.reg selects XMM register
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.reg+X86_XMM0);
		break;

	case MMM: // mod/rm refers to memory
ismem:
		out->how = X86_REF_SIB;
		DecodeSib(i, &out->sib);
		break;

	case EEE:
		if(3 != d.mod) goto ismem;
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.rm+X86_EAX);
		break;

	case QQQ:
		if(3 != d.mod) goto ismem;
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.rm+X86_MM0);
		break;

	case WWW:
		if(3 != d.mod) goto ismem;
		out->how = X86_REF_REG;
		out->reg = (X86_Reg)(d.rm+X86_XMM0);
		break;

	case III:
		out->how = X86_REF_IMM;
		DecodeImm(i, out);
		break;

	case JJJ:
		out->how = X86_REF_JMP;
		DecodeImm(i, out);
		// do sign extension for future use
		if(out->what == X86_REF_BYTE)
			out->imm = (X86_SignedVal)(s1)out->imm;
		else if(out->what == X86_REF_WORD)
			out->imm = (X86_SignedVal)(s2)out->imm;
		break;

	case OOO: // offset
		out->how = X86_REF_OFF;
		if(d.adr32) {
			out->imm = LSB32(d.pc);
			d.pc += 4;
		} else {
			out->imm = LSB16(d.pc);
			d.pc += 2;
		}
		break;

	case AAA: // direct address or ds:offset
		out->how = X86_REF_IMM;
		out->what = X86_REF_DWORD;
		out->imm = LSB16(d.pc)*16;
		d.pc += 2;

		if(d.arg32) {
			out->imm += LSB32(d.pc);
			d.pc += 4;
		} else {
			out->imm += LSB16(d.pc);
			d.pc += 2;
		}

		break;

	case ONE: case TWO: case TRI:
		out->how = X86_REF_IMM;
		out->what = X86_REF_BYTE;
		out->imm = t - ONE + 1;
		break;

	case XXX: case YYY: // string instruction
		break;

	default:
		fprintf(stderr, "DecodeArg(): anvalid arg type");
		abort();
	}

	// dump argument
	for(; d.pq < d.pc; d.pq++) {
		*d.pd++ = BinToHex(*d.pq>>4);
		*d.pd++ = BinToHex(*d.pq&0xf);
	}
}


static void DecodeCommand(X86_Instr *i) {
	X86_Opcode *o;

	// collect prefixes
	for(o = &map[*d.pc]; ; o = &map[*d.pc++]) {
		switch(o->c) {
		case X86_0F:
			o = &map0f[*d.pc++];
			goto break_loop; // after 0x0f ther can't be more prefixes

		case X86_SEGOVR:
			i->pre.seg = o->a1;
			break;

		case X86_OPROVR:
			i->pre.opr = true;
			d.arg32 = !d.arg32;
			break;

		case X86_ADROVR:
			i->pre.adr = true;
			d.adr32 = !d.adr32;
			break;

		case X86_REPE:
			i->pre.repe = true;

		case X86_REPNE:
			i->pre.repne = true;
			break;

		default:
			goto break_loop;
		}
		;
		*d.pd++ = BinToHex(*d.pc >> 4);
		*d.pd++ = BinToHex(*d.pc & 0xf);
	}
break_loop:

	i->cmd = o->c;
	d.a1 = o->a1;
	d.a2 = o->a2;
	d.a3 = o->a3;
	d.f = o->f;

	d.pc++;

	// check, if instruction has modrm byte
	if(d.f & MODRM) {
		d.mod = MOD(*d.pc);
		d.reg = REG(*d.pc);
		d.rm = RM(*d.pc);
		d.pc++;

		// check, if instruction is extended
		switch(i->cmd) {
		case X86_FPU0: case X86_FPU1: case X86_FPU2: case X86_FPU3:
		case X86_FPU4: case X86_FPU5: case X86_FPU6: case X86_FPU7:
			break;

		case X86_SSE:
			break;

		case X86_GRP09: case X86_GRP10:                 case X86_GRP12:
		case X86_GRP13: case X86_GRP14: case X86_GRP15: case X86_GRP16:
			if(d.mod == 3) { // mod == 11B
				o = &mapext2nd[((i->cmd - X86_GRP09)<<3) | d.reg];
				i->cmd = o->c;
				d.a1 = UpdateOpcode(d.a1, o->a1);
				d.a2 = UpdateOpcode(d.a2, o->a2);
				d.a3 = UpdateOpcode(d.a3, o->a3);
				break;
			}
		// fallthrough..

		case X86_GRP01: case X86_GRP02: case X86_GRP03: case X86_GRP04:
		case X86_GRP05: case X86_GRP06: case X86_GRP07: case X86_GRP08:
		case X86_GRP11:
			o = &mapext[((i->cmd - X86_GRP01)<<3) | d.reg];

			i->cmd = o->c;
			d.a1 = UpdateOpcode(d.a1, o->a1);
			d.a2 = UpdateOpcode(d.a2, o->a2);
			d.a3 = UpdateOpcode(d.a3, o->a3);
			break;
		default:
			break;
		}
	}
}

// decodes one x86 instruction into 'ins' and returns its size
bool X86_Dec(X86_Instr *i, u1 *in) {
	static bool first_time = 1;
	if(first_time) {
		SetupTables();
		first_time = 0;
	}

	// do init
	memset(&i->pre, 0, sizeof(i->pre));
	d.arg32 = i->usr.mode&X86_MODE_IA32;
	d.adr32 = i->usr.mode&X86_MODE_IA32;
	d.pq = d.pc = in;
	d.pd = i->dump;

	DecodeCommand(i);

	if(i->cmd == X86_NH) return false;
	if(i->cmd == X86_SSE) return false;

	// dump command to hex
	for( ; d.pq < d.pc; d.pq++) {
		*d.pd++ = BinToHex(*d.pq>>4);
		*d.pd = 0;
		*d.pd++ = BinToHex(*d.pq&0xf);
	}

	// decode arguments
	if(d.a1) {
		DecodeArg(i, &i->argv[0], d.a1);
		if(d.a2) {
			DecodeArg(i, &i->argv[1], d.a2);
			if(d.a3) {
				DecodeArg(i, &i->argv[2], d.a3);
				i->argc = 3;
			} else {
				i->argc = 2;
				// NOTE: x86 does sign extension of immediates, if they loaded to larger register
				// and that is funny, cuz I cant find info about this in intel's manuals
				if(i->argv[0].what != i->argv[1].what &&
						i->argv[1].how == X86_REF_IMM) {
					if(i->argv[0].what == X86_REF_DWORD) {
						if(i->argv[1].what == X86_REF_BYTE) {
							i->argv[1].imm = (X86_SignedVal)(s1)i->argv[1].imm;
							i->argv[1].what = X86_REF_DWORD;
						} else if(i->argv[1].what == X86_REF_WORD) {
							i->argv[1].imm = (X86_SignedVal)(s2)i->argv[1].imm;
							i->argv[1].what = X86_REF_DWORD;
						}
					} else if(i->argv[0].what == X86_REF_WORD) {
						if(i->argv[1].what == X86_REF_BYTE) {
							i->argv[1].imm = (X86_SignedVal)(s1)i->argv[1].imm;
							i->argv[1].what = X86_REF_WORD;
						}
					}
				}
			}
		} else i->argc = 1;
	} else i->argc = 0;

	*d.pd++ = 0;
	i->size = d.pc - in;

	return true;
}

