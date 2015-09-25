/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "chip/gc_vsc_chip_desc.h"
#include "vir/codegen/gc_vsc_vir_reg_alloc.h"
#include "vir/transform/gc_vsc_vir_uniform.h"

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
    gcvNULL, /* colorFunc */
    gcvNULL, /* liveFunc */
    0, /* channelMask */
    gcvNULL, /* nextLR */
    gcvNULL, /* nextActiveLR */
    gcvNULL, /* usedColorLR */
    gcvTRUE, /* deadIntervalAvail */
    VIR_RA_LS_MAX_WEIGHT        /* weight */
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

    gctUINT hashVal = (((((gctUINT)(gctUINTPTR_T) pOutputKey->useInst) & 0xF) << 8) |
                          (pOutputKey->masterRegNo & 0xFF)) & 0xFFF;

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
        useIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &useKey);
        if (useIdx != VIR_INVALID_USAGE_INDEX)
        {
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, useIdx);
            return pUsage->webIdx;
        }
    }

    return VIR_INVALID_WEB_INDEX;
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
void _VIR_RA_ColorMap_Init(
    VIR_RA_LS           *pRA,
    VIR_RA_ColorMap     *pCM,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_MM              *pMM,
    VIR_RA_HWReg_Type   hwRegType)
{
    VSC_OPTN_RAOptions  *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);

    switch(hwRegType)
    {
    case VIR_RA_HWREG_GR:
        pCM->maxReg = pHwCfg->maxGPRCountPerCore;

        /* VS/TCS/TES/GS for v6.0 to at most 61 temps*/
        if ((VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5) &&
            (pShader->shaderKind == VIR_SHADER_VERTEX ||
             pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
             pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION ||
             pShader->shaderKind == VIR_SHADER_GEOMETRY))
        {
            pCM->maxReg = pCM->maxReg - 3;
        }

        if (VSC_OPTN_InRange(VIR_Shader_GetId(pShader),
                              VSC_OPTN_RAOptions_GetSpillBeforeShader(pOption),
                              VSC_OPTN_RAOptions_GetSpillAfterShader(pOption)))
        {
            if (VSC_OPTN_RAOptions_GetRegisterCount(pOption) > 0)
            {
                /* set the maxReg from the command line, should not larger than maxGPRCountPerCore*/
                if (VSC_OPTN_RAOptions_GetRegisterCount(pOption) < pHwCfg->maxGPRCountPerCore)
                {
                    pCM->maxReg = VSC_OPTN_RAOptions_GetRegisterCount(pOption);
                }
            }
        }
        break;
    case VIR_RA_HWREG_A0:
        pCM->maxReg = 1;
        break;
    default:
        break;
    }

    pCM->maxAllocReg = 0;
    pCM->availReg = 0;

    /* each register needs 4 channels (w, z, y, x) */
    vscBV_Initialize(&pCM->usedColor, pMM,
        pCM->maxReg * VIR_CHANNEL_NUM);
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
        if (pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
        {
            if (isLRSpilled(pLR))
            {
                VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_SPILLED);
            }

            _VIR_RA_SetLRColor(pLR, VIR_RA_INVALID_REG, 0);
            _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
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
    pLR->deadIntervals = 0;

    pLR->nextLR = gcvNULL;
    pLR->nextActiveLR = gcvNULL;

    pLR->deadIntervalAvail = gcvTRUE;
    pLR->usedColorLR = gcvNULL;

    pLR->weight = VIR_RA_LS_MAX_WEIGHT;
}

void _VIR_RA_LS_InitLRShader(
    VIR_RA_LS_Liverange     *pLR,
    gctUINT                  index)
{
    _VIR_RA_LS_InitLRFunc(pLR, index);

    /* has one value in the shader */
    pLR->webIdx = index;
    pLR->flags = 0;
    pLR->masterWebIdx = VIR_INVALID_WEB_INDEX;
    pLR->hwType = VIR_RA_HWREG_GR;
    pLR->channelMask = 0;
    pLR->firstRegNo = VIR_RA_LS_REG_MAX;
    pLR->regNoRange = 1;
    pLR->colorFunc = gcvNULL;
    _VIR_RA_SetLRColor(pLR, VIR_RA_INVALID_REG, 0);
    _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
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

            usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                webIdx = _VIR_RA_LS_Use2Web(pRA, usageIdx);
                pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
                VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_SUB_SAMPLE_DEPTH);
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
    gctUINT      webIdx;

    /* sampleMask/Id/Pos need sampleDepth */
    if (pShader->shaderKind == VIR_SHADER_FRAGMENT &&
        pShader->sampleMaskIdRegStart == VIR_REG_MULTISAMPLEDEPTH &&
        VIR_Shader_PS_NeedSampleMaskId(pShader))
    {
        return gcvTRUE;
    }

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx ++)
    {
        VIR_RA_LS_Liverange* pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
        if (isLRsubSampleDepth(pLR))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static void _VIR_RA_LS_Init(
    VIR_RA_LS           *pRA,
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VIR_LIVENESS_INFO   *pLvInfo,
    VSC_OPTN_RAOptions  *pOptions,
    VIR_Dumper          *pDumper,
    VIR_CALL_GRAPH      *pCG)
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

    /* initialize the memory pool */
    vscPMP_Intialize(VIR_RA_LS_GetPmp(pRA), gcvNULL,
        VIR_RS_LS_MEM_BLK_SIZE, sizeof(void*), gcvTRUE);

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
        pLR = _VIR_RA_LS_Web2LR(pRA, i);
        _VIR_RA_LS_InitLRShader(pLR, i);
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
                            _VIR_RA_OutputKey_HFUNC, _VIR_RA_OutputKey_HKCMP, 2048);

    _VIR_RA_Check_SpecialFlags(pRA);

    /* make sure that liveness is valid */
    gcmASSERT(vscVIR_GetDFAValidity(&pLvInfo->baseTsDFA.baseDFA) &&
              pLvInfo->pDuInfo->bWebTableBuilt);

    /* to-do: remove this reserved register, if we move select_map to
       a phase before RA
    */
    pRA->resRegister = VIR_INVALID_ID;

    /* for register spills */
    /* the offset where the next spill should be */
    pRA->spillOffset = 0;
    /* reserved HW register for base address, offset, or threadIndex
       baseRegister.x base address for spill
       baseRegister.y computed offset for spill (LDARR/STARR)
       baseRegister.z threadIndex got from the atomic_add
       baseRegister.w save the MOVA src0 (in case of redefine) */
    pRA->baseRegister = VIR_INVALID_ID;
    /* reserved HW register for data, we may need to reserve more than one
       registers to save the data, since in some instruction, maybe more than
       one src is spilled */
    pRA->resDataRegisterCount = 0;
    for (i = 0; i < VIR_MAX_SRC_NUM; i++)
    {
        pRA->dataRegister[i] = VIR_INVALID_ID;
        pRA->dataSymId[i] = VIR_INVALID_ID;
    }
    pRA->baseAddrSymId  = VIR_INVALID_ID;
    pRA->spillOffsetSymId  = VIR_INVALID_ID;
    pRA->threadIdxSymId  = VIR_INVALID_ID;

    pRA->samplePosRegister = VIR_INVALID_ID;
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
    vscPMP_Finalize(VIR_RA_LS_GetPmp(pRA));
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

    VIR_LOG(pDumper, "web[%d]: \t", pLR->webIdx);
    VIR_LOG(pDumper, "tmp(%d", _VIR_RS_LS_GetWebRegNo(pRA, pLR));
    if (pLR->regNoRange > 1)
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

    VIR_LOG(pDumper, "\n");
    VIR_LOG_FLUSH(pDumper);
}

void VIR_RS_LS_DumpLRTable(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc)
{
    VIR_RA_LS_Liverange *pLR;
    gctUINT             webIdx;

    for (webIdx = 0; webIdx < VIR_RA_LS_GetNumWeb(pRA); webIdx++)
    {
        pLR = _VIR_RA_LS_Web2LR(pRA, webIdx);
        if (pLR->liveFunc == pFunc)
        {
            _VIR_RS_LS_DumpLR(pRA, pLR, gcvFALSE);
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
        if (pLR->liveFunc == pFunc)
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
        pLvInfo->pDuInfo, pInst, pOpnd, gcvFALSE);
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

/* return true if the LDARR will be removed and
   the use of its def will be replaced with indexing */
gctBOOL _VIR_RA_LS_removableLDARR(
    VIR_RA_LS           *pRA,
    VIR_Instruction     *pInst,
    gctBOOL             needReplace)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);

    gctUINT             defIdx = VIR_INVALID_DEF_INDEX, srcIndex;
    VIR_DEF             *pDef;
    VSC_DU_ITERATOR     duIter;

    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_USAGE               *pUsage;
    VIR_Instruction         *pUseInst;
    VIR_Operand             *pBaseOpnd = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand             *pDest = VIR_Inst_GetDest(pInst);

    gctBOOL         retValue = gcvTRUE;
    VIR_Swizzle     useSwizzle = VIR_SWIZZLE_INVALID;
    VIR_Operand     *newOpnd = gcvNULL;
    VIR_OperandInfo srcInfo;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR);

    /* dual16 should not replace its uses, since instruction with indexing
       should not be dual16 */
    if (VIR_Shader_isDual16Mode(pShader) &&
        !VIR_TypeId_isSamplerOrImage(VIR_Operand_GetType(pDest)))
    {
        return gcvFALSE;
    }

    /* get ldarr's dst LR */
    defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
    while (VIR_INVALID_DEF_INDEX != defIdx)
    {
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

        /* go through all the uses */
        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
        for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
        {
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
            pUseInst = pUsage->usageKey.pUsageInst;

            if(VIR_IS_OUTPUT_USAGE_INST(pUseInst))
            {
                retValue = gcvFALSE;
                continue;
            }

            /* already replaced*/
            if (VIR_Operand_GetRelAddrMode(pUsage->usageKey.pOperand) != VIR_INDEXED_NONE)
            {
                continue;
            }

            if (VIR_Inst_GetOpcode(pUseInst) == VIR_OP_STORE ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_STORE_ATTR ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_ATTR_ST ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_IMG_STORE ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_VX_IMG_STORE ||
                VIR_Inst_GetOpcode(pUseInst) == VIR_OP_IMG_STORE_3D)
            {
                retValue = gcvFALSE;
                continue;
            }

            VIR_Operand_GetOperandInfo(pUseInst, pUsage->usageKey.pOperand, &srcInfo);

            if(!vscVIR_IsUniqueDefInstOfUsageInst(
                        pLvInfo->pDuInfo,
                        pUseInst,
                        pUsage->usageKey.pOperand,
                        pInst,
                        gcvNULL))
            {
                retValue = gcvFALSE;
                continue;
            }

            srcIndex = VIR_Inst_GetSourceIndex(pUseInst, pUsage->usageKey.pOperand);
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

            /* replace its uses
                pInst:    LDARR t1.x base, offset
                pUseInst: ADD t2.x, t1.x, t3.x
                ==>
                ADD t2.x, base[offset].x, t3.x
                !sampler use has to be replaced
            */
            if (needReplace)
            {
                useSwizzle = VIR_Operand_GetSwizzle(pUseInst->src[srcIndex]);
                VIR_Function_DupOperand(pFunc, pBaseOpnd, &newOpnd);
                VIR_Operand_SetSwizzle(newOpnd, useSwizzle);
                VIR_Operand_SetType(newOpnd, VIR_Operand_GetType(pUseInst->src[srcIndex]));

                /* update the du - not complete yet
                   only delete the usage of t1, not add usage for base and offset */
                vscVIR_DeleteUsage(pLvInfo->pDuInfo,
                        pInst,
                        pUseInst,
                        pUsage->usageKey.pOperand,
                        srcInfo.u1.virRegInfo.virReg,
                        1,
                        VIR_Swizzle_2_Enable(useSwizzle),
                        VIR_HALF_CHANNEL_MASK_FULL,
                        gcvNULL);

                pUseInst->src[srcIndex] = newOpnd;

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

/* return true, if the opnd/symbol/DEF is not needed to assign registers
   (i.e., in memory) */
gctBOOL _VIR_RA_LS_IsExcludedLR(
    VIR_RA_LS       *pRA,
    VIR_Operand     *pOpnd,
    VIR_Symbol      *pSym,
    VIR_DEF         *pDef,
    VIR_Instruction *pInst)
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

    if (pDef &&
        (pDef->flags.bIsPerVtxCp ||
         pDef->flags.bIsPerPrim))
    {
        return gcvTRUE;
    }

    /* in v60, USC has constraint. If store instruction's src2 is
       immediate/uniform/indirect, there must be a store destination.
       store_attr/attr_st could not arrive here, since its dest should be per-patch,
       or per-vertex. we will use reservered register for its destination if needed. */
    if (pInst &&
        pInst != VIR_INPUT_DEF_INST &&
        pInst != VIR_HW_SPECIAL_DEF_INST &&
        (VIR_Inst_GetOpcode(pInst) == VIR_OP_STORE ||
         VIR_Inst_GetOpcode(pInst) == VIR_OP_IMG_STORE ||
         VIR_Inst_GetOpcode(pInst) == VIR_OP_VX_IMG_STORE ||
         VIR_Inst_GetOpcode(pInst) == VIR_OP_IMG_STORE_3D))
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

/* return true if instruction requires no shift for its def */
gctBOOL
_VIR_RA_LS_IsRestrictInst(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    VIR_Shader  *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(pInst);

    if (opcode == VIR_OP_MOD            ||
        opcode == VIR_OP_LOAD_ATTR      ||
        opcode == VIR_OP_ATTR_LD        ||
        opcode == VIR_OP_IMG_ADDR       ||
        opcode == VIR_OP_IMG_ADDR_3D    ||
        opcode == VIR_OP_IMG_LOAD       ||
        opcode == VIR_OP_VX_IMG_LOAD    ||
        opcode == VIR_OP_IMG_LOAD_3D    ||
        opcode == VIR_OP_VX_IMG_LOAD_3D ||
        opcode == VIR_OP_ARCTRIG        ||
        VIR_OPCODE_isTexLd(opcode)      ||
        VIR_OPCODE_isAtom(opcode))
    {
        return gcvTRUE;
    }

    if (opcode == VIR_OP_DIV)
    {
        VIR_Operand *pOpnd = VIR_Inst_GetDest(pInst);
        VIR_Type    *type = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(pOpnd));
        VIR_PrimitiveTypeId baseType = VIR_Type_GetBaseTypeId(type);
        if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
        {
            return gcvTRUE;
        }
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
        gcmASSERT(varSym && VIR_Symbol_isOutput(varSym));
        regNo = varSym->u2.tempIndex;
    }

    return regNo;
}

/* we need to assign consecutive registers to output array/matrix */
void _VIR_RA_LS_HandleOutputLR(
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

    if (pDef->flags.bIsOutput && pDef->flags.bDynIndexed)
    {
        /* get the vreg sym */
        VIR_Symbol *dstSym = VIR_Operand_GetSymbol(pInst->dest);
        VIR_Symbol *varSym = gcvNULL;

        _VIR_RA_LS_SetRestrictLR(pRA, defIdx);

        _VIR_RA_LS_SetDynIndexingLR(pRA, defIdx);

        if (VIR_Symbol_isVreg(dstSym))
        {
            /* get the vreg variable sym, which should be an output */
            varSym = VIR_Symbol_GetVregVariable(dstSym);
            gcmASSERT(varSym && VIR_Symbol_isOutput(varSym));

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
                pLR->regNoRange = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(varSym));

                VIR_RA_LR_ClrFlag(pLR, VIR_RA_LRFLAG_INVALID);
            }
            else
            {
                /* if output array temps are in seperate LRs */
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

void _VIR_RA_LS_SetRegNoRange(
    VIR_RA_LS           *pRA,
    gctUINT             defIdx,
    gctUINT             firstRegNo,
    gctUINT             regNoRange)
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
                cruption issue
                TODO for perf consideration:
                1. Allow unindexing output array be allocate into discreted regs, OR
                2. Consider individual conflictions when color assignments, but this looks difficult because
                    we are using linear scan allocation */

            pMasterLR->startPoint = vscMIN(pMasterLR->startPoint, pLR->startPoint);
            pMasterLR->endPoint = vscMAX(pMasterLR->endPoint, pLR->endPoint);
        }
    }
}

void _VIR_RS_LS_MarkLRLive(
    VIR_RA_LS   *pRA,
    gctUINT     defIdx,
    VIR_Enable  enable,
    gctBOOL     posAfter)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
    VIR_RA_LS_Interval      *pInterval;

    if (pLR->liveFunc == gcvNULL)
    {
        /* This is the first time we see this live range and it is a use */
        pLR->startPoint = 0;
        /* We mark the LR live after the current pos.
           This is used for mark live at BB's live out*/
        if (posAfter)
        {
            pLR->endPoint = VIR_RA_LS_GetCurrPos(pRA) + 1;
        }
        else
        {
            pLR->endPoint = VIR_RA_LS_GetCurrPos(pRA);
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
                pInterval->startPoint = VIR_RA_LS_GetCurrPos(pRA) + 1;
            }
            else
            {
                pInterval->startPoint = VIR_RA_LS_GetCurrPos(pRA);
            }
            pInterval->endPoint = pLR->startPoint;
            pInterval->next = pLR->deadIntervals;
            pLR->deadIntervals = pInterval;
            pLR->startPoint = 0;
        }
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

    VIR_DEF                 *pDef;
    VSC_DU_ITERATOR         duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_USAGE               *pUsage;
    VIR_RA_LS_Liverange     *pUseLR;

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
    if (pLR->hwType == VIR_RA_HWREG_A0)
    {
        pLR->origEndPoint = pLR->endPoint;

        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

        /* go through all the uses, find the longest live range */
        VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
        pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
        for (; pUsageNode != gcvNULL; )
        {
            pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, pUsageNode->usageIdx);
            defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pUsage->usageKey.pUsageInst);

            /* we only need to extend LDARR's live range, not STARR's */
            if ((VIR_INVALID_DEF_INDEX != defIdx) &&
                (VIR_Inst_GetOpcode(pUsage->usageKey.pUsageInst) == VIR_OP_LDARR))
            {
                pUseLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
                if (pUseLR->endPoint > pLR->endPoint)
                {
                    pLR->endPoint = pUseLR->endPoint;
                }
            }
            pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter);
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

