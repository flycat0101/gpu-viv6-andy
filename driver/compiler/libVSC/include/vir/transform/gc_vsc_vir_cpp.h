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


#ifndef __gc_vsc_vir_cpp_h_
#define __gc_vsc_vir_cpp_h_

#include "gc_vsc.h"

#define VSC_CPP_MAX_INST_COUNT_GENERAL      3400
#define VSC_CPP_MAX_INST_COUNT_DEQP         2048

BEGIN_EXTERN_C()

typedef enum _VSC_CPP_FLAG
{
    VSC_CPP_NONE                    = 0x00,
    VSC_CPP_USE_SRC_TYPE_FROM_MOVE  = 0x01,
    /*
    ** Check if we need to copy the MOVE from a output parameter,
    ** we can't enable it if there is a inliner after this CPP pass due to the limitation in VIR_Shader_FindParmInst:
    ** we only check the output parameter once so if it have multiple usages, we can only detect the first usage.
    */
    VSC_CPP_COPY_FROM_OUTPUT_PARAM  = 0x02,
    /* Find the nearest defined instruction when there are multiple definitions. */
    VSC_CPP_FIND_NEAREST_DEF_INST   = 0x04,
} VSC_CPP_FLAG;

typedef struct _VSC_CPP_PASS_DATA
{
    VSC_CPP_FLAG            cppFlag;
    gctBOOL                 bGlobalCPP;
} VSC_CPP_PASS_DATA;

typedef struct VIR_CPP_COPYPROPAGATION
{
    gcePATCH_ID             appNameId;
    VIR_Shader              *shader;
    VIR_BASIC_BLOCK         *curr_bb;
    VIR_DEF_USAGE_INFO      *du_info;
    VSC_OPTN_CPPOptions     *options;
    VIR_Dumper              *dumper;

    VSC_CPP_PASS_DATA       *passData;

    gctINT                  fwOptCount;
    gctINT                  bwOptCount;

    VSC_MM                  *pMM;

    VSC_HW_CONFIG           *pHwCfg;

    gctBOOL                 bInvalidCfg;

} VSC_CPP_CopyPropagation;

#define VSC_CPP_GetAppNameId(cpp)       ((cpp)->appNameId)
#define VSC_CPP_SetAppNameId(cpp, s)    ((cpp)->appNameId = (s))
#define VSC_CPP_GetShader(cpp)          ((cpp)->shader)
#define VSC_CPP_SetShader(cpp, s)       ((cpp)->shader = (s))
#define VSC_CPP_GetCurrBB(cpp)          ((cpp)->curr_bb)
#define VSC_CPP_SetCurrBB(cpp, b)       ((cpp)->curr_bb = (b))
#define VSC_CPP_GetDUInfo(cpp)          ((cpp)->du_info)
#define VSC_CPP_SetDUInfo(cpp, d)       ((cpp)->du_info = (d))
#define VSC_CPP_GetOptions(cpp)         ((cpp)->options)
#define VSC_CPP_SetOptions(cpp, o)      ((cpp)->options = (o))
#define VSC_CPP_GetPassData(cpp)        ((cpp)->passData)
#define VSC_CPP_SetPassData(cpp, d)     ((cpp)->passData = (d))
#define VSC_CPP_GetDumper(cpp)          ((cpp)->dumper)
#define VSC_CPP_SetDumper(cpp, d)       ((cpp)->dumper = (d))
#define VSC_CPP_isGlobalCPP(cpp)        ((cpp)->passData->bGlobalCPP)
#define VSC_CPP_SetGlobalCPP(cpp, g)    ((cpp)->passData->bGlobalCPP = (g))
#define VSC_CPP_GetFlag(cpp)            ((cpp)->passData->cppFlag)
#define VSC_CPP_GetFWOptCount(cpp)      ((cpp)->fwOptCount)
#define VSC_CPP_SetFWOptCount(cpp, s)   ((cpp)->fwOptCount = (s))
#define VSC_CPP_GetBWOptCount(cpp)      ((cpp)->bwOptCount)
#define VSC_CPP_SetBWOptCount(cpp, s)   ((cpp)->bwOptCount = (s))
#define VSC_CPP_SetMM(cpp, s)           ((cpp)->pMM = (s))
#define VSC_CPP_GetMM(cpp)              ((cpp)->pMM)
#define VSC_CPP_SetHWCFG(cpp, s)        ((cpp)->pHwCfg = (s))
#define VSC_CPP_GetHWCFG(cpp)           ((cpp)->pHwCfg)
#define VSC_CPP_SetInvalidCfg(cpp, s)   ((cpp)->bInvalidCfg = (s))
#define VSC_CPP_GetInvalidCfg(cpp)      ((cpp)->bInvalidCfg)

extern VSC_ErrCode VSC_CPP_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_CPP_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_CPP_PerformOnShader);

extern VSC_ErrCode VSC_SCPP_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_SCPP_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_SCPP_PerformOnShader);

END_EXTERN_C()

#endif

