/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef __R3000A_H__
#define __R3000A_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "psxcommon.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "psxbios.h"

#ifdef __GNUC__
#	define likely(x)       __builtin_expect(!!(x),1)
#	define unlikely(x)     __builtin_expect(!!(x),0)
#else
#	define likely(x)       (x)
#	define unlikely(x)     (x)
#endif

enum R3000Aexception {
	R3000E_Int = 0,      // Interrupt
	R3000E_AdEL = 4,     // Address error (on load/I-fetch)
	R3000E_AdES = 5,     // Address error (on store)
	R3000E_IBE = 6,      // Bus error (instruction fetch)
	R3000E_DBE = 7,      // Bus error (data load/store)
	R3000E_Syscall = 8,  // syscall instruction
	R3000E_Bp = 9,       // Breakpoint - a break instruction
	R3000E_RI = 10,      // reserved instruction
	R3000E_CpU = 11,     // Co-Processor unusable
	R3000E_Ov = 12       // arithmetic overflow
};

enum R3000Anote {
	R3000ACPU_NOTIFY_CACHE_ISOLATED = 0,
	R3000ACPU_NOTIFY_CACHE_UNISOLATED = 1,
	R3000ACPU_NOTIFY_BEFORE_SAVE,  // data arg - hle if non-null
	R3000ACPU_NOTIFY_AFTER_LOAD,
};

enum blockExecCaller {
	EXEC_CALLER_BOOT,
	EXEC_CALLER_HLE,
};

typedef struct {
	int  (*Init)();
	void (*Reset)();
	void (*Execute)();
	void (*ExecuteBlock)(enum blockExecCaller caller); /* executes up to a jump */
	void (*Clear)(u32 Addr, u32 Size);
	void (*Notify)(enum R3000Anote note, void *data);
	void (*ApplyConfig)();
	void (*Shutdown)();
} R3000Acpu;

extern R3000Acpu *psxCpu;
extern R3000Acpu psxInt;
extern R3000Acpu psxIntDbg;
#if defined(__x86_64__) || defined(__i386__) || defined(__sh__) || defined(__ppc__) || defined(HW_RVL) || defined(HW_DOL)
extern R3000Acpu psxRec;
extern R3000Acpu psxLightrec;
#define PSXREC
#endif

typedef union {
#if defined(HW_RVL) || defined(HW_DOL)
	struct { u8 h3, h2, h, l; } b;
	struct { s8 h3, h2, h, l; } sb;
	struct { u16 h, l; } w;
	struct { s16 h, l; } sw;
#else
	struct { u8 l, h, h2, h3; } b;
	struct { u16 l, h; } w;
	struct { s8 l, h, h2, h3; } sb;
	struct { s16 l, h; } sw;
#endif
} PAIR;

typedef union {
	struct {
		u32   r0, at, v0, v1, a0, a1, a2, a3,
						t0, t1, t2, t3, t4, t5, t6, t7,
						s0, s1, s2, s3, s4, s5, s6, s7,
						t8, t9, k0, k1, gp, sp, fp, ra, lo, hi;
	} n;
	u32 r[34]; /* Lo, Hi in r[32] and r[33] */
	PAIR p[34];
} psxGPRRegs;

typedef union {
	struct {
		u32 Reserved0, Reserved1, Reserved2,  BPC,
		    Reserved4, BDA,       Target,     DCIC,
		    BadVAddr,  BDAM,      Reserved10, BPCM,
		    SR,        Cause,     EPC,        PRid,
		    Reserved16[16];
	} n;
	u32 r[32];
	PAIR p[32];
} psxCP0Regs;

typedef struct {
	short x, y;
} SVector2D;

typedef struct {
	short z, pad;
} SVector2Dz;

typedef struct {
	short x, y, z, pad;
} SVector3D;

typedef struct {
	short x, y, z, pad;
} LVector3D;

typedef struct {
	unsigned char r, g, b, c;
} CBGR;

typedef struct {
	short m11, m12, m13, m21, m22, m23, m31, m32, m33, pad;
} SMatrix3D;

typedef union {
	struct {
		SVector3D     v0, v1, v2;
		CBGR          rgb;
		s32          otz;
		s32          ir0, ir1, ir2, ir3;
		SVector2D     sxy0, sxy1, sxy2, sxyp;
		SVector2Dz    sz0, sz1, sz2, sz3;
		CBGR          rgb0, rgb1, rgb2;
		s32          reserved;
		s32          mac0, mac1, mac2, mac3;
		u32 irgb, orgb;
		s32          lzcs, lzcr;
	} n;
	u32 r[32];
	PAIR p[32];
} psxCP2Data;

typedef union {
	struct {
		SMatrix3D rMatrix;
		s32      trX, trY, trZ;
		SMatrix3D lMatrix;
		s32      rbk, gbk, bbk;
		SMatrix3D cMatrix;
		s32      rfc, gfc, bfc;
		s32      ofx, ofy;
		s32      h;
		s32      dqa, dqb;
		s32      zsf3, zsf4;
		s32      flag;
	} n;
	u32 r[32];
	PAIR p[32];
} psxCP2Ctrl;

