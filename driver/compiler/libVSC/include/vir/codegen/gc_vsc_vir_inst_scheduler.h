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


#ifndef __gc_vsc_vir_inst_sched_h_
#define __gc_vsc_vir_inst_sched_h_

#include "gc_vsc.h"
#include "vir/analysis/gc_vsc_vir_dfa.h"

BEGIN_EXTERN_C()

#define VSC_IS_DEPDAG_NODES_BV_COUNT    2
#define VSC_IS_DEPDAG_EDGES_BV_COUNT    3
/* data structure of dependence DAG */
typedef struct VSC_IS_DEPDAG
{
    VSC_DIRECTED_GRAPH d_graph;
    VSC_HASH_TABLE inst2node;
    VSC_SIMPLE_RESIZABLE_ARRAY kill_nodes_array;
    VSC_BIT_VECTOR* nodes_bvs[VSC_IS_DEPDAG_NODES_BV_COUNT];
    gctUINT using_nodes_bvs[VSC_IS_DEPDAG_NODES_BV_COUNT];
    VSC_BIT_VECTOR* edges_bvs[VSC_IS_DEPDAG_EDGES_BV_COUNT];
    gctUINT using_edges_bvs[VSC_IS_DEPDAG_EDGES_BV_COUNT];
    VSC_MM* mm;
} VSC_IS_DepDag;

#define VSC_IS_DepDag_GetDGraph(dd)             (&(dd)->d_graph)
#define VSC_IS_DepDag_GetNodeCount(dd)          ((dd)->d_graph.nodeList.info.count)
#define VSC_IS_DepDag_GetGeneratedNodeCount(dd) vscDG_GetHistNodeCount(VSC_IS_DepDag_GetDGraph(dd))
#define VSC_IS_DepDag_GetGeneratedEdgeCount(dd) vscDG_GetHistEdgeCount(VSC_IS_DepDag_GetDGraph(dd))
#define VSC_IS_DepDag_GetInst2Node(dd)          (&(dd)->inst2node)
#define VSC_IS_DepDag_GetKillNodesArray(dd)     (&(dd)->kill_nodes_array)
#define VSC_IS_DepDag_GetNodes_BV(dd, i)        ((dd)->nodes_bvs[i])
#define VSC_IS_DepDag_SetNodes_BV(dd, i, bv)    ((dd)->nodes_bvs[i] = (bv))
#define VSC_IS_DepDag_GetUsingNodesBVs(dd, i)   ((dd)->using_nodes_bvs[i])
#define VSC_IS_DepDag_SetUsingNodesBVs(dd, i)   ((dd)->using_nodes_bvs[i] = 1)
#define VSC_IS_DepDag_ResetUsingNodesBVs(dd, i) ((dd)->using_nodes_bvs[i] = 0)
#define VSC_IS_DepDag_GetEdges_BV(dd, i)        ((dd)->edges_bvs[i])
#define VSC_IS_DepDag_SetEdges_BV(dd, i, bv)    ((dd)->edges_bvs[i] = (bv))
#define VSC_IS_DepDag_GetUsingEdgesBVs(dd, i)   ((dd)->using_edges_bvs[i])
#define VSC_IS_DepDag_SetUsingEdgesBVs(dd, i)   ((dd)->using_edges_bvs[i] = 1)
#define VSC_IS_DepDag_ResetUsingEdgesBVs(dd, i) ((dd)->using_edges_bvs[i] = 0)
#define VSC_IS_DepDag_GetMM(dd)                 ((dd)->mm)
#define VSC_IS_DepDag_SetMM(dd, m)              ((dd)->mm = (m))

typedef struct VIR_IS_INSTSCHED
{
    VIR_Shader* shader;
    VSC_HW_CONFIG*  hwCfg;
    VSC_HW_UARCH_CAPS* hwUArchCaps;
    VIR_BASIC_BLOCK* curr_bb;
    VIR_DEF_USAGE_INFO* du_info;
    VSC_IS_DepDag* curr_dep_dag;
    gctUINT32 register_count;
    gctUINT32 texld_dep_bubble;
    gctUINT32 memld_dep_bubble;
    gctUINT32 memst_dep_bubble;
    gctUINT32 cacheld_dep_bubble;
    gctUINT32 cachest_dep_bubble;
    gctUINT32 texld_interface_bubble;
    gctUINT32 memld_interface_bubble;
    VSC_OPTN_ISOptions* options;
    VIR_Dumper* dumper;
    VSC_MM* pMM;
} VSC_IS_InstSched;

