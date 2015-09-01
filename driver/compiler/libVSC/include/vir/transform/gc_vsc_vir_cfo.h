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


#ifndef __gc_vsc_vir_cfo_h_
#define __gc_vsc_vir_cfo_h_

/******************************************************************************
            control flow optimization
            including: if conversion

******************************************************************************/

#include "gc_vsc.h"

BEGIN_EXTERN_C()

extern VSC_ErrCode VSC_CFO_PerformOnShader(
    IN VIR_Shader           *shader,
    IN VSC_OPTN_LCSEOptions *options,
    IN VIR_Dumper           *dumper
    );

END_EXTERN_C()

#endif /* __gc_vsc_vir_cfo_h_ */

