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


#ifndef __gc_vsc_vir_misc_opts_h_
#define __gc_vsc_vir_misc_opts_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

VSC_ErrCode vscVIR_RemoveNop(VIR_Shader* pShader);

VSC_ErrCode vscVIR_PutImmValueToUniform(VIR_Shader* pShader, VSC_HW_CONFIG* pHwCfg);

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VIR_Shader* pShader, VSC_HW_CONFIG* pHwCfg);

VSC_ErrCode vscVIR_AdjustPrecision(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo, VSC_HW_CONFIG* pHwCfg);

VSC_ErrCode vscVIR_PatchDual16Shader(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo);

VSC_ErrCode vscVIR_ConvertVirtualInstructions(VIR_Shader* pShader);
END_EXTERN_C()

#endif /* __gc_vsc_vir_misc_opts_h_ */

