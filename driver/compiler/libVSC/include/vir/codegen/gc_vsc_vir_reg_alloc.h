/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_reg_alloc_h_
#define __gc_vsc_vir_reg_alloc_h_

#include "gc_vsc.h"
#include "vir/analysis/gc_vsc_vir_dfa.h"

BEGIN_EXTERN_C()

/* constants define */
#define VIR_RA_INVALID_REG          0x3FF

#define VIR_RS_LS_MEM_BLK_SIZE      2048

#define VIR_RA_LS_POS_MAX           (2048*6)
#define VIR_RA_LS_POS_MIN           0

#define VIR_RA_LS_REG_MAX           0xFFFFFFFF

/* the color is assigned as attribute */
#define VIR_RA_LS_ATTRIBUTE_FUNC    (VIR_Function*)(-1)

/* the max value for live range's weight */
#define VIR_RA_LS_MAX_WEIGHT        5.0

#define VIR_RA_LS_MAX_MOVA_REG      4

/*
** DEST may need a extra register(for some undefinite-write instructions, e.g. CMOV),
** and for a 256 register pair, they must be consecutive and start with even temp register, so we may need one more for it.
*/
#define VIR_RA_LS_DATA_REG_NUM      (VIR_MAX_SRC_NUM + 1 + 1)

/* we use a gctUINT to represent a register pair.
   we use 10 bit to represent a HW register number, which restricts the
   register count to 1024. If we ever could assign more than that, we need
   to enlarge here and _hwRegId in operand and symbol as well. */
typedef struct _VIR_RA_HWREG_COLOR
{
    VIR_HwRegId             _hwRegId        : 10;  /* hardware physical register index */
    gctUINT                 _hwShift        : 2;
    VIR_HwRegId             _HIhwRegId      : 10;  /* in dual16, a pair of register is needed */
    gctUINT                 _HIhwShift      : 2;   /* in dual16, a pair of register is needed */
    gctUINT                 _reserved       : 8;   /* not used bits */
} VIR_RA_HWReg_Color;

typedef struct _VIR_RA_TEMP_HWREG_COLOR
{
    gctUINT                 tempEndPoint;
    VIR_RA_HWReg_Color      color;
} VIR_RA_TEMP_HWReg_Color;

/* hw register types */
typedef enum _VIR_RA_HWREG_TYPE
{
    VIR_RA_HWREG_GR = 0,
    VIR_RA_HWREG_A0 = 1,
    VIR_RA_HWREG_B0 = 2,
    /* add other special register support */
    VIR_RA_HWREG_TYPE_COUNT
} VIR_RA_HWReg_Type;

/* color mapping for a type of hw registers */
typedef struct _VIR_RA_COLOR_MAP
{
    gctUINT                 maxReg;          /* maximal number of hw reg */
    gctUINT                 availReg;        /* smallest available hw reg to assign */
    gctUINT                 maxAllocReg;     /* maximal number of hw reg allocated*/


    /* record whether an register is used or not
       each register takes 4-bits for channels [w, z, y, x] */
    VSC_BIT_VECTOR          usedColor;

} VIR_RA_ColorMap;

/* color mapping for all types of hw registers */
typedef struct _VIR_RA_COLOR_POOL
{
    VIR_RA_ColorMap        colorMap[VIR_RA_HWREG_TYPE_COUNT];

} VIR_RA_ColorPool;

/* linear scan register allocator defines */

/* liverange defines */
typedef struct VIR_RA_LS_INTERVAL VIR_RA_LS_Interval;
struct VIR_RA_LS_INTERVAL
{
    gctUINT                 startPoint;
    gctUINT                 endPoint;
    VIR_RA_LS_Interval      *next;

};

