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

VX_INTERNAL_API vx_status vxoGraph_RetrieveTopology(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_DetectAllHeadNodes(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_DetectAllTailNodes(vx_graph graph);

VX_INTERNAL_API vx_bool vxoReference_HasWriteDependency(vx_reference ref1, vx_reference ref2);

VX_INTERNAL_API vx_status vxoGraph_getParentNodes(vx_node node, vx_uint32 *num_parents, vx_uint32 **parents);

VX_INTERNAL_API vx_status vxoGraph_getChildNodes(vx_node node, vx_uint32 *num_children, vx_uint32 **children);

VX_INTERNAL_API void vxoGraph_GenerateNextNodeTable(vx_graph graph,
        vx_uint32 lastNodeIndexTable[VX_MAX_REF_COUNT], vx_uint32 lastNodeCount,
        OUT vx_uint32 nextNodeIndexTable[VX_MAX_REF_COUNT], OUT vx_uint32_ptr nextNodeCount,
        INOUT vx_uint32 leftNodeIndexTable[VX_MAX_REF_COUNT], INOUT vx_uint32_ptr leftNodeCount);

VX_INTERNAL_API void vxoGraph_PolluteIfInput(vx_graph graph, vx_reference targetRef);
VX_INTERNAL_API vx_uint32 vxoGraph_GetNextNodeIndex(vx_graph graph, vx_uint32 nodeIndex);
VX_INTERNAL_API vx_status vxoMultiGpu_FreeMemory(vx_node node);

VX_INTERNAL_API vx_status vxoMultiGPU_SplitOperation(vx_node node, vxnne_operation srcOperation);

/* statement functions for generating binary graph online */
VX_INTERNAL_API vx_status vxoGraph_UserKernelPreprocess(vx_graph graph, vx_bool first);

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodeParameters(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodeWriteDependencies(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_Adapter(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_AllocateAllMemoryObjects(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_DetectCycle(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_DetectUnvisitedNodes(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodesByTarget(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_InitializeAllNodeKernels(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_CaculateCostFactors(vx_graph graph);

VX_INTERNAL_API void vxoGraph_GenerateOperationTable(vx_graph graph);

VX_INTERNAL_API void vxoGraph_GenerateOpParentChild(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraphParallel_AnalyzeOperationsBefore(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_PredictPerf(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_VerifyTiling(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraph_VerifyVirtualBuffer(vx_graph graph);

VX_INTERNAL_API void vxoGraph_GenerateAllNodeIndexTable(vx_graph graph);

VX_INTERNAL_API vx_status vxoGraphParallel_AnalyzeOperationsAfter(vx_graph graph);

VX_INTERNAL_API void vxoGraph_VerifyOperationSync(vx_graph graph);

EXTERN_C_END

#endif /* __GC_VX_GRAPH_H__ */

