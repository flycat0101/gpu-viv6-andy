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


#ifndef __gc_vsc_vir_peephole_h_
#define __gc_vsc_vir_peephole_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_PH_PEEPHOLE
{
    VIR_Shader *            shader;
    VIR_BASIC_BLOCK *       curr_bb;
    VIR_DEF_USAGE_INFO *    du_info;

    /* hash tables */
    VSC_HASH_TABLE *        work_set;
    VSC_HASH_TABLE *        usage_set;
    VSC_HASH_TABLE *        def_set0;
    VSC_HASH_TABLE *        def_set1;

    VSC_HW_CONFIG *         hwCfg;
    VSC_OPTN_PHOptions *    options;
    VIR_Dumper *            dumper;
    VSC_MM *                pMM;

    gctBOOL     cfgChanged;
    gctBOOL     exprChanged;

} VSC_PH_Peephole;

#define VSC_PH_Peephole_GetShader(ph)          ((ph)->shader)
#define VSC_PH_Peephole_SetShader(ph, s)       ((ph)->shader = (s))
#define VSC_PH_Peephole_GetCurrFunc(ph)        (VIR_Shader_GetCurrentFunction((ph)->shader))
#define VSC_PH_Peephole_GetCurrBB(ph)          ((ph)->curr_bb)
#define VSC_PH_Peephole_SetCurrBB(ph, b)       ((ph)->curr_bb = (b))
#define VSC_PH_Peephole_GetDUInfo(ph)          ((ph)->du_info)
#define VSC_PH_Peephole_SetDUInfo(ph, d)       ((ph)->du_info = (d))
#define VSC_PH_Peephole_GetHwCfg(ph)           ((ph)->hwCfg)
#define VSC_PH_Peephole_SetHwCfg(ph, d)        ((ph)->hwCfg = (d))
#define VSC_PH_Peephole_GetOptions(ph)         ((ph)->options)
#define VSC_PH_Peephole_SetOptions(ph, o)      ((ph)->options = (o))
#define VSC_PH_Peephole_GetDumper(ph)          ((ph)->dumper)
#define VSC_PH_Peephole_SetDumper(ph, d)       ((ph)->dumper = (d))
#define VSC_PH_Peephole_GetMM(ph)              ((ph)->pMM)
#define VSC_PH_Peephole_GetCfgChanged(ph)      ((ph)->cfgChanged)
#define VSC_PH_Peephole_SetCfgChanged(ph, c)   ((ph)->cfgChanged = (c))
#define VSC_PH_Peephole_GetExprChanged(ph)     ((ph)->exprChanged)
#define VSC_PH_Peephole_SetExprChanged(ph, e)  ((ph)->exprChanged = (e))
#define VSC_PH_Peephole_WorkSet(ph)            ((ph)->work_set)
#define VSC_PH_Peephole_UsageSet(ph)           ((ph)->usage_set)
#define VSC_PH_Peephole_DefSet(ph)             ((ph)->def_set0)
#define VSC_PH_Peephole_DefSet0(ph)            ((ph)->def_set0)
#define VSC_PH_Peephole_DefSet1(ph)            ((ph)->def_set1)

extern VSC_ErrCode VSC_PH_Peephole_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_PH_Peephole_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_PH_Peephole_PerformOnShader);

VSC_ErrCode
_VSC_PH_InitHashTable(
    IN OUT VSC_PH_Peephole*  ph,
    IN OUT VSC_HASH_TABLE ** ppHT,
    IN PFN_VSC_HASH_FUNC     pfnHashFunc,
    IN PFN_VSC_KEY_CMP       pfnKeyCmp,
    IN gctINT                tableSize
    );

void
_VSC_PH_ResetHashTable(
    IN OUT VSC_HASH_TABLE * pHT
    );


END_EXTERN_C()

#endif

