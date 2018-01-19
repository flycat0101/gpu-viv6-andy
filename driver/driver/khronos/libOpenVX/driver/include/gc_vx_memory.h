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


#ifndef __GC_VX_MEMORY_H__
#define __GC_VX_MEMORY_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_bool vxoMemory_Allocate(vx_context context, vx_memory memory);

VX_INTERNAL_API vx_bool vxoMemory_Free(vx_context context, vx_memory memory);

VX_INTERNAL_API vx_bool vxoMemory_WrapUserMemory(vx_context context, vx_memory memory);

VX_INTERNAL_API vx_bool vxoMemory_FreeWrappedMemory(vx_context context, vx_memory memory);

VX_INTERNAL_API void vxoMemory_Dump(vx_memory memory);

VX_INTERNAL_API vx_size vxoMemory_ComputeSize(vx_memory memory, vx_uint32 planeIndex);

VX_INTERNAL_API vx_size vxoMemory_ComputeElementCount(vx_memory memory, vx_uint32 planeIndex);

VX_INTERNAL_API vx_status vxoMemory_CAllocate(vx_context context, void** memory, vx_uint32 size);

VX_INTERNAL_API vx_status vxoMemory_CFree(vx_context context, void** memory);

EXTERN_C_END

#endif /* __GC_VX_MEMORY_H__ */

