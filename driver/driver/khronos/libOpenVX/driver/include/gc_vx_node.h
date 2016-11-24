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


#ifndef __GC_VX_NODE_H__
#define __GC_VX_NODE_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_node vxoNode_CreateSpecific(
        vx_graph graph, vx_enum kernelEnum, vx_reference parameters[], vx_uint32 paramCount);

VX_INTERNAL_API void vxoNode_Dump(vx_node node);

VX_INTERNAL_CALLBACK_API void vxoNode_Destructor(vx_reference ref);

VX_INTERNAL_API vx_status vxoNode_SetParameter(vx_node node, vx_uint32 index, vx_reference value);

VX_INTERNAL_API vx_parameter vxoNode_GetParameter(vx_node node, vx_uint32 index);

typedef enum vx_node_attribute_internal_e
{
    VX_NODE_ATTRIBUTE_TILE_MEMORY_PTR = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_NODE) + 0xD,
}
vx_node_attribute_internal_e;

VX_INTERNAL_API void vxoNode_RemoveFromGraph(vx_node_ptr nodePtr);

VX_INTERNAL_API vx_status vxoNode_SetChildGraph(vx_node node, vx_graph graph);

VX_INTERNAL_API vx_graph vxoNode_GetChildGraph(vx_node node);

VX_INTERNAL_API void vxoNode_EnableVirtualAccessible(vx_node node);

VX_INTERNAL_API void vxoNode_DisableVirtualAccessible(vx_node node);

VX_INTERNAL_API vx_status vxoNode_Record(vx_node node);

VX_INTERNAL_API vx_status vxoNode_Replay(vx_node node);

VX_INTERNAL_API vx_status vxoNode_Release(vx_node_ptr nodePtr);

EXTERN_C_END

#endif /* __GC_VX_NODE_H__ */

