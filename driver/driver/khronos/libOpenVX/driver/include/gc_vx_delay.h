/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __GC_VX_DELAY_H__
#define __GC_VX_DELAY_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_delay vxoDelay_Create(vx_context context, vx_reference exemplar, vx_size count);

VX_INTERNAL_CALLBACK_API void vxoDelay_Destructor(vx_reference ref);

VX_INTERNAL_API vx_bool vxoParameterValue_BindToDelay(vx_reference value, vx_node node, vx_uint32 index);

VX_INTERNAL_API vx_bool vxoParameterValue_UnbindFromDelay(vx_reference value, vx_node node, vx_uint32 index);

EXTERN_C_END

#endif /* __GC_VX_DELAY_H__ */
