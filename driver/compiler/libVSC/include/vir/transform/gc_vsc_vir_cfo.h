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


#ifndef __gc_vsc_vir_cfo_h_
#define __gc_vsc_vir_cfo_h_

/******************************************************************************
            control flow optimization
            including: if conversion

******************************************************************************/

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_CFO
{
    VIR_Shader* shader;
    VSC_HW_CONFIG* hwCfg;
    VSC_OPTN_CFOOptions* options;
    VIR_Dumper* dumper;
    VSC_MM* mm;
} VIR_CFO;

#define VIR_CFO_GetShader(lo)              ((lo)->shader)
#define VIR_CFO_SetShader(lo, s)           ((lo)->shader = (s))
#define VIR_CFO_GetHWCfg(lo)               ((lo)->hwCfg)
#define VIR_CFO_SetHWCfg(lo, h)            ((lo)->hwCfg = (h))
#define VIR_CFO_GetOptions(lo)             ((lo)->options)
#define VIR_CFO_SetOptions(lo, o)          ((lo)->options = (o))
#define VIR_CFO_GetDumper(lo)              ((lo)->dumper)
#define VIR_CFO_SetDumper(lo, d)           ((lo)->dumper = (d))
#define VIR_CFO_GetMM(lo)                  ((lo)->mm)
#define VIR_CFO_SetMM(lo, m)               ((lo)->mm = (m))

void
VIR_CFO_Init(
    VIR_CFO* cfo,
    VIR_Shader* shader,
    VSC_HW_CONFIG* hwCfg,
    VSC_OPTN_CFOOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    );

void
VIR_CFO_Final(
    VIR_CFO* cfo
    );

VSC_ErrCode
VIR_CFO_PerformUnreachableEliminationOnShader(
    VIR_CFO* cfo,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_CFO_PerformUnreachableEliminationOnShader(
    VIR_CFO* cfo,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_CFO_PerformSelectGenerationOnShader(
    VIR_CFO* cfo,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_CFO_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_CFO_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_cfo_h_ */