typedef enum _VIR_RA_LR_FLAG
{
    VIR_RA_LRFLAG_NONE              = 0x0, /* No flags. */
    VIR_RA_LRFLAG_NO_SHIFT          = 0x1, /* whether this LR allows shift */
    VIR_RA_LRFLAG_INVALID           = 0x2, /* whether this LR is invalid and no color assigned.
                                                  A LR is invalid because it is not the first temp in
                                                  output array/matrix. */
    VIR_RA_LRFLAG_SUB_SAMPLE_DEPTH  = 0x4, /* whether this LR is for sub-sample depth */
    VIR_RA_LRFLAG_ATOMCMPXHG        = 0x8, /* whether this LR is for atom_cmp_xchg */

    VIR_RA_LRFLAG_SPILLED           = 0x10, /* whether this LR is spilled */
    VIR_RA_LRFLAG_RM_LDARR_DEST     = 0x20, /* whether this LR is a removable LDARR dest and no need
                                                   for coloring */
    VIR_RA_LRFLAG_MASTER_WEB_IDX_SET= 0x40, /* whether the masterWebIdx of this LR is replaced by its master webIdx */

    VIR_RA_LRFLAG_DYN_INDEXING      = 0x80, /* whether LR is dynamically indexed */

    VIR_RA_LRFLAG_VX_EVEN           = 0x100, /* whether LR is in VX special instruciton and needs to be put into even number register */
    VIR_RA_LRFLAG_VX_ODD            = 0x200, /* whether LR is in VX special instruciton and needs to be put into odd number register */

    VIR_RA_LRFLAG_NO_USED_COLOR     = 0x400, /* whether LR should not assign a used color */

    VIR_RA_LRFLAG_A0_INVALID        = 0x1000, /* this A0 LR has been invalid */

    VIR_RA_LRFLAG_ST_DEP            = 0x2000, /* whether this LR has depedence because of store */
    VIR_RA_LRFLAG_LD_DEP            = 0x4000, /* whether this LR has depedence because of load */

    VIR_RA_LRFLAG_VX                = 0x8000, /* whether LR is in VX instruction */

    VIR_RA_LRHIGHPVEC2              = 0x10000, /* whether LR is highpvec2 */

} VIR_RA_LRFlag;

typedef struct VIR_RA_LS_LIVERANGE VIR_RA_LS_Liverange;
struct VIR_RA_LS_LIVERANGE
{
    gctUINT                 webIdx;
    gctUINT                 firstRegNo;
    gctUINT                 regNoRange;

    VIR_RA_LRFlag           flags;          /* LR flags */

    gctUINT                 masterWebIdx;   /* for invalid LR, where to find its color */

    VIR_RA_HWReg_Type       hwType;         /* hw register type to allocate, GR or SR*/

    gctUINT                 startPoint;
    gctUINT                 endPoint;
    gctUINT                 origEndPoint;   /* this is for MOVA to save its original LR */
    VIR_RA_LS_Interval      *deadIntervals;
    union
    {
        VIR_RA_HWReg_Color  color;          /* the color for this LR*/
        gctUINT             spillOffset;    /* the spill offset if LR is spilled  */
    }u1;

    VIR_RA_TEMP_HWReg_Color tempColor;      /* the temp color for this LR*/

    VIR_Function            *liveFunc;     /* function where the web appears */
    VIR_Function            *colorFunc;    /* function where the web is colored */

    VIR_Enable              channelMask;

    VIR_RA_LS_Liverange     *nextLR;        /* used in the sorted live range list */
    VIR_RA_LS_Liverange     *nextActiveLR;  /* used in the active live range list */

    VIR_RA_LS_Liverange     *usedColorLR;
    gctBOOL                 deadIntervalAvail;

    gctFLOAT                weight;         /* the weight for LR, used for spill selection */

    gctUINT                 currDef;
    gctINT                  minDefPos;     /* earliest define pos in the liverange */

    VIR_Instruction         *pLDSTInst;
} ;

#define VIR_RA_LR_SetFlag(LR, Val)        do {(LR)->flags |= (Val); } while (0)
#define VIR_RA_LR_ClrFlag(LR, Val)        do {(LR)->flags &= ~(Val); } while (0)

#define isLRNoShift(LR)                 (((LR)->flags & VIR_RA_LRFLAG_NO_SHIFT) != 0)
#define isLRInvalid(LR)                 (((LR)->flags & VIR_RA_LRFLAG_INVALID) != 0)
#define isLRSubSampleDepth(LR)          (((LR)->flags & VIR_RA_LRFLAG_SUB_SAMPLE_DEPTH) != 0)
#define isLRAtomCmpXchg(LR)             (((LR)->flags & VIR_RA_LRFLAG_ATOMCMPXHG) != 0)
#define isLRSpilled(LR)                 (((LR)->flags & VIR_RA_LRFLAG_SPILLED) != 0)
#define isLRCanLdarrBeRemoved(LR)       (((LR)->flags & VIR_RA_LRFLAG_RM_LDARR_DEST) != 0)
#define isLRMasterWebIdxSet(LR)         (((LR)->flags & VIR_RA_LRFLAG_MASTER_WEB_IDX_SET) != 0)
#define isLRDynIndexing(LR)             (((LR)->flags & VIR_RA_LRFLAG_DYN_INDEXING) != 0)
#define isLRVXEven(LR)                  (((LR)->flags & VIR_RA_LRFLAG_VX_EVEN) != 0)
#define isLRVXOdd(LR)                   (((LR)->flags & VIR_RA_LRFLAG_VX_ODD) != 0)
#define isLRNoUsedColor(LR)             (((LR)->flags & VIR_RA_LRFLAG_NO_USED_COLOR) != 0)
#define isLRA0Invalid(LR)               (((LR)->flags & VIR_RA_LRFLAG_A0_INVALID) != 0)
#define isLRSTDependence(LR)            (((LR)->flags & VIR_RA_LRFLAG_ST_DEP) != 0)
#define isLRLDDependence(LR)            (((LR)->flags & VIR_RA_LRFLAG_LD_DEP) != 0)
#define isLRVX(LR)                      (((LR)->flags & VIR_RA_LRFLAG_VX) != 0)
#define isLRHighpVec2(LR)               (((LR)->flags & VIR_RA_LRHIGHPVEC2) != 0)

