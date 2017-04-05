/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_chip_mc_codec_h_
#define __gc_vsc_chip_mc_codec_h_

BEGIN_EXTERN_C()

/*
  Following are interfaces that user can use to do MC encoding and decoding. User can
  NOT access MC binary directly, it must go thru this mc-codec. For MC codec, it WON'T
  change any value or property of field of MC, it JUST does codec!!! So we dont redefine
  any emuration or structures for the content of field of MC, user just strictly follows
  defines in AQShader.h to fill these codec related helper structures.
*/

/* Source type that HW does not define, we define these just for interface cleanness */
#define MC_AUXILIARY_SRC_TYPE_SAMPLER          0xF0

/* To save opcode bit field, HW makes several texld_xxx mixed into one opcode (HW uses src
   valid or mode in fixed src to determine which operation is really used). So we define
   several auxiliary opcodes to distinguish these opcodes for interface cleanness. */
#define MC_AUXILIARY_OP_CODE_TEXLD_LOD         0xFFFF0000 /* Extended from 0x6F */
#define MC_AUXILIARY_OP_CODE_TEXLD_LOD_PCF     0xFFFF0001 /* Extended from 0x6F */
#define MC_AUXILIARY_OP_CODE_TEXLD_BIAS        0xFFFF0002 /* Extended from 0x18 */
#define MC_AUXILIARY_OP_CODE_TEXLD_BIAS_PCF    0xFFFF0003 /* Extended from 0x18 */
#define MC_AUXILIARY_OP_CODE_TEXLD_PLAIN       0xFFFF0004 /* Extended from 0x18 */
#define MC_AUXILIARY_OP_CODE_TEXLD_PCF         0xFFFF0005 /* Extended from 0x18 */
#define MC_AUXILIARY_OP_CODE_TEXLD_U_PLAIN     0xFFFF0006 /* Extended from 0x7B */
#define MC_AUXILIARY_OP_CODE_TEXLD_U_LOD       0xFFFF0007 /* Extended from 0x7B */
#define MC_AUXILIARY_OP_CODE_TEXLD_U_BIAS      0xFFFF0008 /* Extended from 0x7B */
#define MC_AUXILIARY_OP_CODE_TEXLD_GATHER      0xFFFF0009 /* Extended from 0x7D */
#define MC_AUXILIARY_OP_CODE_TEXLD_GATHER_PCF  0xFFFF000a /* Extended from 0x7D */
#define MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_PLAIN 0xFFFF000b /* Extended from 0x7B */
#define MC_AUXILIARY_OP_CODE_TEXLD_U_F_B_BIAS  0xFFFF000c /* Extended from 0x7B */

/* Normally, stores don't need dst, but for the chips that equiped with USC, if src2 is
   immediate/constant/dynamic-indexing, dst must be provided. To differ with normal stores,
   following axuiliary opcodes are defined, that means these stores have dst */
#define MC_AUXILIARY_OP_CODE_USC_STORE         0xFFFF0020
#define MC_AUXILIARY_OP_CODE_USC_IMG_STORE     0xFFFF0021
#define MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D  0xFFFF0022
#define MC_AUXILIARY_OP_CODE_USC_STORE_ATTR    0xFFFF0023
#define MC_AUXILIARY_OP_CODE_USC_STOREP        0xFFFF0024

#define MC_AUXILIARY_OP_CODE_OFFSET            MC_AUXILIARY_OP_CODE_TEXLD_LOD

#define IS_ATOMIC_MC_OPCODE(opcode)                          \
    (((opcode) == 0x65)               || \
     ((opcode) == 0x66)              || \
     ((opcode) == 0x67)          || \
     ((opcode) == 0x68)               || \
     ((opcode) == 0x69)               || \
     ((opcode) == 0x6A)                || \
     ((opcode) == 0x6B)               || \
     ((opcode) == 0x6C))

#define IS_IMG_ATOMIC_MC_OPCODE(opcode)                      \
     ((opcode) == 0x46)

#define IS_NORMAL_LOAD_MC_OPCODE(opcode)                     \
    (((opcode) == 0x32)                   || \
     ((opcode) == 0x39))

#define IS_NORMAL_STORE_MC_OPCODE(opcode)                    \
    (((opcode) == 0x33)                  || \
     ((opcode) == MC_AUXILIARY_OP_CODE_USC_STORE)         || \
     ((opcode) == 0x3A)                 || \
     ((opcode) == MC_AUXILIARY_OP_CODE_USC_STOREP))