/* return gcvTRUE if all the defs in the web are dead */
gctBOOL _VIR_RS_LS_AllDefDead(
    VIR_RA_LS  *pRA,
    gctUINT     defIdx)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             webIdx = _VIR_RA_LS_Def2Web(pRA, defIdx);
    VIR_WEB             *pWeb = GET_WEB_BY_IDX(&pLvInfo->pDuInfo->webTable, webIdx);
    gctUINT             nextDefIdx = pWeb->firstDefIdx;
    VIR_DEF             *pDef;

    while (VIR_INVALID_DEF_INDEX != nextDefIdx)
    {
        if (_VIR_RS_LS_TestLiveLRVec(pRA, nextDefIdx))
        {
            return gcvFALSE;
        }
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, nextDefIdx);
        nextDefIdx = pDef->nextDefInWebIdx;
    }

    return gcvTRUE;
}

void _VIR_RA_LS_MarkDef(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    gctUINT         firstRegNo,
    gctUINT         regNoRange,
    VIR_Enable      defEnableMask,
    gctBOOL         bDstIndexing)
{
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             regNo, defIdx;
    gctUINT8            channel;
    VIR_DEF_KEY         defKey;
    VIR_DEF             *pDef;
    gctBOOL             removableLDARR = gcvFALSE;

    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR &&
        _VIR_RA_LS_removableLDARR(pRA, pInst, gcvFALSE))
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

            defKey.pDefInst = VIR_ANY_DEF_INST;
            defKey.regNo = regNo;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->defTable, &defKey);
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                VIR_RA_LS_Liverange     *pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
                gcmASSERT(pLR);

                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                gcmASSERT(pDef);

                /* skip the per-vertex output and per-patch output */
                if (!_VIR_RA_LS_IsExcludedLR(pRA, gcvNULL, gcvNULL, pDef, pInst))
                {
                    if (pDef->defKey.pDefInst == pInst && pDef->defKey.channel == channel)
                    {
                        if (_VIR_RA_LS_IsRestrictInst(pRA, pInst) ||
                            pDef->flags.bIsOutput)
                        {
                            _VIR_RA_LS_SetRestrictLR(pRA, defIdx);
                        }

                        _VIR_RA_LS_SetRegNoRange(pRA, defIdx, firstRegNo, regNoRange);

                        _VIR_RA_LS_HandleOutputLR(pRA, pInst, defIdx);

                        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_MOVA)
                        {
                            _VIR_RA_LS_SetHwRegType(pRA, defIdx, VIR_RA_HWREG_A0);
                        }

                        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_ATOMCMPXCHG &&
                            VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasHalti5)
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_ATOMCMPXHG);
                        }

                        if (removableLDARR)
                        {
                            VIR_RA_LR_SetFlag(pLR, VIR_RA_LRFLAG_RM_LDARR_DEST);
                        }

                        /* Two cases that must mark dead
                           1. Def is a definite write. It is same logic as LV analysis.
                           2. Def is an orphan. It is the special logic for LR marker as we need give all
                              webs a legal LR range */
                        if ((!bDstIndexing &&
                             !VIR_OPCODE_CONDITIONAL_WRITE(VIR_Inst_GetOpcode(pInst)) &&
                             !VIR_OPCODE_DestOnlyUseEnable(VIR_Inst_GetOpcode(pInst))) ||
                            DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain) == 0)
                        {
                            _VIR_RS_LS_UnsetLiveLRVec(pRA, defIdx);

                            /* if LR's mask (LR mask is the "OR" of def's mask) matches
                               the web's channelmask, mark the LR dead
                               and the defs in the web are all dead (to-do: not need maskMatch?) */
                            if (_VIR_RS_LS_MaskMatch(pRA, (0x1 << channel), defIdx) &&
                                _VIR_RS_LS_AllDefDead(pRA, defIdx))
                            {
                                _VIR_RS_LS_MarkLRDead(pRA, defIdx, (0x1 << channel), gcvFALSE);
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

    gctBOOL                 posAfter = gcvFALSE;
    if (_VIR_RA_LS_isUseCrossInst(pRA, pInst))
    {
        posAfter = gcvTRUE;
    }

    /* Find the usage */
    usageKey.pUsageInst = pInst;
    usageKey.pOperand = pOperand;
    usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
    if (VIR_INVALID_USAGE_INDEX != usageIdx)
    {
        pUsage = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
        gcmASSERT(pUsage);
        gcmASSERT(pUsage->webIdx != VIR_INVALID_WEB_INDEX);

        /* Each def of this usage are live now */
        /* to-do: optimize this to set LR once */
        for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
        {
            defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
            gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
            pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);

            if (pDef->flags.bIsPerVtxCp ||
                pDef->flags.bIsPerPrim)
            {
                continue;
            }

            _VIR_RA_LS_SetRegNoRange(pRA, defIdx, firstRegNo, regNoRange);
            _VIR_RS_LS_MarkLRLive(pRA, defIdx, defEnableMask, posAfter);
            _VIR_RS_LS_SetLiveLRVec(pRA, defIdx);
        }
    }

    /* A WAR to fix an issue introduced by mova + ldarr seq.
       !!!TODO: We should support mova + Rb[Ro.single_channel] in MC level, and remove mova + ldarr
                and mova + starr seq */
    if (pUsage)
    {
        gctUINT          firstRegNo1, regNoRange1;
        VIR_Enable       defEnableMask1;
        VIR_OperandInfo  operandInfo, operandInfo1;

        defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, 0);
        pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
        if (!pDef->flags.bIsPerVtxCp && !pDef->flags.bIsPerPrim)
        {
            if (pDef->defKey.pDefInst < VIR_INPUT_DEF_INST)
            {
                if (vscVIR_IsUniqueDefInstOfUsageInst(pLvInfo->pDuInfo, pInst, pOperand,
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

void _VIR_RA_LS_MarkUses(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst)
{
    gctUINT                 firstRegNo, regNoRange;
    VIR_Enable              defEnableMask;
    VIR_OperandInfo         operandInfo, operandInfo1;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand             *pOpnd;

    /* A ldarr inst to attribute array may potentially read all elements in array */
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR)
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
                regNoRange = 1;
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
    VIR_Instruction *pInst)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_LIVENESS_INFO   *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    gctUINT             outputIdx, usageIdx;
    VIR_USAGE_KEY       usageKey;
    VIR_USAGE           *pUsage;
    gctUINT             i, j, defIdx;
    VIR_Enable          outEnable;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT);

    for (outputIdx = 0;
        outputIdx < VIR_IdList_Count(VIR_Shader_GetOutputs(pShader));
        outputIdx ++)
    {
        VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                            VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader), outputIdx));

        for (i = 0; i <VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pOutputSym)); i++)
        {
            /* Find the usage */
            usageKey.pUsageInst = pInst;
            usageKey.pOperand = (VIR_Operand*)(gctUINTPTR_T) (pOutputSym->u2.tempIndex+i);
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
                    _VIR_RS_LS_MarkLRLive(pRA, defIdx, outEnable, gcvFALSE);
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
    if ((regNo ==  VIR_SR_INSTATNCEID) ||
        (regNo ==  VIR_SR_VERTEXID) ||
        (regNo == VIR_REG_MULTISAMPLEDEPTH) ||
        (regNo == VIR_REG_SAMPLE_POS) ||
        (regNo == VIR_REG_SAMPLE_ID) ||
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
    VIR_RA_LS   *pRA,
    gctUINT     hwType,
    gctUINT     reservedDataReg)
{
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_ColorPool    *pCP = VIR_RA_LS_GetColorPool(pRA);
    gctUINT             maxFreeReg = 0;
    gctFLOAT            workGroupSize = 0, threadCount;
    VSC_HW_CONFIG       *hwConfig = VIR_RA_LS_GetHwCfg(pRA);
    gctUINT             maxReg = pCP->colorMap[hwType].maxReg;

    /* if the shader needs sampleDepth, we need to make sure the last
       register is for sampleDepth */
    if (_VIR_RA_isShaderNeedSampleDepth(pRA))
    {
        maxReg -= 1;
    }

    if (VIR_Shader_HasBarrier(pShader))
    {
        /* if compute shader has barrier, the temp count must follow
           ceiling(work_group_size/(shader_core_count*4*threads_per_register)) <= floor(113/temp_register_count)
        */

        /* VIR_SHADER_TESSELLATION_CONTROL should not have barrier if core == 8 */
        gcmASSERT(pShader->shaderKind == VIR_SHADER_CL ||
                  pShader->shaderKind == VIR_SHADER_COMPUTE ||
                  pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL);
        threadCount = (gctFLOAT) (hwConfig->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1));

        if (hwConfig->hwFeatureFlags.hasHalti5)
        {
            /*  128: total temp registers per shader.
                16: reserved 1 page (4 registers) each for other shaders.
                3: partial page (up to 7 registers) used by current shader previously.
                free registers: 128 - 16 - 3 = 109 */
            maxFreeReg = 109;
        }
        else
        {
            /* 128: total temp registers per shader.
               8: reserved 1 page (8 registers) for VS or PS.
               7: partial page (up to 7 registers) used by PS or VS.
               free registers: 128 - 8 - 7 = 113 */
            maxFreeReg = 113;
        }

        if (pShader->shaderKind == VIR_SHADER_COMPUTE)
        {
            workGroupSize = (gctFLOAT) (pShader->shaderLayout.compute.workGroupSize[0] *
                                        pShader->shaderLayout.compute.workGroupSize[1] *
                                        pShader->shaderLayout.compute.workGroupSize[2]);
            maxReg = maxFreeReg / (gctUINT) (ceil(workGroupSize / threadCount));
        }
        else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
        {
            workGroupSize = (gctFLOAT) (pShader->shaderLayout.tcs.tcsPatchOutputVertices);
            maxReg = maxFreeReg / (gctUINT) (ceil(workGroupSize / threadCount));
        }
        else if (pShader->shaderKind == VIR_SHADER_CL)
        {
            workGroupSize = 128.0;  /* assume minumum work group size is 128 */
            maxReg = maxFreeReg / (gctUINT) (ceil(workGroupSize / threadCount));
        }
    }

    /* we need to reserve 2 more registers for spilling */
    if (reservedDataReg > 0 &&
        hwType == VIR_RA_HWREG_GR)
    {
        maxReg = maxReg - reservedDataReg - 1;
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
        /* every spill has 16 bytes, since regNoRange is coming from VIR_Type_GetVirRegCount
           to-do: change to based on size */
        pRA->spillOffset += pLR->regNoRange * 16;
    }
    else
    {
        _VIR_RA_SetLRColor(pLR,
                            _VIR_RA_Color_RegNo(color),
                            _VIR_RA_Color_Shift(color));
        _VIR_RA_SetLRColorHI(pLR,
                            _VIR_RA_Color_HIRegNo(color),
                            _VIR_RA_Color_HIShift(color));

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
    /* unset the usedColor bit accordingly */
    gcmASSERT(!_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pRemoveLR)));

    /* this LR is used color from usedColorLR's dead interval*/
    if (pRemoveLR->usedColorLR != gcvNULL)
    {
        pRemoveLR->usedColorLR->deadIntervalAvail = gcvTRUE;
        pRemoveLR->usedColorLR = gcvNULL;
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
                _VIR_RA_Color_Channels(
                    VIR_RA_LS_LR2WebChannelMask(pRA, pRemoveLR),
                    _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pRemoveLR))));

            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pRemoveLR)))
            {
                _VIR_RA_LS_ClearUsedColor(pRA, pRemoveLR->hwType,
                    _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pRemoveLR)) + i * (highDiff + 1),
                    _VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pRemoveLR),
                            _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pRemoveLR))));
            }
        }
    }
}

void _VIR_RA_LS_ExpireActiveLRs(
    VIR_RA_LS           *pRA,
    gctUINT             pos)
{
    VIR_RA_LS_Liverange *pPrev, *pCurr, *pNext;

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

    gctUINT   i, regNo, regNoHI, highDiff = 0;

    gcmASSERT(!isLRSpilled(pLR));

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

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "add LR%d to the active list", pLR->webIdx);
            VIR_LOG_FLUSH(pDumper);
        }

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
                _VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                   regNo,
                   _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
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

            _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType,
                regNo,
                _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR))));

            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)))
            {
                if (newColor &&
                    pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC &&
                    _VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                       regNoHI,
                       _VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
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

                _VIR_RA_LS_SetUsedColor(pRA, pLR->hwType,
                    regNoHI,
                    _VIR_RA_Color_Channels(
                            VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                            _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR))));
            }
        }

         /* set the register allocation water mark */
        _VIR_RA_LS_SetMaxAllocReg(pRA, _VIR_RA_GetLRColor(pLR),
            pLR->hwType, pLR->regNoRange);

        if (pLR->colorFunc != VIR_RA_LS_ATTRIBUTE_FUNC)
        {
            pLR->colorFunc = pFunc;
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

    if (srcLR != gcvNULL)
    {
        srcChannel = _VIR_RA_Color_Channels(
                        VIR_RA_LS_LR2WebChannelMask(pRA, srcLR),
                        _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(srcLR)));
        srcColor = gcvTRUE;
    }

    switch (dstChannel)
    {
    case VIR_ENABLE_X:
        /* See if x-component is available. */
        if ((srcColor && ((srcChannel & 0x1) == 0x1)) ||
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
        /* to-do: check x-component is available, need shift to be -1*/
        /* See if y-component is available. */
        if ((srcColor && ((srcChannel & 0x2) == 0x2)) ||
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
        /* See if x- and y-components are available. */
        if ((srcColor && ((srcChannel & 0x3) == 0x3)) ||
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

/* return VIR_RA_INVALID_COLOR if there is no color to assign
   i.e., all usedColor are set,
   in dual16, the high precision temp needs two registers */
VIR_RA_HWReg_Color _VIR_RA_LS_FindNewColor(
    VIR_RA_LS   *pRA,
    gctUINT     webIdx,
    gctBOOL     needHI,
    gctUINT     reservedDataReg)
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &retValue);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &retValue);

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

        if (needHI && pLR->regNoRange > 1)
        {
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
                        regno = regno - pLR->regNoRange;
                        _VIR_RA_MakeColor(regno, LRShift, &retValue);
                        break;
                    }
                }
                else
                {
                    regno++;
                }
            }

            if (needHI)
            {
                gctUINT regnoHI;

                if (pLR->hwType ==  VIR_RA_HWREG_GR)
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
                                break;
                            }
                        }
                        else
                        {
                            regnoHI++;
                        }
                    }
                }
                else
                {
                    gcmASSERT(pLR->regNoRange == 1);
                    regnoHI = regno;
                    i = 1;
                    while (LRShift + i <= 3)
                    {
                        if (_VIR_RA_LS_ChannelAvail(pRA, regnoHI, 0x1 << (LRShift + i), VIR_RA_HWREG_A0))
                        {
                            _VIR_RA_MakeColorHI(regnoHI, LRShift + i, &retValue);
                            break;
                        }
                        i++;
                    }
                    gcmASSERT(LRShift + i <= 3);
                }
            }
        }
    }

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

        if (pLR->hwType == VIR_RA_HWREG_A0)
        {
            /* when all 4 A0 channels are used, we need to keep ldarr (as a move) */
            if (LRShift == 3)
            {
                pLR->endPoint = pLR->origEndPoint;
            }
        }
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &invalidColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &invalidColor);

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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &retValue);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &retValue);

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

/* ===========================================================================
   VIR_CG_MapUniforms related functions
   ===========================================================================
*/
void VIR_CG_UniformColorMap_Init(
    IN VIR_Shader           *pShader,
    IN VSC_HW_CONFIG        *pHwConfig,
    IN VSC_MM               *pMM,
    IN OUT VIR_RA_ColorMap  *uniformCM,
    OUT gctUINT             *CodeGenUniformBase
    )
{
    uniformCM->maxAllocReg = 0;
    uniformCM->availReg = 0;

    /* get from the hardware configuration */
    switch (pShader->shaderKind) {
    case VIR_SHADER_FRAGMENT:
    case VIR_SHADER_COMPUTE:
    case VIR_SHADER_CL:
        *CodeGenUniformBase = pHwConfig->psConstRegAddrBase;
        uniformCM->maxReg = pHwConfig->maxPSConstRegCount;
        break;
    case VIR_SHADER_VERTEX:
        *CodeGenUniformBase = pHwConfig->vsConstRegAddrBase;
        uniformCM->maxReg = pHwConfig->maxVSConstRegCount;
        break;
    case VIR_SHADER_TESSELLATION_CONTROL:
        *CodeGenUniformBase = pHwConfig->tcsConstRegAddrBase;
        uniformCM->maxReg = pHwConfig->maxTCSConstRegCount;
        break;
    case VIR_SHADER_TESSELLATION_EVALUATION:
        *CodeGenUniformBase = pHwConfig->tesConstRegAddrBase;
        uniformCM->maxReg = pHwConfig->maxTESConstRegCount;
        break;
    case VIR_SHADER_GEOMETRY:
        *CodeGenUniformBase = pHwConfig->gsConstRegAddrBase;
        uniformCM->maxReg = pHwConfig->maxGSConstRegCount;
        break;
    default:
        gcmASSERT(0);
        break;
    }

    /* each const registers needs 4 channels (w, z, y, x) */
    vscBV_Initialize(&uniformCM->usedColor,
        pMM,
        uniformCM->maxReg * VIR_CHANNEL_NUM);
}