typedef enum _VIR_RA_LS_FLAG
{
    VIR_RA_LS_FLAG_NONE                        = 0x0, /* No flags. */
    VIR_RA_LS_FLAG_HAS_SUB_SAMPLE_DEPTH_LR     = 0x1, /* whether has a LR that is for sub-sample depth */
} VIR_RA_LSFlag;

#define isRALSSubSampleDepth(RA)        (((RA)->raFlags & VIR_RA_LS_FLAG_HAS_SUB_SAMPLE_DEPTH_LR) != 0)

/* register allocator define */
typedef struct VIR_REG_ALLOC_LINEAR_SCAN
{
    VIR_Shader                 *pShader;
    VIR_Dumper                 *pDumper;
    VSC_OPTN_RAOptions         *pOptions;
    VSC_MM                     *pMM;

    VSC_HW_CONFIG              *pHwCfg;

    VIR_LIVENESS_INFO          *pLvInfo;
    VIR_CALL_GRAPH             *pCG;

    gctUINT                     numWeb;
    gctUINT                     numDef;

    VIR_RA_LSFlag               raFlags;

    VIR_RA_ColorPool            colorPool;

    /* live range table */
    VSC_SIMPLE_RESIZABLE_ARRAY  LRTable;

    /* live ranges who are live at current */
    VSC_BIT_VECTOR              liveLRVec;

    gctUINT                     currPos;

    /* sorted live range list head */
    VIR_RA_LS_Liverange        *sortedLRHead;

    /* active live range list head */
    VIR_RA_LS_Liverange        *activeLRHead;

    /* a hash table to connect invalid LR with its master LR
       KEY <useInst, master_tempIdx>, VALUE master_webIdx */
    VSC_HASH_TABLE              *outputLRTable;

    VIR_HwRegId                 resRegister;

    gctBOOL                     needBoundsCheck;   /* for Robust OOB check HW and OOB check enabled */
    /* the offset for next spill, it starts from 0 */
    gctUINT                     spillOffset;
    /* reserved HW register for base address, bounds info, offset, or threadIndex
       baseRegister.x base address for spill
       if bounds check:
         baseRegister.yz  spill mem block start and end address
         baseRegister.w   computed offset for spill (LDARR/STARR),
                          save dest for spill(STARR) if the src2 is not in temp
         baseRegister.w   threadIndex got from the atomic add
       else
         baseRegister.y   computed offset for spill (LDARR/STARR),
                          save dest for spill(STARR) if the src2 is not in temp
         baseRegister.y   threadIndex got from the atomic add */
    VIR_HwRegId                 baseRegister;
    gctUINT                     scratchChannel;  /* computed offset for spill, save dest for spill
                                                  * threadIndex got from the atomic add */

    gctBOOL                     bReservedMovaReg;
    /* reserved HW register for saving MOVA src0 if
       LDARR spill base  */
    gctUINT                     movaRegCount;
    VIR_HwRegId                 movaRegister[VIR_RA_LS_MAX_MOVA_REG];

    /* a hash table to connect spilled offset' MOVA with its MOV instruction
       (we need a MOV, in case MOVA's src is redefined, we reserve the register
        to save it ) */
    VSC_HASH_TABLE              *movaHash;

    /* reserved HW register for data, we may need to reserve more than one
       registers to save the data, since in some instruction, maybe more than
       one src is spilled, and even the dest may need a extra register(for some undefinite-write instructions, e.g. CMOV). */
    gctUINT                     resDataRegisterCount;
    VIR_HwRegId                 dataRegister[VIR_RA_LS_DATA_REG_NUM];
    gctBOOL                     dataRegisterUsed[VIR_RA_LS_DATA_REG_NUM];
    gctUINT                     dataRegisterEndPoint[VIR_RA_LS_DATA_REG_NUM];

    /* save and use the correct symbolId to make IR valid */
    VIR_SymId                   baseAddrSymId;
    VIR_SymId                   spillOffsetSymId;
    VIR_SymId                   threadIdxSymId;
    VIR_SymId                   dataSymId[VIR_RA_LS_DATA_REG_NUM];

    VIR_HwRegId                 samplePosRegister;

    /* Extended the endPoint of LS instruction. */
    gctBOOL                     bExtendedLSEndPoint;

    /* Change the instruction ID. */
    gctBOOL                     bInstIdChanged;

    gctUINT                     trueDepPoint;

    /* registers that are occupied because of false dep */
    VSC_BIT_VECTOR              falseDepRegVec;

    /* Save the current max register count. */
    gctUINT                     currentMaxRegCount;

    gctBOOL                     bEnableDebug;
    VSC_DIContext               *DIContext;

} VIR_RA_LS;