#define VSC_IS_InstSched_GetShader(is)                      ((is)->shader)
#define VSC_IS_InstSched_SetShader(is, s)                   ((is)->shader = (s))
#define VSC_IS_InstSched_GetHwCfg(is)                       ((is)->hwCfg)
#define VSC_IS_InstSched_SetHwCfg(is, h)                    ((is)->hwCfg = (h))
#define VSC_IS_InstSched_GetHwUArchCaps(is)                 ((is)->hwUArchCaps)
#define VSC_IS_InstSched_SetHwUArchCaps(is, h)              ((is)->hwUArchCaps = (h))
#define VSC_IS_InstSched_GetCurrBB(is)                      ((is)->curr_bb)
#define VSC_IS_InstSched_SetCurrBB(is, b)                   ((is)->curr_bb = (b))
#define VSC_IS_InstSched_GetDUInfo(is)                      ((is)->du_info)
#define VSC_IS_InstSched_SetDUInfo(is, d)                   ((is)->du_info = (d))
#define VSC_IS_InstSched_GetCurrDepDag(is)                  ((is)->curr_dep_dag)
#define VSC_IS_InstSched_SetCurrDepDag(is, c)               ((is)->curr_dep_dag = (c))
#define VSC_IS_InstSched_GetRegisterCount(is)               ((is)->register_count)
#define VSC_IS_InstSched_SetRegisterCount(is, r)            ((is)->register_count = (r))
#define VSC_IS_InstSched_GetTexldDepBubble(is)              ((is)->texld_dep_bubble)
#define VSC_IS_InstSched_SetTexldDepBubble(is, t)           ((is)->texld_dep_bubble = (t))
#define VSC_IS_InstSched_GetMemldDepBubble(is)              ((is)->memld_dep_bubble)
#define VSC_IS_InstSched_SetMemldDepBubble(is, m)           ((is)->memld_dep_bubble = (m))
#define VSC_IS_InstSched_GetMemstDepBubble(is)              ((is)->memst_dep_bubble)
#define VSC_IS_InstSched_SetMemstDepBubble(is, m)           ((is)->memst_dep_bubble = (m))
#define VSC_IS_InstSched_GetCacheldDepBubble(is)            ((is)->cacheld_dep_bubble)
#define VSC_IS_InstSched_SetCacheldDepBubble(is, a)         ((is)->cacheld_dep_bubble = (a))
#define VSC_IS_InstSched_GetCachestDepBubble(is)            ((is)->cachest_dep_bubble)
#define VSC_IS_InstSched_SetCachestDepBubble(is, a)         ((is)->cachest_dep_bubble = (a))
#define VSC_IS_InstSched_GetTexldInterfaceBubble(is)        ((is)->texld_interface_bubble)
#define VSC_IS_InstSched_SetTexldInterfaceBubble(is, t)     ((is)->texld_interface_bubble = (t))
#define VSC_IS_InstSched_GetMemldInterfaceBubble(is)        ((is)->memld_interface_bubble)
#define VSC_IS_InstSched_SetMemldInterfaceBubble(is, m)     ((is)->memld_interface_bubble = (m))
#define VSC_IS_InstSched_GetOptions(is)                     ((is)->options)
#define VSC_IS_InstSched_SetOptions(is, o)                  ((is)->options = (o))
#define VSC_IS_InstSched_GetDumper(is)                      ((is)->dumper)
#define VSC_IS_InstSched_SetDumper(is, d)                   ((is)->dumper = (d))
#define VSC_IS_InstSched_GetMM(is)                          ((is)->pMM)
#define VSC_IS_InstSched_SetMM(is, m)                       ((is)->pMM = (m))

#define PRE_PASS_IS 0
#define PST_PASS_IS 1

extern VSC_ErrCode VSC_IS_InstSched_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VSC_IS_InstSched_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_IS_InstSched_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_inst_sched_h_ */