void VIR_CG_SetUniformUsed(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           startIdx,
    IN gctINT           rows,
    IN gctUINT8         enable
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;

    while (rows-- > 0)
    {
         /* Test if x-component is available. */
        if (enable & 0x1)
        {
             vscBV_SetBit(usedColor, startIdx * 4 + 0);
        }

        /* Test if y-component is available. */
        if (enable & 0x2)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 1);
        }

        /* Test if z-component is available. */
        if (enable & 0x4)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 2);
        }

        /* Test if w-component is available. */
        if (enable & 0x8)
        {
            vscBV_SetBit(usedColor, startIdx * 4 + 3);
        }

        startIdx++;
    }
}

gctBOOL VIR_CG_UniformAvailable(
    IN VIR_RA_ColorMap      *uniformColorMap,
    IN gctINT               startIdx,
    IN gctINT               rows,
    IN gctUINT8             enable
    )
{
    VSC_BIT_VECTOR  *usedColor = &uniformColorMap->usedColor;

    while (rows-- > 0)
    {
         /* Test if x-component is available. */
        if ((enable & 0x1) && vscBV_TestBit(usedColor, startIdx * 4 + 0))
        {
            return gcvFALSE;
        }

        /* Test if y-component is available. */
        if ((enable & 0x2) && vscBV_TestBit(usedColor, startIdx * 4 + 1))
        {
            return gcvFALSE;
        }

        /* Test if z-component is available. */
        if ((enable & 0x4) && vscBV_TestBit(usedColor, startIdx * 4 + 2))
        {
            return gcvFALSE;
        }

        /* Test if w-component is available. */
        if ((enable & 0x8) && vscBV_TestBit(usedColor, startIdx * 4 + 3))
        {
            return gcvFALSE;
        }

        startIdx++;
    }

    return gcvTRUE;
}

VSC_ErrCode
VIR_CG_FindUniformUse(
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN VIR_TypeId       type,
    IN gctINT           arraySize,
    IN gctBOOL          restricted,
    OUT gctINT          *Physical,
    OUT gctUINT8        *Swizzle,
    OUT gctINT          *Shift)
{
    gctINT i;
    gctUINT32 components = 0;
    gctINT shift;
    gctUINT8 swizzle = 0, enable = 0;

    components = VIR_GetTypeComponents(type);

    if ((gctINT) uniformColorMap->maxReg < arraySize)
    {
        return VSC_ERR_OUT_OF_RESOURCE;
    }

    for (i = 0; i <= ((gctINT) uniformColorMap->maxReg) - arraySize; ++i)
    {
        shift = -1;

        switch (components)
        {
        case 1:
            /* See if x-component is available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_X;
                swizzle = gcSL_SWIZZLE_XXXX;
            }

            /* See if y-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_Y;
                swizzle = gcSL_SWIZZLE_YYYY;
            }

            /* See if z-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 2))
            {
                shift   = 2;
                enable  = gcSL_ENABLE_Z;
                swizzle = gcSL_SWIZZLE_ZZZZ;
            }

            /* See if w-component is available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x1 << 3))
            {
                shift   = 3;
                enable  = gcSL_ENABLE_W;
                swizzle = gcSL_SWIZZLE_WWWW;
            }

            break;

        case 2:
            /* See if x- and y-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XY;
                swizzle = gcSL_SWIZZLE_XYYY;
            }

            /* See if y- and z-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_YZ;
                swizzle = gcSL_SWIZZLE_YZZZ;
            }

            /* See if z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x3 << 2))
            {
                shift   = 2;
                enable  = gcSL_ENABLE_ZW;
                swizzle = gcSL_SWIZZLE_ZWWW;
            }

            break;

        case 3:
            /* See if x-, y- and z-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x7 << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XYZ;
                swizzle = gcSL_SWIZZLE_XYZZ;
            }

            /* See if y-, z- and w-components are available. */
            else if (!restricted && VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0x7 << 1))
            {
                shift   = 1;
                enable  = gcSL_ENABLE_YZW;
                swizzle = gcSL_SWIZZLE_YZWW;
            }

            break;

        case 4:
            /* See if x-, y-, z- and w-components are available. */
            if (VIR_CG_UniformAvailable(uniformColorMap, i, arraySize, 0xF << 0))
            {
                shift   = 0;
                enable  = gcSL_ENABLE_XYZW;
                swizzle = gcSL_SWIZZLE_XYZW;
            }

            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        if (shift >=0)
        {
            *Physical = i;
            *Shift = shift;
            *Swizzle = swizzle;

            /* Set the uniform used */
            VIR_CG_SetUniformUsed(uniformColorMap, i, arraySize, enable);

            return VSC_ERR_NONE;
        }
    }

    return VSC_ERR_OUT_OF_RESOURCE;
}

