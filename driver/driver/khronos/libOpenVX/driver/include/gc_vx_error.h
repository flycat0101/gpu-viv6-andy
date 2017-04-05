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


#ifndef __GC_VX_ERROR_H__
#define __GC_VX_ERROR_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_error vxoError_Create(vx_context context, vx_status status);

VX_INTERNAL_API vx_status vxoError_Release(vx_error_ptr errorPtr);

EXTERN_C_END

#endif /* __GC_VX_ERROR_H__ */


