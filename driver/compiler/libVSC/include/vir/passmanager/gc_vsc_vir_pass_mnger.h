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


#ifndef __gc_vsc_vir_pass_mnger_h_
#define __gc_vsc_vir_pass_mnger_h_

BEGIN_EXTERN_C()

typedef struct _VSC_PASS_MANAGER
{
    VSC_SHADER_COMPILER_PARAM* pCompilerParam;

    /* User options control each pass */
    VSC_OPTN_Options*          options;

    /* Control related analysis info */
    VIR_CALL_GRAPH             callGraph;

    /* Global data related analysis info */
    VIR_DEF_USAGE_INFO         duInfo;
    VIR_LIVENESS_INFO          lvInfo;
}VSC_PASS_MANAGER;

void vscInitializePassManager(VSC_PASS_MANAGER* pPassMnger,
                              VSC_SHADER_COMPILER_PARAM* pCompilerParam);

void vscFinalizePassManager(VSC_PASS_MANAGER* pPassMnger);

END_EXTERN_C()

#endif /* __gc_vsc_vir_pass_mnger_h_ */

