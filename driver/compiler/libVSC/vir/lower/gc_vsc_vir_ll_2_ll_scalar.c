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


#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

static gctBOOL
_hasNotIntVecDiv(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportFullCompIntDiv);
}

static VIR_PatternMatchInst _rcpSclPatInst0[] = {
    { VIR_OP_RCP, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _rcpSclRepInst0[] = {
    { VIR_OP_RCP, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _rcpSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_rcpScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _rsqSclPatInst0[] = {
    { VIR_OP_RSQ, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _rsqSclRepInst0[] = {
    { VIR_OP_RSQ, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _rsqSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_rsqScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _expSclPatInst0[] = {
    { VIR_OP_EXP2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _expSclRepInst0[] = {
    { VIR_OP_EXP2, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _expSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_expScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _logSclPatInst0[] = {
    { VIR_OP_PRE_LOG2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _logSclRepInst0[] = {
    { VIR_OP_PRE_LOG2, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _logSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_logScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sqrtSclPatInst0[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sqrtSclRepInst0[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _sqrtSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_sqrtScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sinpiSclPatInst0[] = {
    { VIR_OP_SINPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sinpiSclRepInst0[] = {
    { VIR_OP_SINPI, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _sinpiSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_sinpiScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _cospiSclPatInst0[] = {
    { VIR_OP_COSPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _cospiSclRepInst0[] = {
    { VIR_OP_COSPI, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _cospiSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_cospiScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _divSclPatInst0[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_enableFullNewLinker, _hasNotIntVecDiv, VIR_Lower_IsDstInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divSclRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_PatternMatchInst _divSclPatInst1[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_enableFullNewLinker, VIR_Lower_IsDstFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divSclRepInst1[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _divSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_divScl, 0) },
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_divScl, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _modSclPatInst0[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNotIntVecDiv, VIR_Lower_IsDstInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modSclRepInst0[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_PatternMatchInst _modSclPatInst1[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modSclRepInst1[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _modSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_modScl, 0) },
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_modScl, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _fixPatInst0[] = {
    { VIR_OP_FIX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _fixRepInst0[] = {
    { VIR_OP_FIX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _fixPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_fix, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetLowerPatternPhaseScalar(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_RCP:
        return _rcpSclPattern;
    case VIR_OP_RSQ:
        return _rsqSclPattern;
    case VIR_OP_EXP2:
        return _expSclPattern;
    case VIR_OP_PRE_LOG2:
        return _logSclPattern;
    case VIR_OP_SQRT:
        return _sqrtSclPattern;
    case VIR_OP_SINPI:
        return _sinpiSclPattern;
    case VIR_OP_COSPI:
        return _cospiSclPattern;
    case VIR_OP_DIV:
        return _divSclPattern;
    case VIR_OP_MOD:
        return _modSclPattern;
    case VIR_OP_FIX:
        return _fixPattern;
    default:
        break;
    }
    return gcvNULL;
}


static gctBOOL
_CmpInstuction(
    IN VIR_PatternContext   *Context,
    IN VIR_PatternMatchInst *Inst0,
    IN VIR_Instruction      *Inst1
    )
{
    return Inst0->opcode == Inst1->_opcode;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Scalar(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, VscContext, Shader, Context->pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                  _GetLowerPatternPhaseScalar, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Scalar failed.");

    VIR_PatternContext_Finalize(&Context->header);

    return errCode;
}