#define VIR_RA_LS_GetShader(ra)                 ((ra)->pShader)
#define VIR_RA_LS_SetShader(ra, s)              ((ra)->pShader = (s))
#define VIR_RA_LS_GetDumper(ra)                 ((ra)->pDumper)
#define VIR_RA_LS_SetDumper(ra, d)              ((ra)->pDumper = (d))
#define VIR_RA_LS_GetOptions(ra)                ((ra)->pOptions)
#define VIR_RA_LS_SetOptions(ra, o)             ((ra)->pOptions = (o))
#define VIR_RA_LS_GetMM(ra)                     ((ra)->pMM)
#define VIR_RA_LS_GetHwCfg(ra)                  ((ra)->pHwCfg)
#define VIR_RA_LS_SetHwCfg(ra, hc)              ((ra)->pHwCfg = (hc))
#define VIR_RA_LS_GetLvInfo(ra)                 ((ra)->pLvInfo)
#define VIR_RA_LS_SetLvInfo(ra, lv)             ((ra)->pLvInfo = (lv))
#define VIR_RA_LS_GetCallGraph(ra)              ((ra)->pCG)
#define VIR_RA_LS_SetCallGraph(ra, cg)          ((ra)->pCG = (cg))
#define VIR_RA_LS_GetNumWeb(ra)                 ((ra)->numWeb)
#define VIR_RA_LS_SetNumWeb(ra, s)              ((ra)->numWeb = (s))
#define VIR_RA_LS_GetNumDef(ra)                 ((ra)->numDef)
#define VIR_RA_LS_SetNumDef(ra, s)              ((ra)->numDef = (s))
#define VIR_RA_LS_SetFlags(ra, f)               ((ra)->raFlags = (f))
#define VIR_RA_LS_SetFlag(ra, f)                do {(ra)->raFlags |= (f); } while (gcvFALSE)
#define VIR_RA_LS_ClrFlag(ra, f)                do {(ra)->raFlags &= ~(f); } while (gcvFALSE)
#define VIR_RA_LS_GetColorPool(ra)              (&((ra)->colorPool))
#define VIR_RA_LS_GetLRTable(ra)                (&((ra)->LRTable))
#define VIR_RA_LS_GetLiveLRVec(ra)              (&((ra)->liveLRVec))
#define VIR_RA_LS_GetCurrPos(ra)                ((ra)->currPos)
#define VIR_RA_LS_GetSortedLRHead(ra)           ((ra)->sortedLRHead)
#define VIR_RA_LS_SetSortedLRHead(ra, h)        ((ra)->sortedLRHead = (h))
#define VIR_RA_LS_GetActiveLRHead(ra)           ((ra)->activeLRHead)
#define VIR_RA_LS_SetActiveLRHead(ra, h)        ((ra)->activeLRHead = (h))
#define VIR_RA_LS_GetFalseDepRegVec(ra)         (&((ra)->falseDepRegVec))
#define VIR_RA_LS_GetExtendLSEndPoint(ra)       ((ra)->bExtendedLSEndPoint)
#define VIR_RA_LS_SetExtendLSEndPoint(ra, h)    ((ra)->bExtendedLSEndPoint = (h))
#define VIR_RA_LS_GetInstIdChanged(ra)          ((ra)->bInstIdChanged)
#define VIR_RA_LS_SetInstIdChanged(ra, h)       ((ra)->bInstIdChanged = (h))
#define VIR_RA_LS_GetEnableDebug(ra)            ((ra)->bEnableDebug)
#define VIR_RA_LS_SetEnableDebug(ra, h)         ((ra)->bEnableDebug = (h))

extern VSC_ErrCode VIR_RA_LS_PerformTempRegAlloc(
    IN VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VIR_RA_LS_PerformTempRegAlloc);
DECLARE_SH_NECESSITY_CHECK(VIR_RA_LS_PerformTempRegAlloc);

extern VSC_ErrCode VIR_RA_PerformUniformAlloc(
    VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VIR_RA_PerformUniformAlloc);
DECLARE_SH_NECESSITY_CHECK(VIR_RA_PerformUniformAlloc);

END_EXTERN_C()

#endif /* __gc_vsc_vir_reg_alloc_h_ */