enum {
	PSXINT_SIO = 0,
	PSXINT_CDR,
	PSXINT_CDREAD,
	PSXINT_GPUDMA,
	PSXINT_MDECOUTDMA,
	PSXINT_SPUDMA,
	PSXINT_GPUBUSY,
	PSXINT_MDECINDMA,
	PSXINT_GPUOTCDMA,
	PSXINT_CDRDMA,
	PSXINT_NEWDRC_CHECK,
	PSXINT_RCNT,
	PSXINT_CDRLID,
	PSXINT_CDRPLAY,
	PSXINT_SPU_UPDATE,
	PSXINT_SPU_IRQ,
	PSXINT_LIGHTGUN,
	PSXINT_COUNT
};

enum R3000Abdt {
	// corresponds to bits 31,30 of Cause reg
	R3000A_BRANCH_TAKEN = 3,
	R3000A_BRANCH_NOT_TAKEN = 2,
	// none or tells that there was an exception in DS back to doBranch
	R3000A_BRANCH_NONE_OR_EXCEPTION = 0,
};

typedef struct psxCP2Regs {
	psxCP2Data CP2D; 	/* Cop2 data registers */
	psxCP2Ctrl CP2C; 	/* Cop2 control registers */
} psxCP2Regs;

typedef struct {
	// note: some cores like lightrec don't keep their data here,
	// so use R3000ACPU_NOTIFY_BEFORE_SAVE to sync
	psxGPRRegs GPR;		/* General Purpose Registers */
	psxCP0Regs CP0;		/* Coprocessor0 Registers */
	union {
		struct {
			psxCP2Data CP2D; 	/* Cop2 data registers */
			psxCP2Ctrl CP2C; 	/* Cop2 control registers */
		};
		psxCP2Regs CP2;
	};
	u32 pc;				/* Program counter */
	u32 code;			/* The instruction */
	u32 cycle;
	u32 interrupt;
	struct { u32 sCycle, cycle; } intCycle[32];
	u32 gteBusyCycle;
	u32 muldivBusyCycle;
	u32 subCycle;       /* interpreter cycle counting */
	u32 subCycleStep;
	u32 biuReg;
	u8  branching;      /* interp. R3000A_BRANCH_TAKEN / not, 0 if not branch */
	u8  dloadSel;       /* interp. delay load state */
	u8  dloadReg[2];
	u32 dloadVal[2];
	// warning: changing anything in psxRegisters requires update of all
	// asm in libpcsxcore/new_dynarec/
	u8 ICache_Addr[0x1000];
	u8 ICache_Code[0x1000];
	bool ICache_valid;
	u32 io_cycle_counter;
} psxRegisters;

extern psxRegisters psxRegs;

/*
Formula One 2001
- Use old CPU cache code when the RAM location is
  updated with new code (affects in-game racing)

TODO:
- I-cache / D-cache swapping
- Isolate D-cache from RAM
*/

static inline u32 *Read_ICache(u32 pc, bool isolate) {
	u32 pc_bank, pc_offset, pc_cache;
	u8 *IAddr, *ICode;

	pc_bank = pc >> 24;
	pc_offset = pc & 0xffffff;
	pc_cache = pc & 0xfff;

	IAddr = psxRegs.ICache_Addr;
	ICode = psxRegs.ICache_Code;

	// clear I-cache
	if (!psxRegs.ICache_valid) {
		memset(psxRegs.ICache_Addr, 0xff, sizeof(psxRegs.ICache_Addr));
		memset(psxRegs.ICache_Code, 0xff, sizeof(psxRegs.ICache_Code));

		psxRegs.ICache_valid = TRUE;
	}

	// uncached
	if (pc_bank >= 0xa0)
		return (u32 *)PSXM(pc);

	// cached - RAM
	if (pc_bank == 0x80 || pc_bank == 0x00) {
		if (SWAP32(*(u32 *)(IAddr + pc_cache)) == pc_offset) {
			// Cache hit - return last opcode used
			return (u32 *)(ICode + pc_cache);
		} else {
			// Cache miss - addresses don't match
			// - default: 0xffffffff (not init)

			if (!isolate) {
				// cache line is 4 bytes wide
				pc_offset &= ~0xf;
				pc_cache &= ~0xf;

				// address line
				*(u32 *)(IAddr + pc_cache + 0x0) = SWAP32(pc_offset + 0x0);
				*(u32 *)(IAddr + pc_cache + 0x4) = SWAP32(pc_offset + 0x4);
				*(u32 *)(IAddr + pc_cache + 0x8) = SWAP32(pc_offset + 0x8);
				*(u32 *)(IAddr + pc_cache + 0xc) = SWAP32(pc_offset + 0xc);

				// opcode line
				pc_offset = pc & ~0xf;
				*(u32 *)(ICode + pc_cache + 0x0) = psxMu32ref(pc_offset + 0x0);
				*(u32 *)(ICode + pc_cache + 0x4) = psxMu32ref(pc_offset + 0x4);
				*(u32 *)(ICode + pc_cache + 0x8) = psxMu32ref(pc_offset + 0x8);
				*(u32 *)(ICode + pc_cache + 0xc) = psxMu32ref(pc_offset + 0xc);
			}

			// normal code
			return (u32 *)PSXM(pc);
		}
	}

	/*
	TODO: Probably should add cached BIOS
	*/

	// default
	return (u32 *)PSXM(pc);
}

