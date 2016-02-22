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


#ifndef __GC_VX_GRAPH_H__
#define __GC_VX_GRAPH_H__

EXTERN_C_BEGIN

VX_INTERNAL_API void vxoGraph_Dump(vx_graph graph);

VX_INTERNAL_API void vxoGraph_ClearAllVisitedFlags(vx_graph graph);

VX_INTERNAL_API void vxoGraph_ClearAllExecutedFlags(vx_graph graph);

VX_INTERNAL_CALLBACK_API void vxoGraph_Destructor(vx_reference ref);

VX_INTERNAL_API vx_status vxoGraph_SetParameter(vx_graph graph, vx_uint32 index, vx_reference value);

VX_INTERNAL_API vx_status vxoGraph_FindAllRelatedNodes(
        vx_graph graph, vx_direction_e direction, vx_reference ref,
        OUT vx_uint32 nodeIndexTable[], INOUT vx_uint32 *nodeCountPtr);

VX_INTERNAL_API void vxoGraph_GenerateNextNodeTable(vx_graph graph,
        vx_uint32 lastNodeIndexTable[VX_MAX_REF_COUNT], vx_uint32 lastNodeCount,
        OUT vx_uint32 nextNodeIndexTable[VX_MAX_REF_COUNT], OUT vx_uint32_ptr nextNodeCount,
        INOUT vx_uint32 leftNodeIndexTable[VX_MAX_REF_COUNT], INOUT vx_uint32_ptr leftNodeCount);

VX_INTERNAL_API void vxoGraph_PolluteIfInput(vx_graph graph, vx_reference targetRef);

EXTERN_C_END

#endif /* __GC_VX_GRAPH_H__ */