gctBOOL VIR_CG_ConstUniformExistBefore(
    IN VIR_Shader       *pShader,
    IN VIR_Symbol       *pSym,
    IN OUT VIR_Uniform  *pSymUniform
    )
{
    gctBOOL isConstExistBefore = gcvFALSE, match = gcvFALSE;
    gctINT  i, index = 0, count;
    gctUINT8 swizzle = 0;
    gctBOOL valid[4] = {gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE};
    VIR_Uniform *uniform = gcvNULL;

    VIR_Const *pConstVal = (VIR_Const *) VIR_GetSymFromId(&pShader->constTable,
        pSymUniform->u.initializer);

    switch (pConstVal->type)
    {
    case VIR_TYPE_FLOAT_X32:
    case VIR_TYPE_FLOAT_X16:
    case VIR_TYPE_FLOAT_X8:
    case VIR_TYPE_INTEGER_X32:
    case VIR_TYPE_INTEGER_X16:
    case VIR_TYPE_INTEGER_X8:
    case VIR_TYPE_UINT_X32:
    case VIR_TYPE_UINT_X16:
    case VIR_TYPE_UINT_X8:
        return gcvFALSE;
    case VIR_TYPE_FLOAT_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_FLOAT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INTEGER_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INTEGER_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INTEGER_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT32:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT16_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT16:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT16_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT16:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_UINT8_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_UINT8:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_INT8_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_INT8:
        valid[0] = gcvTRUE;
        break;
    case VIR_TYPE_BOOLEAN_X4:
        valid[3] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN_X3:
        valid[2] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN_X2:
        valid[1] = gcvTRUE;
        /* fall through */
    case VIR_TYPE_BOOLEAN:
        valid[0] = gcvTRUE;
        break;
    default:
        gcmASSERT(gcvFALSE);
    }

    count = VIR_GetTypeComponents(pConstVal->type);

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);
        uniform = VIR_Symbol_GetUniform(sym);

        if (uniform == gcvNULL || (pSymUniform == uniform))
        {
            continue;
        }

        if (isSymUniformCompiletimeInitialized(sym) &&
            (uniform->physical != -1))
        {
            VIR_Const *constVal = (VIR_Const *) VIR_GetSymFromId(
                &pShader->constTable, uniform->u.initializer);
            gctINT constCount = VIR_GetTypeComponents(constVal->type);

            gcmASSERT(constCount > 0);

            switch (count)
            {
            case 1:
                for (index = 0; index < constCount; ++index)
                {
                    if (pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 2:
                for (index = 0; index < constCount - 1; ++index)
                {
                    if ((!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                    &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                    )
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 3:
                for (index = 0; index < constCount - 2; ++index)
                {
                    if ((!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                    &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                    &&  (!valid[2] || pConstVal->value.vecVal.u32Value[2] == constVal->value.vecVal.u32Value[index + 2])
                    )
                    {
                        match = gcvTRUE;
                        break;
                    }
                }
                break;

            case 4:
                index = 0;

                if (constCount == count
                &&  (!valid[0] || pConstVal->value.vecVal.u32Value[0] == constVal->value.vecVal.u32Value[index])
                &&  (!valid[1] || pConstVal->value.vecVal.u32Value[1] == constVal->value.vecVal.u32Value[index + 1])
                &&  (!valid[2] || pConstVal->value.vecVal.u32Value[2] == constVal->value.vecVal.u32Value[index + 2])
                &&  (!valid[3] || pConstVal->value.vecVal.u32Value[3] == constVal->value.vecVal.u32Value[index + 3])
                )
                {
                    match = gcvTRUE;
                }
                break;
            }

            if (match)
            {
                isConstExistBefore = gcvTRUE;
                break;
            }
        }
    }

    if (isConstExistBefore)
    {
        /* if there is a const uniform already exit, use the existed uniform */
        pSymUniform->physical = uniform->physical;
        pSymUniform->address = uniform->address;
        swizzle = uniform->swizzle >> (index * 2);
        switch (count)
        {
        case 1:
            swizzle = (swizzle & 0x03) | ((swizzle & 0x03) << 2);
            /* fall through */
        case 2:
            swizzle = (swizzle & 0x0F) | ((swizzle & 0x0C) << 2);
            /* fall through */
        case 3:
            swizzle = (swizzle & 0x3F) | ((swizzle & 0x30) << 2);
            /* fall through */
        default:
            break;
        }
        pSymUniform->swizzle = swizzle;
    }

    return isConstExistBefore;
}

VSC_ErrCode VIR_CG_MapNonSamplerUniforms(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN gctINT           UniformIndex,
    IN gctBOOL          Initialized,
    IN VIR_RA_ColorMap  *uniformColorMap,
    IN gctINT           codeGenUniformBase,
    IN gctBOOL          TreatSamplerAsConst,
    OUT gctINT          *NextUniformIndex
    )
{
    VSC_ErrCode retValue = VSC_ERR_NONE;

    gctINT    shift = 0, lastUniformIndex, i;
    VIR_TypeId type = VIR_TYPE_FLOAT_X4;
    gctINT arraySize = 0, physical = 0;
    gctSIZE_T maxComp = 0;
    gctUINT8 swizzle = 0;
    gctBOOL unblockUniformBlock = gcvFALSE;
    gctBOOL handleDefaultUBO = gcvFALSE;
    gctBOOL restricted = gcvFALSE;

    /* Determine base address for uniforms. */
    const gctUINT32 uniformBaseAddress = codeGenUniformBase * 4;

    VSC_GetUniformIndexingRange(pShader,
                                UniformIndex,
                                &lastUniformIndex);

    if (TreatSamplerAsConst)
    {
        lastUniformIndex = UniformIndex;
    }

    if (VIR_IdList_Count(&pShader->uniformBlocks) > 0)
    {
        if (pShader->_enableDefaultUBO  &&
            pHwConfig->hwFeatureFlags.hasHalti1)
        {
            handleDefaultUBO = gcvTRUE;
        }
        else if (!pHwConfig->hwFeatureFlags.hasHalti1)
        {
            unblockUniformBlock = gcvTRUE;
        }
        else
        {
            unblockUniformBlock = gcvFALSE;
        }
    }

    for (i = UniformIndex; i <= lastUniformIndex; i ++)
    {
        /* Get uniform. */
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;

        if (!isSymUniformUsedInShader(sym) &&
            !isSymUniformImplicitlyUsed(sym) &&
            VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
        {
            continue;
        }

        if (VIR_Symbol_isImage(sym))
        {
            symUniform = VIR_Symbol_GetImage(sym);
        }
        else if (VIR_Symbol_isSampler(sym))
        {
            if (TreatSamplerAsConst)
            {
                symUniform = VIR_Symbol_GetSampler(sym);
            }
        }
        else
        {
            symUniform = VIR_Symbol_GetUniform(sym);
        }

        if (symUniform == gcvNULL)
        {
            continue;
        }

        switch (VIR_Symbol_GetUniformKind(sym))
        {
        case VIR_UNIFORM_NORMAL:
        case VIR_UNIFORM_LOD_MIN_MAX:
        case VIR_UNIFORM_LEVEL_BASE_SIZE:
        case VIR_UNIFORM_STORAGE_BLOCK_ADDRESS:
        case VIR_UNIFORM_GLOBAL_SIZE:
        case VIR_UNIFORM_LOCAL_SIZE:
        case VIR_UNIFORM_NUM_GROUPS:
        case VIR_UNIFORM_GLOBAL_OFFSET:
        case VIR_UNIFORM_WORK_DIM:
        case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
        case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        case VIR_UNIFORM_SAMPLE_LOCATION:
        case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
            if (isSymUniformMovedToAUBO(sym))
            {
                continue;
            }
            break;

        case VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS:
            if (handleDefaultUBO)
            {
                if (!isSymUniformUsedInShader(sym))
                {
                    continue;
                }
            }
            else if (unblockUniformBlock)
            {
                continue;
            }
            break;

        case VIR_UNIFORM_BLOCK_MEMBER:
            if(handleDefaultUBO)
            {
                if(!isSymUniformMovedToDUB(sym))
                {
                    continue;
                }
            }
            else if(!unblockUniformBlock)
            {
                continue;
            }
            break;

        default:
            continue;
        }

        /*
        ** When allocate constant register for #num_group, we should always allocate XYZ for it
        ** because HW always use XYZ for this uniform.
        */
        if (VIR_Symbol_GetUniformKind(sym) == VIR_UNIFORM_NUM_GROUPS)
            restricted = gcvTRUE;

        if (!VIR_Symbol_isSampler(sym) || TreatSamplerAsConst)
        {
            gctUINT32 components = 0, rows = 0;
            VIR_Type    *symType = VIR_Symbol_GetType(sym);

            components = VIR_Symbol_GetComponents(sym);

            if (symUniform->realUseArraySize == -1)
            {
                rows = VIR_Type_GetVirRegCount(pShader, symType);

                if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
                {
                    symUniform->realUseArraySize = VIR_Type_GetArrayLength(symType);
                }
                else
                {
                    symUniform->realUseArraySize = 1;
                }
            }
            else
            {
                VIR_Type * baseType = VIR_Shader_GetTypeFromId(pShader,
                                                                VIR_Type_GetBaseTypeId(symType));
                rows = symUniform->realUseArraySize * VIR_Type_GetVirRegCount(pShader, baseType);
            }

            if (maxComp < components)
                maxComp = components;

            arraySize += rows;
        }
    }

    switch (maxComp)
    {
    case 1:
        type = VIR_TYPE_FLOAT32;
        break;
    case 2:
        type = VIR_TYPE_FLOAT_X2;
        break;
    case 3:
        type = VIR_TYPE_FLOAT_X3;
        break;
    case 4:
        type = VIR_TYPE_FLOAT_X4;
        break;
    case 8:
        type = VIR_TYPE_FLOAT_X4;
        arraySize *= 2;
        break;
    case 16:
        type = VIR_TYPE_FLOAT_X4;
        arraySize *= 4;
        break;
    case 32:
        type = VIR_TYPE_FLOAT_X4;
        arraySize *= 8;
        break;
    default:
        gcmASSERT(0);
        if (NextUniformIndex)
        {
            *NextUniformIndex = lastUniformIndex + 1;
        }
        return retValue;
    }

    if (arraySize > 0)
    {
        if (Initialized)
        {
            /* Get uniform. */
            VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, UniformIndex);
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
            VIR_Uniform *symUniform = VIR_Symbol_GetUniform(sym);
            gcmASSERT(symUniform != gcvNULL);

            if (!VIR_CG_ConstUniformExistBefore(pShader, sym, symUniform))
            {
                VIR_CG_FindUniformUse(uniformColorMap,
                                      type,
                                      1,
                                      gcvFALSE,
                                      &physical,
                                      &swizzle,
                                      &shift);

                symUniform->swizzle  = swizzle;
                symUniform->physical = physical;
                symUniform->address  = uniformBaseAddress +
                                       symUniform->physical * 16 + shift * 4;
            }

        }
        else
        {
            VIR_CG_FindUniformUse(uniformColorMap,
                                  type,
                                  arraySize,
                                  restricted,
                                  &physical,
                                  &swizzle,
                                  &shift);

            /* Set physical address for uniform. */
            for (i = UniformIndex; i <= lastUniformIndex; i ++)
            {
                /* Get uniform. */
                VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
                VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
                VIR_Uniform *symUniform = gcvNULL;

                if (!isSymUniformUsedInShader(sym) &&
                    VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
                {
                    continue;
                }

                if (VIR_Symbol_isImage(sym))
                {
                    symUniform = VIR_Symbol_GetImage(sym);
                }
                else if (VIR_Symbol_isSampler(sym))
                {
                    if (TreatSamplerAsConst);
                    {
                        symUniform = VIR_Symbol_GetSampler(sym);
                    }
                }
                else
                {
                    symUniform = VIR_Symbol_GetUniform(sym);
                }

                if (symUniform == gcvNULL)
                {
                    gcmASSERT(VIR_Symbol_isSampler(sym));
                    continue;
                }

                switch (VIR_Symbol_GetUniformKind(sym))
                {
                case VIR_UNIFORM_NORMAL:
                case VIR_UNIFORM_LOD_MIN_MAX:
                case VIR_UNIFORM_LEVEL_BASE_SIZE:
                case VIR_UNIFORM_STORAGE_BLOCK_ADDRESS:
                case VIR_UNIFORM_GLOBAL_SIZE:
                case VIR_UNIFORM_LOCAL_SIZE:
                case VIR_UNIFORM_NUM_GROUPS:
                case VIR_UNIFORM_GLOBAL_OFFSET:
                case VIR_UNIFORM_WORK_DIM:
                case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
                case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
                case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
                case VIR_UNIFORM_SAMPLE_LOCATION:
                case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
                case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
                    if (isSymUniformMovedToAUBO(sym))
                    {
                        continue;
                    }
                    break;

                case VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS:
                    if (handleDefaultUBO)
                    {
                        if (!isSymUniformUsedInShader(sym))
                        {
                            continue;
                        }
                    }else if (unblockUniformBlock)
                    {
                        continue;
                    }
                    break;

                case VIR_UNIFORM_BLOCK_MEMBER:
                    if(handleDefaultUBO)
                    {
                        if(!isSymUniformMovedToDUB(sym))
                        {
                            continue;
                        }
                    }
                    else if(!unblockUniformBlock)
                    {
                        continue;
                    }
                    break;

                default:
                    continue;
                }

                if (VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER))
                {
                    gcmASSERT(symUniform->baseBindingUniform != gcvNULL);

                    if (symUniform->baseBindingUniform->physical == -1)
                    {
                        gctUINT32    rows           = 0;
                        VIR_Uniform *baseUniform    = symUniform->baseBindingUniform;
                        VIR_Symbol  *baseUniformSym = VIR_Shader_GetSymFromId(pShader, baseUniform->sym);
                        VIR_Type    *symType        = VIR_Symbol_GetType(baseUniformSym);

                        baseUniform->swizzle  = swizzle;
                        baseUniform->physical = physical;
                        baseUniform->address  = uniformBaseAddress +
                            baseUniform->physical * 16 + shift * 4;

                        rows = VIR_Type_GetVirRegCount(pShader, symType);
                        physical += rows;
                    }

                    symUniform->swizzle  = symUniform->baseBindingUniform->swizzle;
                    symUniform->physical = symUniform->baseBindingUniform->physical;
                    symUniform->address  = symUniform->baseBindingUniform->address;
                }
                else if (!VIR_Symbol_isSampler(sym) || TreatSamplerAsConst)
                {
                    gctUINT32 rows = 0;
                    VIR_Type  *symType = VIR_Symbol_GetType(sym);

                    symUniform->swizzle = swizzle;
                    symUniform->physical = physical;
                    symUniform->address = uniformBaseAddress +
                                          symUniform->physical * 16 + shift * 4;
                    rows = VIR_Type_GetVirRegCount(pShader, symType);
                    physical += rows;
                }
            }
        }
    }

    if (NextUniformIndex)
        *NextUniformIndex = lastUniformIndex + 1;

    return retValue;
}

VSC_ErrCode VIR_CG_MapUniforms(
    IN VIR_Shader       *pShader,
    IN VSC_HW_CONFIG    *pHwConfig,
    IN VSC_MM           *pMM
    )
{
    VSC_ErrCode         retValue = VSC_ERR_NONE;
    gctINT              i, nextUniformIndex = 0;
    gctBOOL             unblockUniformBlock = gcvFALSE;
    gctBOOL             handleDefaultUBO = gcvFALSE;
    gctBOOL             allocateSamplerReverse = gcvFALSE;
    gctUINT32           vsSamplers = 0, psSamplers = 0;
    gctINT              maxSampler = 0, sampler = 0, samplerRegNoBase = 0;
    VIR_RA_ColorMap     uniformColorMap;
    gctUINT             codeGenUniformBase;
    VIR_ShaderKind      shaderKind = VIR_Shader_GetKind(pShader);
    gctINT              samplerCount = 0;

    /* initialize the colorMap */
    VIR_CG_UniformColorMap_Init(
        pShader,
        pHwConfig,
        pMM,
        &uniformColorMap,
        &codeGenUniformBase);

    /* Config sampler setting. */
    if (VIR_Shader_isPackUnifiedSampler(pShader))
    {
        samplerRegNoBase = VIR_Shader_GetSamplerBaseOffset(pShader);
        VIR_Shader_CalcSamplerCount(pShader, &samplerCount);

        gcmASSERT(samplerRegNoBase != -1);

        /*
        ** Set the max sampler count and the start index.
        ** If this chip can support sampler base offset, all sampler index begin with 0,
        ** and we need to program samplerRegNoBase, otherwise, we set samplerRegNoBase to 0.
        */
        if (VIR_Shader_IsGPipe(pShader))
        {
            allocateSamplerReverse = gcvTRUE;

            if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
            {
                maxSampler = 0;
                sampler = samplerCount - 1;
            }
            else
            {
                maxSampler = samplerRegNoBase;
                sampler = samplerRegNoBase + samplerCount - 1;
                samplerRegNoBase = 0;
            }
        }
        else
        {
            maxSampler = samplerCount;
            sampler = 0;
        }
    }
    else if (pHwConfig->hwFeatureFlags.hasSamplerBaseOffset)
    {
        sampler = 0;
        switch (shaderKind)
        {
        case VIR_SHADER_FRAGMENT:
            maxSampler = pHwConfig->maxPSSamplerCount;
            samplerRegNoBase = pHwConfig->psSamplerRegNoBase;
            break;

        case VIR_SHADER_CL:
        case VIR_SHADER_COMPUTE:
            maxSampler = pHwConfig->maxCSSamplerCount;
            samplerRegNoBase = pHwConfig->csSamplerRegNoBase;
            break;

        case VIR_SHADER_VERTEX:
            maxSampler = pHwConfig->maxVSSamplerCount;
            samplerRegNoBase = pHwConfig->vsSamplerRegNoBase;
            break;

        case VIR_SHADER_TESSELLATION_CONTROL:
            maxSampler = pHwConfig->maxTCSSamplerCount;
            samplerRegNoBase = pHwConfig->tcsSamplerRegNoBase;
            break;

        case VIR_SHADER_TESSELLATION_EVALUATION:
            maxSampler = pHwConfig->maxTESSamplerCount;
            samplerRegNoBase = pHwConfig->tesSamplerRegNoBase;
            break;

        case VIR_SHADER_GEOMETRY:
            maxSampler = pHwConfig->maxGSSamplerCount;
            samplerRegNoBase = pHwConfig->gsSamplerRegNoBase;
            break;

        default:
            gcmASSERT(0);
            maxSampler = pHwConfig->maxPSSamplerCount;
            samplerRegNoBase = pHwConfig->psSamplerRegNoBase;
            break;
        }
    }
    else
    {
        /* map Sampler */
        vsSamplers = pHwConfig->maxVSSamplerCount;
        psSamplers = pHwConfig->maxPSSamplerCount;

        /* Determine starting sampler index. */
        sampler = (shaderKind == VIR_SHADER_VERTEX)
                ? psSamplers
                : 0;

        /* Determine maximum sampler index. */
        /* Note that CL kernel can use all samplers if unified. */
        maxSampler = (shaderKind == VIR_SHADER_FRAGMENT)
                   ? psSamplers
                   : psSamplers + vsSamplers;

        samplerRegNoBase = 0;
    }
    /* Update the sampler base offset. */
    VIR_Shader_SetSamplerBaseOffset(pShader, samplerRegNoBase);

    /* set the handleDefaultUBO flag */
    if (VIR_IdList_Count(&pShader->uniformBlocks) > 0)
    {
        if (pShader->_enableDefaultUBO  &&
            pHwConfig->hwFeatureFlags.hasHalti1)
        {
            handleDefaultUBO = gcvTRUE;
        }
        else if (!pHwConfig->hwFeatureFlags.hasHalti1)
        {
            unblockUniformBlock = gcvTRUE;
        }
        else
        {
            unblockUniformBlock = gcvFALSE;
        }
    }

    /* check uniform usage: if a uniform is used in shader or LTC expression */
    VSC_CheckUniformUsage(pShader);

    /* Map all uniforms. */
    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Uniform *symUniform = gcvNULL;
        VIR_PrimitiveTypeId baseType;

        if (VIR_Symbol_isUniform(sym))
        {
           symUniform = VIR_Symbol_GetUniform(sym);
        }
        else if (VIR_Symbol_isSampler(sym))
        {
            symUniform = VIR_Symbol_GetSampler(sym);
        }
        else if (VIR_Symbol_isImage(sym))
        {
            symUniform = VIR_Symbol_GetImage(sym);
        }

        if (symUniform == gcvNULL)
        {
            continue;
        }

        switch (VIR_Symbol_GetUniformKind(sym))
        {
        case VIR_UNIFORM_NORMAL:
        case VIR_UNIFORM_LOD_MIN_MAX:
        case VIR_UNIFORM_LEVEL_BASE_SIZE:
        case VIR_UNIFORM_STORAGE_BLOCK_ADDRESS:
        case VIR_UNIFORM_GLOBAL_SIZE:
        case VIR_UNIFORM_LOCAL_SIZE:
        case VIR_UNIFORM_NUM_GROUPS:
        case VIR_UNIFORM_GLOBAL_OFFSET:
        case VIR_UNIFORM_WORK_DIM:
        case VIR_UNIFORM_KERNEL_ARG_CONSTANT:
        case VIR_UNIFORM_KERNEL_ARG_LOCAL_MEM_SIZE:
        case VIR_UNIFORM_KERNEL_ARG_PRIVATE:
        case VIR_UNIFORM_SAMPLE_LOCATION:
        case VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS:
        case VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS:
            if (isSymUniformMovedToAUBO(sym))
            {
                continue;
            }
            break;

        case VIR_UNIFORM_UNIFORM_BLOCK_ADDRESS:
            if (handleDefaultUBO)
            {
                if (!(isSymUniformUsedInShader(sym)))
                {
                    continue;
                }
            }
            else if (unblockUniformBlock)
            {
                continue;
            }
            break;

        case VIR_UNIFORM_BLOCK_MEMBER:
            if(handleDefaultUBO)
            {
                if(!isSymUniformMovedToDUB(sym))
                {
                    continue;
                }
            }
            else if(!unblockUniformBlock)
            {
                continue;
            }
            break;

        default:
            continue;
        }

        baseType = VIR_Type_GetBaseTypeId(sym->type);

        switch(baseType)
        {
        case VIR_TYPE_SAMPLER_GENERIC:
        case VIR_TYPE_SAMPLER:
        case VIR_TYPE_SAMPLER_1D:
        case VIR_TYPE_SAMPLER_2D:
        case VIR_TYPE_SAMPLER_3D:
        case VIR_TYPE_SAMPLER_EXTERNAL_OES:
        case VIR_TYPE_SAMPLER_2D_SHADOW:
        case VIR_TYPE_SAMPLER_CUBE_SHADOW:
        case VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW:
        case VIR_TYPE_SAMPLER_CUBIC:
        case VIR_TYPE_SAMPLER_CUBE_ARRAY:
        case VIR_TYPE_SAMPLER_1D_ARRAY:
        case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
        case VIR_TYPE_SAMPLER_2D_ARRAY:
        case VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW:
        case VIR_TYPE_ISAMPLER_2D:
        case VIR_TYPE_ISAMPLER_2D_ARRAY:
        case VIR_TYPE_ISAMPLER_3D:
        case VIR_TYPE_ISAMPLER_CUBIC:
        case VIR_TYPE_ISAMPLER_CUBE_ARRAY:
        case VIR_TYPE_USAMPLER_2D:
        case VIR_TYPE_USAMPLER_2D_ARRAY:
        case VIR_TYPE_USAMPLER_3D:
        case VIR_TYPE_USAMPLER_CUBIC:
        case VIR_TYPE_USAMPLER_CUBE_ARRAY:
        case VIR_TYPE_SAMPLER_2D_MS:
        case VIR_TYPE_ISAMPLER_2D_MS:
        case VIR_TYPE_USAMPLER_2D_MS:
        case VIR_TYPE_SAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_ISAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_USAMPLER_2D_MS_ARRAY:
        case VIR_TYPE_SAMPLER_BUFFER:
        case VIR_TYPE_ISAMPLER_BUFFER:
        case VIR_TYPE_USAMPLER_BUFFER:

            /* This physical address of base sampler symbol is always 0. */
            if (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(pShader))
            {
                VIR_Uniform_SetPhysical(symUniform, 0);
                continue;
            }

            /* If this texture is not used on shader, we can skip it. */
            if (!(isSymUniformUsedInShader(sym)) &&
                !(isSymUniformUsedInTextureSize(sym)))
            {
                VIR_Uniform_SetPhysical(symUniform, -1);
                VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                continue;
            }

            VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

            if (isSymUniformTreatSamplerAsConst(sym))
            {
                VIR_CG_MapNonSamplerUniforms(pShader,
                    pHwConfig,
                    i,
                    gcvFALSE,
                    &uniformColorMap,
                    codeGenUniformBase,
                    gcvTRUE,
                    &nextUniformIndex);
                break;
            }

            if (symUniform->realUseArraySize == -1)
            {
                VIR_Type    *symType = VIR_Symbol_GetType(sym);

                if (VIR_Type_GetKind(symType) == VIR_TY_ARRAY)
                {
                    symUniform->realUseArraySize = VIR_Type_GetArrayLength(symType);
                }
                else
                {
                    symUniform->realUseArraySize = 1;
                }
            }

            /* Test if sampler is in range */
            if (allocateSamplerReverse)
            {
                if (sampler < maxSampler)
                {
                    retValue = VSC_ERR_OUT_OF_RESOURCE;
                }
                else
                {
                    gctINT arrayLength = 1;

                    if (VIR_Type_GetKind(sym->type) == VIR_TY_ARRAY)
                    {
                        arrayLength = (gctINT)VIR_Type_GetArrayLength(sym->type);
                    }

                    VIR_Uniform_SetPhysical(symUniform, sampler + 1 - arrayLength);
                    sampler -= arrayLength;
                }
            }
            else
            {
                if (sampler >= maxSampler)
                {
                    retValue = VSC_ERR_OUT_OF_RESOURCE;
                }
                else
                {
                    VIR_Uniform_SetPhysical(symUniform, sampler);

                    if (VIR_Type_GetKind(sym->type) == VIR_TY_ARRAY)
                    {
                        sampler += VIR_Type_GetArrayLength(sym->type);
                    }
                    else
                    {
                        sampler += 1;
                    }
                }
            }

            if (VIR_Uniform_GetPhysical(symUniform) != VIR_Uniform_GetOrigPhysical(symUniform))
            {
                VIR_Shader_SetNeedToAjustSamplerPhysical(pShader, gcvTRUE);
            }
            break;

        default:
            if (nextUniformIndex > i)
                continue;

            if (isSymUniformForcedToActive(sym))
            {
                VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);
            }

            if (!isSymUniformUsedInShader(sym) &&
                !isSymUniformUsedInLTC(sym) &&
                !isSymUniformImplicitlyUsed(sym) &&
                VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
            {
                if (!isSymUniformForcedToActive(sym))
                {
                    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_INACTIVE);
                }

                continue;
            }

            VIR_Symbol_ClrFlag(sym, VIR_SYMFLAG_INACTIVE);

            if (!isSymUniformUsedInShader(sym) &&
                isSymUniformUsedInLTC(sym) &&
                VIR_Symbol_GetUniformKind(sym) != VIR_UNIFORM_NUM_GROUPS)
            {
                /* skip uniforms only used in LTC. */
                continue;
            }

            VIR_CG_MapNonSamplerUniforms(pShader,
                pHwConfig,
                i,
                isSymUniformCompiletimeInitialized(sym),
                &uniformColorMap,
                codeGenUniformBase,
                gcvFALSE,
                &nextUniformIndex);
            break;
        }
    }

    vscBV_Destroy(&uniformColorMap.usedColor);

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
    else if (VIR_Symbol_GetName(pAttr) == VIR_NAME_VERTEX_ID)
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
             (pLR && isLRsubSampleDepth(pLR)))
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
            if (!VIR_Shader_GS_HasRestartOp(pShader))
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
            if (!VIR_Shader_GS_HasRestartOp(pShader) &&
                !VIR_Shader_HasInstanceId(pShader))
            {
                /*primitiveId in r0.z */
                if (regNo && shift)
                {
                    *regNo = 0;
                    *shift = 2;
                }
            }
            else if ((VIR_Shader_GS_HasRestartOp(pShader)) &&
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
    gctUINT     vtxInstanceIdRegNo = NOT_ASSIGNED;
    gctUINT     primIdWebIdx = NOT_ASSIGNED, ptCoordWebIdx = NOT_ASSIGNED;
    VIR_Symbol  *primIdAttr = gcvNULL, *ptCoordAttr = gcvNULL;

    VIR_RA_HWReg_Color  LRColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &LRColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &LRColor);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "\nAssign color for attributes\n");
        VIR_LOG_FLUSH(pDumper);
    }

    /* reserve registers per HW spec */
    if (pShader->shaderKind == VIR_SHADER_VERTEX)
    {
        /* Start at register 0 for vertex shaders */
        reg = 0;
    }
    else if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        /* Start at register 1 for fragment shaders,
           0 is reserved for builtin attributes,
           in dual16 mode, r0, r1 is position if position is in the shader
           */

        if (VIR_Shader_PS_NeedSampleMaskId(pShader))
        {
            gctUINT unusedChannel = _VIR_RA_Check_First_Unused_Pos_Attr_Channel(pRA);

            /* In dual16, Sample mask should always be in HIGHP. So sample ID and sample position should always be in HIGHP as well.
               Because,
               o   Medium position mode doesn’t support mask LOCATION_W  (can’t)and LOCATION_Z (make it simple).
               o   Sample depth is always high precision. So LOCATION_SUB_Z is also high precision. */
            if (VIR_Shader_isDual16Mode(pShader))
            {
                /* sampleMaskIdRegStart will be changed to last temp */
                pShader->sampleMaskIdRegStart = VIR_REG_MULTISAMPLEDEPTH;
                pShader->sampleMaskIdChannelStart = CHANNEL_X;
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
            reg = 2;
        }
        else
        {
            reg = 1;
        }
    }
    else if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
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

        /* if both input and output vertex are less than 4, we pack their regmap in one register. */
        if ((pShader->shaderLayout.tcs.tcsPatchInputVertices <= 4) &&
            (pShader->shaderLayout.tcs.tcsPatchOutputVertices <= 4))
        {
            VIR_Shader_SetFlag(pShader, VIR_SHFLAG_TCS_USE_PACKED_REMAP);
            /* input regmap and output regmap are packed in one register */
            reg = 1 + 1;
        }
        else
        {
            reg = 1 + inputCount + (pShader->shaderLayout.tcs.hasOutputVertexAccess ? outputCount : 0);
        }

        /* TCS remap always start from r1 */
        pShader->remapRegStart = 1;
        pShader->remapChannelStart = CHANNEL_X;
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
        gctINT  inputCount = ((pShader->shaderLayout.tes.tessPatchInputVertices -1) / 8 + 1);
        reg = 1 + inputCount;

        /* TES remap always start from r1 */
        pShader->remapRegStart = 1;
        pShader->remapChannelStart = CHANNEL_X;
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

        if (!VIR_Shader_GS_HasRestartOp(pShader) &&
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
                reg = 2;
            }
            else
            {
                pShader->remapRegStart = 0;
                pShader->remapChannelStart = CHANNEL_Z;
                reg = 1;
            }
        }
        else if ((VIR_Shader_GS_HasRestartOp(pShader)) &&
                  VIR_Shader_HasInstanceId(pShader) &&
                  VIR_Shader_HasPrimitiveId(pShader))
        {
            if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
            {
                pShader->remapRegStart = 2;
                pShader->remapChannelStart = CHANNEL_X;
                reg = 3;
            }
            else
            {
                pShader->remapRegStart = 1;
                pShader->remapChannelStart = CHANNEL_Z;
                reg = 2;
            }
        }
        else
        {
            pShader->remapRegStart = 1;
            pShader->remapChannelStart = CHANNEL_X;
            reg = 2;
        }
    }
    else if (pShader->shaderKind == VIR_SHADER_COMPUTE ||
             pShader->shaderKind == VIR_SHADER_CL)
    {
        reg = 0;
    }
    else
    {
        gcmASSERT(gcvFALSE);
        reg = 0;
    }
    pCP->colorMap[VIR_RA_HWREG_GR].availReg = reg;

    /* only allocate for used attributes */
    for (currAttr = 0; currAttr < attrCount; currAttr++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader,
                                    VIR_IdList_GetId(pAttrs, currAttr));
        VIR_Type    *attrType = VIR_Symbol_GetType(attribute);
        gctUINT     attrRegCount = VIR_Type_GetVirRegCount(pShader, attrType);
        gctUINT     components = VIR_Symbol_GetComponents(attribute);

        VIR_DEF_KEY     defKey;
        gctUINT         defIdx, webIdx;
        gctUINT         attFlags;
        gctUINT         curReg = reg;
        gctUINT8        shift = 0;
        gctBOOL         skipAlloc = gcvFALSE;

        VIR_RA_LS_Liverange *pLR = gcvNULL;

        /* we don't need to assign register for per-vertex/per-patch input,
           we will use load_attr to access them */
        if (_VIR_RA_LS_IsExcludedLR(pRA, gcvNULL, attribute, gcvNULL, gcvNULL))
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
            pLR->regNoRange = VIR_Type_GetVirRegCount(pShader, attrType);

            /* the attributes that is put into special register */
            if ((VIR_Symbol_GetName(attribute) == VIR_NAME_VERTEX_ID ||
                 VIR_Symbol_GetName(attribute) == VIR_NAME_INSTANCE_ID) &&
                VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.vtxInstanceIdAsAttr == gcvFALSE)
            {
                gcmASSERT(VIR_Symbol_GetPrecision(attribute) != VIR_PRECISION_HIGH);

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

                    vtxInstanceIdRegNo = curReg;
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
                VIR_Symbol_SetHwRegId(attribute, curReg);
                if (VIR_Shader_isDual16Mode(pShader) &&
                    VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
                {
                    switch (components)
                    {
                    case 1:
                    case 2:
                        if (VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.highpVaryingShift)
                        {
                            VIR_Symbol_SetHIHwRegId(attribute, curReg);
                            VIR_Symbol_SetHIHwShift(attribute, 2);
                        }
                        else
                        {
                            VIR_Symbol_SetHIHwRegId(attribute, curReg + 1);
                            VIR_Symbol_SetHIHwShift(attribute, 0);
                        }

                        break;
                    case 3:
                    case 4:
                        VIR_Symbol_SetHIHwRegId(attribute, curReg + 1);
                        VIR_Symbol_SetHIHwShift(attribute, 0);
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
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
                attFlags = (gctUINT) VIR_Symbol_GetFlag(attribute);
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

    /* assert VIR_NAME_VERTEX_ID/VIR_NAME_INSTANCE_ID are last one */
    if (vtxInstanceIdRegNo != NOT_ASSIGNED)
    {
        gcmASSERT(vtxInstanceIdRegNo == reg);
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
        _VIR_RA_LS_SetUsedColor(pRA, VIR_RA_HWREG_GR, 0,
            (pShader->sampleMaskIdRegStart == 0 && VIR_Shader_PS_NeedSampleMaskId(pShader)) ? 0xF : 0x3);
        _VIR_RA_MakeColor(0, 0, &LRColor);
        _VIR_RA_LS_SetMaxAllocReg(pRA, LRColor, VIR_RA_HWREG_GR, 1);
    }

    return retValue;
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

    /* process the live out for this BB as following:
       tempVec = xor(LRLiveVec, outFlow)
       for each 1 in tempVec
        if thisDefId in outFlow (i.e., LR live_out but not live yet)
            make the LR live (update the LRLiveVec)
        else (i.e., LR is live but not live_out any more)
            make the LR dead (update the LRLiveVec)
      to-do need to reset the channel mask */
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

        if (_VIR_RA_LS_IsExcludedLR(pRA, gcvNULL, gcvNULL, pDef, gcvNULL))
        {
            continue;
        }

        if (vscBV_TestBit(&pBlkFlow->outFlow, thisDefId))
        {
            _VIR_RS_LS_MarkLRLive(pRA, thisDefId, (0x1 << pDef->defKey.channel), gcvTRUE);
            _VIR_RS_LS_SetLiveLRVec(pRA, thisDefId);
        }
        else
        {
            /* need to change from channel to enable */
            if (_VIR_RS_LS_MaskMatch(pRA, (0x1 << pDef->defKey.channel), thisDefId))
            {
                _VIR_RS_LS_MarkLRDead(pRA, thisDefId, (0x1 << pDef->defKey.channel), gcvTRUE);
            }
            _VIR_RS_LS_UnsetLiveLRVec(pRA, thisDefId);
        }
    }

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
            _VIR_RA_LS_MarkDef(pRA, pInst, firstRegNo, regNoRange, defEnableMask, bDstIndexing);
        }

        /* mark live for all uses */
        _VIR_RA_LS_MarkUses(pRA, pInst);

        /* Emit has implicit usage for all outputs, so we also gen these */
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT ||
            VIR_Inst_GetOpcode(pInst) == VIR_OP_AQ_EMIT)
        {
            _VIR_RA_LS_Mark_Outputs(pRA, pInst);
        }

        curPos = VIR_RA_LS_GetCurrPos(pRA);
        VIR_RA_LS_SetCurrPos(pRA, curPos-1);

        /* set instruction id, for dump liveness with inst id */
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
            if(pBB != CFG_GET_ENTRY_BB(pCfg) && pBB != CFG_GET_EXIT_BB(pCfg))
            {
                retValue = _VIR_RA_LS_BuildLRTableBB(pRA, pBB, pFuncFlow);
                CHECK_ERROR(retValue, "_VIR_RA_LS_BuildLRTableBB");
                if (retValue)
                {
                    break;
                }
            }
        }

        _VIR_RA_LS_SetMasterLR(pRA);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_BUILD_LR))
    {
        VIR_LOG(pDumper, "\n============== liverange table [%s] ==============\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_RS_LS_DumpLRTable(pRA, pFunc);
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
            /* find all defs */
            defIdx = pWeb->firstDefIdx;
            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pLvInfo->pDuInfo->defTable, defIdx);
                usageCount += DU_CHAIN_GET_USAGE_COUNT(&pDef->duChain);
                defIdx = pDef->nextDefInWebIdx;
            }

            pLR->weight = (gctFLOAT)(pLR->endPoint - pLR->startPoint) / (gctFLOAT)(pWeb->numOfDef + usageCount);
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
   _VIR_RA_LS_AssignColors
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
    VIR_RA_HWReg_Color  *color)
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

    if (pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg + regNoRange < pCP->colorMap[VIR_RA_HWREG_GR].maxReg)
    {
        gctUINT regNo = pCP->colorMap[VIR_RA_HWREG_GR].maxAllocReg + 1;
        _VIR_RA_MakeColor(regNo, 0, color);
        _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, color);

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
        {
            VIR_LOG(pDumper, "find brand new ");
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
                VIR_LOG(pDumper, " to be used because of web[%d]\n",
                    pLR->webIdx);
                VIR_LOG_FLUSH(pDumper);
            }
        }
    }
}

