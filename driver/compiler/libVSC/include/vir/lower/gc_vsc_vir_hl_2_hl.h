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


#ifndef __gc_vsc_vir_hl_2_hl_h_
#define __gc_vsc_vir_hl_2_hl_h_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

typedef struct _VIR_PATTERN_HL_2_HL_CONTEXT
{
    VIR_PatternContext          header;
    VSC_MM                      *pMM;
} VIR_PatternHL2HLContext;

VSC_ErrCode
VIR_Lower_HighLevel_To_HighLevel_Expand(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_Lower_HighLevel_To_HighLevel_Expand);
DECLARE_SH_NECESSITY_CHECK(VIR_Lower_HighLevel_To_HighLevel_Expand);

END_EXTERN_C()
#endif

