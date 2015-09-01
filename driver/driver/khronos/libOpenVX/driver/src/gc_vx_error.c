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


#include <gc_vx_common.h>

VX_INTERNAL_API vx_error vxoError_Create(vx_context context, vx_status status)
{
    vx_error error = (vx_error)vxoReference_Create(context, VX_TYPE_ERROR, VX_REF_INTERNAL, &context->base);

    if (error == VX_NULL) return VX_NULL;

    error->status = status;

    return error;
}

VX_INTERNAL_API vx_status vxError_Release(vx_error_ptr errorPtr)
{
    return vxoReference_Release((vx_reference_ptr)errorPtr, VX_TYPE_ERROR, VX_REF_INTERNAL);
}
