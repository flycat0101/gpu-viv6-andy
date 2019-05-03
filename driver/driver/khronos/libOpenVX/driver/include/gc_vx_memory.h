/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
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
VX_INTERNAL_API vx_bool vxoMemory_AllocateEx(vx_context context, vx_memory memory);
VX_INTERNAL_API vx_bool vxoMemory_AllocateSize(vx_context context, vx_memory memory, vx_size size);

VX_INTERNAL_API vx_bool vxoMemory_Free(vx_context context, vx_memory memory);
VX_INTERNAL_API vx_bool vxoMemory_FreeEx(vx_context context, vx_memory memory);

VX_INTERNAL_API vx_bool vxoMemory_WrapUserMemory(vx_context context, vx_memory memory);

VX_INTERNAL_API vx_bool vxoMemory_FreeWrappedMemory(vx_context context, vx_memory memory);

VX_INTERNAL_API void vxoMemory_Dump(vx_memory memory);

VX_INTERNAL_API vx_size vxoMemory_ComputeSize(vx_memory memory, vx_uint32 planeIndex);

VX_INTERNAL_API vx_size vxoMemory_ComputeElementCount(vx_memory memory, vx_uint32 planeIndex);

VX_INTERNAL_API vx_bool vxoMemoryPool_Initialize(vx_graph graph, vx_size size);
VX_INTERNAL_API vx_bool vxoMemoryPool_Deinitialize(vx_graph graph);
VX_INTERNAL_API vx_bool vxoMemoryPool_LockDown(vx_graph graph, vx_size size);
VX_INTERNAL_API vx_bool vxoMemoryPool_Reset(vx_graph graph, vx_bool freeMem);
VX_INTERNAL_API vx_bool vxoMemoryPool_GetBase(vx_graph graph, vx_uint8_ptr* logical, vx_uint32* physical);

VX_INTERNAL_API vx_size
vxoMemory_GetSize(
    vx_memory memory
    );

VX_INTERNAL_API void
vxoMemory_SetSize(
    vx_memory memory,
    vx_size   size
    );

VX_INTERNAL_API vx_enum
vxoMemory_GetType(
    vx_memory memory
    );

VX_INTERNAL_API void
vxoMemory_SetType(
    vx_memory memory,
    vx_enum   type
    );

VX_INTERNAL_API void
vxoMemory_SetAddress(
    vx_memory    memory,
    vx_uint8_ptr logical,
    vx_uint32    physical
    );

VX_INTERNAL_API void
vxoMemory_ResetOffset(
    vx_memory memory
    );

VX_INTERNAL_API vx_status
vxoMemoryPool_RequestList(
    vx_graph            graph,
    vxnne_mem_request   list,
    vx_uint32           list_count,
    vx_uint32           start,
    vx_uint32           count,
    vx_uint32           *max_sizes
    );

EXTERN_C_END

#endif /* __GC_VX_MEMORY_H__ */

