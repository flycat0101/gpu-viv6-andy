/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/codegen/gc_vsc_vir_reg_alloc.h"

/* Register Allocation for Shader
 *
 * Whole shader program register allocation
 *
 *   The register allocator allocates registers based on the global liveness
 *   information. The register allocator traverse the call graph and uses
 *   the linear scan algorithm to color each function. Since there is no "global"
 *   interference information, the new register is used to resolve the conflict.
 *   Thus the order to traverse call graph does not matter.
 *
 * Linear scan register allocation (RA_LS)
 *
 *      000  BB1:
 *      001     r1 <- ...
 *      002     r2 <- ...
 *      003     if cond goto BB3 else goto BB2
 *      004  BB2:
 *      005     r3 <- ...
 *      006     ... <- r3
 *      007     goto BB4
 *      008  BB3:
 *      009     ... <- r2
 *      010  BB4:
 *      011  ... <- r1
 *
 *  there are three interesting live ranges for the above case:
 *      LR1:  r1, <1, 11>
 *      LR2:  r2, <2, 9>
 *      LR3:  r3, <5, 6>
 *
 *  there is a dead interval in the LR2, r2 is not live in <4, 7>.
 *  thus we use (startPoint, endPoint and dead intervals)
 *  to represent a live range.
 *
 *  things get complicated with channel color assignment.
 *  all the channels in the same use(or def) must be assigned as
 *  the same register (for performance purpose).
 *  thus the live range is for a "web" (including all related
 *  channels), the live range should have channel information.
 *
 *  example here:
 */

/*
** So far we allocate 16 bytes for every single temp register, no matter what data type of temp register is.
** But for all components, we need to pack them together based on the data type,
** because that for LOAD/STORE instruction, we use data type to calculate the componenet offset.
** e.g.
**  For a UINT8_X4, we allocate 16 bytes, but only use the first 4 bytes.
**  For a UINT_X4, we allocate 16 bytes, and use all of them.
*/
#define __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__          16
#define __MAX_TEMP_REGISTER_COMPONENT_SIZE_IN_BYTE__    4

/* end mark for the live range list, used in sorting live ranges
   and ending the active live range list */
static VIR_RA_LS_Liverange LREndMark = {
    VIR_INVALID_WEB_INDEX, /* webIdx */
    0, /* firstRegNo */
    1, /* regNoRange */
    0, /* flags    */
    VIR_INVALID_WEB_INDEX, /* masterWebIdx */
    VIR_RA_HWREG_GR, /* hwType */
    VIR_RA_LS_POS_MAX, /* startPoint */
    VIR_RA_LS_POS_MAX, /* endPoint */
    VIR_RA_LS_POS_MAX, /* origEndPoint */
    gcvNULL, /* deadIntervals */
    {                           /* u1 */
        {
        VIR_RA_INVALID_REG,
        0,
        VIR_RA_INVALID_REG,
        0,
        0
        }
    }, /* color */
    {                           /* tempColor */
        VIR_INVALID_ID,
        {
        VIR_RA_INVALID_REG,
        0,
        VIR_RA_INVALID_REG,
        0,
        0
        }
    },
    gcvNULL, /* colorFunc */
    gcvNULL, /* liveFunc */
    0, /* channelMask */
    gcvNULL, /* nextLR */
    gcvNULL, /* nextActiveLR */
    gcvNULL, /* usedColorLR */
    gcvTRUE, /* deadIntervalAvail */
    VIR_RA_LS_MAX_WEIGHT, /* weight */
    VIR_RA_LS_POS_MAX, /* currDef */
    VIR_RA_LS_POS_MAX, /* minDefPos */
    gcvNULL                     /* pLDSTInst */
};

/* coloring const */
static const char* VIR_RA_ChannelsName[16] =
{
    "none", /* 0x0 0000 */
    "x", /* 0x1 0001 */
    "y", /* 0x2 0010 */
    "xy", /* 0x3 0011 */
    "z", /* 0x4 0100 */
    "xz", /* 0x5 0101 */
    "yz", /* 0x6 0110 */
    "xyz", /* 0x7 0111 */
    "w", /* 0x8 1000 */
    "xw", /* 0x9 1001 */
    "yw", /* 0xA 1010 */
    "xyw", /* 0xB 1011 */

    "zw", /* 0xC 1100 */
    "xzw", /* 0xD 1101 */
    "yzw", /* 0xE 1110 */
    "xyzw", /* 0xF 1111 */
};

/* invalid register color */
static VIR_RA_HWReg_Color InvalidColor = {
    VIR_RA_INVALID_REG,
    0,
    VIR_RA_INVALID_REG,
    0,
    0
};

/* ===========================================================================
   utility functions
   ===========================================================================
*/

/* const key for defInst of spilled src0 of LDARR */
typedef struct _VSC_RA_MOVA_CONSTKEY
{
    VIR_Instruction*  defInst;
    VIR_Enable       enable;
} _VSC_RA_MOVA_CONSTKEY;

static gctUINT _HFUNC_RA_MOVA_CONSTKEY(const void* ptr)
{
    gctUINT hashVal = (gctUINT)((((gctUINTPTR_T)((_VSC_RA_MOVA_CONSTKEY*)ptr)->defInst) << 4) |
                                (((_VSC_RA_MOVA_CONSTKEY*)ptr)->enable & 0xf));

    return hashVal;
}

static gctBOOL _HKCMP_RA_MOVA_CONSTKEY(const void* pHashKey1, const void* pHashKey2)
{
    return (((_VSC_RA_MOVA_CONSTKEY*)pHashKey1)->defInst == ((_VSC_RA_MOVA_CONSTKEY*)pHashKey2)->defInst)
        && (((_VSC_RA_MOVA_CONSTKEY*)pHashKey1)->enable == ((_VSC_RA_MOVA_CONSTKEY*)pHashKey2)->enable);
}

static _VSC_RA_MOVA_CONSTKEY*
_VSC_MOVA_NewConstKey(
    VSC_MM      *pMM,
    VIR_Instruction*  defInst,
    VIR_Enable       enable
    )
{
    _VSC_RA_MOVA_CONSTKEY* constKey = (_VSC_RA_MOVA_CONSTKEY*)vscMM_Alloc(pMM, sizeof(_VSC_RA_MOVA_CONSTKEY));
    constKey->defInst = defInst;
    constKey->enable =  enable;
    return constKey;
}

static gctBOOL
_VSC_RA_MOVA_GetConstVal(
    IN VSC_HASH_TABLE*    movaHash,
    IN VIR_Instruction*   defInst,
    IN VIR_Enable        enable,
    OUT void**            retConst
    )
{
    _VSC_RA_MOVA_CONSTKEY    constKey;
    constKey.defInst = defInst;
    constKey.enable = enable;
    return vscHTBL_DirectTestAndGet(movaHash, &constKey, retConst);
}

static void
_VSC_RA_MOVA_RemoveConstValAllChannel(
    IN OUT VSC_HASH_TABLE*    movaHash,
    IN VIR_Instruction*       defInst
    )
{
    _VSC_RA_MOVA_CONSTKEY    constKey;
    constKey.defInst = defInst;
    constKey.enable = VIR_ENABLE_X;
    vscHTBL_DirectRemove(movaHash, &constKey);
    constKey.enable = VIR_ENABLE_Y;
    vscHTBL_DirectRemove(movaHash, &constKey);
    constKey.enable = VIR_ENABLE_Z;
    vscHTBL_DirectRemove(movaHash, &constKey);
    constKey.enable = VIR_ENABLE_W;
    vscHTBL_DirectRemove(movaHash, &constKey);
}

static gctBOOL
_VSC_RA_EnableSingleChannel(
    IN VIR_Enable            enable
    )
{
    return (enable == VIR_ENABLE_X ||
            enable == VIR_ENABLE_Y ||
            enable == VIR_ENABLE_Z ||
            enable == VIR_ENABLE_W);
}

static VIR_RA_HWReg_Color
_VIR_RA_GetLRColor(
    VIR_RA_LS_Liverange *pLR)
{
    if (isLRSpilled(pLR))
    {
        return InvalidColor;
    }
    else
    {
        return pLR->u1.color;
    }
}

static void
_VIR_RA_SetLRColor(
    VIR_RA_LS_Liverange *pLR,
    VIR_HwRegId         regNo,
    gctUINT             shift)
{
    gcmASSERT(!isLRSpilled(pLR));

    pLR->u1.color._hwRegId = regNo;
    pLR->u1.color._hwShift = shift;
}

static void
_VIR_RA_SetLRColorHI(
    VIR_RA_LS_Liverange *pLR,
    VIR_HwRegId         regNo,
    gctUINT             shift)
{
    gcmASSERT(!isLRSpilled(pLR));

    pLR->u1.color._HIhwRegId = regNo;
    pLR->u1.color._HIhwShift = shift;
}

static gctUINT
_VIR_RA_GetLRSpillOffset(
    VIR_RA_LS_Liverange *pLR)
{
    gcmASSERT(isLRSpilled(pLR));

    return pLR->u1.spillOffset;
}

static void
_VIR_RA_SetLRSpillOffset(
    VIR_RA_LS_Liverange *pLR,
    gctUINT     spillOffset)
{
    gcmASSERT(isLRSpilled(pLR));

    pLR->u1.spillOffset = spillOffset;
}

static VIR_RA_HWReg_Color
_VIR_RA_GetLRTempColor(
    VIR_RA_LS_Liverange *pLR)
{
    gcmASSERT(isLRSpilled(pLR));

    return pLR->tempColor.color;
}

static void
_VIR_RA_SetLRTempColor(
    VIR_RA_LS_Liverange *pLR,
    VIR_HwRegId         regNo,
    gctUINT             shift,
    gctBOOL             bInit)
{
    gcmASSERT(bInit || isLRSpilled(pLR));

    pLR->tempColor.color._hwRegId = regNo;
    pLR->tempColor.color._hwShift = shift;
}

static void
_VIR_RA_SetLRTempColorHI(
    VIR_RA_LS_Liverange *pLR,
    VIR_HwRegId         regNo,
    gctUINT             shift,
    gctBOOL             bInit)
{
    gcmASSERT(bInit || isLRSpilled(pLR));

    pLR->tempColor.color._HIhwRegId = regNo;
    pLR->tempColor.color._HIhwShift = shift;
}

static gctUINT
_VIR_RA_GetLRTempColorEndPoint(
    VIR_RA_LS_Liverange *pLR)
{
    gcmASSERT(isLRSpilled(pLR));

    return pLR->tempColor.tempEndPoint;
}

static void
_VIR_RA_SetLRTempColorEndPoint(
    VIR_RA_LS_Liverange *pLR,
    gctUINT             endPoint,
    gctBOOL             bInit)
{
    gcmASSERT(bInit || isLRSpilled(pLR));

    pLR->tempColor.tempEndPoint = endPoint;
}

static void
_VIR_RA_InitLRTempColor(
    VIR_RA_LS_Liverange *pLR
    )
{
    _VIR_RA_SetLRTempColor(pLR, VIR_RA_INVALID_REG, 0, gcvTRUE);
    _VIR_RA_SetLRTempColorHI(pLR, VIR_RA_INVALID_REG, 0, gcvTRUE);
    _VIR_RA_SetLRTempColorEndPoint(pLR, VIR_INVALID_ID, gcvTRUE);
}

static VIR_HwRegId
_VIR_RA_Color_RegNo(
    VIR_RA_HWReg_Color  color)
{
    return color._hwRegId;
}

static gctUINT
_VIR_RA_Color_Shift(
    VIR_RA_HWReg_Color  color)
{
    return color._hwShift;
}

static VIR_HwRegId
_VIR_RA_Color_HIRegNo(
    VIR_RA_HWReg_Color  color)
{
    return color._HIhwRegId;
}

static gctUINT
_VIR_RA_Color_HIShift(
    VIR_RA_HWReg_Color  color)
{
    return color._HIhwShift;
}

static gctUINT
_VIR_RA_Color_Channels(
    VIR_Enable          channelMask,
    gctUINT             shift)
{
    gcmASSERT((channelMask << shift) <= 0xF);
    return channelMask << shift;
}

const char* _VIR_RA_Color_ChannelsName(
    gctUINT channels)
{
    return VIR_RA_ChannelsName[channels];
};

static void
_VIR_RA_MakeColor(
    VIR_HwRegId     hwRegNo,
    gctUINT         shift,
    VIR_RA_HWReg_Color *color)
{
    color->_hwRegId = hwRegNo;
    color->_hwShift = shift;

    /* Just make linux build happy */
    color->_reserved = 0;
}

static void
_VIR_RA_MakeColorHI(
    VIR_HwRegId     hwRegNo,
    gctUINT         shift,
    VIR_RA_HWReg_Color *color)
{
    color->_HIhwRegId = hwRegNo;
    color->_HIhwShift = shift;
}

static gctUINT
_VIR_RA_LS_GetDefPrecison(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    VIR_Shader  *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Symbol  *pSym = gcvNULL;


    VIR_DEF *pDef = GET_DEF_BY_IDX(&pRA->pLvInfo->pDuInfo->defTable, defIdx);
    if(pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
    {
        VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
        gctUINT                 currAttr;

        for (currAttr = 0; currAttr < VIR_IdList_Count(pAttrs); currAttr ++)
        {
            pSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, currAttr));
            if (pSym->u2.tempIndex == pDef->defKey.regNo)
            {
                break;
            }
        }
        gcmASSERT(pSym);
    }
    else
    {
        gcmASSERT(pDef->defKey.pDefInst->dest);
        pSym = VIR_Operand_GetSymbol(pDef->defKey.pDefInst->dest);
        gcmASSERT(pSym);
    }
    return VIR_Symbol_GetPrecision(pSym);
}

static gctUINT
_VIR_RA_LS_GetWebPrecison(
    VIR_RA_LS   *pRA,
    VIR_WEB     *pWeb)
{
    return _VIR_RA_LS_GetDefPrecison(pRA, pWeb->firstDefIdx);
}

/* define a hash key for search outputLRTable.
   we need useInst, since output can be used in different instructions.
   for example: t0 and t1 are output array(or matrix)'s temp
   t0 = 1; (LR1)
   t1 = 2; (LR2)
   emit;
   t0 = 3; (LR3)
   t1 = 4; (LR4)
   emit;
   these t0, t1, are in 4 different webs. we need to connect LR2 with LR1,
   (LR4 with LR3) to find its color based on useInst (emit).
   */
typedef struct _VIR_RA_OUTPUT_KEY
{
    gctUINT         masterRegNo;
    VIR_Instruction *useInst;
} VIR_RA_OutputKey;

static gctUINT _VIR_RA_OutputKey_HFUNC(const void* pKey)
{
    VIR_RA_OutputKey*     pOutputKey = (VIR_RA_OutputKey*)pKey;

    gctUINT hashVal = (((gctUINT)(gctUINTPTR_T) pOutputKey->useInst) << 4) ^
                          pOutputKey->masterRegNo ;

    return hashVal;
}

static gctBOOL _VIR_RA_OutputKey_HKCMP(const void* pHashKey1, const void* pHashKey2)
{
    return (((VIR_RA_OutputKey*)pHashKey1)->useInst == ((VIR_RA_OutputKey*)pHashKey2)->useInst)
        && (((VIR_RA_OutputKey*)pHashKey1)->masterRegNo == ((VIR_RA_OutputKey*)pHashKey2)->masterRegNo);
}

static VIR_RA_OutputKey* _VIR_RA_NewOutputKey(
    VIR_RA_LS       *pRA,
    gctUINT         masterRegNo,
    VIR_Instruction *pInst
    )
{
    VIR_RA_OutputKey* outputKey = (VIR_RA_OutputKey*)vscMM_Alloc(VIR_RA_LS_GetMM(pRA), sizeof(VIR_RA_OutputKey));
    outputKey->masterRegNo = masterRegNo;
    outputKey->useInst = pInst;
    return outputKey;
}

/* get the webIdx based on defIdx */
gctUINT _VIR_RA_LS_Def2Web(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_DEF             *pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
    gcmASSERT(pDef);

    return pDef->webIdx;
}

/* get the webIdx based on useIdx */
gctUINT _VIR_RA_LS_Use2Web(
    VIR_RA_LS   *pRA,
    gctUINT     useIdx)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_USAGE           *pUse = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, useIdx);
    gcmASSERT(pUse);

    return pUse->webIdx;
}

/* get the LR based on webIdx */
VIR_RA_LS_Liverange* _VIR_RA_LS_Web2LR(
    VIR_RA_LS       *pRA,
    gctUINT         webIdx)
{
    VIR_RA_LS_Liverange     *pLR = gcvNULL;
    gcmASSERT (VIR_INVALID_WEB_INDEX != webIdx);
    pLR = (VIR_RA_LS_Liverange*) vscSRARR_GetElement(
                                        VIR_RA_LS_GetLRTable(pRA),
                                        webIdx);
    return pLR;
}

/* get the LR based on webIdx,
   if the LR is invalid, get its master's LR*/
VIR_RA_LS_Liverange* _VIR_RA_LS_Web2ColorLR(
    VIR_RA_LS       *pRA,
    gctUINT         webIdx)
{
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

    /* if this LR has not assigned color (i.e. invalid LR),
       find the color in its master LR masterWebIdx */
    if (isLRInvalid(pLR))
    {
        gcmASSERT(VIR_INVALID_WEB_INDEX != pLR->masterWebIdx);
        pLR = (VIR_RA_LS_Liverange*) vscSRARR_GetElement(
                                        VIR_RA_LS_GetLRTable(pRA),
                                        pLR->masterWebIdx);
    }

    return pLR;
}

VIR_RA_LS_Liverange* _VIR_RA_LS_Def2LR(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    gctUINT                 webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    gcmASSERT (VIR_INVALID_WEB_INDEX != webIdx);

    return _VIR_RA_LS_Web2LR(pRA, webIdx);
}

VIR_RA_LS_Liverange* _VIR_RA_LS_Def2ColorLR(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    gctUINT                 webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    gcmASSERT (VIR_INVALID_WEB_INDEX != webIdx);

    return _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
}

VIR_RA_LS_Liverange* _VIR_RA_LS_Use2LR(
    VIR_RA_LS       *pRA,
    gctUINT         useIdx)
{
    gctUINT                 webIdx = _VIR_RA_LS_Use2Web(pRA, useIdx);
    gcmASSERT (VIR_INVALID_WEB_INDEX != webIdx);

    return _VIR_RA_LS_Web2LR(pRA, webIdx);
}

gctUINT _VIR_RA_LS_SrcOpnd2WebIdx(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    gctUINT         useIdx = VIR_INVALID_USAGE_INDEX;
    VIR_OperandInfo operandInfo;
    VIR_USAGE_KEY   useKey;
    VIR_USAGE       *pUsage;

    gcmASSERT(pOpnd);

    VIR_Operand_GetOperandInfo(
        pInst,
        pOpnd,
        &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        useKey.pUsageInst = pInst;
        useKey.pOperand = pOpnd;
        useKey.bIsIndexingRegUsage = gcvFALSE;
        useIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &useKey);
        if (useIdx != VIR_INVALID_USAGE_INDEX)
        {
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, useIdx);
            return pUsage->webIdx;
        }
    }

    return VIR_INVALID_WEB_INDEX;
}

static gctBOOL
_VIR_RA_LS_OperandEvenReg(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd)
{
    gctUINT             webIdx;
    VIR_RA_LS_Liverange *pOrigLR;

    webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pOpnd);
    if (VIR_INVALID_WEB_INDEX != webIdx)
    {
        pOrigLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        if (isLRVXEven(pOrigLR))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_VIR_RA_LS_OperandOddReg(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd)
{
    gctUINT             webIdx;
    VIR_RA_LS_Liverange *pOrigLR;

    webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pOpnd);
    if (VIR_INVALID_WEB_INDEX != webIdx)
    {
        pOrigLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        if (isLRVXOdd(pOrigLR))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL _VIR_RA_LS_InstNeedEvenOddReg(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    /* Check temp 256 register pair. */
    if (VIR_OPCODE_SrcsTemp256(VIR_Inst_GetOpcode(pInst)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctUINT _VIR_RA_LS_InstFirstDefIdx(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_OperandInfo     operandInfo;
    VIR_DEF_KEY         defKey;
    gctUINT             defIdx = VIR_INVALID_DEF_INDEX;
    VIR_Operand         *pOpnd = pInst->dest;
    gcmASSERT(pOpnd);

    VIR_Operand_GetOperandInfo(
        pInst,
        pOpnd,
        &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        defKey.pDefInst = pInst;
        defKey.regNo = operandInfo.u1.virRegInfo.virReg;
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
        gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
    }

    return defIdx;
}

void VIR_RA_LS_SetCurrPos(
    VIR_RA_LS           *pRA,
    gctINT             s)
{
    gcmASSERT (s >= 0);
    pRA->currPos = s;
}

void _VIR_RA_LS_ClearChannelMask(
    VIR_RA_LS           *pRA)
{
    VIR_RA_LS_Liverange *pLR;
    gctUINT i;

    for (i = 0; i < VIR_RA_LS_GetNumWeb(pRA); i++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, i);
        pLR->channelMask = 0;
    }
}

/* get the channel mask from VIR_WEB */
VIR_Enable VIR_RA_LS_LR2WebChannelMask(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             webIdx = pLR->webIdx;
    VIR_WEB             *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    VIR_Enable          retEnable = VIR_ENABLE_NONE;
    gcmASSERT(pWeb);

    if (isLRAtomCmpXchg(pLR))
    {
        /* v60 USC requires that ATOM_CMP_XCHG has at least two channel enabled.
           (USC uses temp as buffer).
           It does not matter what is the other channel, since that channel is
           only used by HW, not the shader. */
        switch (pWeb->channelMask)
        {
        case 1:
        case 2:
        case 3:
            retEnable = VIR_ENABLE_XY;
            break;
        case 4:
        case 8:
        case 12:
            retEnable = VIR_ENABLE_ZW;
            break;
        default:
            retEnable = VIR_ENABLE_XYZW;
        }
    }
    else
    {
        retEnable = (VIR_Enable) pWeb->channelMask;
    }

    return retEnable;
}

/* ===========================================================================
   functions to initialize (color pool, ...)
   ===========================================================================
*/
void _VIR_RA_FlaseDepReg_Init(
    VIR_RA_LS           *pRA,
    VSC_MM              *pMM,
    gctINT              regCount)
{
    vscBV_Initialize(VIR_RA_LS_GetFalseDepRegVec(pRA), pMM, regCount);
}

void _VIR_RA_FlaseDepReg_Finalize(
    VIR_RA_LS           *pRA)
{
    vscBV_Finalize(VIR_RA_LS_GetFalseDepRegVec(pRA));
}

void _VIR_RA_FlaseDepReg_ClearAll(
    VIR_RA_LS           *pRA)
{
    vscBV_ClearAll(VIR_RA_LS_GetFalseDepRegVec(pRA));
}

void _VIR_RA_FlaseDepReg_Set(
    VIR_RA_LS           *pRA,
    gctINT              regNo)
{
    vscBV_SetBit(VIR_RA_LS_GetFalseDepRegVec(pRA), regNo);
}

void _VIR_RA_FlaseDepReg_Clear(
    VIR_RA_LS           *pRA,
    gctINT              regNo)
{
    vscBV_ClearBit(VIR_RA_LS_GetFalseDepRegVec(pRA), regNo);
}

gctBOOL _VIR_RA_FlaseDepReg_Test(
    VIR_RA_LS           *pRA,
    gctINT              regNo)
{
    return vscBV_TestBit(VIR_RA_LS_GetFalseDepRegVec(pRA), regNo);
}

gctUINT _VIR_RA_GetMaxRegCount(
    VIR_RA_LS           *pRA,
    VSC_HW_CONFIG       *pHwCfg,
    VIR_RA_HWReg_Type   hwRegType)
{
    gctUINT             regCount = 0;
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);

    switch (hwRegType)
    {
    case VIR_RA_HWREG_GR:
        regCount = pHwCfg->maxGPRCountPerThread;

        /* VS/TCS/TES/GS for v6.0 to at most 61 temps*/
        if ((VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5) &&
            (pShader->shaderKind == VIR_SHADER_VERTEX ||
             pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
             pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION ||
             pShader->shaderKind == VIR_SHADER_GEOMETRY))
        {
            regCount -= 3;
        }

        if (VSC_OPTN_InRange(VIR_Shader_GetId(pShader),
                              VSC_OPTN_RAOptions_GetSpillBeforeShader(pOption),
                              VSC_OPTN_RAOptions_GetSpillAfterShader(pOption)))
        {
            if (VSC_OPTN_RAOptions_GetRegisterCount(pOption) > 0)
            {
                /* set the maxReg from the command line, should not larger than maxGPRCountPerThread */
                if (VSC_OPTN_RAOptions_GetRegisterCount(pOption) < pHwCfg->maxGPRCountPerThread)
                {
                    regCount = VSC_OPTN_RAOptions_GetRegisterCount(pOption);
                }
            }
        }
        break;

    case VIR_RA_HWREG_A0:
        regCount = 1;
        break;

    case VIR_RA_HWREG_B0:
        regCount = 1;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return regCount;
}

void _VIR_RA_ColorMap_Init(
    VIR_RA_LS           *pRA,
    VIR_RA_ColorMap     *pCM,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_MM              *pMM,
    VIR_RA_HWReg_Type   hwRegType)
{
    pCM->maxReg = _VIR_RA_GetMaxRegCount(pRA, pHwCfg, hwRegType);
    pCM->maxAllocReg = 0;
    pCM->availReg = 0;

    /* each register needs 4 channels (w, z, y, x) */
    vscBV_Initialize(&pCM->usedColor, pMM,
        pCM->maxReg * VIR_CHANNEL_NUM);

    if (hwRegType == VIR_RA_HWREG_GR)
    {
        _VIR_RA_FlaseDepReg_Init(pRA, pMM, pCM->maxReg);
    }
}

void _VIR_RA_ColorPool_Init(
    VIR_RA_LS           *pRA,
    VIR_RA_ColorPool    *pCP,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_MM              *pMM)
{
    /* initialize each hw register coloring map */
    gctUINT i;
    for (i = 0; i < VIR_RA_HWREG_TYPE_COUNT; i++)
    {
        _VIR_RA_ColorMap_Init(pRA, &pCP->colorMap[i], pHwCfg, pMM, i);
    }
}

void _VIR_RA_ColorPool_ClearAll(
    VIR_RA_ColorPool    *pCP)
{
    /* clear each hw register coloring map */
    gctUINT i;
    for (i = 0; i < VIR_RA_HWREG_TYPE_COUNT; i++)
    {
        vscBV_ClearAll(&pCP->colorMap[i].usedColor);
    }
}

void _VIR_RA_LRTable_ClearColor(
    VIR_RA_LS           *pRA)
{
    gctUINT             i;
    VIR_RA_LS_Liverange *pLR;

    for (i = 0; i < VIR_RA_LS_GetNumWeb(pRA); i++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, i);

        if (isLRSpilled(pLR))
        {
            _VIR_RA_InitLRTempColor(pLR);
        }

        if (pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
        {
            if (isLRSpilled(pLR))
            {
                VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_SPILLED);
            }

            _VIR_RA_SetLRColor(pLR, VIR_RA_INVALID_REG, 0);
            _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
        }

        /* We need to recompute the minDefPos of each LR if needed. */
        if (VIR_RA_LS_GetInstIdChanged(pRA))
        {
            VIR_DEF              *pDef;
            VIR_Instruction      *pDefInst;
            VIR_WEB              *pWeb;
            gctUINT               defIdx;

            pLR->minDefPos = VIR_RA_LS_POS_MAX;

            pWeb = GET_WEB_BY_IDX(&pRA->pLvInfo->pDuInfo->webTable, i);
            /* find all defs */
            defIdx = pWeb->firstDefIdx;
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pRA->pLvInfo->pDuInfo->defTable, defIdx);
                pDefInst = pDef->defKey.pDefInst;
                if (!VIR_IS_IMPLICIT_DEF_INST(pDefInst) && (pLR->minDefPos > (gctUINT)VIR_Inst_GetId(pDefInst)))
                {
                    pLR->minDefPos = (gctUINT)VIR_Inst_GetId(pDefInst); /* find early define pos of current webIdx */
                }
                defIdx = pDef->nextDefInWebIdx;
            }
        }
    }
}

void _VIR_RA_LS_InitLRFunc(
    VIR_RA_LS_Liverange     *pLR,
    gctUINT                  index)
{
    /* changes in function */
    pLR->liveFunc = gcvNULL;

    pLR->startPoint = 0;
    pLR->endPoint = 0;
    pLR->origEndPoint = 0;
    pLR->deadIntervals = 0;

    pLR->nextLR = gcvNULL;
    pLR->nextActiveLR = gcvNULL;

    pLR->deadIntervalAvail = gcvTRUE;
    pLR->usedColorLR = gcvNULL;

    pLR->weight = VIR_RA_LS_MAX_WEIGHT;

    pLR->currDef = VIR_RA_LS_POS_MAX;

    /* Clear LD/ST dep. */
    pLR->pLDSTInst  = gcvNULL;
    VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_LD_DEP);
    VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_ST_DEP);
}

void _VIR_RA_LS_InitLRShader(
    VIR_RA_LS_Liverange     *pLR,
    gctUINT                  index)
{
    _VIR_RA_LS_InitLRFunc(pLR, index);

    /* has one value in the shader */
    pLR->webIdx = index;
    pLR->flags = VIR_RA_LRFLAG_NONE;
    pLR->masterWebIdx = VIR_INVALID_WEB_INDEX;
    pLR->hwType = VIR_RA_HWREG_GR;
    pLR->channelMask = 0;
    pLR->firstRegNo = VIR_RA_LS_REG_MAX;
    pLR->regNoRange = 1;
    pLR->colorFunc = gcvNULL;
    pLR->minDefPos = VIR_RA_LS_POS_MAX;
    _VIR_RA_SetLRColor(pLR, VIR_RA_INVALID_REG, 0);
    _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
    _VIR_RA_InitLRTempColor(pLR);
}

void _VIR_RA_LS_InitLRTable(
    VIR_RA_LS           *pRA)
{
    gctUINT             i;
    VIR_RA_LS_Liverange *pLR;

    /* initialize every LR in LRTable */
    for (i = 0; i < VIR_RA_LS_GetNumWeb(pRA); i++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, i);
        _VIR_RA_LS_InitLRFunc(pLR, i);
    }

    /* initialize the sorted LR list head
       sortedLRHead is used in sorting LRs
       sorted LR list is sorted in the increasing order of startPoint */
    pLR = VIR_RA_LS_GetSortedLRHead(pRA);
    pLR->startPoint = VIR_RA_LS_POS_MAX;
    pLR->nextLR = &(LREndMark);

    /* initialize the active LR list head
       activeLRHead is used in assigning colors
       active LR list is sorted in the increasing order of endPoint */
    pLR = VIR_RA_LS_GetActiveLRHead(pRA);
    pLR->endPoint = VIR_RA_LS_POS_MIN;
    pLR->nextActiveLR = &(LREndMark);
}

/* ===========================================================================
   _VIR_RA_Check_SpecialFlags
   check for sampleDepth/Id/Pos/Mask and set related flags
   ===========================================================================
*/
static void _VIR_RA_Check_SpecialFlags(VIR_RA_LS  *pRA)
{
    VIR_RA_LS_Liverange     *pLR;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    VIR_AttributeIdList     *pOutputs = VIR_Shader_GetOutputs(pShader);
    gctUINT                 currAttr,currOutput;
    VIR_DEF_KEY             defKey;
    VIR_USAGE_KEY           usageKey;
    gctUINT                 defIdx, usageIdx, webIdx;
    gctUINT8                currChannel;

    for (currAttr = 0; currAttr < VIR_IdList_Count(pAttrs); currAttr ++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, currAttr));

        if (VIR_Symbol_GetName(attribute) == VIR_NAME_SUBSAMPLE_DEPTH)
        {
            defKey.pDefInst = VIR_INPUT_DEF_INST;
            defKey.regNo = VIR_Symbol_GetVariableVregIndex(attribute);

            for (currChannel = 0; currChannel < VIR_CHANNEL_NUM; currChannel++)
            {
                defKey.channel = currChannel;

                defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
                if (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
                    pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
                    VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SUB_SAMPLE_DEPTH);
                    VIR_RA_LS_SetFlag(pRA, VIR_RA_LS_FLAG_HAS_SUB_SAMPLE_DEPTH_LR);
                    break;
                }
            }

            /* if sampleDepth is found, the last register should be sampleDepth */
            break;
        }

        if (VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_ID ||
            VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_POSITION ||
            VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_MASK_IN)
        {
            /* sampleId/Pos/Mask could be in r0 if available or sampleDepth */
            VIR_Shader_SetFlag(pShader, VIR_SHFLAG_PS_NEED_SAMPLE_MASK_ID);
        }
    }

    for (currOutput = 0; currOutput < VIR_IdList_Count(pOutputs); currOutput ++)
    {
        VIR_Symbol  *output = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputs, currOutput));

        if (VIR_Symbol_GetName(output) == VIR_NAME_SUBSAMPLE_DEPTH)
        {
            usageKey.pUsageInst = VIR_OUTPUT_USAGE_INST;
            usageKey.pOperand = (VIR_Operand*)(gctUINTPTR_T)VIR_Symbol_GetVariableVregIndex(output);
            usageKey.bIsIndexingRegUsage = gcvFALSE;

            usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                webIdx = _VIR_RA_LS_Use2Web(pRA, usageIdx);
                pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
                VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SUB_SAMPLE_DEPTH);
                VIR_RA_LS_SetFlag(pRA, VIR_RA_LS_FLAG_HAS_SUB_SAMPLE_DEPTH_LR);
            }

            break;
        }

        if (VIR_Symbol_GetName(output) == VIR_NAME_SAMPLE_MASK)
        {
            VIR_Shader_SetFlag(pShader, VIR_SHFLAG_PS_NEED_SAMPLE_MASK_ID);
        }
    }
}

static gctBOOL _VIR_RA_isShaderNeedSampleDepth(
    VIR_RA_LS  *pRA)
{
    VIR_Shader   *pShader = VIR_RA_LS_GetShader(pRA);

    /* sampleMask/Id/Pos need sampleDepth */
    if (pShader->shaderKind == VIR_SHADER_FRAGMENT &&
        pShader->sampleMaskIdRegStart == VIR_REG_MULTISAMPLEDEPTH &&
        VIR_Shader_PS_NeedSampleMaskId(pShader))
    {
        return gcvTRUE;
    }

    if (isRALSSubSampleDepth(pRA))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* return true if instruction requires no shift for its def */
static gctBOOL
_VIR_RA_LS_IsRestrictInst(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    VIR_Shader  *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(pInst);

    if (opcode == VIR_OP_MOD            ||
        opcode == VIR_OP_REM            ||
        opcode == VIR_OP_LOAD_ATTR      ||
        opcode == VIR_OP_LOAD_ATTR_O    ||
        opcode == VIR_OP_ATTR_LD        ||
        opcode == VIR_OP_IMG_ADDR       ||
        opcode == VIR_OP_IMG_ADDR_3D    ||
        opcode == VIR_OP_IMG_LOAD       ||
        opcode == VIR_OP_VX_IMG_LOAD    ||
        opcode == VIR_OP_IMG_LOAD_3D    ||
        opcode == VIR_OP_VX_IMG_LOAD_3D ||
        opcode == VIR_OP_ARCTRIG        ||
        opcode == VIR_OP_DP2            ||
        VIR_OPCODE_isCmplx(opcode)      ||
        VIR_OPCODE_isTexLd(opcode)      ||
        VIR_OPCODE_isAtom(opcode)       ||
        VIR_OPCODE_isVX(opcode)     ||
        opcode == VIR_OP_SWIZZLE)
    {
        return gcvTRUE;
    }

    if (opcode == VIR_OP_DIV)
    {
        VIR_Operand *pOpnd = VIR_Inst_GetDest(pInst);
        VIR_Type    *type = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(pOpnd));
        VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(type);
        if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* a0/b0 is special allocated in RA */
static gctBOOL
_VIR_RA_InstDestIsA0B0(
    VIR_Instruction      *pInst)
{
    if (pInst && (VIR_Inst_GetOpcode(pInst) == VIR_OP_MOVA ||
                  VIR_Inst_GetOpcode(pInst) == VIR_OP_MOVBIX))
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/* set LR to be restrict (e.g, non-component wise instruction, or
   output temp) */
void _VIR_RA_LS_SetRestrictLR(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    gcmASSERT(pLR);
    VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_NO_SHIFT);
}

static void _VIR_RA_LS_Init(
    VIR_RA_LS           *pRA,
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VIR_LIVENESS_INFO   *pLvInfo,
    VSC_OPTN_RAOptions  *pOptions,
    VIR_Dumper          *pDumper,
    VIR_CALL_GRAPH      *pCG,
    VSC_MM              *pMM)
{
    gctUINT     numWeb = BT_GET_MAX_VALID_ID(&pLvInfo->pDuInfo->webTable);
    gctUINT     numDef = BT_GET_MAX_VALID_ID(&pLvInfo->pDuInfo->defTable);

    VIR_RA_LS_Liverange *pLR;
    gctUINT             i;

    VIR_RA_LS_SetShader(pRA, pShader);
    VIR_RA_LS_SetHwCfg(pRA, pHwCfg);
    VIR_RA_LS_SetLvInfo(pRA, pLvInfo);
    VIR_RA_LS_SetDumper(pRA, pDumper);
    VIR_RA_LS_SetOptions(pRA, pOptions);
    VIR_RA_LS_SetCallGraph(pRA, pCG);

    VIR_RA_LS_SetNumWeb(pRA, numWeb);
    VIR_RA_LS_SetNumDef(pRA, numDef);

    VIR_RA_LS_SetFlags(pRA, VIR_RA_LS_FLAG_NONE);

    /* initialize the memory pool */
    pRA->pMM = pMM;

    /* color pool initialization */
    _VIR_RA_ColorPool_Init(
        pRA,
        VIR_RA_LS_GetColorPool(pRA),
        pHwCfg,
        VIR_RA_LS_GetMM(pRA));

    /* allocate the live LR vector */
    vscBV_Initialize(
        VIR_RA_LS_GetLiveLRVec(pRA),
        VIR_RA_LS_GetMM(pRA),
        numDef);

    /* allocate the live range table */
    vscSRARR_Initialize(
        VIR_RA_LS_GetLRTable(pRA),
        VIR_RA_LS_GetMM(pRA),
        numWeb,
        sizeof(VIR_RA_LS_Liverange),
        gcvNULL);
    vscSRARR_SetElementCount(VIR_RA_LS_GetLRTable(pRA), numWeb);

    /* initialize webIdx/color/channelmask for each LR in LRTable
       other parts of LR will be initialized inside
       _VIR_RA_LS_InitLRTable that is called for each function */
    for (i = 0; i < VIR_RA_LS_GetNumWeb(pRA); i++)
    {
        VIR_DEF              *pDef;
        VIR_Instruction      *pDefInst;
        VIR_WEB              *pWeb;
        gctUINT               defIdx;
        pLR = _VIR_RA_LS_Web2LR(pRA, i);
        _VIR_RA_LS_InitLRShader(pLR, i);
        pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, i);
        /* find all defs */
        defIdx = pWeb->firstDefIdx;
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
            pDefInst = pDef->defKey.pDefInst;
            /* set restricted flag according to define instruction, so even the
             * LR is node defined out of usage function, we can still can get
             * the correct no-shift restriction
             */
            if (!VIR_IS_IMPLICIT_DEF_INST(pDefInst) && _VIR_RA_LS_IsRestrictInst(pRA, pDefInst) )
            {
                _VIR_RA_LS_SetRestrictLR(pRA, defIdx);
                break;
            }
            defIdx = pDef->nextDefInWebIdx;
        }
        /* compute the minDefPos of each LR */
        defIdx = pWeb->firstDefIdx;
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
            pDefInst = pDef->defKey.pDefInst;
            if (!VIR_IS_IMPLICIT_DEF_INST(pDefInst) && (pLR->minDefPos > (gctUINT)VIR_Inst_GetId(pDefInst)))
            {
                pLR->minDefPos = (gctUINT)VIR_Inst_GetId(pDefInst); /* find early define pos of current webIdx */
            }
            /* check symbol of destOper is highpvec2 and set flag */
            if (VIR_Shader_isDual16Mode(pShader) && !isLRHighpVec2(pLR) &&
                (pDefInst != VIR_INPUT_DEF_INST) &&
                (pDefInst != VIR_HW_SPECIAL_DEF_INST) &&
                (!VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(pDefInst))) && /* texld should not use same registers for dual16 */
                (!_VIR_RA_InstDestIsA0B0(pDefInst)))
            {
                VIR_Operand *dstOper = VIR_Inst_GetDest(pDefInst);
                if (dstOper)
                {
                    VIR_Enable  dstEnable = VIR_Operand_GetEnable(dstOper);
                    VIR_Symbol  *dstSym = VIR_Operand_GetSymbol(dstOper);
                    VIR_Symbol  *rSym = VIR_Operand_GetUnderlyingSymbol(dstOper);
                    if (dstSym && VIR_Symbol_IsHighpVec2(dstSym) &&
                        (dstEnable <= 0x3) &&    /* symbol type and enable bits may not consistent in some temp variable */
                        (!VIR_Symbol_SpecialRegAlloc(dstSym)) &&
                        (!VIR_Symbol_isInputOrOutput(dstSym) && (!rSym || !VIR_Symbol_isInputOrOutput(rSym)))) /* now hardware doesn't support highp input/output variable in same register*/
                    {
                        VIR_RA_LR_SetFlag(pLR, VIR_RA_LRHIGHPVEC2);
                    }
                }
            }
            defIdx = pDef->nextDefInWebIdx;
        }
    }

    /* allocate the LR sorted list head
       the initialization is inside _VIR_RA_LS_InitLRTable that is
       called for each function */
    pLR = (VIR_RA_LS_Liverange*)vscMM_Alloc(
                VIR_RA_LS_GetMM(pRA), sizeof(VIR_RA_LS_Liverange));
    VIR_RA_LS_SetSortedLRHead(pRA, pLR);

    /* allocate the LR active list head
       the initialization is inside _VIR_RA_LS_InitLRTable that is
       called for each function */
    pLR = (VIR_RA_LS_Liverange*)vscMM_Alloc(
                VIR_RA_LS_GetMM(pRA), sizeof(VIR_RA_LS_Liverange));
    VIR_RA_LS_SetActiveLRHead(pRA, pLR);

    /* allocate for outputLRTable */
    pRA->outputLRTable = vscHTBL_Create(VIR_RA_LS_GetMM(pRA),
                            _VIR_RA_OutputKey_HFUNC, _VIR_RA_OutputKey_HKCMP, 512);

    _VIR_RA_Check_SpecialFlags(pRA);

    /* make sure that liveness is valid */
    gcmASSERT(vscVIR_CheckDFAFlowBuilt(&pLvInfo->baseTsDFA.baseDFA) &&
              pLvInfo->pDuInfo->bWebTableBuilt);

    pRA->resRegister = VIR_INVALID_ID;

    /* for register spills */
    /* the offset where the next spill should be */
    pRA->spillOffset = (pShader->vidmemSizeOfSpill + 15) & ~ (gctUINT)0x0F;
    /* reserved HW register for base address, offset, or threadIndex
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
    pRA->baseRegister = VIR_INVALID_ID;

    pRA->bReservedMovaReg = gcvFALSE;

    if (pRA->bReservedMovaReg)
    {
        pRA->movaRegCount = 2;

        pRA->movaHash = vscHTBL_Create(VIR_RA_LS_GetMM(pRA),
                                       _HFUNC_RA_MOVA_CONSTKEY, _HKCMP_RA_MOVA_CONSTKEY, 64);
    }
    else
    {
        pRA->movaRegCount = 0;
    }

    for (i = 0; i < VIR_RA_LS_MAX_MOVA_REG; i++)
    {
        pRA->movaRegister[i] = VIR_INVALID_ID;
    }

    /* reserved HW register for data, we may need to reserve more than one
       registers to save the data, since in some instruction, maybe more than
       one src is spilled */
    pRA->resDataRegisterCount = 0;
    pRA->dataRegisterUsedMaskBit = 0;
    for (i = 0; i < VIR_RA_LS_DATA_REG_NUM; i++)
    {
        pRA->dataRegister[i] = VIR_INVALID_ID;

        pRA->dataRegisterEndPoint[i] = VIR_INVALID_ID;
        pRA->dataSymId[i] = VIR_INVALID_ID;
    }
    pRA->baseAddrSymId  = VIR_INVALID_ID;
    pRA->spillOffsetSymId  = VIR_INVALID_ID;
    pRA->threadIdxSymId  = VIR_INVALID_ID;

    pRA->samplePosRegister = VIR_INVALID_ID;

    pRA->currentMaxGRCount = VIR_RA_LS_REG_MAX;

    pRA->DIContext = gcvNULL;
    VIR_RA_LS_SetExtendLSEndPoint(pRA, gcvFALSE);
    VIR_RA_LS_SetInstIdChanged(pRA, gcvFALSE);
    VIR_RA_LS_SetEnableDebug(pRA, gcvFALSE);
    VIR_RA_LS_SetDisableDual16AndRecolor(pRA, gcvFALSE);
}

static VSC_ErrCode
_VIR_RA_LS_InvalidDataRegisterUsedMask(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst
    )
{
    gctUINT             i;

    for (i = 0; i < VIR_RA_LS_DATA_REG_NUM; i++)
    {
        if (pRA->dataRegisterEndPoint[i] == VIR_INVALID_ID || (gctUINT)VIR_Inst_GetId(pInst) > pRA->dataRegisterEndPoint[i])
        {
            pRA->dataRegisterUsedMaskBit &= (~(1 << i));

            if ((gctUINT)VIR_Inst_GetId(pInst) > pRA->dataRegisterEndPoint[i])
            {
                pRA->dataRegisterEndPoint[i] = VIR_INVALID_ID;
            }
        }
    }

    return VSC_ERR_NONE;
}

/* ===========================================================================
   functions to finalize (color pool, ...)
   ===========================================================================
*/
void VIR_RA_ColorPool_Finalize(
    VIR_RA_ColorPool    *pCP)
{
    gctUINT i;
    for (i = 0; i < VIR_RA_HWREG_TYPE_COUNT; i++)
    {
        vscBV_Finalize(&pCP->colorMap[i].usedColor);
    }
}

static void _VIR_RA_LS_Final(
    VIR_RA_LS       *pRA)
{
    VIR_RA_LS_SetShader(pRA, gcvNULL);
    VIR_RA_LS_SetOptions(pRA, gcvNULL);
    VIR_RA_LS_SetDumper(pRA, gcvNULL);
    VIR_RA_ColorPool_Finalize(VIR_RA_LS_GetColorPool(pRA));
    _VIR_RA_FlaseDepReg_Finalize(pRA);

    if (pRA->bReservedMovaReg)
    {
        vscHTBL_Destroy(pRA->movaHash);
    }
}

/* ===========================================================================
   dump functions
   ===========================================================================
*/
static gctBOOL _VIR_RA_LS_IsInvalidLOWColor(
    VIR_RA_HWReg_Color color)
{
    return (color._hwRegId == VIR_RA_INVALID_REG);
}

static gctBOOL _VIR_RA_LS_IsInvalidHIColor(
    VIR_RA_HWReg_Color color)
{
    return (color._HIhwRegId == VIR_RA_INVALID_REG);
}

static gctBOOL _VIR_RA_LS_IsInvalidColor(
    VIR_RA_HWReg_Color color)
{
    return (_VIR_RA_LS_IsInvalidLOWColor(color) &&
            _VIR_RA_LS_IsInvalidHIColor(color));
}

static gctBOOL _VIR_RA_LS_IsTwoSameColor(
    VIR_RA_HWReg_Color color1,
    VIR_RA_HWReg_Color color2)
{
    if ((color1._hwRegId == color2._hwRegId)        &&
        (color1._hwShift == color2._hwShift)        &&
        (color1._HIhwRegId == color2._HIhwRegId)    &&
        (color1._HIhwShift == color2._HIhwShift)    &&
        (color1._reserved == color2._reserved))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

void _VIR_RA_LS_DumpColor(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Color  color,
    VIR_RA_LS_Liverange *pLR)
{
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VIR_RA_HWReg_Type   hwType = pLR->hwType;
    gctUINT             regNoRange = pLR->regNoRange, highDiff = 0;

    if (isLRSpilled(pLR))
    {
        VIR_LOG(pDumper, "color:[spilled %d]", _VIR_RA_GetLRSpillOffset(pLR));
    }
    else if (_VIR_RA_LS_IsInvalidColor(color))
    {
        VIR_LOG(pDumper, "color:[invalid]");
    }
    else if (_VIR_RA_LS_IsInvalidHIColor(color))
    {
        /* dump the low register info */
        switch(hwType)
        {
        case VIR_RA_HWREG_GR:
            if (_VIR_RA_Color_RegNo(color) == VIR_SR_INSTATNCEID)
            {
                VIR_LOG(pDumper, "color:[InstanceId.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else if (_VIR_RA_Color_RegNo(color) == VIR_SR_VERTEXID)
            {
                VIR_LOG(pDumper, "color:[VertexId.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else if (_VIR_RA_Color_RegNo(color) == VIR_REG_MULTISAMPLEDEPTH)
            {
                VIR_LOG(pDumper, "color:[subsampledepth.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else if (_VIR_RA_Color_RegNo(color) == VIR_REG_SAMPLE_POS)
            {
                VIR_LOG(pDumper, "color:[samplepos.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else if (_VIR_RA_Color_RegNo(color) == VIR_REG_SAMPLE_ID)
            {
                VIR_LOG(pDumper, "color:[sampleid.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else if (_VIR_RA_Color_RegNo(color) == VIR_REG_SAMPLE_MASK_IN)
            {
                VIR_LOG(pDumper, "color:[samplemaskin.%s]",
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(color))));
            }
            else
            {
                if (regNoRange > 1)
                {
                    VIR_LOG(pDumper, "color:[r%d-%d.%s]",
                        _VIR_RA_Color_RegNo(color), _VIR_RA_Color_RegNo(color) + regNoRange - 1,
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))));
                }
                else
                {
                    VIR_LOG(pDumper, "color:[r%d.%s]",
                        _VIR_RA_Color_RegNo(color),
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))));
                }
            }
            break;
        case VIR_RA_HWREG_A0:
                VIR_LOG(pDumper, "color:[a%d.%s]",
                    _VIR_RA_Color_RegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))));
            break;
        case VIR_RA_HWREG_B0:
                VIR_LOG(pDumper, "color:[b%d.%s]",
                    _VIR_RA_Color_RegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))));
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else
    {
        /* dump the register pair info */
        switch(hwType)
        {
        case VIR_RA_HWREG_GR:
            {
                if (regNoRange > 1)
                {
                    /* array highp case */
                    highDiff = _VIR_RA_Color_HIRegNo(color) - _VIR_RA_Color_RegNo(color);
                    VIR_LOG(pDumper, "color:[r%d-%d.%s,r%d-%d.%s]",
                        _VIR_RA_Color_RegNo(color), _VIR_RA_Color_RegNo(color) + (regNoRange - 1) * (highDiff + 1),
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))),
                        _VIR_RA_Color_HIRegNo(color), _VIR_RA_Color_HIRegNo(color) + (regNoRange - 1) * (highDiff + 1),
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(color))));
                }
                else
                {
                    VIR_LOG(pDumper, "color:[r%d.%s, r%d.%s]",
                        _VIR_RA_Color_RegNo(color),
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))),
                        _VIR_RA_Color_HIRegNo(color),
                        _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(color))));
                }
            }
            break;
        case VIR_RA_HWREG_A0:
            VIR_LOG(pDumper, "color:[a%d.%s, a%d.%s]",
                    _VIR_RA_Color_RegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))),
                    _VIR_RA_Color_HIRegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(color))));
            break;
        case VIR_RA_HWREG_B0:
            VIR_LOG(pDumper, "color:[b%d.%s, b%d.%s]",
                    _VIR_RA_Color_RegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_Shift(color))),
                    _VIR_RA_Color_HIRegNo(color),
                    _VIR_RA_Color_ChannelsName(_VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(color))));
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
}

gctUINT _VIR_RS_LS_GetWebRegNo(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR)
{
    if (pLR->firstRegNo != VIR_RA_LS_REG_MAX)
    {
        return pLR->firstRegNo;
    }
    else
    {
        VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
        VIR_WEB *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, pLR->webIdx);
        VIR_DEF *pDef = gcvNULL;
        if (pWeb->firstDefIdx != VIR_INVALID_DEF_INDEX)
        {
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, pWeb->firstDefIdx);
            gcmASSERT(pDef);
            return pDef->defKey.regNo;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    return VIR_RA_LS_REG_MAX;
}

void _VIR_RS_LS_DumpLR(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR,
    gctBOOL             wColor)
{
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VIR_RA_LS_Interval  *pInterval;

    VIR_LOG(pDumper, "LR%d: \t", pLR->webIdx);
    VIR_LOG(pDumper, "tmp(%d", _VIR_RS_LS_GetWebRegNo(pRA, pLR));
    if (pLR->regNoRange > 1 && !isLRInvalid(pLR))
    {
        VIR_LOG(pDumper, "-%d) \t", pLR->firstRegNo + pLR->regNoRange-1);
    }
    else
    {
        VIR_LOG(pDumper, ") \t");
    }
    VIR_LOG(pDumper, "mask[%d] \t", VIR_RA_LS_LR2WebChannelMask(pRA, pLR));

    if (isLRNoShift(pLR))
    {
        VIR_LOG(pDumper, "restricted:[Y] \t");
    }
    else
    {
        VIR_LOG(pDumper, "restricted:[N] \t");
    }

    if (isLRInvalid(pLR))
    {
        VIR_LOG(pDumper, "master:[%d] \t", pLR->masterWebIdx);
    }
    else
    {
        VIR_LOG(pDumper, "master:[N] \t");
    }

    switch(pLR->hwType)
    {
    case VIR_RA_HWREG_GR:
        VIR_LOG(pDumper, "type:[G] \t");
        break;
    case VIR_RA_HWREG_A0:
        VIR_LOG(pDumper, "type:[A0] \t");
        break;
    case VIR_RA_HWREG_B0:
        VIR_LOG(pDumper, "type:[B0] \t");
        break;
    default:
        gcmASSERT(gcvFALSE);
    }

    VIR_LOG(pDumper, "weight:[%f] \t", pLR->weight);

    VIR_LOG(pDumper, "live interval:[%d, %d] \t", pLR->startPoint, pLR->endPoint);

    VIR_LOG(pDumper, "dead intervals: ");

    for (pInterval = pLR->deadIntervals; pInterval != gcvNULL;
        pInterval = pInterval->next)
    {
        VIR_LOG(pDumper, "[%d, %d] ", pInterval->startPoint,
            pInterval->endPoint);
    }

    if (wColor)
    {
        VIR_LOG(pDumper, "\t");
        _VIR_RA_LS_DumpColor(pRA, _VIR_RA_GetLRColor(pLR), pLR);
    }

    if (isLRHighpVec2(pLR))
    {
        VIR_LOG(pDumper, "highpvec2\t");
    }

    VIR_LOG(pDumper, "\n");
    VIR_LOG_FLUSH(pDumper);
}

void VIR_RS_LS_DumpLRTable(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc,
    gctBOOL         wColor)
{
    VIR_RA_LS_Liverange *pLR;
    gctUINT             webIdx;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
        if (pLR->liveFunc == pFunc || pFunc == (void*)-1)
        {
            _VIR_RS_LS_DumpLR(pRA, pLR, wColor);
        }
    }
}

void VIR_RS_LS_DumpSortedLRTable(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc,
    gctBOOL         wColor)
{
    VIR_RA_LS_Liverange *pLR = VIR_RA_LS_GetSortedLRHead(pRA)->nextLR;

    while (pLR != &LREndMark)
    {
        if (pLR->liveFunc == pFunc || pFunc == (void*)-1)
        {
            _VIR_RS_LS_DumpLR(pRA, pLR, wColor);
        }

        pLR = pLR->nextLR;
    }
}

/* ===========================================================================
   functions to build live ranges
   ===========================================================================
*/
void _VIR_RS_LS_SetLiveLRVec(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx)
{
    vscBV_SetBit(VIR_RA_LS_GetLiveLRVec(pRA), defIdx);
}

void _VIR_RS_LS_UnsetLiveLRVec(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx)
{
    vscBV_ClearBit(VIR_RA_LS_GetLiveLRVec(pRA), defIdx);
}

gctBOOL _VIR_RS_LS_TestLiveLRVec(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx)
{
    return vscBV_TestBit(VIR_RA_LS_GetLiveLRVec(pRA), defIdx);
}

void _VIR_RA_LS_SetHwRegType(
    VIR_RA_LS           *pRA,
    gctUINT             defIdx,
    VIR_RA_HWReg_Type   hwType)
{
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    pLR->hwType = hwType;
}

gctBOOL
_VIR_RA_LS_isPontialUniform(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF     *pDef = gcvNULL;

    if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL &&
        VIR_Symbol_GetKind(VIR_Operand_GetSymbol(pOpnd)) == VIR_SYM_UNIFORM)
    {
        return gcvTRUE;
    }

    vscVIR_InitGeneralUdIterator(&udIter,
        pLvInfo->pDuInfo, pInst, pOpnd, gcvFALSE, gcvFALSE);
    for(pDef = vscVIR_GeneralUdIterator_First(&udIter);
        pDef != gcvNULL;
        pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (pDef->defKey.pDefInst != VIR_INPUT_DEF_INST &&
            VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_LDARR)
        {
            VIR_Operand *baseOpnd = VIR_Inst_GetSource(pDef->defKey.pDefInst, 0);
            if (VIR_Operand_GetOpKind(baseOpnd) == VIR_OPND_SYMBOL &&
                VIR_Symbol_GetKind(VIR_Operand_GetSymbol(baseOpnd)) == VIR_SYM_UNIFORM)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

gctBOOL
_VIR_RA_LS_isUniformIndex(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctBOOL         isLDARR,
    gctUINT         *defIdx,
    VIR_Instruction **defInst)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(pInst, (isLDARR ? 1 : 0));

    VIR_USAGE_KEY   useKey;
    VIR_USAGE       *pUsage;
    VIR_OperandInfo operandInfo;
    gctUINT         useIdx;
    VIR_DEF         *pDef;

    gcmASSERT(pOpnd);

    VIR_Operand_GetOperandInfo(
        pInst,
        pOpnd,
        &operandInfo);

    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
    {
        useKey.pUsageInst = pInst;
        useKey.pOperand = pOpnd;
        useKey.bIsIndexingRegUsage = gcvFALSE;
        useIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &useKey);
        if (VIR_INVALID_USAGE_INDEX != useIdx)
        {
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, useIdx);
            gcmASSERT(UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain) == 1);
            *defIdx = UD_CHAIN_GET_FIRST_DEF(&pUsage->udChain);
            gcmASSERT(VIR_INVALID_DEF_INDEX != *defIdx);
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, *defIdx);
            *defInst = pDef->defKey.pDefInst;
            gcmASSERT(VIR_Inst_GetOpcode(*defInst) == VIR_OP_MOVA);

            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* return true, if the opnd/symbol is not needed to assign registers
   (i.e., in memory) */
gctBOOL _VIR_RA_LS_IsOpndSymExcludedLR(
    VIR_RA_LS       *pRA,
    VIR_Operand     *pOpnd,
    VIR_Symbol      *pSym)
{
    VIR_Symbol      *pVarSym = gcvNULL;

    if (pOpnd &&
        (VIR_Operand_IsArrayedPerVertex(pOpnd) ||
         VIR_Operand_IsPerPatch(pOpnd)))
    {
        return gcvTRUE;
    }

    if (pSym)
    {
        pVarSym = pSym;
    }
    else if (pOpnd)
    {
        pVarSym = VIR_Operand_GetUnderlyingSymbol(pOpnd);
    }

    if (pVarSym &&
        (isSymArrayedPerVertex(pVarSym) ||
         VIR_Symbol_isPerPatch(pVarSym)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL _VIR_RA_LS_InstNeedStoreDest(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    /* in v60, USC has constraint. If store instruction's src2 is
       immediate/uniform/indirect, there must be a store destination.
       store_attr/attr_st could not arrive here, since its dest should be per-patch,
       or per-vertex. we will use reservered register for its destination if needed. */
    if (pInst &&
        pInst != VIR_INPUT_DEF_INST &&
        pInst != VIR_HW_SPECIAL_DEF_INST &&
        VIR_OPCODE_hasStoreOperation(VIR_Inst_GetOpcode(pInst)))
    {
        if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5 &&
            VIR_Inst_Store_Have_Dst(pInst))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}
/* return true, if the inst is not needed to assign registers
   (i.e., in memory) */
gctBOOL _VIR_RA_LS_IsInstExcludedLR(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    /* in v60, USC has constraint. If store instruction's src2 is
       immediate/uniform/indirect, there must be a store destination.
       store_attr/attr_st could not arrive here, since its dest should be per-patch,
       or per-vertex. we will use reservered register for its destination if needed. */
    if (pInst &&
        pInst != VIR_INPUT_DEF_INST &&
        pInst != VIR_HW_SPECIAL_DEF_INST &&
        VIR_OPCODE_hasStoreOperation(VIR_Inst_GetOpcode(pInst)))
    {
        if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5)
        {
            return gcvTRUE;
        }
        else if (!VIR_Inst_Store_Have_Dst(pInst))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* return true, if the opnd/symbol/DEF is not needed to assign registers
   (i.e., in memory) */
gctBOOL _VIR_RA_LS_IsDefExcludedLR(
    VIR_DEF         *pDef)
{
    if (pDef &&
        (pDef->flags.nativeDefFlags.bIsPerVtxCp ||
         pDef->flags.nativeDefFlags.bIsPerPrim))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

void _VIR_RA_LS_SetDynIndexingLR(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    gcmASSERT(pLR);
    VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_DYN_INDEXING);
}

/* get def's output use instruction.
   it should be VIR_OUTPUT_USAGE_INST or emit */
VIR_Instruction *
_VIR_RA_LS_GetDefOutputUseInst(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    VIR_Instruction     *useInst = gcvNULL;
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_DEF             *pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
    VSC_DU_ITERATOR     duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_USAGE               *pUsage;

    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
    {
        pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
        if (VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
        {
            if (useInst == gcvNULL)
            {
                useInst = pUsage->usageKey.pUsageInst;
            }
            else
            {
                /* assume there is no two different output in one def*/
                gcmASSERT(useInst != pUsage->usageKey.pUsageInst);
            }
            break;
        }
    }

    return useInst;
}

/* get def varible starting register */
gctUINT
_VIR_RA_LS_GetDefMasterRegNo(
    VIR_RA_LS       *pRA,
    gctUINT         defIdx)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_DEF             *pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    gctUINT             regNo = pLR->firstRegNo;

    VIR_Symbol *dstSym = VIR_Operand_GetSymbol(pDef->defKey.pDefInst->dest);
    VIR_Symbol *varSym = gcvNULL;

    if (VIR_Symbol_isVreg(dstSym))
    {
        varSym = VIR_Symbol_GetVregVariable(dstSym);
        gcmASSERT(varSym);
        regNo = varSym->u2.tempIndex;
    }

    return regNo;
}

/* several cases we need to assign consecutive registers for a LR
   1) array/matrix/uchar_x16 with dynamic indexing
   there maybe other requirements later */
void _VIR_RA_LS_HandleMultiRegLR(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctUINT         defIdx)
{
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    VIR_DEF                 *pDef;

    pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
    gcmASSERT(pDef);

    if (pDef->flags.nativeDefFlags.bIsOutput && pDef->flags.deducedDefFlags.bDynIndexed)
    {
        /* get the vreg sym */
        VIR_Symbol *dstSym = VIR_Operand_GetSymbol(pInst->dest);
        VIR_Symbol *varSym = gcvNULL;

        _VIR_RA_LS_SetRestrictLR(pRA, defIdx);

        _VIR_RA_LS_SetDynIndexingLR(pRA, defIdx);

        if (VIR_Symbol_isVreg(dstSym))
        {
            /* get the vreg variable sym */
            varSym = VIR_Symbol_GetVregVariable(dstSym);
            gcmASSERT(varSym);

            if (VIR_Symbol_isVariable(varSym))
            {
                if (varSym->u2.tempIndex == dstSym->u1.vregIndex)
                {
                    /* the first output temp will be assigned to colors */

                    VIR_RA_OutputKey        *outputKey;
                    VIR_Instruction         *outputInst;

                    outputInst = _VIR_RA_LS_GetDefOutputUseInst(pRA, defIdx);
                    outputKey = _VIR_RA_NewOutputKey(pRA, varSym->u2.tempIndex, outputInst);

                    if(!vscHTBL_DirectTestAndGet(pRA->outputLRTable, (void*) outputKey, gcvNULL))
                    {
                        vscHTBL_DirectSet(pRA->outputLRTable,
                            (void*)_VIR_RA_NewOutputKey(pRA, varSym->u2.tempIndex, outputInst),
                            ((void*)(gctSIZE_T) pLR->webIdx));
                    }
                    vscMM_Free(VIR_RA_LS_GetMM(pRA), outputKey);

                    /* based on output's type, set RegNoRange to make sure that
                    the matrix/vec is assigned to consective registers. This is
                    required because of the current driver's limitation. Maybe relax
                    after driver change. */
                    pLR->regNoRange = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(varSym), -1);

                    VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_INVALID);
                }
                else
                {
                    if (pLR->firstRegNo == dstSym->u1.vregIndex)
                    {
                        /* the non-first output temp will be invalid LR and no color will be assigned */
                        VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_INVALID);

                        /* put the defIdx input masterWebIdx, it will be used inside
                            _VIR_RA_LS_SetMasterLR to find its master webIdx
                            and then it is set to master webIdx */
                        pLR->masterWebIdx = defIdx;
                    }
                }
            }
        }
    }
}

void _VIR_RA_LS_SetRegNoRange(
    VIR_RA_LS           *pRA,
    gctUINT             defIdx,
    gctUINT             firstRegNo,
    gctUINT             regNoRange,
    gctBOOL             bFromDst)
{
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);

    if (pLR->firstRegNo > firstRegNo)
    {
        pLR->firstRegNo = firstRegNo;
    }
    if (pLR->regNoRange < regNoRange)
    {
        pLR->regNoRange = regNoRange;
    }

    /*
    ** We may generate some internal instructions whose DEST is a attribute(mostly we generate them to recalculate this attribute),
    ** in this case, we need to copy the color from the attribute LR to the register LR.
    */
    if (bFromDst && pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
    {
        VIR_Symbol*             pTempSym = gcvNULL;
        VIR_Symbol*             pAttrSym = gcvNULL;
        VIR_DEF_KEY             defKey;
        gctUINT                 defIdx, webIdx;
        VIR_RA_LS_Liverange*    pAttrLR = gcvNULL;
        VIR_RA_HWReg_Color      hwColor;

        pTempSym = VIR_Shader_FindSymbolByTempIndex(pRA->pShader, firstRegNo);

        if (pTempSym &&
            VIR_Symbol_GetVregVariable(pTempSym) &&
            VIR_Symbol_isAttribute(VIR_Symbol_GetVregVariable(pTempSym)))
        {
            pLR->colorFunc = VIR_RA_LS_ATTRIBUTE_FUNC;

            pAttrSym = VIR_Symbol_GetVregVariable(pTempSym);
            defKey.pDefInst = VIR_INPUT_DEF_INST;
            defKey.regNo = VIR_Symbol_GetVariableVregIndex(pAttrSym);
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&pRA->pLvInfo->pDuInfo->defTable, &defKey);

            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
                pAttrLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
                hwColor = _VIR_RA_GetLRColor(pAttrLR);
                if (!_VIR_RA_LS_IsInvalidLOWColor(hwColor))
                {
                    _VIR_RA_SetLRColor(pLR, _VIR_RA_Color_RegNo(hwColor), _VIR_RA_Color_Shift(hwColor));
                    _VIR_RA_SetLRColorHI(pLR, _VIR_RA_Color_HIRegNo(hwColor), _VIR_RA_Color_HIShift(hwColor));
                }
            }
        }
    }
}

/* this function is to set masterWebIdx for each LR which does not need to assign color */
static void
_VIR_RA_LS_SetMasterLR(
    VIR_RA_LS       *pRA)
{
    VIR_RA_LS_Liverange *pLR, *pMasterLR = gcvNULL;
    gctUINT             webIdx;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        /* If the masterWebIdx of this LR is already set, then we don't need to set again. */
        if (isLRInvalid(pLR) && !isLRMasterWebIdxSet(pLR))
        {
            VIR_RA_OutputKey        *outputKey;
            gctSIZE_T                masterWebIdx;

            outputKey = _VIR_RA_NewOutputKey(pRA,
                _VIR_RA_LS_GetDefMasterRegNo(pRA, pLR->masterWebIdx),
                _VIR_RA_LS_GetDefOutputUseInst(pRA, pLR->masterWebIdx));

            if(vscHTBL_DirectTestAndGet(pRA->outputLRTable, (void*)(outputKey), (void**)&masterWebIdx))
            {
                pMasterLR = _VIR_RA_LS_Web2LR(pRA, (gctUINT)masterWebIdx);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            vscMM_Free(VIR_RA_LS_GetMM(pRA), outputKey);

            pLR->masterWebIdx = pMasterLR->webIdx;
            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_MASTER_WEB_IDX_SET);

            /* Because we consider all webs belong to output array as consecutive regs, so we temporarily
                set live range of first web to cover live ranges of all indivial webs to resolve register
                cruption issue */
            pMasterLR->startPoint = vscMIN(pMasterLR->startPoint, pLR->startPoint);
            pMasterLR->endPoint = vscMAX(pMasterLR->endPoint, pLR->endPoint);
        }
    }
}

void _VIR_RS_LS_MarkLRLive(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx,
    VIR_Enable  enable,
    gctBOOL     posAfter,
    gctINT      newPos
    )
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    VIR_RA_LS_Interval  *pInterval;
    gctUINT             positionIndex = (newPos != -1) ? (gctUINT)newPos : VIR_RA_LS_GetCurrPos(pRA);

    if (pLR->liveFunc == gcvNULL)
    {
        /* This is the first time we see this live range and it is a use */
        pLR->startPoint = 0;
        /* We mark the LR live after the current pos.
           This is used for mark live at BB's live out*/
        if (posAfter)
        {
            pLR->endPoint = positionIndex + 1;
        }
        else
        {
            pLR->endPoint = positionIndex;
        }
        pLR->liveFunc = pFunc;
    }
    else
    {
        /* if a live range already marked as dead by a previous def
           we need to add a new dead interval */
        if (pLR->startPoint != 0)
        {
            pInterval = (VIR_RA_LS_Interval*)vscMM_Alloc(
                VIR_RA_LS_GetMM(pRA), sizeof(VIR_RA_LS_Interval));
            if (posAfter)
            {
                pInterval->startPoint = positionIndex + 1;
            }
            else
            {
                pInterval->startPoint = positionIndex;
            }
            pInterval->endPoint = pLR->startPoint;
            pInterval->next = pLR->deadIntervals;
            pLR->deadIntervals = pInterval;
            pLR->startPoint = 0;
        }
    }

    /* if this LR is defined by a load */
    if (isLRLDDependence(pLR))
    {
        if (pRA->trueDepPoint > positionIndex)
        {
            pRA->trueDepPoint = positionIndex;
        }
    }

    if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
        (isLRSTDependence(pLR) ||
         isLRLDDependence(pLR)))
    {
        gctUINT extendedEndPoint = 0;

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption), VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT))
        {
            /* use max value as the extendedEndPoint for performance */
            extendedEndPoint = vscMAX((positionIndex + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)),
                                      pRA->trueDepPoint);
            if ((positionIndex + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)) != pRA->trueDepPoint)
            {
                VIR_RA_LS_SetExtendLSEndPoint(pRA, gcvTRUE);
            }
        }
        else
        {
            extendedEndPoint = vscMIN((positionIndex + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)),
                                      pRA->trueDepPoint);
        }

        extendedEndPoint = vscMIN(extendedEndPoint, VIR_Function_GetInstCount(pFunc));
        extendedEndPoint = vscMAX(extendedEndPoint, pLR->endPoint);
        pLR->endPoint = extendedEndPoint;
    }
}

void _VIR_RS_LS_MarkLRDead(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx,
    VIR_Enable  enable,
    gctBOOL     posAfter)
{
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function            *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_DEF                 *pDef;
    VSC_DU_ITERATOR         duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_USAGE               *pUsage;
    VIR_RA_LS_Liverange     *pUseLR;
    gctUINT                 tempDefIdx;
    VIR_DEF                 *pTempDef;
    VIR_DEF_KEY             defKey;
    VIR_OperandInfo         operandInfo;
    VIR_DU_CHAIN_USAGE_NODE *pTempUsageNode;
    VIR_USAGE*              pTempUsage;

    if (pLR->liveFunc == gcvNULL)
    {
        /* This is the first time we see this live range and it is a def */
        /* We mark the LR dead after the current pos.
           This is used for mark dead at BB's live out*/
        if (posAfter)
        {
            pLR->startPoint = VIR_RA_LS_GetCurrPos(pRA) + 1;
            pLR->endPoint = VIR_RA_LS_GetCurrPos(pRA) + 1;
        }
        else
        {
            pLR->startPoint = VIR_RA_LS_GetCurrPos(pRA);
            pLR->endPoint = VIR_RA_LS_GetCurrPos(pRA);
        }
        pLR->liveFunc = pFunc;
    }
    else
    {
        if (posAfter)
        {
            pLR->startPoint = VIR_RA_LS_GetCurrPos(pRA) + 1;
        }
        else
        {
            pLR->startPoint = VIR_RA_LS_GetCurrPos(pRA);
        }
    }

    if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
        (isLRSTDependence(pLR) ||
         isLRLDDependence(pLR)))
    {
        gctUINT extendedEndPoint = 0;

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption), VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT))
        {
            /* use max value as the extendedEndPoint for performance */
            extendedEndPoint = vscMAX((VIR_RA_LS_GetCurrPos(pRA) + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)),
                                      pRA->trueDepPoint);
            if ((VIR_RA_LS_GetCurrPos(pRA) + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)) != pRA->trueDepPoint)
            {
                VIR_RA_LS_SetExtendLSEndPoint(pRA, gcvTRUE);
            }
        }
        else
        {
            extendedEndPoint = vscMIN((VIR_RA_LS_GetCurrPos(pRA) + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption)),
                                      pRA->trueDepPoint);
        }

        extendedEndPoint = vscMIN(extendedEndPoint, VIR_Function_GetInstCount(pFunc));
        extendedEndPoint = vscMAX(extendedEndPoint, pLR->endPoint);
        pLR->endPoint = extendedEndPoint;
    }

    /* Because now we are not using channel web, so a LR dead means all
       channels on this LR are dead, thus the channelMask of LR must be
       reset to zero, otherwise if this web becomes live again with only
       part channels of web, then when any def of this web is visited,
       there will be wrong LR dead introduced because channelMask of LR
       is always equal to web's channelMask. */
    pLR->channelMask = 0;

    /* special handling for mova: to extend its endPoint
    to the last use's endPoint:
    1: mova t2.x, t1.x          [1, 2]
    2: ldarr dst, src0, t2.x    [2, 4]
    ==>
    1: mova t2.x, t1.x          [1, 4]
    2: ldarr dst, src0, t2.x    [2, 4] */
    if (isLRA0(pLR))
    {
        gctUINT8    channel;
        gctUINT     movaDefIdx;
        VIR_DEF     *movaDef;

        pLR->origEndPoint = pLR->endPoint;

        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

        /* go through MOVA's all def */
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(VIR_Inst_GetEnable(pDef->defKey.pDefInst), channel))
            {
                continue;
            }

            defKey.pDefInst = pDef->defKey.pDefInst;
            defKey.regNo = pDef->defKey.regNo;
            defKey.channel = channel;
            movaDefIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
            gcmASSERT(VIR_INVALID_DEF_INDEX != movaDefIdx);
            movaDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, movaDefIdx);

            /* go through all the uses, find the longest live range */
            VSC_DU_ITERATOR_INIT(&duIter, &movaDef->duChain);
            pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
            for (; pUsageNode != gcvNULL; )
            {
                pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
                defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pUsage->usageKey.pUsageInst);

                /* we only need to extend LDARR's live range, not STARR's */
                if ((VIR_INVALID_DEF_INDEX != defIdx) &&
                    (VIR_Inst_GetOpcode(pUsage->usageKey.pUsageInst) == VIR_OP_LDARR))
                {
                    VIR_Operand_GetOperandInfo(pUsage->usageKey.pUsageInst,
                                               pUsage->usageKey.pUsageInst->dest,
                                               &operandInfo);

                    defKey.pDefInst = pUsage->usageKey.pUsageInst;
                    defKey.regNo = operandInfo.u1.virRegInfo.virReg;
                    defKey.channel = VIR_CHANNEL_ANY;
                    tempDefIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != tempDefIdx);
                    pTempDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, tempDefIdx);
                    /* du chain could be empty
                    */
                    if (!DU_CHAIN_CHECK_EMPTY(&pTempDef->duChain))
                    {
                        pTempUsageNode = DU_CHAIN_GET_FIRST_USAGE(&pTempDef->duChain);
                        pTempUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pTempUsageNode->usageIdx);

                        if (!VIR_IS_OUTPUT_USAGE_INST(pTempUsage->usageKey.pUsageInst))
                        {
                            if (vscVIR_IsUniqueDefInstOfUsageInst(pLvInfo->pDuInfo, pTempUsage->usageKey.pUsageInst,
                                                                  pTempUsage->usageKey.pOperand, pTempUsage->usageKey.bIsIndexingRegUsage,
                                                                  pUsage->usageKey.pUsageInst, gcvNULL))
                            {
                                pUseLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
                                if (pUseLR->endPoint > pLR->endPoint)
                                {
                                    pLR->endPoint = pUseLR->endPoint;
                                }
                            }
                        }
                    }
                }

                pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter);
            }
        }
    }
}

/* return true if the LR's channel mask matches the Web's channel mask.
   to compute LR's channel mask, we need to "or" def's channel mask.
   we also need to reset LR's channel mask at the based on BB live out
   for example:
   t1.x =           <- we need to mark this LR's dead here
   t1.y =
   t1.z =
   ...
        = t1.xyz
*/
gctBOOL _VIR_RS_LS_MaskMatch(
    VIR_RA_LS  *pRA,
    VIR_Enable  defEnableMask,
    gctUINT     defIdx)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    VIR_WEB             *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

    pLR->channelMask |= defEnableMask;

    if (pWeb->channelMask == pLR->channelMask)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

void _VIR_RS_LS_UnsetOtherLiveLRVec(
    VIR_RA_LS           *pRA,
    gctUINT             defIdx
    )
{
    VIR_LIVENESS_INFO *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    VIR_WEB *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    gctUINT thisIdx;
    VIR_DEF *pDef;

    /* find all defs */
    thisIdx = pWeb->firstDefIdx;
    while (VIR_INVALID_DEF_INDEX != thisIdx)
    {
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, thisIdx);
        _VIR_RS_LS_UnsetLiveLRVec(pRA, thisIdx);

        thisIdx = pDef->nextDefInWebIdx;
    }
}

/* return true if the LDARR will be removed and
   the use of its def will be replaced with indexing */
gctBOOL _VIR_RA_LS_removableLDARR(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    gctBOOL             bAnalyzeOnly)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    gctUINT             defIdx = VIR_INVALID_DEF_INDEX, srcIndex;
    VIR_DEF             *pDef;
    VSC_DU_ITERATOR     duIter;

    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_USAGE               *pUsage;
    VIR_Instruction         *pUseInst;
    VIR_Operand             *pUseOperand;
    VIR_Operand             *pBaseOpnd = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand             *pOffsetOpnd = VIR_Inst_GetSource(pInst, 1);
    VIR_OperandInfo         offsetOpndInfo;
    VIR_Operand             *pDest = VIR_Inst_GetDest(pInst);

    gctBOOL         retValue = gcvTRUE;
    VIR_Swizzle     srcSwizzle = VIR_SWIZZLE_INVALID;
    VIR_Swizzle     useSwizzle = VIR_SWIZZLE_INVALID;
    VIR_Swizzle     mappingSwizzle = VIR_SWIZZLE_INVALID;
    VIR_Operand     *newOpnd = gcvNULL;
    VIR_OperandInfo srcInfo;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR);

    /* dual16 should not replace its uses, since instruction with indexing
       should not be dual16 */
    if (VIR_Shader_isDual16Mode(pShader) &&
        !VIR_TypeId_isSamplerOrImage(VIR_Operand_GetTypeId(pDest)))
    {
        return gcvFALSE;
    }

    if (pRA->pHwCfg->hwFeatureFlags.hasUniformB0 &&
        VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pBaseOpnd)))
    {
        return gcvFALSE;
    }

    /* we should not replace for a0.w or LR whose color has been valid,
       but sampler must be replaced */
    if (!bAnalyzeOnly)
    {
        gctUINT src0WebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, VIR_Inst_GetSource(pInst, 1));
        VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, src0WebIdx);
        if ((_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)) == 3 ||
             isLRA0B0Invalid(pLR)) &&
            !VIR_Symbol_isSampler(VIR_Operand_GetSymbol(pBaseOpnd)))
        {
            return gcvFALSE;
        }
    }

    /* get ldarr's dst LR */
    defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
    while (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

        if (pDef->defKey.pDefInst != pInst)
        {
            defIdx = pDef->nextDefIdxOfSameRegNo;
            continue;
        }

        /* go through all the uses */
        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
        {
            VIR_Instruction *redefinedBase = gcvNULL;
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
            pUseInst = pUsage->usageKey.pUsageInst;
            pUseOperand = pUsage->usageKey.pOperand;

            if(VIR_IS_OUTPUT_USAGE_INST(pUseInst))
            {
                retValue = gcvFALSE;
                continue;
            }

            if (VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(pUseInst))    ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_STORE_ATTR   ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_ATTR_ST      ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_IMG_STORE    ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_VX_IMG_STORE ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_IMG_STORE_3D ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_LDARR)
            {
                retValue = gcvFALSE;
                continue;
            }

            /* already replaced*/
            if (VIR_Operand_GetRelAddrMode(pUseOperand) != VIR_INDEXED_NONE)
            {
                continue;
            }

            /* So far we only check the unique def. */
            if (!vscVIR_IsUniqueDefInstOfUsageInst(pLvInfo->pDuInfo,
                                                   pUseInst,
                                                   pUseOperand,
                                                   pUsage->usageKey.bIsIndexingRegUsage,
                                                   pInst,
                                                   gcvNULL))
            {
                retValue = gcvFALSE;
                continue;
            }

            /* check baseOpnd is redefined between pInst and pUseInst */
            if (vscVIR_RedefineBetweenInsts(pRA->pMM, pLvInfo->pDuInfo, pInst, pUseInst, pBaseOpnd, &redefinedBase))
            {
                retValue = gcvFALSE;
                continue;
            }

            VIR_Operand_GetOperandInfo(pUseInst, pUseOperand, &srcInfo);

            srcIndex = VIR_Inst_GetSourceIndex(pUseInst, pUseOperand);
            gcmASSERT(srcIndex < VIR_MAX_SRC_NUM);

            /* An instruction can not hold two different uniforms access (indexing
               uniform might hit this limitation) */
            if (VIR_Operand_GetOpKind(pBaseOpnd) == VIR_OPND_SYMBOL &&
                VIR_Symbol_GetKind(VIR_Operand_GetSymbol(pBaseOpnd)) == VIR_SYM_UNIFORM)
            {
                gctUINT      k;
                VIR_Operand *opnd;

                for (k = 0; k < VIR_Inst_GetSrcNum(pUseInst); ++ k)
                {
                    if (k == srcIndex)
                    {
                        continue;
                    }

                    opnd = VIR_Inst_GetSource(pUseInst, k);

                    /* pontial uniform */
                    if (_VIR_RA_LS_isPontialUniform(pRA, pUseInst, opnd))
                    {
                        retValue = gcvFALSE;
                        continue;
                    }
                }
                if (!retValue)
                {
                    continue;
                }
            }

            /* Find a matched case here, we need to extend the LR of MOVA. */
            if (bAnalyzeOnly)
            {
                VIR_Operand_GetOperandInfo(pInst, pOffsetOpnd, &offsetOpndInfo);
                if (offsetOpndInfo.isVreg)
                {
                    VIR_GENERAL_UD_ITERATOR udIter;
                    VIR_DEF*                pDef;
                    gctUINT                 defIdx;

                    vscVIR_InitGeneralUdIterator(&udIter, pLvInfo->pDuInfo, pInst, pOffsetOpnd, gcvFALSE, gcvFALSE);
                    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                         pDef != gcvNULL;
                         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                    {
                        if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst))
                        {
                            continue;
                        }

                        if (VIR_Inst_GetOpcode(pDef->defKey.pDefInst) != VIR_OP_MOVA)
                        {
                            continue;
                        }

                        defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pDef->defKey.pDefInst);
                        _VIR_RS_LS_MarkLRLive(pRA, defIdx, pDef->OrgEnableMask, gcvTRUE, VIR_Inst_GetId(pUseInst));
                    }
                }
            }
            /* replace its uses
                pInst:    LDARR t1.x base, offset
                pUseInst: ADD t2.x, t1.x, t3.x
                ==>
                ADD t2.x, base[offset].x, t3.x
                !sampler use has to be replaced
            */
            else
            {
                srcSwizzle = VIR_Operand_GetSwizzle(pBaseOpnd);
                useSwizzle = VIR_Operand_GetSwizzle(pUseInst->src[srcIndex]);
                mappingSwizzle = VIR_Enable_GetMappingSwizzle(
                                    VIR_Operand_GetEnable(pInst->dest),
                                    srcSwizzle);
                VIR_Function_DupOperand(VIR_Inst_GetFunction(pInst), pBaseOpnd, &newOpnd);
                VIR_Operand_SetSwizzle(newOpnd,
                                       VIR_Swizzle_ApplyMappingSwizzle(useSwizzle, mappingSwizzle));
                VIR_Operand_SetTypeId(newOpnd, VIR_Operand_GetTypeId(pUseInst->src[srcIndex]));

                /* We need to copy the modifier and the order from the usage operand. */
                VIR_Operand_SetModifier(newOpnd, VIR_Operand_GetModifier(pUseOperand));
                VIR_Operand_SetModOrder(newOpnd, VIR_Operand_GetModOrder(pUseOperand));

                /* update the du - not complete yet
                   only delete the usage of t1, not add usage for base and offset */
                vscVIR_DeleteUsage(pLvInfo->pDuInfo,
                                   pInst,
                                   pUseInst,
                                   pUseOperand,
                                   pUsage->usageKey.bIsIndexingRegUsage,
                                   srcInfo.u1.virRegInfo.virReg,
                                   1,
                                   VIR_Swizzle_2_Enable(useSwizzle),
                                   VIR_HALF_CHANNEL_MASK_FULL,
                                   gcvNULL);
                VIR_Function_FreeOperand(VIR_Inst_GetFunction(pUseInst), VIR_Inst_GetSource(pUseInst, srcIndex));
                VIR_Inst_SetSource(pUseInst, srcIndex, newOpnd);

                if (VIR_Shader_isDual16Mode(pShader))
                {
                    VIR_Inst_SetThreadMode(pUseInst, VIR_THREAD_D16_DUAL_32);
                }
            }
        }

        defIdx = pDef->nextDefIdxOfSameRegNo;
    }

    return retValue;
}

void _VIR_RA_LS_ChangeMovaType(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst)
{

    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    VIR_DEF             *pDef;
    VIR_USAGE_KEY       usageKey;
    VIR_USAGE           *pUsage;
    gctUINT             usageIdx, defIdx, i;

    usageKey.pUsageInst = pInst;
    usageKey.pOperand = pInst->src[0];
    usageKey.bIsIndexingRegUsage = gcvFALSE;
    usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
    if (VIR_INVALID_USAGE_INDEX != usageIdx)
    {
        pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
        gcmASSERT(pUsage);

        for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
        {
            defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
            if (VIR_INVALID_USAGE_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst) ||
                    pDef->defKey.pDefInst == VIR_UNDEF_INST)
                {
                    continue;
                }
                gcmASSERT(pDef->defKey.pDefInst->dest);
                VIR_Operand_SetTypeId(pInst->dest, VIR_Operand_GetTypeId(pDef->defKey.pDefInst->dest));
                break;
            }
        }
    }
}

void _VIR_RA_LS_MarkDef(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctUINT         firstRegNo,
    gctUINT         regNoRange,
    VIR_Enable      defEnableMask,
    gctBOOL         bDstIndexing,
    VIR_TS_BLOCK_FLOW   *pBlkFlow)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             regNo, defIdx;
    gctUINT8            channel;
    VIR_DEF             *pDef;
    gctBOOL             removableLDARR = gcvFALSE;
    VIR_OpCode          instOpcode = VIR_Inst_GetOpcode(pInst);

    if (_VIR_RA_LS_IsInstExcludedLR(pRA, pInst))
    {
        /* some instruction may not actually define anything, such as
         * if the SOTRE has no need to have temp register to hold it
         * value if the value is already held in temp register */
        return;
    }

    if (instOpcode == VIR_OP_LDARR && _VIR_RA_LS_removableLDARR(pRA, pInst, gcvTRUE))
    {
        removableLDARR = gcvTRUE;
    }

    for (regNo = firstRegNo; regNo < firstRegNo + regNoRange; regNo++)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            if (!VSC_UTILS_TST_BIT(defEnableMask, channel))
            {
                continue;
            }

            defIdx = vscVIR_FindFirstDefIndex(pLvInfo->pDuInfo, regNo);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
                gcmASSERT(pLR);

                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                gcmASSERT(pDef);

                /* skip the per-vertex output and per-patch output */
                if (!_VIR_RA_LS_IsDefExcludedLR(pDef))
                {
                    if (pDef->defKey.pDefInst == pInst && (pDef->defKey.channel == channel))
                    {
                        if (_VIR_RA_LS_IsRestrictInst(pRA, pInst) ||
                            pDef->flags.nativeDefFlags.bIsOutput ||
                            pDef->flags.deducedDefFlags.bHasUsageOnNoSwizzleInst)
                        {
                            _VIR_RA_LS_SetRestrictLR(pRA, defIdx);
                        }

                        if (instOpcode == VIR_OP_MOVA)
                        {
                            _VIR_RA_LS_ChangeMovaType(pRA, pInst);

                            _VIR_RA_LS_SetHwRegType(pRA, defIdx, VIR_RA_HWREG_A0);

                            if (pRA->pHwCfg->hwFeatureFlags.hasUniformB0 &&
                                VIR_Operand_isUniformIndex(VIR_Inst_GetDest(pInst)))
                            {
                                _VIR_RA_LS_SetHwRegType(pRA, defIdx, VIR_RA_HWREG_B0);
                            }
                        }
                        else if (instOpcode == VIR_OP_LDARR &&
                                 pRA->pHwCfg->hwFeatureFlags.hasUniformB0 &&
                                 VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInst, 0))) &&
                                 !VIR_Operand_isUniformIndex(VIR_Inst_GetSource(pInst, 1)))
                        {
                            gctUINT     movaDefIdx = 0;
                            VIR_Instruction *movaInst = gcvNULL;
                            if (_VIR_RA_LS_isUniformIndex(pRA, pInst, gcvTRUE, &movaDefIdx, &movaInst))
                            {
                                _VIR_RA_LS_SetHwRegType(pRA, movaDefIdx, VIR_RA_HWREG_B0);
                                VIR_Operand_SetFlag(VIR_Inst_GetDest(movaInst), VIR_OPNDFLAG_UNIFORM_INDEX);
                            }
                        }
                        else if (instOpcode == VIR_OP_STARR &&
                            pRA->pHwCfg->hwFeatureFlags.hasUniformB0 &&
                            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetDest(pInst))) &&
                            !VIR_Operand_isUniformIndex(VIR_Inst_GetSource(pInst, 0)))
                        {
                            gctUINT     movaDefIdx = 0;
                            VIR_Instruction *movaInst = gcvNULL;
                            if (_VIR_RA_LS_isUniformIndex(pRA, pInst, gcvFALSE, &movaDefIdx, &movaInst))
                            {
                                _VIR_RA_LS_SetHwRegType(pRA, movaDefIdx, VIR_RA_HWREG_B0);
                                VIR_Operand_SetFlag(VIR_Inst_GetDest(movaInst), VIR_OPNDFLAG_UNIFORM_INDEX);
                            }
                        }
                        else if (VIR_OPCODE_isAtomCmpxChg(instOpcode))
                        {
                            if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5)
                            {
                                VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_ATOMCMPXHG);
                            }
                        }
                        else if (instOpcode == VIR_OP_VX_SELECTADD)
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_NO_USED_COLOR);
                        }
                        else if (VIR_OPCODE_isAttrLd(instOpcode) ||
                                 VIR_OPCODE_isMemLd(instOpcode) ||
                                 VIR_OPCODE_isImgLd(instOpcode))
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_LD_DEP);
                            pLR->pLDSTInst  = pInst;
                        }
                        else if (VIR_OPCODE_hasStoreOperation(instOpcode))
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_ST_DEP);
                            pLR->pLDSTInst  = pInst;
                        }

                        if (VIR_OPCODE_isVX(instOpcode))
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_VX);

                           if (pLR->currDef > VIR_RA_LS_GetCurrPos(pRA))
                            {
                                pLR->currDef = VIR_RA_LS_GetCurrPos(pRA);
                            }
                        }
                        /* set nousedcolor flag in debug mode */
                        if (gcmOPT_DisableOPTforDebugger())
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_NO_USED_COLOR);
                        }

                        _VIR_RA_LS_SetRegNoRange(pRA, defIdx, firstRegNo, regNoRange, gcvTRUE);

                        _VIR_RA_LS_HandleMultiRegLR(pRA, pInst, defIdx);

                        if (removableLDARR)
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_RM_LDARR_DEST);
                        }

                        /* Two cases that must mark dead
                           1. Def is a definite write. It is same logic as LV analysis.
                           2. Def is an orphan. It is the special logic for LR marker as we need give all
                              webs a legal LR range */
                        if ((!bDstIndexing &&
                             vscVIR_IsInstDefiniteWrite(pRA->pLvInfo->pDuInfo, pInst, firstRegNo, gcvTRUE))
                            ||
                            DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0)
                        {
                            _VIR_RS_LS_UnsetLiveLRVec(pRA, defIdx);

                            /* if LR's mask (LR mask is the "OR" of def's mask) matches
                               the web's channelmask, mark the LR dead

                               And since a LR is holding the entrie variable, if this variable is an array,
                               we can't mark the entire LR dead except it doesn't have usage. */
                            if ((_VIR_RS_LS_MaskMatch(pRA, (0x1 << channel), defIdx) && pLR->regNoRange == 1) ||
                                _VIR_RA_LS_InstNeedStoreDest(pRA, pInst) ||
                                DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0)
                            {
                                _VIR_RS_LS_MarkLRDead(pRA, defIdx, (0x1 << channel), gcvFALSE);

                                _VIR_RS_LS_UnsetOtherLiveLRVec(pRA, defIdx);
                            }
                            else if ((pLR->regNoRange == 1) &&
                                     (defEnableMask == pLR->channelMask) &&
                                     (pLR->minDefPos == (gctUINT)VIR_Inst_GetId(pInst))) /*visted all channels in curr def and find early define to set startPoint */
                            {
                                /* check regNo is not in the inFlow of first define inst's BB
                                   if there's partial define, regNo should be in the inFlow of first defined inst's BB */
                                gctBOOL regNoInInflow = gcvFALSE;
                                gctUINT index, i = 0;
                                VIR_DEF     *pDef = gcvNULL;

                                while ((index = vscBV_FindSetBitForward(&pBlkFlow->inFlow, i)) != (gctUINT)INVALID_BIT_LOC)
                                {
                                    pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, index);
                                    gcmASSERT(pDef);
                                    if (pDef->defKey.regNo == regNo)
                                    {
                                        regNoInInflow = gcvTRUE;
                                        break;
                                    }
                                    i = index + 1;
                                }
                                /* This is used to mark liverange like following case DEAD.
                                 * Liverange channel mask is reset for each bb and _VIR_RS_LS_MaskMatch return false
                                 * when dealing with t1.x in BB2(earlist define for t1)
                                 *       BB1
                                 *      /  \
                                 *   BB2:   \
                                 *   t1.x=   BB3:
                                 *      \   t1.x=
                                 *       \/
                                 *       BB4
                                 *       /\
                                 *   BB5:   \
                                 *  t1.y=    BB6:
                                 *           t1.y=
                                 *       \/
                                 *       BB7
                                 *       = t1.xy
                                 */
                                if (!regNoInInflow)
                                {
                                    _VIR_RS_LS_MarkLRDead(pRA, defIdx, (0x1 << channel), gcvFALSE);
                                    _VIR_RS_LS_UnsetOtherLiveLRVec(pRA, defIdx);
                                }
                            }
                        }
                    }
                }

                /* Get next def with same regNo */
                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }
}

gctBOOL _VIR_RA_LS_isUseCrossInst(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    VIR_Shader  *pShader = VIR_RA_LS_GetShader(pRA);

    if ((VIR_Inst_GetOpcode(pInst) == VIR_OP_ATAN) ||
        (VIR_Inst_GetOpcode(pInst) == VIR_OP_ASIN) ||
        (VIR_Inst_GetOpcode(pInst) == VIR_OP_ACOS) ||
        (VIR_Inst_GetOpcode(pInst) == VIR_OP_COS) ||
        (VIR_Inst_GetOpcode(pInst) == VIR_OP_SIN))
    {
        return gcvTRUE;
    }

    if (VIR_Shader_isDual16Mode(pShader) &&
        VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_32)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

void _VIR_RA_LS_MarkUse(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOperand,
    gctUINT         firstRegNo,
    gctUINT         regNoRange,
    VIR_Enable      defEnableMask)
{
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT                 usageIdx, defIdx, i;
    VIR_USAGE_KEY           usageKey;
    VIR_USAGE               *pUsage = gcvNULL;
    VIR_DEF*                pDef = gcvNULL;
    VIR_RA_LS_Liverange     *pLR;

    gctBOOL                 posAfter = gcvFALSE;
    if (_VIR_RA_LS_isUseCrossInst(pRA, pInst))
    {
        posAfter = gcvTRUE;
    }

    /* Find the usage */
    usageKey.pUsageInst = pInst;
    usageKey.pOperand = pOperand;
    usageKey.bIsIndexingRegUsage = gcvFALSE;
    usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
    if (VIR_INVALID_USAGE_INDEX != usageIdx)
    {
        pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
        gcmASSERT(pUsage);
        gcmASSERT(pUsage->webIdx != VIR_INVALID_WEB_INDEX);

        for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
        {
            defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
            gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

            if (pDef->flags.nativeDefFlags.bIsPerVtxCp ||
                pDef->flags.nativeDefFlags.bIsPerPrim)
            {
                continue;
            }

            pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);

            if (pDef->flags.deducedDefFlags.bHasUsageOnFalseDepInst)
            {
                VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_ST_DEP);
            }

            if (pDef->defKey.pDefInst != VIR_UNDEF_INST &&
                pDef->defKey.pDefInst != VIR_HW_SPECIAL_DEF_INST &&
                pDef->defKey.pDefInst != VIR_INPUT_DEF_INST)
            {
                VIR_OpCode instOpcode = VIR_Inst_GetOpcode(pDef->defKey.pDefInst);


                if (VIR_OPCODE_isAttrLd(instOpcode) ||
                    VIR_OPCODE_isMemLd(instOpcode) ||
                    VIR_OPCODE_isImgLd(instOpcode))
                {
                    VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_LD_DEP);
                    pLR->pLDSTInst = pDef->defKey.pDefInst;
                }
            }

            _VIR_RA_LS_SetRegNoRange(pRA, defIdx, firstRegNo, regNoRange, gcvFALSE);
            _VIR_RS_LS_MarkLRLive(pRA, defIdx, defEnableMask, posAfter, -1);
            _VIR_RS_LS_SetLiveLRVec(pRA, defIdx);

            pLR->channelMask &= ~(1 << pDef->defKey.channel);
        }
    }

    if (pUsage)
    {
        gctUINT          firstRegNo1, regNoRange1;
        VIR_Enable       defEnableMask1;
        VIR_OperandInfo  operandInfo, operandInfo1;

        defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, 0);
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
        if (!pDef->flags.nativeDefFlags.bIsPerVtxCp && !pDef->flags.nativeDefFlags.bIsPerPrim)
        {
            if (pDef->defKey.pDefInst < VIR_INPUT_DEF_INST)
            {
                if (vscVIR_IsUniqueDefInstOfUsageInst(pLvInfo->pDuInfo, pInst, pOperand, gcvFALSE,
                                                      pDef->defKey.pDefInst, gcvNULL) &&
                    VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_LDARR)
                {
                    VIR_Operand_GetOperandInfo(pDef->defKey.pDefInst,
                                               pDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                               &operandInfo);

                    VIR_Operand_GetOperandInfo(pDef->defKey.pDefInst,
                                               pDef->defKey.pDefInst->src[VIR_Operand_Src1],
                                               &operandInfo1);

                    if (operandInfo1.isImmVal)
                    {
                        firstRegNo1 = operandInfo.u1.virRegInfo.virReg + operandInfo1.u1.immValue.iValue;
                        regNoRange1 = 1;
                    }
                    else
                    {
                        firstRegNo1 = operandInfo.u1.virRegInfo.startVirReg;
                        regNoRange1 = operandInfo.u1.virRegInfo.virRegCount;
                    }

                    if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
                    {
                        defEnableMask1 = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pDef->defKey.pDefInst->src[VIR_Operand_Src0]));

                        _VIR_RA_LS_MarkUse(pRA,
                                           pDef->defKey.pDefInst,
                                           pDef->defKey.pDefInst->src[VIR_Operand_Src0],
                                           firstRegNo1,
                                           regNoRange1,
                                           defEnableMask1);
                    }
                }
            }
        }
    }
}

/* extend the liverange of src0 of MOVA conservatively for correctness
 * if src0 of LDARR is selected spilled, mov will be inserted before ldarr (see _VIR_RA_LS_InsertSpillOffset)
 * in following case, extend endPoint of temp(238).x to temp(538).x's endPoint
 * 158: MOVA               int addr_reg  temp(537).x, int temp(232).x
      <- "mov baseReg.x  temp(232).x" inserted here
 * 159: LDARR              int temp(533).x, int temp(17).x, int temp(537).x
  ...
 * 163: MOVA               int addr_reg  temp(538).x, int temp(238).x
 * 164: LDARR              int temp(535).x, int temp(25).x, int temp(537).x
       <- "mov baseReg.x temp(238).x" inserted here
 * 167: LDARR              int temp(536).x, int temp(25).x, int temp(538).x
 */
static void _VIR_RA_LS_ExtendLRofMOVASrc0(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    gctUINT                  src0WebIdx, defIdx, endPoint;
    VIR_Operand             *pSrc0 = pInst->src[VIR_Operand_Src0];
    VIR_RA_LS_Liverange     *pSrc0LR = gcvNULL;
    gctBOOL                  needExtendLR = gcvTRUE;
    VIR_Enable               enable = VIR_Operand_GetEnable(pInst->dest);
    VIR_RA_LS_Liverange     *pDestLR = gcvNULL;

    if (VIR_Operand_isImm(pSrc0) || VIR_Operand_isConst(pSrc0))
    {
        return;
    }
    endPoint = gcvMAXUINT32;
    defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
    if (defIdx != VIR_INVALID_DEF_INDEX)
    {
        if (_VSC_RA_EnableSingleChannel(enable))
        {
            VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
            VSC_DU_ITERATOR          duIter;
            VIR_DEF                 *pDef;
            VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
            VIR_USAGE               *pUsage;
            VIR_Instruction         *pUseInst;

            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
            /* go through all the uses */
            VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
            pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
            for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
            {
                pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
                pUseInst = pUsage->usageKey.pUsageInst;
                /* only deal usage instr is LDARR */
                if (pUseInst->_opcode == VIR_OP_LDARR || pUseInst->_opcode == VIR_OP_STARR)
                {
                    /* if MOVA defines one channel and LDARR/STARR is the next of MOVA, donothing*/
                    if (VIR_Inst_GetNext(pInst) == pUseInst)
                    {
                        needExtendLR = gcvFALSE;
                        /* only one mov instruction for multi same temp useages is inserted if use reservedMovaReg*/
                        if (pRA->bReservedMovaReg)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (pRA->bReservedMovaReg)
                        {
                            /* set nearest pos to endPoint */
                            if (endPoint > ((gctUINT)VIR_Inst_GetId(pUseInst) + 1))
                            {
                                endPoint = ((gctUINT)VIR_Inst_GetId(pUseInst) + 1);
                            }
                        }
                        else
                        {
                            /* extend src0 endpoint to dest endpoint */
                            needExtendLR = gcvTRUE;
                            pDestLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
                            gcmASSERT(pDestLR);
                            endPoint = pDestLR->endPoint;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            pDestLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
            gcmASSERT(pDestLR);
            endPoint = pDestLR->endPoint;
        }
    }
    if (needExtendLR && (endPoint != gcvMAXUINT32))  /* if endPoint is gcvMAXUINT32, no usage is LDARR and donothing */
    {
        src0WebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pSrc0);
        if (VIR_INVALID_WEB_INDEX != src0WebIdx)
        {
            pSrc0LR = _VIR_RA_LS_Web2ColorLR(pRA, src0WebIdx);
            gcmASSERT(pSrc0LR);
            if (endPoint > pSrc0LR->endPoint)
            {
                pSrc0LR->endPoint = endPoint;
            }
        }
    }
}

void _VIR_RA_LS_MarkUses(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    gctUINT                 firstRegNo, regNoRange;
    VIR_Enable              defEnableMask;
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand             *pOpnd;
    VIR_OpCode               opCode = VIR_Inst_GetOpcode(pInst);

    /* A ldarr inst to attribute array may potentially read all elements in array */
    if (opCode == VIR_OP_LDARR)
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src0],
                                   &operandInfo);

        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->src[VIR_Operand_Src1],
                                   &operandInfo1);

        if (operandInfo1.isImmVal)
        {
            firstRegNo = operandInfo.u1.virRegInfo.virReg +
                         operandInfo1.u1.immValue.iValue;
            regNoRange = 1;
        }
        else
        {
            if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo1))
            {
                defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(
                    pInst->src[VIR_Operand_Src1]));

                _VIR_RA_LS_MarkUse(pRA,
                                   pInst,
                                   pInst->src[VIR_Operand_Src1],
                                   operandInfo1.u1.virRegInfo.virReg,
                                   1,
                                   defEnableMask);
            }

            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
        }

        if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(
                pInst->src[VIR_Operand_Src0]));

            _VIR_RA_LS_MarkUse(pRA,
                               pInst,
                               pInst->src[VIR_Operand_Src0],
                               firstRegNo,
                               regNoRange,
                               defEnableMask);
        }
    }
    else
    {
        VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
        pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

        for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
        {
            VIR_Operand_GetOperandInfo(pInst,
                                       pOpnd,
                                       &operandInfo);

            if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
            {
                firstRegNo = operandInfo.u1.virRegInfo.virReg;
                regNoRange = operandInfo.u1.virRegInfo.virRegCount;
                defEnableMask = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd));

                _VIR_RA_LS_MarkUse(pRA,
                                   pInst,
                                   pOpnd,
                                   firstRegNo,
                                   regNoRange,
                                   defEnableMask);
            }
        }
    }

    /* specially handling for MOVA, extend the endPC of src0 as the dest */
    if (opCode == VIR_OP_MOVA)
    {
        _VIR_RA_LS_ExtendLRofMOVASrc0(pRA, pInst);
    }

    /* Check if any LR need even or odd register. */
    if (_VIR_RA_LS_InstNeedEvenOddReg(pRA, pInst))
    {
        gctUINT             higherSrcWebIdx, lowerWebIdx;
        gctUINT             higherSrcIdx = VIR_OPCODE_Src0Src1Temp256(opCode) ? 0 : 1;
        gctUINT             lowerSrcIdx = VIR_OPCODE_Src0Src1Temp256(opCode) ? 1 : 2;
        VIR_RA_LS_Liverange *pHigherSrcLR = gcvNULL;
        VIR_RA_LS_Liverange *pLowerSrcLR = gcvNULL;

        higherSrcWebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, VIR_Inst_GetSource(pInst, higherSrcIdx));
        if (VIR_INVALID_WEB_INDEX != higherSrcWebIdx)
        {
            pHigherSrcLR = _VIR_RA_LS_Web2ColorLR(pRA, higherSrcWebIdx);
            gcmASSERT(pHigherSrcLR);

            pHigherSrcLR->regNoRange = 2;
            VIR_RA_LR_SetFlag(pHigherSrcLR, VIR_RA_LRFLAG_VX_EVEN);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        lowerWebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, VIR_Inst_GetSource(pInst, lowerSrcIdx));
        if (VIR_INVALID_WEB_INDEX != lowerWebIdx)
        {
            pLowerSrcLR = _VIR_RA_LS_Web2ColorLR(pRA, lowerWebIdx);
            gcmASSERT(pLowerSrcLR);

            if (pHigherSrcLR != pLowerSrcLR)
            {
                VIR_RA_LR_SetFlag(pLowerSrcLR, VIR_RA_LRFLAG_INVALID | VIR_RA_LRFLAG_MASTER_WEB_IDX_SET | VIR_RA_LRFLAG_VX_ODD);
                pLowerSrcLR->masterWebIdx = higherSrcWebIdx;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
}

void _VIR_RA_LS_GetSym_Enable_Swizzle(
    VIR_Symbol      *pSym,
    VIR_Enable      *symEnable,
    VIR_Swizzle     *symSwizzle
    )
{
    gctUINT         components = VIR_Symbol_GetComponents(pSym);

    switch(components)
    {
    case 0:
        if (symSwizzle)
        {
            *symSwizzle = VIR_SWIZZLE_INVALID;
        }
        if (symEnable)
        {
            *symEnable = VIR_ENABLE_NONE;
        }
        break;
    case 1:
        if (symSwizzle)
        {
            *symSwizzle = VIR_SWIZZLE_XXXX;
        }
        if (symEnable)
        {
            *symEnable = VIR_ENABLE_X;
        }
        break;
    case 2:
        if (symSwizzle)
        {
            *symSwizzle = VIR_SWIZZLE_XYYY;
        }
        if (symEnable)
        {
            *symEnable = VIR_ENABLE_XY;
        }
        break;
    case 3:
        if (symSwizzle)
        {
            *symSwizzle = VIR_SWIZZLE_XYZZ;
        }
        if (symEnable)
        {
            *symEnable = VIR_ENABLE_XYZ;
        }
        break;
    case 4:
        if (symSwizzle)
        {
            *symSwizzle = VIR_SWIZZLE_XYZW;
        }
        if (symEnable)
        {
            *symEnable = VIR_ENABLE_XYZW;
        }
        break;
    default:
        gcmASSERT(gcvFALSE);
    }
}

/* mark the use of outputs in EMIT instruction */
void _VIR_RA_LS_Mark_Outputs(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctBOOL         bCheckAllOutput,
    gctINT          streamNumber)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             outputIdx, usageIdx;
    VIR_USAGE_KEY       usageKey;
    VIR_USAGE           *pUsage;
    gctUINT             i, j, defIdx;
    VIR_Enable          outEnable;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT0 ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT  ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM);

    for (outputIdx = 0;
         outputIdx < VIR_IdList_Count(VIR_Shader_GetOutputs(pShader));
         outputIdx ++)
    {
        VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                            VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader), outputIdx));

        /* Check for the specified output only. */
        if (!bCheckAllOutput && VIR_Symbol_GetStreamNumber(pOutputSym) != streamNumber)
        {
            continue;
        }

        for (i = 0; i <VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pOutputSym), -1); i++)
        {
            /* Find the usage */
            usageKey.pUsageInst = pInst;
            usageKey.pOperand = (VIR_Operand*)(gctUINTPTR_T) (pOutputSym->u2.tempIndex+i);
            usageKey.bIsIndexingRegUsage = gcvFALSE;
            usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
                gcmASSERT(pUsage);
                gcmASSERT(pUsage->webIdx != VIR_INVALID_WEB_INDEX);

                for (j = 0; j < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); j ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, j);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);

                    _VIR_RA_LS_GetSym_Enable_Swizzle(pOutputSym, &outEnable, gcvNULL);
                    _VIR_RS_LS_MarkLRLive(pRA, defIdx, outEnable, gcvFALSE, -1);
                    _VIR_RS_LS_SetLiveLRVec(pRA, defIdx);
                }
            }
        }
    }
}

/* ===========================================================================
   functions to assign colors
   ===========================================================================
*/
gctBOOL _VIR_RS_LS_IsSpecialReg(
    gctUINT         regNo)
{
    if ((regNo ==  VIR_SR_INSTATNCEID)          ||
        (regNo ==  VIR_SR_VERTEXID)             ||
        (regNo == VIR_REG_MULTISAMPLEDEPTH)     ||
        (regNo == VIR_REG_SAMPLE_POS)           ||
        (regNo == VIR_REG_SAMPLE_ID)            ||
        (regNo == VIR_REG_SAMPLE_MASK_IN))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

void _VIR_RA_LS_SetMaxAllocReg(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Color  color,
    VIR_RA_HWReg_Type   hwType,
    gctUINT             regNoRange)
{
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    gctUINT             maxReg = 0;
    gctUINT             highDiff = 0;

    gcmASSERT(!_VIR_RA_LS_IsInvalidColor(color));

    if (regNoRange > 1 && !_VIR_RA_LS_IsInvalidHIColor(color))
    {
        /* array case - we should allocate consecutive registers */
        highDiff = _VIR_RA_Color_HIRegNo(color) - _VIR_RA_Color_RegNo(color);
        gcmASSERT(highDiff == 0 || highDiff == 1);
    }

    maxReg = _VIR_RA_Color_RegNo(color) + (regNoRange - 1) * (highDiff + 1);
    if (!_VIR_RS_LS_IsSpecialReg(maxReg))
    {
        if (maxReg > pCP->colorMap[hwType].maxAllocReg)
        {
            pCP->colorMap[hwType].maxAllocReg = maxReg;
        }
    }

    if (!_VIR_RA_LS_IsInvalidHIColor(color))
    {
        maxReg = _VIR_RA_Color_HIRegNo(color) + (regNoRange - 1) * (highDiff + 1);
        if (!_VIR_RS_LS_IsSpecialReg(maxReg))
        {
            if (maxReg > pCP->colorMap[hwType].maxAllocReg)
            {
                pCP->colorMap[hwType].maxAllocReg = maxReg;
            }
        }
    }
}

gctUINT
_VIR_RA_LS_GetMaxReg(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Type   hwType,
    gctUINT             reservedDataReg)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VSC_HW_CONFIG       *hwConfig = VIR_RA_LS_GetHwCfg(pRA);
    gctUINT             maxReg = pCP->colorMap[hwType].maxReg;

    gcmASSERT(hwType == VIR_RA_HWREG_GR || hwType == VIR_RA_HWREG_A0 || hwType == VIR_RA_HWREG_B0);

    /* Now only the count of the general registers are floating, the other kinds of registers are all fixed. */
    if (hwType != VIR_RA_HWREG_GR)
    {
        return maxReg;
    }

    /* Check if we need to adjust the maximum number. */
    if (pRA->currentMaxGRCount != VIR_RA_LS_REG_MAX)
    {
        maxReg = pRA->currentMaxGRCount;
    }
    else
    {
        /* if the shader needs sampleDepth, we need to make sure the last
           register is for sampleDepth */
        if (_VIR_RA_isShaderNeedSampleDepth(pRA))
        {
            maxReg -= 1;
        }

        if (VIR_Shader_CalcMaxRegBasedOnWorkGroupSize(pShader))
        {
            maxReg = vscMIN(VIR_Shader_GetMaxFreeRegCountPerThread(pShader, hwConfig), pCP->colorMap[hwType].maxReg);
        }

        maxReg = vscMIN(maxReg, hwConfig->maxGPRCountPerThread);

        /* we need to reserve 3 more registers (baseRegister/movaRegister) +
           data registers for spilling */
        if (reservedDataReg > 0)
        {
            if (maxReg > reservedDataReg + 1 + pRA->movaRegCount)
            {
                maxReg = maxReg - reservedDataReg - 1 - pRA->movaRegCount;
            }
            else
            {
                maxReg = 0;
            }
        }

        pRA->currentMaxGRCount = maxReg;
    }

    return maxReg;
}

void _VIR_RA_LS_AssignColorWeb(
    VIR_RA_LS           *pRA,
    gctUINT             webIdx,
    VIR_RA_HWReg_Type   hwRegType,
    VIR_RA_HWReg_Color  color,
    VIR_Function        *pFunc)
{
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

    gcmASSERT(hwRegType == pLR->hwType );

    if (pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
    {
        pLR->colorFunc = pFunc;
    }

    /* spill case */
    if (_VIR_RA_LS_IsInvalidColor(color))
    {
        VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SPILLED);
        _VIR_RA_SetLRSpillOffset(pLR, pRA->spillOffset);
        pRA->spillOffset += pLR->regNoRange * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__;
    }
    else
    {
        pLR->u1.color = color;
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        if (!isLRSpilled(pLR))
        {
            VIR_LOG(pDumper, "assign ");
            _VIR_RA_LS_DumpColor(pRA, _VIR_RA_GetLRColor(pLR), pLR);
            VIR_LOG(pDumper, " to LR%d\n", webIdx);
        }
        else
        {
            VIR_LOG(pDumper, "spill LR%d to offset %d ", webIdx,
                _VIR_RA_GetLRSpillOffset(pLR));
        }
        VIR_LOG_FLUSH(pDumper);
    }
}

void _VIR_RA_LS_SetUsedColor(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Type   hwType,
    gctUINT             regNo,
    gctUINT             channels)
{
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VSC_BIT_VECTOR      *usedColor = &(pCP->colorMap[hwType].usedColor);

    gctUINT i;
    if (!_VIR_RS_LS_IsSpecialReg(regNo))
    {
        for (i = 0; i< 4; i++)
        {
            if (channels & (0x1 << i))
            {
                vscBV_SetBit(usedColor, (regNo * 4 + i));
            }
        }
    }
}

void _VIR_RA_LS_ClearUsedColor(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Type   hwType,
    gctUINT             regNo,
    gctUINT             channels)
{
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VSC_BIT_VECTOR      *usedColor = &(pCP->colorMap[hwType].usedColor);

    gctUINT i;
    if (!_VIR_RS_LS_IsSpecialReg(regNo))
    {
        for (i =0; i< 4; i++)
        {
            if (channels & (0x1 << i))
            {
                vscBV_ClearBit(usedColor, (regNo * 4 + i));
            }
        }
    }
}

gctBOOL _VIR_RA_LS_TestUsedColor(
    VIR_RA_LS           *pRA,
    VIR_RA_HWReg_Type   hwType,
    gctUINT             regNo,
    gctUINT             channels)
{
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VSC_BIT_VECTOR      *usedColor = &(pCP->colorMap[hwType].usedColor);

    gctUINT i;
    if (!_VIR_RS_LS_IsSpecialReg(regNo))
    {
        for (i = 0; i< 4; i++)
        {
            if (channels & (0x1 << i))
            {
                if (vscBV_TestBit(usedColor, (regNo * 4 + i)))
                {
                    return gcvTRUE;
                }
            }
        }
    }

    return gcvFALSE;
}

void _VIR_RA_LS_RemoveLRfromActiveList(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pPrev,
    VIR_RA_LS_Liverange *pRemoveLR)
{
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    gctUINT             i, highDiff = 0;
    VIR_Enable          LREnable = VIR_RA_LS_LR2WebChannelMask(pRA, pRemoveLR);

    gcmASSERT(pRemoveLR == pPrev->nextActiveLR);

    /* remove pRemoveLR from the active list */
    pPrev->nextActiveLR = pRemoveLR->nextActiveLR;
    pRemoveLR->nextActiveLR = gcvNULL;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
    VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "remove LR%d from the active list ", pRemoveLR->webIdx);
        _VIR_RA_LS_DumpColor(pRA, _VIR_RA_GetLRColor(pRemoveLR), pRemoveLR);
        VIR_LOG_FLUSH(pDumper);
    }

    /* it is possible that LR is a invalid color (A0's color is used by other A0) */
    if (!_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pRemoveLR)))
    {
        /* unset the usedColor bit accordingly */

        /* this LR is used color from usedColorLR's dead interval*/
        if (pRemoveLR->usedColorLR != gcvNULL)
        {
            pRemoveLR->usedColorLR->deadIntervalAvail = gcvTRUE;
            pRemoveLR->usedColorLR = gcvNULL;
            /* if pRemoveLR allocated a used color(which is non ld-st LR), clear falseDepReg here */
            gcmASSERT(pRemoveLR->regNoRange == 1);
            if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                (isLRSTDependence(pRemoveLR) ||
                 isLRLDDependence(pRemoveLR)))
            {
                _VIR_RA_FlaseDepReg_Clear(pRA,
                                          _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pRemoveLR)));

                if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pRemoveLR)))
                {
                    _VIR_RA_FlaseDepReg_Clear(pRA,
                                              _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pRemoveLR)));
                }
            }
        }
        else
        {
            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pRemoveLR)) &&
                pRemoveLR->regNoRange > 1)
            {
                highDiff = _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pRemoveLR)) - _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pRemoveLR));
            }
            for (i = 0; i< pRemoveLR->regNoRange; i++)
            {
                _VIR_RA_LS_ClearUsedColor(pRA, pRemoveLR->hwType,
                        _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pRemoveLR)) + i * (highDiff + 1),
                        _VIR_RA_Color_Channels(LREnable,
                            _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pRemoveLR))));

                if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pRemoveLR)))
                {
                    _VIR_RA_LS_ClearUsedColor(pRA, pRemoveLR->hwType,
                            _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pRemoveLR)) + i * (highDiff + 1),
                            _VIR_RA_Color_Channels(LREnable,
                                    _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pRemoveLR))));
                }

                if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                    (isLRSTDependence(pRemoveLR) ||
                     isLRLDDependence(pRemoveLR)))
                {
                    _VIR_RA_FlaseDepReg_Clear(pRA,
                        _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pRemoveLR)) + i * (highDiff + 1));

                    if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pRemoveLR)))
                    {
                        _VIR_RA_FlaseDepReg_Clear(pRA,
                            _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pRemoveLR)) + i * (highDiff + 1));
                    }
                }
            }
        }
    }
}

void _VIR_RA_LS_ExpireActiveLRs(
    VIR_RA_LS           *pRA,
    gctUINT             pos)
{
    VIR_RA_LS_Liverange *pPrev, *pCurr, *pNext;
    gctBOOL             debugEnabled = gcmOPT_DisableOPTforDebugger();

    if (!debugEnabled)
    {
        pPrev = VIR_RA_LS_GetActiveLRHead(pRA);
        pCurr = pPrev->nextActiveLR;
        while ((pCurr != &LREndMark) &&
               (pCurr->endPoint <= pos))
        {
            pNext = pCurr->nextActiveLR;

            /* remove pCurr from the active list */
            _VIR_RA_LS_RemoveLRfromActiveList(pRA, pPrev, pCurr);

            /* active LR is already sorted, we don't need to move pPrev */
            pCurr = pNext;
        }
    }
}

VSC_ErrCode
_VIR_RA_LS_SetUsedColorForLR(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR,
    gctBOOL             newColor,
    gctUINT             reservedDataReg
    )
{
    VSC_ErrCode retValue = VSC_ERR_NONE;
    VIR_Enable  LREnable = VIR_RA_LS_LR2WebChannelMask(pRA, pLR);
    gctUINT     highDiff = 0, i, regNo, regNoHI;
    gctBOOL     debugEnabled = gcmOPT_DisableOPTforDebugger();

    /* Set the usedColor bit accordingly */
    gcmASSERT(!_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pLR)));
    if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)) &&
        pLR->regNoRange > 1)
    {
        highDiff = _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) - _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR));
    }

    for (i = 0; i < pLR->regNoRange; i++)
    {
        regNo = _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + i * (highDiff + 1);
        regNoHI = _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + i * (highDiff + 1);

        if (newColor &&
            pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC &&
            !debugEnabled &&
            _VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                regNo,
                _VIR_RA_Color_Channels(LREnable,
                    _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)))))
        {
            /* opps, RA has some issue, two LR assigned to the same color*/
            gcmASSERT(gcvFALSE);
        }

        /* make sure the register is enough (for input) */
        if (!_VIR_RS_LS_IsSpecialReg(regNo) &&
            regNo >= _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
        {
            /* only input can come here, temp should be already spilled */
            gcmASSERT(pLR->colorFunc == VIR_RA_LS_ATTRIBUTE_FUNC);
            retValue = VSC_RA_ERR_OUT_OF_REG_FAIL;
            return retValue;
        }

        _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType, regNo,
                _VIR_RA_Color_Channels(LREnable,
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR))));

        if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
            (isLRSTDependence(pLR) ||
             isLRLDDependence(pLR)))
        {
            _VIR_RA_FlaseDepReg_Set(pRA, regNo);
        }

        if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)))
        {
            if (newColor &&
                pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC &&
                !isLRB0(pLR) &&
                _VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                    regNoHI,
                    _VIR_RA_Color_Channels(LREnable,
                        _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)))))
            {
                /* opps, RA has some issue, two LR assigned to the same color*/
                gcmASSERT(gcvFALSE);
            }

            if (!_VIR_RS_LS_IsSpecialReg(regNoHI) &&
                regNoHI >= _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
            {
                retValue = VSC_RA_ERR_OUT_OF_REG_FAIL;
                return retValue;
            }

            _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType, regNoHI,
                    _VIR_RA_Color_Channels(LREnable,
                            _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR))));

            if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                (isLRSTDependence(pLR) ||
                 isLRLDDependence(pLR)))
            {
                _VIR_RA_FlaseDepReg_Set(pRA, regNoHI);
            }
        }
    }

    return retValue;
}

VSC_ErrCode
_VIR_RA_LS_AddActiveLRs(
    VIR_RA_LS           *pRA,
    gctUINT             webIdx,
    gctBOOL             newColor,
    VIR_Function        *pFunc,
    gctUINT             reservedDataReg)
{
    VSC_ErrCode         retValue = VSC_ERR_NONE;
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_RA_LS_Liverange *pPrev, *pNext;
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

    if(!isLRSpilled(pLR))
    {
        pPrev = VIR_RA_LS_GetActiveLRHead(pRA);
        pNext = pPrev->nextActiveLR;
        while ((pNext != &LREndMark) &&
               (pNext != pLR) &&
               (pNext->endPoint <= pLR->endPoint))
        {
            pPrev = pNext;
            pNext = pNext->nextActiveLR;
        }

        /* insert pLR between pHead and pHeadNext
           don't insert the live range twice */
        if (pNext != pLR)
        {
            pPrev->nextActiveLR = pLR;
            pLR->nextActiveLR = pNext;

            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption), VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
            {
                VIR_LOG(pDumper, "add LR%d to the active list", pLR->webIdx);
                VIR_LOG_FLUSH(pDumper);
            }

            retValue = _VIR_RA_LS_SetUsedColorForLR(pRA, pLR, newColor, reservedDataReg);
            CHECK_ERROR(retValue, "set used color for LR");

            /* set the register allocation water mark */
            _VIR_RA_LS_SetMaxAllocReg(pRA, _VIR_RA_GetLRColor(pLR),
                pLR->hwType, pLR->regNoRange);

            if (pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
            {
                pLR->colorFunc = pFunc;
            }
        }
    }
    return retValue;
}

/* return gcvTRUE if all channels are available (usedColor bit is not set)*/
gctBOOL _VIR_RA_LS_ChannelAvail(
    VIR_RA_LS           *pRA,
    gctUINT             regno,
    gctUINT8            enable,
    VIR_RA_HWReg_Type   hwType)
{
    VSC_BIT_VECTOR  *usedColor =
        &(VIR_RA_LS_GetColorPool(pRA)->colorMap[hwType].usedColor);

    /* Test if x-component is available. */
    if ((enable & 0x1) && vscBV_TestBit(usedColor, regno * 4 + 0))
    {
        return gcvFALSE;
    }

    /* Test if y-component is available. */
    if ((enable & 0x2) && vscBV_TestBit(usedColor, regno * 4 + 1))
    {
        return gcvFALSE;
    }

    /* Test if z-component is available. */
    if ((enable & 0x4) && vscBV_TestBit(usedColor, regno * 4 + 2))
    {
        return gcvFALSE;
    }

    /* Test if w-component is available. */
    if ((enable & 0x8) && vscBV_TestBit(usedColor, regno * 4 + 3))
    {
        return gcvFALSE;
    }

    /* All requested channels are available */
    return gcvTRUE;

}

/* return gcvTRUE if dstLR can fit into srcLR's channel
   if srcLR is gcvNULL, then return gcvTRUE if the new color's channel can
   fit (new color is represent by regno).
   if restrict is set, then no shift is allowed.
*/
gctBOOL _VIR_RA_LS_ChannelFit(
    VIR_RA_LS               *pRA,
    VIR_RA_LS_Liverange     *dstLR,
    VIR_RA_LS_Liverange     *srcLR,
    gctUINT                 regno,
    gctUINT                 *shift)
{
    gctBOOL         retValue = gcvFALSE;
    VIR_Enable      dstChannel = VIR_RA_LS_LR2WebChannelMask(pRA, dstLR);
    gctUINT         srcChannel = 0xF;
    gctBOOL         srcColor = gcvFALSE;
    gctBOOL         restricted = isLRNoShift(dstLR);

    if (_VIR_RA_FlaseDepReg_Test(pRA, regno))
    {
        return gcvFALSE;
    }

    if (srcLR != gcvNULL)
    {
        srcChannel = _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, srcLR),
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(srcLR)));
        srcColor = gcvTRUE;
    }

    /* check enable channel of highpVec2 LR should be VIR_ENABLE_X/VIR_ENABLE_Y/VIR_ENABLE_XY */
    gcmASSERT(!isLRHighpVec2(dstLR) || dstChannel <= 0x3);

    switch (dstChannel)
    {
    case VIR_ENABLE_X:
        /*check x and z are avaible */
        if (isLRHighpVec2(dstLR))
        {
            if ((srcColor && ((srcChannel & 0x5) == 0x5)) ||
                (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x5 << 0, dstLR->hwType)))
            {
                *shift = 0;
                retValue = gcvTRUE;
            }
        }
        /* See if x-component is available. */
        else if ((srcColor && ((srcChannel & 0x1) == 0x1)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 0, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if y-component is available. */
        else if  (!restricted &&
                 ((srcColor && ((srcChannel & 0x2) == 0x2)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 1, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        /* See if z-component is available. */
        else if  (!restricted &&
                 ((srcColor && ((srcChannel & 0x4) == 0x4)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 2, dstLR->hwType))))
        {
            *shift = 2;
            retValue = gcvTRUE;
        }
        /* See if w-component is available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0x8) == 0x8)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 3, dstLR->hwType))))
        {
            *shift = 3;
            retValue = gcvTRUE;
        }
        break;

    case VIR_ENABLE_Y:
        /* See if y and w-component are available. */
        if (isLRHighpVec2(dstLR))
        {
            /*check y and w are avaiable*/
            if ((srcColor && ((srcChannel & 0xA) == 0xA)) ||
                (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0xA << 0, dstLR->hwType)))
            {
                *shift = 0;
                retValue = gcvTRUE;
            }
        }
        else if ((srcColor && ((srcChannel & 0x2) == 0x2)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 1, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if z-component is available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0x4) == 0x4)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 2, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        /* See if w-component is available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0x8) == 0x8)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 3, dstLR->hwType))))
        {
            *shift = 2;
            retValue = gcvTRUE;
        }
        break;
    case VIR_ENABLE_Z:
        /* See if z-component is available. */
        if ((srcColor && ((srcChannel & 0x4) == 0x4)) ||
             (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 2, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if w-component is available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0x8) == 0x8)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 3, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        break;
    case VIR_ENABLE_W:
        /* See if w-component is available. */
        if ((srcColor && ((srcChannel & 0x8) == 0x8)) ||
             (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x1 << 3, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        break;
    case VIR_ENABLE_XY:
        /* check xyzw are avaible */
        if (isLRHighpVec2(dstLR))
        {
            if ((srcColor && ((srcChannel & 0xF) == 0xF)) ||
                (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0xF << 0, dstLR->hwType)))
            {
                *shift = 0;
                retValue = gcvTRUE;
            }
        }
        /* See if x- and y-components are available. */
        else if ((srcColor && ((srcChannel & 0x3) == 0x3)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 0, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if y- and z-components are available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0x6) == 0x6)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 1, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        /* See if z- and w-components are available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0xC) == 0xC)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 2, dstLR->hwType))))
        {
            *shift = 2;
            retValue = gcvTRUE;
        }
        break;

    case VIR_ENABLE_YZ:
        /* See if y- and z-components are available. */
        if ((srcColor && ((srcChannel & 0x6) == 0x6)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 1, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if z- and w-components are available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0xC) == 0xC)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 2, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        break;

    case VIR_ENABLE_ZW:
        if  ((srcColor && ((srcChannel & 0xC) == 0xC)) ||
             (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x3 << 2, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        break;
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XZ:
        /* See if x-, y- and z-components are available. */
        if ((srcColor && ((srcChannel & 0x7) == 0x7)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x7 << 0, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        /* See if y-, z- and w-components are available. */
        else if (!restricted &&
                 ((srcColor && ((srcChannel & 0xE) == 0xE)) ||
                 (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x7 << 1, dstLR->hwType))))
        {
            *shift = 1;
            retValue = gcvTRUE;
        }
        break;

    case VIR_ENABLE_YZW:
    case VIR_ENABLE_YW:
        /* See if y-, z- and w-components are available. */
        if ((srcColor && ((srcChannel & 0xE) == 0xE)) ||
            (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0x7 << 1, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        break;

    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_XW:
        /* See if x-, y-, z- and w-components are available. */
        if ((srcColor && ((srcChannel & 0xF) == 0xF)) ||
             (!srcColor && _VIR_RA_LS_ChannelAvail(pRA, regno, 0xF << 0, dstLR->hwType)))
        {
            *shift = 0;
            retValue = gcvTRUE;
        }
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return retValue;
}

/* In general, the new color is got from the colorMap from smallest available color
   In some cases, we need to choose a prefered color.
   for example,
   MOV dest is better to be the same as src0  */
VIR_RA_HWReg_Color
_VIR_RA_LS_GetPreferedColor(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pDefInst,
    VIR_RA_LS_Liverange *pLR,
    gctBOOL             needHI)
{
    VIR_RA_HWReg_Color  retValue;
    gctUINT             srcWebIdx;
    VIR_RA_LS_Liverange *pSrcLR;

    retValue = InvalidColor;

    if (VIR_Inst_GetOpcode(pDefInst) == VIR_OP_MOV)
    {
        srcWebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pDefInst, VIR_Inst_GetSource(pDefInst, 0));
        if (srcWebIdx != VIR_INVALID_WEB_INDEX && pLR->regNoRange == 1)
        {
            pSrcLR = _VIR_RA_LS_Web2ColorLR(pRA, srcWebIdx);

            if (VIR_Enable_Covers(VIR_RA_LS_LR2WebChannelMask(pRA, pSrcLR), VIR_RA_LS_LR2WebChannelMask(pRA, pLR)))
            {
                retValue = _VIR_RA_GetLRColor(pSrcLR);
                if (_VIR_RA_LS_IsInvalidLOWColor(retValue) ||
                    _VIR_RS_LS_IsSpecialReg(_VIR_RA_Color_RegNo(retValue)) ||
                    _VIR_RA_LS_TestUsedColor(pRA,
                        pLR->hwType,
                        _VIR_RA_Color_RegNo(retValue),
                        _VIR_RA_Color_Channels(VIR_RA_LS_LR2WebChannelMask(pRA, pSrcLR), _VIR_RA_Color_Shift(retValue))) ||
                    (isLRNoShift(pLR) && _VIR_RA_Color_Shift(retValue) != 0))
                {
                    /* MOV is better to be src0 color */
                    retValue = InvalidColor;
                }

                if (needHI &&
                    (_VIR_RA_LS_IsInvalidHIColor(retValue) ||
                     _VIR_RA_LS_TestUsedColor(pRA,
                        pLR->hwType,
                        _VIR_RA_Color_HIRegNo(retValue),
                        _VIR_RA_Color_Channels(VIR_RA_LS_LR2WebChannelMask(pRA, pSrcLR), _VIR_RA_Color_HIShift(retValue))) ||
                     (isLRNoShift(pLR) && _VIR_RA_Color_HIShift(retValue) != 0)))
                {
                    retValue = InvalidColor;
                }
            }
        }
    }
    return retValue;
}

/* return VIR_RA_INVALID_COLOR if there is no color to assign
   i.e., all usedColor are set,
   in dual16, the high precision temp needs two registers */
VIR_RA_HWReg_Color _VIR_RA_LS_FindNewColor(
    VIR_RA_LS   *pRA,
    gctUINT     webIdx,
    gctBOOL     needHI,
    gctUINT     reservedDataReg,
    gctINT      startReg,
    gctBOOL     *pPreferedColorOnly)
{
    VIR_RA_HWReg_Color  retValue;
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    VIR_WEB     *pWeb;
    VIR_DEF     *pDef;
    VIR_Symbol  *pSym;
    gctUINT     LRShift = 0, LRShiftHI = 0;

    gctBOOL     preferedColorOnly = gcvFALSE;

    retValue = InvalidColor;

    pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    gcmASSERT(pWeb);
    gcmASSERT(pWeb->firstDefIdx != VIR_INVALID_DEF_INDEX);
    pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, pWeb->firstDefIdx);
    gcmASSERT(pDef);
    gcmASSERT(pDef->defKey.pDefInst->dest);
    pSym = VIR_Operand_GetSymbol(pDef->defKey.pDefInst->dest);

    if (pSym)
    {
        if (VIR_Symbol_isVreg(pSym) && VIR_Symbol_GetVregVariable(pSym))
        {
            pSym = VIR_Symbol_GetVregVariable(pSym);
        }
    }

    /* special handling for special symbols: depth*/
    if (pSym &&
        VIR_Symbol_isVariable(pSym) &&
        VIR_Symbol_GetName(pSym) == VIR_NAME_DEPTH)
    {
        gcmASSERT(VIR_RA_LS_GetShader(pRA)->shaderKind == VIR_SHADER_FRAGMENT);
        _VIR_RA_MakeColor(0, CHANNEL_Z, &retValue);
        if (needHI)
        {
            _VIR_RA_MakeColorHI(1, CHANNEL_Z, &retValue);
        }
        else
        {
            _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &retValue);
        }
    }
    else
    {
        gctUINT regno = pCP->colorMap[pLR->hwType].availReg;
        gctUINT i, allRegsFit = 0, shift = 0;

        if (startReg != -1)
        {
            regno = startReg;
        }

        /* choosing prefered color */
        retValue = _VIR_RA_LS_GetPreferedColor(pRA, pDef->defKey.pDefInst, pLR, needHI);
        preferedColorOnly = gcvTRUE;

        if (!_VIR_RA_LS_IsInvalidLOWColor(retValue) &&
            !(needHI && _VIR_RA_LS_IsInvalidHIColor(retValue)) &&
            (startReg == -1 || (gctINT) retValue._hwRegId >= startReg) )
        {
            goto OnRet;
        }
        else if (needHI && pLR->regNoRange > 1)
        {
            gcmASSERT(!isLRVXEven(pLR) && !isLRHighpVec2(pLR));

            /* highp array is assigned as consecutive register pairs */
            while (regno < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
            {
                allRegsFit = 0;
                if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regno, &LRShift))
                {
                    allRegsFit++;
                    regno++;

                    for (i = 1; i < pLR->regNoRange * 2; i++)
                    {
                        if (regno < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
                        {
                            if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regno, &shift) &&
                                (shift == LRShift))
                            {
                                allRegsFit++;
                                regno++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (allRegsFit == pLR->regNoRange * 2)
                    {
                        regno = regno - pLR->regNoRange * 2;
                        _VIR_RA_MakeColor(regno, LRShift, &retValue);
                        _VIR_RA_MakeColorHI(regno + 1, LRShift, &retValue);

                        preferedColorOnly = gcvFALSE;
                        break;
                    }
                }
                else
                {
                    regno++;
                }
            }
        }
        else
        {
            while (regno < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
            {
                allRegsFit = 0;
                if (isLRVXEven(pLR))
                {
                    if ((regno & 0x01) == 1)
                    {
                        regno ++;
                    }
                }
                if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regno, &LRShift))
                {
                    allRegsFit++;
                    regno++;

                    for (i = 1; i < pLR->regNoRange; i++)
                    {
                        if (regno < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
                        {
                            if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regno, &shift) &&
                                (shift == LRShift))
                            {
                                allRegsFit++;
                                regno++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (allRegsFit == pLR->regNoRange)
                    {
                        /* B0 only has one component */
                        if (!(isLRB0(pLR) && LRShift > 0))
                        {
                            regno = regno - pLR->regNoRange;
                            _VIR_RA_MakeColor(regno, LRShift, &retValue);

                            preferedColorOnly = gcvFALSE;
                            break;
                        }
                    }
                }
                else
                {
                    regno++;
                }
            }

            if (needHI && !_VIR_RA_LS_IsInvalidLOWColor(retValue))
            {
                gctUINT regnoHI;

                if (pLR->hwType ==  VIR_RA_HWREG_GR)
                {
                    if (isLRHighpVec2(pLR))
                    {
                        gcmASSERT(pLR->regNoRange == 1);
                        gcmASSERT(retValue._hwShift <= 1);
                        _VIR_RA_MakeColorHI(regno, ((retValue._hwShift) + 2), &retValue);
                        preferedColorOnly = gcvFALSE;
                    }
                    else
                    {
                        regnoHI = regno + pLR->regNoRange;
                        while (regnoHI < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
                        {
                            allRegsFit = 0;
                            if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regnoHI, &LRShiftHI))
                            {
                                allRegsFit++;
                                regnoHI++;

                                for (i = 1; i < pLR->regNoRange; i++)
                                {
                                    if (regnoHI < _VIR_RA_LS_GetMaxReg(pRA, pLR->hwType, reservedDataReg))
                                    {
                                        if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regnoHI, &shift) &&
                                            (shift == LRShiftHI))
                                        {
                                            allRegsFit++;
                                            regnoHI++;
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                if (allRegsFit == pLR->regNoRange)
                                {
                                    /* hihgp array is assigned as an array of a pair of registers*/
                                    regnoHI = regnoHI - pLR->regNoRange;
                                    _VIR_RA_MakeColorHI(regnoHI, LRShiftHI, &retValue);

                                    preferedColorOnly = gcvFALSE;
                                    break;
                                }
                            }
                            else
                            {
                                regnoHI++;
                            }
                        }
                    }
                }
                else
                {
                    VIR_Enable A0Enable = VIR_RA_LS_LR2WebChannelMask(pRA, pLR);
                    gctINT    j = 0, allChannelsFit = 0;

                    /* B0 should not needHI, since it only has one component */
                    gcmASSERT(isLRA0(pLR));
                    gcmASSERT(pLR->regNoRange == 1);
                    regnoHI = regno;
                    LRShiftHI = VIR_Enable_Channel_Count(A0Enable) + LRShift;
                    while (LRShiftHI <= 3)
                    {
                        for (j = 0; j < VIR_CHANNEL_COUNT; j++)
                        {
                            if (A0Enable & (0x1 << j))
                            {
                                if (LRShiftHI + j <= 3 &&
                                    _VIR_RA_LS_ChannelAvail(pRA, regnoHI, 0x1 << (LRShiftHI + j), VIR_RA_HWREG_A0))
                                {
                                    allChannelsFit ++;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

                        if (allChannelsFit == VIR_Enable_Channel_Count(A0Enable))
                        {
                            break;
                        }

                        LRShiftHI++;
                    }
                    if (LRShiftHI <= 3)
                    {
                        _VIR_RA_MakeColorHI(regnoHI, LRShiftHI, &retValue);

                        preferedColorOnly = gcvFALSE;
                    }
                }
            }
        }
    }

OnRet:
    /* find a new color */
    if (!_VIR_RA_LS_IsInvalidLOWColor(retValue) &&
        !(needHI && _VIR_RA_LS_IsInvalidHIColor(retValue)))
    {
        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "find new ");
            _VIR_RA_LS_DumpColor(pRA, retValue, pLR);
            VIR_LOG(pDumper, " for LR%d\n", pLR->webIdx);
            VIR_LOG_FLUSH(pDumper);
        }

        if (isLRA0(pLR))
        {
            /* when all 4 A0 channels are used, we need to keep ldarr (as a move) */
            if (LRShift == 3)
            {
                pLR->endPoint = pLR->origEndPoint;
            }
        }
    }
    else
    {
        retValue = InvalidColor;
    }

    if (pPreferedColorOnly)
    {
        *pPreferedColorOnly = preferedColorOnly;
    }

    return retValue;
}

/* return true if the LR is spillable */
gctBOOL _VIR_RA_LS_isSpillable(
    VIR_RA_LS               *pRA,
    VIR_RA_LS_Liverange     *pLR
    )
{
    VIR_Shader  *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Symbol  *sym;

    /* input should not be spilled,
       used color should not be spilled */
    if (pLR->deadIntervalAvail == gcvFALSE ||
        pLR->usedColorLR != gcvNULL  ||
        pLR->colorFunc  == VIR_RA_LS_ATTRIBUTE_FUNC
        )
    {
        return gcvFALSE;
    }

    sym = VIR_Shader_FindSymbolByTempIndex(pShader, pLR->firstRegNo);

    /* output should not be spilled */
    if (sym)
    {
        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
        {
            sym = VIR_Symbol_GetVregVariable(sym);
        }
        if (VIR_Symbol_isOutput(sym))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

/* choose a candidate LR in active LR list to spill or
   choose the LR that is going to be colored to spill
   return ERR if there is no suitable LR to spill */
VSC_ErrCode
_VIR_RA_LS_SpillRegister(
    VIR_RA_LS           *pRA,
    gctUINT             webIdx,
    VIR_Function        *pFunc,
    VIR_RA_HWReg_Color  *spillColor)
{
    VSC_ErrCode         retvalue = VSC_ERR_NONE;
    VIR_RA_HWReg_Color  invalidColor;
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
    VIR_RA_LS_Liverange *pTmpLR, *pPrev;

    invalidColor = InvalidColor;

    /* choose a spill candidate based on heuristics - weight and etc */
    pPrev = VIR_RA_LS_GetActiveLRHead(pRA);
    pTmpLR = pPrev->nextActiveLR;
    while (pTmpLR != &LREndMark)
    {
        if (_VIR_RA_LS_isSpillable(pRA, pTmpLR) &&
            (pTmpLR->hwType == pLR->hwType) &&
            (pTmpLR->regNoRange >= pLR->regNoRange) &&
            VIR_Enable_Covers(VIR_RA_LS_LR2WebChannelMask(pRA, pTmpLR),
                              VIR_RA_LS_LR2WebChannelMask(pRA, pLR)) &&
            pTmpLR->weight < pLR->weight
            )
        {
            break;
        }
        pPrev = pTmpLR;
        pTmpLR = pPrev->nextActiveLR;
    }

    if (pTmpLR != &LREndMark)
    {
        /* choose pTmpLR to spill */
        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "choose LR%d to spill", pTmpLR->webIdx);
            VIR_LOG_FLUSH(pDumper);
        }

       *spillColor = _VIR_RA_GetLRColor(pTmpLR);

       /* remove pTmpLR from the active LR list */
        _VIR_RA_LS_RemoveLRfromActiveList(pRA, pPrev, pTmpLR);

        /* assign invalid color (i.e., spill) to pTmpLR */
        _VIR_RA_LS_AssignColorWeb(pRA, pTmpLR->webIdx, pTmpLR->hwType, invalidColor, pFunc);
    }
    else
    {
        if (_VIR_RA_LS_isSpillable(pRA, pLR))
        {
            /* choose pLR to spill */
            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
            {
                VIR_LOG(pDumper, "choose LR%d to spill", pLR->webIdx);
                VIR_LOG_FLUSH(pDumper);
            }
        }
        else
        {
            /* could not find a spill candidate */
            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
            {
                VIR_LOG(pDumper, "could not find spill candidates!!!");
                VIR_LOG_FLUSH(pDumper);
            }

            retvalue = VSC_RA_ERR_OUT_OF_REG_FAIL;
        }
    }

    return retvalue;
}

/* return gcvTRUE if dstLR can fit into any of srcLR's dead interval */
gctBOOL _VIR_RA_LS_FitLR(
    VIR_RA_LS               *pRA,
    VIR_RA_LS_Liverange     *srcLR,
    VIR_RA_LS_Liverange     *dstLR,
    gctUINT                 *dstShift)
{
    gctBOOL     retValue = gcvFALSE;
    VIR_RA_LS_Interval  *pInterval;

    /* regno is no use in this case */
    gctUINT     regno = 0;
    if ((srcLR->deadIntervalAvail) &&
        (dstLR->hwType == srcLR->hwType) &&
        _VIR_RA_LS_ChannelFit(pRA, dstLR, srcLR, regno, dstShift))
    {
        pInterval = srcLR->deadIntervals;
        while (pInterval)
        {
            /* if the dstLR is within srcLR's dead interval */
            if ((dstLR->startPoint >= pInterval->startPoint) &&
                (dstLR->endPoint <= pInterval->endPoint))
            {
                retValue = gcvTRUE;
                break;
            }
            pInterval = pInterval->next;
        }
    }

    return  retValue;
}

gctBOOL _VIR_RS_LS_ColorOverLapping(
    VIR_RA_LS              *pRA,
    VIR_RA_LS_Liverange    *pLR1,
    VIR_RA_LS_Liverange    *pLR2)
{
    VIR_RA_HWReg_Color     color1 = _VIR_RA_GetLRColor(pLR1);
    VIR_RA_HWReg_Color     color2 = _VIR_RA_GetLRColor(pLR2);

    if (_VIR_RA_Color_RegNo(color1) !=  _VIR_RA_Color_RegNo(color2))
    {
        return gcvFALSE;
    }

    if (((_VIR_RA_Color_Channels(VIR_RA_LS_LR2WebChannelMask(pRA, pLR1),
                                _VIR_RA_Color_Shift(color1)))
         & (_VIR_RA_Color_Channels(VIR_RA_LS_LR2WebChannelMask(pRA, pLR2),
                                  _VIR_RA_Color_Shift(color2)))) == 0)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL _VIR_RA_LS_ActiveLRHaveSameColor(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR)
{
    VIR_RA_LS_Liverange     *pTmpLR;

    pTmpLR = VIR_RA_LS_GetActiveLRHead(pRA)->nextActiveLR;
    while (pTmpLR != &LREndMark)
    {
        if (pTmpLR != pLR &&
            _VIR_RS_LS_ColorOverLapping(pRA, pTmpLR, pLR))
        {
            return gcvTRUE;
        }
        pTmpLR = pTmpLR->nextActiveLR;
    }
    return gcvFALSE;
}

/* return VIR_RA_INVALID_COLOR if could not find a used color to assign
   we can assign a color used by srcLR iff the dstLR can fit into any of
   srcLR's dead intervals */
VIR_RA_HWReg_Color _VIR_RA_LS_FindUsedColor(
    VIR_RA_LS           *pRA,
    gctUINT             webIdx,
    VIR_RA_LS_Liverange **ppUsedColorLR)
{
    VIR_RA_HWReg_Color retValue;

    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

    VIR_RA_LS_Liverange     *pActive;
    gctUINT                 LRShift = 0;

    retValue = InvalidColor;

    /* it could be tricky here. the active lists could have two LRs that have
       overlapping color.
       For example:
       foo:
       1: temp(5).x = temp(3).x + 1;
       temp(5).x and temp(3).x can be assigned to the same r0.x

       main:
       1: temp(3).x = 2;
       ..
       3: call @ foo; (temp(5).x = temp(3).x + 1;)
       4:  = temp(5).x;
       ...
       6: temp(3).x = 5;
       7: temp(9).x = 6;
       8:  = temp(9).x;
       9: call @ foo; (temp(5).x = temp(3).x + 1;)
       10:  = temp(5).x;
       temp(5).x dead range[5-9]
       temp(3).x dead range[4-6]
       temp(9).x can fit into temp(5)'s dead interval,
       but could not fit temp(3).x's dead interval.
       so temp(9).x should NOT assigned to r0.x.
       The optimal way is to compare the live range
       with all same color's LR's dead interval. For simplicity, we just don't use
       this used color if it is used by to active LRs.
       */
    pActive = VIR_RA_LS_GetActiveLRHead(pRA)->nextActiveLR;
    while ((pActive != &LREndMark) &&
           (!_VIR_RA_LS_FitLR(pRA, pActive, pLR, &LRShift) ||
           _VIR_RA_LS_ActiveLRHaveSameColor(pRA, pActive)  ||
           (pShader->shaderKind == VIR_SHADER_FRAGMENT &&
            VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5 &&
            _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pActive)) == 0 &&
            (LRShift == 0 || LRShift == 1))))
    {
        pActive = pActive->nextActiveLR;
    }

    /* find a color from an active LR whose dead interval can cover this LR */
    if (pActive != &LREndMark)
    {
        _VIR_RA_MakeColor(_VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pActive)),
                               LRShift,
                               &retValue);
        _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &retValue);

        *ppUsedColorLR = pActive;

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "find used ");
            _VIR_RA_LS_DumpColor(pRA, retValue, pLR);
            VIR_LOG(pDumper, "for LR%d\n", pLR->webIdx);
            VIR_LOG_FLUSH(pDumper);
        }
    }

    return retValue;
}

static gctUINT _VIR_RA_Check_First_Unused_Pos_Attr_Channel(VIR_RA_LS  *pRA)
{
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    gctUINT                 currAttr;
    VIR_DEF_KEY             defKey;
    gctUINT                 defIdx;
    VIR_DEF*                pDef;
    gctUINT8                currChannel;
    gctUINT                 firstUnusedChannel = NOT_ASSIGNED;
    gctBOOL                 bHasPosition = gcvFALSE;

    for (currAttr = 0; currAttr < VIR_IdList_Count(pAttrs); currAttr ++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, currAttr));

        if (VIR_Symbol_GetName(attribute) == VIR_NAME_POSITION)
        {
            defKey.pDefInst = VIR_INPUT_DEF_INST;
            defKey.regNo = VIR_Symbol_GetVariableVregIndex(attribute);

            for (currChannel = VIR_CHANNEL_Z; currChannel < VIR_CHANNEL_NUM; currChannel++)
            {
                defKey.channel = currChannel;

                defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
                if (VIR_INVALID_DEF_INDEX == defIdx)
                {
                    firstUnusedChannel = currChannel;
                    break;
                }
                else
                {
                    pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                    gcmASSERT(pDef);

                    if (DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0)
                    {
                        firstUnusedChannel = currChannel;
                        break;
                    }
                }
            }

            bHasPosition = gcvTRUE;
            break;
        }
    }

    return bHasPosition ? firstUnusedChannel : CHANNEL_Z;
}

/* ===========================================================================
   _VIR_RA_LS_AssignAttributes
   assign colors for attributes
   ===========================================================================
*/
/* primitiveId is load from per-patch data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Patch_r0(
    VIR_RA_LS       *pRA,
    VIR_Symbol      *primitiveIdSym,
    VIR_HwRegId     hwRegNo);

/* return gcvTRUE if the attribute is a builtin and is put in a
   pre-defined register */
gctBOOL _VIR_RA_LS_handleBuiltinAttr(
    VIR_RA_LS_Liverange *pLR,
    VIR_Shader      *pShader,
    VIR_Symbol      *pAttr,
    gctUINT         *regNo,
    gctUINT8        *shift)
{
    gctBOOL     retValue = gcvFALSE;

    if (VIR_Symbol_GetName(pAttr) == VIR_NAME_POSITION ||
        VIR_Symbol_GetName(pAttr) == VIR_NAME_FRONT_FACING ||
        VIR_Symbol_GetName(pAttr) == VIR_NAME_HELPER_INVOCATION)
    {
        if (regNo && shift)
        {
            *regNo = 0;
            *shift = 0;
        }
        retValue = gcvTRUE;
    }
    else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_VERTEX_ID ||
             VIR_Symbol_GetName(pAttr) == VIR_NAME_VERTEX_INDEX)
    {
        if (regNo && shift)
        {
            /* not set regNo, VIR_NAME_VERTEX_ID/VIR_NAME_INSTANCE_ID will
               be the last used HW register for attributes */
            *shift = 0;
        }
        retValue = gcvTRUE;
    }
    else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_INSTANCE_ID)
    {
        if (regNo && shift)
        {
            /* not set regNo, VIR_NAME_VERTEX_ID/VIR_NAME_INSTANCE_ID will
               be the last used HW register for attributes */
            *shift = 1;
        }
        retValue = gcvTRUE;
    }
    else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_SAMPLE_ID ||
             VIR_Symbol_GetName(pAttr) == VIR_NAME_SAMPLE_MASK_IN ||
             VIR_Symbol_GetName(pAttr) == VIR_NAME_SAMPLE_POSITION ||
             (pLR && isLRSubSampleDepth(pLR)))
    {
        if (regNo && shift)
        {
            /* not set regNo, they will be the in special HW register for attributes */
            *shift = 0;
        }
        retValue = gcvTRUE;
    }
    /* TCS specific */
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        if (VIR_Symbol_GetName(pAttr) == VIR_NAME_PRIMITIVE_ID)
        {
            if (regNo && shift)
            {
                *regNo = 0;
                *shift = 2;
            }
            retValue = gcvTRUE;
        }
        else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_INVOCATION_ID)
        {
            if (regNo && shift)
            {
                *regNo = 0;
                *shift = 1;
            }
            retValue = gcvTRUE;
        }
    }
    /* TES specific */
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        if (VIR_Symbol_GetName(pAttr) == VIR_NAME_TESS_COORD)
        {
            if (regNo && shift)
            {
                *regNo = 0;
                *shift = 0;
            }
            retValue = gcvTRUE;
        }
    }
    /* GS specific */
    else if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        if (VIR_Symbol_GetName(pAttr) == VIR_NAME_INVOCATION_ID)
        {
            if (!VIR_Shader_HasRestartOrStreamOut(pShader))
            {
                /* instanceId in r0.z*/
                if (regNo && shift)
                {
                    *regNo = 0;
                    *shift = 2;
                }
            }
            else
            {
                /* instanceId in r0.w*/
                if (regNo && shift)
                {
                    *regNo = 0;
                    *shift = 3;
                }
            }
            retValue = gcvTRUE;
        }
        else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_PRIMITIVE_ID_IN)
        {
            if (!VIR_Shader_HasRestartOrStreamOut(pShader) &&
                !VIR_Shader_HasInstanceId(pShader))
            {
                /*primitiveId in r0.z */
                if (regNo && shift)
                {
                    *regNo = 0;
                    *shift = 2;
                }
            }
            else if (VIR_Shader_HasRestartOrStreamOut(pShader) &&
                     !VIR_Shader_HasInstanceId(pShader))
            {
                /*primitiveId in r0.w */
                if (regNo && shift)
                {
                    *regNo = 0;
                    *shift = 3;
                }
            }
            else
            {
                /*primitiveId in r1.x */
                if (regNo && shift)
                {
                    *regNo = 1;
                    *shift = 0;
                }
            }
            retValue = gcvTRUE;
        }
    }

    return retValue;
}

gctBOOL _VIR_RA_NeedHighpPosition(
    VIR_Shader      *pShader)
{
    gctUINT                 inputIdx, outputIdx;
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    VIR_AttributeIdList     *pOutputs = VIR_Shader_GetOutputs(pShader);

    for (inputIdx = 0; inputIdx < VIR_IdList_Count(pAttrs); inputIdx ++)
    {
        VIR_Symbol  *pInputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, inputIdx));

        if (!isSymUnused(pInputSym) && !isSymVectorizedOut(pInputSym))
        {
            if (VIR_Symbol_GetName(pInputSym) == VIR_NAME_POSITION ||
                VIR_Symbol_GetName(pInputSym) == VIR_NAME_POSITION_W ||
                VIR_Symbol_GetName(pInputSym) == VIR_NAME_IN_POSITION ||
                VIR_Symbol_GetName(pInputSym) == VIR_NAME_LAYER)
            {
                return gcvTRUE;
            }
        }
    }

    for (outputIdx = 0; outputIdx < VIR_IdList_Count(pOutputs); outputIdx ++)
    {
        VIR_Symbol  *pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputs, outputIdx));

        if (!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym))
        {
            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_DEPTH ||
                VIR_Symbol_GetName(pOutputSym) == VIR_NAME_PS_OUT_LAYER)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

void _VIR_RA_FillPsInputPosPCCompValid(
    VIR_RA_LS       *pRA,
    VIR_Symbol      *attribute)
{
    VIR_Shader        *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_DEF_KEY        defKey;
    VIR_DEF*           pDef;
    gctUINT            defIdx;
    gctUINT8           channel;

    if (VIR_Symbol_GetName(attribute) == VIR_NAME_POSITION)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            defKey.pDefInst = VIR_INPUT_DEF_INST;
            defKey.regNo = VIR_Symbol_GetVariableVregIndex(attribute);
            defKey.channel = channel;
            defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                gcmASSERT(pDef);

                if (DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) != 0)
                {
                    pShader->psInputPosCompValid[channel] = gcvTRUE;
                }
            }
        }
    }

    if (VIR_Symbol_GetName(attribute) == VIR_NAME_POINT_COORD)
    {
        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
        {
            defKey.pDefInst = VIR_INPUT_DEF_INST;
            defKey.regNo = VIR_Symbol_GetVariableVregIndex(attribute);
            defKey.channel = channel;
            defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                gcmASSERT(pDef);

                if (DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) != 0)
                {
                    pShader->psInputPCCompValid[channel] = gcvTRUE;
                }
            }
        }
    }
}

static gctBOOL
_VIR_RA_ShaderEnableDepth(
    VIR_Shader *pShader
    )
{
    gctUINT                 outputIdx;
    VIR_AttributeIdList     *pOutputs = VIR_Shader_GetOutputs(pShader);
    gctBOOL                 bHasOutputColor = gcvFALSE;

    gcmASSERT(pShader->shaderKind == VIR_SHADER_FRAGMENT);

    for (outputIdx = 0; outputIdx < VIR_IdList_Count(pOutputs); outputIdx ++)
    {
        VIR_Symbol  *pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputs, outputIdx));
        VIR_NameId  outputNameId = VIR_Symbol_GetName(pOutputSym);

        if (!isSymUnused(pOutputSym))
        {
            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_DEPTH)
            {
                return gcvTRUE;
            }
            /*
            ** If there is no color output in a fragment shader, driver still enables depth no matter
            ** gl_FragDepth is used or not. In this case, we need to use r0.w for sampleMask.
            ** PS, by default, if a output is not a built-in output, then it should be a color output.
            */
            else if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_COLOR || !VIR_Shader_IsNameBuiltIn(pShader, outputNameId))
            {
                bHasOutputColor = gcvTRUE;
            }
        }
    }

    if (!bHasOutputColor)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctUINT
_VIR_RA_LS_GetStartReg(
    VIR_RA_LS       *pRA)
{
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    gctUINT startReg = 0;

    switch (pShader->shaderKind)
    {
    case VIR_SHADER_VERTEX:
        /* Start at register 0 for vertex shaders */
        startReg = 0;
        break;
    case VIR_SHADER_FRAGMENT:
        /* Start at register 1 for fragment shaders,
           0 is reserved for builtin attributes,
           in dual16 mode, r0, r1 is position if position is in the shader
           */
        if (VIR_Shader_PS_NeedSampleMaskId(pShader))
        {
            gctUINT unusedChannel = _VIR_RA_Check_First_Unused_Pos_Attr_Channel(pRA);

            /* sepcial symbol depth will use r0.z and reset unusedChannel to next channel */
            if (_VIR_RA_ShaderEnableDepth(pRA->pShader) && (unusedChannel == CHANNEL_Z))
            {
                unusedChannel = CHANNEL_W;
            }

            /* In dual16, Sample mask should always be in HIGHP.
               Because,
               o   Medium position mode doesn’t support mask LOCATION_W  (can’t)and LOCATION_Z (make it simple).
               o   Sample depth is always high precision. So LOCATION_SUB_Z is also high precision.
               After 620, HW has a fix to make LOCATION_Z work */
            if (VIR_Shader_isDual16Mode(pShader))
            {
                /* after HW fix, we can put sampleMask in LOCATION_Z */
                VSC_HW_CONFIG   *hwConfig = VIR_RA_LS_GetHwCfg(pRA);
                if (hwConfig->hwFeatureFlags.hasSampleMaskInR0ZWFix &&
                    unusedChannel == CHANNEL_Z)
                {
                    pShader->sampleMaskIdRegStart = 0;
                    pShader->sampleMaskIdChannelStart = CHANNEL_Z;
                }
                else
                {
                    /* sampleMaskIdRegStart will be changed to last temp */
                    pShader->sampleMaskIdRegStart = VIR_REG_MULTISAMPLEDEPTH;
                    pShader->sampleMaskIdChannelStart = CHANNEL_X;
                }
            }
            else
            {
                /* sampleMaskIdRegStart will be changed to last temp or r0.z/ro.w */
                pShader->sampleMaskIdRegStart = (unusedChannel == NOT_ASSIGNED) ? VIR_REG_MULTISAMPLEDEPTH : 0;
                pShader->sampleMaskIdChannelStart = (unusedChannel == NOT_ASSIGNED) ? CHANNEL_X : unusedChannel;
            }
        }

        if (VIR_Shader_isDual16Mode(pShader) && _VIR_RA_NeedHighpPosition(pShader))
        {
            startReg = 2;
        }
        else
        {
            startReg = 1;
        }

        pShader->psRegStartIndex = startReg;

        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        {
            /* For TCS
               r0.x remap addr for output vertex
               r0.y InvocationID
               r0.z primitiveID
               r0.w per-patch addr
               r1 - rx remap for input vertex
               r(x+1) - ry remap for output vertex (if there is read of output vertex)

               This is tricky here:
               we assume the count of input vertex is the max of the count of output vertex
               and the max const index of input vertex access.
               If at runtime, the input vertex count is different than this assumed count,
               we need recompilation!!
               */
            gctINT  inputCount = ((pShader->shaderLayout.tcs.tcsPatchInputVertices - 1) / 8 + 1);
            gctINT  outputCount = ((pShader->shaderLayout.tcs.tcsPatchOutputVertices - 1) / 8 + 1);

            gcmASSERT(pShader->shaderLayout.tcs.tcsPatchInputVertices > 0);

            /* if both input and output vertex are less than 4, we pack their regmap in one register. */
            if ((pShader->shaderLayout.tcs.tcsPatchInputVertices <= 4) &&
                (pShader->shaderLayout.tcs.tcsPatchOutputVertices <= 4))
            {
                VIR_Shader_SetFlag(pShader, VIR_SHFLAG_TCS_USE_PACKED_REMAP);
                /* input regmap and output regmap are packed in one register */
                startReg = 1 + 1;
            }
            else
            {
                startReg = 1 + inputCount + (pShader->shaderLayout.tcs.hasOutputVertexAccess ? outputCount : 0);
            }

            /* TCS remap always start from r1 */
            pShader->remapRegStart = 1;
            pShader->remapChannelStart = CHANNEL_X;
        }
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        {
            /* For TES
               r0.x Tess.Coordinate u
               r0.y Tess.Coordinate v
               r0.z Tess.Coordinate w
               r0.w output vertex addr and per-patch addr
               r1-rx remap for input vertex
            */
            gctINT  inputCount = ((pShader->shaderLayout.tes.tessPatchInputVertices -1) / 8 + 1);
            startReg = 1 + inputCount;

            /* TES remap always start from r1 */
            pShader->remapRegStart = 1;
            pShader->remapChannelStart = CHANNEL_X;
        }
        break;
    case VIR_SHADER_GEOMETRY:
        /* For GS
          r0.x meta data address and current vertex address
          r0.y vertex counter
          r0.z Instance ID (if no restart or stream out and has instance id)
               Primitive ID (if no restart or stream out and has Primitive id)
               Restart flags and Stream out indices (if has restart or stream out)
          r0.w instance ID (if has restart or stream out and instance id)
               Primitive ID (if has restart or stream out and Primitive id)
          r1.x Primitive ID (if has restart or stream out, instance id and Primitive id)
          r0.z, r1.x, remap for input
          r1.z, r2.x */

        if (!VIR_Shader_HasRestartOrStreamOut(pShader) &&
            !VIR_Shader_HasInstanceId(pShader) &&
            !VIR_Shader_HasPrimitiveId(pShader))
        {
            /* points 1 vertex, lines 2 vertices, line adjacency 4 vertices,
               triangles 3 vertices, triangle adjacency 6 vertices;
               one components can fit 2 remap */
            if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
            {
                pShader->remapRegStart = 1;
                pShader->remapChannelStart = CHANNEL_X;
                startReg = 2;
            }
            else
            {
                pShader->remapRegStart = 0;
                pShader->remapChannelStart = CHANNEL_Z;
                startReg = 1;
            }
        }
        else if (VIR_Shader_HasRestartOrStreamOut(pShader) &&
                  VIR_Shader_HasInstanceId(pShader) &&
                  VIR_Shader_HasPrimitiveId(pShader))
        {
            if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
            {
                pShader->remapRegStart = 2;
                pShader->remapChannelStart = CHANNEL_X;
                startReg = 3;
            }
            else
            {
                pShader->remapRegStart = 1;
                pShader->remapChannelStart = CHANNEL_Z;
                startReg = 2;
            }
        }
        else
        {
            pShader->remapRegStart = 1;
            pShader->remapChannelStart = CHANNEL_X;
            startReg = 2;
        }
        break;
    case VIR_SHADER_COMPUTE:
        startReg = 0;
        break;
    default:
        gcmASSERT(gcvFALSE);
        startReg = 0;
        break;
    }

    return startReg;
}

void _VIR_RA_LS_InitializeOpnd(
    VIR_RA_LS       *pRA)
{

    VIR_Shader*       pShader = VIR_RA_LS_GetShader(pRA);
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
            inst != gcvNULL;
            inst = VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SrcOperand_Iterator  opndIter;
            VIR_Operand             *opnd;

            /* initialize operand's hwRegId for register allocation */
            if (VIR_Inst_GetDest(inst))
            {
                VIR_Operand_SetHwRegId(VIR_Inst_GetDest(inst), VIR_INVALID_HWREG);
            }

            VIR_SrcOperand_Iterator_Init(inst, &opndIter);
            opnd = VIR_SrcOperand_Iterator_First(&opndIter);
            for (; opnd != gcvNULL; opnd = VIR_SrcOperand_Iterator_Next(&opndIter))
            {
                VIR_Operand_SetHwRegId(opnd, VIR_INVALID_HWREG);
            }
        }
    }
}

VSC_ErrCode _VIR_RA_LS_AssignAttributes(
    VIR_RA_LS       *pRA)
{
    VSC_ErrCode retValue  = VSC_ERR_NONE;

    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    VIR_RA_ColorPool        *pCP = VIR_RA_LS_GetColorPool(pRA);
    VIR_Dumper              *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);

    gctUINT     attrCount = VIR_IdList_Count(pAttrs);
    gctUINT     reg, currAttr;
    gctUINT     primIdWebIdx = NOT_ASSIGNED, ptCoordWebIdx = NOT_ASSIGNED;
    gctUINT     instanceIdWebIdx = NOT_ASSIGNED, vertexIdWebIdx = NOT_ASSIGNED;
    VIR_Symbol  *primIdAttr = gcvNULL, *ptCoordAttr = gcvNULL;
    VIR_Symbol  *instanceIdAttr = gcvNULL, *vertexIdAttr = gcvNULL;

    VIR_RA_HWReg_Color  LRColor;

    LRColor = InvalidColor;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "\nAssign color for attributes\n");
        VIR_LOG_FLUSH(pDumper);
    }

    /* reserve registers per HW spec */
    reg = _VIR_RA_LS_GetStartReg(pRA);
    pCP->colorMap[VIR_RA_HWREG_GR].availReg = reg;

    /* only allocate for used attributes */
    for (currAttr = 0; currAttr < attrCount; currAttr++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader,
                                    VIR_IdList_GetId(pAttrs, currAttr));
        VIR_Type    *attrType = VIR_Symbol_GetType(attribute);
        gctUINT     attrRegCount = VIR_Type_GetVirRegCount(pShader, attrType, -1);
        gctUINT     components = VIR_Symbol_GetComponents(attribute);

        gctUINT         location;
        VIR_IdList      *pLocationList = gcvNULL;
        gctUINT         j;

        VIR_DEF_KEY     defKey;
        gctUINT         defIdx, webIdx;
        gctUINT         attFlags;
        gctUINT         curReg = reg;
        gctUINT8        shift = 0;
        gctBOOL         skipAlloc = gcvFALSE;

        VIR_RA_LS_Liverange *pLR = gcvNULL;

        /* we don't need to assign register for per-vertex/per-patch input,
           we will use load_attr to access them */
        if (_VIR_RA_LS_IsOpndSymExcludedLR(pRA, gcvNULL, attribute))
        {
            continue;
        }

        if (isSymUnused(attribute) || isSymVectorizedOut(attribute))
        {
            continue;
        }

        if (VIR_Shader_isDual16Mode(pShader) &&
            VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
        {
            if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.highpVaryingShift)
            {
                if (components > 2)
                {
                    attrRegCount = attrRegCount * 2;
                }
            }
            else
            {
                attrRegCount = attrRegCount * 2;
            }
        }

        /* Fill psInpuPosCompValid and psInpuPCCompValid. */
        if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
        {
            _VIR_RA_FillPsInputPosPCCompValid(pRA, attribute);
        }

        defKey.pDefInst = VIR_INPUT_DEF_INST;
        defKey.regNo = VIR_Symbol_GetVariableVregIndex(attribute);
        defKey.channel = VIR_CHANNEL_ANY;
        defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);

        if (VIR_INVALID_DEF_INDEX != defIdx)
        {
            webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
            pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
            gcmASSERT(pLR);
            pLR->regNoRange = VIR_Type_GetVirRegCount(pShader, attrType, -1);

            /* It has been assigned by other aliased attribute. */
            if (!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)) || isLRSpilled(pLR))
            {
                continue;
            }

            /* Get the location and the location list. */
            location = VIR_Symbol_GetLocation(attribute);
            if (location != -1 && VIR_Shader_HasAliasedAttribute(pShader))
            {
                pLocationList = &VIR_Shader_GetAttributeAliasList(pShader)[location];
            }
            else
            {
                pLocationList = gcvNULL;
            }

            /* Check if any aliased attribute has been processed, if found, just use its info. */
            if (pLocationList && VIR_IdList_Count(pLocationList) > 1)
            {
                VIR_RA_HWReg_Color  aliasedColor = InvalidColor;
                gctUINT             spillOffset = NOT_ASSIGNED;
                VIR_Symbol*         pCopySym = gcvNULL;
                VIR_RA_LS_Liverange*pCopyLR = gcvNULL;

                for (j = 0; j < VIR_IdList_Count(pLocationList); j++)
                {
                    VIR_Symbol*             pAliasedSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pLocationList, j));
                    gctUINT                 aliasedWebIdx;
                    VIR_RA_LS_Liverange*    pAliasedLR = gcvNULL;

                    defKey.pDefInst = VIR_INPUT_DEF_INST;
                    defKey.regNo = VIR_Symbol_GetVariableVregIndex(pAliasedSym);
                    defKey.channel = VIR_CHANNEL_ANY;
                    defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);

                    if (VIR_INVALID_DEF_INDEX != defIdx)
                    {
                        aliasedWebIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
                        pAliasedLR = _VIR_RA_LS_Web2LR(pRA, aliasedWebIdx);
                        gcmASSERT(pAliasedLR);

                        if (_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pAliasedLR)) && !isLRSpilled(pAliasedLR))
                        {
                            continue;
                        }

                        /* Find a valid LR, use it to set all the other aliased LR. */
                        if (isLRSpilled(pAliasedLR))
                        {
                            spillOffset = _VIR_RA_GetLRSpillOffset(pAliasedLR);
                        }
                        else
                        {
                            aliasedColor = _VIR_RA_GetLRColor(pAliasedLR);
                        }

                        pCopySym = pAliasedSym;
                        pCopyLR = pAliasedLR;

                        for (j = 0; j < VIR_IdList_Count(pLocationList); j++)
                        {
                            VIR_Symbol*             pAliasedSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pLocationList, j));
                            gctUINT                 aliasedWebIdx;
                            VIR_RA_LS_Liverange*    pAliasedLR = gcvNULL;

                            if (pAliasedSym == pCopySym)
                            {
                                continue;
                            }

                            defKey.pDefInst = VIR_INPUT_DEF_INST;
                            defKey.regNo = VIR_Symbol_GetVariableVregIndex(pAliasedSym);
                            defKey.channel = VIR_CHANNEL_ANY;
                            defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);

                            if (VIR_INVALID_DEF_INDEX != defIdx)
                            {
                                aliasedWebIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
                                pAliasedLR = _VIR_RA_LS_Web2LR(pRA, aliasedWebIdx);
                                gcmASSERT(pAliasedLR);

                                pAliasedLR->colorFunc = pCopyLR->colorFunc;
                                /* spill case */
                                if (spillOffset != NOT_ASSIGNED)
                                {
                                    VIR_RA_LR_SetFlag(pAliasedLR, VIR_RA_LRFLAG_SPILLED);
                                    _VIR_RA_SetLRSpillOffset(pAliasedLR, spillOffset);
                                }
                                else
                                {
                                    pAliasedLR->u1.color = aliasedColor;
                                }

                                VIR_Symbol_SetHwRegId(pAliasedSym, VIR_Symbol_GetHwRegId(pCopySym));
                                VIR_Symbol_SetHwShift(pAliasedSym, VIR_Symbol_GetHwShift(pCopySym));
                                VIR_Symbol_SetHIHwRegId(pAliasedSym, VIR_Symbol_GetHIHwRegId(pCopySym));
                                VIR_Symbol_SetHIHwShift(pAliasedSym, VIR_Symbol_GetHIHwShift(pCopySym));
                            }
                        }
                        break;
                    }
                }
            }

            /* It has been assigned by other aliased attribute. */
            if (!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)) || isLRSpilled(pLR))
            {
                continue;
            }

            /* the attributes that is put into special register */
            if ((VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_ID ||
                 VIR_Symbol_GetName(attribute) == VIR_NAME_INSTANCE_ID) &&
                VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.vtxInstanceIdAsAttr == gcvFALSE)
            {
                /* gcmASSERT(VIR_Symbol_GetPrecision(attribute) != VIR_PRECISION_HIGH); */

                /* if vtxInstanceIdAsAttr is not enabled, assign a special regNo
                    to the web, otherwise, assign as the last attribute  */
                pLR->deadIntervalAvail = gcvFALSE;
                if (VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_ID)
                {
                    _VIR_RA_MakeColor(VIR_SR_VERTEXID, 0, &LRColor);
                }
                else
                {
                    _VIR_RA_MakeColor(VIR_SR_INSTATNCEID, 0, &LRColor);
                }
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else if (VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_ID)
            {
                _VIR_RA_MakeColor(VIR_REG_SAMPLE_ID, 0, &LRColor);
                _VIR_RA_MakeColorHI(VIR_REG_SAMPLE_ID, 0, &LRColor);
                pLR->deadIntervalAvail = gcvFALSE;
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else if (VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_MASK_IN)
            {
                _VIR_RA_MakeColor(VIR_REG_SAMPLE_MASK_IN, 0, &LRColor);
                _VIR_RA_MakeColorHI(VIR_REG_SAMPLE_MASK_IN, 0, &LRColor);
                pLR->deadIntervalAvail = gcvFALSE;
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else if (VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_MASK)
            {
                _VIR_RA_MakeColor(pShader->sampleMaskIdRegStart,
                                    pShader->sampleMaskIdChannelStart,
                                    &LRColor);
                _VIR_RA_MakeColorHI(pShader->sampleMaskIdRegStart+1,
                                    pShader->sampleMaskIdChannelStart,
                                    &LRColor);
                pLR->deadIntervalAvail = gcvFALSE;
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else if (VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_POSITION)
            {
                _VIR_RA_MakeColor(VIR_REG_SAMPLE_POS, 0, &LRColor);
                _VIR_RA_MakeColorHI(VIR_REG_SAMPLE_POS, 0, &LRColor);
                pLR->deadIntervalAvail = gcvFALSE;
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else if (VIR_Symbol_GetName(attribute) == VIR_NAME_SUBSAMPLE_DEPTH)
            {
                _VIR_RA_MakeColor(VIR_REG_MULTISAMPLEDEPTH, 0, &LRColor);
                _VIR_RA_MakeColorHI(VIR_REG_MULTISAMPLEDEPTH, 0, &LRColor);
                _VIR_RA_LS_AssignColorWeb(
                    pRA,
                    webIdx,
                    VIR_RA_HWREG_GR,
                    LRColor,
                    VIR_RA_LS_ATTRIBUTE_FUNC);
            }
            else
            {
                /* assign a color: usually, the LR's channelMask is set at the def.
                Since there is no def for attributes, we use Web's channelMask
                to set LR's channelMask. */
                if (VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_ID ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_INDEX ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_INSTANCE_ID)
                {
                    /* For the HW that does not support zero attributes for FE2VS, if there is only
                        vertexid or instanceid for vs's inputs, set the hw reg number of these 2 to 1
                        because HW will insert an element for FE2VS when programming stream */
                    if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.supportZeroAttrsInFE && curReg == 0)
                    {
                        curReg = 1;
                        reg = curReg;
                    }

                    skipAlloc = gcvTRUE;

                    if (VIR_Symbol_GetName(attribute) == VIR_NAME_INSTANCE_ID)
                    {
                        instanceIdWebIdx = webIdx;
                        instanceIdAttr = attribute;
                    }
                    else
                    {
                        vertexIdWebIdx = webIdx;
                        vertexIdAttr = attribute;
                    }
                }

                if (!_VIR_RA_LS_handleBuiltinAttr(pLR, pShader, attribute, &curReg, &shift))
                {
                    /* record the primitiveId location in TES, since we need to load it from
                        patch data*/
                    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION &&
                        VIR_Symbol_GetName(attribute) == VIR_NAME_PRIMITIVE_ID)
                    {
                        /* since primitive id is per-patch data, channel z,
                            we are using .z to save one move */
                        shift = 2;
                        _VIR_RA_LS_GenLoadAttr_Patch_r0(pRA, attribute, curReg);
                    }

                    /* the primitive id in PS is assigned to the last register */
                    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
                    {
                        if (VIR_Shader_PS_NeedSpecAllocPrimId(pShader))
                        {
                            skipAlloc = gcvTRUE;
                            primIdWebIdx = webIdx;
                            primIdAttr = attribute;
                        }

                        if (VIR_Shader_PS_NeedSpecAllocPtCoord(pShader))
                        {
                            skipAlloc = gcvTRUE;
                            ptCoordWebIdx = webIdx;
                            ptCoordAttr = attribute;
                        }
                    }
                }

                if (!skipAlloc)
                {
                    if (shift != 0)
                    {
                        gcmASSERT(!VIR_Symbol_GetCannotShift(attribute));
                    }

                    _VIR_RA_MakeColor(curReg, shift, &LRColor);
                    if (VIR_Shader_isDual16Mode(pShader) &&
                        VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
                    {
                        switch (components)
                        {
                        case 1:
                        case 2:
                            /* in arch_version_5_4_4 and beyond, +1 temp for each highp varying with 3 or 4 components */
                            if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.highpVaryingShift)
                            {
                                shift = 2;
                                _VIR_RA_MakeColorHI(curReg, shift, &LRColor);
                            }
                            else
                            {
                                shift = 0;
                                _VIR_RA_MakeColorHI(curReg + 1, shift, &LRColor);
                            }
                            break;
                        case 3:
                        case 4:
                            _VIR_RA_MakeColorHI(curReg + 1, shift, &LRColor);
                            break;
                        default:
                            gcmASSERT(gcvFALSE);
                            break;
                        }
                    }
                    else
                    {
                        _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &LRColor);
                    }

                    _VIR_RA_LS_AssignColorWeb(
                        pRA,
                        webIdx,
                        VIR_RA_HWREG_GR,
                        LRColor,
                        VIR_RA_LS_ATTRIBUTE_FUNC);
                }
            }
            if (!skipAlloc)
            {
                VIR_Symbol_SetHwRegId(attribute, LRColor._hwRegId);
                VIR_Symbol_SetHwShift(attribute, LRColor._hwShift);
                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
                {
                    VIR_Symbol_SetHIHwRegId(attribute, LRColor._HIhwRegId);
                    VIR_Symbol_SetHIHwShift(attribute, LRColor._HIhwShift);
                }

                /* Move to next register */
                if (!_VIR_RA_LS_handleBuiltinAttr(pLR, pShader, attribute, gcvNULL, gcvNULL))
                {
                    reg += attrRegCount;
                }
            }
        }
        else
        {
            if (!isSymAlwaysUsed(attribute))
            {
                attFlags = (gctUINT) VIR_Symbol_GetFlags(attribute);
                attFlags = (attFlags & ~VIR_SYMFLAG_UNUSED) | VIR_SYMFLAG_UNUSED;
                VIR_Symbol_SetFlag(attribute, (VIR_SymFlag) attFlags);
            }
            else
            {
                if (((VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_ID ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_INSTANCE_ID) &&
                    VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.vtxInstanceIdAsAttr == gcvFALSE) ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_ID ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_MASK_IN ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_POSITION ||
                    VIR_Symbol_GetName(attribute) == VIR_NAME_SAMPLE_POSITION
                    )
                {
                    /* Skip */
                }
                else
                {
                    if (!_VIR_RA_LS_handleBuiltinAttr(pLR, pShader, attribute, &curReg, &shift))
                    {
                        /* the primitive id in PS is assigned to the last register */
                        if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
                        {
                            if (VIR_Shader_PS_NeedSpecAllocPrimId(pShader))
                            {
                                skipAlloc = gcvTRUE;
                            }

                            if (VIR_Shader_PS_NeedSpecAllocPtCoord(pShader))
                            {
                                skipAlloc = gcvTRUE;
                            }
                        }
                    }
                }

                if (!skipAlloc)
                {
                    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
                    VIR_Symbol_SetHwRegId(attribute, curReg);
                    /* it is possible that some input are not in du (i.e., not used), thus it will not
                       be added to active list and change the maxAllocReg. Thus we have to update
                       the maxAllocReg here. */
                    if (curReg > pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg)
                    {
                        pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg = curReg;
                    }
                    if (VIR_Shader_isDual16Mode(pShader) &&
                        VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
                    {
                        VIR_Symbol_SetHIHwRegId(attribute, curReg + 1);
                        if (curReg + 1 > pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg)
                        {
                            pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg = curReg + 1;
                        }
                    }

                    /* Move to next register */
                    if (!_VIR_RA_LS_handleBuiltinAttr(pLR, pShader, attribute, gcvNULL, gcvNULL))
                    {
                        reg += attrRegCount;
                    }
                }
            }
        }
    }

    /* assign the (last - 1) hw register to pt-coord */
    if (ptCoordWebIdx != NOT_ASSIGNED)
    {
        VIR_RA_LS_Liverange *pLR;
        pLR = _VIR_RA_LS_Web2LR(pRA, ptCoordWebIdx);
        gcmASSERT(pLR);
        pLR->regNoRange = 1;
        _VIR_RA_MakeColor(reg, 0, &LRColor);
        _VIR_RA_LS_AssignColorWeb(
            pRA,
            ptCoordWebIdx,
            VIR_RA_HWREG_GR,
            LRColor,
            VIR_RA_LS_ATTRIBUTE_FUNC);
        gcmASSERT(ptCoordAttr != gcvNULL);
        gcmASSERT(VIR_Symbol_GetPrecision(ptCoordAttr) != VIR_PRECISION_HIGH);
        VIR_Symbol_SetHwRegId(ptCoordAttr, reg);
        VIR_Symbol_SetHwShift(ptCoordAttr, 0);
        reg ++;
    }

    /* assign VIR_NAME_VERTEX_ID/VIR_NAME_INSTANCE_ID are last one,
       since they could be not-last-attr */
    if (instanceIdWebIdx != NOT_ASSIGNED ||
        vertexIdWebIdx  != NOT_ASSIGNED)
    {
        VIR_RA_LS_Liverange *pLR;

        gcmASSERT(primIdWebIdx == NOT_ASSIGNED);

        if (vertexIdWebIdx != NOT_ASSIGNED)
        {
            pLR = _VIR_RA_LS_Web2LR(pRA, vertexIdWebIdx);
            gcmASSERT(pLR);
            pLR->regNoRange = 1;
            _VIR_RA_MakeColor(reg, 0, &LRColor);
            _VIR_RA_LS_AssignColorWeb(
                pRA,
                vertexIdWebIdx,
                VIR_RA_HWREG_GR,
                LRColor,
                VIR_RA_LS_ATTRIBUTE_FUNC);
            gcmASSERT(vertexIdAttr != gcvNULL);
            VIR_Symbol_SetHwRegId(vertexIdAttr, reg);
            VIR_Symbol_SetHwShift(vertexIdAttr, 0);
        }
        if (instanceIdWebIdx != NOT_ASSIGNED)
        {
            pLR = _VIR_RA_LS_Web2LR(pRA, instanceIdWebIdx);
            gcmASSERT(pLR);
            pLR->regNoRange = 1;
            _VIR_RA_MakeColor(reg, 1, &LRColor);
            _VIR_RA_LS_AssignColorWeb(
                pRA,
                instanceIdWebIdx,
                VIR_RA_HWREG_GR,
                LRColor,
                VIR_RA_LS_ATTRIBUTE_FUNC);
            gcmASSERT(instanceIdAttr != gcvNULL);
            VIR_Symbol_SetHwRegId(instanceIdAttr, reg);
            VIR_Symbol_SetHwShift(instanceIdAttr, 1);
        }
        reg ++;
    }

    /* assign the last hw register to primitive id */
    if (primIdWebIdx != NOT_ASSIGNED)
    {
        VIR_RA_LS_Liverange *pLR;
        pLR = _VIR_RA_LS_Web2LR(pRA, primIdWebIdx);
        gcmASSERT(pLR);
        pLR->regNoRange = 1;
        _VIR_RA_MakeColor(reg, 0, &LRColor);
        _VIR_RA_LS_AssignColorWeb(
            pRA,
            primIdWebIdx,
            VIR_RA_HWREG_GR,
            LRColor,
            VIR_RA_LS_ATTRIBUTE_FUNC);
        gcmASSERT(primIdAttr != gcvNULL);
        gcmASSERT(VIR_Symbol_GetPrecision(primIdAttr) != VIR_PRECISION_HIGH);
        VIR_Symbol_SetHwRegId(primIdAttr, reg);
        reg ++;
    }

    /* reserved one register (to save select_map dest or store_attr src2...),
       need to refine later. */
    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
        pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION ||
        pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        pRA->resRegister = reg;
        _VIR_RA_LS_SetUsedColor(pRA, VIR_RA_HWREG_GR, reg, 0xF);
        _VIR_RA_MakeColor(reg, 0, &LRColor);
        _VIR_RA_LS_SetMaxAllocReg(pRA, LRColor, VIR_RA_HWREG_GR, 1);
        pCP->colorMap[VIR_RA_HWREG_GR].availReg ++;
    }
    else if (pShader->shaderKind == VIR_SHADER_FRAGMENT &&
             VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5)
    {
        /* skip .z in make color if sampleMaskId use .w (if sampleMaskID use .w, means shader has .depth which uses r0.z)
         * if sampleMaskId use .z, keep .w unused for other special attributes usage, like gl_layer
         */
        gctUINT channels = ((pShader->sampleMaskIdRegStart == 0) && (pShader->sampleMaskIdChannelStart == CHANNEL_W))? 0xB : 0x7;
        _VIR_RA_LS_SetUsedColor(pRA, VIR_RA_HWREG_GR, 0,
            (pShader->sampleMaskIdRegStart == 0 && VIR_Shader_PS_NeedSampleMaskId(pShader)) ? channels : 0x3);
        _VIR_RA_MakeColor(0, 0, &LRColor);
        _VIR_RA_LS_SetMaxAllocReg(pRA, LRColor, VIR_RA_HWREG_GR, 1);
    }

    return retValue;
}

gctBOOL _VIR_RS_LS_AllDefInWebNotOut(
    VIR_RA_LS           *pRA,
    VSC_BIT_VECTOR      *pOutFlow,
    gctUINT             defIdx
    )
{
    VIR_LIVENESS_INFO *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    VIR_WEB *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    gctUINT thisIdx;
    VIR_DEF *pDef;

    /* find all defs */
    thisIdx = pWeb->firstDefIdx;
    while (VIR_INVALID_DEF_INDEX != thisIdx)
    {
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, thisIdx);
        if(vscBV_TestBit(pOutFlow, thisIdx))
        {
            return gcvFALSE;
        }
        thisIdx = pDef->nextDefInWebIdx;
    }

    return gcvTRUE;
}

/* ===========================================================================
   _VIR_RA_LS_BuildLRTableBB
   build live range table for basic block. The logic of this func must be
   same as local genkill resolver of LV
   ===========================================================================
*/
VSC_ErrCode _VIR_RA_LS_BuildLRTableBB(
    VIR_RA_LS           *pRA,
    VIR_BASIC_BLOCK     *pBB,
    VIR_TS_FUNC_FLOW    *pFuncFlow
    )
{
    VSC_ErrCode             retValue  = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VSC_MM                  *pMM = VIR_RA_LS_GetMM(pRA);

    VIR_DEF                 *pDef;
    VIR_Instruction         *pStartInst = BB_GET_START_INST(pBB);
    VIR_Instruction         *pInst = BB_GET_END_INST(pBB);
    VIR_TS_BLOCK_FLOW       *pBlkFlow;
    VIR_Enable              defEnableMask;
    gctUINT                 thisDefId, firstRegNo, regNoRange, i;
    VSC_BIT_VECTOR          *tempVec;
    gctBOOL                 bDstIndexing;
    gctINT                  curPos = 0;

    if(BB_GET_LENGTH(pBB) == 0)
    {
        return retValue;
    }

    /* clear the channel mask at each bb, to avoid such situation
        B1:
           t1.x = ...
           t1.y = ...   should clean channel mask before processing B1,
                        otherwise t1 will be dead here, which is wrong
           goto B3:
        B2:
           t1.x = ...   (channel mask x)
        B3:
           t2.xy = t1.xy  (live here)
    */

    _VIR_RA_LS_ClearChannelMask(pRA);

    pBlkFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(
        &pFuncFlow->tsBlkFlowArray,
        pBB->dgNode.id);
    tempVec = vscBV_Create(pMM, VIR_RA_LS_GetNumDef(pRA));
    vscBV_Xor(tempVec, &pBlkFlow->outFlow, VIR_RA_LS_GetLiveLRVec(pRA));
    i = 0;
    while ((thisDefId = vscBV_FindSetBitForward(tempVec, i)) != (gctUINT)INVALID_BIT_LOC)
    {
        i = thisDefId + 1;

        /* need to skip the per-vertex input/output */
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, thisDefId);

        if (_VIR_RA_LS_IsDefExcludedLR(pDef))
        {
            continue;
        }

        if (vscBV_TestBit(&pBlkFlow->outFlow, thisDefId))
        {
            _VIR_RS_LS_MarkLRLive(pRA, thisDefId, (0x1 << pDef->defKey.channel), gcvTRUE, -1);
            _VIR_RS_LS_SetLiveLRVec(pRA, thisDefId);
        }
        else
        {
            /* need to change from channel to enable */
            if (_VIR_RS_LS_MaskMatch(pRA, (0x1 << pDef->defKey.channel), thisDefId) &&
                _VIR_RS_LS_AllDefInWebNotOut(pRA, &pBlkFlow->outFlow, thisDefId))
            {
                _VIR_RS_LS_MarkLRDead(pRA, thisDefId, (0x1 << pDef->defKey.channel), gcvTRUE);
            }
            _VIR_RS_LS_UnsetLiveLRVec(pRA, thisDefId);
        }
    }

    vscBV_Destroy(tempVec);

    _VIR_RA_LS_ClearChannelMask(pRA);

    /* backward traversing the instructions inside BB
       mark the def dead and uses live */
    while (pInst)
    {
        /* mark dead at real def */
        if (vscVIR_QueryRealWriteVirRegInfo(pShader,
                                            pInst,
                                            &defEnableMask,
                                            gcvNULL,
                                            &firstRegNo,
                                            &regNoRange,
                                            gcvNULL,
                                            &bDstIndexing))
        {
            /* mark dead for the real def */
            _VIR_RA_LS_MarkDef(pRA, pInst, firstRegNo, regNoRange, defEnableMask, bDstIndexing, pBlkFlow);
        }

        /* mark live for all uses */
        _VIR_RA_LS_MarkUses(pRA, pInst);

        if (VIR_OPCODE_hasStoreOperation(VIR_Inst_GetOpcode(pInst)))
        {
             VIR_RA_LS_Liverange *pLR = gcvNULL;

            if (!VIR_Inst_Store_Have_Dst(pInst))
            {
                /* store data is the dest */
                VIR_Operand      *pDataOpnd = VIR_OPCODE_useSrc3AsInstType(VIR_Inst_GetOpcode(pInst)) ?
                    VIR_Inst_GetSource(pInst, 3) : VIR_Inst_GetSource(pInst, 2);

                gctUINT webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pDataOpnd);
                if (VIR_INVALID_WEB_INDEX != webIdx)
                {
                    pLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                }

                gcmASSERT(pLR != gcvNULL);

                VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_ST_DEP);
                pLR->pLDSTInst  = pInst;
            }
        }

        /* Emit has implicit usage for all outputs, so we also gen these */
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT0   ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT    ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM)
        {
            gctBOOL     bCheckAllOutput = gcvTRUE;
            gctINT      streamNumber = 0;

            if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM)
            {
                bCheckAllOutput = gcvFALSE;
                gcmASSERT(VIR_Operand_isImm(VIR_Inst_GetSource(pInst, 0)));
                streamNumber = VIR_Operand_GetImmediateInt(VIR_Inst_GetSource(pInst, 0));
            }

            _VIR_RA_LS_Mark_Outputs(pRA, pInst, bCheckAllOutput, streamNumber);
        }

        curPos = VIR_RA_LS_GetCurrPos(pRA);
        VIR_RA_LS_SetCurrPos(pRA, curPos-1);

        /* set instruction id, for dump liveness with inst id */
        if (VIR_Inst_GetId(pInst) != curPos)
        {
            VIR_RA_LS_SetInstIdChanged(pRA, gcvTRUE);
        }
        VIR_Inst_SetId(pInst, curPos);

        /* If current inst is the start inst of block, just bail out */
        if (pInst == pStartInst)
        {
            break;
        }
        pInst = VIR_Inst_GetPrev(pInst);
    }

    return retValue;
}

/* ===========================================================================
   _VIR_RA_LS_BuildLRTable
   build live range table for function
   ===========================================================================
*/
VSC_ErrCode _VIR_RA_LS_BuildLRTable(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_CFG             *pCfg = VIR_Function_GetCFG(pFunc);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    gctUINT             countOfBasicBlk = vscDG_GetNodeCount(&pCfg->dgGraph);
    VIR_BASIC_BLOCK     **ppBasicBlkRPO;
    gctUINT             bbIdx;
    gctUINT             webIdx;

    VIR_TS_FUNC_FLOW    *pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(
        &VIR_RA_LS_GetLvInfo(pRA)->baseTsDFA.tsFuncFlowArray,
        pFunc->pFuncBlock->dgNode.id);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_BUILD_LR))
    {
        VIR_LOG(pDumper, "\nBuild liverange table:\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    if (countOfBasicBlk != 0)
    {
        /* initialize the live range table */
        _VIR_RA_LS_InitLRTable(pRA);
        /* initiallize the LR vector */
        vscBV_ClearAll(VIR_RA_LS_GetLiveLRVec(pRA));

        /* initialize the current code position to be the total
           number of instructions in the pFunc */
        gcmASSERT(VIR_Function_GetInstCount(pFunc) < VIR_RA_LS_POS_MAX);
        VIR_RA_LS_SetCurrPos(pRA, VIR_Function_GetInstCount(pFunc));
        pRA->trueDepPoint = VIR_Function_GetInstCount(pFunc);

        /* RPO travering of CFG */
        ppBasicBlkRPO = (VIR_BASIC_BLOCK**)vscMM_Alloc(
            VIR_RA_LS_GetMM(pRA),
            sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);

        vscDG_PstOrderTraversal(&pCfg->dgGraph,
                                VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                                gcvTRUE,
                                gcvTRUE,
                                (VSC_DG_NODE**)ppBasicBlkRPO);


        for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
        {
            VIR_BASIC_BLOCK *pBB = ppBasicBlkRPO[bbIdx];

            retValue = _VIR_RA_LS_BuildLRTableBB(pRA, pBB, pFuncFlow);
            CHECK_ERROR(retValue, "_VIR_RA_LS_BuildLRTableBB");
            if (retValue)
            {
                break;
            }
        }

        _VIR_RA_LS_SetMasterLR(pRA);
    }

    /* adjust VX LR's start point */
    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        if (isLRVX(pLR))
        {
            if (pLR->startPoint == 0)
            {
                pLR->startPoint = pLR->currDef;
            }
        }
        /*reset highpvec2 flag if pShader is no longer dual16*/
        if (!VIR_Shader_isDual16Mode(pShader))
        {
           VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRHIGHPVEC2);
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_BUILD_LR))
    {
        VIR_LOG(pDumper, "\n============== liverange table [%s] ==============\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_RS_LS_DumpLRTable(pRA, pFunc, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return retValue;
}

/* ===========================================================================
   _VIR_RA_LS_SortLRTable
   sort the live ranges (in LRTable) appearing inside pFunc in the increasing
   order of startPoint
   ===========================================================================
*/
VSC_ErrCode _VIR_RA_LS_SortLRTable(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_RA_LS_Liverange  *pHead, *pEnd, *pLR;
    gctUINT              webIdx;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_SORT_LR))
    {
        VIR_LOG(pDumper, "\nSort liveranges:\t\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    pHead = VIR_RA_LS_GetSortedLRHead(pRA);
    pEnd = pHead->nextLR;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        if (isLRA0OrB0(pLR))
        {
            continue;
        }

        if (pLR->liveFunc == pFunc)
        {
            gctUINT                 curStart, nextStart;
            VIR_RA_LS_Liverange     *pHeadNext, *pEndNext;

            curStart = pLR->startPoint;
            pHeadNext = VIR_RA_LS_GetSortedLRHead(pRA)->nextLR;

            if ((pHead->startPoint >= curStart) || (pEnd->startPoint < curStart))
            {
                if (pHead->startPoint >= curStart)
                {
                    pHead = VIR_RA_LS_GetSortedLRHead(pRA);
                    pEnd = pHeadNext;
                }

                if (pEnd->startPoint < curStart)
                {
                    pEndNext = pEnd->nextLR;

                    do
                    {
                        pHead = pEnd;
                        pEnd = pEndNext;
                        nextStart = pEndNext->startPoint;
                        pEndNext = pEndNext->nextLR;
                    } while (nextStart < curStart);
                }
            }
            pLR->nextLR = pEnd;
            pEnd = pLR;
            pHead->nextLR = pLR;
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_SORT_LR))
    {
        VIR_LOG(pDumper, "\n============== sorted liverange list [%s] ==============\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_RS_LS_DumpSortedLRTable(pRA, pFunc, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return retValue;
}

/* the heuristic for choosing spill: choose smallest weight to spill
   weight = lengh of live range / number of def and use
*/
void
_VIR_RA_LS_computeWeight(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper              *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);

    gctUINT                 webIdx, defIdx, usageCount = 0;
    VIR_RA_LS_Liverange     *pLR;
    VIR_WEB                 *pWeb;
    VIR_DEF                 *pDef;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
        pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
        if (pLR->liveFunc == pFunc)
        {
            VIR_RA_LS_Interval *deadInterval = gcvNULL;
            gctUINT deadInterval_len = 0;
            gctUINT liveInterval_len = pLR->endPoint - pLR->startPoint;
            /* find all defs */
            defIdx = pWeb->firstDefIdx;
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                usageCount += DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain);
                defIdx = pDef->nextDefInWebIdx;
            }

            /* reverse weight computation, the weight value is large if live range is short and more reference
             * if live range is long and less used, it will be selected spilled first
             * consider deadInterval range especially for multi-inlined function, which temp variables
             * will be used in a range and in multi places
             */
            for (deadInterval = pLR->deadIntervals; deadInterval != gcvNULL;
                 deadInterval = deadInterval->next)
            {
                deadInterval_len += deadInterval->endPoint - deadInterval->startPoint;
            }
            liveInterval_len = (liveInterval_len - deadInterval_len > 0) ? liveInterval_len - deadInterval_len : 1;
            pLR->weight = (gctFLOAT)(pWeb->numOfDef + usageCount) / (gctFLOAT)(liveInterval_len);
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "\n============== weighted liverange list [%s] ==============\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_RS_LS_DumpSortedLRTable(pRA, pFunc, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }
}

/* ===========================================================================
   _VIR_RA_LS_AssignColorsForGeneralReg
   assign color to the webs in the function
   ===========================================================================
*/

/* find a brand new color. this function is called when a conflict is met.
   For example:
   main:
   temp(1): LR1  --> r0
   temp(2): LR2  --> r1
   foo:
   temp(3): LR3  --> r0
   temp(1): LR1, assigned in main to r0, but now r0 is assigned to LR3, thus
   change LR1's color to a branch new color (not just available color).
   we don't use available color since that color maybe used in main to
   the LR that is interfered with LR1.
   we may optimize here.
   */
gctBOOL _VIR_RA_LS_FindBrandnewColor(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR,
    VIR_RA_HWReg_Color  *color,
    gctUINT             reservedDataReg)
{
    gctBOOL             retValue = gcvTRUE;
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    gctUINT             regNoRange = 1;

    if (pLR)
    {
        /* only for VIR_RA_HWREG_GR*/
        gcmASSERT(pLR->hwType == VIR_RA_HWREG_GR);
        regNoRange = pLR->regNoRange;
    }

    if (pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg + regNoRange < _VIR_RA_LS_GetMaxReg(pRA, VIR_RA_HWREG_GR, reservedDataReg))
    {
        gctUINT regNo = pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg + 1;
        _VIR_RA_MakeColor(regNo, 0, color);
        _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, color);

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "find brand new [r%d]", regNo);
            VIR_LOG_FLUSH(pDumper);
        }
    }
    else
    {
        retValue = gcvFALSE;
    }

    return retValue;
}

void  _VIR_RA_LS_Reserve_AttrColor(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    gctUINT i = 0, webIdx = VIR_INVALID_WEB_INDEX, highDiff = 0;
    VIR_RA_LS_Liverange *pLR;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

        if (pLR->liveFunc == pFunc &&
            pLR->colorFunc == VIR_RA_LS_ATTRIBUTE_FUNC)
        {
            /* attribute is not spilled */
            gcmASSERT(!isLRSpilled(pLR));
            gcmASSERT(!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)));

            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)))
            {
                highDiff = _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) - _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR));
            }

            for (i = 0; i < pLR->regNoRange; i++)
            {
                _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType,
                    _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + i * (highDiff + 1),
                    _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR))));

                /* mark the high register */
                if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)))
                {
                    _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType,
                        _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + i * (highDiff + 1),
                        _VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR))));
                }
            }
            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
            {
                VIR_LOG(pDumper, "mark ");
                _VIR_RA_LS_DumpColor(pRA, _VIR_RA_GetLRColor(pLR), pLR);
                VIR_LOG(pDumper, " to be used because of LR%d\n",
                    pLR->webIdx);
                VIR_LOG_FLUSH(pDumper);
            }
        }
    }
}

/* check whether there is any def/use in the web (whose reg is the same as the one that
   will be used for ld/st), that is in the bubble window with ld/st.
   1) add r1.x, ...
   2) ld  r1.yzw, ... ==> we should not assign r1.yzw to ld dest, since there is a use of r1.x in 5)
   ..
   5) add r2, r1.x
*/
gctBOOL _VIR_RA_LS_UseAfterInst(
    VIR_RA_LS           *pRA,
    VIR_RA_LS_Liverange *pLR,
    gctUINT             regNo)
{
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_RA_LS_Liverange *pPrev, *pCurr;
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctINT             ldstPos, ldstPosEnd;

    gcmASSERT(pLR->pLDSTInst != gcvNULL);
    ldstPos = VIR_Inst_GetId(pLR->pLDSTInst);
    ldstPosEnd = ldstPos  + VSC_OPTN_RAOptions_GetSTBubbleSize(pOption);

    pPrev = VIR_RA_LS_GetActiveLRHead(pRA);
    pCurr = pPrev->nextActiveLR;

    while (pCurr != &LREndMark)
    {
        if (pCurr->u1.color._hwRegId == regNo)
        {
            gctUINT     webIdx = pCurr->webIdx;
            VIR_WEB     *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
            gctUINT     defIdx = pWeb->firstDefIdx;
            VIR_DEF     *pDef;

            VSC_DU_ITERATOR     duIter;
            VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
            VIR_USAGE               *pUsage;

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

                if (pDef->defKey.pDefInst == VIR_UNDEF_INST ||
                    pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST ||
                    pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
                {
                    defIdx = pDef->nextDefInWebIdx;
                    continue;
                }

                if (VIR_Inst_GetId(pDef->defKey.pDefInst) > ldstPos &&
                    VIR_Inst_GetId(pDef->defKey.pDefInst) < ldstPosEnd)
                {
                    return gcvTRUE;
                }

                VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                {
                    pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);

                    if (VIR_IS_IMPLICIT_USAGE_INST(pUsage->usageKey.pUsageInst))
                    {
                        continue;
                    }

                    if (VIR_Inst_GetId(pUsage->usageKey.pUsageInst) > ldstPos &&
                        VIR_Inst_GetId(pUsage->usageKey.pUsageInst) < ldstPosEnd)
                    {
                        return gcvTRUE;
                    }
                }

                defIdx = pDef->nextDefInWebIdx;
            }
        }
        pCurr = pCurr->nextActiveLR;
    }

    return gcvFALSE;
}


VSC_ErrCode  _VIR_RA_LS_AssignColorLR(
    VIR_RA_LS           *pRA,
    VIR_Function        *pFunc,
    VIR_RA_LS_Liverange *pLR,
    gctUINT             reservedDataReg)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    gctUINT             webIdx = pLR->webIdx;
    VIR_WEB             *pWeb = GET_WEB_BY_IDX(&pRA->pLvInfo->pDuInfo->webTable, webIdx);
    VIR_RA_HWReg_Color  curColor;
    gctBOOL             newColor = gcvFALSE, needHI = gcvFALSE;
    VIR_Enable          LREnable = VIR_RA_LS_LR2WebChannelMask(pRA, pLR);
    gctBOOL             debugEnabled = gcmOPT_DisableOPTforDebugger();

    curColor = InvalidColor;

    if (VIR_Shader_isDual16Mode(pShader) &&
        _VIR_RA_LS_GetWebPrecison(pRA, pWeb) == VIR_PRECISION_HIGH)
    {
        needHI = gcvTRUE;
    }

    if (!isLRInvalid(pLR) && !isLRSpilled(pLR))
    {
        curColor = _VIR_RA_GetLRColor(pLR);
        if (_VIR_RA_LS_IsInvalidColor(curColor))
        {
            if (isLRSubSampleDepth(pLR))
            {
                _VIR_RA_MakeColor(VIR_REG_MULTISAMPLEDEPTH, 0, &curColor);
                _VIR_RA_MakeColorHI(VIR_REG_MULTISAMPLEDEPTH, 0, &curColor);
            }
            else
            {
                VIR_DEF *pDefTemp = GET_DEF_BY_IDX(&pRA->pLvInfo->pDuInfo->defTable, pWeb->firstDefIdx);
                if (pDefTemp->flags.nativeDefFlags.bHwSpecialInput || pDefTemp->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
                {
                    _VIR_RA_MakeColor(pShader->sampleMaskIdRegStart,
                                      pShader->sampleMaskIdChannelStart,
                                      &curColor);
                    _VIR_RA_MakeColorHI(pShader->sampleMaskIdRegStart+1,
                                      pShader->sampleMaskIdChannelStart,
                                      &curColor);
                    /* special register should be treated as attributes */
                    pLR->colorFunc = VIR_RA_LS_ATTRIBUTE_FUNC;
                }
                else
                {
                    VIR_RA_HWReg_Type regType = pLR->hwType;
                    if (!debugEnabled ||
                        /* allocation register if still has register to use */
                        (!VIR_RA_LS_GetEnableDebug(pRA) &&
                         (VIR_RA_LS_GetColorPool(pRA)->colorMap[regType].maxAllocReg) <  _VIR_RA_LS_GetMaxReg(pRA, regType, reservedDataReg)) ||
                        !_VIR_RA_LS_isSpillable(pRA, pLR))
                    {
                        curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, needHI, reservedDataReg, -1, gcvNULL);
                        newColor = gcvTRUE;

                        /* this is to handle false dependency because ld/st does not
                           support component dependence check
                        */
                        if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                            (isLRSTDependence(pLR) || isLRLDDependence(pLR)))
                        {
                            if (!_VIR_RA_LS_IsInvalidLOWColor(curColor) &&
                                _VIR_RA_LS_TestUsedColor(pRA, pLR->hwType, curColor._hwRegId, 0xF))
                            {

                                while (!_VIR_RA_LS_IsInvalidLOWColor(curColor) &&
                                       _VIR_RA_LS_UseAfterInst(pRA, pLR, curColor._hwRegId))
                                {
                                    gctBOOL preferedColorOnly = gcvFALSE;
                                    VIR_RA_HWReg_Color  newColor;

                                    newColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, needHI,
                                        reservedDataReg, ((gctINT) curColor._hwRegId + 1), &preferedColorOnly);

                                    if (preferedColorOnly &&
                                        _VIR_RA_LS_IsTwoSameColor(newColor, curColor))
                                    {
                                        if (reservedDataReg == 0)
                                        {
                                            return VSC_RA_ERR_OUT_OF_REG_SPILL;
                                        }
                                        else
                                        {
                                            return VSC_RA_ERR_OUT_OF_REG_FAIL;
                                        }
                                    }
                                    else
                                    {
                                        curColor = newColor;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (_VIR_RA_LS_IsInvalidLOWColor(curColor) ||
                (needHI && _VIR_RA_LS_IsInvalidHIColor(curColor)))
            {
                if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                    VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
                {
                    VIR_LOG(pDumper, "could not find color for LR%d\n", pLR->webIdx);
                    VIR_LOG_FLUSH(pDumper);
                }
                /* we could not find a color */
                if (isLRA0OrB0(pLR))
                {
                    retValue = VSC_RA_ERR_OUT_OF_REG_FAIL;
                    return retValue;
                }
                else if (reservedDataReg == 0)
                {
                    retValue = VSC_RA_ERR_OUT_OF_REG_SPILL;
                    return retValue;
                }
                else
                {
                    /* handle the spill */
                    retValue = _VIR_RA_LS_SpillRegister(pRA, webIdx, pFunc, &curColor);
                    if (retValue == VSC_RA_ERR_OUT_OF_REG_FAIL)
                    {
                        return retValue;
                    }
                }
            }
            _VIR_RA_LS_AssignColorWeb(pRA, webIdx, pLR->hwType, curColor, pFunc);
        }
        else
        {
            /* attribute should stay in one color, no adjust color */
            if (pLR->colorFunc != pFunc &&
                pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
            {
                gctUINT  regIdx = 0, highDiff = 0;
                gctBOOL conflict = gcvFALSE;
                gctBOOL conflictHI = gcvFALSE;
                VIR_RA_HWReg_Color LRColor = _VIR_RA_GetLRColor(pLR);

                if (!_VIR_RA_LS_IsInvalidHIColor(LRColor) &&
                    pLR->regNoRange > 1)
                {
                    highDiff = _VIR_RA_Color_HIRegNo(LRColor) - _VIR_RA_Color_RegNo(LRColor);
                }

                /* the color is assigned in other place, try to see whether
                    there is any conflict */
                for (regIdx = 0; regIdx < pLR->regNoRange; regIdx++)
                {
                    if (_VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                            _VIR_RA_Color_RegNo(LRColor) + regIdx * (highDiff + 1),
                            _VIR_RA_Color_Channels(LREnable,
                                _VIR_RA_Color_Shift(LRColor))))
                    {
                        conflict = gcvTRUE;
                    }

                    if (!_VIR_RA_LS_IsInvalidHIColor(LRColor))
                    {
                        if (_VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                            _VIR_RA_Color_HIRegNo(LRColor) + regIdx * (highDiff + 1),
                            _VIR_RA_Color_Channels(LREnable,
                                _VIR_RA_Color_HIShift(LRColor))))
                        {
                            conflictHI = gcvTRUE;
                        }
                    }
                }
                /* if conflict, we need to find a brand new color for it */
                if (conflict)
                {
                    if (_VIR_RA_LS_FindBrandnewColor(pRA, pLR, &curColor, reservedDataReg))
                    {
                        _VIR_RA_SetLRColor(pLR,
                            _VIR_RA_Color_RegNo(curColor),
                            _VIR_RA_Color_Shift(curColor));
                    }
                    /* we could not find a color */
                    else
                    {
                        /* need hi color */
                        if (!_VIR_RA_LS_IsInvalidHIColor(LRColor))
                        {
                            /* release the hi color */
                            _VIR_RA_LS_ClearUsedColor(pRA, pLR->hwType,
                                    _VIR_RA_Color_HIRegNo(LRColor) + regIdx * (highDiff + 1),
                                    _VIR_RA_Color_Channels(LREnable,
                                        _VIR_RA_Color_HIShift(LRColor)));

                            if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                                (isLRSTDependence(pLR) ||
                                 isLRLDDependence(pLR)))
                            {
                                _VIR_RA_FlaseDepReg_Clear(pRA,
                                    _VIR_RA_Color_HIRegNo(LRColor) + regIdx * (highDiff + 1));
                            }

                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SPILLED);
                            _VIR_RA_SetLRSpillOffset(pLR, pRA->spillOffset);
                            pRA->spillOffset += 2 * pLR->regNoRange * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__;
                            conflictHI = gcvFALSE;
                        }
                        else
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SPILLED);
                            _VIR_RA_SetLRSpillOffset(pLR, pRA->spillOffset);
                            pRA->spillOffset += pLR->regNoRange * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__;
                            if (reservedDataReg == 0)
                            {
                                return VSC_RA_ERR_OUT_OF_REG_SPILL;
                            }
                        }
                    }
                }
                if (conflictHI)
                {
                    if (_VIR_RA_LS_FindBrandnewColor(pRA, pLR, &curColor, reservedDataReg))
                    {
                        _VIR_RA_SetLRColorHI(pLR,
                            _VIR_RA_Color_RegNo(curColor),
                            _VIR_RA_Color_Shift(curColor));
                    }
                    else
                    {
                        /* need low color */
                        if (!isLRSpilled(pLR))
                        {
                            /* release the low color */
                            _VIR_RA_LS_ClearUsedColor(pRA, pLR->hwType,
                                    _VIR_RA_Color_RegNo(LRColor) + regIdx * (highDiff + 1),
                                    _VIR_RA_Color_Channels(LREnable,
                                        _VIR_RA_Color_Shift(LRColor)));

                            if (!pRA->pHwCfg->hwFeatureFlags.supportPerCompDepForLS &&
                                (isLRSTDependence(pLR) ||
                                 isLRLDDependence(pLR)))
                            {
                                _VIR_RA_FlaseDepReg_Clear(pRA,
                                    _VIR_RA_Color_RegNo(LRColor) + regIdx * (highDiff + 1));
                            }

                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SPILLED);
                            _VIR_RA_SetLRSpillOffset(pLR, pRA->spillOffset);
                            pRA->spillOffset += 2 * pLR->regNoRange * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__;
                        }
                        else
                        {
                            /* should be done earlier */
                            gcmASSERT(gcvFALSE);
                        }
                    }
                }
            }
            newColor = gcvTRUE;
        }

        if (!(_VIR_RA_LS_IsInvalidLOWColor(curColor) ||
             (needHI && _VIR_RA_LS_IsInvalidHIColor(curColor))))
        {
            /* curColor is invalid in spill case and no need to add LR to active list */
            retValue = _VIR_RA_LS_AddActiveLRs(pRA, webIdx, newColor, pFunc, reservedDataReg);
        }
    }

    return retValue;
}

VSC_ErrCode _VIR_RA_LS_AssignColorsForGeneralReg(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc,
    gctUINT         reservedDataReg)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_TS_FUNC_FLOW    *pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(
        &VIR_RA_LS_GetLvInfo(pRA)->baseTsDFA.tsFuncFlowArray,
        pFunc->pFuncBlock->dgNode.id);

    VIR_RA_LS_Liverange *pLR, *pUsedColorLR;
    gctUINT             i = 0, defIdx;
    VIR_RA_HWReg_Color  curColor;
    VIR_DEF             *pDef = gcvNULL;
    gctBOOL             newColor = gcvTRUE;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "\nAssign colors:\t\t\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    /* before assigning for each function, reserve the color for attributes */
    _VIR_RA_LS_Reserve_AttrColor(pRA, pFunc);

    /* add live_in variables to the active LR list */
    while ((defIdx = vscBV_FindSetBitForward(&pFuncFlow->inFlow, i)) !=
        (gctUINT)INVALID_BIT_LOC)
    {
        i = defIdx + 1;

        pDef = GET_DEF_BY_IDX(&pRA->pLvInfo->pDuInfo->defTable, defIdx);

        /* skip the per-vertex input/output */
        if (_VIR_RA_LS_IsDefExcludedLR(pDef))
        {
            continue;
        }

        pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);

        /* for VX instruction, we have adjusted their startPoint to the firstDef,
           although they appear in function's liveIn
        */
        if (pLR->startPoint > 0)
        {
            gcmASSERT(isLRVX(pLR));
            continue;
        }

        if (_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pLR)))
        {
            retValue = _VIR_RA_LS_AssignColorLR(pRA, pFunc, pLR, reservedDataReg);
        }

        if (retValue != VSC_ERR_NONE)
        {
            goto OnError;
        }
    }

    /* for each liverange in order of increasing start point */
    pLR = VIR_RA_LS_GetSortedLRHead(pRA)->nextLR;
    while (pLR != &LREndMark)
    {
        gcmASSERT(pLR->liveFunc == pFunc);

        /* expire the old not active LRs */
        _VIR_RA_LS_ExpireActiveLRs(pRA, pLR->startPoint);

        if (!isLRInvalid(pLR) && !isLRSpilled(pLR))
        {
            if (_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pLR)))
            {
                curColor = InvalidColor;

                /* dual16 should not be assigned used color
                   for example:
                     000  BB1:
                     001     r1 <- ...
                     002     ...
                     003     if cond goto BB3 else goto BB2
                     004  BB2:
                     005     ... <- r1
                     006     ...
                     007     goto BB4
                     008  BB3:
                     009     ... <- r1
                     010  BB4:
                     011  ...
                     we have a dead interval 6-9. But if r1 is assigned to new LR at 6,
                     it will have problem. In particular, T0 goes to BB2, r1 is re-assigned at 6.
                     Then T1 goes to BB3, r1 value is already changed.
                   */
                if (pLR->regNoRange == 1 &&
                   !VIR_Shader_isDual16Mode(pShader) &&
                   !isLRNoUsedColor(pLR))
                {
                     /* check whether we could find a color in the active LRs */
                    pUsedColorLR = gcvNULL;
                    curColor = _VIR_RA_LS_FindUsedColor(pRA, pLR->webIdx, &pUsedColorLR);
                    if (!_VIR_RA_LS_IsInvalidColor(curColor))
                    {
                        gcmASSERT(pUsedColorLR != gcvNULL);
                        pLR->usedColorLR = pUsedColorLR;
                        pUsedColorLR->deadIntervalAvail = gcvFALSE;
                        newColor = gcvFALSE;

                        /* find a used color */
                        _VIR_RA_LS_AssignColorWeb(pRA,
                                      pLR->webIdx,
                                      pLR->hwType,
                                      curColor,
                                      pFunc);

                        /* add the LR to the active LRs list */
                        retValue = _VIR_RA_LS_AddActiveLRs(pRA, pLR->webIdx, newColor, pFunc, reservedDataReg);

                        /* find a used color */
                        pLR = pLR->nextLR;
                        continue;
                    }
                }
            }
            retValue = _VIR_RA_LS_AssignColorLR(pRA, pFunc, pLR, reservedDataReg);

            if (retValue != VSC_ERR_NONE)
            {
                goto OnError;
            }
        }

        pLR = pLR->nextLR;
    }

    /* expire the LR that ends at the end of function */
    _VIR_RA_LS_ExpireActiveLRs(pRA, VIR_Function_GetInstCount(pFunc) + 1);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "\n============== liverange coloring [%s] ==============\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_RS_LS_DumpSortedLRTable(pRA, pFunc, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

OnError:
    return retValue;
}

/* ===========================================================================
   functions to rewrite the code
   fill the operand's hw register info
   ===========================================================================
*/
void _VIR_RA_LS_SetOperandHwRegInfo(
    VIR_RA_LS           *pRA,
    VIR_Operand         *pOpnd,
    VIR_RA_HWReg_Color  color)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);

    VIR_Operand_SetRegAllocated(pOpnd);

    gcmASSERT(!_VIR_RA_LS_IsInvalidLOWColor(color));

    gcmASSERT(VIR_RA_INVALID_REG != _VIR_RA_Color_RegNo(color));
    gcmASSERT(_VIR_RA_Color_Shift(color) <= 3);
    VIR_Operand_SetHwRegId(pOpnd, _VIR_RA_Color_RegNo(color));
    VIR_Operand_SetHwShift(pOpnd, _VIR_RA_Color_Shift(color));

    if (VIR_Shader_isDual16Mode(pShader) &&
        VIR_Operand_GetPrecision(pOpnd) == VIR_PRECISION_HIGH)
    {
        gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(color));
        VIR_Operand_SetHIHwRegId(pOpnd, _VIR_RA_Color_HIRegNo(color));
        VIR_Operand_SetHIHwShift(pOpnd, _VIR_RA_Color_HIShift(color));
    }
}

void _VIR_RA_LS_SetSymbolHwRegInfo(
    VIR_RA_LS           *pRA,
    VIR_Symbol          *pSym,
    VIR_RA_LS_Liverange *pLR,
    gctUINT             diffReg)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Symbol          *varSym;
    gctUINT             i;

    if (VIR_Symbol_isVariable(pSym))
    {
        /* ps output gl_layer must be allocated to r0.w */
        if (VIR_Symbol_GetName(pSym) == VIR_NAME_PS_OUT_LAYER &&
            pShader->shaderKind == VIR_SHADER_FRAGMENT)
        {
            /* r0.w is not allocated */
            gcmASSERT(_VIR_RA_LS_TestUsedColor(pRA, VIR_RA_HWREG_GR, 0, 0x8) == gcvFALSE);
            VIR_Symbol_SetHwRegId(pSym, 0);
            VIR_Symbol_SetHwShift(pSym, 3);
            _VIR_RA_SetLRColor(pLR, 0, 3);
            if (VIR_Shader_isDual16Mode(pShader))
            {
                gcmASSERT(VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH);
                /* r1.w is not allocated */
                gcmASSERT(_VIR_RA_LS_TestUsedColor(pRA, VIR_RA_HWREG_GR, 1, 0x8) == gcvFALSE);
                VIR_Symbol_SetHIHwRegId(pSym, 1);
                VIR_Symbol_SetHIHwShift(pSym, 3);
                _VIR_RA_SetLRColorHI(pLR, 1, 3);
            }
            else
            {
                 _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
            }
        }
        else if(VIR_Symbol_isInput(pSym))
        {
            /* If the symbol is attribute. The Shift and RegId must have be set at _VIR_RA_LS_AssignAttributes
               and this value can not be changed.
            */
        }
        else
        {
            if (!isLRInvalid(pLR))
            {
                VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));

                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                {
                    gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                    VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                    VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                }

                if (isLRDynIndexing(pLR))
                {
                    for (i = 0; i < pLR->regNoRange; i++)
                    {
                        pSym = VIR_Shader_FindSymbolByTempIndex(pShader, pLR->firstRegNo + i);

                        VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + i);
                        VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                        if (VIR_Shader_isDual16Mode(pShader) &&
                            VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                        {
                            gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                            VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + i);
                            VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                        }
                    }
                }
                else
                {
                    pSym = VIR_Shader_FindSymbolByTempIndex(pShader, pLR->firstRegNo);
                    VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                    VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                    if (VIR_Shader_isDual16Mode(pShader) &&
                        VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                    {
                        gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                        VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                        VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                    }
                }
            }
        }
    }
    else if (VIR_Symbol_isVreg(pSym))
    {
        varSym = VIR_Symbol_GetVregVariable(pSym);
        if (varSym && VIR_Symbol_isVariable(varSym))
        {
            /* ps output gl_layer must be allocated to r0.w or r0.w/r1.w for dual16 */
            if (VIR_Symbol_GetName(varSym) == VIR_NAME_PS_OUT_LAYER &&
                pShader->shaderKind == VIR_SHADER_FRAGMENT)
            {
                /* r0.w is not allocated */
                gcmASSERT(_VIR_RA_LS_TestUsedColor(pRA, VIR_RA_HWREG_GR, 0, 0x8) == gcvFALSE);
                VIR_Symbol_SetHwRegId(varSym, 0);
                VIR_Symbol_SetHwShift(varSym, 3);
                _VIR_RA_SetLRColor(pLR, 0, 3);
                gcmASSERT(VIR_Symbol_GetPrecision(varSym) == VIR_PRECISION_HIGH);
                if (VIR_Shader_isDual16Mode(pShader))
                {
                    gcmASSERT(_VIR_RA_LS_TestUsedColor(pRA, VIR_RA_HWREG_GR, 1, 0x8) == gcvFALSE);
                    VIR_Symbol_SetHIHwRegId(varSym, 1);
                    VIR_Symbol_SetHIHwShift(varSym, 3);
                    _VIR_RA_SetLRColorHI(pLR, 1, 3);
                }
                else
                {
                    _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
                }
            }
            else if(VIR_Symbol_isInput(varSym))
            {
                /* If the symbol is attribute. The Shift and RegId must have be set at _VIR_RA_LS_AssignAttributes
                   and this value can not be changed.
                */
            }
            else
            {
                if (!isLRInvalid(pLR))
                {
                    VIR_Symbol_SetHwRegId(varSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)));
                    VIR_Symbol_SetHwShift(varSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));

                    if (VIR_Shader_isDual16Mode(pShader) &&
                        VIR_Symbol_GetPrecision(varSym) == VIR_PRECISION_HIGH)
                    {
                        gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                        VIR_Symbol_SetHIHwRegId(varSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)));
                        VIR_Symbol_SetHIHwShift(varSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                    }

                    if (isLRDynIndexing(pLR))
                    {
                        for (i = 0; i < pLR->regNoRange; i++)
                        {
                            pSym = VIR_Shader_FindSymbolByTempIndex(pShader, pLR->firstRegNo + i);

                            VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + i);
                            VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                            if (VIR_Shader_isDual16Mode(pShader) &&
                                VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                            {
                                gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                                VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + i);
                                VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                            }
                        }
                    }
                    else
                    {
                        VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                        VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                        if (VIR_Shader_isDual16Mode(pShader) &&
                            VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                        {
                            gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                            VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                            VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                        }
                    }
                }
            }
        }
        else
        {
            if (!isLRInvalid(pLR))
            {
                VIR_Symbol_SetHwRegId(pSym, _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                VIR_Symbol_SetHwShift(pSym, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
                {
                    gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                    VIR_Symbol_SetHIHwRegId(pSym, _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg);
                    VIR_Symbol_SetHIHwShift(pSym, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)));
                }
            }
        }
    }
}

void    _VIR_RA_LS_ComputeAttrIndexEnable(
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd,
    VIR_Symbol      *Sym,
    gctINT          *attrIndex,
    VIR_Enable      *enable)
{
    VIR_OperandInfo operandInfo;
    VIR_Symbol      *varSym = gcvNULL;
    VIR_Enable      firstChannelEnable;

    if (!Sym)
    {
        gcmASSERT(pInst && pOpnd);

        varSym = VIR_Operand_GetUnderlyingSymbol(pOpnd);
    }
    else
    {
        varSym = Sym;
    }

    gcmASSERT(varSym);
    gcmASSERT(VIR_Symbol_GetHwFirstCompIndex(varSym) != NOT_ASSIGNED);

    if (attrIndex)
    {
        *attrIndex = (VIR_Symbol_GetHwFirstCompIndex(varSym) / CHANNEL_NUM);
    }

    firstChannelEnable = (1 << (VIR_Symbol_GetHwFirstCompIndex(varSym) % CHANNEL_NUM));

    /* Currently, we don't support packed memory, otherwise, we need split several load_attr
       or store_attr with different enables. We will consider packed memory later when calc
       hwFirstCompIndex in linker */
    if (enable)
    {
        if (pOpnd)
        {
            gctUINT immIdxNo = VIR_Operand_GetMatrixConstIndex(pOpnd);

            VIR_Operand_GetOperandInfo(pInst,
                                       pOpnd,
                                       &operandInfo);

            if (VIR_Symbol_GetName(varSym) == VIR_NAME_TESS_LEVEL_OUTER ||
                VIR_Symbol_GetName(varSym) == VIR_NAME_TESS_LEVEL_INNER)
            {
                /* packed mode, change the enable */
                gcmASSERT(!VIR_Operand_GetIsConstIndexing(pOpnd));
                gcmASSERT(firstChannelEnable == VIR_ENABLE_X);
                gcmASSERT(operandInfo.u1.virRegInfo.virReg + immIdxNo - varSym->u2.tempIndex <= 3);
                *enable = (firstChannelEnable << (gctINT)(operandInfo.u1.virRegInfo.virReg + immIdxNo - varSym->u2.tempIndex));
            }
            else
            {
                /* not packed mode, change the attrIndex */
                *attrIndex = *attrIndex + (operandInfo.u1.virRegInfo.virReg + immIdxNo - varSym->u2.tempIndex);
            }
        }
        else
        {
            if (VIR_Symbol_GetName(varSym) == VIR_NAME_PRIMITIVE_ID)
            {
                *enable = firstChannelEnable;
            }
            else
            {
                gcmASSERT(firstChannelEnable == VIR_ENABLE_X);
                *enable = VIR_ENABLE_XYZW;
            }
        }
    }
}

VSC_ErrCode
_VIR_RA_LS_RewriteColor_Src(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pSrcInst,
    VIR_Operand     *pSrcOpnd,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd
    );

VSC_ErrCode
_VIR_RA_LS_RewriteColor_Dest(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd
    );

/* functions for register spill */
VSC_ErrCode
_VIR_RA_LS_GenTemp(
    VIR_RA_LS       *pRA,
    VIR_SymId       *pSymId)
{
    VSC_ErrCode         virErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_VirRegId        regId    = VIR_Shader_NewVirRegId(pShader, 1);

    virErrCode = VIR_Shader_AddSymbol(
        pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X4),
        VIR_STORAGE_UNKNOWN,
        pSymId);

    return virErrCode;
}

static gctUINT
_VIR_RA_LS_ComputeOpndShift(
    VIR_Operand         *pOpnd
    )
{
    gctUINT opndShift = 0;
    VIR_Enable opndEnable = VIR_ENABLE_NONE;

    if (VIR_Operand_isLvalue(pOpnd))
    {
        opndEnable = VIR_Operand_GetEnable(pOpnd);
    }
    else
    {
        opndEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd));
    }

    for (opndShift = 0; opndShift < VIR_CHANNEL_COUNT; opndShift ++)
    {
        if ((opndEnable & (0x1 << opndShift)) != 0)
        {
            break;
        }
    }
    gcmASSERT(opndShift < 4);
    return opndShift;
}

static gctUINT
_VIR_RA_LS_ComputeHwRegComponentSize(
    VIR_Shader*         pShader,
    VIR_TypeId          typeId
    )
{
    gctUINT             compSize = 1;
    VIR_Type*           pCompType;

    if (!VIR_TypeId_isPrimitive(typeId))
    {
        VIR_Type*       pType = VIR_Shader_GetTypeFromId(pShader, typeId);

        if (VIR_Type_isArray(pType))
        {
            while (VIR_Type_isArray(pType))
            {
                pType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pType));
            }
            typeId = VIR_Type_GetBaseTypeId(pType);
        }
        else if (VIR_Type_isPointer(pType))
        {
            pType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pType));
            typeId = VIR_Type_GetBaseTypeId(pType);
        }
    }

    gcmASSERT(VIR_TypeId_isPrimitive(typeId));

    pCompType = VIR_Shader_GetTypeFromId(pShader, VIR_GetTypeComponentType(typeId));
    compSize = VIR_Type_GetSize(pCompType);

    /*
    ** The maximum size of a one HW register componenet is 4,
    ** for those vir registers whose component size is larger than 4, e.g. uint, long,
    ** we use multiple HW componenets to descripte them, but here we need to use the HW componenet size.
    */
    compSize = vscMIN(compSize, __MAX_TEMP_REGISTER_COMPONENT_SIZE_IN_BYTE__);

    gcmASSERT(compSize != 0);

    return compSize;
}

static gctUINT
_VIR_RA_LS_ComputeSpillOffset(
    VIR_RA_LS           *pRA,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VIR_Symbol  *opndSym = VIR_Operand_GetSymbol(pOpnd);
    gctUINT     spillOffset = 0, swizzleOffset = 0, constOffset = 0;
    gctUINT     compSize = _VIR_RA_LS_ComputeHwRegComponentSize(pRA->pShader, VIR_Symbol_GetTypeId(opndSym));

    /* load/store destinationMask has "shift", e.g.,
       store.yz base, offset, data (similar to store.xy) ==>
       base[0] and base[4] was changed */
    swizzleOffset = _VIR_RA_LS_ComputeOpndShift(pOpnd);

    /* matrix const */
    constOffset = VIR_Operand_GetMatrixConstIndex(pOpnd);

    /* array const (already include matrix type)
       for example mat4 temp[10];
       temp[3][1].x ==> temp(0+1+12).x
       matrixConstIndex = 1
       constIndexingImmed = 12 */
    if (VIR_Operand_GetIsConstIndexing(pOpnd))
    {
        constOffset += VIR_Operand_GetConstIndexingImmed(pOpnd);
    }

    /* spillOffset = variableSpillOffset + tempOffset + swizzleOffset
       since the load/store use swizzle XYZW, thus we need swizzleOffset */
    gcmASSERT(VIR_Symbol_GetKind(opndSym) == VIR_SYM_VIRREG);
    spillOffset = _VIR_RA_GetLRSpillOffset(pLR) +
        (opndSym->u1.vregIndex - pLR->firstRegNo + constOffset) * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__ + swizzleOffset * compSize;
    gcmASSERT(spillOffset < pRA->spillOffset);

    return spillOffset;
}

static VIR_Swizzle
_VIR_RA_LS_SwizzleWShift(
    VIR_Operand     *pOpnd)
{
    gctUINT i;
    VIR_Swizzle srcSwizzle = VIR_Operand_GetSwizzle(pOpnd);
    gctUINT    swizzleShift = _VIR_RA_LS_ComputeOpndShift(pOpnd);
    VIR_Swizzle  newSwizzle = 0;

    gcmASSERT(!VIR_Operand_isLvalue(pOpnd));

    for (i = 0 ; i < VIR_CHANNEL_COUNT; i++)
    {
        gcmASSERT((gctUINT)VIR_Swizzle_GetChannel(srcSwizzle, i) >= swizzleShift);
        VIR_Swizzle_SetChannel(newSwizzle, i,
            VIR_Swizzle_GetChannel(srcSwizzle, i) - swizzleShift);
    }
    return newSwizzle;
}

static VIR_Enable
_VIR_RA_LS_EnableWShift(
    VIR_Operand     *pOpnd)
{
    gctUINT i;
    VIR_Enable dstEnable = VIR_Operand_GetEnable(pOpnd);
    gctUINT    enableShift = _VIR_RA_LS_ComputeOpndShift(pOpnd);
    VIR_Enable  newEnable = VIR_ENABLE_NONE;

    gcmASSERT(VIR_Operand_isLvalue(pOpnd));

    for (i = 0 ; i < VIR_CHANNEL_COUNT; i++)
    {
        if (dstEnable & (1 << i))
        {
            gcmASSERT(i >= enableShift);

            newEnable = newEnable | (1 << (i - enableShift));
        }
    }

    return newEnable;
}

static VSC_ErrCode
_VIR_RA_LS_SetHWRegForBaseRegister(
    VIR_RA_LS*          pRA,
    VIR_Operand*        pOpnd,
    gctUINT             shift
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_RA_HWReg_Color  curColor = {0, 0, 0, 0, 0};
    VIR_Shader*         pShader = VIR_RA_LS_GetShader(pRA);

    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);

    _VIR_RA_MakeColor(pRA->baseRegister, shift, &curColor);
    if (VIR_Shader_isDual16Mode(pShader))
    {
        VIR_Operand_SetPrecision(pOpnd, VIR_PRECISION_HIGH);
        _VIR_RA_MakeColorHI(pRA->baseRegister + 1, 0, &curColor);
    }
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);

    return retErrCode;
}

static VSC_ErrCode
_VIR_RA_LS_InsertSpill(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_RA_HWReg_Color  curColor;
    gctUINT             i = 0;
    VIR_Swizzle         newSwizzle  = _VIR_RA_LS_SwizzleWShift(pOpnd);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "spill instruction:\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    curColor = InvalidColor;

    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_LOAD_S,
        VIR_Operand_GetTypeId(pOpnd),
        pInst,
        gcvTRUE,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    newInst->sourceLoc = pInst->sourceLoc;

    /* src0 - base */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* src1 offset */
    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* set the dest for the newInst - should be set to the orig symbol */
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                VIR_Operand_GetSymbolId_(pOpnd),
                                VIR_Operand_GetTypeId(pOpnd));
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* HW can't support FP16 directly, it converts FP16 to FP32, so we need to use UINT16. */
    VIR_Operand_SetTypeId(newInst->dest, VIR_TypeId_ConvertFP16Type(pShader, VIR_Operand_GetTypeId(pOpnd)));

    for (i = 0 ; i < VIR_RA_LS_DATA_REG_NUM; i++)
    {
        if ((pRA->dataRegisterUsedMaskBit & (1 << i)) == 0)
        {
            gcmASSERT(pRA->dataRegister[i] != VIR_INVALID_ID);

            if (_VIR_RA_LS_OperandEvenReg(pRA, pInst, pOpnd))
            {
                if ((pRA->dataRegister[i] & 0x01) == 1)
                {
                    continue;
                }
            }
            else if (_VIR_RA_LS_OperandOddReg(pRA, pInst, pOpnd))
            {
                if ((pRA->dataRegister[i] & 0x01) == 0)
                {
                    continue;
                }
            }

            _VIR_RA_MakeColor(pRA->dataRegister[i], 0, &curColor);
            pRA->dataRegisterUsedMaskBit |= (1 << i);
            break;
        }
    }
    gcmASSERT(i < VIR_RA_LS_DATA_REG_NUM);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_Swizzle_2_Enable(newSwizzle));

    /* set the src for the pInst */
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
    VIR_Operand_SetSwizzle(pOpnd, newSwizzle);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "==>\n");
        VIR_Inst_Dump(pDumper, newInst);
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG(pDumper, "\n");
        VIR_LOG_FLUSH(pDumper);
    }

    return retErrCode;
}

static VSC_ErrCode
_VIR_RA_LS_InsertSpillForDest(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR,
    VIR_HwRegId         *pDataRegister,
    gctUINT             *pDataRegisterIdx
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_RA_HWReg_Color  curColor;
    gctUINT             i = 0;
    VIR_HwRegId         dataRegister = VIR_INVALID_ID;
    gctUINT             dataRegisterIdx = 0;

    gcmASSERT(VIR_Operand_isLvalue(pOpnd));

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "spill instruction:\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    curColor = InvalidColor;

    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_LOAD_S,
        VIR_Operand_GetTypeId(pOpnd),
        pInst,
        gcvTRUE,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    newInst->sourceLoc = pInst->sourceLoc;

    /* src0 - base */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* src1 offset */
    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* set the dest for the newInst - should be set to the orig symbol */
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                VIR_Operand_GetSymbolId_(pOpnd),
                                VIR_Operand_GetTypeId(pOpnd));
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* HW can't support FP16 directly, it converts FP16 to FP32, so we need to use UINT16. */
    VIR_Operand_SetTypeId(newInst->dest, VIR_TypeId_ConvertFP16Type(pShader, VIR_Operand_GetTypeId(pOpnd)));

    for (i = 0 ; i < VIR_RA_LS_DATA_REG_NUM; i++)
    {
        if ((pRA->dataRegisterUsedMaskBit & (1 << i)) == 0)
        {
            gcmASSERT(pRA->dataRegister[i] != VIR_INVALID_ID);

            if (_VIR_RA_LS_OperandEvenReg(pRA, pInst, pOpnd))
            {
                if ((pRA->dataRegister[i] & 0x01) == 1)
                {
                    continue;
                }
            }
            else if (_VIR_RA_LS_OperandOddReg(pRA, pInst, pOpnd))
            {
                if ((pRA->dataRegister[i] & 0x01) == 0)
                {
                    continue;
                }
            }

            _VIR_RA_MakeColor(pRA->dataRegister[i], 0, &curColor);
            pRA->dataRegisterUsedMaskBit |= (1 << i);
            dataRegisterIdx = i;
            dataRegister= pRA->dataRegister[i];
            break;
        }
    }
    gcmASSERT(i < VIR_RA_LS_DATA_REG_NUM);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_Operand_GetEnable(pOpnd));

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "==>\n");
        VIR_Inst_Dump(pDumper, newInst);
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG(pDumper, "\n");
        VIR_LOG_FLUSH(pDumper);
    }

    if (pDataRegister)
    {
        *pDataRegister = dataRegister;
    }
    if (pDataRegisterIdx)
    {
        *pDataRegisterIdx = dataRegisterIdx;
    }

    return retErrCode;
}

static gctBOOL
_VIR_RA_LS_NeedToSpillDest(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR,
    gctBOOL             *pSaveInSpillDataRegister,
    gctUINT             *pEndPoint
    )
{
    VIR_Instruction     *pNextInst = VIR_Inst_GetNext(pInst);
    gctBOOL             bSaveInSpillDataRegister = gcvFALSE;
    gctUINT             endPoint = VIR_INVALID_ID;

    if (!(/* Righ now only check GR.*/
          pLR->hwType == VIR_RA_HWREG_GR &&
          /* Skip even/odd pair. */
          !isLRVXEven(pLR) && !isLRVXOdd(pLR)))
    {
        return gcvTRUE;
    }

    /* Usage must be next to the def. */
    if (pNextInst && VIR_Inst_GetBasicBlock(pNextInst) == VIR_Inst_GetBasicBlock(pInst))
    {
        /* Only have one def and one usage.*/
        if (pLR->deadIntervals == gcvNULL &&
            pLR->startPoint == (gctUINT)VIR_Inst_GetId(pInst) &&
            (pLR->startPoint + 1 == pLR->endPoint) &&
            pLR->endPoint == (gctUINT)VIR_Inst_GetId(pNextInst))
        {
            bSaveInSpillDataRegister = gcvTRUE;
            endPoint = pLR->endPoint;

            if (pSaveInSpillDataRegister)
            {
                *pSaveInSpillDataRegister = bSaveInSpillDataRegister;
            }
            if (pEndPoint)
            {
                *pEndPoint = endPoint;
            }
            return gcvFALSE;
        }
        else
        {
            VIR_Operand                 *pSrcOpnd = gcvNULL;
            VIR_OperandInfo             srcOpndInfo, destOpndInfo;
            VIR_Enable                  enable = VIR_Operand_GetEnable(pOpnd);
            gctUINT                     i;

            VIR_Operand_GetOperandInfo(pInst, pOpnd, &destOpndInfo);

            for (i = 0; i < VIR_Inst_GetSrcNum(pNextInst); i++)
            {
                pSrcOpnd = VIR_Inst_GetSource(pNextInst, i);
                if (pSrcOpnd == gcvNULL)
                {
                    continue;
                }

                VIR_Operand_GetOperandInfo(pNextInst, pSrcOpnd, &srcOpndInfo);
                if (!srcOpndInfo.isVreg)
                {
                    continue;
                }

                if (srcOpndInfo.u1.virRegInfo.virReg != destOpndInfo.u1.virRegInfo.virReg)
                {
                    continue;
                }

                if (!VIR_Enable_Covers(enable, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pSrcOpnd))))
                {
                    continue;
                }

                bSaveInSpillDataRegister = gcvTRUE;
                endPoint = VIR_Inst_GetId(pNextInst);
                break;
            }

            if (pSaveInSpillDataRegister)
            {
                *pSaveInSpillDataRegister = bSaveInSpillDataRegister;
            }
            if (pEndPoint)
            {
                *pEndPoint = endPoint;
            }
            return gcvTRUE;
        }
    }

    return gcvTRUE;
}

static VSC_ErrCode
_VIR_RA_LS_InsertFill(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId;
    VIR_RA_HWReg_Color  curColor = {0, 0, 0, 0, 0};
    VIR_Operand         *newOpnd = gcvNULL;

    gctBOOL             needToReloadDest = gcvFALSE;
    gctBOOL             needToSpillDest = gcvTRUE;
    VIR_HwRegId         destHwRegId = VIR_INVALID_ID;
    gctUINT             dataRegisterIdx = 0, dataRegisterEndPoint = VIR_INVALID_ID;
    gctBOOL             bSaveInSpillDataRegister = gcvFALSE;

    VIR_Symbol          *opndSym = VIR_Operand_GetSymbol(pOpnd);
    gctUINT              opndSymTypeSize, dataOpndTypeSize;

    /*
    ** Check dest:
    **  For those instructions that some of the enabled dest channels may not be written,
    **  if it is spilled, we need to spill the dest first.
    */
    if (!vscVIR_IsInstDefiniteWrite(VIR_RA_LS_GetLvInfo(pRA)->pDuInfo, pInst, VIR_INVALID_ID, gcvFALSE))
    {
        needToReloadDest = gcvTRUE;
    }

    if (needToReloadDest)
    {
        _VIR_RA_LS_InsertSpillForDest(pRA, pInst, pOpnd, pLR, &destHwRegId, &dataRegisterIdx);
    }
    else
    {
        destHwRegId = pRA->dataRegister[0];
        pRA->dataRegisterUsedMaskBit |= (1 << 0);
        dataRegisterIdx = 0;
    }

    gcmASSERT(dataRegisterIdx < VIR_RA_LS_DATA_REG_NUM);

    if (!needToReloadDest &&
        VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption), VSC_OPTN_RAOptions_SPILL_DEST_OPT) &&
        !_VIR_RA_LS_NeedToSpillDest(pRA, pInst, pOpnd, pLR, &bSaveInSpillDataRegister, &dataRegisterEndPoint))
    {
        needToSpillDest = gcvFALSE;
    }

    /* Save in the spillDataRegister. */
    if (bSaveInSpillDataRegister)
    {
        pRA->dataRegisterEndPoint[dataRegisterIdx] = dataRegisterEndPoint;
        _VIR_RA_MakeColor(destHwRegId, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);

        if (needToSpillDest)
        {
            _VIR_RA_SetLRTempColor(pLR, _VIR_RA_Color_RegNo(curColor), _VIR_RA_Color_Shift(curColor), gcvFALSE);
            _VIR_RA_SetLRTempColorEndPoint(pLR, dataRegisterEndPoint, gcvFALSE);
        }
        else
        {
            /* Change this LR to non-spilled. */
            VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_SPILLED);
            _VIR_RA_SetLRColor(pLR, _VIR_RA_Color_RegNo(curColor), _VIR_RA_Color_Shift(curColor));

            return retErrCode;
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "fill instruction:\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    curColor = InvalidColor;

    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_STORE_S,
        VIR_Operand_GetTypeId(pOpnd),
        pInst,
        gcvTRUE,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    newInst->sourceLoc = pInst->sourceLoc;

    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* compute offset */
    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* src2 */
    newOpnd =  VIR_Inst_GetSource(newInst, VIR_Operand_Src2);
    VIR_Operand_Copy(newOpnd, pOpnd);
    VIR_Operand_Change2Src(newOpnd);

    /* HW can't support FP16 directly, it converts FP16 to FP32, so we need to use UINT16. */
    VIR_Operand_SetTypeId(newOpnd, VIR_TypeId_ConvertFP16Type(pShader, VIR_Operand_GetTypeId(newOpnd)));

    /*
    ** Since we use the symbol's type to compute the offset and use the SRC2's type as the InstType,
    ** if the type size of SRC2 is larger than the symbol's, some data may be abandoned.
    */
    opndSymTypeSize = _VIR_RA_LS_ComputeHwRegComponentSize(pShader, VIR_Symbol_GetTypeId(opndSym));
    dataOpndTypeSize = _VIR_RA_LS_ComputeHwRegComponentSize(pShader, VIR_Operand_GetTypeId(newOpnd));

    if (dataOpndTypeSize > opndSymTypeSize)
    {
        WARNING_REPORT(retErrCode, "Data type mismatch, some data may be abandoned!");
    }

    /* fill (destination)  */
    gcmASSERT(destHwRegId != VIR_INVALID_ID);
    _VIR_RA_MakeColor(destHwRegId, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);

    /* change the old dest to tempRegister */
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);

    /* set the dest for the newInst
       since src2 is always in temp, only dest enable matter.
        */
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, _VIR_RA_LS_EnableWShift(pOpnd));

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "==>\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_Inst_Dump(pDumper, newInst);
        VIR_LOG(pDumper, "\n");
        VIR_LOG_FLUSH(pDumper);
    }

    return retErrCode;
}

/*
    insert the MAD to compute the offset
    1: mad  offset.x, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__, spillOffset
*/
static VSC_ErrCode
_VIR_RA_LS_InsertSpillOffset(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR,
    gctBOOL             isLDARR)
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    VIR_Instruction     *madInst = gcvNULL, *movInst = gcvNULL;
    VIR_RA_HWReg_Color  curColor;
    VIR_Operand         *src0Opnd, *idxOpnd;
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF             *pDef;

    VIR_Instruction     *pMovaInst = gcvNULL;
    VIR_Operand         *pMovaSrc0Opnd = gcvNULL;
    VIR_RA_LS_Liverange *pMovaLR;
    gctUINT             movaWebIdx;
    gctBOOL             bMovaSrc0Spill = gcvFALSE;

    VIR_SymId           tmpSymId = VIR_INVALID_ID;
    gctUINT             shift = 0;
    VSC_MM              *pMM =  VIR_RA_LS_GetMM(pRA);
    VIR_Enable          enable_channel;

    /* Generate the temp register for spillOffset. */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }

    curColor = InvalidColor;

    /* mad instruction */
    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_MAD,
        VIR_TYPE_UINT32,
        pInst,
        gcvTRUE,
        &madInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - mova src0 */
    if (isLDARR)
    {
        idxOpnd = pInst->src[VIR_Operand_Src1];
    }
    else
    {
        idxOpnd = pInst->src[VIR_Operand_Src0];
    }

    /* compute the enable according to the usage */
    enable_channel = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(idxOpnd));
    /* enable channel should be one channel since ldarr/starr is not compontwise */
    if (!_VSC_RA_EnableSingleChannel(enable_channel))
    {
        gcmASSERT(0);
    }

    vscVIR_InitGeneralUdIterator(&udIter, pLvInfo->pDuInfo, pInst, idxOpnd, gcvFALSE, gcvFALSE);
    for(pDef = vscVIR_GeneralUdIterator_First(&udIter);
        pDef != gcvNULL;
        pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        if (VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_MOVA)
        {
            gcmASSERT(vscVIR_IsUniqueDefInstOfUsageInst(
                        pLvInfo->pDuInfo,
                        pInst,
                        idxOpnd,
                        gcvFALSE,
                        pDef->defKey.pDefInst,
                        gcvNULL));
            break;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    pMovaInst = pDef->defKey.pDefInst;
    pMovaSrc0Opnd = VIR_Inst_GetSource(pMovaInst, 0);

    /* Check if the SRC0 of this MOVA is spilled or not. */
    movaWebIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pMovaInst, pMovaSrc0Opnd);
    if (VIR_INVALID_WEB_INDEX != movaWebIdx)
    {
        pMovaLR = _VIR_RA_LS_Web2ColorLR(pRA, movaWebIdx);
        bMovaSrc0Spill = isLRSpilled(pMovaLR);
    }

    /* insert a MOV after MOVA, in case mova_src0 is redefined,
       to-do: insert only when redefined
       add enable_channel to hash function for case like one defInst has multi usages
       030: MOVA               ivec4 addr_reg  temp(1503), uvec4 temp(1753)
       031: LDARR              char temp(36).x, char temp(970).x, ivec4 temp(1503).x
       032: LDARR              char_X2 temp(1248).x, char temp(970).x, ivec4 temp(1503).y
       033: LDARR              char_X2 temp(1248).y, char temp(970).x, ivec4 temp(1503).z
       ...
       ivec4 temp(1503).x, ivec4 temp(1503).y, ivec4 temp(1503).z has same defInst but need different enable channel
    */
    /* We always need to insert a MOV if we don't reserve any register for MOVA or the src1 of MOVA is spilled. */
    if (!pRA->bReservedMovaReg
        ||
        bMovaSrc0Spill
        ||
        !_VSC_RA_MOVA_GetConstVal(pRA->movaHash, pDef->defKey.pDefInst, enable_channel, (void *)&movInst))
    {
        if (isLDARR)
        {
            retErrCode = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_MOV,
                VIR_TYPE_UINT32,
                madInst,
                gcvTRUE,
                &movInst);
        }
        else
        {
            /* TODO::we cann't handle vectorized MOVA + STARR now
             * check MOVA src0 is single channel if usage of dest is STARR
             */
            if (!_VSC_RA_EnableSingleChannel(enable_channel))
            {
                gcmASSERT(gcvFALSE);
            }
            retErrCode = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_MOV,
                VIR_TYPE_UINT32,
                madInst,
                gcvTRUE,
                &movInst);
        }

        if (retErrCode != VSC_ERR_NONE) return retErrCode;
        src0Opnd = VIR_Inst_GetSource(movInst, VIR_Operand_Src0);
        VIR_Operand_Copy(src0Opnd, pMovaSrc0Opnd);

        /*
        ** When the SRC0 of this MOVA is spilled, we need to mark it as INVALID so we can reload it from the spill memory.
        */
        if (bMovaSrc0Spill)
        {
            VIR_Operand_SetHwRegId(src0Opnd, VIR_INVALID_HWREG);
        }

        retErrCode = _VIR_RA_LS_RewriteColor_Src(pRA,
                                                 pMovaInst,
                                                 pMovaSrc0Opnd,
                                                 movInst,
                                                 src0Opnd);

        if (pRA->bReservedMovaReg)
        {
            shift = vscHTBL_CountItems(pRA->movaHash);
            /* we can only support max of 8 array spilled for now */
            gcmASSERT(shift < 4 * pRA->movaRegCount);

            retErrCode = _VIR_RA_LS_GenTemp(pRA, &tmpSymId);
            VIR_Operand_SetTempRegister(movInst->dest,
                                        pFunc,
                                        tmpSymId,
                                        VIR_TYPE_UINT32);
            /* use movaRegister.x */
            _VIR_RA_MakeColor(pRA->movaRegister[(shift/4)], (shift % 4), &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, movInst->dest, curColor);
        }
        else
        {
            VIR_Operand_SetTempRegister(movInst->dest,
                                        pFunc,
                                        pRA->spillOffsetSymId,
                                        VIR_TYPE_UINT32);
            gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
            /* use baseRegister's scratch channel */
            _VIR_RA_LS_SetHWRegForBaseRegister(pRA, movInst->dest, pRA->scratchChannel);
        }
        VIR_Operand_SetEnable(movInst->dest, VIR_ENABLE_X);
        /* set swizzle of src operand according to the usage */
        /* set swizzle of src operand according to the usage, case like
         * 030: MOVA               ivec4 addr_reg  temp(1503), uvec4 temp(1753).wzyx
         * 031: LDARR              char temp(36).x, char temp(970).x, ivec4 temp(1503).x
         *  ==>
         *      MOV          t1.x       uvec4 temp(1752).w
         *      MAD          offset.x, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__, spillOffset
         */
        if (!_VSC_RA_EnableSingleChannel(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(src0Opnd))))
        {
            gctSIZE_T channel = 0;
            VIR_Swizzle swizzle_channel;
            switch(enable_channel)
            {
                case VIR_ENABLE_X:    channel = 0; break;
                case VIR_ENABLE_Y:    channel = 1; break;
                case VIR_ENABLE_Z:    channel = 2; break;
                case VIR_ENABLE_W:    channel = 3; break;
                default:
                {
                    gcmASSERT(gcvFALSE);
                }
            };
            swizzle_channel = VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(src0Opnd), channel);
            VIR_Operand_SetSwizzle(src0Opnd, swizzle_channel);
        }

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "MOV instruction:\n");
            VIR_LOG_FLUSH(pDumper);
            VIR_Inst_Dump(pDumper, movInst);
        }

        if (pRA->bReservedMovaReg && !bMovaSrc0Spill)
        {
            vscHTBL_DirectSet(pRA->movaHash, _VSC_MOVA_NewConstKey(pMM, pDef->defKey.pDefInst, enable_channel), (void*)movInst);
        }
    }
    else
    {
        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "previous MOV instruction:\n");
            VIR_LOG_FLUSH(pDumper);
            VIR_Inst_Dump(pDumper, movInst);
        }

        tmpSymId = VIR_Operand_GetSymbolId_(VIR_Inst_GetDest(movInst));
        shift = VIR_Operand_GetHwShift(VIR_Inst_GetDest(movInst));
    }

    /* mad src0 - coming from extra MOV */
    if (pRA->bReservedMovaReg)
    {
        gcmASSERT(tmpSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(madInst->src[VIR_Operand_Src0],
                                    pFunc,
                                    tmpSymId,
                                    VIR_TYPE_UINT32);
        /* use movaRegister.x */
        _VIR_RA_MakeColor(pRA->movaRegister[(shift / 4)], (shift % 4), &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, madInst->src[VIR_Operand_Src0], curColor);
    }
    else
    {
        VIR_Operand_SetTempRegister(madInst->src[VIR_Operand_Src0],
                                    pFunc,
                                    pRA->spillOffsetSymId,
                                    VIR_TYPE_UINT32);
        gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
        /* use baseRegister's scratch channel */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, madInst->src[VIR_Operand_Src0], pRA->scratchChannel);
    }
    VIR_Operand_SetSwizzle(madInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__ (will change to based on type) */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src1], __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__);

    /* src2 - spillOffset */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src2],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* dest */
    VIR_Operand_SetTempRegister(madInst->dest,
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, madInst->dest, pRA->scratchChannel);
    VIR_Operand_SetEnable(madInst->dest, VIR_ENABLE_X);

    /* Delete the mova usage cause it it not used anymore. */
    vscVIR_DeleteUsage(pLvInfo->pDuInfo,
                       pMovaInst,
                       pInst,
                       idxOpnd,
                       gcvFALSE,
                       pDef->defKey.regNo,
                       1,
                       (VIR_Enable)(1 << pDef->defKey.channel),
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

    /* Remove mova if pInst is its only user or there is not user anymore. */
    if (vscVIR_IsUniqueUsageInstOfDefInst(pLvInfo->pDuInfo, pMovaInst, pInst, gcvNULL, gcvFALSE, gcvNULL, gcvNULL, gcvNULL))
    {
        VIR_Pass_DeleteInstruction(pFunc, pMovaInst, gcvNULL);
        if (pRA->bReservedMovaReg)
        {
             _VSC_RA_MOVA_RemoveConstValAllChannel(pRA->movaHash, pMovaInst);
        }
    }

    /* All data registers that used in MAD instruction are valid now. */
    retErrCode = _VIR_RA_LS_InvalidDataRegisterUsedMask(pRA, pInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "MAD instruction:\n");
        VIR_LOG_FLUSH(pDumper);
        VIR_Inst_Dump(pDumper, madInst);
    }

    return retErrCode;
}

/*
    1: mova t2.x, t1.x
    2: ldarr dst1.xy, src0, t2.x (src0 is spilled)
    ==>
    1: mad  baseRegister.y, spillOffset, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__
    2: load dataRegister.xy, baseRegister.x, baseRegister.y
    3: mov dst1.xy, dataRegister.xy
*/
static VSC_ErrCode
_VIR_RA_LS_InsertSpill_LDARR(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId;
    VIR_RA_HWReg_Color  curColor;
    VIR_Swizzle         newSwizzle = _VIR_RA_LS_SwizzleWShift(pOpnd);

    curColor = InvalidColor;

    /* mad instruction */
    retErrCode = _VIR_RA_LS_InsertSpillOffset(pRA, pInst, pOpnd, pLR, gcvTRUE);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* load instruction */
    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_LOAD_S,
        VIR_TYPE_UINT32,
        pInst,
        gcvTRUE,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src 0 - base */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.x */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* src1 offset */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src1], pRA->scratchChannel);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);

    /* set the dest for the newInst */
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_Operand_GetTypeId(pOpnd));
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* HW can't support FP16 directly, it converts FP16 to FP32, so we need to use UINT16. */
    VIR_Operand_SetTypeId(newInst->dest, VIR_TypeId_ConvertFP16Type(pShader, VIR_Operand_GetTypeId(pOpnd)));

    /* destination is always dataRegister[0] */
    _VIR_RA_MakeColor(pRA->dataRegister[0], 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_Swizzle_2_Enable(newSwizzle));

    /* change ldarr to mov */
    VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(pInst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(pInst, 1);

    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                symId,
                                VIR_Operand_GetTypeId(pOpnd));
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
    VIR_Operand_SetSwizzle(pOpnd, newSwizzle);
    VIR_Operand_SetRelAddrMode(pOpnd, VIR_INDEXED_NONE);
    VIR_Operand_SetRelIndexing(pOpnd, 0);

    _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pInst->dest);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "Load instruction:\n");
        VIR_LOG_FLUSH(pDumper);
        VIR_Inst_Dump(pDumper, newInst);

        VIR_LOG(pDumper, "Transformed instruction:\n");
        VIR_LOG_FLUSH(pDumper);
        VIR_Inst_Dump(pDumper, pInst);
    }

    return retErrCode;
}

/*
    1: mova t2.x, t1.x
    2: starr dest, t2.x, t3.x   (dest is spilled)
    ==>
    1: mad  offset.x, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__, spillOffset
    2: store spillBase.x, offset.x, t3.x
*/
static VSC_ErrCode
_VIR_RA_LS_InsertFill_STARR(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId;
    VIR_RA_HWReg_Color  curColor;
    VIR_Operand         *newOpnd = gcvNULL;
    VIR_OperandInfo     operandInfo;

    curColor = InvalidColor;

    /* mad instruction */
    retErrCode = _VIR_RA_LS_InsertSpillOffset(pRA, pInst, pOpnd, pLR, gcvFALSE);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* store instruction */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_STORE_S,
        VIR_Operand_GetTypeId(pOpnd),
        pInst,
        gcvTRUE,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - base */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);

    /* src1 - offset */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
     /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->src[VIR_Operand_Src1], pRA->scratchChannel);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);

    /* src2 - data */
    newOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src2);
    VIR_Operand_Copy(newOpnd, pInst->src[VIR_Operand_Src1]);
    _VIR_RA_LS_RewriteColor_Src(pRA,
                                pInst,
                                pInst->src[VIR_Operand_Src1],
                                newInst,
                                newOpnd);

    /* HW can't support FP16 directly, it converts FP16 to FP32, so we need to use UINT16. */
    VIR_Operand_SetTypeId(newOpnd, VIR_TypeId_ConvertFP16Type(pShader, VIR_Operand_GetTypeId(newOpnd)));

    VIR_Operand_GetOperandInfo(newInst, newOpnd, &operandInfo);

    /*
    ** Set the dest for the newInst:
    ** We can't guarantee that the src2 is alway in temp, if not, we need to use baseRegister.y for dest.
    */
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    VIR_Operand_SetEnable(newInst->dest, _VIR_RA_LS_EnableWShift(pOpnd));

    if (operandInfo.isVreg)
    {
        _VIR_RA_MakeColor(0, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    }
    else
    {
         /* use baseRegister's scratch channel */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, newInst->dest, pRA->scratchChannel);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "Store instruction:\n");
        VIR_LOG_FLUSH(pDumper);
        VIR_Inst_Dump(pDumper, newInst);
    }

    /* remove STARR instruction */
    VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);

    return retErrCode;
}

static gctUINT
_VIR_RA_LS_GetSpillSize(
    VIR_RA_LS           *pRA,
    gctBOOL             bUse16BitMod,
    gctUINT             *pGroupSize)
{
    /*
    spill size = sizeof(spill for each thread) * number of threads + 4
    the memory layout:
    atomic counter for index (4 bytes)
    spill for thread 0       (sizeof spill for each thread)
    spill for thread 1       (sizeof spill for each thread)
    ...
    spill for thread n       (sizeof spill for each thread)
    */

    VIR_Shader      *pShader = VIR_RA_LS_GetShader(pRA);
    VSC_HW_CONFIG   *hwConfig = VIR_RA_LS_GetHwCfg(pRA);

    gctUINT     spillSize = 4, threadCount, groupCount, groupSize;
    gctUINT     maxGPRPerWorkgroup = hwConfig->maxGPRCountPerCore / 4;
    gctFLOAT    index = 0.0;

    threadCount = hwConfig->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
    groupCount =  maxGPRPerWorkgroup / VIR_Shader_GetRegWatermark(pShader);
    groupSize = groupCount * threadCount * hwConfig->maxClusterCount;

    /*
    ** Since we use the globalId as the index to MOD the groupSize, if we still use 16bit MOD,
    ** we need to make sure that the groupSize can be divided by 0x10000.
    ** So there is no overlapping problem when the globalId is greater than 0xFFFF.
    */
    if (bUse16BitMod)
    {
        while ((groupSize > (gctUINT)gcoMATH_Exp2(index)) && ((gctUINT)gcoMATH_Exp2(index) < 0x10000))
        {
            index++;
        }
        groupSize = (gctUINT)gcoMATH_Exp2(index);
        gcmASSERT(groupSize < 0x10000);
    }

    if (!bUse16BitMod || groupSize * 2 < 0x10000)
    {
        groupSize *= 2;
    }

    spillSize += (pRA->spillOffset) * groupSize;

    if (pGroupSize)
    {
        *pGroupSize = groupSize;
    }

    return spillSize;
}

/* The Spill Memory Block Layout
 *    +-----------------------------+  <--- #TempRegSpillMemAddr
 *    |  spill-id-counter (32-bit)  |
 *    +-----------------------------+  <---- start of spill memory area
 *    |/////// 1st id //////////////|    A
 *    |///// spill area ////////////|    |
 *    |\\\\\\\ 2nd id \\\\\\\\\\\\\\|    |   total spill memory size =
 *    |\\\\\ spill area \\\\\\\\\\\\|    |     perThreadSpillMemSz * groupSize
 *    |                             |    |
 *    |                             |    |   thread_i_spill_area =
 *    |                             |    |     (#TempRegSpillMemAddr + 4) +
 *    |\\\\\\ last id \\\\\\\\\\\\\\|    |     thread_spill_id * perThreadSpillMemSz
 *    |\\\\\ spill area \\\\\\\\\\\\|    V
 *    +-----------------------------+  <---- end of spill memory area
 *
 * o In non Robust OutOfBounds Check mode, the #TempRegSpillMemAddr is a uniform
 *   contains 32-bit address the Spill Memory Block;
 * o In Robust OutOfBounds Check mode, the #TempRegSpillMemAddr is a uniform
 *   contains the Spill Memory Block bounds info:
 *        .x:    start address of the Spill Memory Block
 *        .y:    start address of the Spill Memory Block (same as .x)
 *        .z:    end address of the Spill Memory Block
 *
 *  When address is assigned to some pointer variable, the .x value can be changed
 *  but .y and .z will never be changed so the HW can do effctive bounds check
 */
static VSC_ErrCode
_VIR_RA_LS_SpillAddrComputation(
    VIR_RA_LS           *pRA
    )
{
    VSC_ErrCode         retErrCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    VIR_Instruction     *atomicAddInst = gcvNULL, *madInst = gcvNULL;
    VIR_Instruction     *addInst = gcvNULL, *modInst = gcvNULL;
    VIR_Instruction     *movInst = gcvNULL;

    VIR_Symbol*         uSymSpillMemAddr = gcvNULL;
    gctUINT             groupSize = 0;
    gctBOOL             use16BitMod = gcvTRUE;

    gctUINT             spillSize = _VIR_RA_LS_GetSpillSize(pRA, use16BitMod, &groupSize);
    gctUINT             uniformIdx;

    pShader->vidmemSizeOfSpill = spillSize;

    for (uniformIdx = 0; uniformIdx < VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)); uniformIdx ++)
    {
        uSymSpillMemAddr = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(VIR_Shader_GetUniforms(pShader), uniformIdx));
        if (VIR_Symbol_GetUniformKind(uSymSpillMemAddr) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS)
        {
            break;
        }
    }

    gcmASSERT(uniformIdx < VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)));

    /*  using atomic counter to get index
        atomica_dd index, baseAddr, 0, 1 */
    retErrCode = VIR_Function_PrependInstruction(pFunc,
                    VIR_OP_ATOMADD_S,
                    VIR_TYPE_UINT32,
                    &atomicAddInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - base */
    VIR_Operand_SetOpKind(atomicAddInst->src[VIR_Operand_Src0], VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(atomicAddInst->src[VIR_Operand_Src0], uSymSpillMemAddr);
    VIR_Operand_SetSwizzle(atomicAddInst->src[VIR_Operand_Src0], pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_XXXX);

    /* src1 - 0 */
    VIR_Operand_SetImmediateUint(atomicAddInst->src[VIR_Operand_Src1], 0);

    /* src2 - 1 */
    VIR_Operand_SetImmediateUint(atomicAddInst->src[VIR_Operand_Src2], 1);

    /* dest */
    if (pRA->threadIdxSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->threadIdxSymId);
    }
    VIR_Operand_SetTempRegister(atomicAddInst->dest,
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, atomicAddInst->dest, pRA->scratchChannel);
    VIR_Operand_SetEnable(atomicAddInst->dest, VIR_ENABLE_X);

    /*
    ** For a CS, swathing is disabled now, so we need to reset the index to 0 for a new swath. */
    /* index = mod(index, groupSize) */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                  VIR_OP_IMOD,
                                                  use16BitMod ? VIR_TYPE_UINT16 : VIR_TYPE_UINT32,
                                                  movInst ? movInst : atomicAddInst,
                                                  gcvTRUE,
                                                  &modInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(modInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT16);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, modInst->src[VIR_Operand_Src0], pRA->scratchChannel);
    VIR_Operand_SetSwizzle(modInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - based on group size */
    VIR_Operand_SetImmediateUint(modInst->src[VIR_Operand_Src1], groupSize);

    /* dest */
    VIR_Operand_SetTempRegister(modInst->dest,
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT16);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, modInst->dest, pRA->scratchChannel);
    VIR_Operand_SetEnable(modInst->dest, VIR_ENABLE_X);

    /* base = (baseAddress + 4 ) + index * sizeof(spill for each thread)
       ==> mad base, i, sizeof(spill), baseAddress
           add base, base, 4 */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                  VIR_OP_MAD,
                                                  VIR_TYPE_UINT32,
                                                  modInst,
                                                  gcvTRUE,
                                                  &madInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(madInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT32);
    /* use baseRegister's scratch channel */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, madInst->src[VIR_Operand_Src0], pRA->scratchChannel);
    VIR_Operand_SetSwizzle(madInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - sizeof spill */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src1], pRA->spillOffset);

    /* src2 - physical address*/
    VIR_Operand_SetOpKind(madInst->src[VIR_Operand_Src2], VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(madInst->src[VIR_Operand_Src2], uSymSpillMemAddr);
    VIR_Operand_SetSwizzle(madInst->src[VIR_Operand_Src2], VIR_SWIZZLE_XXXX);

    /* dest */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    VIR_Operand_SetTempRegister(madInst->dest,
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.x */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, madInst->dest, 0);
    VIR_Operand_SetEnable(madInst->dest, VIR_ENABLE_X);

    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                    VIR_OP_ADD,
                    VIR_TYPE_UINT32,
                    madInst,
                    gcvTRUE,
                    &addInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(addInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_UINT32);
     /* use baseRegister.x */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, addInst->src[VIR_Operand_Src0], 0);
    VIR_Operand_SetSwizzle(addInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - 4 */
    VIR_Operand_SetImmediateUint(addInst->src[VIR_Operand_Src1], 4);

    /* dest */
    VIR_Operand_SetTempRegister(addInst->dest,
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.x */
    _VIR_RA_LS_SetHWRegForBaseRegister(pRA, addInst->dest, 0);
    VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_X);

    if (pRA->needBoundsCheck)
    {
        VIR_Instruction*    pAddInst = gcvNULL;
        VIR_Operand*        pOpnd = gcvNULL;

        /* MOV  baseRegister.y, baseRegister.x */
        retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                      VIR_OP_MOV,
                                                      VIR_TYPE_UINT32,
                                                      addInst,
                                                      gcvTRUE,
                                                      &movInst);
        if (retErrCode != VSC_ERR_NONE) return retErrCode;

        /* dest */
        pOpnd = VIR_Inst_GetDest(movInst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    pRA->baseAddrSymId,
                                    VIR_TYPE_UINT32);
        gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
        /* use baseRegister.y */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, pOpnd, 0);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_Y);

        /* src0 */
        pOpnd = VIR_Inst_GetSource(movInst, 0);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    pRA->baseAddrSymId,
                                    VIR_TYPE_UINT32);
        gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
        /* use baseRegister.x */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, pOpnd, 0);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

        /* ADD  baseRegister.z, baseRegister.x, sizeof(spill) */
        retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                      VIR_OP_ADD,
                                                      VIR_TYPE_UINT32,
                                                      movInst,
                                                      gcvTRUE,
                                                      &pAddInst);
        if (retErrCode != VSC_ERR_NONE) return retErrCode;

        /* dest */
        pOpnd = VIR_Inst_GetDest(pAddInst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    pRA->baseAddrSymId,
                                    VIR_TYPE_UINT32);
        gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
        /* use baseRegister.z */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, pOpnd, 0);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_Z);

        /* src0 */
        pOpnd = VIR_Inst_GetSource(pAddInst, 0);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    pRA->baseAddrSymId,
                                    VIR_TYPE_UINT32);
        gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
        /* use baseRegister.x */
        _VIR_RA_LS_SetHWRegForBaseRegister(pRA, pOpnd, 0);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

        /* src1 */
        pOpnd = VIR_Inst_GetSource(pAddInst, 1);
        VIR_Operand_SetImmediateUint(pOpnd, pRA->spillOffset);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "Base address computation instruction:\n");
        VIR_Inst_Dump(pDumper, atomicAddInst);
        VIR_Inst_Dump(pDumper, modInst);
        VIR_Inst_Dump(pDumper, madInst);
        VIR_Inst_Dump(pDumper, addInst);
        VIR_LOG_FLUSH(pDumper);
    }

    return retErrCode;
}

/* use pSrcOpnd's color to write color for pOpnd */
VSC_ErrCode
_VIR_RA_LS_RewriteColor_Src(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pSrcInst,
    VIR_Operand     *pSrcOpnd,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd
    )
{
    VSC_ErrCode             retErrCode = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_ColorPool        *pCP = VIR_RA_LS_GetColorPool(pRA);
    VIR_OperandInfo         operandInfo;
    VIR_RA_LS_Liverange     *pLR, *pOrigLR;
    gctUINT                 webIdx, diffReg = 0;
    VIR_RA_HWReg_Color      curColor;
    VIR_Symbol              *pSym = gcvNULL;

    if (VIR_Operand_GetHwRegId(pOpnd) != VIR_INVALID_HWREG)
    {
        return retErrCode;
    }

    curColor = InvalidColor;

    VIR_Operand_GetOperandInfo(pSrcInst,
                               pSrcOpnd,
                               &operandInfo);
    webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pSrcInst, pSrcOpnd);
    if (VIR_INVALID_WEB_INDEX != webIdx)
    {
        pOrigLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
        pLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
        if (!isLRSpilled(pLR))
        {
            gcmASSERT(!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)));
            pSym = VIR_Operand_GetSymbol(pOpnd);
            _VIR_RA_LS_SetSymbolHwRegInfo(pRA, pSym, pLR, diffReg);

            /* get the array element color */
            if (pLR->hwType == VIR_RA_HWREG_GR)
            {
                if (isLRVXEven(pLR))
                {
                    gcmASSERT((_VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) & 0x1) == 0);
                }

                if (isLRVXOdd(pOrigLR))
                {
                    diffReg = 1;
                }
                else
                {
                    diffReg = operandInfo.u1.virRegInfo.virReg - pLR->firstRegNo;
                }
                _VIR_RA_MakeColor(_VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg,
                                  _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)),
                                  &curColor);
                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Operand_GetPrecision(pOpnd) == VIR_PRECISION_HIGH)
                {
                    gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                    _VIR_RA_MakeColorHI(_VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg,
                                        _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)),
                                        &curColor);
                }
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            }
            else if (isLRA0(pLR))
            {
                gcmASSERT(VIR_Operand_GetPrecision(pOpnd) != VIR_PRECISION_HIGH);
                _VIR_RA_MakeColor(VIR_SR_A0,
                                  _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)),
                                  &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            }
            else if (isLRB0(pLR))
            {
                gcmASSERT(VIR_Operand_GetPrecision(pOpnd) != VIR_PRECISION_HIGH);
                _VIR_RA_MakeColor(VIR_SR_B0,
                                  _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)),
                                  &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            /* spill case - insert a load */
            gcmASSERT(isLRSpilled(pLR));
            /* ldarr case is handled in other place */
            gcmASSERT(!(VIR_Inst_GetOpcode(pSrcInst) == VIR_OP_LDARR &&
                        VIR_Inst_GetSourceIndex(pSrcInst, pSrcOpnd) == 0));

            if (_VIR_RA_GetLRTempColorEndPoint(pLR) != VIR_INVALID_ID &&
                (gctUINT)VIR_Inst_GetId(pInst) <= _VIR_RA_GetLRTempColorEndPoint(pLR))
            {
                if (pLR->hwType == VIR_RA_HWREG_GR)
                {
                    _VIR_RA_MakeColor(_VIR_RA_Color_RegNo(_VIR_RA_GetLRTempColor(pLR)) + diffReg,
                                      _VIR_RA_Color_Shift(_VIR_RA_GetLRTempColor(pLR)),
                                      &curColor);
                    if (VIR_Shader_isDual16Mode(pShader) &&
                        VIR_Operand_GetPrecision(pOpnd) == VIR_PRECISION_HIGH)
                    {
                        gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRTempColor(pLR)));
                        _VIR_RA_MakeColorHI(_VIR_RA_Color_HIRegNo(_VIR_RA_GetLRTempColor(pLR)) + diffReg,
                                            _VIR_RA_Color_HIShift(_VIR_RA_GetLRTempColor(pLR)),
                                            &curColor);
                    }
                    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }
            else
            {
                _VIR_RA_LS_InsertSpill(pRA, pInst, pOpnd, pLR);
            }
        }
    }

    if (VIR_Operand_GetHwRegId(pOpnd) == VIR_REG_SAMPLE_MASK_IN ||
        VIR_Operand_GetHwRegId(pOpnd) == VIR_REG_SAMPLE_ID)
    {
        gctUINT swizzle = pShader->sampleMaskIdChannelStart;
        VIR_Operand_SetSwizzle(pOpnd, (VIR_Swizzle) (swizzle | (swizzle << 2) | (swizzle << 4) | (swizzle << 6)));
    }
    else if (VIR_Operand_GetHwRegId(pOpnd) == VIR_REG_SAMPLE_POS)
    {
        if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasSamplePosSwizzleFix &&
            (VIR_Operand_GetSwizzle(pOpnd) != VIR_SWIZZLE_XYZW))
        {
            VIR_Function    *pFunc = VIR_Shader_GetCurrentFunction(pShader);
            VIR_Instruction *newInst = gcvNULL;
            VIR_SymId       symId;
            VIR_Operand     *pNewOpnd = gcvNULL;

            VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_MOV,
                VIR_Operand_GetTypeId(pOpnd),
                pSrcInst,
                gcvTRUE,
                &newInst);

            pNewOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_Copy(pNewOpnd, pOpnd);
            gcmASSERT(_VIR_RA_Color_RegNo(curColor) == VIR_REG_SAMPLE_POS);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, pNewOpnd, curColor);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZW);

            _VIR_RA_LS_GenTemp(pRA, &symId);
            VIR_Operand_SetTempRegister(newInst->dest,
                                        pFunc,
                                        symId,
                                        VIR_Operand_GetTypeId(pOpnd));
            if (pRA->samplePosRegister != VIR_INVALID_ID)
            {
                _VIR_RA_MakeColor(pRA->samplePosRegister, 0, &curColor);
            }
            else
            {
                /* find a brand new color for the temp */
                if (_VIR_RA_LS_FindBrandnewColor(pRA, gcvNULL, &curColor, 0))
                {
                    /* update the maxAllocReg*/
                    pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg ++;
                    pRA->samplePosRegister = _VIR_RA_Color_RegNo(curColor);
                }
                else
                {
                    /* could not find a temp register, need to spill*/
                    retErrCode = VSC_RA_ERR_OUT_OF_REG_SPILL;
                    return retErrCode;
                }
            }
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
            VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_XYZW);

            /* change the pInst */
            VIR_Operand_SetTempRegister(pOpnd,
                                        pFunc,
                                        symId,
                                        VIR_Operand_GetTypeId(pOpnd));
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            VIR_Inst_SetThreadMode(newInst, VIR_THREAD_D16_DUAL_32);
        }
    }
    else if (VIR_Operand_isSymbol(pSrcOpnd))
    {
        /* mark color of load_s/store_s which is not generated in RA */
        VIR_Symbol *sym = VIR_Operand_GetSymbol(pSrcOpnd);
        if (VIR_Symbol_GetIndex(sym) == pRA->baseAddrSymId)
        {
            _VIR_RA_LS_SetHWRegForBaseRegister(pRA, pSrcOpnd, 0);
            VIR_Operand_SetSwizzle(pSrcOpnd, pRA->needBoundsCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_X);
        }
    }

    return retErrCode;
}

/* use pInst->dest color to write color for pOpnd */
VSC_ErrCode
_VIR_RA_LS_RewriteColor_Dest(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd
    )
{
    VSC_ErrCode             retErrCode = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_OperandInfo         operandInfo;
    VIR_RA_LS_Liverange     *pLR;
    gctUINT                 defIdx, diffReg = 0;
    VIR_RA_HWReg_Color      curColor;
    VIR_Symbol              *pSym = gcvNULL;

    if (VIR_Operand_GetHwRegId(pOpnd) != VIR_INVALID_HWREG)
    {
        return retErrCode;
    }

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetDest(pInst));

    if (!_VIR_RA_LS_IsInstExcludedLR(pRA, pInst))
    {
        VIR_Operand_GetOperandInfo(pInst,
                                   VIR_Inst_GetDest(pInst),
                                   &operandInfo);

        defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
        if (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pLR =  _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
            if (!isLRSpilled(pLR))
            {
                gcmASSERT (!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)));

                /* A0 should not be written in instr other than MOVA */
                gcmASSERT(pLR->hwType == VIR_RA_HWREG_GR);
                /* get the array element color */
                diffReg = operandInfo.u1.virRegInfo.virReg - pLR->firstRegNo;
                gcmASSERT(diffReg < pLR->regNoRange);
                pSym = VIR_Operand_GetSymbol(pOpnd);
                _VIR_RA_LS_SetSymbolHwRegInfo(pRA, pSym, pLR, diffReg);
                _VIR_RA_MakeColor(_VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + diffReg,
                                  _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)),
                                  &curColor);
                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Operand_GetPrecision(pOpnd) == VIR_PRECISION_HIGH)
                {
                    gcmASSERT(!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)));
                    _VIR_RA_MakeColorHI(_VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + diffReg,
                                        _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)),
                                        &curColor);
                }
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            }
            else
            {
                /* spill case - insert a store */
                gcmASSERT(isLRSpilled(pLR));
                /* the starr case is handled in other place */
                gcmASSERT(VIR_Inst_GetOpcode(pInst) != VIR_OP_STARR);
                _VIR_RA_LS_InsertFill(pRA, pInst, pOpnd, pLR);
            }
        }
    }

    return retErrCode;
}

VSC_ErrCode _VIR_RS_LS_InsertSelectMap(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctINT          remapIndex,
    gctINT          samplerSwizzler,
    VIR_SymId       dstSymId,
    gctINT          destReg,
    gctBOOL         extraMOV,
    VIR_SymId       movDstSymId)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId = VIR_INVALID_ID;
    VIR_RA_HWReg_Color      curColor;

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_SELECT_MAP,
            VIR_TYPE_UINT16,
            pInst,
            gcvTRUE,
            &newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* set src0 - index value */
    if (extraMOV)
    {
        /* set src0 - index value, coming from the dest for an mov,
           which is added to resolve select_map src0 has no swizzle issue */
        gcmASSERT(movDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                    pFunc,
                                    movDstSymId,
                                    VIR_Operand_GetTypeId(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(destReg, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_YYYY);
    }
    else
    {
        /* set src0 - index value, coming from the attr_ld src1 (non constant) */
        gcmASSERT(VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src1]) != VIR_OPND_IMMEDIATE);
        VIR_Operand_Copy(newInst->src[VIR_Operand_Src0], pInst->src[VIR_Operand_Src1]);
        _VIR_RA_LS_RewriteColor_Src(pRA,
                                    pInst,
                                    pInst->src[VIR_Operand_Src1],
                                    newInst,
                                    newInst->src[VIR_Operand_Src0]);
    }

    /* set src1 */
    retValue = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    /* newInst->src[VIR_Operand_Src1] is a new temp, only set opnd's hw reg info,
       no need to set symbol's hw reg info */
    _VIR_RA_MakeColor(remapIndex, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_XYZW);

    /* set src2 */
    retValue = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(remapIndex + 1, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src2], VIR_SWIZZLE_XYZW);

    /* set the samplerSwizzler
        bit7 enable the component selection
        bit[3:0] range to compare */
    VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src3], samplerSwizzler);

    /* set destination and assigned to the reserved one.
       select_map instruction type is coming from dest,
       which should be VIR_TYPE_UINT16 */
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                dstSymId,
                                VIR_TYPE_UINT16);
    _VIR_RA_MakeColor(destReg, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_X);

    return retValue;
}

VSC_ErrCode _VIR_RA_LS_InsertLoadAttr(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_SymId       regmapSymId,
    VIR_Swizzle     regmapSwizzle,
    gctINT          regmapReg,
    gctBOOL         loadFromOutput,
    gctUINT8        regmapChannel,
    gctINT          regmapIndex,
    VIR_Operand     *src1Opnd,
    gctINT          attrIndex,
    VIR_Instruction **newInst
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_RA_HWReg_Color  curColor;
    VIR_Operand         *newSrc = gcvNULL;
    VIR_Operand         *src2Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src2);

    curColor = InvalidColor;

    /* load_attr dest, src0 (regmapBase), src1 (regmapIndex), src2 (attrIndex) */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                loadFromOutput ? VIR_OP_LOAD_ATTR_O : VIR_OP_LOAD_ATTR,
                VIR_TYPE_UINT16,
                pInst,
                gcvTRUE,
                newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* src 0 regmap*/
    newSrc = VIR_Inst_GetSource(*newInst, VIR_Operand_Src0);
    VIR_Operand_SetTempRegister(newSrc,
                                pFunc,
                                regmapSymId,
                                VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetSwizzle(newSrc, regmapSwizzle);
    _VIR_RA_MakeColor(regmapReg, regmapChannel, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newSrc, curColor);

    /* src 1 regmap index */
    newSrc = VIR_Inst_GetSource(*newInst, VIR_Operand_Src1);
    if (regmapIndex != -1)
    {
        VIR_Operand_SetImmediateInt(newSrc, regmapIndex);
    }
    else
    {
        gcmASSERT(src1Opnd);
        VIR_Operand_Copy(newSrc, src1Opnd);
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd, *newInst, newSrc);
    }

    /* src 2 attribute index
       it could be possibly changed to temp by the client afterward */
    newSrc = VIR_Inst_GetSource(*newInst, VIR_Operand_Src2);
    if (VIR_Operand_GetOpKind(src2Opnd) == VIR_OPND_IMMEDIATE)
    {
        VIR_Operand_SetImmediateInt(newSrc, attrIndex);
    }
    else
    {
        VIR_Operand_Copy(newSrc, src2Opnd);
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src2Opnd, *newInst, newSrc);
    }

    /* only setup the src, leave the destination to the client */

    return retValue;
}

static gctBOOL
_VIR_RA_LS_NotNeedOOB(
    VIR_Shader      *pShader,
    VIR_Instruction *pInst,
    gctBOOL         inputVertex,
    gctINT         inputControlCount,
    gctINT         outputControlCount)
{
    gctBOOL         retValue = gcvFALSE;
    VIR_Operand     *src1Opnd = VIR_Inst_GetSource(pInst, 1);
    VIR_Symbol      *src1Sym = VIR_Operand_GetSymbol(src1Opnd);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        if (inputVertex)
        {
            /* this access is gl_InvocationID + offset:
               if offset==0 && inputControlCount >= outputControlCount,
               no need for OOB check */
            if (inputControlCount >= outputControlCount &&
                src1Sym &&
                VIR_Symbol_GetName(src1Sym) == VIR_NAME_INVOCATION_ID)
            {
                retValue = gcvTRUE;
            }
        }
        else
        {
            /* this access is gl_InvocationID + offset:
               if offset == 0, no need for OOB check */
            if (src1Sym &&
                VIR_Symbol_GetName(src1Sym) == VIR_NAME_INVOCATION_ID)
            {
                retValue = gcvTRUE;
            }
        }
    }
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        retValue = gcvFALSE;
    }
    else if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        retValue = gcvFALSE;
    }
    return retValue;
}

VSC_ErrCode _VIR_RA_LS_InsertMOD(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctINT          vertexCount,
    VIR_Instruction **newInst,
    VIR_SymId       *modDest
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Operand         *srcOpnd = gcvNULL;
    VIR_Operand         *newOpnd = gcvNULL;
    VIR_RA_HWReg_Color  curColor;

    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LOAD_ATTR ||
        VIR_Inst_GetOpcode(pInst) == VIR_OP_LOAD_ATTR_O)
    {
        srcOpnd = pInst->src[VIR_Operand_Src1];
    }
    else
    {
        srcOpnd = pInst->src[VIR_Operand_Src0];
    }

    curColor = InvalidColor;

    /* MOD index, index, vertexCount */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_IMOD,
                VIR_TYPE_INT16,
                pInst,
                gcvTRUE,
                newInst);

    if (pInst->_parentUseBB)
    {
        VIR_Inst_SetBasicBlock(*newInst, VIR_Inst_GetBasicBlock(pInst));
    }
    else
    {
        VIR_Inst_SetFunction(*newInst, VIR_Inst_GetFunction(pInst));
    }

    /* src 0 index*/
    newOpnd = VIR_Inst_GetSource((*newInst), VIR_Operand_Src0);
    VIR_Operand_Copy(newOpnd, srcOpnd);
    if (VIR_Operand_GetOpKind(newOpnd) == VIR_OPND_SYMBOL)
    {
        VIR_Symbol * srcSym = VIR_Operand_GetSymbol(newOpnd);

        if (!VIR_Symbol_isUniform(srcSym))
        {
            _VIR_RA_MakeColor(
                VIR_Operand_GetHwRegId(srcOpnd),
                VIR_Operand_GetHwShift(srcOpnd),
                &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->src[VIR_Operand_Src0], curColor);
        }
    }

    VIR_Operand_SetImmediateInt((*newInst)->src[VIR_Operand_Src1], vertexCount);

    /* destination */
    _VIR_RA_LS_GenTemp(pRA, modDest);
     VIR_Operand_SetTempRegister((*newInst)->dest,
                                pFunc,
                                *modDest,
                                VIR_TYPE_INT16
                                );
     /* use reserved register.z */
    _VIR_RA_MakeColor(pRA->resRegister, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->dest, curColor);
    VIR_Operand_SetEnable((*newInst)->dest, VIR_ENABLE_X);

    return retValue;
}

void _VIR_RA_LS_GenLoadAttr_SetEnable(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Instruction *newInst,
    VIR_Enable      ldEnable)
{
    VIR_Shader      *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Function    *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Operand     *destOpnd = VIR_Inst_GetDest(pInst);
    VIR_Operand     *src0Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
    VIR_Operand     *newDest = VIR_Inst_GetDest(newInst);
    VIR_RA_HWReg_Color  curColor;

    curColor = InvalidColor;

     /* set dest enable, need to consider the src0's swizzle
        ATTR_LD    temp.x, in_te_attr.hp.z, 1, 0
        ==>
        LOAD_ATTR temp.z, remap, remapIdx, attrIdx, 0
        ...
        ADD temp2.x, temp.z, 3

        if temp.x is output, since we could not change output swizzle,
        we have to add an extra mov
        ATTR_LD    temp.x, in_te_attr.hp.z, 1, 0
        ==>
        LOAD_ATTR temp1.z, remap, remapIdx, attrIdx, 0
        MOV       temp.x, temp1.z

        Or if there is another def of temp.z, we have to generate an extra mov
    */
    if (VIR_Operand_GetEnable(destOpnd) != ldEnable)
    {
        VIR_OperandInfo         instDstInfo;
        gctUINT8                 channel;
        gctBOOL                 needExtraMov = gcvTRUE;
        gctUINT                 defIdx = VIR_INVALID_DEF_INDEX;

        VIR_Operand_GetOperandInfo(pInst, destOpnd, &instDstInfo);
        for(channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
        {
            if (ldEnable & (1 << channel))
            {
                defIdx = vscVIR_FindFirstDefIndexWithChannel(pLvInfo->pDuInfo,
                                                             instDstInfo.u1.virRegInfo.virReg,
                                                             channel);
                if (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    needExtraMov = gcvTRUE;
                    break;
                }
            }
        }

        if (instDstInfo.isOutput || needExtraMov)
        {
            VIR_Swizzle useSwizzle;
            VIR_SymId   symId = VIR_INVALID_ID;

            _VIR_RA_LS_GenTemp(pRA, &symId);
            VIR_Operand_SetTempRegister(newDest,
                                        pFunc,
                                        symId,
                                        VIR_Operand_GetTypeId(destOpnd));
            _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, newDest, curColor);
            VIR_Operand_SetEnable(newDest, ldEnable);

            /* change the attr_ld to mov */
            VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
            VIR_Inst_SetSrcNum(pInst, 1);
            VIR_Operand_SetTempRegister(src0Opnd,
                                        pFunc,
                                        symId,
                                        VIR_Operand_GetTypeId(destOpnd));
            _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, src0Opnd, curColor);
            useSwizzle = VIR_Enable_2_Swizzle(ldEnable);
            useSwizzle = VIR_Swizzle_SwizzleWShiftEnable(useSwizzle,
                VIR_Operand_GetEnable(destOpnd));
            VIR_Operand_SetSwizzle(src0Opnd, useSwizzle);
            VIR_Operand_SetMatrixConstIndex(src0Opnd, 0);
            _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, destOpnd);
        }
        else
        {
            /* change the all uses's swizzle */
            VIR_GENERAL_DU_ITERATOR  instDUIter;
            VIR_USAGE                *instUsage         = gcvNULL;

            gctUINT8                channel1;
            VIR_Swizzle             newSwizzle = VIR_Enable_2_Swizzle(ldEnable);

            VIR_Operand_Copy(newDest, destOpnd);
            _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, newDest);
            VIR_Operand_SetEnable(newDest, ldEnable);

            for(channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
            {
                if(VIR_Operand_GetEnable(destOpnd) & (1 << channel))
                {
                    vscVIR_InitGeneralDuIterator(
                        &instDUIter,
                        pRA->pLvInfo->pDuInfo,
                        pInst,
                        instDstInfo.u1.virRegInfo.virReg,
                        channel,
                        gcvFALSE);

                    for(instUsage = vscVIR_GeneralDuIterator_First(&instDUIter);
                        instUsage != gcvNULL;
                        instUsage = vscVIR_GeneralDuIterator_Next(&instDUIter))
                    {
                        VIR_Operand     *useOpnd = instUsage->usageKey.pOperand;
                        VIR_Swizzle     useSwizzle = VIR_Operand_GetSwizzle(useOpnd);

                        gcmASSERT(!VIR_IS_IMPLICIT_USAGE_INST(instUsage->usageKey.pUsageInst));

                        for(channel1 = 0; channel1 < VIR_CHANNEL_COUNT; ++channel1)
                        {
                            if (VIR_Swizzle_GetChannel(useSwizzle, channel1) == channel)
                            {
                                VIR_Swizzle_SetChannel(useSwizzle, channel1,
                                    VIR_Swizzle_GetChannel(newSwizzle, channel));
                            }
                        }

                        VIR_Operand_SetSwizzle(useOpnd, useSwizzle);
                    }
                }
            }

            /* remove ldarr */
            VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
        }
    }
    else
    {
        /* set dest enable, need to consider the pOpnd's swizzle
           ATTR_LD    temp.x, in_te_attr.hp.x, 1, 0
           ==>
           LOAD_ATTR temp.x, remap, remapIdx, attrIdx, 0
        */
        VIR_Operand_Copy(newDest, destOpnd);
        _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, newDest);

        /* remove ldarr */
        VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
    }
}

/* generate load_attr for per-vertex data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Vertex(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_Instruction     *newInst = gcvNULL, *modInst = gcvNULL, *addSrc1Inst = gcvNULL;
    VIR_SymId           dstSymId = VIR_INVALID_ID;
    VIR_SymId           addDstSymId = VIR_INVALID_ID, movDstSymId = VIR_INVALID_ID;

    gctINT              vertexCount = 0, outputCount = 0;
    gctBOOL             inputVertex = gcvTRUE;

    gctBOOL             firstSelectMap = gcvFALSE;
    gctBOOL             secondSelectMap = gcvFALSE;
    gctINT              outputRemapStart = 0;
    gctINT              destReg = pRA->resRegister;
    gctINT              attrIndex = 0;
    gctINT              regmapIndex = -1;

    VIR_Enable          ldEnable = VIR_ENABLE_NONE;
    VIR_Swizzle         opndSwizzle = VIR_SWIZZLE_INVALID;

    VIR_Operand         *src0Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
    VIR_Symbol          *src0Sym = VIR_Operand_GetUnderlyingSymbol(src0Opnd);
    VIR_Operand         *src1Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src1);
    VIR_Operand         *src2Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src2);
    VIR_Operand         *destOpnd = VIR_Inst_GetDest(pInst);

    gctBOOL             needUpdateSrc1 = gcvFALSE;
    gctBOOL             needUpdateSrc2 = gcvFALSE;
    gctBOOL             extraMOV    = gcvFALSE;
    VIR_RA_HWReg_Color  curColor;

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR for attribute */
    VIR_Symbol_SetFlag(src0Sym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        vertexCount = pShader->shaderLayout.tcs.tcsPatchInputVertices;
        outputCount = pShader->shaderLayout.tcs.tcsPatchOutputVertices;
    }
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        vertexCount = pShader->shaderLayout.tes.tessPatchInputVertices;
        outputCount = vertexCount;
    }
    else if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
         /* points 1 vertex, lines 2 vertices, line adjacency 4 vertices,
            triangles 3 vertices, triangle adjacency 6 vertices
            set the exact number to do the OOB check */
        switch (pShader->shaderLayout.geo.geoInPrimitive)
        {
        case VIR_GEO_POINTS:
            vertexCount = 1;
            break;

        case VIR_GEO_LINES:
            vertexCount = 2;
            break;

        case VIR_GEO_LINES_ADJACENCY:
            vertexCount = 4;
            break;

        case VIR_GEO_TRIANGLES:
            vertexCount = 3;
            break;

        case VIR_GEO_TRIANGLES_ADJACENCY:
            vertexCount = 6;
            break;

        default:
            gcmASSERT(gcvFALSE);
            vertexCount = 6;
            break;
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* the remapAddress is different for input vertex or output vertex */
    if (VIR_Symbol_isOutput(src0Sym))
    {
        inputVertex = gcvFALSE;

        /* only TCS have output regmap */
        gcmASSERT(pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL);
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL &&
        VIR_Shader_TCS_UsePackedRemap(pShader))
    {
        /* input vertex regmap and output vertex regmap are packed in r1 */
        firstSelectMap = gcvFALSE;
        secondSelectMap = gcvFALSE;
        outputRemapStart = 1;
        destReg = 1;
    }
    else
    {
        switch ((vertexCount - 1) / 8)
        {
        case 0:
            firstSelectMap = gcvFALSE;
            secondSelectMap = gcvFALSE;
            outputRemapStart = 2;
            destReg = (inputVertex ?  pShader->remapRegStart : 2);
            break;
        case 1:
            firstSelectMap = gcvTRUE;
            secondSelectMap = gcvFALSE;
            outputRemapStart = 3;
            break;
        case 2:
            firstSelectMap = gcvTRUE;
            secondSelectMap = gcvTRUE;
            outputRemapStart = 4;
            break;
        case 3:
            firstSelectMap = gcvTRUE;
            secondSelectMap = gcvTRUE;
            outputRemapStart = 5;
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        if (!inputVertex)
        {
            destReg = pRA->resRegister;
            switch ((outputCount - 1) / 8)
            {
            case 0:
                firstSelectMap = gcvFALSE;
                secondSelectMap = gcvFALSE;
                destReg = outputRemapStart;
                break;
            case 1:
                firstSelectMap = gcvTRUE;
                secondSelectMap = gcvFALSE;
                break;
            case 2:
                firstSelectMap = gcvTRUE;
                secondSelectMap = gcvTRUE;
                break;
            case 3:
                firstSelectMap = gcvTRUE;
                secondSelectMap = gcvTRUE;
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
            }
        }
    }

    if (VIR_Operand_GetOpKind(src1Opnd) == VIR_OPND_IMMEDIATE)
    {
        gctUINT indexValue = 0;

        /* const index access */
        firstSelectMap = gcvFALSE;
        secondSelectMap = gcvFALSE;

        /* write this way to be clear */
        switch (VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src1Opnd)))
        {
        case VIR_TYPE_UINT32:
        case VIR_TYPE_INT32:
            indexValue = VIR_Operand_GetImmediateUint(src1Opnd);
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        switch (indexValue / 8)
        {
            case 0:
                destReg = (inputVertex ? pShader->remapRegStart : outputRemapStart);
                break;
            case 1:
                destReg = (inputVertex ? 2 : outputRemapStart + 1);
                gcmASSERT(pShader->shaderKind != VIR_SHADER_GEOMETRY);
                break;
            case 2:
                destReg = (inputVertex ? 3 : outputRemapStart + 2);
                gcmASSERT(pShader->shaderKind != VIR_SHADER_GEOMETRY);
                break;
            case 3:
                destReg = (inputVertex ? 4 : outputRemapStart + 3);
                gcmASSERT(pShader->shaderKind != VIR_SHADER_GEOMETRY);
                break;
            default:
                gcmASSERT(gcvFALSE);
                break;
        }

        regmapIndex = indexValue % 8;

        if ((inputVertex && (regmapIndex >= vertexCount)) ||
            (!inputVertex && (regmapIndex >= outputCount)))
        {
            VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
            VIR_Inst_SetSrcNum(pInst, 1);
            VIR_Operand_SetImmediateFloat(src0Opnd, 0.0);
            _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, destOpnd);

            return retValue;
        }
    }

    _VIR_RA_LS_GenTemp(pRA, &dstSymId);

    if (firstSelectMap)
    {
        /* HW has no src0 swizzle for select_map instruction,
           an extra mov is needed */
        if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasSelectMapSwizzleFix)
        {
            gctUINT webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, src1Opnd);
            VIR_RA_LS_Liverange *pLR;
            gctBOOL     xChannel = gcvTRUE;

            if (VIR_INVALID_WEB_INDEX != webIdx)
            {
                pLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                if (!_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pLR)) &&
                    _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)) != 0)
                {
                    xChannel = gcvFALSE;
                }
            }

            if (VIR_Operand_GetSwizzle(src1Opnd) != VIR_SWIZZLE_X ||
                !xChannel)
            {
                VIR_Instruction *movInst = gcvNULL;
                VIR_Operand     *movSrc = gcvNULL, *movDest = gcvNULL;


                VIR_Function_AddInstructionBefore(pFunc,
                    VIR_OP_MOV,
                    VIR_Operand_GetTypeId(src1Opnd),
                    pInst,
                    gcvTRUE,
                    &movInst);

                movSrc = VIR_Inst_GetSource(movInst, VIR_Operand_Src0);
                VIR_Operand_Copy(movSrc, src1Opnd);
                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd, movInst, movSrc);

                movDest = VIR_Inst_GetDest(movInst);
                _VIR_RA_LS_GenTemp(pRA, &movDstSymId);
                VIR_Operand_SetTempRegister(movDest,
                                            pFunc,
                                            movDstSymId,
                                            VIR_Operand_GetTypeId(src1Opnd));
                _VIR_RA_MakeColor(destReg, 0, &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, movDest, curColor);
                VIR_Operand_SetEnable(movDest, VIR_ENABLE_Y);
                extraMOV = gcvTRUE;
            }
        }

        /* first select map is needed */
        _VIR_RS_LS_InsertSelectMap(
            pRA,
            pInst,
            (inputVertex ? 1 : outputRemapStart),
            0x80,
            dstSymId,
            pRA->resRegister,
            extraMOV,
            movDstSymId);

        regmapIndex = 0;
    }

    if (secondSelectMap)
    {
        /* second select map is needed */
        _VIR_RS_LS_InsertSelectMap(
            pRA,
            pInst,
            (inputVertex ? 3 : outputRemapStart+2),
            0x81,
            dstSymId,
            pRA->resRegister,
            extraMOV,
            movDstSymId);
    }

    /* set the regmapIndex:
       if input vertex regmap and output regmpa are packed:
       1) input is index and output is index + 4
       (since input regmap is in xy channel and output regmap is in zw channel)

       if input vertex regmap and output regmpa are not packed:
       1) 0, when there is a select_map, since select_map enables component
       2) index % 8, when index is a const
       3) index, when count < 8
       */
    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL &&
        VIR_Shader_TCS_UsePackedRemap(pShader))
    {
        if (regmapIndex != -1)
        {
            if (!inputVertex)
            {
                regmapIndex = regmapIndex + 4;
            }
        }
        else
        {
            if (!inputVertex)
            {
                VIR_Operand  * addSrc = gcvNULL, *addDest = gcvNULL;

                /* insert an add instruction */
                retValue = VIR_Function_AddInstructionBefore(pFunc,
                    VIR_OP_ADD,
                    VIR_TYPE_UINT16,
                    pInst,
                    gcvTRUE,
                    &addSrc1Inst);
                if (retValue != VSC_ERR_NONE) return retValue;

                addSrc = VIR_Inst_GetSource(addSrc1Inst, VIR_Operand_Src0);
                VIR_Operand_Copy(addSrc, src1Opnd);
                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd, addSrc1Inst, addSrc);

                addSrc = VIR_Inst_GetSource(addSrc1Inst, VIR_Operand_Src1);
                VIR_Operand_SetImmediateInt(addSrc, 4);

                addDest = VIR_Inst_GetDest(addSrc1Inst);
                _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
                VIR_Operand_SetTempRegister(addDest,
                                            pFunc,
                                            addDstSymId,
                                            VIR_Operand_GetTypeId(src1Opnd));
                _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, addDest, curColor);
                VIR_Operand_SetEnable(addDest, VIR_ENABLE_X);

                /* use the new temp as the src1 opnd */
                needUpdateSrc1 = gcvTRUE;
            }
        }
    }

    /* set the attribute index */
    opndSwizzle = VIR_Operand_GetSwizzle(src0Opnd);
    ldEnable = VIR_Enable_ApplyMappingSwizzle(VIR_Inst_GetEnable(pInst), opndSwizzle);
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst, src0Opnd, gcvNULL, &attrIndex, &ldEnable);

    /* offset is in src2 */
    if (VIR_Operand_GetOpKind(src2Opnd) == VIR_OPND_IMMEDIATE)
    {
        gctUINT indexValue = 0;

        switch (VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src2Opnd)))
        {
        case VIR_TYPE_UINT32:
        case VIR_TYPE_INT32:
            indexValue = VIR_Operand_GetImmediateUint(src2Opnd);
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        attrIndex = attrIndex + indexValue;
    }
    else if (attrIndex != 0)
    {
        VIR_Instruction *addInst;
        VIR_Operand     *addSrc = gcvNULL, *addDest = gcvNULL;

        /* insert an add instruction */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            gcvTRUE,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        addSrc = VIR_Inst_GetSource(addInst, VIR_Operand_Src0);
        VIR_Operand_Copy(addSrc, src2Opnd);
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src2Opnd, addInst, addSrc);

        addSrc = VIR_Inst_GetSource(addInst, VIR_Operand_Src1);
        VIR_Operand_SetImmediateInt(addSrc, attrIndex);

        addDest = VIR_Inst_GetDest(addInst);
        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addDest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(src2Opnd));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addDest, curColor);
        /* x component could be already used by src1 */
        VIR_Operand_SetEnable(addDest, VIR_ENABLE_Y);

        /* use the new temp as the src2 opnd */
        needUpdateSrc2 = gcvTRUE;
    }

    /* ATTR_LD dest, base, invocationId, offset ==>
       LOAD_ATTR dest, regmapBase, regmapIndex, attributeIndex */
    retValue = _VIR_RA_LS_InsertLoadAttr(
        pRA,
        pInst,
        dstSymId,
        VIR_SWIZZLE_XYZW,
        destReg,
        !inputVertex,
        pShader->remapChannelStart,
        regmapIndex,
        src1Opnd,
        attrIndex,
        &newInst);

    if (needUpdateSrc1)
    {
        VIR_Operand *newSrc = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);

        /* src1 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newSrc,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(src1Opnd));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newSrc, curColor);
        VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_X);
    }

    if (needUpdateSrc2)
    {
        VIR_Operand *newSrc = VIR_Inst_GetSource(newInst, VIR_Operand_Src2);
        /* src2 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newSrc,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(src2Opnd));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newSrc, curColor);
        VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_Y);
    }

    /* HW has no out-of-bound check, thus SW needs to add a MOD instruction
       before the load_attr instruction */
    if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasLoadAttrOOBFix &&
        regmapIndex == -1 &&
        !_VIR_RA_LS_NotNeedOOB(pShader, pInst, inputVertex, vertexCount, outputCount))
    {
        VIR_SymId   modDest;
        VIR_Operand *modDestOpnd = gcvNULL, *baseOpnd = gcvNULL;
        if (needUpdateSrc1)
        {
            retValue = _VIR_RA_LS_InsertMOD(pRA, addSrc1Inst, inputVertex ? vertexCount : outputCount, &modInst, &modDest);
            /* change the src0*/
            baseOpnd = VIR_Inst_GetSource(addSrc1Inst, VIR_Operand_Src0);
        }
        else
        {
            retValue = _VIR_RA_LS_InsertMOD(pRA, newInst, inputVertex ? vertexCount : outputCount, &modInst, &modDest);
            /* change the src1*/
            baseOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
        }

        VIR_Operand_SetTempRegister(baseOpnd,
                                    pFunc,
                                    modDest,
                                    VIR_TYPE_INT16);

        modDestOpnd = VIR_Inst_GetDest(modInst);
        _VIR_RA_MakeColor(
            VIR_Operand_GetHwRegId(modDestOpnd),
            VIR_Operand_GetHwShift(modDestOpnd),
            &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, baseOpnd, curColor);
        VIR_Operand_SetSwizzle(baseOpnd, VIR_SWIZZLE_X);
    }

    _VIR_RA_LS_GenLoadAttr_SetEnable(pRA, pInst, newInst, ldEnable);

    return retValue;
}


/* generate load_attr for per-patch data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Patch(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Instruction     *newInst = gcvNULL;

    VIR_SymId           symId = VIR_INVALID_ID;
    VIR_Enable          ldEnable = VIR_ENABLE_NONE;
    VIR_Swizzle         opndSwizzle = VIR_SWIZZLE_INVALID;
    gctINT              attrIndex = 0;
    gctINT              regmapIndex;

    gctBOOL             needUpdateSrc2 = gcvFALSE;
    gctBOOL             perPatchOutput;
    VIR_SymId           addDstSymId = VIR_INVALID_ID;
    VIR_Symbol          *sym = gcvNULL;
    VIR_RA_HWReg_Color  curColor;

    VIR_Operand         *src0Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
    VIR_Operand         *src2Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src2);

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    sym = VIR_Operand_GetUnderlyingSymbol(src0Opnd);

    perPatchOutput = VIR_Symbol_isPerPatchOutput(sym);

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR flag */
    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
    {
        /* regmap index 1, the per-patch patch-address is in high 16bit */
        regmapIndex = 1;
    }
    else
    {
        /* regmap index 0, the per-patch patch-address is in low 16bit */
        regmapIndex = 0;
    }

    /* compute the enable for load_attr */
    opndSwizzle  = VIR_Operand_GetSwizzle(src0Opnd);
    ldEnable = VIR_Enable_ApplyMappingSwizzle(VIR_Inst_GetEnable(pInst), opndSwizzle);

    /* attribute index */
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst, src0Opnd, gcvNULL, &attrIndex, &ldEnable);

     /* offset in ATTR_LD src2 */
    if (VIR_Operand_GetOpKind(src2Opnd) == VIR_OPND_IMMEDIATE)
    {
        gctINT  src2Immx = VIR_Operand_GetImmediateInt(src2Opnd);
        if (VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_OUTER ||
            VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_INNER)
        {
            /* packed mode */
            attrIndex = attrIndex + (src2Immx / 4);
            ldEnable = ldEnable << (src2Immx % 4);
        }
        else
        {
            /* not packed mode */
            attrIndex = attrIndex + src2Immx;
        }
    }
    else
    {
         VIR_Instruction *addInst;
         VIR_Operand     *addSrc = gcvNULL, *addDest = gcvNULL;

         gcmASSERT(VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_OUTER &&
                   VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_INNER);

        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            gcvTRUE,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        addSrc = VIR_Inst_GetSource(addInst, VIR_Operand_Src0);
        VIR_Operand_Copy(addSrc, src2Opnd);
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src2Opnd, addInst, addSrc);

        addSrc = VIR_Inst_GetSource(addInst, VIR_Operand_Src1);
        VIR_Operand_SetImmediateInt(addSrc, attrIndex);

        addDest = VIR_Inst_GetDest(addInst);
        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addDest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(src2Opnd));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addDest, curColor);
        VIR_Operand_SetEnable(addDest, VIR_ENABLE_X);

        /* use the new temp as the src2 opnd */
        needUpdateSrc2 = gcvTRUE;
    }

    /* ATTR_LD dest, base, invocationId, offset ==>
       LOAD_ATTR dest, r0.w, 0/1, attributeIndex */

    _VIR_RA_LS_GenTemp(pRA, &symId);
    retValue = _VIR_RA_LS_InsertLoadAttr(
        pRA,
        pInst,
        symId,
        VIR_SWIZZLE_X,
        0,
        perPatchOutput,
        CHANNEL_W,
        regmapIndex,
        gcvNULL,
        attrIndex,
        &newInst);

    if (needUpdateSrc2)
    {
        /* src2 coming from the add instruction */
        VIR_Operand     *newSrc = gcvNULL;

        newSrc = VIR_Inst_GetSource(newInst, VIR_Operand_Src2);
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newSrc,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(src2Opnd));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newSrc, curColor);
        VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_X);
    }

    _VIR_RA_LS_GenLoadAttr_SetEnable(pRA, pInst, newInst, ldEnable);

    return retValue;
}

static VIR_TypeId
_VIR_RA_LS_GenBaseTypeID(
    IN VIR_Shader   *pShader,
    IN VIR_Symbol   *pSym
    )
{
    VIR_TypeId  result = VIR_TYPE_UNKNOWN;
    VIR_Type    *symType = VIR_Symbol_GetType(pSym);

    if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
    {
        VIR_TypeId base_type_id = VIR_Type_GetBaseTypeId(symType);
        symType = VIR_Shader_GetTypeFromId(pShader, base_type_id);
    }

    switch(VIR_Type_GetKind(symType))
    {
        case VIR_TY_SCALAR:
        case VIR_TY_VECTOR:
            result = VIR_Type_GetIndex(symType);
            break;
        case VIR_TY_MATRIX:
            result = VIR_GetTypeRowType(VIR_Type_GetIndex(symType));
            break;
        default:
            gcmASSERT(0);
    }
    return result;
}

/* generate store_attr for GS output data */
VSC_ErrCode _VIR_RA_LS_GenStoreAttr_Output(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Symbol      *pSym,
    gctUINT         attrOffset,
    VIR_HwRegId     hwStartReg,
    gctUINT         hwShift
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId = VIR_INVALID_ID;
    gctINT              attrIndex = 0;
    VIR_Swizzle         valSwizzle = VIR_SWIZZLE_XYZW;
    VIR_TypeId          elementTypeId = _VIR_RA_LS_GenBaseTypeID(pShader, pSym);
    VIR_RA_HWReg_Color      curColor;

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT0 || VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM);

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR flag */
    VIR_Symbol_SetFlag(pSym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    _VIR_RA_LS_GetSym_Enable_Swizzle(pSym, gcvNULL, &valSwizzle);


    /* STORE_ATTR output, r0.x, attributeIndex, rx */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_STORE_ATTR,
                VIR_TYPE_UINT16,
                pInst,
                gcvTRUE,
                &newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* the base is in r0.x */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);

    if (pRA->pHwCfg->hwFeatureFlags.multiCluster)
    {
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XYYY);
    }

    /* set attributeIndex */
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst,
                                        gcvNULL,
                                        pSym,
                                        &attrIndex,
                                        gcvNULL);

    VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src1], attrIndex + attrOffset);

    /* value in ATTR_ST src2 */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                pFunc,
                                symId,
                                elementTypeId);
    _VIR_RA_MakeColor(hwStartReg, hwShift, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src2], valSwizzle);

    /* only store_attr destionation enable matters, since src2 is always temp */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_Swizzle_2_Enable(valSwizzle));

    return retValue;
}

/* generate store_attr for per-vertex/per-patch data */
VSC_ErrCode _VIR_RA_LS_GenStoreAttr(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd,
    gctBOOL         isPerVertex
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId symId = VIR_INVALID_ID;
    gctINT              attrIndex = 0;
    VIR_Enable          stEnable = VIR_ENABLE_NONE;
    VIR_SymId           addDstSymId = VIR_INVALID_ID;
    VIR_Symbol          *sym = gcvNULL;
    VIR_RA_HWReg_Color      curColor;

    curColor = InvalidColor;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_ST);

    if (pOpnd)
    {
        sym = VIR_Operand_GetUnderlyingSymbol(pOpnd);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR flag */
    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    /* ATTR_ST output, invocationId, offset, value ==>
       STORE_ATTR output, r0.x/r0.w, attributeIndex, value */
    retValue= VIR_Function_AddInstructionAfter(pFunc,
                VIR_OP_STORE_ATTR,
                VIR_TYPE_UINT16,
                pInst,
                gcvTRUE,
                &newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* the base is in r0.x (per-vertex) or r0.w (per-patch),
       for GS, output base is also in r0.x */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    if (isPerVertex)
    {
        _VIR_RA_MakeColor(0, CHANNEL_X, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    }
    else
    {
        _VIR_RA_MakeColor(0, CHANNEL_W, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    }

    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_GEOMETRY &&
        pRA->pHwCfg->hwFeatureFlags.multiCluster)
    {
        gcmASSERT(curColor._hwShift == CHANNEL_X);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XYYY);
    }

    /* set attributeIndex */
    stEnable = VIR_Operand_GetEnable(pInst->dest);
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst, pOpnd, gcvNULL, &attrIndex, &stEnable);
    VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src1], attrIndex);

    /* invocationId (src0) in ATTR_ST should have no use here
       per-vertex: should always be gl_Invocation,
       per-patch: should always be 0 (add assertion) */
    /* offset in ATTR_ST src1 */
    if (VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src1]) == VIR_OPND_IMMEDIATE)
    {
        gctINT src1Immx = VIR_Operand_GetImmediateInt(pInst->src[VIR_Operand_Src1]);

        if (VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_OUTER ||
            VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_INNER)
        {
            /* packed mode */
            attrIndex = attrIndex + (src1Immx / 4);
            stEnable = stEnable << (src1Immx % 4);
        }
        else
        {
            attrIndex = attrIndex + src1Immx;
        }

        VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src1], attrIndex);
    }
    else
    {
        VIR_Instruction *addInst;

        gcmASSERT(VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_OUTER &&
                  VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_INNER);

        /* insert an add instruction */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            gcvTRUE,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        VIR_Operand_Copy(addInst->src[VIR_Operand_Src0], pInst->src[VIR_Operand_Src1]);
        _VIR_RA_LS_RewriteColor_Src(pRA,
                                    pInst,
                                    pInst->src[VIR_Operand_Src1],
                                    addInst,
                                    addInst->src[VIR_Operand_Src0]);

        VIR_Operand_SetImmediateInt(addInst->src[VIR_Operand_Src1], attrIndex);

        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addInst->dest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->dest, curColor);
        VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_X);

        /* src1 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetTypeId(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);
    }

    /* value in ATTR_ST src2 */
    VIR_Operand_Copy(newInst->src[VIR_Operand_Src2], pInst->src[VIR_Operand_Src2]);
    _VIR_RA_LS_RewriteColor_Src(pRA,
                                pInst,
                                pInst->src[VIR_Operand_Src2],
                                newInst,
                                newInst->src[VIR_Operand_Src2]);

    newInst->dest = pInst->dest;
    if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5 &&
        VIR_Inst_Store_Have_Dst(pInst))
    {
        gcmASSERT(pRA->resRegister != VIR_INVALID_ID);
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    }
    VIR_Operand_SetEnable(newInst->dest, stEnable);

    gcmASSERT(stEnable == VIR_Operand_GetEnable(pInst->dest));


    /* remove attr_st */
    VIR_Inst_SetDest(pInst, gcvNULL);
    VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);

    pInst = newInst;

    return retValue;
}

/* primitiveId is load from per-patch data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Patch_r0(
    VIR_RA_LS       *pRA,
    VIR_Symbol      *primitiveIdSym,
    VIR_HwRegId     hwRegNo)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId = VIR_INVALID_ID;
    gctINT              attrIndex = 0;
    VIR_Enable          destEnable = VIR_ENABLE_NONE;

    VIR_RA_HWReg_Color      curColor;

    curColor = InvalidColor;

    /* load_attr */
    VIR_Function_PrependInstruction(pFunc,
                VIR_OP_LOAD_ATTR,
                VIR_TYPE_UINT16,
                &newInst);

    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    /* base is r0.w */
    _VIR_RA_MakeColor(0, 3, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);

    /* regmap index 1, the per-patch patch-address is in high 16bit in TES */
    VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src1], 0x1);

    /* attribute index */
    _VIR_RA_LS_ComputeAttrIndexEnable(gcvNULL, gcvNULL, primitiveIdSym, &attrIndex, &destEnable);
    VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src2], attrIndex);

    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetEnable(newInst->dest, destEnable);
    _VIR_RA_MakeColor(hwRegNo, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);

    VIR_Symbol_SetFlag(primitiveIdSym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    return retValue;
}

VSC_ErrCode _VIR_RA_LS_InsertStoreAttr(
    VIR_RA_LS       *pRA,
    gctINT          attrIndex,
    VIR_Enable      destEnable,
    VIR_Instruction **newInst)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_SymId           symId = VIR_INVALID_ID;
    VIR_RA_HWReg_Color      curColor;

    curColor = InvalidColor;
    gcmASSERT(VIR_Shader_GetKind(pShader) != VIR_SHADER_GEOMETRY);

    retValue = VIR_Function_AddInstruction(pFunc,
                VIR_OP_STORE_ATTR,
                VIR_TYPE_UINT16,
                newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* src0 - regmap */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister((*newInst)->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, CHANNEL_W, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->src[VIR_Operand_Src0], curColor);

    /* src1 - attrIndex */
    VIR_Operand_SetImmediateInt((*newInst)->src[VIR_Operand_Src1], attrIndex);

    /* src2 need to be set by the clients */

    /* in some cases (src2 is temp), store_attr destionation enable matters,
       in other cases (src2 is immediate/uniform/indirect), store_attr needs destination
       dest need to be reset by the client when needed */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister((*newInst)->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->dest, curColor);
    VIR_Operand_SetEnable((*newInst)->dest, destEnable);

    return retValue;
}

/* per HW requirement, generate store_attr for r0.z and ro.w data at the end of the shader */
VSC_ErrCode _VIR_RA_LS_GenStoreAttr_Patch_r0(
    VIR_RA_LS       *pRA)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_SymId           symId = VIR_INVALID_ID;
    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* store_attr for r0.w and r0.z */
    /* no need to reset dest, because src2 is temp */
    _VIR_RA_LS_InsertStoreAttr(pRA, 1, VIR_ENABLE_ZW, &newInst);

    /* set the src2 */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src2], VIR_SWIZZLE_XXXY);

    return retValue;
}


/* generate emit */
VSC_ErrCode _VIR_RA_LS_GenEmitRestart(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctBOOL         isEmit
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_SymId           symId = VIR_INVALID_ID;
    VIR_Instruction     *newInst = gcvNULL;
    VIR_RA_HWReg_Color  curColor;
    gctBOOL             bCheckAllOutput = gcvTRUE;
    gctINT              streamNumber = 0;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT0         ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM   ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_RESTART0      ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_RESTART_STREAM);

    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT_STREAM)
    {
        bCheckAllOutput = gcvFALSE;
        gcmASSERT(VIR_Operand_isImm(VIR_Inst_GetSource(pInst, 0)));
        streamNumber = VIR_Operand_GetImmediateInt(VIR_Inst_GetSource(pInst, 0));
    }

    curColor = InvalidColor;

    if (isEmit)
    {
        /* generating a sequence of store_attr */
        gctUINT         outputIdx, usageIdx;
        VIR_USAGE_KEY   usageKey;
        gctUINT         i=0;
        for (outputIdx = 0;
             outputIdx < VIR_IdList_Count(VIR_Shader_GetOutputs(pShader));
             outputIdx ++)
        {
            VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader), outputIdx));

            if (isSymVectorizedOut(pOutputSym))
            {
                continue;
            }

            if (!bCheckAllOutput && VIR_Symbol_GetStreamNumber(pOutputSym) != streamNumber)
            {
                continue;
            }

            /* Find the usage, the matrix output could be in non-consecutive registers,
               thus need to find corresponding usages */
            for (i = 0; i < VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pOutputSym), -1); i++)
            {
                usageKey.pUsageInst = pInst;
                usageKey.pOperand = (VIR_Operand*)(gctUINTPTR_T) (pOutputSym->u2.tempIndex + i);
                usageKey.bIsIndexingRegUsage = gcvFALSE;
                usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
                if (VIR_INVALID_USAGE_INDEX != usageIdx)
                {
                    VIR_USAGE   *pUse = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
                    VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2ColorLR(pRA, pUse->webIdx);
                    _VIR_RA_LS_GenStoreAttr_Output(pRA, pInst, pOutputSym, i,
                        _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)),
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
                }
            }
        }
    }

    /* generate aq_emit */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                isEmit ? VIR_OP_EMIT : VIR_OP_RESTART,
                VIR_TYPE_UINT16,
                pInst,
                gcvTRUE,
                &newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* src0 - r0 */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XYZW);
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);

    /* src1 - streamNumber. */
    if (VIR_Shader_GS_HasStreamOut(pShader))
    {
        VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(newInst, 1), streamNumber);
    }

    /* samperswizzle */
    if (isEmit)
    {
        VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src2],
            VIR_Shader_GS_HasRestartOp(pShader) ? 1 : 0);
    }

    /* dest - r0 */
    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_TYPE_FLOAT_X4);
    _VIR_RA_MakeColor(0, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_XYZW);

    /* remove emit */
    VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
    pInst = newInst;

    return retValue;
}

VSC_ErrCode _VIR_RA_LS_RewriteColorInst(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand             *pSrcOpnd, *pDestOpnd;
    VIR_RA_LS_Liverange     *pBaseLR;

    gctUINT             defIdx, webIdx = 0;
    VIR_OpCode          opcode    = VIR_OP_NOP;

    retValue = _VIR_RA_LS_InvalidDataRegisterUsedMask(pRA, pInst);
    if(retValue != VSC_ERR_NONE) return retValue;

    opcode = VIR_Inst_GetOpcode(pInst);
    switch(opcode)
    {
    case VIR_OP_MOVA:
        /*
        ** If the dest operand of a MOVA is invalid(optimize this in _VIR_RA_LS_AssignColorForA0B0Inst),
        ** then we don't need to write the source operand.
        */
        if (VIR_Operand_GetHwRegId(VIR_Inst_GetDest(pInst)) != VIR_INVALID_HWREG)
        {
            _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[0], pInst, pInst->src[0]);
        }
        break;

    case VIR_OP_LDARR:
        {
            /* 3 cases to handle LDARR:
            case 1: replace the indexed array load in its use,
            only when dst1 is only defined by ldarr
            1: mova t2.x, t1.x
            2: ldarr dst1, src0, t2.x
            ...
            4: op dst, dst1, XXX   <== dst1 is only defined by ldarr
            ==>
            1: mova a0.x, t1.x
            2: (removed)
            ...
            4: op dst, src0[a0.x], XXX

            case 2: dst1 is defined by other places, we don't replace its use,
            but we need to change  the ldarr to a mov instruction
            1: mova t2.x, t1.x
            2: ldarr dst1.xyz, src0, t2.x
            3: mov dst1.w, 0
            4: op dst, dst1, ...   <== dst1 is defined in two places
            ==>
            1: mova a0.x, t1.x
            2: mov dst1.xyz, sr0[a0.x]
            3: mov dst1.w, 0
            4: op dst, dst1, XXX

            case 3: the addr is spilled
            1: mova t2.x, t1.x
            2: ldarr dst1, src0, t2.x src0 is spilled
            ==>
            1: mad  offset.x, spillOffset, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__
            2: load t3.x, spillBase.x, offset.x
            */

            /* get the base src0 */
            pSrcOpnd = pInst->src[VIR_Operand_Src0];
            webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pSrcOpnd);
            if (VIR_INVALID_WEB_INDEX != webIdx)
            {
                pBaseLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                if (isLRSpilled(pBaseLR))
                {
                    _VIR_RA_LS_InsertSpill_LDARR(pRA, pInst, pSrcOpnd, pBaseLR);
                    break;
                }
            }

            {
                /* color the base opnd */
                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pSrcOpnd, pInst, pSrcOpnd);

                if (!_VIR_RA_LS_removableLDARR(pRA, pInst, gcvFALSE))
                {
                    /* change the ldarr to mov instruction if could not be deleted */
                    VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(pInst, 1);
                    _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pInst->dest);
                    if (VIR_Shader_isDual16Mode(pShader))
                    {
                        VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_32);
                    }
                }
                else
                {
                    VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
                }
            }
        }
        break;

    case VIR_OP_STARR:
        {
            /*
            replace the indexed array load in its use
            1: mova t2.x, t1.x
            2: starr dest, t2.x, t3.x
            ==>
            1: mova a0.x, t1.x
            2: mov dest[a0.x], t3.x

            case 2: if the addr is spilled
            1: mova t2.x, t1.x
            2: starr dest, t2.x, t3.x   (dest is spilled)
            ==>
            1: mad  offset.x, t1.x, __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__, spillOffset
            2: store spillBase.x, offset.x, t3.x

            */

            /* get the dest, set its indexing */
            pDestOpnd = pInst->dest;
            defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pBaseLR =  _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
                if (isLRSpilled(pBaseLR))
                {
                    _VIR_RA_LS_InsertFill_STARR(pRA, pInst, pDestOpnd, pBaseLR);
                    break;
                }
            }

            {
                _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pDestOpnd);

                /* change the starr to mov */
                VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
                VIR_Inst_SetSource(pInst, 0, pInst->src[VIR_Operand_Src1]);
                VIR_Inst_SetSrcNum(pInst, 1);

                _VIR_RA_LS_RewriteColor_Src(pRA,
                                            pInst,
                                            pInst->src[VIR_Operand_Src0],
                                            pInst,
                                            pInst->src[VIR_Operand_Src0]);
            }
        }
        break;
    case VIR_OP_ATTR_LD:
        /* converter makes sure that pervertex/perpatch only in attr_ld/attr_st */
        /* generate the load_attr */
        pSrcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
        if (VIR_Operand_IsArrayedPerVertex(pSrcOpnd))
        {
            _VIR_RA_LS_GenLoadAttr_Vertex(pRA, pInst);
        }
        else if (VIR_Operand_IsPerPatch(pSrcOpnd))
        {
            _VIR_RA_LS_GenLoadAttr_Patch(pRA, pInst);
        }
        else
        {
            gcmASSERT(isSymUnused(VIR_Operand_GetUnderlyingSymbol(pSrcOpnd)) ||
                      isSymVectorizedOut(VIR_Operand_GetUnderlyingSymbol(pSrcOpnd)));

            /* unused attr/output */
            VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
        }
        break;
    case VIR_OP_ATTR_ST:
        /* generate the store_attr */
        pDestOpnd = pInst->dest;
        if (VIR_Operand_IsArrayedPerVertex(pDestOpnd))
        {
            _VIR_RA_LS_GenStoreAttr(pRA, pInst, pDestOpnd, gcvTRUE);
        }
        else if (VIR_Operand_IsPerPatch(pDestOpnd))
        {
            _VIR_RA_LS_GenStoreAttr(pRA, pInst, pDestOpnd, gcvFALSE);
        }
        else
        {
            gcmASSERT(isSymUnused(VIR_Operand_GetUnderlyingSymbol(pDestOpnd)) ||
                      isSymVectorizedOut(VIR_Operand_GetUnderlyingSymbol(pDestOpnd)));

            /* unused attr/output */
            VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
        }
        break;
    case VIR_OP_STORE_ATTR:
    case VIR_OP_LOAD_ATTR:
    case VIR_OP_LOAD_ATTR_O:
        /* nothing to do, already colored when generating */
        break;
    case VIR_OP_EMIT0:
    case VIR_OP_EMIT_STREAM:
        _VIR_RA_LS_GenEmitRestart(pRA, pInst, gcvTRUE);
        break;
    case VIR_OP_RESTART0:
    case VIR_OP_RESTART_STREAM:
        /* we don't need to generate restart for the case where
           output primitive type is point */
        if (VIR_Shader_GS_HasRestartOp(pShader))
        {
            _VIR_RA_LS_GenEmitRestart(pRA, pInst, gcvFALSE);
        }
        else
        {
            VIR_Pass_DeleteInstruction(pFunc, pInst, gcvNULL);
        }
        break;
    case VIR_OP_ATOMCMPXCHG:
    case VIR_OP_ATOMCMPXCHG_L:
        if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5)
        {
            VIR_Swizzle  src2Swizzle = VIR_Operand_GetSwizzle(pInst->src[2]);
            VIR_Enable   dstEnable = VIR_ENABLE_XY;
            VIR_RA_LS_Liverange *pLR;

            defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pLR =  _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
                dstEnable = VIR_RA_LS_LR2WebChannelMask(pRA, pLR);
            }
            VIR_Operand_SetEnable(pInst->dest, dstEnable);
            VIR_Swizzle_SetChannel(src2Swizzle, 2,
                                   VIR_Swizzle_GetChannel(src2Swizzle, 0));
            VIR_Swizzle_SetChannel(src2Swizzle, 3,
                                   VIR_Swizzle_GetChannel(src2Swizzle, 1));
            VIR_Operand_SetSwizzle(pInst->src[2], src2Swizzle);
        }
        /* intentionally fall through */
    default: /* other instructions */
        {
            /*
            ** rewrite the src operands:
            **  When register spill happens, for a instruction that need even/odd pair register,
            **  we need to check the even/odd required operand first.
            */
            if (_VIR_RA_LS_InstNeedEvenOddReg(pRA, pInst) && pShader->hasRegisterSpill)
            {
                VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
                pSrcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
                for (; pSrcOpnd != gcvNULL; pSrcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
                {
                    gcmASSERT(!VIR_Operand_IsPerPatch(pSrcOpnd) &&
                              !VIR_Operand_IsArrayedPerVertex(pSrcOpnd));

                    if (_VIR_RA_LS_OperandEvenReg(pRA, pInst, pSrcOpnd) ||
                        _VIR_RA_LS_OperandOddReg(pRA, pInst, pSrcOpnd))
                    {
                        retValue = _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pSrcOpnd, pInst, pSrcOpnd);
                        if (retValue != VSC_ERR_NONE)
                        {
                            return retValue;
                        }
                    }
                }
            }

            VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
            pSrcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
            for (; pSrcOpnd != gcvNULL; pSrcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                gcmASSERT(!VIR_Operand_IsPerPatch(pSrcOpnd) &&
                          !VIR_Operand_IsArrayedPerVertex(pSrcOpnd));
                retValue = _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pSrcOpnd, pInst, pSrcOpnd);
                if (retValue != VSC_ERR_NONE)
                {
                    return retValue;
                }
            }

            /* rewrite the dst operands */
            pDestOpnd = pInst->dest;
            if (pDestOpnd)
            {
                gcmASSERT(!VIR_Operand_IsPerPatch(pDestOpnd) &&
                          !VIR_Operand_IsArrayedPerVertex(pDestOpnd));
                _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pDestOpnd);
            }
        }
        break;
    }

    return retValue;
}

gctBOOL
_VIR_RA_LS_OperandSpilled(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd)
{
    gctUINT             webIdx;
    VIR_RA_LS_Liverange *pLR;

    webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pOpnd);
    if (VIR_INVALID_WEB_INDEX != webIdx)
    {
        pLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
        if (isLRSpilled(pLR))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* ===========================================================================
   _VIR_RA_LS_CheckInstructionDstAndSrcs
   check the max number of dst/sources that is spilled in an instruction
   ===========================================================================
*/
void
_VIR_RA_LS_CheckInstructionDstAndSrcs(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc,
    gctUINT         *spilledOpndCount)
{
    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand         *pSrcOpnd;
    gctUINT             maxSpilledOpndCount = 0;
    gctUINT             curSpilledOpndCount = 0;
    gctUINT             maxEvenRegCount = 0, evenRegCount = 0;
    VIR_RA_LS_Liverange     *pLR;
    gctUINT                 defIdx;

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    for (; pInst != gcvNULL;
           pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        curSpilledOpndCount = 0;
        evenRegCount = 0;

        /*
        ** Check dest:
        **  For those instructions that some of the enabled dest channels may not be written,
        **  if it is spilled, we need to spill the dest first.
        */
        if (!vscVIR_IsInstDefiniteWrite(VIR_RA_LS_GetLvInfo(pRA)->pDuInfo, pInst, VIR_INVALID_ID, gcvFALSE))
        {
            defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
            if (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pLR =  _VIR_RA_LS_Def2ColorLR(pRA, defIdx);

                if (isLRSpilled(pLR))
                {
                    curSpilledOpndCount++;
                }
            }
        }

        /* Check sources. */
        VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
        pSrcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
        for (; pSrcOpnd != gcvNULL; pSrcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
        {
            if (_VIR_RA_LS_OperandSpilled(pRA, pInst, pSrcOpnd))
            {
                curSpilledOpndCount++;

                if (_VIR_RA_LS_OperandEvenReg(pRA, pInst, pSrcOpnd))
                {
                    evenRegCount++;
                }
            }
        }

        if (maxSpilledOpndCount < curSpilledOpndCount)
        {
            maxSpilledOpndCount = curSpilledOpndCount;
        }

        if (evenRegCount > maxEvenRegCount)
        {
            maxEvenRegCount = evenRegCount;
        }
    }

    /* Right now we can't assume it always starts with the even register, so we need a extra register. */
    if (maxSpilledOpndCount <= maxEvenRegCount * 2)
    {
        maxSpilledOpndCount++;
    }

    /* Return the value. */
    if (spilledOpndCount && ((*spilledOpndCount) < maxSpilledOpndCount)) /* set spilledOpndCount only if spilledOpndCount less than maxSpilledOpndCount */
    {
        *spilledOpndCount = maxSpilledOpndCount;
    }
}

/* ===========================================================================
   _VIR_RA_LS_RewriteColors
   put the color infor into the operand
   ===========================================================================
*/
VSC_ErrCode _VIR_RA_LS_RewriteColors(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Instruction     *pInst, *nextInst;
    VIR_InstIterator    instIter;
    gctUINT             i;
    VIR_RA_LS_Liverange *pLR;

    /* We need to clear the temp color for a new function because so far we use instruction ID to check usage. */
    for (i = 0; i < VIR_RA_LS_GetNumWeb(pRA); i++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, i);
        if (isLRSpilled(pLR))
        {
            _VIR_RA_InitLRTempColor(pLR);
        }
    }

    /* Set the current function. */
    VIR_Shader_SetCurrentFunction(VIR_RA_LS_GetShader(pRA), pFunc);

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    while (pInst != gcvNULL)
    {
        nextInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);

        retValue = _VIR_RA_LS_RewriteColorInst(pRA, pInst);
        CHECK_ERROR(retValue, "_VIR_RA_LS_RewriteColorInst");

        pInst = nextInst;
    }

    return retValue;
}

gctBOOL
_VIR_RA_LS_ClearUsedColorFromActiveLR(
    VIR_RA_LS           *pRA,
    gctUINT             webIdx)
{
    VIR_RA_HWReg_Color      curColor;
    VIR_Shader*             pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Dumper*             pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions*     pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_RA_LS_Liverange*    pPrev = gcvNULL;
    VIR_RA_LS_Liverange*    pCurr = gcvNULL;
    VIR_RA_LS_Liverange*    pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
    VIR_RA_HWReg_Type       hwRegType = pLR->hwType;
    gctUINT                 shift = 0;
    gctUINT                 regCount = 0;

    if (!isLRA0OrB0(pLR))
    {
        return gcvFALSE;
    }

    /* There are only one A0/B0 register. */
    regCount  = _VIR_RA_GetMaxRegCount(pRA, pRA->pHwCfg, hwRegType);
    gcmASSERT(regCount == 1);

    curColor = InvalidColor;
    pPrev = VIR_RA_LS_GetActiveLRHead(pRA);
    pCurr = pPrev->nextActiveLR;

    while (pCurr != &LREndMark)
    {
        /* Check the matched HW register type. */
        if (pCurr->hwType == hwRegType && !_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pCurr)))
        {
            curColor = _VIR_RA_GetLRColor(pCurr);

            /* clear the used bits*/
            _VIR_RA_LS_ClearUsedColor(pRA, hwRegType,
                _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pCurr)),
                _VIR_RA_Color_Channels(
                    VIR_RA_LS_LR2WebChannelMask(pRA, pCurr),
                    _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pCurr))));

            if (VIR_Shader_isDual16Mode(pShader))
            {
                 _VIR_RA_LS_ClearUsedColor(pRA, hwRegType,
                    _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pCurr)),
                    _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pCurr),
                        _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pCurr))));
            }

            /* make the pPrev to be a invalid color */
            _VIR_RA_SetLRColor(pCurr, VIR_RA_INVALID_REG, 0);
            _VIR_RA_SetLRColorHI(pCurr, VIR_RA_INVALID_REG, 0);
            VIR_RA_LR_SetFlag(pCurr, VIR_RA_LRFLAG_A0B0_INVALID);

            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption), VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
            {
                VIR_LOG(pDumper, "LR%d ", pCurr->webIdx);
                _VIR_RA_LS_DumpColor(pRA, curColor, pCurr);
                VIR_LOG(pDumper, " was replaced by LR%d\n", webIdx);
                VIR_LOG_FLUSH(pDumper);
            }

            /* Since we get enough register here, we can leave now. */
            if (_VIR_RA_LS_ChannelFit(pRA, pLR, gcvNULL, regCount - 1, &shift))
            {
                return gcvTRUE;
            }
        }
        pCurr = pCurr->nextActiveLR;
    }

    return gcvFALSE;
}

VSC_ErrCode _VIR_RA_LS_AssignColorForA0B0Inst(
    VIR_RA_LS           *pRA,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_OpCode          opcode    = VIR_Inst_GetOpcode(pInst);
    gctUINT             defIdx = VIR_INVALID_DEF_INDEX;
    VIR_RA_LS_Liverange *pMovaLR;
    VIR_RA_LS_Liverange *pBaseLR;
    VIR_RA_HWReg_Color  curColor;
    VIR_Operand         *pOpnd = gcvNULL;
    gctUINT             webIdx = VIR_INVALID_WEB_INDEX;
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_DEF_USAGE_INFO  *pDuInfo = VIR_RA_LS_GetLvInfo(pRA)->pDuInfo;
    gctBOOL             bClearUsedColor = gcvFALSE;
    gctBOOL             bRegisterSpill = pShader->hasRegisterSpill;
    gctBOOL             bTryToDisableDual16AndRecolor = gcvFALSE;
    gctBOOL             bUnderDual16 = gcvFALSE;

    curColor = InvalidColor;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption), VSC_OPTN_RAOptions_DISABLE_DUAL16_FOR_A0B0))
    {
        bTryToDisableDual16AndRecolor = gcvTRUE;
    }

    if (opcode == VIR_OP_MOVA)
    {
        /*
        ** When all usage operands of this MOVA are spilled, we don't need to assign A0/B0 for the DEST of this MOVA
        ** because in that case we generate a extra MAD to calcuate the offset of the base operand and
        ** use LOAD/STORE to access the usage operands, not the temp register directly.
        */
        if (bRegisterSpill)
        {
            VIR_OperandInfo         instDstInfo;
            VIR_Enable              enable = VIR_Inst_GetEnable(pInst);
            gctUINT8                channel;
            VIR_GENERAL_DU_ITERATOR instDUIter;
            VIR_USAGE*              pInstUsage = gcvNULL;
            VIR_OpCode              usageInstOpcode;
            VIR_Instruction*        pUsageInst = gcvNULL;
            gctBOOL                 bAllUsageSpilled = gcvTRUE;

            VIR_Operand_GetOperandInfo(pInst, VIR_Inst_GetDest(pInst), &instDstInfo);

            for (channel = 0; channel < VIR_CHANNEL_COUNT; ++channel)
            {
                if (bAllUsageSpilled == gcvFALSE)
                {
                    break;
                }

                if (!(enable & (1 << channel)))
                {
                    continue;
                }

                /* Get all usage instructions. */
                vscVIR_InitGeneralDuIterator(&instDUIter,
                                             pRA->pLvInfo->pDuInfo,
                                             pInst,
                                             instDstInfo.u1.virRegInfo.virReg,
                                             channel,
                                             gcvFALSE);
                for (pInstUsage = vscVIR_GeneralDuIterator_First(&instDUIter);
                     pInstUsage != gcvNULL;
                     pInstUsage = vscVIR_GeneralDuIterator_Next(&instDUIter))
                {
                    pUsageInst = pInstUsage->usageKey.pUsageInst;

                    if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
                    {
                        continue;
                    }

                    /* Get the base operand LR. */
                    usageInstOpcode = VIR_Inst_GetOpcode(pUsageInst);
                    if (usageInstOpcode == VIR_OP_LDARR)
                    {
                        webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pUsageInst, VIR_Inst_GetSource(pUsageInst, 0));
                        if (webIdx == VIR_INVALID_WEB_INDEX)
                        {
                            bAllUsageSpilled = gcvFALSE;
                            break;
                        }
                        pBaseLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                    }
                    else if (usageInstOpcode == VIR_OP_STARR)
                    {
                        defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pUsageInst);
                        if (defIdx == VIR_INVALID_DEF_INDEX)
                        {
                            bAllUsageSpilled = gcvFALSE;
                            break;
                        }
                        pBaseLR = _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
                    }
                    else
                    {
                        /* Invalid case here. */
                        gcmASSERT(gcvFALSE);
                        bAllUsageSpilled = gcvFALSE;
                        break;
                    }

                    if (pBaseLR == gcvNULL || !isLRSpilled(pBaseLR))
                    {
                        bAllUsageSpilled = gcvFALSE;
                        break;
                    }
                }
            }

            if (bAllUsageSpilled)
            {
                _VIR_RA_LS_ExpireActiveLRs(pRA, VIR_Inst_GetId(pInst));
                return retValue;
            }
        }

        defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
        pMovaLR = _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
        webIdx = pMovaLR->webIdx;

        gcmASSERT(isLRA0OrB0(pMovaLR));

        if (_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pMovaLR)))
        {
            if (isLRB0(pMovaLR))
            {
                curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, gcvFALSE, 0, -1, gcvNULL);
                curColor._HIhwRegId = curColor._hwRegId;
                curColor._HIhwShift = curColor._hwShift;
            }
            else
            {
                curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, VIR_Shader_isDual16Mode(pShader), 0, -1, gcvNULL);
                bUnderDual16 = VIR_Shader_isDual16Mode(pShader);
            }

            /*
            ** Here are two solutions when failing to find a new color:
            **  1) Disable dual16 if it is possible and re-color.
            **  2) Clear the used color from the active LRs, then try to find the new color again.
            */
            if (_VIR_RA_LS_IsInvalidColor(curColor))
            {
                /* Disable dual16 if it is possible and re-color. */
                if (bTryToDisableDual16AndRecolor && bUnderDual16)
                {
                    VIR_RA_LS_SetDisableDual16AndRecolor(pRA, gcvTRUE);
                    return retValue;
                }

                /* Clear the used color from the active LRs, then try to find the new color again. */
                bClearUsedColor = _VIR_RA_LS_ClearUsedColorFromActiveLR(pRA, webIdx);

                if (bClearUsedColor)
                {
                    if (isLRB0(pMovaLR))
                    {
                        curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, gcvFALSE, 0, -1, gcvNULL);
                        curColor._HIhwRegId = curColor._hwRegId;
                        curColor._HIhwShift = curColor._hwShift;
                    }
                    else
                    {
                        curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, VIR_Shader_isDual16Mode(pShader), 0, -1, gcvNULL);
                    }
                }

                if(!_VIR_RA_LS_IsInvalidColor(curColor))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
                    {
                        VIR_LOG(pDumper, "find new ");
                        _VIR_RA_LS_DumpColor(pRA, curColor, pMovaLR);
                        VIR_LOG(pDumper, " for LR%d\n", pMovaLR->webIdx);
                        VIR_LOG_FLUSH(pDumper);
                    }
                }
                else
                {
                    retValue = VSC_RA_ERR_OUT_OF_A0B0_REG_FAIL;
                    return retValue;
                }
            }

            pMovaLR->u1.color = curColor;

            retValue = _VIR_RA_LS_AddActiveLRs(pRA, webIdx, gcvTRUE, pFunc, 0);
        }

        _VIR_RA_MakeColor(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);
        if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pMovaLR)))
        {
            _VIR_RA_MakeColorHI(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
                _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);
        }

        _VIR_RA_LS_SetOperandHwRegInfo(pRA, pInst->dest, curColor);
    }
    else if (opcode == VIR_OP_LDARR || opcode == VIR_OP_STARR)
    {
        VIR_Instruction         *pNewInst = gcvNULL, *pDefInst = gcvNULL;
        VIR_Operand             *pNewOpnd = gcvNULL;
        VIR_GENERAL_UD_ITERATOR udIter;
        VIR_DEF                 *pDef = gcvNULL;
        VIR_RA_LS_Liverange     *pMovaSrc0;
        VIR_Swizzle             src_swizzle = VIR_SWIZZLE_INVALID;
        gctUINT                 indexSymId;
        VIR_Symbol              *indexSym;
        VIR_Type                *pOpndType = gcvNULL;
        VIR_OperandInfo         operandInfo;

        pBaseLR = gcvNULL;

        if (opcode == VIR_OP_LDARR)
        {
            pOpnd = VIR_Inst_GetSource(pInst, 1);

            /* When register spill is enable, try to get the base LR. */
            if (bRegisterSpill)
            {
                webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, VIR_Inst_GetSource(pInst, 0));
                if (webIdx != VIR_INVALID_WEB_INDEX)
                {
                    pBaseLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                }
                else
                {
                    pBaseLR = gcvNULL;
                }
            }
        }
        else
        {
            pOpnd = VIR_Inst_GetSource(pInst, 0);

            /* When register spill is enable, try to get the base LR. */
            if (bRegisterSpill)
            {
                defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
                if (defIdx != VIR_INVALID_DEF_INDEX)
                {
                    pBaseLR = _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
                }
                else
                {
                    pBaseLR = gcvNULL;
                }
            }
        }

        webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pOpnd);
        /* assume LDARR/STARR dst, base, immediate is already simplified*/
        gcmASSERT(webIdx != VIR_INVALID_WEB_INDEX);

        pMovaLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);

        /* If the base operand is spilled, then it loads the data from the memory, and it doesn't need a A0/B0 register. */
        if ((pBaseLR == gcvNULL || !isLRSpilled(pBaseLR))
            &&
            _VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pMovaLR)))
        {
            curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, VIR_Shader_isDual16Mode(pShader), 0, -1, gcvNULL);

            if (_VIR_RA_LS_IsInvalidColor(curColor))
            {
                /* Clear the used color from the active LRs, then try to find the new color again. */
                bClearUsedColor = _VIR_RA_LS_ClearUsedColorFromActiveLR(pRA, webIdx);

                if (bClearUsedColor)
                {
                    curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, VIR_Shader_isDual16Mode(pShader), 0, -1, gcvNULL);
                }

                if(_VIR_RA_LS_IsInvalidColor(curColor))
                {
                    retValue = VSC_RA_ERR_OUT_OF_A0B0_REG_FAIL;
                    return retValue;
                }
            }

            pMovaLR->u1.color = curColor;

            retValue = _VIR_RA_LS_SetUsedColorForLR(pRA, pMovaLR, gcvTRUE, 0);
            CHECK_ERROR(retValue, "set used color for LR");

            /* insert a MOVA instruction here */
            retValue = VIR_Function_AddInstructionBefore(pFunc,
                            VIR_OP_MOVA,
                            VIR_Operand_GetTypeId(pOpnd),
                            pInst,
                            gcvTRUE,
                            &pNewInst);

            vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, pOpnd, gcvFALSE, gcvFALSE);
            for(pDef = vscVIR_GeneralUdIterator_First(&udIter);
                pDef != gcvNULL;
                pDef = vscVIR_GeneralUdIterator_Next(&udIter))
            {
                gcmASSERT(VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_MOVA);

                if (pDefInst == gcvNULL)
                {
                    pDefInst = pDef->defKey.pDefInst;
                }

                if (pDef->defKey.pDefInst != pDefInst)
                {
                    gcmASSERT(gcvFALSE);
                }
            }

            pNewOpnd = VIR_Inst_GetDest(pNewInst);
            VIR_Operand_Copy(pNewOpnd, VIR_Inst_GetDest(pDefInst));
            pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
            VIR_Operand_Copy(pNewOpnd, VIR_Inst_GetSource(pDefInst, 0));

            VIR_Operand_GetOperandInfo(
                pDefInst,
                VIR_Inst_GetSource(pDefInst, 0),
                &operandInfo);

            /* extend the live range of MOVA src0 to here*/
            webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pDefInst, VIR_Inst_GetSource(pDefInst, 0));
            if (VIR_INVALID_WEB_INDEX != webIdx)
            {
                VIR_WEB *pWeb = GET_WEB_BY_IDX(&pDuInfo->webTable, webIdx);

                pMovaSrc0 = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                if (pMovaSrc0->endPoint < (gctUINT) (VIR_Inst_GetId(pInst) + 1))
                {
                    pMovaSrc0->endPoint = VIR_Inst_GetId(pInst) + 1;
                }

                defIdx = pWeb->firstDefIdx;
                while (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                    if (VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pNewOpnd)) & (1 << pDef->defKey.channel))
                    {
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                        pDef->defKey.pDefInst,
                                        pNewInst,
                                        pNewOpnd,
                                        gcvFALSE,
                                        operandInfo.u1.virRegInfo.startVirReg,
                                        operandInfo.u1.virRegInfo.virRegCount,
                                        (1 << pDef->defKey.channel),
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
                    }

                    defIdx = pDef->nextDefInWebIdx;
                }
            }

            if (VIR_Shader_isDual16Mode(pShader))
            {
                VIR_Inst_SetThreadMode(pNewInst, VIR_THREAD_D16_DUAL_32);
            }

            _VIR_RA_MakeColor(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
                _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);

            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pMovaLR)))
            {
                _VIR_RA_MakeColorHI(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
                    _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);
            }

            _VIR_RA_LS_SetOperandHwRegInfo(pRA, VIR_Inst_GetDest(pNewInst), curColor);
        }

        _VIR_RA_MakeColor(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
            _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);
        if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pMovaLR)))
        {
            _VIR_RA_MakeColorHI(isLRB0(pMovaLR) ? VIR_SR_B0 : VIR_SR_A0,
                _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pMovaLR)), &curColor);
        }

        _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);

        /* get movea LR's shift to put into src0 index_shift */
        src_swizzle = VIR_Operand_GetSwizzle(pOpnd);
        src_swizzle = src_swizzle & 0x3;

        /* relIndex is sym id */
        indexSym  = VIR_Operand_GetSymbol(pOpnd);
        indexSymId  = VIR_Operand_GetSymbolId_(pOpnd);

        _VIR_RA_LS_SetSymbolHwRegInfo(pRA, indexSym, pMovaLR, 0);

        if (opcode == VIR_OP_LDARR)
        {
            VIR_Operand_SetRelIndexing(VIR_Inst_GetSource(pInst, 0), indexSymId);
            VIR_Operand_SetRelAddrMode(VIR_Inst_GetSource(pInst, 0),
                                       (_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMovaLR)) + src_swizzle + 1));
            pOpndType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(VIR_Inst_GetSource(pInst, 0)));
            VIR_Operand_SetTypeId(VIR_Inst_GetSource(pInst, 0), VIR_Type_GetBaseTypeId(pOpndType));
        }
        else
        {
            VIR_Operand_SetRelIndexing(VIR_Inst_GetDest(pInst), indexSymId);
            VIR_Operand_SetRelAddrMode(VIR_Inst_GetDest(pInst),
                                        (_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMovaLR)) + src_swizzle + 1));
            pOpndType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst)));
            VIR_Operand_SetTypeId(VIR_Inst_GetDest(pInst), VIR_Type_GetBaseTypeId(pOpndType));
        }
    }

    _VIR_RA_LS_ExpireActiveLRs(pRA, VIR_Inst_GetId(pInst));

    return retValue;
}

VSC_ErrCode
_VIR_RA_LS_AssignColorForA0B0(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "============== Assign color for A0/B0 registers ==============\n");
        VIR_LOG_FLUSH(pDumper);
    }

    VIR_Shader_SetCurrentFunction(VIR_RA_LS_GetShader(pRA), pFunc);

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    for (pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
         pInst != gcvNULL;
         pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        retValue = _VIR_RA_LS_AssignColorForA0B0Inst(pRA, pFunc, pInst);
        CHECK_ERROR(retValue, "Fail to assign A0/B0 color instruction.");

        /* No need to check the left instructions when we need to re-color. */
        if (VIR_RA_LS_GetDisableDual16AndRecolor(pRA))
        {
            return retValue;
        }
    }

    return retValue;
}

/* ===========================================================================
   VIR_RA_LS_PerformOnFunction_Pre
   linear scan register allocation on function
   we can't insert any new instruction in this function!!!!!!
   ===========================================================================
*/
static VSC_ErrCode _VIR_RA_LS_PerformOnFunction_Pre(
    VIR_RA_LS       *pRA,
    gctUINT         reservedDataReg)
{
    VSC_ErrCode             retValue  = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function            *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper              *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_CONTROL_FLOW_GRAPH  *pCfg = VIR_Function_GetCFG(pFunc);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "\nPre Processing function:\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    /* build live ranges in LRTable for pFunc */
    retValue = _VIR_RA_LS_BuildLRTable(pRA, pFunc);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "\nCFG with Liveness Information:\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_CFG_LIVENESS_Dump(pDumper, pLvInfo, pCfg);
        VIR_LOG_FLUSH(pDumper);
    }

    /* sort live ranges in LRTable for pFunc */
    retValue = _VIR_RA_LS_SortLRTable(pRA, pFunc);
    CHECK_ERROR(retValue, "_VIR_RA_LS_SortLRTable");

    /* compute weight for each LR */
    _VIR_RA_LS_computeWeight(pRA, pFunc);

    /* Assign the colors for the generate registers. */
    retValue = _VIR_RA_LS_AssignColorsForGeneralReg(pRA, pFunc, reservedDataReg);
    CHECK_ERROR(retValue, "Fail to assign colors for the generate registers.");

    return retValue;
}

/* ===========================================================================
   VIR_RA_LS_PerformOnFunction_Post
   post linear scan register allocation on function
   we can insert any new instruction in this function.
   ===========================================================================
*/
static VSC_ErrCode _VIR_RA_LS_PerformOnFunction_Post(
    VIR_RA_LS       *pRA,
    gctUINT         reservedDataReg
    )
{
    VSC_ErrCode             retValue  = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function            *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper              *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "\nPost Processing function:\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    /* I: Assign colors for A0/B0. */
    retValue = _VIR_RA_LS_AssignColorForA0B0(pRA, pFunc);
    CHECK_ERROR(retValue, "Fail to assign colors for the A0/B0 registers.");

    /* No need to check the left instructions when we need to re-color. */
    if (VIR_RA_LS_GetDisableDual16AndRecolor(pRA))
    {
        return retValue;
    }

    return retValue;
}

/* ===========================================================================
   _VIR_RA_LS_MovHWInputRegisters
   generate mov rx, rx for HW generated inputs to ease HW debug
   ===========================================================================
*/
VSC_ErrCode
_VIR_RA_LS_PrependMOV(
    VIR_RA_LS           *pRA,
    VIR_Function        *pFunc,
    VIR_TypeId          elementTypeId,
    VIR_RA_HWReg_Color  curColor,
    VIR_Swizzle         attrSwizzle,
    VIR_Enable          attrEnable,
    VIR_Instruction     **newInst)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_SymId           symId = VIR_INVALID_ID;

    retValue = VIR_Function_PrependInstruction(pFunc,
                VIR_OP_MOV,
                elementTypeId,
                newInst);
    if (retValue != VSC_ERR_NONE)
    {
        return retValue;
    }

    _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister((*newInst)->src[VIR_Operand_Src0],
                                pFunc,
                                symId,
                                elementTypeId);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle((*newInst)->src[VIR_Operand_Src0], attrSwizzle);

    VIR_Operand_SetTempRegister((*newInst)->dest,
                                pFunc,
                                symId,
                                elementTypeId);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->dest, curColor);
    VIR_Operand_SetEnable((*newInst)->dest, attrEnable);

    return retValue;
}

VSC_ErrCode
_VIR_RA_LS_MovHWInputRegisters(
    VIR_RA_LS           *pRA,
    VIR_Shader          *pShader)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Function        *pFunc = VIR_Shader_GetMainFunction(pShader);

    VIR_Instruction     *newInst = gcvNULL;
    VIR_RA_HWReg_Color  curColor;

    curColor = InvalidColor;

    /* based on different shader type, the dump is different */
    if (pShader->shaderKind == VIR_SHADER_FRAGMENT ||
        pShader->shaderKind == VIR_SHADER_VERTEX ||
        pShader->shaderKind == VIR_SHADER_COMPUTE)
    {
        gctUINT             inputIdx;

        for (inputIdx = 0;
            inputIdx < VIR_IdList_Count(VIR_Shader_GetAttributes(pShader));
            inputIdx ++)
        {
            VIR_Symbol      *pAttrSym = VIR_Shader_GetSymFromId(pShader,
                                            VIR_IdList_GetId(VIR_Shader_GetAttributes(pShader), inputIdx));
            VIR_TypeId      elementTypeId = _VIR_RA_LS_GenBaseTypeID(pShader, pAttrSym);
            gctUINT         hwStartReg = VIR_Symbol_GetHwRegId(pAttrSym);
            gctUINT         regIdx = 0;

            /* PS should not have mov r0, r0
                or mov r1, r1 in dual16 highp mode */
            if ((pShader->shaderKind == VIR_SHADER_FRAGMENT) &&
                (_VIR_RS_LS_IsSpecialReg(hwStartReg) ||
                 (hwStartReg == 0) ||
                 ((hwStartReg == 1) && VIR_Shader_isDual16Mode(pShader))))
            {
                continue;
            }

            for (regIdx = 0; regIdx < VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pAttrSym), -1); regIdx++)
            {
                gctUINT         hwReg = hwStartReg + regIdx;
                VIR_Enable      attrEnable = VIR_ENABLE_NONE;
                VIR_Swizzle     attrSwizzle = VIR_SWIZZLE_INVALID;

                _VIR_RA_LS_GetSym_Enable_Swizzle(pAttrSym, &attrEnable, &attrSwizzle);
                _VIR_RA_MakeColor(hwReg, VIR_Symbol_GetHwShift(pAttrSym), &curColor);

                retValue = _VIR_RA_LS_PrependMOV(pRA, pFunc, elementTypeId,
                    curColor, attrSwizzle, attrEnable, &newInst);
                CHECK_ERROR(retValue, "_VIR_RA_LS_PrependMOV");
            }
        }
    }
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
            pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION ||
            pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        gctINT  inputCount, outputCount, regIdx, lastRegNum = 0;

        if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
        {
            /* For TCS
               r0.x remap addr for output vertex
               r0.y InvocationID
               r0.z primitiveID
               r0.w per-patch addr
               r1 - rx remap for input vertex
               r(x+1) - ry remap for output vertex (if there is read of output vertex)
           */
            inputCount = ((pShader->shaderLayout.tcs.tcsPatchInputVertices - 1) / 8 + 1);
            outputCount = ((pShader->shaderLayout.tcs.tcsPatchOutputVertices - 1) / 8 + 1);

            if (VIR_Shader_TCS_UsePackedRemap(pShader))
            {
                /* remap is packed in r1 */
                lastRegNum  = 1;
            }
            else
            {
                lastRegNum = inputCount + (pShader->shaderLayout.tcs.hasOutputVertexAccess ? outputCount : 0);
            }
        }
        else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
        {
            /* For TES
               r0.x Tess.Coordinate u
               r0.y Tess.Coordinate v
               r0.z Tess.Coordinate w
               r0.w output vertex addr and per-patch addr
               r1-rx remap for input vertex
            */
            inputCount = ((pShader->shaderLayout.tes.tessPatchInputVertices -1) / 8 + 1);
            lastRegNum  = inputCount;
        }
        else if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
        {
            /* For GS
              r0.x meta data address and current vertex address
              r0.y vertex counter
              r0.z Instance ID (if no restart or stream out and has instance id)
                   Primitive ID (if no restart or stream out and has Primitive id)
                   Restart flags and Stream out indices (if has restart or stream out)
              r0.w instance ID (if has restart or stream out and instance id)
                   Primitive ID (if has restart or stream out and Primitive id)
              r1.x Primitive ID (if has restart or stream out, instance id and Primitive id)
              r0.z, r1.x, remap for input
              r1.z, r2.x */

            if (!VIR_Shader_HasRestartOrStreamOut(pShader) &&
                !VIR_Shader_HasInstanceId(pShader) &&
                !VIR_Shader_HasPrimitiveId(pShader))
            {
                /* points 1 vertex, lines 2 vertices, line adjacency 4 vertices,
                   triangles 3 vertices, triangle adjacency 6 vertices;
                   one components can fit 2 remap */
                if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
                {
                    lastRegNum = 1;
                }
                else
                {
                    lastRegNum = 0;
                }
            }
            else if (VIR_Shader_HasRestartOrStreamOut(pShader) &&
                      VIR_Shader_HasInstanceId(pShader) &&
                      VIR_Shader_HasPrimitiveId(pShader))
            {
                if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
                {
                    lastRegNum = 2;
                }
                else
                {
                    lastRegNum = 1;
                }
            }
            else
            {
                lastRegNum = 1;
            }
        }
        /*  mov rx, rx */
        for (regIdx = lastRegNum; regIdx >= 0; regIdx --)
        {
            _VIR_RA_MakeColor(regIdx, 0, &curColor);
            retValue = _VIR_RA_LS_PrependMOV(pRA, pFunc, VIR_TYPE_FLOAT_X4,
                        curColor, VIR_SWIZZLE_XYZW, VIR_ENABLE_XYZW, &newInst);
            CHECK_ERROR(retValue, "_VIR_RA_LS_PrependMOV");
        }
    }

    return retValue;
}

/* ===========================================================================
   _VIR_RA_LS_SetReservedReg
   set the shader's reserved baseRegister and dataRegister for spills
   ===========================================================================
*/
void _VIR_RA_LS_SetReservedReg(
    VIR_RA_LS           *pRA)
{
    VIR_Shader      *pShader = VIR_RA_LS_GetShader(pRA);
    gctUINT         hwRegCount = 0, i = 0;

    hwRegCount = VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxAllocReg + 1;

    if (pShader->hasRegisterSpill)
    {
        /* set the baseRegister and dataRegister */
        for (i = 0; i < pRA->resDataRegisterCount; i++)
        {
            pRA->dataRegister[i] = hwRegCount + i;
            gcmASSERT(pRA->dataRegister[i] < VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg);
        }
        pRA->baseRegister = hwRegCount + pRA->resDataRegisterCount;
        gcmASSERT(pRA->baseRegister < VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg);

        for (i = 0; i < pRA->movaRegCount; i++)
        {
            pRA->movaRegister[i] = pRA->baseRegister + 1 + i;
            gcmASSERT(pRA->movaRegister[i] < VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg);
        }
    }
}

void _VIR_RA_LS_UpdateWorkgroupIdAndBaseAddr(
    VIR_RA_LS   *pRA,
    VIR_Shader  *pShader,
    gctUINT     numWorkGroup)
{
    VIR_Function    *mainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_InstIterator inst_iter;
    VIR_Instruction *inst;
    VIR_Operand     *indexOpnd = gcvNULL;
    VIR_Operand     *localAddrOpnd = gcvNULL;
    VIR_Symbol      *localAddrSym = gcvNULL;
    gctBOOL         updated = gcvFALSE;
    gctBOOL         isOCL = VIR_Shader_IsCL(pShader);

    VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(mainFunc));
    for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
         inst != gcvNULL;
         inst = (VIR_Instruction *)VIR_InstIterator_Next(&inst_iter))
    {
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_IMOD)
        {
            if (VIR_Operand_isSymbol(VIR_Inst_GetSource(inst, VIR_Operand_Src1)))
            {
                VIR_Symbol  *sym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(inst, VIR_Operand_Src1));

                if (sym &&
                    VIR_Symbol_isUniform(sym) &&
                    strcmp(VIR_Shader_GetSymNameString(pShader, sym), _sldWorkGroupCountName) == 0)
                {
                    VIR_Operand_SetImmediateInt(inst->src[VIR_Operand_Src1], numWorkGroup);

                    /* Mark workGroupCount uniform as inactive. */
                    VIR_Symbol_ClrFlag(sym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                }
            }
        }
        else if (VIR_Inst_GetOpcode(inst) == VIR_OP_IMADLO0 || VIR_Inst_GetOpcode(inst) == VIR_OP_ADD)
        {
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_IMADLO0)
            {
                indexOpnd = VIR_Inst_GetSource(inst, 0);
                localAddrOpnd = VIR_Inst_GetSource(inst, VIR_Operand_Src2);
                localAddrSym = VIR_Operand_GetSymbol(localAddrOpnd);
            }
            else
            {
                indexOpnd = gcvNULL;
                localAddrOpnd = VIR_Inst_GetSource(inst, VIR_Operand_Src1);
                localAddrSym = VIR_Operand_GetSymbol(localAddrOpnd);
            }

            if (localAddrSym &&
                VIR_Symbol_isUniform(localAddrSym) &&
                ((isOCL && strcmp(VIR_Shader_GetSymNameString(pShader, localAddrSym), _sldLocalStorageAddressName) == 0) ||
                 (!isOCL && strcmp(VIR_Shader_GetSymNameString(pShader, localAddrSym), _sldSharedVariableStorageBlockName) == 0)))
            {
                if (numWorkGroup == 1 && indexOpnd)
                {
                    VIR_Operand_SetImmediateUint(indexOpnd, 0);
                }

                /* Change the base address of local storage to 0. */
                VIR_Operand_SetImmediateUint(localAddrOpnd, 0);

                /* Mark local address uniform as inactive. */
                VIR_Symbol_ClrFlag(localAddrSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
                VIR_Symbol_SetFlag(localAddrSym, VIR_SYMFLAG_INACTIVE);

                updated = gcvTRUE;
                break;
            }
        }
    }

    if (!updated)
    {
        /* we could not find the instruction to compute the local base address */
        gcmASSERT(gcvFALSE);
    }
}

/* ===========================================================================
   _VIR_RA_LS_UpdateWorkgroupNum
   Update workGroupNum for CS.
   ===========================================================================
*/
void _VIR_RA_LS_UpdateWorkgroupNum(
    VIR_RA_LS   *pRA,
    VIR_Shader  *pShader,
    gctUINT     numWorkGroup)
{
    VIR_Function    *mainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_InstIterator inst_iter;
    VIR_Instruction *inst;
    gctUINT16       i;
    gctUINT16       maxMatchCount = 0, count = 0;

    for (i = 0; i < 3; i++)
    {
        maxMatchCount = VIR_Shader_GetWorkGroupSizeFactor(pShader, i);

        if (maxMatchCount != 0)
        {
            break;
        }
    }

    if (maxMatchCount == 0)
    {
        maxMatchCount = 1;
    }

    VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(mainFunc));
    for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
        inst != gcvNULL;
        inst = (VIR_Instruction *)VIR_InstIterator_Next(&inst_iter))
    {
        gctBOOL     bMatch = gcvFALSE;

        if (VIR_Inst_GetOpcode(inst) == VIR_OP_IMOD)
        {
            VIR_Operand *dest = VIR_Inst_GetDest(inst);
            VIR_Symbol  *sym = VIR_Operand_GetUnderlyingSymbol(dest);

            if (sym)
            {
                gctSTRING name = VIR_Shader_GetSymNameString(pShader, sym);

                if (gcoOS_StrNCmp(name, _sldWorkGroupIdName, sizeof(_sldWorkGroupIdName)) == 0)
                {
                    bMatch = gcvTRUE;
                }
            }

            if (!bMatch && VIR_Operand_isImm(VIR_Inst_GetSource(inst, 1)))
            {
                gctUINT specialValue = VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(inst, 1));

                if (specialValue == __INIT_VALUE_FOR_WORK_GROUP_INDEX__)
                {
                    bMatch = gcvTRUE;
                }
            }

            if (bMatch)
            {
                VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(inst, 1), numWorkGroup);
                count++;

                if (count == maxMatchCount)
                {
                    break;
                }
            }
        }
    }

    if (count != maxMatchCount)
    {
        if (count != 0 && maxMatchCount != 1)
        {
            gcmASSERT(gcvFALSE);
        }
        /* If the shared variable is not used in the shader, we could not find the
        mod instruction to change */
    }
}

void _VIR_RA_LS_ChangeLocalToGlobal(
    VIR_Shader *pShader)
{
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction  *inst;
        VIR_OpCode       opc;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            opc = VIR_Inst_GetOpcode(inst);

            switch (opc)
            {
            case VIR_OP_LOAD_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_LOAD);
                break;
            case VIR_OP_STORE_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_STORE);
                break;
            case VIR_OP_ATOMADD_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMADD);
                break;
            case VIR_OP_ATOMSUB_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMSUB);
                break;
            case VIR_OP_ATOMCMPXCHG_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMCMPXCHG);
                break;
            case VIR_OP_ATOMMAX_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMMAX);
                break;
            case VIR_OP_ATOMMIN_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMMIN);
                break;
            case VIR_OP_ATOMOR_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMOR);
                break;
            case VIR_OP_ATOMAND_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMAND);
                break;
            case VIR_OP_ATOMXOR_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMXOR);
                break;
            case VIR_OP_ATOMXCHG_L:
                VIR_Inst_SetOpcode(inst, VIR_OP_ATOMXCHG);
                break;
            default:
                break;
            }
        }
    }
}

void _VIR_RA_LS_SetRegWatermark(
    VIR_RA_LS           *pRA)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_Dumper          *pDumper = VIR_RA_LS_GetDumper(pRA);
    gctUINT             hwRegCount = 0;
    VSC_HW_CONFIG       *pHwCfg = VIR_RA_LS_GetHwCfg(pRA);
    gctUINT             numWorkGroup = 1, numWorkThread = 1;
    gctBOOL             numWorkGroupChanged = gcvFALSE;

    /* set register allocated to shader */
    VIR_Shader_SetRegAllocated(pShader, gcvTRUE);

    hwRegCount = VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxAllocReg + 1;

    if (pShader->hasRegisterSpill)
    {
        /* spill will reserve some registers */
        hwRegCount = hwRegCount + pRA->movaRegCount + 1 + pRA->resDataRegisterCount;
    }

    if (_VIR_RA_isShaderNeedSampleDepth(pRA))
    {
        /* the last temp as sampleDepth */
        hwRegCount += (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
        pShader->sampleMaskIdRegStart = hwRegCount - 1;
    }

    gcmASSERT(hwRegCount <= VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg);

    /* for performance experiment purpose only: set register water mark from commondline,
       must be a valid number */
    if (VSC_OPTN_RAOptions_GetRegWaterMark(pOption) > hwRegCount &&
        VSC_OPTN_RAOptions_GetRegWaterMark(pOption) < VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg)
    {
        hwRegCount = VSC_OPTN_RAOptions_GetRegWaterMark(pOption);
    }

    VIR_Shader_SetRegWatermark(pShader, hwRegCount);

    /* Calculate the concurrent workGroupNumber and workThreadNumber first. */
    if (VIR_Shader_IsCL(pShader) || VIR_Shader_IsGlCompute(pShader))
    {
        /* Set the work group number. */
        numWorkGroup = VIR_Shader_ComputeWorkGroupNum(pShader, pHwCfg);
        VIR_Shader_SetCurrWorkGrpNum(pShader, numWorkGroup);

        /* Set the work thread number. */
        numWorkThread = VIR_Shader_ComputeWorkThreadNum(pShader, pHwCfg);
        VIR_Shader_SetCurrWorkThreadNum(pShader, numWorkThread);

        /* Set the workGroupCount per shader group. */
        VIR_Shader_SetWorkGroupNumPerShaderGroup(pShader, VIR_Shader_ComputeWorkGroupNumPerShaderGroup(pShader, pHwCfg));

        /* Update the work group number for CS first. */
        if (!VIR_Shader_IsUseHwManagedLS(pShader) &&
            VIR_Shader_GetShareMemorySize(pShader) > 0 && VIR_Shader_IsGlCompute(pShader))
        {
            _VIR_RA_LS_UpdateWorkgroupNum(pRA, pShader, numWorkGroup);
        }
    }

    /* Check if local storage can be enabled:
        1) For those chips can't support PSCS throttle, enable local storage only when temp register meets local storage requirement.
        2) For the other chips, just enable it, HW can choose the minimal value.
    */
    if (VIR_Shader_UseLocalMem(pShader) && !VIR_Shader_IsUseHwManagedLS(pShader))
    {
        gctUINT localMemorySize = VIR_Shader_GetShareMemorySize(pShader);

        gcmASSERT(VIR_Shader_IsCL(pShader) || VIR_Shader_IsGlCompute(pShader));

        if (localMemorySize  == 0)
        {
            VIR_Shader_ClrFlag(pShader, VIR_SHFLAG_USE_LOCAL_MEM);
        }
        else
        {
            gctUINT localMemoryReq = pHwCfg->maxLocalMemSizeInByte / localMemorySize;

            if (pHwCfg->hwFeatureFlags.supportPSCSThrottle)
            {
                if (localMemoryReq < numWorkGroup)
                {
                    numWorkGroup = localMemoryReq;
                    numWorkGroupChanged = gcvTRUE;
                }
            }

            /* (maxFreeReg / hwRegCount) * threadCount <= localMemoryReq * workgroupSize */
            if (numWorkGroup <= localMemoryReq &&
                (!VIR_Shader_UseLocalMemAtom(pShader) || pHwCfg->hwFeatureFlags.supportLSAtom))
            {
                /* Add imod for computing workgroupId. */
                _VIR_RA_LS_UpdateWorkgroupIdAndBaseAddr(pRA, pShader, numWorkGroup);

                /* We may need to update workgroupNum. */
                if (numWorkGroupChanged)
                {
                    VIR_Shader_SetCurrWorkGrpNum(pShader, numWorkGroup);
                    if (VIR_Shader_IsGlCompute(pShader))
                    {
                        _VIR_RA_LS_UpdateWorkgroupNum(pRA, pShader, numWorkGroup);
                    }
                }
            }
            else
            {
                _VIR_RA_LS_ChangeLocalToGlobal(pShader);
                VIR_Shader_ClrFlag(pShader, VIR_SHFLAG_USE_LOCAL_MEM);
            }
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_WATERMARK))
    {
        VIR_LOG(pDumper, "================ shader (id:%d) %d register used ================\n",
            VIR_Shader_GetId(pShader), hwRegCount);
        VIR_LOG_FLUSH(pDumper);
    }
}

/* ===========================================================================
   _VIR_RA_SetInputOutputFlag
   When opt is not ready, it is possible that outputs are expected to put into USC,
   but they are not used by any inst. For these outputs, we should also
   set LOAD_STORE_ATTR flag
   ===========================================================================
*/
void _VIR_RA_SetInputOutputFlag(
    VIR_RA_LS           *pRA)
{
    VIR_Shader      *pShader = VIR_RA_LS_GetShader(pRA);
    gctUINT         outputIdx, inputIdx;

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        for (outputIdx = 0;
             outputIdx < VIR_IdList_Count(VIR_Shader_GetOutputs(pShader));
             outputIdx ++)
        {
            VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader), outputIdx));

            if (VIR_Symbol_GetHwFirstCompIndex(pOutputSym) != NOT_ASSIGNED && !isSymLoadStoreAttr(pOutputSym))
            {
                VIR_Symbol_SetFlag(pOutputSym, VIR_SYMFLAG_LOAD_STORE_ATTR);
            }
        }

        for (outputIdx = 0;
             outputIdx < VIR_IdList_Count(VIR_Shader_GetPerpatchOutputs(pShader));
             outputIdx ++)
        {
            VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetPerpatchOutputs(pShader), outputIdx));

            if (VIR_Symbol_GetHwFirstCompIndex(pOutputSym) != NOT_ASSIGNED && !isSymLoadStoreAttr(pOutputSym))
            {
                VIR_Symbol_SetFlag(pOutputSym, VIR_SYMFLAG_LOAD_STORE_ATTR);
            }
        }
    }

    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION ||
        pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        /* When opt is not ready, it is possible that inputs are expected to put into USC, but
            they are not used by any inst. For these inputs, we should also set LOAD_STORE_ATTR
            flag */
        for (inputIdx = 0;
             inputIdx < VIR_IdList_Count(VIR_Shader_GetAttributes(pShader));
             inputIdx ++)
        {
            VIR_Symbol* pAttrSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetAttributes(pShader), inputIdx));

            if (VIR_Symbol_GetHwFirstCompIndex(pAttrSym) != NOT_ASSIGNED && !isSymLoadStoreAttr(pAttrSym))
            {
                VIR_Symbol_SetFlag(pAttrSym, VIR_SYMFLAG_LOAD_STORE_ATTR);
            }
        }

        for (inputIdx = 0;
             inputIdx < VIR_IdList_Count(VIR_Shader_GetPerpatchAttributes(pShader));
             inputIdx ++)
        {
            VIR_Symbol* pAttrSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetPerpatchAttributes(pShader), inputIdx));

            if (VIR_Symbol_GetHwFirstCompIndex(pAttrSym) != NOT_ASSIGNED && !isSymLoadStoreAttr(pAttrSym))
            {
                VIR_Symbol_SetFlag(pAttrSym, VIR_SYMFLAG_LOAD_STORE_ATTR);
            }
        }
    }
}

void
_VIR_RA_LS_WriteDebugInfo(
    VIR_RA_LS           *pRA)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_LS_Liverange *pLR;
    gctUINT             webIdx;
    VSC_DI_SW_LOC       SWLoc;
    VSC_DI_HW_LOC       HWLoc;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;
    VIR_Function        *pFunc;
    gctUINT             instCount, totalInstCount = 0;


    gcmASSERT(pRA->DIContext);

    /* write tmp debug */
    if (gcmOPT_EnableDebugDump())
        gcmPRINT("------------hwLoc alloc for tmp reg----------------");

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        pFunc = func_node->function;
        instCount = VIR_Function_GetInstCount(pFunc);
        totalInstCount += instCount;


        for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
        {
            pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);

            if (pLR->colorFunc != pFunc)
            {
                continue;
            }

            /* RA assigns register/memory for all temps */
            SWLoc.reg = gcvTRUE;
            SWLoc.u.reg.start = (gctUINT16)pLR->firstRegNo;
            SWLoc.u.reg.end = (gctUINT16)(pLR->firstRegNo + pLR->regNoRange - 1);

            /* do not set PC range here */
            HWLoc.beginPC = 0;
            HWLoc.endPC = 0;
            HWLoc.next = VSC_DI_INVALID_HW_LOC;

            if (isLRSpilled(pLR))
            {
                HWLoc.reg = gcvFALSE;
                HWLoc.u.offset.baseAddr.type = VSC_DIE_HW_REG_TMP;
                HWLoc.u.offset.baseAddr.start = (unsigned short) pRA->baseRegister; /* base address register */
                HWLoc.u.offset.baseAddr.end = (unsigned short) pRA->baseRegister;
                HWLoc.u.offset.offset = (gctUINT16)_VIR_RA_GetLRSpillOffset(pLR);
                HWLoc.u.offset.endOffset = HWLoc.u.offset.offset + (gctUINT16)pLR->regNoRange * __DEFAULT_TEMP_REGISTER_SIZE_IN_BYTE__;
            }
            else
            {
                HWLoc.reg = gcvTRUE;
                HWLoc.u.reg.type = VSC_DIE_HW_REG_TMP;
                HWLoc.u.reg.start = (gctUINT16)pLR->u1.color._hwRegId;
                HWLoc.u.reg.end = (gctUINT16)(pLR->u1.color._hwRegId + pLR->regNoRange - 1);
                HWLoc.u1.hwShift = pLR->u1.color._hwShift;
            }

            vscDISetHwLocToSWLoc(pRA->DIContext, &SWLoc, &HWLoc);
        }
    }

    /* move this to MC gen */

}

/* scan instructions and replace base operand with ra->baseRegister */
static VSC_ErrCode
_VIR_RA_UpdateBaseOperOfSpecialLoadStore(
    VIR_RA_LS           *pRA)
{

    VIR_Shader        *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VSC_ErrCode       retErrCode = VSC_ERR_NONE;

    /* initialize baseAddrSymId if needed */
    if (pRA->baseAddrSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->baseAddrSymId);
    }
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction  *inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
            if (opcode == VIR_OP_LOAD_S || opcode == VIR_OP_STORE_S ||
                opcode == VIR_OP_MOV    || opcode == VIR_OP_ADD)
            {
                /* if base address src0 is sym "TempRegSpillMemAddr", replace with ra->baseAddrSymId */
                VIR_Operand *src0 = inst->src[VIR_Operand_Src0];
                if (VIR_Operand_isSymbol(src0) &&(VIR_Operand_isSymbol(src0)))
                {
                    VIR_Symbol* sym = VIR_Operand_GetSymbol(src0);
                    if (VIR_Symbol_isUniform(sym) &&
                        (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS))
                    {
                        VIR_Operand_SetTempRegister(inst->src[VIR_Operand_Src0],
                                                    func,
                                                    pRA->baseAddrSymId,
                                                    VIR_TYPE_FLOAT_X4);
                    }
                }
            }
        }
    }

    return retErrCode;
}

static void
_VIR_RA_ClearColorPool(
    VIR_RA_LS           *pRA,
    gctBOOL             bEnableSpill,
    gctUINT             reservedDataReg,
    gctBOOL             bReset)
{
    VIR_Shader          *pShader = pRA->pShader;

    /* clear color pool */
    _VIR_RA_ColorPool_ClearAll(VIR_RA_LS_GetColorPool(pRA));
    _VIR_RA_LRTable_ClearColor(pRA);
    VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxAllocReg = 0;
    VIR_RA_LS_GetActiveLRHead(pRA)->nextActiveLR = &(LREndMark);
    (pRA)->resDataRegisterCount = reservedDataReg;
    pShader->hasRegisterSpill |= bEnableSpill;
    (pRA)->spillOffset = (pShader->vidmemSizeOfSpill + 15) & ~ (gctUINT)0x0F;

    if (bReset)
    {
        pShader->hasRegisterSpill = gcvFALSE;
    }

    /* Disable DUAL16 temporarily if registerSpill is used. */
    if (pShader->hasRegisterSpill)
    {
        VIR_Shader_SetDual16Mode(pShader, gcvFALSE);
    }

    (pRA)->currentMaxGRCount = VIR_RA_LS_REG_MAX;
    _VIR_RA_FlaseDepReg_ClearAll(pRA);
    VIR_RA_LS_SetDisableDual16AndRecolor(pRA, gcvFALSE);
}

DEF_QUERY_PASS_PROP(VIR_RA_LS_PerformTempRegAlloc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCg = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedWeb = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedLvFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VIR_RA_LS_PerformTempRegAlloc)
{
    return gcvTRUE;
}

/* ===========================================================================
   VIR_RA_LS_PerformTempRegAlloc
   linear scan register allocation on shader
   ===========================================================================
*/
VSC_ErrCode VIR_RA_LS_PerformTempRegAlloc(
    IN VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG       *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_LIVENESS_INFO   *pLvInfo = pPassWorker->pLvInfo;
    VSC_OPTN_RAOptions  *pOption = (VSC_OPTN_RAOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VIR_CALL_GRAPH      *pCG = pPassWorker->pCallGraph;
    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);
    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;
    VIR_RA_LS           ra;
    gctUINT             reservedDataReg = 0;
    gctBOOL             needBoundsCheck =
                           (pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK) != 0;

    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    gctBOOL             colorSucceed = gcvFALSE;

    if (needBoundsCheck)
    {
        VIR_Shader_SetFlagsExt1(pShader, VIR_SHFLAG_EXT1_ENABLE_ROBUST_CHECK);
    }
    else
    {
        VIR_Shader_ClrFlagExt1(pShader, VIR_SHFLAG_EXT1_ENABLE_ROBUST_CHECK);
    }
    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "========================================================\n");
        VIR_LOG(pDumper, "Linear Scan Register Allocation\n");
        VIR_LOG_FLUSH(pDumper);
    }

    VIR_Shader_RenumberInstId(pShader);

    _VIR_RA_LS_Init(&ra, pShader, pHwCfg, pLvInfo, pOption, pDumper, pCG, pPassWorker->basePassWorker.pMM);
    ra.needBoundsCheck = needBoundsCheck;
    ra.scratchChannel = needBoundsCheck ? VIR_CHANNEL_W : VIR_CHANNEL_Y;

    if (VIR_RA_LS_GetEnableDebug(&ra))
    {
        /* set the reserved data to be 4, could be smarter */
        reservedDataReg = 4;
        pShader->hasRegisterSpill   = gcvTRUE;
        ra.resDataRegisterCount = 4;
        _VIR_RA_LS_SetReservedReg(&ra);
    }
    if (gcmOPT_EnableDebug() || gcmOPT_DisableOPTforDebugger())
    {
        ra.DIContext = (VSC_DIContext *)pShader->debugInfo;
    }

    if (countOfFuncBlk != 0)
    {
        /* dump before */
        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_INPUT))
        {
            VIR_Shader_Dump(gcvNULL, "Shader before Register Allocation", pShader, gcvTRUE);
            VIR_LOG_FLUSH(pDumper);
        }

        /* if long parameter opt applied, need update base operand of load_s/store_s with ra->baseRegister */
        if (pShader->hasRegisterSpill)
        {
            _VIR_RA_UpdateBaseOperOfSpecialLoadStore(&ra);
            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                VSC_OPTN_RAOptions_TRACE))
            {
                VIR_Shader_Dump(gcvNULL, "Shader after UpdateBaseOperandOfSpillLoadStore", pShader, gcvTRUE);
                VIR_LOG_FLUSH(pDumper);
            }
        }

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption),
            VSC_OPTN_RAOptions_ALLOC_REG))
        {
            _VIR_RA_LS_InitializeOpnd(&ra);

            retValue = _VIR_RA_LS_AssignAttributes(&ra);
            ON_ERROR(retValue, "_VIR_RA_LS_AssignAttributes");

            /* RPO travering of the call graph */
            ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(
                                            VIR_RA_LS_GetMM(&ra),
                                            sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);
            vscDG_PstOrderTraversal(&pCG->dgGraph,
                                    VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                                    gcvFALSE,
                                    gcvTRUE,
                                    (VSC_DG_NODE**)ppFuncBlkRPO);

            /* we have a loop here for serveral rounds of assigning color
               first round - no spill (i.e., no reserved register), if not succeed,
               second round - reserve 2 registers (one for base, one for data),
               then check the max spilled src of an instruction, if it is large than assumed one
               third round - reserve n registers (one for base, n-1 for data) ...
               until successful.
               */
            while (!colorSucceed)
            {
                gctUINT     spilledOpndCount = gcvFALSE;
                gctBOOL     reColor = gcvFALSE;
                gctBOOL     bSpillReg = gcvFALSE;

                /* I: build the live range and assign color for the general registers. */
                for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
                {
                    pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
                    VIR_Shader_SetCurrentFunction(pShader, pFunc);
                    retValue = _VIR_RA_LS_PerformOnFunction_Pre(&ra, reservedDataReg);

                    if (retValue == VSC_RA_ERR_OUT_OF_REG_SPILL ||
                        retValue == VSC_RA_ERR_OUT_OF_REG_FAIL)
                    {
                        /*
                        ** If we did expend the end point of any LS, we can try again by disable this optimization without spill,
                        ** it could be the root cause of running out of registers.
                        */
                        if (!pShader->hasRegisterSpill
                            &&
                            VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption), VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT)
                            &&
                            VIR_RA_LS_GetExtendLSEndPoint(&ra))
                        {
                            VSC_OPTN_RAOptions_SetOPTS(pOption, ~VSC_OPTN_RAOptions_MAX_LS_EXTENED_END_POINT & VSC_OPTN_RAOptions_GetOPTS(pOption));
                            VIR_RA_LS_SetExtendLSEndPoint(&ra, gcvFALSE);
                            reColor = gcvTRUE;
                            break;
                        }

                        /*
                        ** We have two solutions for out of register resource
                        ** 1) Register spilling, this is the general solution.
                        ** 2) Reduce the workGroupSize to use more registers, this is for CL/CS only.
                        ** Register spilling will generate lots of extra instructions, so try solution 2 first if it is possible.
                        */
                        if (VIR_Shader_CalcMaxRegBasedOnWorkGroupSize(pShader) && !VIR_Shader_CheckWorkGroupSizeFixed(pShader))
                        {
                            gctUINT preMaxGRReg = _VIR_RA_LS_GetMaxReg(&ra, VIR_RA_HWREG_GR, reservedDataReg);
                            gctUINT curMaxGRReg = preMaxGRReg;

                            while (VIR_Shader_AdjustWorkGroupSize(pShader, pHwCfg, gcvTRUE, 8) && preMaxGRReg == curMaxGRReg)
                            {
                                curMaxGRReg = _VIR_RA_LS_GetMaxReg(&ra, VIR_RA_HWREG_GR, reservedDataReg);
                            }

                            if (curMaxGRReg > preMaxGRReg)
                            {
                                reColor = gcvTRUE;
                                break;
                            }
                        }

                        if (retValue == VSC_RA_ERR_OUT_OF_REG_SPILL)
                        {
                            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
                            {
                                VIR_LOG(pDumper, "\n!!!%d registers is not enough!!! restart the coloring with spills.\n",
                                    _VIR_RA_LS_GetMaxReg(&ra, VIR_RA_HWREG_GR, gcvFALSE));
                                VIR_LOG_FLUSH(pDumper);
                            }
                            reservedDataReg ++;
                            reColor = gcvTRUE;
                            bSpillReg = gcvTRUE;
                            break;
                        }
                    }

                    if (_VIR_RA_LS_GetMaxReg(&ra, VIR_RA_HWREG_GR, reservedDataReg) < 1)
                    {
                        retValue = VSC_RA_ERR_OUT_OF_REG_SPILL;
                    }

                    if (retValue != VSC_ERR_NONE)
                    {
                        _VIR_RA_ClearColorPool(&ra, gcvFALSE, 0, gcvTRUE);
                    }
                    ON_ERROR(retValue, "_VIR_RA_LS_PerformOnFunction");
                }

                /* II: check if we need to re-color. */
                if (!reColor)
                {
                    if (pShader->hasRegisterSpill)
                    {
                        /* check the max spilled src for an instruction, if more than one
                           we need another round assign color */
                        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
                        for (func_node = VIR_FuncIterator_First(&func_iter);
                             func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
                        {
                            pFunc = func_node->function;
                            _VIR_RA_LS_CheckInstructionDstAndSrcs(&ra, pFunc, &spilledOpndCount);
                        }

                        if (spilledOpndCount > reservedDataReg)
                        {
                            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
                            {
                                VIR_LOG(pDumper, "\n!!!found an intruction with %d spilled operand!!! restart the coloring with spills.\n",
                                    spilledOpndCount);
                                VIR_LOG_FLUSH(pDumper);
                            }
                            reservedDataReg = spilledOpndCount;
                            gcmASSERT(reservedDataReg < VIR_RA_LS_DATA_REG_NUM);
                            reColor = gcvTRUE;
                        }
                        else
                        {
                            colorSucceed = gcvTRUE;
                        }
                    }
                    else
                    {
                        colorSucceed = gcvTRUE;
                    }
                }

                /* III: do the post perform on functions. */
                if (colorSucceed)
                {
                    /*
                    ** All general LRs are assigned successfully, now we can do some post works,
                    ** including assign colors for the A0/B0 registers.
                    */
                    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
                    {
                        pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
                        VIR_Shader_SetCurrentFunction(pShader, pFunc);
                        retValue = _VIR_RA_LS_PerformOnFunction_Post(&ra, reservedDataReg);
                        CHECK_ERROR(retValue, "Fail to post perform on the functions.");

                        if (VIR_RA_LS_GetDisableDual16AndRecolor(&ra))
                        {
                            gcmASSERT(VIR_Shader_isDual16Mode(pShader));
                            VIR_Shader_SetDual16Mode(pShader, gcvFALSE);
                            reColor = gcvTRUE;
                            colorSucceed = gcvFALSE;
                            VIR_RA_LS_SetDisableDual16AndRecolor(&ra, gcvFALSE);
                            break;
                        }
                    }

                    if (colorSucceed)
                    {
                        break;
                    }
                }

                /* IV: We need to clear the color pool if we need to re-color. */
                if (reColor)
                {
                    _VIR_RA_ClearColorPool(&ra, bSpillReg, reservedDataReg, gcvFALSE);
                }
            }

            /* set reserved baseRegiter and dataRegister */
            _VIR_RA_LS_SetReservedReg(&ra);

            /* write the color into the code */
            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
            for (func_node = VIR_FuncIterator_First(&func_iter);
                 func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
            {
                pFunc = func_node->function;
                retValue = _VIR_RA_LS_RewriteColors(&ra, pFunc);
                ON_ERROR(retValue, "_VIR_RA_LS_RewriteColors");
            }

            /* set register count */
            _VIR_RA_LS_SetRegWatermark(&ra);

            /* TCS store r0 out, thus TES can get r0 */
            if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
            {
                retValue = _VIR_RA_LS_GenStoreAttr_Patch_r0(&ra);
                ON_ERROR(retValue, "_VIR_RA_LS_GenStoreAttr_Patch_r0");
            }

            /* for register spill, compute the base address */
            if (pShader->hasRegisterSpill &&
                ra.spillOffset > 0)
            {
                retValue = _VIR_RA_LS_SpillAddrComputation(&ra);
                ON_ERROR(retValue, "_VIR_RA_LS_SpillAddrComputation");
            }
            else
            {
                pShader->hasRegisterSpill = gcvFALSE;
            }

            /* set input/output VIR_SYMFLAG_LOAD_STORE_ATTR flag */
            _VIR_RA_SetInputOutputFlag(&ra);

            /* insert MOV Rn, Rn for input to help HW team to debug */
            if (gcmOPT_hasFeature(FB_INSERT_MOV_INPUT))
            {
                retValue = _VIR_RA_LS_MovHWInputRegisters(&ra, pShader);
                ON_ERROR(retValue, "_VIR_RA_LS_MovHWInputRegisters");
            }
        }
    }

    /* We don't update the DU when insert LOAD_S/STORE_S, so we need to invalid the DU. */
    if (pShader->hasRegisterSpill)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;
        pPassWorker->pResDestroyReq->s.bInvalidateCfg= gcvTRUE;
    }

    if ((gcmOPT_EnableDebug() || gcmOPT_DisableOPTforDebugger()) && ra.DIContext)
    {
        _VIR_RA_LS_WriteDebugInfo(&ra);
    }

    /* dump after */
    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                       VSC_OPTN_RAOptions_TRACE_FINAL) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after Register Allocation", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

OnError:
    /* Check if we need to cut down workGroupSize and try again. */
    if ((retValue == VSC_RA_ERR_OUT_OF_REG_SPILL || retValue == VSC_RA_ERR_OUT_OF_REG_FAIL)
        &&
        pPassWorker->basePassWorker.pPassSpecificData != gcvNULL)
    {
        *((gctBOOL*)pPassWorker->basePassWorker.pPassSpecificData) = VIR_Shader_NeedToCutDownWorkGroupSize(pShader, pHwCfg);
    }

    _VIR_RA_LS_Final(&ra);

    return retValue;
}