#define IS_NORMAL_LOAD_STORE_MC_OPCODE(opcode)               \
    (IS_NORMAL_LOAD_MC_OPCODE(opcode)                     || \
     IS_NORMAL_STORE_MC_OPCODE(opcode))

#define IS_IMG_LOAD_MC_OPCODE(opcode)                        \
    (((opcode) == 0x79)               || \
     ((opcode) == 0x34))

#define IS_IMG_STORE_MC_OPCODE(opcode)                       \
    (((opcode) == 0x7A)              || \
     ((opcode) == 0x35)           || \
     ((opcode) == MC_AUXILIARY_OP_CODE_USC_IMG_STORE)     || \
     ((opcode) == MC_AUXILIARY_OP_CODE_USC_IMG_STORE_3D))

#define IS_IMG_LOAD_STORE_MC_OPCODE(opcode)                  \
    (IS_IMG_LOAD_MC_OPCODE((opcode))                      || \
     IS_IMG_STORE_MC_OPCODE((opcode)))

#define IS_BARRIER_MC_OPCODE(opcode)                         \
     ((opcode) == 0x2A)

#define IS_MEM_ACCESS_MC_OPCODE(opcode)                      \
    (IS_ATOMIC_MC_OPCODE((opcode))                        || \
     IS_NORMAL_LOAD_STORE_MC_OPCODE((opcode))             || \
     IS_IMG_LOAD_STORE_MC_OPCODE((opcode))                || \
     IS_IMG_ATOMIC_MC_OPCODE((opcode)))

/* Codec helper structures */
typedef struct _VSC_MC_CODEC_DST
{
    gctUINT     regNo;
    gctUINT     regType;

    union
    {
        /* For normal dst */
        struct
        {
            gctUINT     writeMask;
            gctUINT     indexingAddr;
        } nmlDst;

        /* For EVIS dst */
        struct
        {
            gctUINT     startCompIdx;
            gctUINT     compIdxRange;
        } evisDst;
    } u;
}VSC_MC_CODEC_DST;

typedef union _VSC_MC_CODEC_IMM_VALUE
{
    gctUINT     ui;
    gctINT      si;
    gctFLOAT    f;
}VSC_MC_CODEC_IMM_VALUE;

typedef struct _VSC_MC_CODEC_SRC
{
    union
    {
        /* For any normal/special reg */
        struct
        {
            gctUINT      regNo;
            gctUINT      swizzle;
            gctUINT      indexingAddr;
            gctBOOL      bNegative;
            gctBOOL      bAbs;
        } reg;

        /* Any immediate value including branch-target */
        struct
        {
            VSC_MC_CODEC_IMM_VALUE immData;
            gctUINT                immType;
        } imm;
    } u;

    gctUINT     regType;
}VSC_MC_CODEC_SRC;

typedef struct _VSC_MC_CODEC_INST_CTRL
{
    gctUINT     condOpCode;
    gctUINT     instType;
    gctUINT     roundingMode;
    gctUINT     packMode;
    gctBOOL     bDenorm;
    gctUINT     threadType;
    gctBOOL     bSkipForHelperKickoff;
    gctBOOL     bResultSat;

    union
    {
        /* For memory access insts, such as LD/ST/MOVBIX/img-LD/img-ST/atomic/imgAtomic.
           NOTE: MOVBIX is put here is because it designed as address calcation of LD,
                 so following ctrl will also control this inst */
        struct
        {
            gctBOOL     bAccessLocalStorage;
            gctBOOL     bUnderEvisMode;

            union
            {
                /* For LD/ST/MOVBIX inst */
                struct
                {
                    gctUINT     offsetLeftShift;
                    gctBOOL     bOffsetX3;
                } lsCtrl;

                /* For IMG_ATOM inst */
                struct
                {
                    gctUINT     atomicMode;
                    gctBOOL     b3dImgMode;
                } imgAtomCtrl;
            } u;
        } maCtrl;

        /* For SELECT_MAP inst */
        struct
        {   gctUINT     rangeToMatch;
            gctBOOL     bCompSel;
        } smCtrl;

        /* For vision inst, which must be under EVIS mode */
        struct
        {   gctUINT     evisState;
            gctUINT     startSrcCompIdx; /* Source bin */
        } visionCtrl;

        /* For load_attr/store_attr */
        struct
        {
            gctBOOL     bNeedUscSync;
            gctUINT     shStageClient;
            gctUINT     attrLayout;
        } lsAttrCtrl;

        /* For emit inst */
        struct
        {
            gctBOOL     bNeedRestartPrim;
            gctBOOL     bJmpToEndOnMaxVtxCnt;
        } emitCtrl;

        /* For pack inst */
        gctUINT     srcSelect;

        /* For branch inst */
        gctUINT     loopOpType;

        /* For mul/norm_mul/mad/mullo/dst/dp/norm_dp inst */
        gctBOOL     bInfX0ToZero;
    } u;
}VSC_MC_CODEC_INST_CTRL;

