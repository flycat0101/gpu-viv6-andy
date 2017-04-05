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


#ifndef __gc_vsc_vir_ep_gen_h_
#define __gc_vsc_vir_ep_gen_h_

BEGIN_EXTERN_C()

typedef struct _VSC_PEP_GEN_PRIV_DATA
{
    PROGRAM_EXECUTABLE_PROFILE*  pOutPEP;
    PEP_CLIENT                   client;
}VSC_PEP_GEN_PRIV_DATA;

VSC_ErrCode
vscVIR_GenerateSEP(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(vscVIR_GenerateSEP);

VSC_ErrCode
vscVIR_GeneratePEP(
    VSC_GPG_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(vscVIR_GeneratePEP);

END_EXTERN_C()

#endif /* __gc_vsc_vir_ep_gen_h_ */


