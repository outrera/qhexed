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


// If mnemonic begins with ampersand ('&'), its mnemonic decodes differently
// depending on operand size (16 or 32 bits).
// If mnemonic begins with dollar ('$'), this mnemonic depends on address size.
//

// '~' will be replaced with 'B', 'W', 'L', if required
// '#' will be replaced with 'E' for 32-bit instruction, or removed

static char *GetCmdName(X86_Instr *in, int c) {
	switch(in->cmd) {
	case X86_UNUSED: return "unused";

	// General Purpose Opcodes
	case X86_AAD: return "aad~";
	case X86_AAM: return "aam~";
	case X86_ADC: return "adc~";
	case X86_ADD: return "add~";
	case X86_AND: return "and~";
	case X86_ARPL: return "arpl";
	case X86_BOUND: return "bound";
	case X86_BSF: return "bsf";
	case X86_BSR: return "bsr";
	case X86_BSWAP: return "bswap";
	case X86_BT: return "bt";
	case X86_BTC: return "btc";
	case X86_BTR: return "btr";
	case X86_BTS: return "bts";
	case X86_CALL: return "call";
	case X86_CBW: return "cbw:cwde";
	case X86_CWD: return "cwd:cdq";
	case X86_CFLUSH: return "cflush";
	case X86_CLC: return "clc";
	case X86_CLD: return "cld";
	case X86_CLI: return "cli";
	case X86_CLTS: return "clts";
	case X86_CMC: return "cmc";
	case X86_CMOVL: return "cmovl";
	case X86_CMOVGE: return "cmovge";
	case X86_CMOVLE: return "cmovle";
	case X86_CMOVG: return "cmovg";
	case X86_CMOVO: return "cmovo";
	case X86_CMOVNO: return "cmovno";
	case X86_CMOVB: return "cmovb";
	case X86_CMOVAE: return "cmovae";
	case X86_CMOVS: return "cmovs";
	case X86_CMOVNS: return "cmovns";
	case X86_CMOVP: return "cmovp";
	case X86_CMOVNP: return "cmovnp";
	case X86_CMOVZ: return "cmovz";
	case X86_CMOVNZ: return "cmovnz";
	case X86_CMOVBE: return "cmovbe";
	case X86_CMOVA: return "cmova";
	case X86_CMP: return "cmp";
	case X86_CMPS: return "cmps";
	case X86_CMPXCH8B: return "cmpxch8b";
	case X86_CMPXCHG: return "cmpxchg";
	case X86_CPUID: return "cpuid";
	case X86_AAS: return "aas";
	case X86_AAA: return "aaa";
	case X86_DAA: return "daa";
	case X86_DAS: return "das";
	case X86_DEC: return "dec";
	case X86_DIV: return "div";
	case X86_ENTER: return "enter";
	case X86_HLT: return "hlt";
	case X86_IDIV: return "idiv~";
	case X86_IMUL: return "imul~";
	case X86_IN: return "in";
	case X86_INC: return "inc";
	case X86_INSB: return "insb";
	case X86_INSV: return "ins~";
	case X86_INT: return "int";
	case X86_INTO: return "into";
	case X86_INVD: return "invd";
	case X86_INVLPG: return "invlpg";
	case X86_IRET: return "iret";
	case X86_JCXZ: return "jcxz:jecxz";
	case X86_LAHF: return "lahf";
	case X86_LAR: return "lar";
	case X86_LDMXCSR: return "ldmxcsr";
	case X86_LDS: return "lds";
	case X86_LEA: return "lea";
	case X86_LEAVE: return "leave";
	case X86_LES: return "les";
	case X86_LFS: return "lfs";
	case X86_LGDT: return "lgdt";
	case X86_LGS: return "lgs";
	case X86_LIDT: return "lidt";
	case X86_LLDT: return "lldt";
	case X86_LMSW: return "lmsw";
	case X86_LODS: return "lods~";
	case X86_LODSB: return "lodsb";
	case X86_LOOP: return "loop$";
	case X86_LOOPNZ: return "loop$nz";
	case X86_LOOPZ: return "loop$z";
	case X86_LSL: return "lsl";
	case X86_LSS: return "lss";
	case X86_LTR: return "ltr";
	case X86_MOV: return "mov~";
	case X86_MOVNTI: return "movnti";
	case X86_MOVS: return "movs";
	case X86_MOVSB: return "movsb";
	case X86_MOVSX: return "movsx";
	case X86_MOVZX: return "movzx";
	case X86_MUL: return "mul~*";
	case X86_NEG: return "neg~";
	case X86_NOP: return "nop~";
	case X86_NOT: return "not~";
	case X86_OR: return "or~";
	case X86_OUT: return "out~";
	case X86_OUTS: return "outs~";
	case X86_POP: return "pop~";
	case X86_POPA: return "popa~";
	case X86_POPF: return "popf~";
	case X86_PUSH: return "push~";
	case X86_PUSHA: return "pusha~";
	case X86_PUSHF: return "pushf~";
	case X86_RCL: return "rcl";
	case X86_RCR: return "rcr";
	case X86_RDMSR: return "rdmsr";
	case X86_RDPMC: return "rdpmc";
	case X86_RDTSC: return "rdtsc";
	case X86_RET: return "ret";
	case X86_RETF: return "retf";
	case X86_ROL: return "rol";
	case X86_ROR: return "ror";
	case X86_RSM: return "rsm";
	case X86_SAHF: return "sahf";
	case X86_SAR: return "sar";
	case X86_SBB: return "sbb";
	case X86_SCAS: return "scas~";
	case X86_SCASB: return "scasb";

	case X86_SETO: return "seto";
	case X86_SETNO: return "setno";
	case X86_SETB: return "setb";
	case X86_SETAE: return "setae";
	case X86_SETZ: return "setz";
	case X86_SETNZ: return "setnz";
	case X86_SETBE: return "setbe";
	case X86_SETA: return "seta";
	case X86_SETS: return "sets";
	case X86_SETNS: return "setns";
	case X86_SETP: return "setp";
	case X86_SETNP: return "setnp";
	case X86_SETL: return "setl";
	case X86_SETGE: return "setge";
	case X86_SETLE: return "setle";
	case X86_SETG: return "setg";

	case X86_SGDT: return "sgdt";
	case X86_SHL: return "shl";
	case X86_SHLD: return "shld";
	case X86_SHR: return "shr";
	case X86_SHRD: return "shrd";
	case X86_SIDT: return "sidt";
	case X86_SLDT: return "sldt";
	case X86_SMSW: return "smsw";
	case X86_STC: return "stc";
	case X86_STD: return "std";
	case X86_STI: return "sti";
	case X86_STMXCSR: return "stmxcsr";
	case X86_STOS: return "stos~";
	case X86_STOSB: return "stosb";
	case X86_STR: return "str";
	case X86_SUB: return "sub";
	case X86_SYSENTER: return "sysenter";
	case X86_SYSEXIT: return "sysexit";
	case X86_TEST: return "test";
	case X86_UD2: return "ud2";
	case X86_VERR: return "verr";
	case X86_VERW: return "verw";
	case X86_WAIT: return "wait";
	case X86_WBINVD: return "wbinvd";
	case X86_WRMSR: return "wrmsr";
	case X86_XADD: return "xadd";
	case X86_XCHG: return "xchg";
	case X86_XLAT: return "xlat";
	case X86_XOR: return "xor";

	// goto
	case X86_JMP: return "jmp";

	// if(cond) goto...
	case X86_JL: return "jl";
	case X86_JGE: return "jge";
	case X86_JLE: return "jle";
	case X86_JG: return "jg";
	case X86_JO: return "jo";
	case X86_JNO: return "jno";
	case X86_JB: return "jb";
	case X86_JAE: return "jae";
	case X86_JS: return "js";
	case X86_JNS: return "jns";
	case X86_JP: return "jp";
	case X86_JNP: return "jnp";
	case X86_JZ: return "jz";
	case X86_JNZ: return "jnz";
	case X86_JBE: return "jbe";
	case X86_JA: return "ja";

	// FPU instructions
	case X86_F2XM1: return "f2xm1";
	case X86_FABS: return "fabs";
	case X86_FADD: return "fadd";
	case X86_FADDP: return "faddp";
	case X86_FBLD: return "fbld";
	case X86_FBSTP: return "fbstp";
	case X86_FCHS: return "fchs";
	case X86_FCLEX: return "fclex";
	case X86_FCMOVB: return "fcmovb";
	case X86_FCMOVBE: return "fcmovbe";
	case X86_FCMOVE: return "fcmove";
	case X86_FCMOVNB: return "fcmovnb";
	case X86_FCMOVNBE: return "fcmovnbe";
	case X86_FCMOVNE: return "fcmovne";
	case X86_FCMOVNU: return "fcmovnu";
	case X86_FCMOVU: return "fcmovu";
	case X86_FCOM: return "fcom";
	case X86_FCOMI: return "fcomi";
	case X86_FCOMIP: return "fcomip";
	case X86_FCOMP: return "fcomp";
	case X86_FCOMPP: return "fcompp";
	case X86_FCOS: return "fcos";
	case X86_FDECSTP: return "fdecstp";
	case X86_FDISI: return "fdisi";
	case X86_FDIV: return "fdiv";
	case X86_FDIVP: return "fdivp";
	case X86_FDIVRP: return "fdivrp";
	case X86_FENI: return "feni";
	case X86_FFREE: return "ffree";
	case X86_FIADD: return "fiadd";
	case X86_FICOM: return "ficom";
	case X86_FICOMP: return "ficomp";
	case X86_FIDIV: return "fidiv";
	case X86_FIDIVR: return "fidivr";
	case X86_FILD: return "fild";
	case X86_FIMUL: return "fimul";
	case X86_FINCSTP: return "fincstp";
	case X86_FINIT: return "finit";
	case X86_FIST: return "fist";
	case X86_FISTP: return "fistp";
	case X86_FISUB: return "fisub";
	case X86_FISUBR: return "fisubr";
	case X86_FLD: return "fld";
	case X86_FLD1: return "fld1";
	case X86_FLDCW: return "fldcw";
	case X86_FLDENV: return "fldenv";
	case X86_FLDL2E: return "fldl2e";
	case X86_FLDL2T: return "fldl2t";
	case X86_FLDLG2: return "fldlg2";
	case X86_FLDLN2: return "fldln2";
	case X86_FLDPI: return "fldpi";
	case X86_FLDZ: return "fldz";
	case X86_FMUL: return "fmul";
	case X86_FMULP: return "fmulp";
	case X86_FNOP: return "fnop";
	case X86_FPATAN: return "fpatan";
	case X86_FPREM: return "fprem";
	case X86_FPREM1: return "fprem1";
	case X86_FPTAN: return "fptan";
	case X86_FRNDINT: return "frndint";
	case X86_FRSTOR: return "frstor";
	case X86_FSAVE: return "fsave";
	case X86_FSCALE: return "fscale";
	case X86_FSIN: return "fsin";
	case X86_FSINCOS: return "fsincos";
	case X86_FSQRT: return "fsqrt";
	case X86_FST: return "fst";
	case X86_FSTCW: return "fstcw";
	case X86_FSTENV: return "fstenv";
	case X86_FSTP: return "fstp";
	case X86_FSTSW: return "fstsw";
	case X86_FSUB: return "fsub";
	case X86_FSUBP: return "fsubp";
	case X86_FSUBRP: return "fsubrp";
	case X86_FTST: return "ftst";
	case X86_FUCOM: return "fucom";
	case X86_FUCOMI: return "fucomi";
	case X86_FUCOMIP: return "fucomip";
	case X86_FUCOMP: return "fucomp";
	case X86_FUCOMPP: return "fucompp";
	case X86_FXAM: return "fxam";
	case X86_FXCH: return "fxch";
	case X86_FXRSTOR: return "fxrstor";
	case X86_FXSAVE: return "fxsave";
	case X86_FXTRACT: return "fxtract";
	case X86_FYL2X: return "fyl2x";
	case X86_FYL2XP1: return "fyl2xp1";

	// AMD 3DNow! (including Athlon/PIII extentions)
	case X86_PAVGUSB: return "pavgusb";
	case X86_PFADD: return "pfadd";
	case X86_PFSUB: return "pfsub";
	case X86_PFSUBR: return "pfsubr";
	case X86_PFACC: return "pfacc";
	case X86_PFCMPGE: return "pfcmpge";
	case X86_PFCMPGT: return "pfcmpgt";
	case X86_PFCMPEQ: return "pfcmpeq";
	case X86_PFMIN: return "pfmin";
	case X86_PFMAX: return "pfmax";
	case X86_PI2FD: return "pi2fd";
	case X86_PF2ID: return "pf2id";
	case X86_PFRCP: return "pfrcp";
	case X86_PFRSQRT: return "pfrsqrt";
	case X86_PFMUL: return "pfmul";
	case X86_PFRCPIT1: return "pfrcpit1";
	case X86_PFRSQIT1: return "pfrsqit1";
	case X86_PFRCPIT2: return "pfrcpit2";
	case X86_PMULHRW: return "pmulhrw";
	case X86_PF2IW: return "pf2iw";
	case X86_PFNACC: return "pfnacc";
	case X86_PFPNACC: return "pfpnacc";
	case X86_PI2FW: return "pi2fw";
	case X86_PSWAPD: return "pswapd";

 	// MMX
	case X86_EMMS: return "emms";
	case X86_MOVD: return "movd";
	case X86_MOVQ: return "movq";
	case X86_PACKSSDW: return "packssdw";
	case X86_PACKSSWB: return "packsswb";
	case X86_PACKUSWB: return "packuswb";
	case X86_PADDB: return "paddb";
	case X86_PADDD: return "paddd";
	case X86_PADDSB: return "paddsb";
	case X86_PADDSW: return "paddsw";
	case X86_PADDUSB: return "paddusb";
	case X86_PADDUSW: return "paddusw";
	case X86_PADDW: return "paddw";
	case X86_PAND: return "pand";
	case X86_PANDN: return "pandn";
	case X86_PCMPEQB: return "pcmpeqb";
	case X86_PCMPEQD: return "pcmpeqd";
	case X86_PCMPEQW: return "pcmpeqw";
	case X86_PCMPGTB: return "pcmpgtb";
	case X86_PCMPGTD: return "pcmpgtd";
	case X86_PCMPGTW: return "pcmpgtw";
	case X86_PMADDWD: return "pmaddwd";
	case X86_PMULHW: return "pmulhw";
	case X86_PMULLW: return "pmullw";
	case X86_POR: return "por";
	case X86_PSLLD: return "pslld";
	case X86_PSLLQ: return "psllq";
	case X86_PSLLW: return "psllw";
	case X86_PSRAD: return "psrad";
	case X86_PSRAW: return "psraw";
	case X86_PSRLD: return "psrld";
	case X86_PSRLQ: return "psrlq";
	case X86_PSRLW: return "psrlw";
	case X86_PSUBB: return "psubb";
	case X86_PSUBD: return "psubd";
	case X86_PSUBSB: return "psubsb";
	case X86_PSUBSW: return "psubsw";
	case X86_PSUBUSB: return "psubusb";
	case X86_PSUBUSW: return "psubusw";
	case X86_PSUBW: return "psubw";
	case X86_PUNPCKHBW: return "punpckhbw";
	case X86_PUNPCKHDQ: return "punpckhdq";
	case X86_PUNPCKHWD: return "punpckhwd";
	case X86_PUNPCKLBW: return "punpcklbw";
	case X86_PUNPCKLDQ: return "punpckldq";
	case X86_PUNPCKLWD: return "punpcklwd";
	case X86_PXOR: return "pxor";

	// AMD extentions to MMX command set (including Athlon/PIII extentions).
	case X86_FEMMS: return "femms";
	case X86_PREFETCH: return "prefetch";
	case X86_PREFETCHW: return "prefetchw";
	case X86_MASKMOVQ: return "maskmovq";
	case X86_MOVNTQ: return "movntq";
	case X86_PAVGB: return "pavgb";
	case X86_PAVGW: return "pavgw";
	case X86_PEXTRW: return "pextrw";
	case X86_PINSRW: return "pinsrw";
	case X86_PMAXSW: return "pmaxsw";
	case X86_PMAXUB: return "pmaxub";
	case X86_PMINSW: return "pminsw";
	case X86_PMINUB: return "pminub";
	case X86_PMOVMSKB: return "pmovmskb";
	case X86_PMULHUW: return "pmulhuw";
	case X86_PREFETCHNTA: return "prefetchnta";
	case X86_PREFETCHT0: return "prefetcht0";
	case X86_PREFETCHT1: return "prefetcht1";
	case X86_PREFETCHT2: return "prefetcht2";
	case X86_PSADBW: return "psadbw";
	case X86_PSHUFW: return "pshufw";
	case X86_SFENCE: return "sfence";

	// SSE
	case X86_LFENCE: return "lfence";
	case X86_MFENCE: return "mfence";

	// special
	case X86_SSE: return "sse";
	case X86_NH: return "nh";
	default: return "error";
	}
}
static char *GetRegName(X86_Reg r, X86_RefWhat w) {
	switch(r) {
	case X86_AH: return "ah";
	case X86_CH: return "ch";
	case X86_DH: return "dh";
	case X86_BH: return "bh";
	case X86_ES: return "es";
	case X86_CS: return "cs";
	case X86_SS: return "ss";
	case X86_DS: return "ds";
	case X86_FS: return "fs";
	case X86_GS: return "gs";
	case X86_CR0: return "cr0";
	case X86_CR1: return "cr1";
	case X86_CR2: return "cr2";
	case X86_CR3: return "cr3";
	case X86_CR4: return "cr4";
	case X86_CR5: return "cr5";
	case X86_CR6: return "cr6";
	case X86_CR7: return "cr7";
	case X86_TR0: return "tr0";
	case X86_TR1: return "tr1";
	case X86_TR2: return "tr2";
	case X86_TR3: return "tr3";
	case X86_TR4: return "tr4";
	case X86_TR5: return "tr5";
	case X86_TR6: return "tr6";
	case X86_TR7: return "tr7";
	case X86_DR0: return "dr0";
	case X86_DR1: return "dr1";
	case X86_DR2: return "dr2";
	case X86_DR3: return "dr3";
	case X86_DR4: return "dr4";
	case X86_DR5: return "dr5";
	case X86_DR6: return "dr6";
	case X86_DR7: return "dr7";
	case X86_MM0: return "mm0";
	case X86_MM1: return "mm1";
	case X86_MM2: return "mm2";
	case X86_MM3: return "mm3";
	case X86_MM4: return "mm4";
	case X86_MM5: return "mm5";
	case X86_MM6: return "mm6";
	case X86_MM7: return "mm7";
	case X86_XMM0: return "xmm0";
	case X86_XMM1: return "xmm1";
	case X86_XMM2: return "xmm2";
	case X86_XMM3: return "xmm3";
	case X86_XMM4: return "xmm4";
	case X86_XMM5: return "xmm5";
	case X86_XMM6: return "xmm6";
	case X86_XMM7: return "xmm7";
	default:
		if(w == X86_REF_BYTE) {
			switch(r) {
			case X86_EAX: return "al";
			case X86_ECX: return "cl";
			case X86_EDX: return "dl";
			case X86_EBX: return "bl";
			case X86_ESP: return "sl";
			case X86_EBP: return "bl";
			case X86_ESI: return "sl";
			case X86_EDI: return "dl";
			default: return "";
			}
		} else if(w == X86_REF_WORD) {
			switch(r) {
			case X86_FLAGS: return "flags";
			case X86_EAX: return "ax";
			case X86_ECX: return "cx";
			case X86_EDX: return "dx";
			case X86_EBX: return "bx";
			case X86_ESP: return "sp";
			case X86_EBP: return "bp";
			case X86_ESI: return "si";
			case X86_EDI: return "di";
			default: return "";
			}
		} else if(w == X86_REF_DWORD) {
			switch(r) {
			case X86_FLAGS: return "#flags";
			case X86_EAX: return "#ax";
			case X86_ECX: return "#cx";
			case X86_EDX: return "#dx";
			case X86_EBX: return "#bx";
			case X86_ESP: return "#sp";
			case X86_EBP: return "#bp";
			case X86_ESI: return "#si";
			case X86_EDI: return "#di";
			default: return "";
			}
		}
		break;
	}
	return "error";
}