VSC_ErrCode  _VIR_RA_LS_AssignColorLR(
    VIR_RA_LS           *pRA,
    VIR_Function        *pFunc,
    VIR_RA_LS_Liverange *pLR,
    gctUINT             reservedDataReg)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);

    gctUINT             webIdx = pLR->webIdx;
    VIR_WEB             *pWeb = GET_WEB_BY_IDX(&pRA->pLvInfo->pDuInfo->webTable, webIdx);
    VIR_RA_HWReg_Color  curColor;
    gctBOOL             newColor = gcvFALSE, needHI = gcvFALSE;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

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
            if (isLRsubSampleDepth(pLR))
            {
                _VIR_RA_MakeColor(VIR_REG_MULTISAMPLEDEPTH, 0, &curColor);
                _VIR_RA_MakeColorHI(VIR_REG_MULTISAMPLEDEPTH, 0, &curColor);
            }
            else
            {
                VIR_DEF *pDefTemp = GET_DEF_BY_IDX(&pRA->pLvInfo->pDuInfo->defTable, pWeb->firstDefIdx);
                if (pDefTemp->flags.bHwSpecialInput || pDefTemp->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST)
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
                    curColor = _VIR_RA_LS_FindNewColor(pRA, webIdx, needHI, reservedDataReg);
                    newColor = gcvTRUE;
                }
            }

            if (_VIR_RA_LS_IsInvalidLOWColor(curColor) ||
                (needHI && _VIR_RA_LS_IsInvalidHIColor(curColor)))
            {
                /* we could not find a color */
                if (pLR->hwType == VIR_RA_HWREG_A0)
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

                if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)) &&
                    pLR->regNoRange > 1)
                {
                    highDiff = _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) - _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR));
                }

                /* the color is assigned in other place, try to see whether
                    there is any conflict */
                for (regIdx = 0; regIdx < pLR->regNoRange; regIdx++)
                {
                    if (_VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                            _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)) + regIdx * (highDiff + 1),
                            _VIR_RA_Color_Channels(
                                VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                                _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)))))
                    {
                        conflict = gcvTRUE;
                    }

                    if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pLR)))
                    {
                        if (_VIR_RA_LS_TestUsedColor(pRA, pLR->hwType,
                            _VIR_RA_Color_HIRegNo(_VIR_RA_GetLRColor(pLR)) + regIdx * (highDiff + 1),
                            _VIR_RA_Color_Channels(
                                VIR_RA_LS_LR2WebChannelMask(pRA, pLR),
                                _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pLR)))))
                        {
                            conflictHI = gcvTRUE;
                        }
                    }
                }
                /* if conflict, we need to find a brand new color for it */
                if (conflict)
                {
                    if (_VIR_RA_LS_FindBrandnewColor(pRA, pLR, &curColor))
                    {
                        _VIR_RA_SetLRColor(pLR,
                            _VIR_RA_Color_RegNo(curColor),
                            _VIR_RA_Color_Shift(curColor));
                    }
                    /* we could not find a color */
                    else if (reservedDataReg == 0)
                    {
                        gcmASSERT(pLR->hwType != VIR_RA_HWREG_A0);
                        /* have not tried spill yet */
                        retValue = VSC_RA_ERR_OUT_OF_REG_SPILL;
                        return retValue;
                    }
                    else
                    {
                        /* already tried spill, could not assign color */
                        retValue = VSC_RA_ERR_OUT_OF_REG_FAIL;
                        return retValue;
                    }
                }
                if (conflictHI)
                {
                    if (_VIR_RA_LS_FindBrandnewColor(pRA, pLR, &curColor))
                    {
                        _VIR_RA_SetLRColorHI(pLR,
                            _VIR_RA_Color_RegNo(curColor),
                            _VIR_RA_Color_Shift(curColor));
                    }
                    else if (reservedDataReg == 0)
                    {
                        gcmASSERT(pLR->hwType != VIR_RA_HWREG_A0);
                        /* have not tried spill yet */
                        retValue = VSC_RA_ERR_OUT_OF_REG_SPILL;
                        return retValue;
                    }
                    else
                    {
                        /* already tried spill, could not assign color */
                        retValue = VSC_RA_ERR_OUT_OF_REG_FAIL;
                        return retValue;
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

VSC_ErrCode _VIR_RA_LS_AssignColors(
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
        if (_VIR_RA_LS_IsExcludedLR(pRA, gcvNULL, gcvNULL, pDef, gcvNULL))
        {
            continue;
        }

        pLR = _VIR_RA_LS_Def2LR(pRA, defIdx);
        retValue = _VIR_RA_LS_AssignColorLR(pRA, pFunc, pLR, reservedDataReg);

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

        if (!isLRInvalid(pLR))
        {
            if (_VIR_RA_LS_IsInvalidColor(_VIR_RA_GetLRColor(pLR)))
            {
                _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
                _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);
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
                   !VIR_Shader_isDual16Mode(pShader))
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

    VIR_Operand_SetRegAllocated(pOpnd, 1);

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
            /* ps output gl_layer must be allocated to r0.w */
            if (VIR_Symbol_GetName(varSym) == VIR_NAME_PS_OUT_LAYER &&
                pShader->shaderKind == VIR_SHADER_FRAGMENT)
            {
                /* r0.w is not allocated */
                gcmASSERT(_VIR_RA_LS_TestUsedColor(pRA, VIR_RA_HWREG_GR, 0, 0x8) == gcvFALSE);
                VIR_Symbol_SetHwRegId(varSym, 0);
                VIR_Symbol_SetHwShift(varSym, 3);
                _VIR_RA_SetLRColor(pLR, 0, 3);
                gcmASSERT(VIR_Symbol_GetPrecision(varSym) != VIR_PRECISION_HIGH);
                _VIR_RA_SetLRColorHI(pLR, VIR_RA_INVALID_REG, 0);
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
                VIR_Operand_SetType(pInst->dest, VIR_Operand_GetType(pDef->defKey.pDefInst->dest));
                break;
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
    VIR_Instruction *pInst,
    VIR_Operand     *pSrcOpnd,
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
_VIR_RA_LS_ComputeSpillOffset(
    VIR_RA_LS           *pRA,
    VIR_Operand         *pOpnd,
    VIR_RA_LS_Liverange *pLR
    )
{
    VIR_Symbol  *opndSym = VIR_Operand_GetSymbol(pOpnd);
    gctUINT     spillOffset = 0, swizzleOffset = 0, constOffset = 0;

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
    spillOffset = _VIR_RA_GetLRSpillOffset(pLR) +
        (opndSym->u1.vregIndex - pLR->firstRegNo + constOffset) * 16 + swizzleOffset * 4;
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
        gcmASSERT((VIR_Swizzle_GetChannel(srcSwizzle, i) - swizzleShift) >= 0);
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
            gcmASSERT((i - enableShift) >= 0);

            newEnable = newEnable | (1 << (i - enableShift));
        }
    }

    return newEnable;
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
    VIR_SymId           symId;
    VIR_RA_HWReg_Color  curColor;
    gctUINT             i = 0;
    VIR_Swizzle         newSwizzle  = _VIR_RA_LS_SwizzleWShift(pOpnd);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "instruction:\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_LOAD_S,
        VIR_Operand_GetType(pOpnd),
        pInst,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 offset */
    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* set the dest for the newInst */
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_Operand_GetType(pOpnd));
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    for (i = 0 ; i < VIR_MAX_SRC_NUM; i++)
    {
        if (!pRA->dataRegisterUsed[i])
        {
            gcmASSERT(pRA->dataRegister[i] != VIR_INVALID_ID);
            _VIR_RA_MakeColor(pRA->dataRegister[i], 0, &curColor);
            pRA->dataRegisterUsed[i] = gcvTRUE;
            break;
        }
    }
    gcmASSERT(i < VIR_MAX_SRC_NUM);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
    VIR_Operand_SetEnable(newInst->dest, VIR_Swizzle_2_Enable(newSwizzle));

    /* set the src for the pInst */
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                symId,
                                VIR_Operand_GetType(pOpnd));
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
    VIR_RA_HWReg_Color  curColor;
    VIR_Operand         *newOpnd = gcvNULL;

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
    {
        VIR_LOG(pDumper, "instruction:\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_STORE_S,
        VIR_Operand_GetType(pOpnd),
        pInst,
        &newInst);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* compute offset */
    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* src2 */
    retErrCode = VIR_Function_DupOperand(pFunc, pOpnd, &newOpnd);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    VIR_Operand_Change2Src(newOpnd);
    newInst->src[VIR_Operand_Src2]  = newOpnd;

    /* fill (destination) can always use dataRegister[0] */
    gcmASSERT(pRA->dataRegister[0] != VIR_INVALID_ID);
    _VIR_RA_MakeColor(pRA->dataRegister[0], 0, &curColor);
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
    1: mad  offset.x, t1.x, 16, spillOffset
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
    VIR_Operand         *src1Opnd, *idxOpnd;
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_DEF             *pDef;
    VIR_SymId           tmpSymId;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* mad instruction */
    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_MAD,
        VIR_TYPE_UINT32,
        pInst,
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
    vscVIR_InitGeneralUdIterator(&udIter,
        pLvInfo->pDuInfo, pInst, idxOpnd, gcvFALSE);
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
                        pDef->defKey.pDefInst,
                        gcvNULL));
            break;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    /* insert a MOV after MOVA, in case mova_src0 is redefined,
       to-do: insert only when redefined */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_MOV,
        VIR_TYPE_UINT32,
        pDef->defKey.pDefInst,
        &movInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    retErrCode = VIR_Function_DupOperand(pFunc,
        pDef->defKey.pDefInst->src[VIR_Operand_Src0], &src1Opnd);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    movInst->src[VIR_Operand_Src0]  = src1Opnd;
    retErrCode = _VIR_RA_LS_RewriteColor_Src(pRA, pDef->defKey.pDefInst,
        pDef->defKey.pDefInst->src[VIR_Operand_Src0], src1Opnd);

    retErrCode = _VIR_RA_LS_GenTemp(pRA, &tmpSymId);
    VIR_Operand_SetTempRegister(movInst->dest,
                                pFunc,
                                tmpSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.w */
    _VIR_RA_MakeColor(pRA->baseRegister, 3, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, movInst->dest, curColor);
    VIR_Operand_SetEnable(movInst->dest, VIR_ENABLE_X);

    /* mad src0 - coming from extra MOV */
    VIR_Operand_SetTempRegister(madInst->src[VIR_Operand_Src0],
                                pFunc,
                                tmpSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.w */
    _VIR_RA_MakeColor(pRA->baseRegister, 3, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, madInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(madInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - 16 (will change to based on type) */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src1], 16);

    /* src2 - spillOffset */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src2],
        _VIR_RA_LS_ComputeSpillOffset(pRA, pOpnd, pLR));

    /* dest */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }
    VIR_Operand_SetTempRegister(madInst->dest,
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.y */
    _VIR_RA_MakeColor(pRA->baseRegister, 1, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, madInst->dest, curColor);
    VIR_Operand_SetEnable(madInst->dest, VIR_ENABLE_X);

    /* remove mova if pInst is its only use */
    if (vscVIR_IsUniqueUsageInstOfDefInst(pLvInfo->pDuInfo, pDef->defKey.pDefInst, pInst, gcvNULL))
    {
        VIR_Function_RemoveInstruction(pFunc, pDef->defKey.pDefInst);
    }

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
    1: mad  baseRegister.y, spillOffset, t1.x, 16
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* mad instruction */
    retErrCode = _VIR_RA_LS_InsertSpillOffset(pRA, pInst, pOpnd, pLR, gcvTRUE);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* load instruction */
    retErrCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_LOAD_S,
        VIR_TYPE_UINT32,
        pInst,
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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 offset */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
    /* use baseRegister.y */
    _VIR_RA_MakeColor(pRA->baseRegister, 1, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);

    /* set the dest for the newInst */
    retErrCode = _VIR_RA_LS_GenTemp(pRA, &symId);
    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                symId,
                                VIR_Operand_GetType(pOpnd));
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
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
                                VIR_Operand_GetType(pOpnd));
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
    VIR_Operand_SetSwizzle(pOpnd, newSwizzle);

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
    1: mad  offset.x, spillOffset, t1.x, 16
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* mad instruction */
    retErrCode = _VIR_RA_LS_InsertSpillOffset(pRA, pInst, pOpnd, pLR, gcvFALSE);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;

    /* store instruction */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_STORE_S,
        VIR_Operand_GetType(pOpnd),
        pInst,
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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - offset */
    if (pRA->spillOffsetSymId == VIR_INVALID_ID)
    {
        retErrCode = _VIR_RA_LS_GenTemp(pRA, &pRA->spillOffsetSymId);
    }
    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                pFunc,
                                pRA->spillOffsetSymId,
                                VIR_TYPE_UINT32);
    /* use baseRegister.y */
    _VIR_RA_MakeColor(pRA->baseRegister, 1, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);

    /* src2 - data */
    retErrCode = VIR_Function_DupOperand(pFunc, pInst->src[VIR_Operand_Src1], &newOpnd);
    if(retErrCode != VSC_ERR_NONE) return retErrCode;
    newInst->src[VIR_Operand_Src2]  = newOpnd;
    _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[VIR_Operand_Src1],
        newOpnd);

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
        VIR_LOG(pDumper, "Store instruction:\n");
        VIR_LOG_FLUSH(pDumper);
        VIR_Inst_Dump(pDumper, newInst);
    }

    /* remove STARR instruction */
    VIR_Function_RemoveInstruction(pFunc, pInst);

    return retErrCode;
}

