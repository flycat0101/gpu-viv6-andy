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


#ifndef __GC_VX_NODE_H__
#define __GC_VX_NODE_H__

EXTERN_C_BEGIN

#define HIGH_PRECISION_COMPUTE 0
VX_INTERNAL_API vx_node vxoNode_CreateSpecific(
        vx_graph graph, vx_enum kernelEnum, vx_reference parameters[], vx_uint32 paramCount);

VX_INTERNAL_API void vxoNode_Dump(vx_node node);

VX_INTERNAL_CALLBACK_API void vxoNode_Destructor(vx_reference ref);

VX_INTERNAL_API vx_bool vxoNode_Adapter(vx_graph graph, vx_node node, vx_uint32 index);

VX_INTERNAL_API vx_status vxoNode_SetParameter(vx_node node, vx_uint32 index, vx_reference value);

VX_INTERNAL_API vx_parameter vxoNode_GetParameter(vx_node node, vx_uint32 index);

typedef enum vx_node_attribute_internal_e
{
    VX_NODE_TILE_MEMORY_PTR = VX_ATTRIBUTE_BASE(VX_ID_VIVANTE, VX_TYPE_NODE) + 0x0,
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

VX_INTERNAL_API vx_status vxoNode_GetTriggerCNNEventID(vx_node node, vx_uint32 * eventID);

VX_INTERNAL_API vx_status vxoNode_SetWaitCNNEventID0(vx_node node, vx_uint32 eventID);

VX_INTERNAL_API vx_status vxoNode_SetWaitCNNEventID1(vx_node node, vx_uint32 eventID);

VX_INTERNAL_API vx_bool vxoNode_IsGPUNode(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_HasCPUfunction(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_IsConvNode(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_IsFCNode(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_IsLeakyReluNode(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_IsMaxPoolingNode(vx_node node);

VX_INTERNAL_API void vxoNode_getInfoFromFCNode(vx_node FCnode, vx_uint32 *pad, vx_uint8 *acc,
                                               vx_uint32 *rounding, vx_uint32 *overflow, vx_uint32 *down_scale_round);


VX_INTERNAL_API vx_status vxoNode_setTensorVxcOptimize(vx_node node);

VX_INTERNAL_API vx_status vxoNode_resetTensorVxcOptimize(vx_node node);

VX_INTERNAL_API vx_bool vxoNode_CheckF32Support(vx_node node);
EXTERN_C_END

#endif /* __GC_VX_NODE_H__ */

