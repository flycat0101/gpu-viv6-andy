/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_hl_2_ml_h_
#define __gc_vsc_vir_hl_2_ml_h_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

typedef struct _VIR_PATTERN_LOWER_CONTEXT
{
    VIR_PatternContext          header;
    VSC_HW_CONFIG*              hwCfg;
} VIR_PatternLowerContext;


VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Reg_Alloc(
    IN  VIR_Shader              *Shader
    );

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Sym_Update(
    IN  VIR_Shader              *Shader
    );

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Preprocess(
    IN  VIR_Shader              *Shader
    );

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg
    );

END_EXTERN_C()
#endif

