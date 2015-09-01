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


#ifndef __gc_vsc_vir_ep_gen_h_
#define __gc_vsc_vir_ep_gen_h_

BEGIN_EXTERN_C()

VSC_ErrCode
vscVIR_GenerateSEP(
    IN  VIR_Shader*                pShader,
    IN  VSC_HW_CONFIG*             pHwCfg,
    IN  gctBOOL                    bDumpSEP,
    OUT SHADER_EXECUTABLE_PROFILE* pOutSEP
    );

END_EXTERN_C()

#endif /* __gc_vsc_vir_ep_gen_h_ */