char *DisVal(char *out, X86_Val v, X86_RefWhat what, bool sign, bool bits32) {
	const char *hexformat = bits32 ? "0x%08X" : "0x%04X";

	if(!sign) {
		if(v < 256) sprintf(out, "%d", v);
		else sprintf(out, hexformat, v);
	} else {
		X86_Val max_positive = (1 << (sizeof(X86_Val)*8 - 2))-1;
		if(v <= max_positive) sprintf(out, "%d", v);
		else sprintf(out, hexformat, v);
	}
	return out;
}

static char *DisSib(X86_Instr *i, char *out, X86_Sib *s) {
	char scale[5], so[5], index[5], base[5], disp[20];
	X86_RefWhat what = i->dec.arg32 ? X86_REF_DWORD: X86_REF_WORD;
	bool bits32 = i->dec.arg32;

	if(s->seg != X86_DS && s->base == X86_EBP)
		sprintf(so, "%%%s:", GetRegName((X86_Reg)s->seg, what));
	else *so = 0;

	if(s->disp) {
		DisVal(out, s->disp, what, 0, bits32);
		sprintf(disp, bits32 ? "0x%08X" : "0x%04X", s->disp);
	} else *disp = 0;

	if(s->base) sprintf(base, "%%%s", GetRegName((X86_Reg)s->base, what));
	else *base = 0;

	if(s->index) {
		sprintf(index, ",%%%s", GetRegName((X86_Reg)s->index, what));
		if(s->scale) sprintf(scale, ",%d", s->scale);
		else *scale = 0;
	} else {
		*index = 0;
		*scale = 0;
	}

	if(s->index || s->base)
		sprintf(out, "%s%s(%s%s%s)", so, disp, base, index, scale);
	else if(s->disp)
		sprintf(out, "%s%s", so, disp);
	else 
		sprintf(out, "0");
	return out;
}

