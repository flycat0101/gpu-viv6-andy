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


#ifndef __gc_vsc_vir_fcp_h_
#define __gc_vsc_vir_fcp_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

#define VIR_FCP_INVALID_REG          0x3FF

VSC_ErrCode VIR_FCP_PerformOnShader(
    VIR_Shader          *pShader,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VSC_OPTN_FCPOptions *pOptions,
    VIR_Dumper          *pDumper);

END_EXTERN_C()

#endif /* __gc_vsc_vir_misc_opts_h_ */



