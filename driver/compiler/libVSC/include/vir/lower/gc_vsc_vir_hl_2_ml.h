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


#ifndef __gc_vsc_vir_hl_2_ml_h_
#define __gc_vsc_vir_hl_2_ml_h_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

typedef struct _VIR_PATTERN_HL_2_ML_CONTEXT
{
    VIR_PatternContext          header;
    PVSC_CONTEXT                vscContext;
    VSC_MM                      *pMM;
} VIR_PatternHL2MLContext;

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  VIR_PatternHL2MLContext *Context
    );

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_Lower_HighLevel_To_MiddleLevel);

VSC_ErrCode
VIR_Lower_MiddleLevel_Process_Intrinsics(
    IN  VIR_Shader   *Shader
    );
END_EXTERN_C()
#endif