static char *DisRef(X86_Instr *i, char *out, X86_Ref *r) {

	switch(r->how) {
	case X86_REF_REG:
		sprintf(out, "%%%s", GetRegName(r->reg, r->what));
		break;

	case X86_REF_SIB:
		DisSib(i, out, &r->sib);
		break;

	case X86_REF_IMM:
		if(r->imm < 256) sprintf(out, "$%d", r->imm);
		else sprintf(out, i->dec.arg32 ? "$0x%08X" : "$0x%04X", r->imm);
		break;

	case X86_REF_OFF:
		sprintf(out, "0x%08X", r->imm);
		break;

	case X86_REF_JMP:
		sprintf(out, "0x%08X", i->usr.ip + i->size + r->imm);
		break;
	case X86_REF_NNN:
		sprintf(out, "error");
		break;
	}
	return out;
}

void X86_Dis(char *out, X86_Instr *in) {
	int a;
	char *p, *q;
	char rep = ' ';
	X86_RefWhat nr;
	char buf[128];
	*out = 0;

	if(in->pre.repe) sprintf(out, "repe ");
	else if(in->pre.repne) sprintf(out, "repne ");

	if(in->pre.lock) sprintf(out, "%slock ", out);

	sprintf(out, "%s%s", out, GetCmdName(in, in->cmd));
	if(in->argc) sprintf(out, "%s ", out);

	if((in->cmd == X86_JMP || in->cmd == X86_CALL) && in->argv[0].what == X86_REF_FAR_PTR)
		sprintf(out, "%s*", out);

	for(a = in->argc-1; a >= 0; --a)
		sprintf(out, "%s%s%s", out, DisRef(in, buf, &in->argv[a]), a ? ", " : "");

	if(in->usr.mode&X86_MODE_IA32) nr = X86_REF_DWORD;
	else nr = X86_REF_WORD;

	if(in->argv[0].what == nr) rep = 0;
	else {
		if(in->argv[0].what == X86_REF_BYTE) rep = 'b';
		else if(in->argv[0].what == X86_REF_WORD) rep = 'w';
		else if(in->argv[0].what == X86_REF_DWORD) rep = 'l';
	}

	// do datatype expansion
	for(p = q = out; *p; p++) {
		if(*p == '~') {
			if(rep) *q++ = rep;
		} else if(*p == '#') {
			if(in->dec.arg32) *q++ = 'e';
		} else *q++ = *p;
	}
	*q = 0;
}