static gctUINT
_VIR_RA_LS_GetSpillSize(
    VIR_RA_LS           *pRA,
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

    /* For a CS shader, the groupSize should be the local work group size. */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_COMPUTE &&
        pShader->shaderLayout.compute.workGroupSize[0] != 0)
    {
        groupSize = (pShader)->shaderLayout.compute.workGroupSize[0]
                  * (pShader)->shaderLayout.compute.workGroupSize[1]
                  * (pShader)->shaderLayout.compute.workGroupSize[2];
    }
    else
    {
        threadCount = hwConfig->maxCoreCount * 4 * (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
        groupCount =  hwConfig->maxGPRCountPerCore / VIR_Shader_GetRegWatermark(pShader);
        groupSize = groupCount * threadCount;
    }

    spillSize += (pRA->spillOffset) * groupSize;

    if (pGroupSize)
    {
        *pGroupSize = groupSize;
    }

    return spillSize;
}

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
    VIR_RA_HWReg_Color  curColor;

    VIR_Symbol*         uSymSpillMemAddr = gcvNULL;
    gctUINT             groupSize = 0;

    gctUINT             spillSize = _VIR_RA_LS_GetSpillSize(pRA, &groupSize);
    gctUINT             uniformIdx;

    pShader->vidmemSizeOfSpill = spillSize;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

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

    VIR_Inst_SetFunction(atomicAddInst, pFunc);
    VIR_Inst_SetBasicBlock(atomicAddInst, CFG_GET_ENTRY_BB(VIR_Function_GetCFG(pFunc)));

    /* src0 - base */
    VIR_Operand_SetOpKind(atomicAddInst->src[VIR_Operand_Src0], VIR_OPND_SYMBOL);
    atomicAddInst->src[VIR_Operand_Src0]->u1.sym = uSymSpillMemAddr;
    VIR_Operand_SetSwizzle(atomicAddInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XXXX);

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
    /* use baseRegister.z */
    _VIR_RA_MakeColor(pRA->baseRegister, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, atomicAddInst->dest, curColor);
    VIR_Operand_SetEnable(atomicAddInst->dest, VIR_ENABLE_X);

    /*
    ** For a CS, swathing is disabled now, so we need to reset the index to 0 for a new swath.
    ** TODO: we don't need this if swathing is enabled.
    */
    /* TODO: HW can't support 32 bit mod/div, use 16 bit mod right now, need to refine. */
    /* index = mod(index, groupSize) */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                  VIR_OP_MOD,
                                                  VIR_TYPE_UINT16,
                                                  atomicAddInst,
                                                  &modInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(modInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT16);
        /* use baseRegister.z */
    _VIR_RA_MakeColor(pRA->baseRegister, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, modInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(modInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - based on group size */
    VIR_Operand_SetImmediateUint(modInst->src[VIR_Operand_Src1], groupSize);

    /* dest */
    VIR_Operand_SetTempRegister(modInst->dest,
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT16);
    gcmASSERT(pRA->baseRegister != VIR_INVALID_ID);
    /* use baseRegister.z */
    _VIR_RA_MakeColor(pRA->baseRegister, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, modInst->dest, curColor);
    VIR_Operand_SetEnable(modInst->dest, VIR_ENABLE_X);

    /* base = (baseAddress + 4 ) + index * sizeof(spill for each thread)
       ==> mad base, i, sizeof(spill), baseAddress
           add base, base, 4 */
    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                                                  VIR_OP_MAD,
                                                  VIR_TYPE_UINT32,
                                                  modInst,
                                                  &madInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(madInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->threadIdxSymId,
                                VIR_TYPE_UINT32);
     /* use baseRegister.z */
    _VIR_RA_MakeColor(pRA->baseRegister, 2, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, madInst->src[VIR_Operand_Src0], curColor);
    VIR_Operand_SetSwizzle(madInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

    /* src1 - sizeof spill */
    VIR_Operand_SetImmediateUint(madInst->src[VIR_Operand_Src1], pRA->spillOffset);

    /* src2 - physical address*/
    VIR_Operand_SetOpKind(madInst->src[VIR_Operand_Src2], VIR_OPND_SYMBOL);
    madInst->src[VIR_Operand_Src2]->u1.sym = uSymSpillMemAddr;
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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, madInst->dest, curColor);
    VIR_Operand_SetEnable(madInst->dest, VIR_ENABLE_X);

    retErrCode = VIR_Function_AddInstructionAfter(pFunc,
                    VIR_OP_ADD,
                    VIR_TYPE_UINT32,
                    madInst,
                    &addInst);
    if (retErrCode != VSC_ERR_NONE) return retErrCode;

    /* src0 - index of thread */
    VIR_Operand_SetTempRegister(addInst->src[VIR_Operand_Src0],
                                pFunc,
                                pRA->baseAddrSymId,
                                VIR_TYPE_UINT32);
     /* use baseRegister.x */
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->src[VIR_Operand_Src0], curColor);
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
    _VIR_RA_MakeColor(pRA->baseRegister, 0, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->dest, curColor);
    VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_X);

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
    VIR_Instruction *pInst,
    VIR_Operand     *pSrcOpnd,
    VIR_Operand     *pOpnd
    )
{
    VSC_ErrCode             retErrCode = VSC_ERR_NONE;
    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_RA_ColorPool        *pCP = VIR_RA_LS_GetColorPool(pRA);
    VIR_OperandInfo         operandInfo;
    VIR_RA_LS_Liverange     *pLR;
    gctUINT                 webIdx, diffReg = 0;
    VIR_RA_HWReg_Color      curColor;
    VIR_Symbol              *pSym = gcvNULL;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    VIR_Operand_GetOperandInfo(pInst,
                               pSrcOpnd,
                               &operandInfo);
    webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pSrcOpnd);
    if (VIR_INVALID_WEB_INDEX != webIdx)
    {
        pLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
        if (!isLRSpilled(pLR))
        {
            gcmASSERT(!_VIR_RA_LS_IsInvalidLOWColor(_VIR_RA_GetLRColor(pLR)));
            pSym = VIR_Operand_GetSymbol(pOpnd);
            _VIR_RA_LS_SetSymbolHwRegInfo(pRA, pSym, pLR, diffReg);

            /* get the array element color */
            if (pLR->hwType == VIR_RA_HWREG_GR)
            {
                diffReg = operandInfo.u1.virRegInfo.virReg - pLR->firstRegNo;
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
            else if (pLR->hwType == VIR_RA_HWREG_A0)
            {
                gcmASSERT(VIR_Operand_GetPrecision(pOpnd) != VIR_PRECISION_HIGH);
                _VIR_RA_MakeColor(VIR_SR_A0,
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
            gcmASSERT(!(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR &&
                        VIR_Inst_GetSourceIndex(pInst, pOpnd) == 0));
            _VIR_RA_LS_InsertSpill(pRA, pInst, pOpnd, pLR);
        }
    }

    if (pOpnd->_hwRegId == VIR_REG_SAMPLE_MASK_IN ||
        pOpnd->_hwRegId == VIR_REG_SAMPLE_ID)
    {
        gctUINT swizzle = pShader->sampleMaskIdChannelStart;
        VIR_Operand_SetSwizzle(pOpnd, (VIR_Swizzle) (swizzle | (swizzle << 2) | (swizzle << 4) | (swizzle << 6)));
    }
    else if (pOpnd->_hwRegId == VIR_REG_SAMPLE_POS)
    {
        /* hw samplePos source type has no swizzle support, thus we need to generate
           an extra mov, if sample pos the swizzle is not XYZW */
        if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasSamplePosSwizzleFix &&
            (VIR_Operand_GetSwizzle(pOpnd) != VIR_SWIZZLE_XYZW))
        {
            VIR_Function    *pFunc = VIR_Shader_GetCurrentFunction(pShader);
            VIR_Instruction *newInst = gcvNULL;
            VIR_SymId       symId;
            VIR_Operand     *pNewOpnd = gcvNULL;

            VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_MOV,
                VIR_Operand_GetType(pOpnd),
                pInst,
                &newInst);

            VIR_Function_DupOperand(pFunc, pOpnd, &pNewOpnd);
            newInst->src[VIR_Operand_Src0] = pNewOpnd;
            gcmASSERT(_VIR_RA_Color_RegNo(curColor) == VIR_REG_SAMPLE_POS);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, pNewOpnd, curColor);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZW);

            _VIR_RA_LS_GenTemp(pRA, &symId);
            VIR_Operand_SetTempRegister(newInst->dest,
                                        pFunc,
                                        symId,
                                        VIR_Operand_GetType(pOpnd));
            if (pRA->samplePosRegister != VIR_INVALID_ID)
            {
                _VIR_RA_MakeColor(pRA->samplePosRegister, 0, &curColor);
            }
            else
            {
                /* find a brand new color for the temp */
                if (_VIR_RA_LS_FindBrandnewColor(pRA, gcvNULL, &curColor))
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
                                        VIR_Operand_GetType(pOpnd));
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
            VIR_Inst_SetThreadMode(newInst, VIR_THREAD_D16_DUAL_32);
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetDest(pInst));

    if (!_VIR_RA_LS_IsExcludedLR(pRA, gcvNULL, gcvNULL, gcvNULL, pInst))
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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_SELECT_MAP,
            VIR_TYPE_UINT16,
            pInst,
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
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(destReg, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_YYYY);
    }
    else
    {
        /* set src0 - index value, coming from the attr_ld src1 (non constant) */
        gcmASSERT(VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src1]) != VIR_OPND_IMMEDIATE);
        newInst->src[VIR_Operand_Src0] = pInst->src[VIR_Operand_Src1];
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[VIR_Operand_Src1],
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
    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* load_attr dest, src0 (regmapBase), src1 (regmapIndex), src2 (attrIndex) */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_LOAD_ATTR,
                VIR_TYPE_UINT16,
                pInst,
                newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    /* src 0 regmap*/
    VIR_Operand_SetTempRegister((*newInst)->src[VIR_Operand_Src0],
                                pFunc,
                                regmapSymId,
                                VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetSwizzle((*newInst)->src[VIR_Operand_Src0], regmapSwizzle);
    _VIR_RA_MakeColor(regmapReg, regmapChannel, &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->src[VIR_Operand_Src0], curColor);

    /* src 1 regmap index */
    if (regmapIndex != -1)
    {
        VIR_Operand_SetImmediateInt((*newInst)->src[VIR_Operand_Src1], regmapIndex);
    }
    else
    {
        gcmASSERT(src1Opnd);
        (*newInst)->src[VIR_Operand_Src1] = src1Opnd;
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd, (*newInst)->src[VIR_Operand_Src1]);
    }

    /* src 2 attribute index
       it could be possibly changed to temp by the client afterward */
    VIR_Operand_SetImmediateInt((*newInst)->src[VIR_Operand_Src2], attrIndex);

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

    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LOAD_ATTR)
    {
        srcOpnd = pInst->src[VIR_Operand_Src1];
    }
    else
    {
        srcOpnd = pInst->src[VIR_Operand_Src0];
    }
    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* MOD index, index, vertexCount */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_MOD,
                VIR_TYPE_INT16,
                pInst,
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
    VIR_Function_DupOperand(pFunc, srcOpnd, &newOpnd);
    (*newInst)->src[VIR_Operand_Src0] = newOpnd;
    _VIR_RA_MakeColor(
        VIR_Operand_GetHwRegId(srcOpnd),
        VIR_Operand_GetHwShift(srcOpnd),
        &curColor);
    _VIR_RA_LS_SetOperandHwRegInfo(pRA, (*newInst)->src[VIR_Operand_Src0], curColor);

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

