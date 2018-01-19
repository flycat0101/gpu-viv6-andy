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


#ifndef __gc_vsc_vir_ssa_h_
#define __gc_vsc_vir_ssa_h_

BEGIN_EXTERN_C()

VSC_ErrCode vscVIR_Transform2SSA(VIR_Shader* pShader);
VSC_ErrCode vscVIR_TransformFromSSA(VIR_Shader* pShader);
VSC_ErrCode vscVIR_TransformFromSpvSSA(VIR_Shader* pShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_ssa_h_ */