/* new_dynarec stuff */
extern u32 event_cycles[PSXINT_COUNT];
extern u32 next_interupt;

void new_dyna_freeze(void *f, int mode);

#define new_dyna_set_event_abs(e, abs) { \
	u32 abs_ = abs; \
	s32 di_ = next_interupt - abs_; \
	event_cycles[e] = abs_; \
	if (di_ > 0) { \
		/*printf("%u: next_interupt %u -> %u\n", psxRegs.cycle, next_interupt, abs_);*/ \
		next_interupt = abs_; \
	} \
}

#define new_dyna_set_event(e, c) \
	new_dyna_set_event_abs(e, psxRegs.cycle + (c))


#if defined(HW_RVL) || defined(HW_DOL) || defined(BIG_ENDIAN)

#define _i32(x) *(s32 *)&x
#define _u32(x) x

#define _i16(x) (((short *)&x)[1])
#define _u16(x) (((unsigned short *)&x)[1])

#define _i8(x) (((char *)&x)[3])
#define _u8(x) (((unsigned char *)&x)[3])

#else

#define _i32(x) *(s32 *)&x
#define _u32(x) x

#define _i16(x) *(short *)&x
#define _u16(x) *(unsigned short *)&x

#define _i8(x) *(char *)&x
#define _u8(x) *(unsigned char *)&x

#endif

/**** R3000A Instruction Macros ****/
#define _PC_       psxRegs.pc       // The next PC to be executed

#define _fOp_(code)		((code >> 26)       )  // The opcode part of the instruction register
#define _fFunct_(code)	((code      ) & 0x3F)  // The funct part of the instruction register
#define _fRd_(code)		((code >> 11) & 0x1F)  // The rd part of the instruction register
#define _fRt_(code)		((code >> 16) & 0x1F)  // The rt part of the instruction register
#define _fRs_(code)		((code >> 21) & 0x1F)  // The rs part of the instruction register
#define _fSa_(code)		((code >>  6) & 0x1F)  // The sa part of the instruction register
#define _fIm_(code)		((u16)code)            // The immediate part of the instruction register
#define _fTarget_(code)	(code & 0x03ffffff)    // The target part of the instruction register

#define _fImm_(code)	((s16)code)            // sign-extended immediate
#define _fImmU_(code)	(code&0xffff)          // zero-extended immediate

#define _Op_     _fOp_(psxRegs.code)
#define _Funct_  _fFunct_(psxRegs.code)
#define _Rd_     _fRd_(psxRegs.code)
#define _Rt_     _fRt_(psxRegs.code)
#define _Rs_     _fRs_(psxRegs.code)
#define _Sa_     _fSa_(psxRegs.code)
#define _Im_     _fIm_(psxRegs.code)
#define _Target_ _fTarget_(psxRegs.code)

#define _Imm_	 _fImm_(psxRegs.code)
#define _ImmU_	 _fImmU_(psxRegs.code)

#define _rRs_   psxRegs.GPR.r[_Rs_]   // Rs register
#define _rRt_   psxRegs.GPR.r[_Rt_]   // Rt register
#define _rRd_   psxRegs.GPR.r[_Rd_]   // Rd register
#define _rSa_   psxRegs.GPR.r[_Sa_]   // Sa register
#define _rFs_   psxRegs.CP0.r[_Rd_]   // Fs register

#define _c2dRs_ psxRegs.CP2D.r[_Rs_]  // Rs cop2 data register
#define _c2dRt_ psxRegs.CP2D.r[_Rt_]  // Rt cop2 data register
#define _c2dRd_ psxRegs.CP2D.r[_Rd_]  // Rd cop2 data register
#define _c2dSa_ psxRegs.CP2D.r[_Sa_]  // Sa cop2 data register

#define _rHi_   psxRegs.GPR.n.hi   // The HI register
#define _rLo_   psxRegs.GPR.n.lo   // The LO register

#define _JumpTarget_    ((_Target_ * 4) + (_PC_ & 0xf0000000))   // Calculates the target during a jump instruction
#define _BranchTarget_  ((s16)_Im_ * 4 + _PC_)                 // Calculates the target during a branch instruction

#define _SetLink(x)     psxRegs.GPR.r[x] = _PC_ + 4;       // Sets the return address in the link register

int  psxInit();
void psxReset();
void psxShutdown();
void psxException(u32 code, enum R3000Abdt bdt, psxCP0Regs *cp0);
void psxBranchTest();
void psxExecuteBios();
int  psxTestLoadDelay(int reg, u32 tmp);
void psxDelayTest(int reg, u32 bpc);
void psxTestSWInts();
void psxJumpTest();


#ifdef __cplusplus
}
#endif
#endif