#define MAX_MC_SRC_COUNT   4
typedef struct _VSC_MC_CODEC_INST
{
    gctUINT                baseOpcode;
    gctUINT                extOpcode;

    VSC_MC_CODEC_INST_CTRL instCtrl;

    VSC_MC_CODEC_DST       dst;
    gctBOOL                bDstValid;

    VSC_MC_CODEC_SRC       src[MAX_MC_SRC_COUNT];

    /* Although HW MC may put src at different src location, however, for mc-codec
       helper inst, it is more convenient to hide this limitation, and squeeze all
       used srcs together. srcCount means real used src count after squeezing. */
    gctUINT                srcCount;
}VSC_MC_CODEC_INST;

/* A dummy MC define for user, it wont see any content of binary */
typedef struct _VSC_MC_RAW_INST
{
    gctUINT word[4];
}VSC_MC_RAW_INST;

#define VSC_MC_INST_DWORD_SIZE   4

/* An object representing mc-codec */
typedef struct _VSC_MC_CODEC
{
    VSC_HW_CONFIG*    pHwCfg;
    gctBOOL           bDual16ModeEnabled;

    gctBOOL           bInit;

    /* If MC is not generated by this mc-codec, some reserved fields may be written
       by garbage, for HW execution, it will not have any issue, but when this kind
       of MC is decoded by this mc-codec, this garbage will make our reverse-engine
       check failed. So when decoding mc which is not generated by this mc-codec, we
       need set this boolean to FALSE */
    gctBOOL           bCanReverseEngineCheck;
}VSC_MC_CODEC;

/* MC codec routines */
void vscMC_BeginCodec(VSC_MC_CODEC* pMcCodec,
                      VSC_HW_CONFIG* pHwCfg,
                      gctBOOL bDual16ModeEnabled,
                      gctBOOL bCanReverseEngineCheck);
void vscMC_EndCodec(VSC_MC_CODEC* pMcCodec);

gctBOOL vscMC_EncodeInstDirect(VSC_MC_CODEC*          pMcCodec,
                               gctUINT                baseOpcode,
                               gctUINT                extOpcode,
                               VSC_MC_CODEC_INST_CTRL instCtrl,
                               VSC_MC_CODEC_DST*      pDst,
                               VSC_MC_CODEC_SRC*      pSrc,
                               gctUINT                srcCount,
                               VSC_MC_RAW_INST*       pOutMCInst);

gctBOOL vscMC_EncodeInst(VSC_MC_CODEC*                pMcCodec,
                         VSC_MC_CODEC_INST*           pInCodecHelperInst,
                         VSC_MC_RAW_INST*             pOutMCInst);

gctBOOL vscMC_DecodeInst(VSC_MC_CODEC*                pMcCodec,
                         VSC_MC_RAW_INST*             pInMCInst,
                         VSC_MC_CODEC_INST*           pOutCodecHelperInst);

gctBOOL vscMC_EncodeSrc(VSC_MC_CODEC*                 pMcCodec,
                        VSC_MC_CODEC_SRC*             pSrc,
                        gctBOOL                       bEvisMode,
                        gctUINT                       mcSrcIdx,
                        VSC_MC_RAW_INST*              pOutMCInst);

gctBOOL vscMC_DecodeSrc(VSC_MC_CODEC*                 pMcCodec,
                        VSC_MC_RAW_INST*              pInMCInst,
                        gctBOOL                       bEvisMode,
                        gctUINT                       mcSrcIdx,
                        VSC_MC_CODEC_SRC*             pOutSrc);

/* Misc routines */
gctUINT vscMC_GetFreeSrcCount(VSC_MC_CODEC*           pMcCodec,
                              VSC_MC_RAW_INST*        pMCInst,
                              gctUINT*                pOutMcSrcIdxArray /* The size of array must be at least 3 */);

END_EXTERN_C()

#endif /* __gc_vsc_chip_mc_codec_h_ */