/* generate load_attr for per-vertex data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Vertex(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function        *pFunc = VIR_Shader_GetCurrentFunction(pShader);

    VIR_Instruction     *newInst = gcvNULL, *modInst = gcvNULL, *addSrc1Inst = gcvNULL;
    VIR_SymId           symId = VIR_INVALID_ID, dstSymId = VIR_INVALID_ID;
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
    VIR_Operand         *src1Opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src1);
    gctBOOL             needUpdateSrc1 = gcvFALSE;
    gctBOOL             needUpdateSrc2 = gcvFALSE;
    gctBOOL             extraMOV    = gcvFALSE;

    VIR_Symbol          *sym = gcvNULL;

    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    sym = VIR_Operand_GetUnderlyingSymbol(pOpnd);

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR for attribute */
    VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_LOAD_STORE_ATTR);

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
    if (VIR_Symbol_isOutput(VIR_Operand_GetUnderlyingSymbol(pOpnd)))
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

    if (VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src1]) == VIR_OPND_IMMEDIATE)
    {
        /* const index access */
        firstSelectMap = gcvFALSE;
        secondSelectMap = gcvFALSE;

        /* write this way to be clear */
        switch(VIR_Operand_GetImmediateUint(pInst->src[VIR_Operand_Src1]) / 8)
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

        regmapIndex = VIR_Operand_GetImmediateUint(pInst->src[VIR_Operand_Src1]) % 8;

        if ((inputVertex && (regmapIndex >= vertexCount)) ||
            (!inputVertex && (regmapIndex >= outputCount)))
        {
            VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
            VIR_Inst_SetSrcNum(pInst, 1);
            VIR_Operand_SetImmediateFloat(pInst->src[VIR_Operand_Src0], 0.0);
            _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pInst->dest);

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

                VIR_Function_AddInstructionBefore(pFunc,
                    VIR_OP_MOV,
                    VIR_Operand_GetType(src1Opnd),
                    pInst,
                    &movInst);

                movInst->src[VIR_Operand_Src0] = src1Opnd;
                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd,
                    movInst->src[VIR_Operand_Src0]);

                _VIR_RA_LS_GenTemp(pRA, &movDstSymId);
                VIR_Operand_SetTempRegister(movInst->dest,
                                            pFunc,
                                            movDstSymId,
                                            VIR_Operand_GetType(src1Opnd));
                _VIR_RA_MakeColor(destReg, 0, &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, movInst->dest, curColor);
                VIR_Operand_SetEnable(movInst->dest, VIR_ENABLE_Y);
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
                /* insert an add instruction */
                retValue = VIR_Function_AddInstructionBefore(pFunc,
                    VIR_OP_ADD,
                    VIR_TYPE_UINT16,
                    pInst,
                    &addSrc1Inst);
                if (retValue != VSC_ERR_NONE) return retValue;

                VIR_Function_DupOperand(pFunc, src1Opnd, &addSrc1Inst->src[VIR_Operand_Src0]);

                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, src1Opnd,
                    addSrc1Inst->src[VIR_Operand_Src0]);

                VIR_Operand_SetImmediateInt(addSrc1Inst->src[VIR_Operand_Src1], 4);

                _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
                VIR_Operand_SetTempRegister(addSrc1Inst->dest,
                                            pFunc,
                                            addDstSymId,
                                            VIR_Operand_GetType(pInst->src[VIR_Operand_Src1]));
                _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
                _VIR_RA_LS_SetOperandHwRegInfo(pRA, addSrc1Inst->dest, curColor);
                VIR_Operand_SetEnable(addSrc1Inst->dest, VIR_ENABLE_X);

                /* use the new temp as the src1 opnd */
                needUpdateSrc1 = gcvTRUE;
            }
        }
    }

    /* set the attribute index */
    opndSwizzle = VIR_Operand_GetSwizzle(pOpnd);
    ldEnable = VIR_Enable_ApplyMappingSwizzle(VIR_Inst_GetEnable(pInst), opndSwizzle);
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst, pOpnd, gcvNULL, &attrIndex, &ldEnable);

    /* offset is in src2 */
    if (VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src2]) == VIR_OPND_IMMEDIATE)
    {
        attrIndex = attrIndex + VIR_Operand_GetImmediateInt(pInst->src[VIR_Operand_Src2]);
    }
    else
    {
        VIR_Instruction *addInst;

        /* insert an add instruction */
        /* to-do: optimize the constant case */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        addInst->src[VIR_Operand_Src0] = pInst->src[VIR_Operand_Src2];
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[VIR_Operand_Src2],
            addInst->src[VIR_Operand_Src0]);

        VIR_Operand_SetImmediateInt(addInst->src[VIR_Operand_Src1], attrIndex);

        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addInst->dest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src2]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->dest, curColor);
        /* x component could be already used by src1 */
        VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_Y);

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
        pShader->remapChannelStart,
        regmapIndex,
        src1Opnd,
        attrIndex,
        &newInst);

    if (needUpdateSrc1)
    {
        /* src1 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);
    }

    if (needUpdateSrc2)
    {
        /* src2 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src2]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src2], VIR_SWIZZLE_Y);
    }

    /* HW has no out-of-bound check, thus SW needs to add a MOD instruction
       before the load_attr instruction */
    if (!VIR_RA_LS_GetHwCfg(pRA)->hwFeatureFlags.hasLoadAttrOOBFix &&
        regmapIndex == -1 &&
        !_VIR_RA_LS_NotNeedOOB(pShader, pInst, inputVertex, vertexCount, outputCount))
    {
        VIR_SymId   modDest;
        if (needUpdateSrc1)
        {
            retValue = _VIR_RA_LS_InsertMOD(pRA, addSrc1Inst, inputVertex ? vertexCount : outputCount, &modInst, &modDest);
            /* change the src0*/
            VIR_Operand_SetTempRegister(addSrc1Inst->src[VIR_Operand_Src0],
                                        pFunc,
                                        modDest,
                                        VIR_TYPE_INT16);
            /* use reserved register.z */
            _VIR_RA_MakeColor(
                VIR_Operand_GetHwRegId(modInst->dest),
                VIR_Operand_GetHwShift(modInst->dest),
                &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, addSrc1Inst->src[VIR_Operand_Src0], curColor);
            VIR_Operand_SetSwizzle(addSrc1Inst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);
        }
        else
        {
            retValue = _VIR_RA_LS_InsertMOD(pRA, newInst, inputVertex ? vertexCount : outputCount, &modInst, &modDest);
            /* change the src1*/
            VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                        pFunc,
                                        modDest,
                                        VIR_TYPE_INT16);
            /* use reserved register.z */
            _VIR_RA_MakeColor(
                VIR_Operand_GetHwRegId(modInst->dest),
                VIR_Operand_GetHwShift(modInst->dest),
                &curColor);
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
            VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);
        }
    }

    /* if generated LOAD_ATTR has a different enable than ATTR_LD instruction,
       a mov is needed. */
    if (VIR_Operand_GetEnable(pInst->dest) != ldEnable)
    {
        /* set dest enable, need to consider the pOpnd's swizzle
           ATTR_LD    temp.x, in_te_attr.hp.z, 1, 0
           ==>
           LOAD_ATTR temp1.z, remap, remapIdx, attrIdx, 0
           MOV       temp.x, temp1.z
        */
        VIR_Swizzle newSwizzle = 0, tempSwizzle = VIR_Enable_2_Swizzle(ldEnable);
        VIR_Enable  movEnable = VIR_Operand_GetEnable(pInst->dest);
        gctUINT     i, j = 0, channelSwizzle;

        for (i = 0; i < CHANNEL_NUM; i ++)
        {
            if (movEnable & (1 << i))
            {
                channelSwizzle = (tempSwizzle >> j++*2) & 0x3;
                newSwizzle |= (channelSwizzle << i*2);
            }
        }

        _VIR_RA_LS_GenTemp(pRA, &symId);
        VIR_Operand_SetTempRegister(newInst->dest,
                                    pFunc,
                                    symId,
                                    VIR_Operand_GetType(pInst->dest));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
        VIR_Operand_SetEnable(newInst->dest, ldEnable);

        /* change the attr_ld to mov */
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(pInst, 1);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    symId,
                                    VIR_Operand_GetType(pInst->dest));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
        VIR_Operand_SetSwizzle(pOpnd, newSwizzle);
        VIR_Operand_SetMatrixConstIndex(pOpnd, 0);
        _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pInst->dest);

    }
    else
    {
        /* set dest enable, need to consider the pOpnd's swizzle
           ATTR_LD    temp.x, in_te_attr.hp.x, 1, 0
           ==>
           LOAD_ATTR temp.x, remap, remapIdx, attrIdx, 0
        */
        newInst->dest = pInst->dest;
        _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, newInst->dest);

        /* remove ldarr */
        VIR_Function_RemoveInstruction(pFunc, pInst);
    }

    return retValue;
}


