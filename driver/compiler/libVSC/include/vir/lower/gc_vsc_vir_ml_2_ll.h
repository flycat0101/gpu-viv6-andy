/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_ml_2_ll_h_
#define __gc_vsc_vir_ml_2_ll_h_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

#ifndef M_PI
#   define M_PI 3.14159265358979323846f
#endif

typedef struct _VIR_PATTERN_LOWER_CONTEXT
{
    VIR_PatternContext          header;
    VSC_HW_CONFIG*              hwCfg;
    VSC_MM*                     pMM;
    gctBOOL                     generateImmediate;
    gctBOOL                     hasNEW_TEXLD;
    gctBOOL                     isCL_X;
    gctBOOL                     hasCL;
    gctBOOL                     hasHalti1;
    gctBOOL                     hasHalti2;
    gctBOOL                     hasHalti3;
    gctBOOL                     hasHalti4;
} VIR_PatternLowerContext;

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_Lower_MiddleLevel_To_LowLevel);

END_EXTERN_C()
#endif

