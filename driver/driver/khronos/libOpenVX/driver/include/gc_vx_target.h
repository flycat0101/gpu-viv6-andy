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


#ifndef __GC_VX_TARGET_H__
#define __GC_VX_TARGET_H__

EXTERN_C_BEGIN

typedef enum _vx_target_priority_e
{
    VX_TARGET_PRIORITY_DEFAULT,

    VX_TARGET_PRIORITY_MAX
}
vx_target_priority_e;

VX_INTERNAL_API void vxoTarget_Dump(vx_target target, vx_uint32 index);

VX_INTERNAL_API vx_status vxoTarget_Load(vx_context context, vx_string moduleName);

VX_INTERNAL_API vx_status vxoTarget_Unload(vx_context context, vx_uint32 index, vx_bool unloadModule);

EXTERN_C_END

#endif /* __GC_VX_TARGET_H__ */