/* generate load_attr for per-patch data */
VSC_ErrCode _VIR_RA_LS_GenLoadAttr_Patch(
    VIR_RA_LS       *pRA,
    VIR_Instruction *pInst,
    VIR_Operand     *pOpnd)
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
    VIR_SymId           addDstSymId = VIR_INVALID_ID;
    VIR_Symbol          *sym = gcvNULL;
    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_ATTR_LD);

    sym = VIR_Operand_GetUnderlyingSymbol(pOpnd);

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
    opndSwizzle  = VIR_Operand_GetSwizzle(pOpnd);
    ldEnable = VIR_Enable_ApplyMappingSwizzle(VIR_Inst_GetEnable(pInst), opndSwizzle);

    /* attribute index */
    _VIR_RA_LS_ComputeAttrIndexEnable(pInst, pOpnd, gcvNULL, &attrIndex, &ldEnable);

     /* offset in ATTR_LD src2 */
    if (VIR_Operand_GetOpKind(pInst->src[VIR_Operand_Src2]) == VIR_OPND_IMMEDIATE)
    {
        gctINT  src2Immx = VIR_Operand_GetImmediateInt(pInst->src[VIR_Operand_Src2]);
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

         /* to-do packed case */
         gcmASSERT(VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_OUTER &&
                   VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_INNER);

        /* insert an add instruction */
        /* to-do: optimize the constant case */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        addInst->src[VIR_Operand_Src0] = pInst->src[VIR_Operand_Src2];
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[VIR_Operand_Src2],
            addInst->src[VIR_Operand_Src0]);

        VIR_Operand_SetImmediateInt(addInst->src[VIR_Operand_Src1], attrIndex);

        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addInst->dest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src2]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->dest, curColor);
        VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_X);

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
        CHANNEL_W,
        regmapIndex,
        gcvNULL,
        attrIndex,
        &newInst);

    if (needUpdateSrc2)
    {
        /* src2 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src2]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src2], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src2], VIR_SWIZZLE_X);
    }

    if (VIR_Operand_GetEnable(pInst->dest) != ldEnable)
    {
        /* set dest enable, need to consider the pOpnd's swizzle
           ATTR_LD    temp.x, in_te_attr.hp.z, 1, 0
           ==>
           LOAD_ATTR temp1.z, remap, remapIdx, attrIdx, 0
           MOV       temp.x, temp1.z
        */
        _VIR_RA_LS_GenTemp(pRA, &symId);
        VIR_Operand_SetTempRegister(newInst->dest,
                                    pFunc,
                                    symId,
                                    VIR_Operand_GetType(pInst->dest));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->dest, curColor);
        VIR_Operand_SetEnable(newInst->dest, ldEnable);

        /* change the attr_ld to mov */
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(pInst, 1);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    symId,
                                    VIR_Operand_GetType(pInst->dest));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, pOpnd, curColor);
        VIR_Operand_SetSwizzle(pOpnd, VIR_Enable_2_Swizzle(ldEnable));
        VIR_Operand_SetMatrixConstIndex(pOpnd, 0);
        _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pInst->dest);
    }
    else
    {
        /* set dest enable, need to consider the pOpnd's swizzle
           ATTR_LD    temp.x, in_te_attr.hp.x, 1, 0
           ==>
           LOAD_ATTR temp.x, remap, remapIdx, attrIdx, 0
        */
        newInst->dest = pInst->dest;
        _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, newInst->dest);

        /* remove attr_ld */
        VIR_Function_RemoveInstruction(pFunc, pInst);
    }

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
    gctUINT             i;
    VIR_HwRegId         hwReg;
    VIR_TypeId          elementTypeId = _VIR_RA_LS_GenBaseTypeID(pShader, pSym);
    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT);

    /* set VIR_SYMFLAG_LOAD_STORE_ATTR flag */
    VIR_Symbol_SetFlag(pSym, VIR_SYMFLAG_LOAD_STORE_ATTR);

    _VIR_RA_LS_GetSym_Enable_Swizzle(pSym, gcvNULL, &valSwizzle);

    for (i = 0; i < VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pSym)); i++)
    {
        hwReg = hwStartReg + i;

        /* STORE_ATTR output, r0.x, attributeIndex, rx */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
                    VIR_OP_STORE_ATTR,
                    VIR_TYPE_UINT16,
                    pInst,
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

        /* set attributeIndex */
        _VIR_RA_LS_ComputeAttrIndexEnable(pInst,
                                            gcvNULL,
                                            pSym,
                                            &attrIndex,
                                            gcvNULL);

        VIR_Operand_SetImmediateInt(newInst->src[VIR_Operand_Src1], attrIndex + i);

        /* value in ATTR_ST src2 */
        _VIR_RA_LS_GenTemp(pRA, &symId);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src2],
                                    pFunc,
                                    symId,
                                    elementTypeId);
        _VIR_RA_MakeColor(hwReg, hwShift, &curColor);
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
    }

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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

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
        _VIR_RA_MakeColor(0, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
    }
    else
    {
        _VIR_RA_MakeColor(0, CHANNEL_W, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src0], curColor);
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

        /* to-do: tessLevelInner/tessLevelOuter dynamic indexing need pack mode? */
        gcmASSERT(VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_OUTER &&
                  VIR_Symbol_GetName(sym) != VIR_NAME_TESS_LEVEL_INNER);

        /* insert an add instruction */
        retValue = VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_ADD,
            VIR_TYPE_UINT16,
            pInst,
            &addInst);
        if (retValue != VSC_ERR_NONE) return retValue;

        addInst->src[VIR_Operand_Src0] = pInst->src[VIR_Operand_Src1];
        _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[VIR_Operand_Src1],
            addInst->src[VIR_Operand_Src0]);

        VIR_Operand_SetImmediateInt(addInst->src[VIR_Operand_Src1], attrIndex);

        _VIR_RA_LS_GenTemp(pRA, &addDstSymId);
        VIR_Operand_SetTempRegister(addInst->dest,
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, addInst->dest, curColor);
        VIR_Operand_SetEnable(addInst->dest, VIR_ENABLE_X);

        /* src1 coming from the add instruction */
        gcmASSERT(addDstSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1],
                                    pFunc,
                                    addDstSymId,
                                    VIR_Operand_GetType(pInst->src[VIR_Operand_Src1]));
        _VIR_RA_MakeColor(pRA->resRegister, 0, &curColor);
        _VIR_RA_LS_SetOperandHwRegInfo(pRA, newInst->src[VIR_Operand_Src1], curColor);
        VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);
    }

    /* value in ATTR_ST src2 */
    newInst->src[VIR_Operand_Src2] = pInst->src[VIR_Operand_Src2];
    _VIR_RA_LS_RewriteColor_Src(pRA, pInst,
                        pInst->src[VIR_Operand_Src2],
                        newInst->src[VIR_Operand_Src2]);

    /* store_attr destionation.
       in some cases (src2 is immediate/uniform/indirect), store_attr
       needs a valid destination. We use the reserved register for destination.
       since its live range is only this instruction. */
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
    VIR_Function_RemoveInstruction(pFunc, pInst);

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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    /* load_attr */
    VIR_Function_PrependInstruction(pFunc,
                VIR_OP_LOAD_ATTR,
                VIR_TYPE_UINT16,
                &newInst);

    VIR_Inst_SetFunction(newInst, pFunc);

    VIR_Inst_SetBasicBlock(newInst, CFG_GET_ENTRY_BB(VIR_Function_GetCFG(pFunc)));

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

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    retValue = VIR_Function_AddInstruction(pFunc,
                VIR_OP_STORE_ATTR,
                VIR_TYPE_UINT16,
                newInst);
    if (retValue != VSC_ERR_NONE) return retValue;

    VIR_Inst_SetFunction((*newInst), pFunc);

    VIR_Inst_SetBasicBlock((*newInst), CFG_GET_EXIT_BB(VIR_Function_GetCFG(pFunc)));

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
    VIR_RA_HWReg_Color      curColor;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_EMIT ||
              VIR_Inst_GetOpcode(pInst) == VIR_OP_RESTART);

    if (isEmit)
    {
        /* generating a sequence of store_attr */
        gctUINT         outputIdx, usageIdx;
        VIR_USAGE_KEY   usageKey;
        for (outputIdx = 0;
            outputIdx < VIR_IdList_Count(VIR_Shader_GetOutputs(pShader));
            outputIdx ++)
        {
            VIR_Symbol* pOutputSym = VIR_Shader_GetSymFromId(pShader,
                                VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader), outputIdx));

            /* Find the usage */
            usageKey.pUsageInst = pInst;
            usageKey.pOperand = (VIR_Operand*)(gctUINTPTR_T) pOutputSym->u2.tempIndex;
            usageIdx = vscBT_HashSearch(&pLvInfo->pDuInfo->usageTable, &usageKey);
            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                VIR_USAGE   *pUse = GET_USAGE_BY_IDX(&pLvInfo->pDuInfo->usageTable, usageIdx);
                VIR_RA_LS_Liverange *pLR = _VIR_RA_LS_Web2ColorLR(pRA, pUse->webIdx);
                _VIR_RA_LS_GenStoreAttr_Output(pRA, pInst, pOutputSym,
                    _VIR_RA_Color_RegNo(_VIR_RA_GetLRColor(pLR)),
                    _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pLR)));
            }
        }
    }

    /* generate aq_emit */
    retValue = VIR_Function_AddInstructionBefore(pFunc,
                isEmit ? VIR_OP_AQ_EMIT : VIR_OP_AQ_RESTART,
                VIR_TYPE_UINT16,
                pInst,
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
    VIR_Function_RemoveInstruction(pFunc, pInst);
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
    VIR_Type                *pOpndType = gcvNULL;
    VIR_RA_LS_Liverange     *pMoveaLR, *pBaseLR;

    gctUINT             defIdx, webIdx = 0;

    VIR_OpCode          opcode    = VIR_OP_NOP;
    VIR_Swizzle         src_swizzle = VIR_SWIZZLE_X;

    VIR_RA_HWReg_Color      curColor;
    gctUINT                 indexSymId;
    VIR_Symbol              *indexSym;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    opcode = VIR_Inst_GetOpcode(pInst);
    switch(opcode)
    {
    case VIR_OP_MOVA:
        {
            defIdx = _VIR_RA_LS_InstFirstDefIdx(pRA, pInst);
            pMoveaLR = _VIR_RA_LS_Def2ColorLR(pRA, defIdx);
            _VIR_RA_MakeColor(VIR_SR_A0, _VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMoveaLR)), &curColor);
            if (!_VIR_RA_LS_IsInvalidHIColor(_VIR_RA_GetLRColor(pMoveaLR)))
            {
                _VIR_RA_MakeColorHI(VIR_SR_A0, _VIR_RA_Color_HIShift(_VIR_RA_GetLRColor(pMoveaLR)), &curColor);
            }
            _VIR_RA_LS_SetOperandHwRegInfo(pRA, pInst->dest, curColor);
            _VIR_RA_LS_ChangeMovaType(pRA, pInst);
            _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pInst->src[0], pInst->src[0]);
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
            1: mad  offset.x, spillOffset, t1.x, 16
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
                /* get mova LR's shift to put into src0 index_shift */
                webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pInst->src[VIR_Operand_Src1]);
                pMoveaLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                gcmASSERT(pMoveaLR->hwType == VIR_RA_HWREG_A0);

                /* relIndex is sym id */
                indexSym = VIR_Operand_GetSymbol(pInst->src[VIR_Operand_Src1]);
                indexSymId  = VIR_Operand_GetSymbolId_(pInst->src[VIR_Operand_Src1]);
                /* get ldarr's src1 swizzle */
                src_swizzle = VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src1]);
                src_swizzle = src_swizzle & 0x3;

                _VIR_RA_LS_SetSymbolHwRegInfo(pRA, indexSym, pMoveaLR, 0);

                /* color the base opnd */
                _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pSrcOpnd, pSrcOpnd);

                /* change base srcOpnd to base[index] */
                VIR_Operand_SetRelIndexing(pSrcOpnd, indexSymId);
                VIR_Operand_SetRelAddrMode(pSrcOpnd,
                    (_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMoveaLR)) + src_swizzle + 1));
                pOpndType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(pSrcOpnd));
                VIR_Operand_SetType(pSrcOpnd, VIR_Type_GetBaseTypeId(pOpndType));

                   /* if mova is using the w component of A0, this ldarr should not be removed,
                       since we are using A0.w to solve more than 4 A0 live ranges problem.
                       for example,
                       mov a0.w, r1.x
                       mov r3, c0[a0.w]
                       mov a0.w, r2.x
                       mov r4, c0[a0.w]
                       add r5, r3, r4 ==> we should NOT replace here, otherwise the overlapping live range */
                if (_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMoveaLR)) == 3 ||
                    !_VIR_RA_LS_removableLDARR(pRA, pInst, gcvTRUE))
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
                    VIR_Function_RemoveInstruction(pFunc, pInst);
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
            1: mad  offset.x, spillOffset, t1.x, 16
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
                /* get movea LR's shift to put into src0 index_shift */
                webIdx = _VIR_RA_LS_SrcOpnd2WebIdx(pRA, pInst, pInst->src[VIR_Operand_Src0]);
                pMoveaLR = _VIR_RA_LS_Web2ColorLR(pRA, webIdx);
                gcmASSERT(pMoveaLR->hwType == VIR_RA_HWREG_A0);
                /* get one component of starr's src0 swizzle */
                src_swizzle = VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src0]);
                src_swizzle = src_swizzle & 0x3;

                /* relIndex is sym id */
                indexSym  = VIR_Operand_GetSymbol(pInst->src[VIR_Operand_Src0]);
                indexSymId  = VIR_Operand_GetSymbolId_(pInst->src[VIR_Operand_Src0]);

                _VIR_RA_LS_SetSymbolHwRegInfo(pRA, indexSym, pMoveaLR, 0);

                VIR_Operand_SetRelIndexing(pDestOpnd, indexSymId);
                VIR_Operand_SetRelAddrMode(pDestOpnd,
                                           (_VIR_RA_Color_Shift(_VIR_RA_GetLRColor(pMoveaLR)) + src_swizzle + 1));
                pOpndType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(pDestOpnd));
                VIR_Operand_SetType(pDestOpnd, VIR_Type_GetBaseTypeId(pOpndType));

                _VIR_RA_LS_RewriteColor_Dest(pRA, pInst, pDestOpnd);

                /* change the starr to mov */
                VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
                VIR_Inst_SetSource(pInst, 0, pInst->src[VIR_Operand_Src1]);
                VIR_Inst_SetSrcNum(pInst, 1);

                _VIR_RA_LS_RewriteColor_Src(pRA, pInst,
                    pInst->src[VIR_Operand_Src0],
                    pInst->src[VIR_Operand_Src0]);
            }
        }
        break;
    case VIR_OP_ATTR_LD:
        /* converter makes sure that pervertex/perpatch only in attr_ld/attr_st */
        /* generate the load_attr */
        pSrcOpnd = pInst->src[VIR_Operand_Src0];
        if (VIR_Operand_IsArrayedPerVertex(pSrcOpnd))
        {
            _VIR_RA_LS_GenLoadAttr_Vertex(pRA, pInst, pSrcOpnd);
        }
        else if (VIR_Operand_IsPerPatch(pSrcOpnd))
        {
            _VIR_RA_LS_GenLoadAttr_Patch(pRA, pInst, pSrcOpnd);
        }
        else
        {
            gcmASSERT(isSymUnused(VIR_Operand_GetUnderlyingSymbol(pSrcOpnd)) ||
                      isSymVectorizedOut(VIR_Operand_GetUnderlyingSymbol(pSrcOpnd)));
            /* unused attr/output */
            VIR_Function_RemoveInstruction(pFunc, pInst);
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
            VIR_Function_RemoveInstruction(pFunc, pInst);
        }
        break;
    case VIR_OP_STORE_ATTR:
    case VIR_OP_LOAD_ATTR:
    case VIR_OP_STORE_S:
        /* nothing to do, already colored when generating */
        break;
    case VIR_OP_EMIT:
        _VIR_RA_LS_GenEmitRestart(pRA, pInst, gcvTRUE);
        break;
    case VIR_OP_RESTART:
        /* we don't need to generate restart for the case where
           output primitive type is point */
        if (VIR_Shader_GS_HasRestartOp(pShader))
        {
            _VIR_RA_LS_GenEmitRestart(pRA, pInst, gcvFALSE);
        }
        else
        {
            VIR_Function_RemoveInstruction(pFunc, pInst);
        }
        break;
    case VIR_OP_ATOMCMPXCHG:
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
            /* rewrite the src operands */
            gctUINT     i;
            VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
            pSrcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
            for (i = 0; i < VIR_MAX_SRC_NUM; i++)
            {
                pRA->dataRegisterUsed[i] = gcvFALSE;
            }
            for (; pSrcOpnd != gcvNULL; pSrcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                gcmASSERT(!VIR_Operand_IsPerPatch(pSrcOpnd) &&
                          !VIR_Operand_IsArrayedPerVertex(pSrcOpnd));
                retValue = _VIR_RA_LS_RewriteColor_Src(pRA, pInst, pSrcOpnd, pSrcOpnd);
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

/* ===========================================================================
   _VIR_RA_LS_CheckInstructionSrcs
   check the max number of sources that is spilled in an instruction
   ===========================================================================
*/

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

void
_VIR_RA_LS_CheckInstructionSrcs(
    VIR_RA_LS       *pRA,
    VIR_Function    *pFunc,
    gctUINT         *spilledSrc)
{
    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;
    VIR_SrcOperand_Iterator srcOpndIter;
    VIR_Operand         *pSrcOpnd;
    gctUINT             curSpilledSrc = 0;

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    for (; pInst != gcvNULL;
           pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        curSpilledSrc = 0;
        VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
        pSrcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
        for (; pSrcOpnd != gcvNULL; pSrcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
        {
            if (_VIR_RA_LS_OperandSpilled(pRA, pInst, pSrcOpnd))
            {
                curSpilledSrc++;
            }
        }
        if (*spilledSrc < curSpilledSrc)
        {
            *spilledSrc = curSpilledSrc;
        }
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
    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;

    VIR_Shader_SetCurrentFunction(VIR_RA_LS_GetShader(pRA), pFunc);

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);

    for (; pInst != gcvNULL;
        pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        retValue = _VIR_RA_LS_RewriteColorInst(pRA, pInst);
        CHECK_ERROR(retValue, "_VIR_RA_LS_RewriteColorInst");
    }

    return retValue;
}

/* ===========================================================================
   VIR_RA_LS_PerformOnFunction
   linear scan register allocation on function
   ===========================================================================
*/
static VSC_ErrCode _VIR_RA_LS_PerformOnFunction(
    VIR_RA_LS       *pRA,
    gctUINT         reservedDataReg)
{
    VSC_ErrCode retValue  = VSC_ERR_NONE;

    VIR_Shader              *pShader = VIR_RA_LS_GetShader(pRA);
    VIR_Function            *pFunc = VIR_Shader_GetCurrentFunction(pShader);
    VIR_Dumper              *pDumper = VIR_RA_LS_GetDumper(pRA);
    VSC_OPTN_RAOptions      *pOption = VIR_RA_LS_GetOptions(pRA);
    VIR_LIVENESS_INFO       *pLvInfo = VIR_RA_LS_GetLvInfo(pRA);
    VIR_CONTROL_FLOW_GRAPH* pCfg = VIR_Function_GetCFG(pFunc);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "\nProcessing function:\t[%s]\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
        VIR_LOG_FLUSH(pDumper);
    }

    /* to-do: optimize the compile-time */

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

    retValue = _VIR_RA_LS_AssignColors(pRA, pFunc, reservedDataReg);

    return retValue;
}

static VSC_ErrCode _AppendTempRegSpillMemAddrUniform(VIR_Shader *pShader)
{
    VSC_ErrCode  virErrCode;
    VIR_NameId   spillMemName;
    VIR_SymId    spillMemSymId;
    VIR_Symbol*  spillMemSym;
    VIR_Uniform *virUniform = gcvNULL;

    virErrCode = VIR_Shader_AddString(pShader,
                                      "#TempRegSpillMemAddr",
                                       &spillMemName);
    ON_ERROR(virErrCode, "Failed to VIR_Shader_AddString");

    /* default ubo symbol */
    virErrCode = VIR_Shader_AddSymbol(pShader,
                                      VIR_SYM_UNIFORM,
                                      spillMemName,
                                      VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                      VIR_STORAGE_UNKNOWN,
                                      &spillMemSymId);

    spillMemSym = VIR_Shader_GetSymFromId(pShader, spillMemSymId);
    VIR_Symbol_SetUniformKind(spillMemSym, VIR_UNIFORM_TEMP_REG_SPILL_MEM_ADDRESS);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
    VIR_Symbol_SetFlag(spillMemSym, VIR_SYMFLAG_COMPILER_GEN);
    VIR_Symbol_SetLocation(spillMemSym, -1);

    virUniform = VIR_Symbol_GetUniform(spillMemSym);
    virUniform->index = pShader->uniformCount - 1;

OnError:
    return virErrCode;
}

VSC_ErrCode VIR_RA_LS_PerformUniformAlloc(
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_OPTN_RAOptions  *pOption,
    VIR_Dumper          *pDumper)
{
    VSC_ErrCode           retValue  = VSC_ERR_NONE;
    VSC_PRIMARY_MEM_POOL  pmp;

    vscPMP_Intialize(&pmp, gcvNULL, VIR_RS_LS_MEM_BLK_SIZE, sizeof(void*), gcvTRUE);

    /* Append an uniform (one channel) for temp register spill mem address.
       This code should be moved to an phase that determine whether we need
       do temp register spill before uniform allocation */
    retValue = _AppendTempRegSpillMemAddrUniform(pShader);
    CHECK_ERROR(retValue, "Append temp-reg spill mem address uniform");

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption),
        VSC_OPTN_RAOptions_ALLOC_UNIFORM))
    {
        retValue = VIR_CG_MapUniforms(pShader, pHwCfg, &pmp.mmWrapper);
        CHECK_ERROR(retValue, "VIR_CG_MapUniforms");

        /* set const register allocated to shader */
        VIR_Shader_SetConstRegAllocated(pShader, gcvTRUE);
    }

    vscPMP_Finalize(&pmp);
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

    VIR_Inst_SetFunction(*newInst, pFunc);
    VIR_Inst_SetBasicBlock(*newInst, CFG_GET_ENTRY_BB(VIR_Function_GetCFG(pFunc)));

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

    /* based on different shader type, the dump is different */
    if (pShader->shaderKind == VIR_SHADER_FRAGMENT ||
        pShader->shaderKind == VIR_SHADER_VERTEX ||
        pShader->shaderKind == VIR_SHADER_COMPUTE ||
        pShader->shaderKind == VIR_SHADER_CL)
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

            for (regIdx = 0; regIdx < VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pAttrSym)); regIdx++)
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

            if (!VIR_Shader_GS_HasRestartOp(pShader) &&
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
            else if ((VIR_Shader_GS_HasRestartOp(pShader)) &&
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
        }
        pRA->baseRegister = hwRegCount + pRA->resDataRegisterCount;
    }
}

/* ===========================================================================
   _VIR_RA_LS_SetRegWatermark
   set the shader's register watermark
   should consider the implicit sampleDepth (the last register)
   ===========================================================================
*/
void _VIR_RA_LS_SetRegWatermark(
    VIR_RA_LS           *pRA)
{
    VIR_Shader      *pShader = VIR_RA_LS_GetShader(pRA);
    gctUINT         hwRegCount = 0;

    /* set register allocated to shader */
    VIR_Shader_SetRegAllocated(pShader, gcvTRUE);

    hwRegCount = VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxAllocReg + 1;

    if (pShader->hasRegisterSpill)
    {
        /* spill will reserve some registers */
        hwRegCount = hwRegCount + 1 + pRA->resDataRegisterCount;
    }

    if (_VIR_RA_isShaderNeedSampleDepth(pRA))
    {
        /* the last temp as sampleDepth */
        hwRegCount += (VIR_Shader_isDual16Mode(pShader) ? 2 : 1);
        pShader->sampleMaskIdRegStart = hwRegCount - 1;
    }

    gcmASSERT(hwRegCount <= VIR_RA_LS_GetColorPool(pRA)->colorMap[VIR_RA_HWREG_GR].maxReg);

    VIR_Shader_SetRegWatermark(pShader, hwRegCount);
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

/* ===========================================================================
   VIR_RA_LS_PerformTempRegAlloc
   linear scan register allocation on shader
   ===========================================================================
*/
VSC_ErrCode VIR_RA_LS_PerformTempRegAlloc(
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VIR_LIVENESS_INFO   *pLvInfo,
    VSC_OPTN_RAOptions  *pOption,
    VIR_Dumper          *pDumper,
    VIR_CALL_GRAPH      *pCG)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;

    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);
    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;
    VIR_RA_LS           ra;
    VIR_RA_HWReg_Color  curColor;
    gctUINT             reservedDataReg = 0;

    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    gctBOOL             colorSucceed = gcvFALSE;

    _VIR_RA_MakeColor(VIR_RA_INVALID_REG, 0, &curColor);
    _VIR_RA_MakeColorHI(VIR_RA_INVALID_REG, 0, &curColor);

    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
        VSC_OPTN_RAOptions_TRACE))
    {
        VIR_LOG(pDumper, "========================================================\n");
        VIR_LOG(pDumper, "Linear Scan Register Allocation\n");
        VIR_LOG_FLUSH(pDumper);
    }

    _VIR_RA_LS_Init(&ra, pShader, pHwCfg, pLvInfo, pOption, pDumper, pCG);

    if (countOfFuncBlk != 0)
    {
        /* dump before */
        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
            VSC_OPTN_RAOptions_TRACE_INPUT))
        {
            VIR_Shader_Dump(gcvNULL, "Shader before Register Allocation", pShader, gcvTRUE);
            VIR_LOG_FLUSH(pDumper);
        }

        if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetOPTS(pOption),
            VSC_OPTN_RAOptions_ALLOC_REG))
        {
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
               until successful
               */
            while (!colorSucceed)
            {
                gctUINT     spilledSrc = gcvFALSE;
                gctBOOL     reColor = gcvFALSE;

                /* build the live range and assign color to */
                for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
                {
                    pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
                    VIR_Shader_SetCurrentFunction(pShader, pFunc);
                    retValue = _VIR_RA_LS_PerformOnFunction(&ra, reservedDataReg);
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
                        break;
                    }
                    ON_ERROR(retValue, "_VIR_RA_LS_PerformOnFunction");
                }

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
                            _VIR_RA_LS_CheckInstructionSrcs(&ra, pFunc, &spilledSrc);
                        }

                        if (spilledSrc > reservedDataReg)
                        {
                            if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                                VSC_OPTN_RAOptions_TRACE_ASSIGN_COLOR))
                            {
                                VIR_LOG(pDumper, "\n!!!found an intruction with %d spilled source!!! restart the coloring with spills.\n",
                                    spilledSrc);
                                VIR_LOG_FLUSH(pDumper);
                            }
                            reservedDataReg = spilledSrc;
                            gcmASSERT(reservedDataReg < VIR_MAX_SRC_NUM);
                            reColor = gcvTRUE;
                        }
                        else
                        {
                            colorSucceed = gcvTRUE;
                            break;
                        }
                    }
                    else
                    {
                        colorSucceed = gcvTRUE;
                        break;
                    }
                }

                if (reColor)
                {
                    /* clear color pool */
                    _VIR_RA_ColorPool_ClearAll(VIR_RA_LS_GetColorPool(&ra));
                    _VIR_RA_LRTable_ClearColor(&ra);
                    VIR_RA_LS_GetColorPool(&ra)->colorMap[VIR_RA_HWREG_GR].maxAllocReg = 0;
                    VIR_RA_LS_GetActiveLRHead(&ra)->nextActiveLR = &(LREndMark);
                    (&ra)->resDataRegisterCount = reservedDataReg;
                    pShader->hasRegisterSpill = gcvTRUE;
                    (&ra)->spillOffset = 0;
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
            if (pShader->hasRegisterSpill)
            {
                retValue = _VIR_RA_LS_SpillAddrComputation(&ra);
                ON_ERROR(retValue, "_VIR_RA_LS_SpillAddrComputation");
            }

            /* set input/output VIR_SYMFLAG_LOAD_STORE_ATTR flag */
            _VIR_RA_SetInputOutputFlag(&ra);

            /* insert MOV Rn, Rn for input to help HW team to debug */
            if (gctOPT_hasFeature(FB_INSERT_MOV_INPUT))
            {
                retValue = _VIR_RA_LS_MovHWInputRegisters(&ra, pShader);
                ON_ERROR(retValue, "_VIR_RA_LS_MovHWInputRegisters");
            }
        }
    }

    /* dump after */
    if (VSC_UTILS_MASK(VSC_OPTN_RAOptions_GetTrace(pOption),
                       VSC_OPTN_RAOptions_TRACE_FINAL) ||
        gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after Register Allocation", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

OnError:
    _VIR_RA_LS_Final(&ra);

    return retValue;
}

