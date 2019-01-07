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


#include "gc_vsc.h"
#include "vir/codegen/gc_vsc_vir_inst_scheduler.h"

typedef struct VSC_IS_DEPDAGEDGE VSC_IS_DepDagEdge;

typedef enum VSC_IS_DEPDAGNODE_FLAG
{
    VSC_IS_DEPDAGNODE_FLAG_NONE = 0,
    VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_PRED             = 0x01,
    VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC             = 0x02,
    VSC_IS_DEPDAGNODE_FLAG_DEPENDING_MOVA               = 0x04,
    VSC_IS_DEPDAGNODE_FLAG_1ST_TIME_DETOURS_TRIED       = 0x08,
    VSC_IS_DEPDAGNODE_FLAG_1ST_TIME_DETOURS_UNFRIENDLY  = 0x10,
    VSC_IS_DEPDAGNODE_FLAG_2nd_DETOURS_TRIED            = 0x20,
    VSC_IS_DEPDAGNODE_FLAG_ALL_DETOURS_DONE             = 0x40,
    VSC_IS_DEPDAGNODE_FLAG_FORK_MERGED                  = 0x80,
} VSC_IS_DepDagNode_Flag;

/* data structure of the node in a dependence DAG */
typedef struct VSC_IS_DEPDAGNODE
{
    VSC_DG_NODE node;
    VIR_Instruction* inst;
    VSC_IS_DepDagNode_Flag flags;
    gctINT32 scheduled_position;
    gctUINT kill_priority;
    struct VSC_IS_DEPDAGNODE* next;
} VSC_IS_DepDagNode;

#define VSC_IS_DepDagNode_GetNode(nd)                       (&((nd)->node))
#define VSC_IS_DepDagNode_GetInst(nd)                       ((nd)->inst)
#define VSC_IS_DepDagNode_SetInst(nd, i)                    ((nd)->inst = (i))
#define VSC_IS_DepDagNode_GetID(nd)                         ((nd)->node.id)
#define VSC_IS_DepDagNode_GetFlags(nd)                      ((nd)->flags)
#define VSC_IS_DepDagNode_SetFlags(nd, f)                   ((nd)->flags = (f))
#define VSC_IS_DepDagNode_AddFlag(nd, f)                    ((nd)->flags |= (f))
#define VSC_IS_DepDagNode_RemoveFlag(nd, f)                 ((nd)->flags &= ~(f))
#define VSC_IS_DepDagNode_HasFlag(nd, f)                    ((nd)->flags & (f))
#define VSC_IS_DepDagNode_GetScheduledPosition(nd)          ((nd)->scheduled_position)
#define VSC_IS_DepDagNode_SetScheduledPosition(nd, s)       ((nd)->scheduled_position = (s))
#define VSC_IS_DepDagNode_IsScheduled(nd)                   ((nd)->scheduled_position != -1)
#define VSC_IS_DepDagNode_GetKillPriority(nd)               ((nd)->kill_priority)
#define VSC_IS_DepDagNode_SetKillPriority(nd, kp)           ((nd)->kill_priority = (kp))
#define VSC_IS_DepDagNode_GetNext(nd)                       ((nd)->next)
#define VSC_IS_DepDagNode_SetNext(nd, n)                    ((nd)->next = (n))
#define VSC_IS_DepDagNode_GetSuccEdgeList(nd)               ((VSC_ADJACENT_LIST*)&((nd)->node.succList))
#define VSC_IS_DepDagNode_GetPredEdgeList(nd)               ((VSC_ADJACENT_LIST*)&((nd)->node.predList))
#define VSC_IS_DepDagNode_GetInDegree(nd)                   (DGND_GET_IN_DEGREE(&(nd)->node))
#define VSC_IS_DepDagNode_GetOutDegree(nd)                  (DGND_GET_OUT_DEGREE(&(nd)->node))

static void _VSC_IS_DepDagNode_Init(
    IN OUT VSC_IS_DepDagNode* node,
    IN VIR_Instruction* inst
    )
{
    vscDGND_Initialize(VSC_IS_DepDagNode_GetNode(node));
    VSC_IS_DepDagNode_SetInst(node, inst);
    VSC_IS_DepDagNode_SetFlags(node, VSC_IS_DEPDAGNODE_FLAG_NONE);
    VSC_IS_DepDagNode_SetScheduledPosition(node, -1);
    VSC_IS_DepDagNode_SetKillPriority(node, 0);
}

static void _VSC_IS_DepDagNode_Dump(
    IN VSC_IS_DepDagNode* node,
    IN VIR_Dumper* dumper
    );
static VSC_IS_DepDagNode* _VSC_IS_DepDagNode_DumpWithEdge(
    IN VSC_IS_DepDagNode* node,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    IN VIR_Dumper* dumper
    );
static void _VSC_IS_DepDagNode_DumpList(
    IN VSC_IS_DepDagNode* start,
    IN VSC_IS_DepDagNode* end,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    IN VIR_Dumper* dumper
    );
static void _VSC_IS_DepDagNode_DumpWithPredSucc(
    IN VSC_IS_DepDagNode* node,
    IN VIR_Dumper* dumper
    );

static gctBOOL _VSC_IS_DepDagNode_DepandsOnNode(
    IN VSC_IS_DepDagNode* node1,
    IN VSC_IS_DepDagNode* node2
    );

static gctUINT32 _VSC_IS_DepDagNode_DepandsOnBubbleSet(
    IN VSC_IS_DepDagNode* node,
    IN VSC_HASH_TABLE* set
    );

/* enum for instruction conflict types */
/* TL           texture load instruction */
/* ML           memory load instruction */
/* MS           memory store instruction */
/* RU           register use*/
/* RS           register set*/
/* COND         conditional dependence */
typedef enum VSC_IS_CONFLICTTYPE
{
    VSC_IS_ConflictType_NONE                = 0,

    VSC_IS_ConflictType_TLRS_RU             = 0x1, /* register save from texture load, to, register use */
    VSC_IS_ConflictType_TLRS_RS             = 0x2, /* register save from texture load, to, register save */

    VSC_IS_ConflictType_MLRS_RU             = 0x4, /* register save from memory load, to, register use */
    VSC_IS_ConflictType_MLRS_RS             = 0x8, /* register save from memory load, to, register save */

    VSC_IS_ConflictType_ML_MS               = 0x10, /* memory load, to, memory store */
    VSC_IS_ConflictType_MS_ML               = 0x20, /* memory store, to, memory load */
    VSC_IS_ConflictType_MS_MS               = 0x40, /* memory store, to, memory store */

    VSC_IS_ConflictType_CLRS_RU             = 0x80, /* register save from cache load, to, register use */
    VSC_IS_ConflictType_CLRS_RS             = 0x100, /* register save from cache load, to, register save */

    VSC_IS_ConflictType_CL_CS               = 0x200, /* cache load, to, cache store */
    VSC_IS_ConflictType_CS_CL               = 0x400, /* cache store, to, cache load */
    VSC_IS_ConflictType_CS_CS               = 0x800, /* cache store, to, cache store */

    VSC_IS_ConflictType_RS_RU               = 0x1000, /* register save, to, register use */
    VSC_IS_ConflictType_RU_RS               = 0x2000, /* register use, to, register save */
    VSC_IS_ConflictType_RS_RS               = 0x4000, /* register save, to, register save */

    VSC_IS_ConflictType_COND                = 0x8000, /* conditional */
    VSC_IS_ConflictType_LOOP_CARRIED        = 0x10000, /* to flag loop carried dependency */
    VSC_IS_ConflictType_CONTINUOUS_BINDING  = 0x20000, /* if A depends on B, after B is scheduled, A must be scheduled immediately */

                                                           /* loose binding means, if A depends on B, some instructions can be scheduled in between but some cannot */
    VSC_IS_ConflictType_LOOSE_BINDING_LDARR = 0x40000, /* between LDARR and its RSRU dependency. LDARR's RURS dependency should dodge */
    VSC_IS_ConflictType_LOOSE_BINDING_MOVA  = 0x80000, /* between MOVA and its RSRU dependency. other MOVAs should dodge */
    VSC_IS_ConflictType_DODGING             = 0x100000, /* if A has loose binding dependency with B, C is up to schedule but cannot be scheduled
                                                            * in between A and B, then set C have DODGING dependency with A OR B */
    VSC_IS_ConflictType_USE_RETURNVALUE     = 0x200000, /* the instruction uses return value of previous call,
                                                            * cannot move across TEXLD, otherwise it will break return
                                                            * value rename after inline when the texld be convert to call */
    VSC_IS_ConflictType_BARRIER             = 0x400000, /* Barrier instruction has dependency with all other instructions
                                                            * in the same bb */
    VSC_IS_ConflictType_EMIT                = 0x800000, /* Emit instruction has dependency with all previous instructions
                                                            * which stores output */
    VSC_IS_ConflictType_ATOMIC              = 0x1000000    /* atomic instruction has dependency with all previous atomic instructions */
} VSC_IS_ConflictType;

#define VSC_IS_ConflictType_ExclusivelySet(ct_var, ct_exclude, ct_value)       ((ct_var) |= ((~(ct_exclude)) & (ct_value)))
#define VSC_IS_ConflictType_Set(ct_var, ct_value)       ((ct_var) |= (ct_value))
#define VSC_IS_ConflictType_RESET(ct_var, ct_value)     ((ct_var) ^= (ct_var) & (ct_value))
#define VSC_IS_ConflictType_HasTLRS_RU(ct)              ((ct) & VSC_IS_ConflictType_TLRS_RU)
#define VSC_IS_ConflictType_HasTLRS_RS(ct)              ((ct) & VSC_IS_ConflictType_TLRS_RS)
#define VSC_IS_ConflictType_HasCLRS_RU(ct)              ((ct) & VSC_IS_ConflictType_CLRS_RU)
#define VSC_IS_ConflictType_HasCLRS_RS(ct)              ((ct) & VSC_IS_ConflictType_CLRS_RS)
#define VSC_IS_ConflictType_HasCL_CS(ct)                ((ct) & VSC_IS_ConflictType_CL_CS)
#define VSC_IS_ConflictType_HasCS_CL(ct)                ((ct) & VSC_IS_ConflictType_CS_CL)
#define VSC_IS_ConflictType_HasCS_CS(ct)                ((ct) & VSC_IS_ConflictType_CS_CS)
#define VSC_IS_ConflictType_HasMLRS_RU(ct)              ((ct) & VSC_IS_ConflictType_MLRS_RU)
#define VSC_IS_ConflictType_HasMLRS_RS(ct)              ((ct) & VSC_IS_ConflictType_MLRS_RS)
#define VSC_IS_ConflictType_HasML_MS(ct)                ((ct) & VSC_IS_ConflictType_ML_MS)
#define VSC_IS_ConflictType_HasMS_ML(ct)                ((ct) & VSC_IS_ConflictType_MS_ML)
#define VSC_IS_ConflictType_HasMS_MS(ct)                ((ct) & VSC_IS_ConflictType_MS_MS)
#define VSC_IS_ConflictType_HasRS_RU(ct)                ((ct) & VSC_IS_ConflictType_RS_RU)
#define VSC_IS_ConflictType_HasRU_RS(ct)                ((ct) & VSC_IS_ConflictType_RU_RS)
#define VSC_IS_ConflictType_HasRS_RS(ct)                ((ct) & VSC_IS_ConflictType_RS_RS)
#define VSC_IS_ConflictType_HasCOND(ct)                 ((ct) & VSC_IS_ConflictType_COND)
#define VSC_IS_ConflictType_IsLOOPCARRIED(ct)           ((ct) & VSC_IS_ConflictType_LOOP_CARRIED)
#define VSC_IS_ConflictType_HasContinuousBinding(ct)    ((ct) & VSC_IS_ConflictType_CONTINUOUS_BINDING)
#define VSC_IS_ConflictType_HasLooseBindingLDARR(ct)    ((ct) & VSC_IS_ConflictType_LOOSE_BINDING_LDARR)
#define VSC_IS_ConflictType_HasLooseBindingMOVA(ct)     ((ct) & VSC_IS_ConflictType_LOOSE_BINDING_MOVA)
#define VSC_IS_ConflictType_HasDodging(ct)              ((ct) & VSC_IS_ConflictType_DODGING)
#define VSC_IS_ConflictType_UseReturnValue(ct)          ((ct) & VSC_IS_ConflictType_USE_RETURNVALUE)
#define VSC_IS_ConflictType_HasBarrier(ct)              ((ct) & VSC_IS_ConflictType_BARRIER)
#define VSC_IS_ConflictType_HasAtomic(ct)               ((ct) & VSC_IS_ConflictType_ATOMIC)

static gctUINT32 _VSC_IS_ConflictType_GetBubble(
    IN VSC_IS_ConflictType conflict_types,
    IN VSC_IS_InstSched* is
    )
{
    gctUINT32 max_bubble = 0;
    gctUINT32 bubble;
    if(conflict_types & VSC_IS_ConflictType_CLRS_RU ||
       conflict_types & VSC_IS_ConflictType_CLRS_RS)
    {
        bubble = VSC_IS_InstSched_GetCacheldDepBubble(is);
        if(bubble > max_bubble)
        {
            max_bubble = bubble;
        }
    }
    if(conflict_types & VSC_IS_ConflictType_TLRS_RU ||
       conflict_types & VSC_IS_ConflictType_TLRS_RS)
    {
        bubble = VSC_IS_InstSched_GetTexldDepBubble(is);
        if(bubble > max_bubble)
        {
            max_bubble = bubble;
        }
    }
    if(conflict_types & VSC_IS_ConflictType_MLRS_RU ||
       conflict_types & VSC_IS_ConflictType_MLRS_RS)
    {
        bubble = VSC_IS_InstSched_GetMemldDepBubble(is);
        if(bubble > max_bubble)
        {
            max_bubble = bubble;
        }
    }
    return max_bubble;
}

static void _VSC_IS_ConflictType_Dump(
    IN gctUINT32 conflict_types,
    IN VIR_Dumper* dumper
    );

typedef enum VSC_IS_DEPDAGEDGE_FLAG
{
    VSC_IS_DEPDAGEDGE_FLAG_NONE             = 0x00,
    VSC_IS_DEPDAGEDGE_FLAG_DELETED          = 0x01,
    VSC_IS_DEPDAGEDGE_FLAG_NEWLY_ADDED      = 0x02,
    VSC_IS_DepDagEdge_Flag_4_DETOURS_UNDONE = 0x04,
} VSC_IS_DepDagEdge_Flag;

/* data structure of the edge in a dependence DAG */
struct VSC_IS_DEPDAGEDGE
{
    VSC_DG_EDGE edge;
    gctUINT32 conflict_type;
    VSC_IS_DepDagEdge_Flag flags;
    gctUINT32 bubble;
};

#define VSC_IS_DepDagEdge_GetEdge(eg)               (&((eg)->edge))
#define VSC_IS_DepDagEdge_GetID(eg)                 (VSC_IS_DepDagEdge_GetEdge(eg)->id)
#define VSC_IS_DepDagEdge_Verify(eg)                gcmASSERT((eg)->edge.pFromNode == ((eg) + 1)->edge.pToNode)
#define VSC_IS_DepDagEdge_GetConflictType(eg)       ((eg)->conflict_type)
#define VSC_IS_DepDagEdge_SetConflictType(eg, c)    VSC_IS_DepDagEdge_Verify(eg); (eg)->conflict_type = (c); (eg+1)->conflict_type = (c)
#define VSC_IS_DepDagEdge_ResetConflictType(eg, c)  VSC_IS_DepDagEdge_Verify(eg); (eg)->conflict_type = VSC_IS_ConflictType_NONE; (eg+1)->conflict_type = VSC_IS_ConflictType_NONE
#define VSC_IS_DepDagEdge_AddConflictType(eg, c)    VSC_IS_DepDagEdge_Verify(eg); (eg)->conflict_type |= (c); (eg+1)->conflict_type |= (c)
#define VSC_IS_DepDagEdge_RemoveConflictType(eg, c) VSC_IS_DepDagEdge_Verify(eg); (eg)->conflict_type &= ~(c); (eg+1)->conflict_type &= ~(c)
#define VSC_IS_DepDagEdge_HasConflictType(eg, c)    ((eg)->conflict_type & (c))
#define VSC_IS_DepDagEdge_GetFlags(eg)              ((eg)->flags)
#define VSC_IS_DepDagEdge_SetFlags(eg, f)           VSC_IS_DepDagEdge_Verify(eg); (eg)->flags = (f); (eg+1)->flags = (f)
#define VSC_IS_DepDagEdge_ResetFlags(eg)            VSC_IS_DepDagEdge_Verify(eg); (eg)->flags = VSC_IS_DEPDAGEDGE_FLAG_NONE; (eg+1)->flags = VSC_IS_DEPDAGEDGE_FLAG_NONE
#define VSC_IS_DepDagEdge_AddFlag(eg, f)            VSC_IS_DepDagEdge_Verify(eg); (eg)->flags |= (f); (eg+1)->flags |= (f)
#define VSC_IS_DepDagEdge_RemoveFlag(eg, f)         VSC_IS_DepDagEdge_Verify(eg); (eg)->flags &= ~(f); (eg+1)->flags &= ~(f)
#define VSC_IS_DepDagEdge_HasFlag(eg, f)            ((eg)->flags & (f))
#define VSC_IS_DepDagEdge_GetBubble(eg)             ((eg)->bubble)
#define VSC_IS_DepDagEdge_SetBubble(eg, b)          VSC_IS_DepDagEdge_Verify(eg); (eg)->bubble = (b); (eg + 1)->bubble = (b)
#define VSC_IS_DepDagEdge_IncBubble(eg, b)          VSC_IS_DepDagEdge_Verify(eg); (eg)->bubble += (b); (eg + 1)->bubble += (b)
#define VSC_IS_DepDagEdge_DecBubble(eg, b)          VSC_IS_DepDagEdge_Verify(eg); (eg)->bubble -= (b); (eg + 1)->bubble -= (b)
#define VSC_IS_DepDagEdge_GetFromNode(eg)           ((VSC_IS_DepDagNode*)(eg)->edge.pFromNode)
#define VSC_IS_DepDagEdge_GetToNode(eg)             ((VSC_IS_DepDagNode*)(eg)->edge.pToNode)

static void _VSC_IS_DepDagEdge_Init(
    IN OUT VSC_IS_DepDagEdge* dde
    )
{
    VSC_IS_DepDagEdge_SetConflictType(dde, VSC_IS_ConflictType_NONE);
    VSC_IS_DepDagEdge_ResetFlags(dde);
    VSC_IS_DepDagEdge_SetBubble(dde, 0);
}

static void _VSC_IS_DepDagEgde_Dump(
    IN VSC_IS_DepDagEdge* edge,
    IN VIR_Dumper* dumper
    );

static VSC_IS_DepDagNode* _VSC_IS_DepDagEgde_DumpWithNode(
    IN VSC_IS_DepDagEdge* edge,
    IN gctBOOL forward,
    IN VIR_Dumper* dumper
    );

static void _VSC_IS_DepDag_Dump(
    IN VSC_IS_DepDag* dag,
    IN VIR_Dumper* dumper
    );

static gctBOOL _VSC_IS_DepDagNode_MarkSubTree(
    IN VSC_IS_DepDagNode* node,
    IN gctBOOL succ,
    IN VSC_IS_DepDagNode_Flag exclueNodeflag,
    IN VSC_IS_DepDagEdge_Flag exclueEdgeflag,
    IN VSC_BIT_VECTOR* excluded_nodes_bv,
    IN OUT VSC_BIT_VECTOR* nodes_bv,
    IN OUT VSC_BIT_VECTOR* edges_bv
    )
{
    VSC_ADJACENT_LIST* edge_list = succ ? VSC_IS_DepDagNode_GetSuccEdgeList(node) : VSC_IS_DepDagNode_GetPredEdgeList(node);
    VSC_UL_ITERATOR iter;
    VSC_IS_DepDagEdge* edge;
    gctBOOL allMarked = gcvTRUE;

    gcmASSERT(nodes_bv);
    if(VSC_IS_DepDagNode_HasFlag(node, exclueNodeflag))
    {
        return gcvFALSE;
    }

    if(excluded_nodes_bv && vscBV_TestBit(nodes_bv, VSC_IS_DepDagNode_GetID(node)))
    {
        return gcvFALSE;
    }

    vscBV_SetBit(nodes_bv, VSC_IS_DepDagNode_GetID(node));
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        if(!VSC_IS_DepDagEdge_HasFlag(edge, exclueEdgeflag))
        {
            VSC_IS_DepDagNode* edge_to = VSC_IS_DepDagEdge_GetToNode(edge);

            if(edges_bv)
            {
                vscBV_SetBit(edges_bv, VSC_IS_DepDagEdge_GetID(edge));
            }
            if(!vscBV_TestBit(nodes_bv, VSC_IS_DepDagNode_GetID(edge_to)))
            {
                gctBOOL result = _VSC_IS_DepDagNode_MarkSubTree(edge_to, succ, exclueNodeflag, exclueEdgeflag, excluded_nodes_bv, nodes_bv, edges_bv);

                allMarked = allMarked && result;
            }
        }
        else
        {
            allMarked = gcvFALSE;
        }
    }

    return allMarked;
}

static void VSC_IS_DepDagNode_PropagateKillPriority(
    IN VSC_IS_DepDagNode* node,
    IN gctUINT priority,
    IN OUT VSC_BIT_VECTOR* nodes_bv
    )
{
    VSC_ADJACENT_LIST* edge_list = VSC_IS_DepDagNode_GetPredEdgeList(node);
    VSC_UL_ITERATOR iter;
    VSC_IS_DepDagEdge* edge;

    vscBV_SetBit(nodes_bv, VSC_IS_DepDagNode_GetID(node));
    if(VSC_IS_DepDagNode_GetKillPriority(node) < priority)
    {
        VSC_IS_DepDagNode_SetKillPriority(node, priority);
    }
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        VSC_IS_DepDagNode* edge_to = VSC_IS_DepDagEdge_GetToNode(edge);

        if(!vscBV_TestBit(nodes_bv, VSC_IS_DepDagNode_GetID(edge_to)))
        {
            VSC_IS_DepDagNode_PropagateKillPriority(edge_to, priority, nodes_bv);
        }
    }
}

/* dependence DAG utilities */
static void _VSC_IS_DepDag_Init(
    IN VSC_IS_DepDag* dag,
    IN VSC_MM* mm
    )
{
    gctUINT i;

    vscDG_Initialize(VSC_IS_DepDag_GetDGraph(dag), mm, 2, 4, sizeof(VSC_IS_DepDagEdge));
    vscHTBL_Initialize(VSC_IS_DepDag_GetInst2Node(dag), mm, vscHFUNC_Default, vscHKCMP_Default, 512);
    vscSRARR_Initialize(VSC_IS_DepDag_GetKillNodesArray(dag), mm, 4, sizeof(VSC_IS_DepDagNode*), gcvNULL);

    for(i = 0; i < VSC_IS_DEPDAG_NODES_BV_COUNT; i++)
    {
        VSC_IS_DepDag_SetNodes_BV(dag, i, gcvNULL);
        VSC_IS_DepDag_ResetUsingNodesBVs(dag, i);
    }

    for(i = 0; i < VSC_IS_DEPDAG_EDGES_BV_COUNT; i++)
    {
        VSC_IS_DepDag_SetEdges_BV(dag, i, gcvNULL);
        VSC_IS_DepDag_ResetUsingEdgesBVs(dag, i);
    }

    VSC_IS_DepDag_SetMM(dag, mm);
}

static VSC_IS_DepDagNode* _VSC_IS_DepDag_NewNode(
    IN OUT VSC_IS_DepDag* dag,
    IN VIR_Instruction* inst)
{
    VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)vscMM_Alloc(VSC_IS_DepDag_GetMM(dag), sizeof(VSC_IS_DepDagNode));
    _VSC_IS_DepDagNode_Init(node, inst);
    if(inst && VIR_Inst_GetOpcode(inst) == VIR_OP_KILL)
    {
        vscSRARR_AddElement(VSC_IS_DepDag_GetKillNodesArray(dag), &node);
    }
    return node;
}

static void _VSC_IS_DepDag_AddNode(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* node)
{
    vscDG_AddNode(VSC_IS_DepDag_GetDGraph(dag), VSC_IS_DepDagNode_GetNode(node));
}

static VSC_IS_DepDagEdge* _VSC_IS_DepDag_GetEdge(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* from,
    IN VSC_IS_DepDagNode* to)
{
    return (VSC_IS_DepDagEdge*)vscDG_GetEdge(VSC_IS_DepDag_GetDGraph(dag),
        VSC_IS_DepDagNode_GetNode(from), VSC_IS_DepDagNode_GetNode(to));
}

static VSC_IS_DepDagEdge* _VSC_IS_DepDag_AddEdge(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* from,
    IN VSC_IS_DepDagNode* to)
{
    gctBOOL is_new_edge = gcvFALSE;
    VSC_IS_DepDagEdge* edge = (VSC_IS_DepDagEdge*)vscDG_AddEdge(VSC_IS_DepDag_GetDGraph(dag),
        VSC_IS_DepDagNode_GetNode(from), VSC_IS_DepDagNode_GetNode(to), &is_new_edge);

    gcmASSERT(VSC_IS_DepDagNode_GetID(from) != VSC_IS_DepDagNode_GetID(to));
    if(is_new_edge)
    {
        _VSC_IS_DepDagEdge_Init(edge);
    }
    return edge;
}


static VSC_IS_DepDagEdge* _VSC_IS_DepDag_ReplaceEdgeToNode(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* from,
    IN VSC_IS_DepDagNode* to,
    IN VSC_IS_DepDagNode* newTo)
{
    return (VSC_IS_DepDagEdge*)vscDG_ReplaceEdgeToNode(VSC_IS_DepDag_GetDGraph(dag),
                                                       VSC_IS_DepDagNode_GetNode(from),
                                                       VSC_IS_DepDagNode_GetNode(to),
                                                       VSC_IS_DepDagNode_GetNode(newTo));
}

static void _VSC_IS_DepDag_RemoveEdge(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* from,
    IN VSC_IS_DepDagNode* to)
{
    vscDG_RemoveEdge(VSC_IS_DepDag_GetDGraph(dag),
                     VSC_IS_DepDagNode_GetNode(from),
                     VSC_IS_DepDagNode_GetNode(to));
}

static VSC_BIT_VECTOR* _VSC_IS_DepDag_RentANodesBV(
    IN VSC_IS_DepDag* dag
    )
{
    gctUINT i;

    for(i = 0; i < VSC_IS_DEPDAG_NODES_BV_COUNT; i++)
    {
        if(VSC_IS_DepDag_GetUsingNodesBVs(dag, i) == 0)
        {
            VSC_BIT_VECTOR* nodes_bv = VSC_IS_DepDag_GetNodes_BV(dag, i);
            if(nodes_bv == gcvNULL)
            {
                nodes_bv = vscBV_Create(VSC_IS_DepDag_GetMM(dag), VSC_IS_DepDag_GetGeneratedNodeCount(dag));
                VSC_IS_DepDag_SetNodes_BV(dag, i, nodes_bv);
            }
            VSC_IS_DepDag_SetUsingNodesBVs(dag, i);
            return nodes_bv;
        }
    }
    gcmASSERT(i != VSC_IS_DEPDAG_NODES_BV_COUNT);

    return gcvNULL;
}

static void _VSC_IS_DepDag_ReturnANodesBV(
    IN VSC_IS_DepDag* dag,
    IN VSC_BIT_VECTOR* nodes_bv
    )
{
    gctUINT i;

    vscBV_ClearAll(nodes_bv);
    for(i = 0; i < VSC_IS_DEPDAG_NODES_BV_COUNT; i++)
    {
        if(VSC_IS_DepDag_GetNodes_BV(dag, i) == nodes_bv)
        {
            gcmASSERT(VSC_IS_DepDag_GetUsingNodesBVs(dag, i));
            VSC_IS_DepDag_ResetUsingNodesBVs(dag, i);
            break;
        }
    }
    gcmASSERT(i != VSC_IS_DEPDAG_NODES_BV_COUNT);
}

static VSC_BIT_VECTOR* _VSC_IS_DepDag_RentAEdgesBV(
    IN VSC_IS_DepDag* dag
    )
{
    gctUINT i;

    for(i = 0; i < VSC_IS_DEPDAG_EDGES_BV_COUNT; i++)
    {
        if(VSC_IS_DepDag_GetUsingEdgesBVs(dag, i) == 0)
        {
            VSC_BIT_VECTOR* edges_bv = VSC_IS_DepDag_GetEdges_BV(dag, i);
            if(edges_bv == gcvNULL)
            {
                edges_bv = vscBV_Create(VSC_IS_DepDag_GetMM(dag), VSC_IS_DepDag_GetGeneratedEdgeCount(dag) * 8);
                VSC_IS_DepDag_SetEdges_BV(dag, i, edges_bv);
            }
            VSC_IS_DepDag_SetUsingEdgesBVs(dag, i);
            return edges_bv;
        }
    }
    gcmASSERT(i != VSC_IS_DEPDAG_EDGES_BV_COUNT);

    return gcvNULL;
}

static void _VSC_IS_DepDag_ReturnAEdgesBV(
    IN VSC_IS_DepDag* dag,
    IN VSC_BIT_VECTOR* edges_bv
    )
{
    gctUINT i;

    vscBV_ClearAll(edges_bv);
    for(i = 0; i < VSC_IS_DEPDAG_EDGES_BV_COUNT; i++)
    {
        if(VSC_IS_DepDag_GetEdges_BV(dag, i) == edges_bv)
        {
            gcmASSERT(VSC_IS_DepDag_GetUsingEdgesBVs(dag, i));
            VSC_IS_DepDag_ResetUsingEdgesBVs(dag, i);
            break;
        }
    }
    gcmASSERT(i != VSC_IS_DEPDAG_EDGES_BV_COUNT);
}

static gctUINT _VSC_IS_DepDag_GetSubTreeNodesCount(
    IN OUT VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* node,
    IN gctBOOL succ
    )
{
    gctUINT result;
    VSC_BIT_VECTOR* nodes_bv = _VSC_IS_DepDag_RentANodesBV(dag);

    _VSC_IS_DepDagNode_MarkSubTree(node, succ, VSC_IS_DEPDAGNODE_FLAG_NONE, VSC_IS_DEPDAGEDGE_FLAG_NONE, gcvNULL, nodes_bv, gcvNULL);
    result = vscBV_CountBits(nodes_bv);
    _VSC_IS_DepDag_ReturnANodesBV(dag, nodes_bv);

    return result;
}

static void _VSC_IS_DepDag_SetKillPriority(
    IN OUT VSC_IS_DepDag* dag
    )
{
    VSC_SIMPLE_RESIZABLE_ARRAY* kill_nodes_array = VSC_IS_DepDag_GetKillNodesArray(dag);
    gctUINT i;

    for(i = 0; i < vscSRARR_GetElementCount(kill_nodes_array); i++)
    {
        VSC_IS_DepDagNode* kill_node = *(VSC_IS_DepDagNode**)vscSRARR_GetElement(kill_nodes_array, i);
        gctUINT priority = gcvMAXUINT32 - _VSC_IS_DepDag_GetSubTreeNodesCount(dag, kill_node, gcvFALSE);
        VSC_BIT_VECTOR* nodes_bv = _VSC_IS_DepDag_RentANodesBV(dag);

        VSC_IS_DepDagNode_PropagateKillPriority(kill_node, priority, nodes_bv);
        _VSC_IS_DepDag_ReturnANodesBV(dag, nodes_bv);
    }
}


static void _VSC_IS_DepDag_Final(
    IN VSC_IS_DepDag* dag
    )
{
    gctUINT i;

    vscDG_Finalize(VSC_IS_DepDag_GetDGraph(dag));
    vscHTBL_Finalize(VSC_IS_DepDag_GetInst2Node(dag));
    vscSRARR_Finalize(VSC_IS_DepDag_GetKillNodesArray(dag));

    for(i = 0; i < VSC_IS_DEPDAG_NODES_BV_COUNT; i++)
    {
        gcmASSERT(VSC_IS_DepDag_GetUsingNodesBVs(dag, i) == 0);
        if(VSC_IS_DepDag_GetNodes_BV(dag, i))
        {
            vscBV_Finalize(VSC_IS_DepDag_GetNodes_BV(dag, i));
            VSC_IS_DepDag_SetNodes_BV(dag, i, gcvNULL);
        }
    }

    for(i = 0; i < VSC_IS_DEPDAG_EDGES_BV_COUNT; i++)
    {
        gcmASSERT(VSC_IS_DepDag_GetUsingEdgesBVs(dag, i) == 0);
        if(VSC_IS_DepDag_GetEdges_BV(dag, i))
        {
            vscBV_Finalize(VSC_IS_DepDag_GetEdges_BV(dag, i));
            VSC_IS_DepDag_SetEdges_BV(dag, i, gcvNULL);
        }
    }
}

static gctUINT32 _VSC_IS_EstimateRegisterUsage(
    IN  VIR_Shader* shader
    )
{
    gctUINT32 i;
    VIR_AttributeIdList* attrs = VIR_Shader_GetAttributes(shader);
    VIR_OutputIdList* outputs = VIR_Shader_GetOutputs(shader);
    gctUINT32 reg_count = 0;
    gctUINT32 inst_count = VIR_Shader_GetTotalInstructionCount(shader);

    for(i = 0; i < attrs->count; i++)
    {
        VIR_Id id = attrs->ids[i];
        VIR_Symbol* sym = VIR_Shader_GetSymFromId(shader, id);
        reg_count += VIR_Symbol_GetVirIoRegCount(shader, sym);
    }

    for(i = 0; i < outputs->count; i++)
    {
        VIR_Id id = outputs->ids[i];
        VIR_Symbol* sym = VIR_Shader_GetSymFromId(shader, id);
        reg_count += VIR_Symbol_GetVirIoRegCount(shader, sym);
    }

    if(shader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        reg_count++;
    }

    if(inst_count > 1000)
    {
        reg_count += 14;
    }
    else if(inst_count > 600)
    {
        reg_count += 12;
    }
    else if(inst_count > 300)
    {
        reg_count += 10;
    }
    else if(inst_count > 120)
    {
        reg_count += 6;
    }
    else if(inst_count > 70)
    {
        reg_count += 5;
    }
    else if(inst_count > 30)
    {
        reg_count += 3;
    }
    else if(inst_count > 10)
    {
        reg_count += 2;
    }

    if (reg_count == 0)
        reg_count = 1;

    return reg_count;
}

static void _VSC_IS_InstSched_InitBubble(
    IN OUT VSC_IS_InstSched* is
    )
{
    VIR_Shader* shader = VSC_IS_InstSched_GetShader(is);
#define HW_CONFIG
#ifdef HW_CONFIG
    VSC_HW_CONFIG* hw_cfg = VSC_IS_InstSched_GetHwCfg(is);
#endif
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);
    gctUINT32 allocatedRegisterCount;
    gctUINT32 groupCount;
    VSC_HW_UARCH_CAPS* hw_uarch_caps = VSC_IS_InstSched_GetHwUArchCaps(is);
#ifndef HW_CONFIG
    gctUINT32 requiredGroupCount;
    gctUINT32 countedGroup;
#endif
    gctUINT32 texld_latency = hw_uarch_caps->texldCycles;
    gctUINT32 memld_latency = hw_uarch_caps->memLdCycles;
    gctUINT32 memst_latency = hw_uarch_caps->memStCycles;
    gctUINT32 cacheld_latency = hw_uarch_caps->cacheLdCycles;
    gctUINT32 cachest_latency = hw_uarch_caps->cacheStCycles;
    gctUINT32 texld_bubble, memld_bubble, memst_bubble, cache_ld_bubble, cache_st_bubble, texld_bandwidth, memld_bandwidth;

    if(VSC_OPTN_ISOptions_GetRegCount(options))
    {
        allocatedRegisterCount = VSC_OPTN_ISOptions_GetRegCount(options);
    }
    else if(!VIR_Shader_isRegAllocated(shader))
    {
        allocatedRegisterCount = _VSC_IS_EstimateRegisterUsage(shader);
    }
    else
    {
        allocatedRegisterCount = VIR_Shader_GetRegWatermark(shader);
    }
    if(allocatedRegisterCount > 14)
    {
        allocatedRegisterCount = 14;
    }
    VSC_IS_InstSched_SetRegisterCount(is, allocatedRegisterCount);

    if(VSC_OPTN_ISOptions_GetTexldCycles(options))
    {
        texld_latency = VSC_OPTN_ISOptions_GetTexldCycles(options);
    }
    if(VSC_OPTN_ISOptions_GetMemldCycles(options))
    {
        memld_latency = VSC_OPTN_ISOptions_GetMemldCycles(options);
    }
    if(VSC_OPTN_ISOptions_GetMemstCycles(options))
    {
        memst_latency = VSC_OPTN_ISOptions_GetMemstCycles(options);
    }
    if(VSC_OPTN_ISOptions_GetCacheldCycles(options))
    {
        cacheld_latency = VSC_OPTN_ISOptions_GetCacheldCycles(options);
    }
    if(VSC_OPTN_ISOptions_GetCachestCycles(options))
    {
        cachest_latency = VSC_OPTN_ISOptions_GetCachestCycles(options);
    }
#ifndef HW_CONFIG
    groupCount = (512 / hw_uarch_caps->hwThreadNumPerHwGrpPerCore) / allocatedRegisterCount;
    requiredGroupCount = hw_uarch_caps->hwShPipelineCycles / hw_uarch_caps->hwThreadNumPerHwGrpPerCore;
    countedGroup = vscMAX(groupCount - 1, requiredGroupCount);

    texld_bubble = hw_uarch_caps->texldCycles / (countedGroup * hw_uarch_caps->hwShGrpDispatchCycles) -
                   (hw_uarch_caps->texldCycles % (countedGroup * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    memld_bubble = hw_uarch_caps->ldCycles / (countedGroup * hw_uarch_caps->hwShGrpDispatchCycles) -
                   (hw_uarch_caps->ldCycles % (countedGroup * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    texld_bandwidth = hw_uarch_caps->rqPortCountOfShCore2TUFifo / hw_uarch_caps->rqPortCountOfTUFifo2TU - 1;
    memld_bandwidth = hw_uarch_caps->rqPortCountOfShCore2L1Fifo / hw_uarch_caps->rqPortCountOfL1Fifo2L1 - 1;
    VSC_IS_InstSched_SetTexldDepBubble(is, texld_bubble);
    VSC_IS_InstSched_SetMemldDepBubble(is, memld_bubble);
    VSC_IS_InstSched_SetTexldInterfaceBubble(is, texld_bandwidth);
    VSC_IS_InstSched_SetMemldInterfaceBubble(is, memld_bandwidth);
#else
    groupCount = (hw_cfg->maxGPRCountPerCore / hw_uarch_caps->hwThreadNumPerHwGrpPerCore) / allocatedRegisterCount - 1;
    if (groupCount <= 0)
    {
        groupCount = 1;
    }
    texld_bubble = texld_latency / (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) -
                   (texld_latency % (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    /* reduce the value of memld_bubble to 6 if hw feature supportPerCompDepForLS is false,
     * which may help to avoid too many load instructions are sheduled together and
     * decrease the register allocation pressure */
    memld_bubble = (hw_cfg->hwFeatureFlags.supportPerCompDepForLS)  ? (memld_latency / (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) -
                   (memld_latency % (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1)) : 6;
    memst_bubble = memst_latency / (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) -
                   (memst_latency % (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    cache_ld_bubble = cacheld_latency / (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) -
                      (cacheld_latency % (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    cache_st_bubble = cachest_latency / (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) -
                      (cachest_latency % (groupCount * hw_uarch_caps->hwShGrpDispatchCycles) ? 0 : 1);
    texld_bandwidth = hw_cfg->maxCoreCount / hw_uarch_caps->texldPerCycle - 1;
    memld_bandwidth = hw_cfg->maxCoreCount / hw_uarch_caps->texldPerCycle - 1;
    VSC_IS_InstSched_SetTexldDepBubble(is, texld_bubble);
    VSC_IS_InstSched_SetMemldDepBubble(is, memld_bubble);
    VSC_IS_InstSched_SetMemstDepBubble(is, memst_bubble);
    VSC_IS_InstSched_SetCacheldDepBubble(is, cache_ld_bubble);
    VSC_IS_InstSched_SetCachestDepBubble(is, cache_st_bubble);
    VSC_IS_InstSched_SetTexldInterfaceBubble(is, texld_bandwidth);
    VSC_IS_InstSched_SetMemldInterfaceBubble(is, memld_bandwidth);
#endif
}

static void _VSC_IS_InstSched_Init(
    IN OUT VSC_IS_InstSched* is,
    IN VIR_Shader* shader,
    IN VSC_HW_CONFIG* hwCfg,
    IN VIR_DEF_USAGE_INFO* du_info,
    IN VSC_OPTN_ISOptions* options,
    IN VIR_Dumper* dumper,
    IN VSC_MM* pMM
    )
{
    VSC_HW_UARCH_CAPS   hwUArchCaps;

    vscQueryHwMicroArchCaps(hwCfg, &hwUArchCaps);

    VSC_IS_InstSched_SetShader(is, shader);
    VSC_IS_InstSched_SetHwCfg(is, hwCfg);
    VSC_IS_InstSched_SetHwUArchCaps(is, &hwUArchCaps);
    VSC_IS_InstSched_SetCurrBB(is, gcvNULL);
    VSC_IS_InstSched_SetDUInfo(is, du_info);
    VSC_IS_InstSched_SetCurrDepDag(is, gcvNULL);
    VSC_IS_InstSched_SetOptions(is, options);
    VSC_IS_InstSched_SetDumper(is, dumper);
    _VSC_IS_InstSched_InitBubble(is);
    VSC_IS_InstSched_SetMM(is, pMM);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_INITIALIZATION))
    {
        VIR_LOG(dumper, "%s\nInstruction Scheduling Initialization\n%s\n", VSC_TRACE_STAR_LINE, VSC_TRACE_STAR_LINE);
        VIR_LOG(dumper, "register count: %d\n", VSC_IS_InstSched_GetRegisterCount(is));
        VIR_LOG(dumper, "texld dep bubble: %d\nmemld dep bubble: %d\ncacheld dep bubble: %d\n", VSC_IS_InstSched_GetTexldDepBubble(is), VSC_IS_InstSched_GetMemldDepBubble(is), VSC_IS_InstSched_GetCacheldDepBubble(is));
        VIR_LOG(dumper, "texld interface bubble: %d\nmemld interface bubble: %d\n", VSC_IS_InstSched_GetTexldInterfaceBubble(is), VSC_IS_InstSched_GetMemldInterfaceBubble(is));
        VIR_LOG_FLUSH(dumper);
    }
}

/* get the bb start/end/length regardless of the starting labels and ending jmp/ret */
static gctUINT32 _VSC_IS_GetBBEssence(
    IN VIR_BB* bb,
    OUT VIR_Instruction** ess_start,
    OUT VIR_Instruction** ess_end)
{
    gctUINT32 len = BB_GET_LENGTH(bb);
    VIR_Instruction* start = gcvNULL;
    VIR_Instruction* end = gcvNULL;
    if(len != 0)
    {
        start = BB_GET_START_INST(bb);
        end = BB_GET_END_INST(bb);

        while(start && VIR_OPCODE_isBBPrefix(VIR_Inst_GetOpcode(start)))
        {
            start = VIR_Inst_GetNext(start);
            --len;
        }

        if(VIR_OPCODE_isBBSuffix(VIR_Inst_GetOpcode(end)))
        {
            end = VIR_Inst_GetPrev(end);
            --len;
            /* if the bb ending jmp is using its prev LDARR, then
               this LDARR should not be counted too */
            if(end && VIR_Inst_GetOpcode(end) == VIR_OP_LDARR)
            {
                end = VIR_Inst_GetPrev(end);
                --len;
            }
        }
    }

    if(ess_start)
    {
        *ess_start = len ? start : gcvNULL;
    }
    if(ess_end)
    {
        *ess_end = len ? end : gcvNULL;
    }

    return len;
}

static void _VSC_IS_InstSched_NewDepDag(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_IS_DepDag* dag = (VSC_IS_DepDag*)vscMM_Alloc(VSC_IS_InstSched_GetMM(is), sizeof(VSC_IS_DepDag));

    _VSC_IS_DepDag_Init(dag, VSC_IS_InstSched_GetMM(is));
    VSC_IS_InstSched_SetCurrDepDag(is, dag);
}

static void _VSC_IS_InstSched_DeleteDepDag(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    if(dag)
    {
        _VSC_IS_DepDag_Final(dag);
        vscMM_Free(VSC_IS_InstSched_GetMM(is), dag);
        VSC_IS_InstSched_SetCurrDepDag(is, gcvNULL);
    }
}

static void _VSC_IS_InstSched_Final(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_IS_InstSched_SetShader(is, gcvNULL);
    _VSC_IS_InstSched_DeleteDepDag(is);
    VSC_IS_InstSched_SetOptions(is, gcvNULL);
    VSC_IS_InstSched_SetDumper(is, gcvNULL);
}

static void _VSC_IS_DumpInstSet(
    IN VSC_HASH_TABLE* inst_set,
    IN VIR_Dumper* dumper
    );

static gctBOOL _VSC_IS_OperandsOverlapping(
    IN VIR_Instruction *    Inst1,
    IN VIR_Operand*         opr1,
    IN VIR_Instruction *    Inst2,
    IN VIR_Operand*         opr2
    )
{
    if(VIR_Operand_GetOpKind(opr1) == VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(opr2) == VIR_OPND_SYMBOL)
    {
        VIR_OperandInfo opr1_info;
        VIR_OperandInfo opr2_info;
        VIR_Enable opr1_enable;
        VIR_Enable opr2_enable;

        VIR_Operand_GetOperandInfo(Inst1, opr1, &opr1_info);
        if(VIR_Id_isInvalid(opr1_info.u1.virRegInfo.virReg))
        {
            return gcvFALSE;
        }
        VIR_Operand_GetOperandInfo(Inst2, opr2, &opr2_info);
        if(VIR_Id_isInvalid(opr2_info.u1.virRegInfo.virReg))
        {
            return gcvFALSE;
        }
        if(VIR_Operand_isLvalue(opr1))
        {
            opr1_enable = VIR_Operand_GetEnable(opr1);
        }
        else
        {
            opr1_enable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opr1));
        }
        if(VIR_Operand_isLvalue(opr2))
        {
            opr2_enable = VIR_Operand_GetEnable(opr2);
        }
        else
        {
            opr2_enable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opr2));
        }

        if(((opr1_info.u1.virRegInfo.virReg >= opr2_info.u1.virRegInfo.startVirReg && opr1_info.u1.virRegInfo.virReg < opr2_info.u1.virRegInfo.startVirReg + opr2_info.u1.virRegInfo.virRegCount) ||
                (opr2_info.u1.virRegInfo.virReg >= opr1_info.u1.virRegInfo.startVirReg && opr2_info.u1.virRegInfo.virReg < opr1_info.u1.virRegInfo.startVirReg + opr1_info.u1.virRegInfo.virRegCount)) &&
               (opr1_enable & opr2_enable))
        {
            return gcvTRUE;
        }
        if(VIR_Operand_isLvalue(opr1) && !VIR_Operand_GetIsConstIndexing(opr2) && VIR_Operand_GetRelAddrMode(opr2))
        {
            VIR_Id relAddrIndex = VIR_Operand_GetRelIndexing(opr2);
            VIR_Enable enable = (VIR_Enable)(1 << (VIR_Operand_GetRelAddrMode(opr2) - 1));

            if((relAddrIndex >= opr1_info.u1.virRegInfo.startVirReg && relAddrIndex < opr1_info.u1.virRegInfo.startVirReg + opr1_info.u1.virRegInfo.virRegCount) &&
               (opr1_enable & enable))
            {
                return gcvTRUE;
            }
        }
        if(VIR_Operand_isLvalue(opr2) && !VIR_Operand_GetIsConstIndexing(opr1) && VIR_Operand_GetRelAddrMode(opr1))
        {
            VIR_Id relAddrIndex = VIR_Operand_GetRelIndexing(opr1);
            VIR_Enable enable = (VIR_Enable)(1 << (VIR_Operand_GetRelAddrMode(opr1) - 1));

            if((relAddrIndex >= opr2_info.u1.virRegInfo.startVirReg && relAddrIndex < opr2_info.u1.virRegInfo.startVirReg + opr2_info.u1.virRegInfo.virRegCount) &&
               (opr2_enable & enable))
            {
                return gcvTRUE;
            }
        }
    }
    /*else if(VIR_Operand_GetOpKind(opr1) == VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(opr2) == VIR_OPND_SYMBOL)*/
    return gcvFALSE;
}

/* return the conflict types of two instruction */
static gctUINT32 _VSC_IS_InstConflict(
    IN VIR_Shader* shader,
    IN VIR_Instruction* inst0,
    IN VIR_Instruction* inst1,
    IN gctUINT32* excludeCTs)
{
    gctUINT32 conflict_types = 0;
    VIR_OpCode opc0, opc1;
    VIR_Operand *dest0, *dest1;
    gctUINT32 i, j;

    if(VIR_Inst_GetOpcode(inst1) == VIR_OP_BARRIER)
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_BARRIER);
        if(VIR_Inst_GetOpcode(inst0) == VIR_OP_BARRIER)
        {
            VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_BARRIER);
        }
        return conflict_types;
    }
    if(VIR_Inst_GetOpcode(inst0) == VIR_OP_BARRIER)
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_BARRIER);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_BARRIER);
        return conflict_types;
    }

    if(VIR_Inst_GetOpcode(inst1) == VIR_OP_EMIT0)
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_EMIT);
        if(VIR_Inst_GetOpcode(inst0) == VIR_OP_EMIT0)
        {
            VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_EMIT);
        }
        return conflict_types;
    }
    if(VIR_Inst_GetOpcode(inst0) == VIR_OP_EMIT0)
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_EMIT);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_EMIT);
        return conflict_types;
    }

    if(VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst0)) && VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst1)))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_ATOMIC);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_ATOMIC);
    }

    opc0 = VIR_Inst_GetOpcode(inst0);
    opc1 = VIR_Inst_GetOpcode(inst1);
    dest0 = VIR_Inst_GetDest(inst0);
    dest1 = VIR_Inst_GetDest(inst1);

    if(dest0 && !VIR_OPCODE_isMemSt(opc0))
    {
        VIR_Operand* src1;
        for(i = 0; i < VIR_Inst_GetSrcNum(inst1); i++)
        {
            src1 = VIR_Inst_GetSource(inst1, i);

            if (VIR_Operand_GetOpKind(src1) == VIR_OPND_TEXLDPARM)
            {
                for (j = 0; j < VIR_TEXLDMODIFIER_COUNT; ++j)
                {
                    if (VIR_Operand_GetTexldModifier(src1, j) != gcvNULL)
                    {
                        VIR_Operand* srcTexMod = VIR_Operand_GetTexldModifier(src1, j);

                        if (_VSC_IS_OperandsOverlapping(inst0, dest0, inst1, srcTexMod))
                        {
                            if (VIR_OPCODE_isTexLd(opc0))
                            {
                                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_TLRS_RU);
                            }
                            else if (VIR_OPCODE_isMemLd(opc0) || VIR_OPCODE_isImgLd(opc0))
                            {
                                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_MLRS_RU);
                            }
                            else if (VIR_OPCODE_isAttrLd(opc0))
                            {
                                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_CLRS_RU);
                            }
                            else
                            {
                                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RS_RU);
                                if(VIR_Inst_GetOpcode(inst0) == VIR_OP_MOVA)
                                {
                                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_LOOSE_BINDING_MOVA);
                                }
                            }
                        }
                    }
                }
            }
            else if(_VSC_IS_OperandsOverlapping(inst0, dest0, inst1, src1))
            {
                if(VIR_OPCODE_isTexLd(opc0))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_TLRS_RU);
                }
                else if(VIR_OPCODE_isMemLd(opc0) || VIR_OPCODE_isImgLd(opc0))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_MLRS_RU);
                }
                else if(VIR_OPCODE_isAttrLd(opc0))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_CLRS_RU);
                }
                else
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RS_RU);
                    if(VIR_Inst_GetOpcode(inst0) == VIR_OP_MOVA)
                    {
                        VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_LOOSE_BINDING_MOVA);
                    }
                }
            }
        }
        if(dest1 && _VSC_IS_OperandsOverlapping(inst0, dest0, inst1, dest1))
        {
            if(VIR_OPCODE_isTexLd(opc0))
            {
                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_TLRS_RS);
            }
            else if(VIR_OPCODE_isMemLd(opc0) || VIR_OPCODE_isImgLd(opc0))
            {
                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_MLRS_RS);
            }
            else if(VIR_OPCODE_isAttrLd(opc0))
            {
                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_CLRS_RS);
            }
            else
            {
                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RS_RS);
            }
        }
    }
    if(dest1 && !VIR_OPCODE_isMemSt(opc1))
    {
        VIR_Operand* src0;
        for(i = 0; i < VIR_Inst_GetSrcNum(inst0); i++)
        {
            src0 = VIR_Inst_GetSource(inst0, i);
            if(VIR_OPCODE_isTexLd(opc1))
            {
                VIR_Symbol *sym = VIR_Operand_GetUnderlyingSymbol(src0);
                if (sym && VIR_Symbol_isOutParam(sym))
                {
                    VSC_IS_ConflictType_Set(conflict_types,
                                            VSC_IS_ConflictType_USE_RETURNVALUE);
                }
            }

            if (VIR_Operand_GetOpKind(src0) == VIR_OPND_TEXLDPARM)
            {
                for (j = 0; j < VIR_TEXLDMODIFIER_COUNT; ++j)
                {
                    if (VIR_Operand_GetTexldModifier(src0, j) != gcvNULL)
                    {
                        VIR_Operand* srcTexMod = VIR_Operand_GetTexldModifier(src0, j);

                        if (_VSC_IS_OperandsOverlapping(inst0, srcTexMod, inst1, dest1))
                        {
                            VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RU_RS);
                        }
                    }
                }
            }
            else if(_VSC_IS_OperandsOverlapping(inst0, src0, inst1, dest1))
            {
                VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RU_RS);
            }
        }
    }

    /* without alias, have to assume memory/cache dependence */
    if(VIR_OPCODE_isAtom(opc0) && VIR_OPCODE_isImgSt(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_ML_MS);
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_ML_MS);
    }
    else if(VIR_OPCODE_isAtom(opc0) && VIR_OPCODE_isImgLd(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_ML);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_ML);
    }
    else if(VIR_OPCODE_isImgSt(opc0) && VIR_OPCODE_isAtom(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_ML);
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_ML);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_ML_MS);
    }
    else if(VIR_OPCODE_isImgLd(opc0) && VIR_OPCODE_isAtom(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_ML_MS);
    }
    else if((VIR_OPCODE_isLocalMemLd(opc0) && VIR_OPCODE_isLocalMemSt(opc1)) ||
       (VIR_OPCODE_isSpecialMemLd(opc0) && VIR_OPCODE_isSpecialMemSt(opc1)) ||
       (VIR_OPCODE_isGlobalMemLd(opc0) && VIR_OPCODE_isGlobalMemSt(opc1)) ||
       (opc0 == VIR_OP_IMG_LOAD && opc1 == VIR_OP_IMG_STORE) ||
       (opc0 == VIR_OP_IMG_LOAD_3D && opc1 == VIR_OP_IMG_STORE_3D) ||
       (opc0 == VIR_OP_VX_IMG_LOAD && opc1 == VIR_OP_VX_IMG_STORE) ||
       (opc0 == VIR_OP_VX_IMG_LOAD_3D && opc1 == VIR_OP_VX_IMG_STORE_3D))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_ML_MS);
    }
    else if((VIR_OPCODE_isLocalMemSt(opc0) && VIR_OPCODE_isLocalMemLd(opc1)) ||
            (VIR_OPCODE_isSpecialMemSt(opc0) && VIR_OPCODE_isSpecialMemLd(opc1)) ||
            (VIR_OPCODE_isGlobalMemSt(opc0) && VIR_OPCODE_isGlobalMemLd(opc1)) ||
            (opc0 == VIR_OP_IMG_STORE && opc1 == VIR_OP_IMG_LOAD) ||
            (opc0 == VIR_OP_IMG_STORE_3D && opc1 == VIR_OP_IMG_LOAD_3D) ||
            (opc0 == VIR_OP_VX_IMG_STORE && opc1 == VIR_OP_VX_IMG_LOAD) ||
            (opc0 == VIR_OP_VX_IMG_STORE_3D && opc1 == VIR_OP_VX_IMG_LOAD_3D) ||
            /* kill should not be scheduled before inst with store flag */
            (VIR_OPCODE_Stores(opc0) && opc1 == VIR_OP_KILL))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_ML);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_ML);
    }
    else if((VIR_OPCODE_isLocalMemSt(opc0) && VIR_OPCODE_isLocalMemSt(opc1)) ||
            (VIR_OPCODE_isSpecialMemSt(opc0) && VIR_OPCODE_isSpecialMemSt(opc1)) ||
            (VIR_OPCODE_isGlobalMemSt(opc0) && VIR_OPCODE_isGlobalMemSt(opc1)) ||
            (opc0 == VIR_OP_IMG_STORE && opc1 == VIR_OP_IMG_STORE) ||
            (opc0 == VIR_OP_IMG_STORE_3D && opc1 == VIR_OP_IMG_STORE_3D) ||
            (opc0 == VIR_OP_VX_IMG_STORE && opc1 == VIR_OP_VX_IMG_STORE) ||
            (opc0 == VIR_OP_VX_IMG_STORE_3D && opc1 == VIR_OP_VX_IMG_STORE_3D))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_MS_MS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_ML_MS);
    }
    if(VIR_OPCODE_isAttrLd(opc0) && VIR_OPCODE_isAttrSt(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_CL_CS);
    }
    else if(VIR_OPCODE_isAttrSt(opc0) && VIR_OPCODE_isAttrLd(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_CS_CL);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_CS_CL);
    }
    else if(VIR_OPCODE_isAttrSt(opc0) && VIR_OPCODE_isAttrSt(opc1))
    {
        VSC_IS_ConflictType_ExclusivelySet(conflict_types, *excludeCTs, VSC_IS_ConflictType_CS_CS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_CS_CS);
        VSC_IS_ConflictType_Set(*excludeCTs, VSC_IS_ConflictType_CL_CS);
    }
    return conflict_types;
}

static void _VSC_IS_BindContinuousNodes(
    IN OUT VSC_IS_DepDag* dag,
    IN VIR_Instruction* inst,
    IN gctUINT count
    )
{
    VSC_HASH_TABLE* inst2node = VSC_IS_DepDag_GetInst2Node(dag);
    VSC_IS_DepDagNode *iter_node = gcvNULL;
    VSC_IS_DepDagNode *next_node = gcvNULL;
    VIR_Instruction* iter_inst = inst;
    gctUINT i;

    gcmASSERT(count > 1);

    vscHTBL_DirectTestAndGet(inst2node, inst, (void**)&iter_node);
    VSC_IS_DepDagNode_AddFlag(iter_node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC);
    gcmASSERT(iter_node);
    for(i = 1; i < count; i++)
    {
        VSC_IS_DepDagEdge* edge;
        VIR_Instruction* next = VIR_Inst_GetNext(iter_inst);

        vscHTBL_DirectTestAndGet(inst2node, next, (void**)&next_node);
        gcmASSERT(next_node);
        VSC_IS_DepDagNode_AddFlag(next_node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_PRED);
        VSC_IS_DepDagNode_AddFlag(next_node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC);
        edge = _VSC_IS_DepDag_GetEdge(dag, iter_node, next_node);
        gcmASSERT(edge);
        VSC_IS_DepDagEdge_AddConflictType(edge, VSC_IS_ConflictType_CONTINUOUS_BINDING);

        iter_inst = VIR_Inst_GetNext(iter_inst);
        iter_node = next_node;
    }
    VSC_IS_DepDagNode_RemoveFlag(next_node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC);
}

static gctBOOL _VSC_IS_LooselyBindLdarr(
    IN OUT VSC_IS_DepDag* dag,
    IN VIR_Instruction* inst
    )
{
    VSC_HASH_TABLE* inst2node = VSC_IS_DepDag_GetInst2Node(dag);
    VSC_IS_DepDagNode *inst_node;
    VSC_ADJACENT_LIST* succ_edge_list;
    VSC_UL_ITERATOR iter0;
    VSC_IS_DepDagEdge* succ_edge0;
    gctBOOL set_dodging = gcvFALSE;

    vscHTBL_DirectTestAndGet(inst2node, inst, (void**)&inst_node);
    gcmASSERT(inst_node);


    succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(inst_node);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter0, succ_edge_list);
    for(succ_edge0 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter0);
        succ_edge0 != gcvNULL; succ_edge0 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter0))
    {
        if(VSC_IS_DepDagEdge_HasConflictType(succ_edge0, VSC_IS_ConflictType_RS_RU))
        {
            VSC_IS_DepDagEdge_AddConflictType(succ_edge0, VSC_IS_ConflictType_LOOSE_BINDING_LDARR);
        }

        if(VSC_IS_DepDagEdge_HasConflictType(succ_edge0, VSC_IS_ConflictType_RU_RS))
        {
            VSC_IS_DepDagNode* dep_node0 = VSC_IS_DepDagEdge_GetToNode(succ_edge0);
            VSC_UL_ITERATOR iter1;
            VSC_IS_DepDagEdge* succ_edge1;
            VSC_ADJACENT_LIST_ITERATOR_INIT(&iter1, succ_edge_list);
            for(succ_edge1 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter1);
                succ_edge1 != gcvNULL; succ_edge1 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter1))
            {
                if(succ_edge0 == succ_edge1)
                {
                    continue;
                }

                if(VSC_IS_DepDagEdge_HasConflictType(succ_edge1, VSC_IS_ConflictType_RS_RU))
                {
                    VSC_IS_DepDagNode* dep_node1 = VSC_IS_DepDagEdge_GetToNode(succ_edge1);

                    if(VSC_IS_DepDagNode_GetID(dep_node1) < VSC_IS_DepDagNode_GetID(dep_node0))
                    {
                        VSC_IS_DepDagEdge* edge = _VSC_IS_DepDag_AddEdge(dag, dep_node1, dep_node0);
                        VSC_IS_DepDagEdge_AddConflictType(edge, VSC_IS_ConflictType_DODGING);
                        set_dodging = gcvTRUE;
                    }
                }
            }
        }
    }

    return set_dodging;
}

static gctBOOL _VSC_IS_BindNodes(
    IN OUT VSC_IS_DepDag* dag,
    IN VIR_Instruction* inst,
    IN gctUINT* continuous_count
    )
{
    gcmASSERT(continuous_count);

    /* Bind continuous nodes, add CONTINUOUS_BINDING dependency */
    {
        gctUINT count = 0;


        if(count)
        {
            VIR_Instruction* iter_inst = inst;
            VIR_Instruction* next;
            VSC_HASH_TABLE* inst2node = VSC_IS_DepDag_GetInst2Node(dag);
            VSC_IS_DepDagNode *node = gcvNULL, *next_node = gcvNULL;
            gctUINT i;

            vscHTBL_DirectTestAndGet(inst2node, iter_inst, (void**)&node);
            gcmASSERT(node);
            for(i = 1; i < count; i++)
            {
                next = VIR_Inst_GetNext(iter_inst);
                vscHTBL_DirectTestAndGet(inst2node, next, (void**)&next_node);
                gcmASSERT(next_node);
                if(!_VSC_IS_DepDagNode_DepandsOnNode(next_node, node))
                {
                    return gcvFALSE;
                }
                iter_inst = next;
                node = next_node;
            }
            _VSC_IS_BindContinuousNodes(dag, inst, count);
            *continuous_count = count;
            return gcvTRUE;
        }
    }

    /* Bind loose binding nodes, add LOOSE_BINDING dependency and DODGING dependency */
    {
        if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
        {
            return _VSC_IS_LooselyBindLdarr(dag, inst);
        }
    }

    return gcvFALSE;
}

static void _VSC_IS_BindNodesOnCurrentBB(
    IN VIR_BASIC_BLOCK* bb,
    IN OUT VSC_IS_DepDag* dag
    )
{
    gctUINT32 len;
    VIR_Instruction *start_inst, *end_inst;
    VIR_Instruction* inst;
    gctUINT32 i;
    len = _VSC_IS_GetBBEssence(bb, &start_inst, &end_inst);

    i = 0;
    inst = start_inst;
    while(i < len)
    {
        gctUINT count = 0;
        _VSC_IS_BindNodes(dag, inst, &count);
        if(count)
        {
            i += count;
            while(count)
            {
                inst = VIR_Inst_GetNext(inst);
                --count;
            }
        }
        else
        {
            ++i;
            inst = VIR_Inst_GetNext(inst);
        }
    }
}

static VSC_ErrCode _VSC_IS_BuildDAGForBB_Gross(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;
    gctUINT32 len;
    VIR_Instruction *start_inst, *end_inst;
    VIR_Instruction* inst;
    gctUINT32 i;
    VIR_Shader* shader = VSC_IS_InstSched_GetShader(is);
    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VSC_HASH_TABLE* inst2node = VSC_IS_DepDag_GetInst2Node(dag);

    len = _VSC_IS_GetBBEssence(bb, &start_inst, &end_inst);

    for(i = 0, inst = start_inst; i < len; i++, inst = VIR_Inst_GetNext(inst))
    {
        VIR_Instruction* prev;
        gctINT32 j;
        VSC_IS_DepDagNode* node = _VSC_IS_DepDag_NewNode(dag, inst);
        gctUINT32 excludeCTs = VSC_IS_ConflictType_NONE;

        vscHTBL_DirectSet(inst2node, inst, node);
        _VSC_IS_DepDag_AddNode(dag, node);
        for(j = i - 1, prev = VIR_Inst_GetPrev(inst); j >= 0; j--, prev = VIR_Inst_GetPrev(prev))
        {
            gctUINT32 ct = _VSC_IS_InstConflict(shader, prev, inst, &excludeCTs);
            VSC_IS_DepDagNode* prev_node = gcvNULL;

            vscHTBL_DirectTestAndGet(inst2node, prev, (void**)&prev_node);
            gcmASSERT(prev_node);
            if(VSC_IS_DepDagNode_HasFlag(prev_node, VSC_IS_DEPDAGNODE_FLAG_DEPENDING_MOVA) &&
                VIR_Inst_GetOpcode(inst) == VIR_OP_MOVA)
            {
                VSC_IS_ConflictType_Set(ct, VSC_IS_ConflictType_DODGING);
            }

            /* add dependence edge */
            if(ct)
            {
                VSC_IS_DepDagEdge* edge;
                gctUINT32 bubble = _VSC_IS_ConflictType_GetBubble(ct, is);

                edge = _VSC_IS_DepDag_AddEdge(dag, prev_node, node);
                VSC_IS_DepDagEdge_SetConflictType(edge, ct);
                VSC_IS_DepDagEdge_SetBubble(edge, bubble);
                if(VSC_IS_ConflictType_HasLooseBindingMOVA(ct))
                {
                    VSC_IS_DepDagNode_AddFlag(node, VSC_IS_DEPDAGNODE_FLAG_DEPENDING_MOVA);
                }
            }
        }
    }

    return err_code;
}

static VSC_ErrCode _VSC_IS_BuildDAGForBB_DUPerChannel(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    gctUINT32 len;
    VIR_Instruction *start_inst, *end_inst;
    VIR_Instruction* def_inst;
    VSC_HASH_TABLE* inst2node = VSC_IS_DepDag_GetInst2Node(dag);
    gctUINT32 i;

    gcmASSERT(VSC_IS_InstSched_GetDUInfo(is));

    len = _VSC_IS_GetBBEssence(bb, &start_inst, &end_inst);
    /* compute for each instruction in BB */
    for(i = 0, def_inst = start_inst; i < len; i++, def_inst = VIR_Inst_GetNext(def_inst))
    {
        VSC_IS_DepDagNode* def_node = gcvNULL;
        gctUINT32 conflict_types = 0;
        VIR_Operand *def_dest;
        VIR_OpCode def_instopc;
        gctUINT8 channel;

        VIR_Enable def_enable = 0;
        VIR_VirRegId def_regid = 1000;
        VIR_GENERAL_DU_ITERATOR du_iter;

        def_dest = VIR_Inst_GetDest(def_inst);

        if(!def_dest)
        {
            continue;
        }

        vscHTBL_DirectTestAndGet(inst2node, def_inst, (void**)&def_node);
        gcmASSERT(def_node);
        def_instopc = VIR_Inst_GetOpcode(def_inst);
        if(VIR_Operand_GetOpKind(def_dest) == VIR_OPND_SYMBOL)
        {
            VIR_Symbol* def_sym = VIR_Operand_GetSymbol(def_dest);
            if(VIR_Symbol_isVreg(def_sym))
            {
                def_regid = VIR_Symbol_GetVregIndex(def_sym);
                def_enable = VIR_Operand_GetEnable(def_dest);
            }
        }

        /* compute per channel */
        for(channel = 0; channel < VIR_CHANNEL_NUM; channel ++)
        {
            VSC_IS_DepDagEdge* edge;
            VIR_USAGE* def_usage;
            if(!(def_enable & (1 << channel)))
            {
                continue;
            }

            /* get RAW depandence */
            vscVIR_InitGeneralDuIterator(&du_iter, VSC_IS_InstSched_GetDUInfo(is), def_inst, def_regid, channel, gcvTRUE);
            for(def_usage = vscVIR_GeneralDuIterator_First(&du_iter); def_usage != gcvNULL;
                def_usage = vscVIR_GeneralDuIterator_Next(&du_iter))
            {
                VSC_IS_DepDagNode* use_node = gcvNULL;
                VIR_Instruction* use_inst = def_usage->usageKey.pUsageInst;

                /* no need to add edge for BB ending branch inst */
                if(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(use_inst)))
                {
                    continue;
                }
                vscHTBL_DirectTestAndGet(inst2node, use_inst, (void**)&use_node);
                gcmASSERT(use_node);
                /* do not add edge for loop DU over inst itself */
                if(VSC_IS_DepDagNode_GetID(use_node) == VSC_IS_DepDagNode_GetID(def_node))
                {
                    continue;
                }
                /* do not add edge for loop DU to prevent cycle in DAG */
                if(VSC_IS_DepDagNode_GetID(use_node) < VSC_IS_DepDagNode_GetID(def_node))
                {
                    edge = _VSC_IS_DepDag_AddEdge(dag, use_node, def_node);
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_LOOP_CARRIED);
                }
                else
                {
                    edge = _VSC_IS_DepDag_AddEdge(dag, def_node, use_node);
                }
                if(VIR_OPCODE_isTexLd(def_instopc))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_TLRS_RU);
                }
                else if(VIR_OPCODE_isMemLd(def_instopc))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_MLRS_RU);
                }
                else if(VIR_OPCODE_isAttrLd(def_instopc))
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_CLRS_RU);
                }
                else
                {
                    VSC_IS_ConflictType_Set(conflict_types, VSC_IS_ConflictType_RS_RU);
                }
                VSC_IS_DepDagEdge_SetConflictType(edge, conflict_types);
            }
        }   /* end compute per channel */
    }   /* end compute for each instruction in BB */

    return err_code;
}

static VSC_ErrCode _VSC_IS_BuildDAGForBB(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;

    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);

    switch(VSC_OPTN_ISOptions_GetDepGran(options))
    {
        case VSC_OPTN_ISOptions_DEPGRAN_GROSS:
            _VSC_IS_BuildDAGForBB_Gross(is);
            break;
        case VSC_OPTN_ISOptions_DEPGRAN_DU_PER_CHANNAL:
            _VSC_IS_BuildDAGForBB_DUPerChannel(is);
            break;
        default:
            gcmASSERT(gcvFALSE);
    }

    /* sometimes we need to bind instructions in case other instructions be scheduled in between*/
    _VSC_IS_BindNodesOnCurrentBB(bb, dag);

    /* add pseudo end node in DAG for Bubble Scheduling */
    if(VSC_OPTN_ISOptions_GetAlgorithm(options) == VSC_OPTN_ISOptions_ALGORITHM_BUBBLESCHEDULING)
    {
        VSC_IS_DepDagNode* pseudo_end;
        VSC_SIMPLE_RESIZABLE_ARRAY* tail_array;
        gctUINT32 i, tail_count;
        VSC_IS_DepDagNode** tail_nodes;

        tail_array = DG_GET_TAIL_ARRAY_P(VSC_IS_DepDag_GetDGraph(dag));
        tail_count = vscSRARR_GetElementCount(tail_array);

        /* because tail_array changes during node adding, we need to copy them at first here */
        tail_nodes = (VSC_IS_DepDagNode**)malloc(sizeof(VSC_IS_DepDagNode*) * tail_count);
        for(i = 0; i < tail_count; i++)
        {
            tail_nodes[i] = *(VSC_IS_DepDagNode**)vscSRARR_GetElement(tail_array, i);
        }
        pseudo_end = _VSC_IS_DepDag_NewNode(dag, gcvNULL);
        _VSC_IS_DepDag_AddNode(dag, pseudo_end);
        /* add the pseudo_end node */
        for(i = 0; i < tail_count; i++)
        {
            _VSC_IS_DepDag_AddEdge(dag, tail_nodes[i], pseudo_end);
        }
        free(tail_nodes);

        _VSC_IS_DepDag_SetKillPriority(dag);
    }

    /* dump dependence dag */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_DAG))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling DAG for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, bb->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        _VSC_IS_DepDag_Dump(dag, dumper);
    }

    return err_code;
}

/********************************************** List Scheduling Starts **********************************************/

static void _VSC_IS_FW_PlaceInstruction(
    IN OUT VIR_Instruction** p_inst,
    IN VIR_Instruction* inst
    )
{
    gcmASSERT(p_inst != gcvNULL);
    gcmASSERT(inst != gcvNULL);
    VIR_Inst_SetNext(*p_inst, inst);
    VIR_Inst_SetPrev(inst, *p_inst);
    *p_inst = inst;
}
static void _VSC_IS_BW_PlaceInstruction(
    IN OUT VIR_Instruction** p_inst,
    IN VIR_Instruction* inst
    )
{
    gcmASSERT(p_inst != gcvNULL);
    gcmASSERT(inst != gcvNULL);
    VIR_Inst_SetPrev(*p_inst, inst);
    VIR_Inst_SetNext(inst, *p_inst);
    *p_inst = inst;
}

struct VSC_IS_HEURISTIC_BASE
{
    VSC_HASH_TABLE* in_set;
    VSC_HASH_TABLE* out_set;
    gctUINT32 to_schedule;
    VSC_IS_DepDagNode* last_scheduled;
    VSC_IS_InstSched* is;
};
typedef struct VSC_IS_HEURISTIC_BASE VSC_IS_Heuristic_Base;

#define VSC_IS_Heuristic_GetInSet(heur)                         (((VSC_IS_Heuristic_Base*)heur)->in_set)
#define VSC_IS_Heuristic_SetInSet(heur, i)                      (((VSC_IS_Heuristic_Base*)heur)->in_set = (i))
#define VSC_IS_Heuristic_GetInSetCount(heur)                    (HTBL_GET_ITEM_COUNT(((VSC_IS_Heuristic_Base*)heur)->in_set))
#define VSC_IS_Heuristic_GetOutSet(heur)                        (((VSC_IS_Heuristic_Base*)heur)->out_set)
#define VSC_IS_Heuristic_SetOutSet(heur, o)                     (((VSC_IS_Heuristic_Base*)heur)->out_set = (o))
#define VSC_IS_Heuristic_GetOutSetCount(heur)                   (HTBL_GET_ITEM_COUNT(((VSC_IS_Heuristic_Base*)heur)->out_set))
#define VSC_IS_Heuristic_GetToSchedule(heur)                    (((VSC_IS_Heuristic_Base*)heur)->to_schedule)
#define VSC_IS_Heuristic_SetToSchedule(heur, t)                 (((VSC_IS_Heuristic_Base*)heur)->to_schedule = (t))
#define VSC_IS_Heuristic_IncToSchedule(heur)                    (((VSC_IS_Heuristic_Base*)heur)->to_schedule++)
#define VSC_IS_Heuristic_DecToSchedule(heur)                    (((VSC_IS_Heuristic_Base*)heur)->to_schedule--)
#define VSC_IS_Heuristic_GetLastScheduled(heur)                 (((VSC_IS_Heuristic_Base*)heur)->last_scheduled)
#define VSC_IS_Heuristic_SetLastScheduled(heur, l)              (((VSC_IS_Heuristic_Base*)heur)->last_scheduled = (l))
#define VSC_IS_Heuristic_GetIS(heur)                            (((VSC_IS_Heuristic_Base*)heur)->is)
#define VSC_IS_Heuristic_SetIS(heur, i)                         (((VSC_IS_Heuristic_Base*)heur)->is = (i))
#define VSC_IS_Heuristic_GetTexldBubble(heur)                   (((VSC_IS_Heuristic_Base*)heur)->is->texld_dep_bubble)
#define VSC_IS_Heuristic_GetMemldBubble(heur)                   (((VSC_IS_Heuristic_Base*)heur)->is->memld_dep_bubble)
#define VSC_IS_Heuristic_GetCacheldBubble(heur)                 (((VSC_IS_Heuristic_Base*)heur)->is->cacheld_dep_bubble)
#define VSC_IS_Heuristic_GetDUInfo(heur)                        (((VSC_IS_Heuristic_Base*)heur)->is->du_info)
#define VSC_IS_Heuristic_GetCurrDAG(heur)                       (((VSC_IS_Heuristic_Base*)heur)->is->curr_dep_dag)
#define VSC_IS_Heuristic_GetInst2Node(heur)                     (((VSC_IS_Heuristic_Base*)heur)->is->inst2node)
#define VSC_IS_Heuristic_GetOptions(heur)                       (((VSC_IS_Heuristic_Base*)heur)->is->options)
#define VSC_IS_Heuristic_GetDumper(heur)                        (((VSC_IS_Heuristic_Base*)heur)->is->dumper)
#define VSC_IS_Heuristic_GetMM(heur)                            (((VSC_IS_Heuristic_Base*)heur)->is->pMM)

static void _VSC_IS_Heuristic_NewOutSet(
    IN OUT VSC_IS_Heuristic_Base* heur
    )
{
    VSC_HASH_TABLE* out_set = vscHTBL_Create(VSC_IS_Heuristic_GetMM(heur), vscHFUNC_Default, vscHKCMP_Default, 512);
    VSC_IS_Heuristic_SetOutSet(heur, out_set);
}

typedef enum VSC_IS_HEURISTIC_RESULT
{
    VSC_IS_Heuristic_Result_NotInvoked          = 0,
    VSC_IS_Heuristic_Result_PrerequisiteFail    = 1,
    VSC_IS_Heuristic_Result_AllMatch            = 2,
    VSC_IS_Heuristic_Result_PartialMatch        = 3,
    VSC_IS_Heuristic_Result_NoMatch             = 4,
} VSC_IS_Heuristic_Result;

#define VSC_IS_Heuristic_Result_HasMatch(r)     ((r) == VSC_IS_Heuristic_Result_AllMatch || \
                                                 (r) == VSC_IS_Heuristic_Result_PartialMatch)

typedef struct VSC_IS_FW_HEURISTIC VSC_IS_FW_Heuristic;
typedef VSC_ErrCode (*_VSC_IS_FW_Heuristic_FuncP)(IN OUT VSC_IS_FW_Heuristic* heur);
typedef struct VSC_IS_FW_HEURFUNCINFO
{
    _VSC_IS_FW_Heuristic_FuncP func;
    gctSTRING func_name;
} VSC_IS_FW_HeurFuncInfo;

#define VSC_IS_FW_HeurFuncInfo_GetFunc(hfi)             ((hfi)->func)
#define VSC_IS_FW_HeurFuncInfo_GetFuncName(hfi)         ((hfi)->func_name)

struct VSC_IS_FW_HEURISTIC
{
    VSC_IS_Heuristic_Base base;
    VSC_IS_FW_HeurFuncInfo heur_func_infos[VSC_OPTN_ISOptions_FW_HEUR_LAST];
    VSC_IS_Heuristic_Result results[VSC_OPTN_ISOptions_FW_HEUR_LAST];
    VSC_HASH_TABLE* texld_with_dep_bubble;
    VSC_HASH_TABLE* memld_with_dep_bubble;
    VSC_HASH_TABLE* cacheld_with_dep_bubble;
    gctUINT32 texld_interface_bubble;
    gctUINT32 memld_interface_bubble;
    /*gctUINT32 cacheld_interface_bubble;*/
    VSC_HASH_TABLE* kill_dep_sets;
};

#define VSC_IS_FW_Heuristic_GetBase(heur)                           (&(heur)->base)
#define VSC_IS_FW_Heuristic_GetHeurFuncInfos(heur)                  ((heur)->heur_func_infos)
#define VSC_IS_FW_Heuristic_GetHeurFuncInfo(heur, i)                ((heur)->heur_func_infos[(i)])
#define VSC_IS_FW_Heuristic_SetHeurFuncInfos(heur, h)               memcpy((heur)->heur_func_infos, (h), sizeof((heur)->heur_func_infos))
#define VSC_IS_FW_Heuristic_GetResults(heur)                        ((heur)->results)
#define VSC_IS_FW_Heuristic_GetResult(heur, i)                      ((heur)->results[(i)])
#define VSC_IS_FW_Heuristic_SetResult(heur, i, r)                   ((heur)->results[(i)] = (r))
#define VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur)             ((heur)->texld_with_dep_bubble)
#define VSC_IS_FW_Heuristic_SetTexldWithDepBubble(heur, t)          ((heur)->texld_with_dep_bubble = (t))
#define VSC_IS_FW_Heuristic_GetTexldWithDepBubbleCount(heur)        (HTBL_GET_ITEM_COUNT((heur)->texld_with_dep_bubble))
#define VSC_IS_FW_Heuristic_GetCacheldWithDepBubble(heur)           ((heur)->cacheld_with_dep_bubble)
#define VSC_IS_FW_Heuristic_SetCacheldWithDepBubble(heur, m)        ((heur)->cacheld_with_dep_bubble = (m))
#define VSC_IS_FW_Heuristic_GetCacheldWithDepBubbleCount(heur)      (HTBL_GET_ITEM_COUNT((heur)->cacheld_with_dep_bubble))
#define VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur)             ((heur)->memld_with_dep_bubble)
#define VSC_IS_FW_Heuristic_SetMemldWithDepBubble(heur, m)          ((heur)->memld_with_dep_bubble = (m))
#define VSC_IS_FW_Heuristic_GetMemldWithDepBubbleCount(heur)        (HTBL_GET_ITEM_COUNT((heur)->memld_with_dep_bubble))
#define VSC_IS_FW_Heuristic_GetTexldInterfaceBubble(heur)           ((heur)->texld_interface_bubble)
#define VSC_IS_FW_Heuristic_SetTexldInterfaceBubble(heur, t)        ((heur)->texld_interface_bubble = (t))
#define VSC_IS_FW_Heuristic_ResetTexldInterfaceBubble(heur)         ((heur)->texld_interface_bubble = (heur)->base.is->texld_interface_bubble)
#define VSC_IS_FW_Heuristic_DecTexldInterfaceBubble(heur)           ((heur)->texld_interface_bubble = (heur)->texld_interface_bubble > 0 ? (heur)->texld_interface_bubble - 1 : (heur)->texld_interface_bubble)
#define VSC_IS_FW_Heuristic_GetMemldInterfaceBubble(heur)           ((heur)->memld_interface_bubble)
#define VSC_IS_FW_Heuristic_SetMemldInterfaceBubble(heur, m)        ((heur)->memld_interface_bubble = (m))
#define VSC_IS_FW_Heuristic_ResetMemldInterfaceBubble(heur)         ((heur)->memld_interface_bubble = (heur)->base.is->memld_interface_bubble)
#define VSC_IS_FW_Heuristic_DecMemldInterfaceBubble(heur)           ((heur)->memld_interface_bubble = (heur)->memld_interface_bubble > 0 ? (heur)->memld_interface_bubble - 1 : (heur)->memld_interface_bubble)
#define VSC_IS_FW_Heuristic_GetKillDepSets(heur)                    ((heur)->kill_dep_sets)
#define VSC_IS_FW_Heuristic_SetKillDepSets(heur, k)                 ((heur)->kill_dep_sets = (k))

/* range could be set by options. only instructions in range will be scheduled */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferRange(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set = gcvNULL;
    VSC_OPTN_ISOptions* options = VSC_IS_Heuristic_GetOptions(heur);
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);
    vscHTBLIterator_Init(&iter, in_set);
    if(!VSC_OPTN_InRange(VSC_IS_Heuristic_GetToSchedule(heur), VSC_OPTN_ISOptions_GetBeforeInst(options), VSC_OPTN_ISOptions_GetAfterInst(options)))
    {
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VSC_IS_DepDagNode_GetID(node) == VSC_IS_Heuristic_GetToSchedule(heur))
            {
                vscHTBL_DirectSet(out_set, node, gcvNULL);
                break;
            }
        }
        gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) == 1);
    }
    else
    {
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VSC_OPTN_InRange(VSC_IS_DepDagNode_GetID(node), VSC_OPTN_ISOptions_GetBeforeInst(options), VSC_OPTN_ISOptions_GetAfterInst(options)))
            {
                vscHTBL_DirectSet(out_set, node, gcvNULL);
            }
        }
        gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) > 0);
    }

    return error_code;
}

/* if the last scheduled instruction has binding instruction, select this binding instruction.
   this heuristic is used to keep some instruction sequence pattern, for exampler, atomic
   instruction sequence */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferBinding(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
#endif
    VSC_IS_DepDagNode* last = VSC_IS_Heuristic_GetLastScheduled(heur);
    VSC_ADJACENT_LIST* succ_edge_list;
    VSC_UL_ITERATOR succ_edge_iter;
    VSC_IS_DepDagEdge* succ_edge;

    if(last == gcvNULL || !VSC_IS_DepDagNode_HasFlag(last, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC))
    {
        return error_code;
    }

    succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(last);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&succ_edge_iter, succ_edge_list);
    for(succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succ_edge_iter);
        succ_edge != gcvNULL; succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succ_edge_iter))
    {
        if(VSC_IS_DepDagEdge_HasConflictType(succ_edge, VSC_IS_ConflictType_CONTINUOUS_BINDING))
        {
            VSC_HASH_TABLE* out_set;

            _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
            out_set = VSC_IS_Heuristic_GetOutSet(heur);
            gcmASSERT(vscHTBL_DirectTestAndGet(in_set, VSC_IS_DepDagEdge_GetToNode(succ_edge), gcvNULL));
            gcmASSERT(VSC_IS_DepDagNode_HasFlag(VSC_IS_DepDagEdge_GetToNode(succ_edge), VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_PRED));
            vscHTBL_DirectSet(out_set, VSC_IS_DepDagEdge_GetToNode(succ_edge), gcvNULL);
            break;
        }
    }
    gcmASSERT(succ_edge != gcvNULL);

    return error_code;
}

/* if kill instruction is ready, we pick it */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferKill(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node)) == VIR_OP_KILL)
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* instructions having kill depending on it will be chose here. of cousre, only those
   instructions which have the minimum dependency weight will be chose. in this way we
   can execute kill instruction as early as possible. */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferKillDep(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    VSC_HASH_TABLE* kill_dep_sets = VSC_IS_FW_Heuristic_GetKillDepSets(heur);
    VSC_HASH_TABLE* smallest_kill_dep_set = gcvNULL;

    if(!HTBL_GET_ITEM_COUNT(kill_dep_sets))
    {
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        VSC_HASH_ITERATOR iter_in;
        VSC_DIRECT_HNODE_PAIR pair_in;
        vscHTBLIterator_Init(&iter_in, kill_dep_sets);
        for(pair_in = vscHTBLIterator_DirectFirst(&iter_in);
            IS_VALID_DIRECT_HNODE_PAIR(&pair_in); pair_in = vscHTBLIterator_DirectNext(&iter_in))
        {
            VSC_HASH_TABLE* kill_dep_set = (VSC_HASH_TABLE*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair_in);
            if(vscHTBL_DirectTestAndGet(kill_dep_set, (void*)node, gcvNULL))
            {
                if(smallest_kill_dep_set == gcvNULL ||
                   HTBL_GET_ITEM_COUNT(kill_dep_set) < HTBL_GET_ITEM_COUNT(smallest_kill_dep_set))
                {
                    smallest_kill_dep_set = kill_dep_set;
                }
            }
        }
    }

    if(smallest_kill_dep_set == gcvNULL)    /* no node is in kill_dep_set */
    {
        return error_code;
    }

    /* pick the nodes which are in the smallest_kill_dep_set */
    vscHTBLIterator_Init(&iter, in_set);
    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(vscHTBL_DirectTestAndGet(smallest_kill_dep_set, (void*)node, gcvNULL))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* if there is not texld dep bubble so far, we prefer texld, so supposedly we can have more
   instruction to fill bubbles */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PrePreferTexld(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_TABLE* texld_with_bubble = VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur);
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    if(HTBL_GET_ITEM_COUNT(texld_with_bubble))
    {
        VSC_IS_FW_Heuristic_SetResult(heur, VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_TEXLD_ID, VSC_IS_Heuristic_Result_PrerequisiteFail);
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* if there is not memld dep bubble so far, we prefer memld, so supposedly we can have more
   instruction to fill bubbles */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PrePreferMemld(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_TABLE* memld_with_bubble = VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur);
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    if(HTBL_GET_ITEM_COUNT(memld_with_bubble))
    {
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* select instructions which can fill current bubble */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferAntiBubble(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_TABLE* texld_with_bubble = VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur);
    VSC_HASH_TABLE* memld_with_bubble = VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur);
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    gctUINT32 min_bubble = gcvMAXUINT32;

    if(!HTBL_GET_ITEM_COUNT(texld_with_bubble) && !HTBL_GET_ITEM_COUNT(memld_with_bubble))
    {
        VSC_IS_FW_Heuristic_SetResult(heur, VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE_ID, VSC_IS_Heuristic_Result_PrerequisiteFail);
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        gctUINT32 texld_bubble = _VSC_IS_DepDagNode_DepandsOnBubbleSet(node, texld_with_bubble);
        gctUINT32 memld_bubble = _VSC_IS_DepDagNode_DepandsOnBubbleSet(node, memld_with_bubble);
        gctUINT32 node_bubble = texld_bubble > memld_bubble ? texld_bubble : memld_bubble;
        if(node_bubble < min_bubble)
        {
            min_bubble = node_bubble;
        }
        vscHTBL_DirectSet(out_set, node, (void*)(gctUINTPTR_T)node_bubble);
    }

    vscHTBLIterator_Init(&iter, out_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        gctUINT32 node_bubble = (gctUINT32)(gctUINTPTR_T)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        if(node_bubble != min_bubble)
        {
            vscHTBL_DirectRemove(out_set, node);
        }
    }

    if(VSC_IS_Heuristic_GetOutSetCount(heur))
    {
        if(VSC_IS_Heuristic_GetOutSetCount(heur) == VSC_IS_Heuristic_GetInSetCount(heur))
        {
            VSC_IS_FW_Heuristic_SetResult(heur, VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE_ID, VSC_IS_Heuristic_Result_AllMatch);
        }
        else
        {
            VSC_IS_FW_Heuristic_SetResult(heur, VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE_ID, VSC_IS_Heuristic_Result_PartialMatch);
        }
    }
    else
    {
        VSC_IS_FW_Heuristic_SetResult(heur, VSC_OPTN_ISOptions_FW_HEUR_PREFER_ANTI_BUBBLE_ID, VSC_IS_Heuristic_Result_NoMatch);
    }

    return error_code;
}

/* the following four heuristics are used to accomodate bandwidth bubble. they may not
   be useful, because bandwidth bubble is not important comparing with dependancy bubble */

/* if there is no texld interface bubble, prefer texld as anti bubble instruction */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PostPreferTexld(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    if(VSC_IS_FW_Heuristic_GetTexldInterfaceBubble(heur))
    {
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* if there is no memld interface bubble, prefer memld as anti bubble instruction */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PostPreferMemld(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    if(VSC_IS_FW_Heuristic_GetMemldInterfaceBubble(heur))
    {
        return error_code;
    }
    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

/* if there is texld interface bubble, delay texld as anti bubble instruction */
static VSC_ErrCode _VSC_IS_FW_Heuristic_DelayTexLd(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBL_DirectDuplicate(out_set, in_set);
    if(VSC_IS_FW_Heuristic_GetTexldWithDepBubbleCount(heur))
    {
        VSC_DIRECT_HNODE_PAIR pair;
        vscHTBLIterator_Init(&iter, out_set);

        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
            {
                vscHTBL_Remove(out_set, node);
            }
        }
    }

    return error_code;
}

/* if there is memld interface bubble, delay memld as anti bubble instruction */
static VSC_ErrCode _VSC_IS_FW_Heuristic_DelayMemLd(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBL_DirectDuplicate(out_set, in_set);
    if(VSC_IS_FW_Heuristic_GetMemldWithDepBubbleCount(heur))
    {
        VSC_DIRECT_HNODE_PAIR pair;
        vscHTBLIterator_Init(&iter, out_set);

        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
            {
                vscHTBL_Remove(out_set, node);
            }
        }
    }

    return error_code;
}

/* prefer texld/memld as anti bubble instruction */
static VSC_ErrCode _VSC_IS_FW_Heuristic_PostPreferTexldMemld(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))) ||
           VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))) ||
           VIR_OPCODE_isAttrLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(node))))
        {
            vscHTBL_DirectSet(out_set, node, gcvNULL);
        }
    }
    return error_code;
}

static VSC_ErrCode _VSC_IS_FW_Heuristic_PreferOrder(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    gctUINT32 min_id = gcvMAXUINT32;
    VSC_IS_DepDagNode* min_node = gcvNULL;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);
    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VSC_IS_DepDagNode_GetID(node) < min_id)
        {
            min_id = VSC_IS_DepDagNode_GetID(node);
            min_node = node;
        }
    }
    gcmASSERT(min_node);
    vscHTBL_DirectSet(out_set, min_node, gcvNULL);

    gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) == 1);
    return error_code;
}

static VSC_ErrCode _VSC_IS_FW_Heuristic_CollectKillDepSet(
    IN VSC_IS_DepDagNode* node,
    IN OUT VSC_HASH_TABLE* kill_dep_set
    )
{
    VSC_ErrCode err_code = VSC_ERR_NONE;
    VSC_ADJACENT_LIST* edge_list;
    VSC_UL_ITERATOR iter;
    VSC_IS_DepDagEdge* edge;

    if(vscHTBL_DirectTestAndGet(kill_dep_set, node, gcvNULL))
    {
        return err_code;
    }
    vscHTBL_DirectSet(kill_dep_set, (void*)node, gcvNULL);
    edge_list = VSC_IS_DepDagNode_GetPredEdgeList(node);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        VSC_IS_DepDagNode* pred = VSC_IS_DepDagEdge_GetToNode(edge);
        err_code = _VSC_IS_FW_Heuristic_CollectKillDepSet(pred, kill_dep_set);
    }
    return err_code;
}

static VSC_ErrCode _VSC_IS_FW_Heuristic_CollectKillDepSets(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(VSC_IS_Heuristic_GetIS(heur));
    VSC_DG_ITERATOR dg_iter;
    VSC_IS_DepDagNode* node;
    VSC_HASH_TABLE* kill_dep_sets = vscHTBL_Create(VSC_IS_Heuristic_GetMM(heur), vscHFUNC_Default, vscHKCMP_Default, 512);

    vscDG_ITERATOR_Initialize(&dg_iter, VSC_IS_DepDag_GetDGraph(dag), VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
        VSC_GRAPH_TRAVERSAL_ORDER_POST, gcvFALSE);

    for(node = (VSC_IS_DepDagNode*)vscDG_ITERATOR_Begin(&dg_iter); node != gcvNULL; node = (VSC_IS_DepDagNode*)vscDG_ITERATOR_Next(&dg_iter))
    {
        VIR_Instruction* inst = VSC_IS_DepDagNode_GetInst(node);

        if(VIR_Inst_GetOpcode(inst) == VIR_OP_KILL)
        {
            VSC_HASH_TABLE* kill_dep_set = vscHTBL_Create(VSC_IS_Heuristic_GetMM(heur), vscHFUNC_Default, vscHKCMP_Default, 512);
            _VSC_IS_FW_Heuristic_CollectKillDepSet(node, kill_dep_set);
            vscHTBL_DirectSet(kill_dep_sets, (void*)node, (void*)kill_dep_set);
        }
    }

    VSC_IS_FW_Heuristic_SetKillDepSets(heur, kill_dep_sets);
    return err_code;
}

static void _VSC_IS_FW_Heuristic_Init(
    IN OUT VSC_IS_FW_Heuristic* heur,
    IN VSC_IS_InstSched* is
    )
{
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);
    VSC_HASH_TABLE* texld_with_bubble;
    VSC_HASH_TABLE* memld_with_bubble;
    VSC_IS_FW_HeurFuncInfo hfis[] = {
        {_VSC_IS_FW_Heuristic_PreferRange, "FW_PreferRange"},
        {_VSC_IS_FW_Heuristic_PreferBinding, "FW_PreferBinding"},
        {_VSC_IS_FW_Heuristic_PreferKillDep, "FW_Pre_PreferKillDep"},
        {_VSC_IS_FW_Heuristic_PreferKill, "FW_PreferKillDep"},
        {_VSC_IS_FW_Heuristic_PrePreferTexld, "FW_PrePreferTexld"},
        {_VSC_IS_FW_Heuristic_PrePreferMemld, "FW_PrePreferMemld"},
        {_VSC_IS_FW_Heuristic_PreferAntiBubble, "FW_PreferAntiBubble"},
        {_VSC_IS_FW_Heuristic_PostPreferTexld, "FW_PostPreferTexld"},
        {_VSC_IS_FW_Heuristic_PostPreferMemld, "FW_PostPreferMemld"},
        {_VSC_IS_FW_Heuristic_DelayTexLd, "FW_DelayTexLd"},
        {_VSC_IS_FW_Heuristic_DelayMemLd, "FW_DelayMemLd"},
        {_VSC_IS_FW_Heuristic_PostPreferTexldMemld, "FW_PostPreferTexldMemld"},
        {_VSC_IS_FW_Heuristic_PreferKillDep, "FW_Post_PreferKillDep"},
        {_VSC_IS_FW_Heuristic_PreferOrder, "FW_PreferOrder"},
    };
    VSC_IS_Heuristic_SetIS(heur, is);
    VSC_IS_FW_Heuristic_SetHeurFuncInfos(heur, hfis);
    gcoOS_ZeroMemory(VSC_IS_FW_Heuristic_GetResults(heur), sizeof(VSC_IS_Heuristic_Result) * VSC_OPTN_ISOptions_FW_HEUR_LAST);
    VSC_IS_Heuristic_SetInSet(heur, gcvNULL);
    VSC_IS_Heuristic_SetOutSet(heur, gcvNULL);
    texld_with_bubble = vscHTBL_Create(VSC_IS_Heuristic_GetMM(heur), vscHFUNC_Default, vscHKCMP_Default, 512);
    VSC_IS_FW_Heuristic_SetTexldWithDepBubble(heur, texld_with_bubble);
    VSC_IS_FW_Heuristic_SetTexldInterfaceBubble(heur, 0);
    memld_with_bubble = vscHTBL_Create(VSC_IS_Heuristic_GetMM(heur), vscHFUNC_Default, vscHKCMP_Default, 512);
    VSC_IS_FW_Heuristic_SetMemldWithDepBubble(heur, memld_with_bubble);
    VSC_IS_FW_Heuristic_SetMemldInterfaceBubble(heur, 0);
    VSC_IS_Heuristic_SetToSchedule(heur, 0);
    VSC_IS_Heuristic_SetLastScheduled(heur, gcvNULL);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetFwHeuristics(options), VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_KILLDEP)
        || VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetFwHeuristics(options), VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_KILLDEP))
    {
        _VSC_IS_FW_Heuristic_CollectKillDepSets(heur);
    }
}

typedef struct VSC_IS_BW_HEURISTIC VSC_IS_BW_Heuristic;
typedef VSC_ErrCode (*_VSC_IS_BW_Heuristic_FuncP)(IN OUT VSC_IS_BW_Heuristic* heur);
typedef struct VSC_IS_BW_HEURFUNCINFO
{
    _VSC_IS_BW_Heuristic_FuncP func;
    gctSTRING func_name;
} VSC_IS_BW_HeurFuncInfo;

#define VSC_IS_BW_HeurFuncInfo_GetFunc(hfi)             ((hfi)->func)
#define VSC_IS_BW_HeurFuncInfo_GetFuncName(hfi)         ((hfi)->func_name)

struct VSC_IS_BW_HEURISTIC
{
    VSC_IS_Heuristic_Base base;
    VSC_IS_BW_HeurFuncInfo heur_func_infos[VSC_OPTN_ISOptions_BW_HEUR_LAST];
    VSC_IS_Heuristic_Result results[VSC_OPTN_ISOptions_BW_HEUR_LAST];
};

#define VSC_IS_BW_Heuristic_GetBase(heur)                           (&(heur)->base)
#define VSC_IS_BW_Heuristic_GetHeurFuncInfos(heur)                  ((heur)->heur_func_infos)
#define VSC_IS_BW_Heuristic_GetHeurFuncInfo(heur, i)                ((heur)->heur_func_infos[(i)])
#define VSC_IS_BW_Heuristic_SetHeurFuncInfos(heur, h)               memcpy((heur)->heur_func_infos, (h), sizeof((heur)->heur_func_infos))
#define VSC_IS_BW_Heuristic_GetResults(heur)                        ((heur)->results)
#define VSC_IS_BW_Heuristic_GetResult(heur, i)                      ((heur)->results[(i)])
#define VSC_IS_BW_Heuristic_SetResult(heur, i, r)                   ((heur)->results[(i)] = (r))

/* range could be set by options. only instructions in range will be scheduled */
static VSC_ErrCode _VSC_IS_BW_Heuristic_PreferRange(
    IN OUT VSC_IS_BW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set = gcvNULL;
    VSC_OPTN_ISOptions* options = VSC_IS_Heuristic_GetOptions(heur);
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);
    vscHTBLIterator_Init(&iter, in_set);
    if(!VSC_OPTN_InRange(VSC_IS_Heuristic_GetToSchedule(heur), VSC_OPTN_ISOptions_GetBeforeInst(options), VSC_OPTN_ISOptions_GetAfterInst(options)))
    {
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VSC_IS_DepDagNode_GetID(node) == VSC_IS_Heuristic_GetToSchedule(heur))
            {
                vscHTBL_DirectSet(out_set, node, gcvNULL);
                break;
            }
        }
        gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) == 1);
    }
    else
    {
        for(pair = vscHTBLIterator_DirectFirst(&iter);
            IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
        {
            VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
            if(VSC_OPTN_InRange(VSC_IS_DepDagNode_GetID(node), VSC_OPTN_ISOptions_GetBeforeInst(options), VSC_OPTN_ISOptions_GetAfterInst(options)))
            {
                vscHTBL_DirectSet(out_set, node, gcvNULL);
            }
        }
        gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) > 0);
    }

    return error_code;
}

static VSC_ErrCode _VSC_IS_BW_Heuristic_PreferOrder(
    IN OUT VSC_IS_BW_Heuristic* heur
    )
{
    VSC_ErrCode error_code = VSC_ERR_NONE;
    VSC_HASH_TABLE* in_set = VSC_IS_Heuristic_GetInSet(heur);
    VSC_HASH_TABLE* out_set;
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    gctUINT32 max_id = gcvMINUINT32;
    VSC_IS_DepDagNode* max_node = gcvNULL;

    _VSC_IS_Heuristic_NewOutSet(VSC_IS_FW_Heuristic_GetBase(heur));
    out_set = VSC_IS_Heuristic_GetOutSet(heur);

    vscHTBLIterator_Init(&iter, in_set);
    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(VSC_IS_DepDagNode_GetID(node) >= max_id)
        {
            max_id = VSC_IS_DepDagNode_GetID(node);
            max_node = node;
        }
    }
    gcmASSERT(max_node);
    vscHTBL_DirectSet(out_set, max_node, gcvNULL);

    gcmASSERT(HTBL_GET_ITEM_COUNT(out_set) == 1);
    return error_code;
}

static void _VSC_IS_BW_Heuristic_Init(
    IN OUT VSC_IS_BW_Heuristic* heur,
    IN VSC_IS_InstSched* is
    )
{
    VSC_IS_BW_HeurFuncInfo hfis[] = {
        {_VSC_IS_BW_Heuristic_PreferRange, "BW_PreferRange"},
        {_VSC_IS_BW_Heuristic_PreferOrder, "BW_PreferOrder"},
    };
    VSC_IS_Heuristic_SetIS(heur, is);
    VSC_IS_BW_Heuristic_SetHeurFuncInfos(heur, hfis);
    gcoOS_ZeroMemory(VSC_IS_BW_Heuristic_GetResults(heur), sizeof(VSC_IS_Heuristic_Result) * VSC_OPTN_ISOptions_BW_HEUR_LAST);
    VSC_IS_Heuristic_SetInSet(heur, gcvNULL);
    VSC_IS_Heuristic_SetOutSet(heur, gcvNULL);
    VSC_IS_Heuristic_SetToSchedule(heur, VSC_IS_DepDag_GetNodeCount(VSC_IS_InstSched_GetCurrDepDag(is)) - 1);
    VSC_IS_Heuristic_SetLastScheduled(heur, gcvNULL);
}

static VSC_IS_DepDagNode* _VSC_IS_Heuristic_GetFirstInstFromInset(
    IN VSC_IS_Heuristic_Base* heur
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    vscHTBLIterator_Init(&iter, VSC_IS_Heuristic_GetInSet(heur));
    pair = vscHTBLIterator_DirectFirst(&iter);
    return (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
}

static VSC_IS_DepDagNode* _VSC_IS_Heuristic_GetFirstInstFromOutset(
    IN VSC_IS_Heuristic_Base* heur
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    vscHTBLIterator_Init(&iter, VSC_IS_Heuristic_GetOutSet(heur));
    pair = vscHTBLIterator_DirectFirst(&iter);
    return (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
}

static void _VSC_IS_FW_Heuristic_Final(
    IN VSC_IS_FW_Heuristic* heur
    )
{
}

static void _VSC_IS_BW_Heuristic_Final(
    IN VSC_IS_BW_Heuristic* heur
    )
{
}

static void _VSC_IS_Heuristic_DumpResultSet(
    IN VSC_IS_Heuristic_Base* heur,
    IN VIR_Dumper* dumper)
{
    if(HTBL_GET_ITEM_COUNT(VSC_IS_Heuristic_GetOutSet(heur)))
    {
        _VSC_IS_DumpInstSet(VSC_IS_Heuristic_GetOutSet(heur), dumper);
    }
    else if(HTBL_GET_ITEM_COUNT(VSC_IS_Heuristic_GetInSet(heur)))
    {
        _VSC_IS_DumpInstSet(VSC_IS_Heuristic_GetInSet(heur), dumper);
    }
}

static void _VSC_IS_FW_Heuristic_Update(
    IN VSC_IS_FW_Heuristic* heur,
    VSC_IS_DepDagNode* result
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    VSC_IS_DepDagNode* node;
    gctUINT32 bubble;

    vscHTBLIterator_Init(&iter, VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur));
    for(pair = vscHTBLIterator_DirectFirst(&iter); IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        bubble = (gctUINT32)(gctUINTPTR_T)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        if(bubble == 1 || _VSC_IS_DepDagNode_DepandsOnNode(result, node))
        {
            vscHTBL_Remove(VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur), node);
        }
        else
        {
            vscHTBL_DirectSet(VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur), node, (void*)(gctUINTPTR_T)(bubble - 1));
        }
    }

    vscHTBLIterator_Init(&iter, VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur));
    for(pair = vscHTBLIterator_DirectFirst(&iter); IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        bubble = (gctUINT32)(gctUINTPTR_T)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        if(bubble == 1 || _VSC_IS_DepDagNode_DepandsOnNode(result, node))
        {
            vscHTBL_Remove(VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur), node);
        }
        else
        {
            vscHTBL_DirectSet(VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur), node, (void*)(gctUINTPTR_T)(bubble - 1));
        }
    }

    if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(result))))
    {
        vscHTBL_DirectSet(VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur), result, (void*)(gctUINTPTR_T)(VSC_IS_Heuristic_GetTexldBubble(heur)));
        VSC_IS_FW_Heuristic_ResetTexldInterfaceBubble(heur);
    }
    else
    {
        VSC_IS_FW_Heuristic_DecTexldInterfaceBubble(heur);
    }
    if(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(VSC_IS_DepDagNode_GetInst(result))))
    {
        vscHTBL_DirectSet(VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur), result, (void*)(gctUINTPTR_T)(VSC_IS_Heuristic_GetMemldBubble(heur)));
        VSC_IS_FW_Heuristic_ResetMemldInterfaceBubble(heur);
    }
    else
    {
        VSC_IS_FW_Heuristic_DecMemldInterfaceBubble(heur);
    }
    /* update kill dependency sets */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetFwHeuristics(VSC_IS_InstSched_GetOptions(VSC_IS_Heuristic_GetIS(heur))), VSC_OPTN_ISOptions_FW_HEUR_PRE_PREFER_KILLDEP)
        || VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetFwHeuristics(VSC_IS_InstSched_GetOptions(VSC_IS_Heuristic_GetIS(heur))), VSC_OPTN_ISOptions_FW_HEUR_POST_PREFER_KILLDEP))
    {
        VSC_HASH_TABLE* kill_dep_sets = VSC_IS_FW_Heuristic_GetKillDepSets(heur);

        if(HTBL_GET_ITEM_COUNT(kill_dep_sets))
        {
            VSC_HASH_ITERATOR iter;
            VSC_DIRECT_HNODE_PAIR pair;
            vscHTBLIterator_Init(&iter, kill_dep_sets);

            for(pair = vscHTBLIterator_DirectFirst(&iter);
                IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
            {
                VSC_HASH_TABLE* kill_dep_set = (VSC_HASH_TABLE*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
                if(vscHTBL_DirectTestAndGet(kill_dep_set, (void*)result, gcvNULL))
                {
                    vscHTBL_DirectRemove(kill_dep_set, (void*)result);
                    if(result == (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair))
                    {
                        vscHTBL_DirectRemove(kill_dep_sets, result);
                    }
                }
            }
        }
    }
    VSC_IS_DepDagNode_SetScheduledPosition(result, VSC_IS_Heuristic_GetToSchedule(heur));
    VSC_IS_Heuristic_IncToSchedule(heur);
    VSC_IS_Heuristic_SetLastScheduled(heur, result);
}

static void _VSC_IS_BW_Heuristic_Update(
    IN VSC_IS_BW_Heuristic* heur,
    VSC_IS_DepDagNode* result
    )
{
    VSC_IS_DepDagNode_SetScheduledPosition(result, VSC_IS_Heuristic_GetToSchedule(heur));
    VSC_IS_Heuristic_DecToSchedule(heur);
    VSC_IS_Heuristic_SetLastScheduled(heur, result);
}

static VSC_IS_DepDagNode* _VSC_IS_FW_DoHeuristics(
    IN OUT VSC_IS_FW_Heuristic* heur
    )
{
    VSC_OPTN_ISOptions* options = VSC_IS_Heuristic_GetOptions(heur);
    gctUINT32 i;
    VSC_IS_DepDagNode* result = gcvNULL;

    /* dump instruction set at the beginning */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
    {
        VIR_Dumper* dumper = VSC_IS_Heuristic_GetDumper(heur);
        VIR_LOG(dumper, "Do Heuristics:\ninput instruction set:\n");
        VIR_LOG_FLUSH(dumper);
        _VSC_IS_DumpInstSet(VSC_IS_Heuristic_GetInSet(heur), dumper);
        VIR_LOG(dumper, "texld bubble set:\n");
        VIR_LOG_FLUSH(dumper);
        _VSC_IS_DumpInstSet(VSC_IS_FW_Heuristic_GetTexldWithDepBubble(heur), dumper);
        VIR_LOG_FLUSH(dumper);
        VIR_LOG(dumper, "memld bubble set:\n");
        VIR_LOG_FLUSH(dumper);
        _VSC_IS_DumpInstSet(VSC_IS_FW_Heuristic_GetMemldWithDepBubble(heur), dumper);
        VIR_LOG_FLUSH(dumper);
        VIR_LOG(dumper, "texld interface bubble: %d\n", VSC_IS_FW_Heuristic_GetTexldInterfaceBubble(heur));
        VIR_LOG_FLUSH(dumper);
        VIR_LOG(dumper, "memld interface bubble: %d\n", VSC_IS_FW_Heuristic_GetMemldInterfaceBubble(heur));
        VIR_LOG_FLUSH(dumper);
    }

    if(VSC_IS_Heuristic_GetInSetCount(heur) == 1)
    {
        result = _VSC_IS_Heuristic_GetFirstInstFromInset(VSC_IS_FW_Heuristic_GetBase(heur));
    }
    else
    {
        for(i = 0; i < VSC_OPTN_ISOptions_FW_HEUR_LAST; i++)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetFwHeuristics(VSC_IS_Heuristic_GetOptions(heur)),
                1 << i))
            {
                /* execute the heuristic */
                VSC_IS_FW_HeurFuncInfo_GetFunc(&VSC_IS_FW_Heuristic_GetHeurFuncInfo(heur, i))(heur);
                /* dump instruction set after a heuristic is executed */
                if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
                {
                    VIR_Dumper* dumper = VSC_IS_Heuristic_GetDumper(heur);
                    VIR_LOG(dumper, "after %s:\n", VSC_IS_FW_HeurFuncInfo_GetFuncName(&VSC_IS_FW_Heuristic_GetHeurFuncInfo(heur, i)));
                    VIR_LOG_FLUSH(dumper);
                    _VSC_IS_Heuristic_DumpResultSet(VSC_IS_FW_Heuristic_GetBase(heur), dumper);
                }
                if(VSC_IS_Heuristic_GetOutSetCount(heur) == 1)
                {
                    result = _VSC_IS_Heuristic_GetFirstInstFromOutset(VSC_IS_FW_Heuristic_GetBase(heur));
                    break;
                }
                else if(VSC_IS_Heuristic_GetOutSetCount(heur) > 1)
                {
                    VSC_IS_Heuristic_SetInSet(heur, VSC_IS_Heuristic_GetOutSet(heur));
                }
            }
        }

        gcmASSERT(VSC_IS_Heuristic_GetOutSetCount(heur) == 1);
    }
    _VSC_IS_FW_Heuristic_Update(heur, result);
    VSC_IS_Heuristic_SetOutSet(heur, gcvNULL);

    return result;
}

static VSC_IS_DepDagNode* _VSC_IS_BW_DoHeuristics(
    IN OUT VSC_IS_BW_Heuristic* heur
    )
{
    VSC_OPTN_ISOptions* options = VSC_IS_Heuristic_GetOptions(heur);
    gctUINT32 i;
    VSC_IS_DepDagNode* result = gcvNULL;

    /* dump instruction set at the beginning */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
    {
        VIR_Dumper* dumper = VSC_IS_Heuristic_GetDumper(heur);
        VIR_LOG(dumper, "Do Heuristics:\ninput instruction set:\n");
        VIR_LOG_FLUSH(dumper);
        _VSC_IS_DumpInstSet(VSC_IS_Heuristic_GetInSet(heur), dumper);
        VIR_LOG_FLUSH(dumper);
    }

    if(VSC_IS_Heuristic_GetInSetCount(heur) == 1)
    {
        VSC_HASH_ITERATOR iter;
        VSC_DIRECT_HNODE_PAIR pair;
        vscHTBLIterator_Init(&iter, VSC_IS_Heuristic_GetInSet(heur));
        pair = vscHTBLIterator_DirectFirst(&iter);
        result = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
    }
    else
    {
        for(i = 0; i < VSC_OPTN_ISOptions_BW_HEUR_LAST; i++)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetBwHeuristics(VSC_IS_Heuristic_GetOptions(heur)),
                1 << i))
            {
                /* execute the heuristic */
                VSC_IS_BW_HeurFuncInfo_GetFunc(&VSC_IS_BW_Heuristic_GetHeurFuncInfo(heur, i))(heur);
                /* dump instruction set after a heuristic is executed */
                if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
                {
                    VIR_Dumper* dumper = VSC_IS_Heuristic_GetDumper(heur);
                    VIR_LOG(dumper, "after %s:\n", VSC_IS_BW_HeurFuncInfo_GetFuncName(&VSC_IS_FW_Heuristic_GetHeurFuncInfo(heur, i)));
                    VIR_LOG_FLUSH(dumper);
                    _VSC_IS_Heuristic_DumpResultSet(VSC_IS_BW_Heuristic_GetBase(heur), dumper);
                }
                if(VSC_IS_Heuristic_GetOutSetCount(heur) == 1)
                {
                    result = _VSC_IS_Heuristic_GetFirstInstFromOutset(VSC_IS_FW_Heuristic_GetBase(heur));
                    break;
                }
                else if(VSC_IS_Heuristic_GetOutSetCount(heur) > 1)
                {
                    VSC_IS_Heuristic_SetInSet(heur, VSC_IS_Heuristic_GetOutSet(heur));
                }
            }
        }
        gcmASSERT(VSC_IS_Heuristic_GetOutSetCount(heur) == 1);
    }
    _VSC_IS_BW_Heuristic_Update(heur, result);
    VSC_IS_Heuristic_SetOutSet(heur, gcvNULL);

    return result;
}

typedef struct VSC_IS_LISTSCHEDULING
{
    VSC_IS_InstSched* is;
    VSC_HASH_TABLE cands;
    VSC_IS_FW_Heuristic fw_heur;
    VSC_IS_BW_Heuristic bw_heur;
} VSC_IS_ListScheduling;

#define VSC_IS_ListScheduling_GetIS(ls)             ((ls)->is)
#define VSC_IS_ListScheduling_SetIS(ls, i)          ((ls)->is = (i))
#define VSC_IS_ListScheduling_GetDag(ls)            ((ls)->is->curr_dep_dag)
#define VSC_IS_ListScheduling_GetCands(ls)          ((VSC_HASH_TABLE*)&((ls)->cands))
#define VSC_IS_ListScheduling_GetMcands(ls)         ((VSC_HASH_TABLE*)&((ls)->mcands))
#define VSC_IS_ListScheduling_GetEcands(ls)         ((VSC_HASH_TABLE*)&((ls)->ecands))
#define VSC_IS_ListScheduling_GetFwHeur(ls)         ((VSC_IS_FW_Heuristic*)&((ls)->fw_heur))
#define VSC_IS_ListScheduling_GetBwHeur(ls)         ((VSC_IS_BW_Heuristic*)&((ls)->bw_heur))
#define VSC_IS_ListScheduling_GetCurTime(ls)        ((ls)->cur_time)
#define VSC_IS_ListScheduling_SetCurTime(ls, t)     ((ls)->cur_time = (t))
#define VSC_IS_ListScheduling_GetExecTime(ls)       (&((ls)->is->exec_time))
#define VSC_IS_ListScheduling_GetOptions(ls)        ((ls)->is->options)
#define VSC_IS_ListScheduling_GetDumper(ls)         ((ls)->is->dumper)
#define VSC_IS_ListScheduling_GetMM(ls)             ((ls)->is->pMM)

static void _VSC_IS_FW_ListScheduling_Init(
    IN OUT VSC_IS_ListScheduling* ls,
    IN VSC_IS_InstSched* is
    )
{
    VSC_SIMPLE_RESIZABLE_ARRAY* root_array;
    gctUINT32 i;

    VSC_IS_ListScheduling_SetIS(ls, is);
    vscHTBL_Initialize(VSC_IS_ListScheduling_GetCands(ls), VSC_IS_ListScheduling_GetMM(ls), vscHFUNC_Default, vscHKCMP_Default, 512);
    _VSC_IS_FW_Heuristic_Init(VSC_IS_ListScheduling_GetFwHeur(ls), is);

    root_array = DG_GET_ROOT_ARRAY_P(VSC_IS_DepDag_GetDGraph(VSC_IS_ListScheduling_GetDag(ls)));
    for(i = 0; i < vscSRARR_GetElementCount(root_array); i++)
    {
        VSC_IS_DepDagNode** node = (VSC_IS_DepDagNode**)vscSRARR_GetElement(root_array, i);
        vscHTBL_DirectSet(VSC_IS_ListScheduling_GetCands(ls), *node, gcvNULL);
    }
}

static void _VSC_IS_BW_ListScheduling_Init(
    IN OUT VSC_IS_ListScheduling* ls,
    IN VSC_IS_InstSched* is
    )
{
    VSC_SIMPLE_RESIZABLE_ARRAY* tail_array;
    gctUINT32 i;

    VSC_IS_ListScheduling_SetIS(ls, is);
    vscHTBL_Initialize(VSC_IS_ListScheduling_GetCands(ls), VSC_IS_ListScheduling_GetMM(ls), vscHFUNC_Default, vscHKCMP_Default, 512);
    _VSC_IS_BW_Heuristic_Init(VSC_IS_ListScheduling_GetBwHeur(ls), is);

    tail_array = DG_GET_TAIL_ARRAY_P(VSC_IS_DepDag_GetDGraph(VSC_IS_ListScheduling_GetDag(ls)));
    for(i = 0; i < vscSRARR_GetElementCount(tail_array); i++)
    {
        VSC_IS_DepDagNode** node = (VSC_IS_DepDagNode**)vscSRARR_GetElement(tail_array, i);
        vscHTBL_DirectSet(VSC_IS_ListScheduling_GetCands(ls), *node, gcvNULL);
    }
}

static void _VSC_IS_FW_ListScheduling_Final(
    IN OUT VSC_IS_ListScheduling* ls
    )
{
    _VSC_IS_FW_Heuristic_Final(VSC_IS_ListScheduling_GetFwHeur(ls));
}

static void _VSC_IS_BW_ListScheduling_Final(
    IN OUT VSC_IS_ListScheduling* ls
    )
{
    _VSC_IS_BW_Heuristic_Final(VSC_IS_ListScheduling_GetBwHeur(ls));
}

static VSC_ErrCode _VSC_IS_DoListScheduling(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode            err_code  = VSC_ERR_NONE;

    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);

    VIR_Instruction* start = gcvNULL;
    VIR_Instruction* end = gcvNULL;
    VIR_Instruction *prev, *next;
    gctUINT scheduled_count = 0;

    _VSC_IS_GetBBEssence(bb, &prev, &next);
    prev = VIR_Inst_GetPrev(prev);
    next = VIR_Inst_GetNext(next);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling: start list scheduling for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, bb->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    /* start list scheduling */
    if(VSC_OPTN_ISOptions_GetIsForward(options))
    {
        VSC_IS_ListScheduling ls;
        VSC_IS_FW_Heuristic* heur = VSC_IS_ListScheduling_GetFwHeur(&ls);
        VIR_Instruction* scheduled_tail = gcvNULL;

        /* initializations */
        _VSC_IS_FW_ListScheduling_Init(&ls, is);


        /* scheduling */
        while(gcvTRUE)
        {
            VSC_IS_DepDagNode* selected_node;

            /* dump candidates */
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
            {
                VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
                VIR_LOG(dumper, "candidates:\n");
                VIR_LOG_FLUSH(dumper);
                _VSC_IS_DumpInstSet(VSC_IS_ListScheduling_GetCands(&ls), dumper);
            }

            /* do heuristics */
            VSC_IS_Heuristic_SetInSet(heur, VSC_IS_ListScheduling_GetCands(&ls));
            selected_node = _VSC_IS_FW_DoHeuristics(heur);

            /* dump selected node */
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
            {
                VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
                VIR_LOG(dumper, "selected node:\n");
                VIR_LOG_FLUSH(dumper);
                _VSC_IS_DepDagNode_Dump(selected_node, dumper);
            }

            /* enqueue selected node */
            if(scheduled_tail == gcvNULL)
            {
                scheduled_tail = VSC_IS_DepDagNode_GetInst(selected_node);
                start = scheduled_tail;
                VIR_Inst_SetPrev(start, prev);
                if(prev)
                {
                    VIR_Inst_SetNext(prev, start);
                }
            }
            else
            {
                _VSC_IS_FW_PlaceInstruction(&scheduled_tail, VSC_IS_DepDagNode_GetInst(selected_node));
            }

            ++scheduled_count;
            /* break if all instructions scheduled */
            if(scheduled_count
               == vscDG_GetNodeCount(VSC_IS_DepDag_GetDGraph(VSC_IS_ListScheduling_GetDag(&ls))))
            {
                break;
            }

            /* update cands */
            vscHTBL_DirectRemove(VSC_IS_ListScheduling_GetCands(&ls), selected_node);
            {
                VSC_ADJACENT_LIST* succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(selected_node);
                VSC_UL_ITERATOR edge_iter;
                VSC_IS_DepDagEdge* succ_edge;
                VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter, succ_edge_list);
                for(succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter);
                    succ_edge != gcvNULL;   /*??*/
                    succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter))
                {
                    VSC_IS_DepDagNode* succ = VSC_IS_DepDagEdge_GetToNode(succ_edge);
                    VSC_ADJACENT_LIST* succ_pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(succ);
                    VSC_UL_ITERATOR succ_edge_iter;
                    VSC_IS_DepDagEdge* succ_pred_edge;
                    VSC_ADJACENT_LIST_ITERATOR_INIT(&succ_edge_iter, succ_pred_edge_list);

                    for(succ_pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succ_edge_iter);
                        succ_pred_edge != gcvNULL;   /*??*/
                        succ_pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succ_edge_iter))
                    {
                        VSC_IS_DepDagNode* succ_pred = VSC_IS_DepDagEdge_GetToNode(succ_pred_edge);
                        if(!VSC_IS_DepDagNode_IsScheduled(succ_pred))
                        {
                            break;
                        }
                    }
                    if(succ_pred_edge == gcvNULL)
                    {
                        vscHTBL_DirectSet(VSC_IS_ListScheduling_GetCands(&ls), succ, gcvNULL);
                    }
                }
            }
        }

        /* update BB */
        gcmASSERT(scheduled_tail);
        VIR_Inst_SetNext(scheduled_tail, next);
        if(next)
        {
            VIR_Inst_SetPrev(next, scheduled_tail);
        }

        if(!VIR_OPCODE_isBBPrefix(VIR_Inst_GetOpcode(BB_GET_START_INST(bb))))
        {
            BB_SET_START_INST(bb, start);
        }
        if(!VIR_OPCODE_isBBSuffix(VIR_Inst_GetOpcode(BB_GET_END_INST(bb))))
        {
            BB_SET_END_INST(bb, scheduled_tail);
        }

        _VSC_IS_FW_ListScheduling_Final(&ls);
    }   /* end list scheduling */
    else
    {
        VSC_IS_ListScheduling ls;
        VSC_IS_BW_Heuristic* heur = VSC_IS_ListScheduling_GetBwHeur(&ls);
        VIR_Instruction* scheduled_head = gcvNULL;

        /* initializations */
        _VSC_IS_BW_ListScheduling_Init(&ls, is);


        /* scheduling */
        while(gcvTRUE)
        {
            VSC_IS_DepDagNode* selected_node;

            /* dump candidates */
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
            {
                VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
                VIR_LOG(dumper, "candidates:\n");
                VIR_LOG_FLUSH(dumper);
                _VSC_IS_DumpInstSet(VSC_IS_ListScheduling_GetCands(&ls), dumper);
            }

            /* do heuristics */
            VSC_IS_Heuristic_SetInSet(heur, VSC_IS_ListScheduling_GetCands(&ls));
            selected_node = _VSC_IS_BW_DoHeuristics(heur);

            /* dump selected node */
            if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_HEURISTIC))
            {
                VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
                VIR_LOG(dumper, "selected node:\n");
                VIR_LOG_FLUSH(dumper);
                _VSC_IS_DepDagNode_Dump(selected_node, dumper);
            }

            /* enqueue selected node */
            if(scheduled_head == gcvNULL)
            {
                scheduled_head = VSC_IS_DepDagNode_GetInst(selected_node);
                end = scheduled_head;
                VIR_Inst_SetNext(end, next);
                if(next)
                {
                    VIR_Inst_SetPrev(next, end);
                }
            }
            else
            {
                _VSC_IS_BW_PlaceInstruction(&scheduled_head, VSC_IS_DepDagNode_GetInst(selected_node));
            }

            ++scheduled_count;
            /* break if all instructions scheduled */
            if(scheduled_count
               == vscDG_GetNodeCount(VSC_IS_DepDag_GetDGraph(VSC_IS_ListScheduling_GetDag(&ls))))
            {
                break;
            }

            /* update cands */
            vscHTBL_DirectRemove(VSC_IS_ListScheduling_GetCands(&ls), selected_node);
            {
                VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(selected_node);
                VSC_UL_ITERATOR edge_iter;
                VSC_IS_DepDagEdge* pred_edge;
                VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter, pred_edge_list);
                for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter);
                    pred_edge != gcvNULL;   /*??*/
                    pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter))
                {
                    VSC_IS_DepDagNode* pred = VSC_IS_DepDagEdge_GetToNode(pred_edge);
                    VSC_ADJACENT_LIST* pred_succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(pred);
                    VSC_UL_ITERATOR pred_succ_edge_iter;
                    VSC_IS_DepDagEdge* pred_succ_edge;
                    VSC_ADJACENT_LIST_ITERATOR_INIT(&pred_succ_edge_iter, pred_succ_edge_list);

                    for(pred_succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&pred_succ_edge_iter);
                        pred_succ_edge != gcvNULL;   /*??*/
                        pred_succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&pred_succ_edge_iter))
                    {
                        VSC_IS_DepDagNode* pred_succ = VSC_IS_DepDagEdge_GetToNode(pred_succ_edge);
                        if(!VSC_IS_DepDagNode_IsScheduled(pred_succ))
                        {
                            break;
                        }
                    }
                    if(pred_succ_edge == gcvNULL)
                    {
                        vscHTBL_DirectSet(VSC_IS_ListScheduling_GetCands(&ls), pred, gcvNULL);
                    }
                }
            }
        }

        /* update BB */
        gcmASSERT(scheduled_head);
        VIR_Inst_SetPrev(scheduled_head, prev);
        if(prev)
        {
            VIR_Inst_SetNext(prev, scheduled_head);
        }

        if(!VIR_OPCODE_isBBPrefix(VIR_Inst_GetOpcode(BB_GET_START_INST(bb))))
        {
            BB_SET_START_INST(bb, scheduled_head);
        }
        if(!VIR_OPCODE_isBBSuffix(VIR_Inst_GetOpcode(BB_GET_END_INST(bb))))
        {
            BB_SET_END_INST(bb, end);
        }

        _VSC_IS_BW_ListScheduling_Final(&ls);
    }   /* end list scheduling */

    return err_code;
}

/********************************************** List Scheduling Ends **********************************************/

/********************************************** Bubble Scheduling Starts **********************************************/

static VSC_IS_DepDagNode* _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(
    IN VSC_IS_DepDagNode* node,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    OUT VSC_IS_DepDagEdge** out_edge
    )
{
    VSC_ADJACENT_LIST* edge_list = succ ? VSC_IS_DepDagNode_GetSuccEdgeList(node) : VSC_IS_DepDagNode_GetPredEdgeList(node);
    VSC_UL_ITERATOR edge_iter;
    VSC_IS_DepDagEdge* edge;
    VSC_IS_DepDagEdge* result_edge = gcvNULL;
    gctUINT32 edge_count = 0;

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter, edge_list);
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter);
        edge != gcvNULL;   /*??*/
        edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter))
    {
        if(edges_bv)
        {
            if(vscBV_TestBit(edges_bv, VSC_IS_DepDagEdge_GetID(edge)))
            {
                result_edge = edge;
                ++edge_count;
            }
        }
        else
        {
            result_edge = edge;
            ++edge_count;
        }
    }

    if(edge_count == 1)
    {
        if(out_edge)
        {
            *out_edge = result_edge;
        }
        return VSC_IS_DepDagEdge_GetToNode(result_edge);
    }
    else
    {
        if(out_edge)
        {
            *out_edge = gcvNULL;
        }
        return gcvNULL;
    }
}

static VSC_IS_DepDagNode* _VSC_IS_DepDagNode_GetNodeOnList(
    IN VSC_IS_DepDagNode* start,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    IN gctUINT32 maxDistance,
    IN gctBOOL no_bubble,
    IN gctUINT32 kill_bubble_bound,
    IN gctUINT32 maxInDegree,
    IN gctUINT32 minID,
    IN gctUINT32 maxID,
    IN VSC_IS_DepDagNode* end,
    OUT gctUINT32* final_distance, /* node distance */
    OUT gctUINT32* bubble_sum, /* bubble sum on the segment */
    OUT VSC_IS_DepDagEdge** last_edge   /* last visited edge on the segment */
    )
{
    if(start == end || maxDistance == 0)
    {
        if(final_distance)
        {
            *final_distance = 0;
        }
        if(bubble_sum)
        {
            *bubble_sum = 0;
        }
        if(last_edge)
        {
            *last_edge = gcvNULL;
        }
        return start;
    }
    else
    {
        VSC_IS_DepDagNode* iter = start;
        VSC_IS_DepDagEdge* prev_edge = gcvNULL;
        gctUINT32 dist = 0;
        gctUINT32 bubble_sum_l = 0;

        while(iter != end)
        {
            VSC_IS_DepDagEdge* edge;

            if(VSC_IS_DepDagNode_GetInDegree(iter) > maxInDegree ||
               VSC_IS_DepDagNode_GetID(iter) < minID ||
               VSC_IS_DepDagNode_GetID(iter) > maxID)
            {
                if(final_distance)
                {
                    *final_distance = dist;
                }
                if(bubble_sum)
                {
                    *bubble_sum = bubble_sum_l;
                }
                if(last_edge)
                {
                    *last_edge = prev_edge;
                }
                return iter;
            }
            if(_VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(iter, succ, edges_bv, &edge))
            {
                if(no_bubble && VSC_IS_DepDagEdge_GetBubble(edge))
                {
                    if(final_distance)
                    {
                        *final_distance = dist;
                    }
                    if(bubble_sum)
                    {
                        *bubble_sum = 0;
                    }
                    if(last_edge)
                    {
                        *last_edge = prev_edge;
                    }
                    return iter;
                }
                bubble_sum_l += VSC_IS_DepDagEdge_GetBubble(edge);
                ++dist;
                iter = VSC_IS_DepDagEdge_GetToNode(edge);
                if(dist == maxDistance ||
                   (succ && VSC_IS_DepDagNode_GetKillPriority(iter) < kill_bubble_bound) ||
                   (!succ && VSC_IS_DepDagNode_GetKillPriority(iter) > kill_bubble_bound))
                {
                    if(final_distance)
                    {
                        *final_distance = dist;
                    }
                    if(bubble_sum)
                    {
                        *bubble_sum = bubble_sum_l;
                    }
                    if(last_edge)
                    {
                        *last_edge = edge;
                    }
                    return iter;
                }
                prev_edge = edge;
            }
            else
            {
                if(final_distance)
                {
                    *final_distance = dist;
                }
                if(bubble_sum)
                {
                    *bubble_sum = bubble_sum_l;
                }
                if(last_edge)
                {
                    *last_edge = prev_edge;
                }
                return iter;
            }
        }
        if(final_distance)
        {
            *final_distance = dist;
        }
        if(bubble_sum)
        {
            *bubble_sum = bubble_sum_l;
        }
        if(last_edge)
        {
            *last_edge = prev_edge;
        }
        return iter;
    }
}

#define DEBUG_FINDING_DETOUR 0
static gctBOOL _VSC_IS_FindDetourTopNode(/* if node is excluded, return true, otherwise return false */
    IN VSC_IS_DepDagNode* node,
    IN VSC_BIT_VECTOR* sub_tree_edges_bv,
    IN gctUINT maxTopOutDegree,
    IN gctUINT maxByPassOutDegree,
    IN gctBOOL succExcluded,
    IN OUT VSC_BIT_VECTOR* visited_sub_tree_edges_bv,
    IN OUT VSC_IS_DepDagNode** head,
    IN OUT VSC_IS_DepDagNode** tail
    )
{
    gctBOOL selfExcluded = gcvFALSE;

    if(!succExcluded)
    {
        VSC_ADJACENT_LIST* succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(node);
        VSC_UL_ITERATOR succ_edge_iter;
        VSC_IS_DepDagEdge* succ_edge;
        gctUINT32 succ_edge_count = 0;

        gcmASSERT(head && tail);

        VSC_ADJACENT_LIST_ITERATOR_INIT(&succ_edge_iter, succ_edge_list);
        for(succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succ_edge_iter);
            succ_edge != gcvNULL;   /*??*/
            succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succ_edge_iter))
        {
            if(vscBV_TestBit(sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(succ_edge)))
            {
                if(!vscBV_TestBit(visited_sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(succ_edge)))
                {
                    /* one succ edge has not been visited, so wait */
                    return gcvFALSE;
                }
                succ_edge_count++;
            }
        }

        if(succ_edge_count == 0 ||
           succ_edge_count > maxTopOutDegree)
        {
            selfExcluded = gcvTRUE;
#if DEBUG_FINDING_DETOUR
            gcoOS_Print("as a top node, node %d has succ edge count %d and is excluded .\n", VSC_IS_DepDagNode_GetID(node), succ_edge_count);
#endif
        }

        /* all succ edge in the sub-tree visited */
        if(!selfExcluded && succ_edge_count >= 2)
        {
#if DEBUG_FINDING_DETOUR
            gcoOS_Print("node %d is added as detour head.\n", VSC_IS_DepDagNode_GetID(node));
#endif
            if(*head)
            {
                gcmASSERT(*tail);

                VSC_IS_DepDagNode_SetNext(*tail, node);
                *tail = node;
            }
            else
            {
                gcmASSERT(!*tail);

                *head = node;
                *tail = node;
            }
            VSC_IS_DepDagNode_SetNext(node, gcvNULL);
        }

        if(VSC_IS_DepDagNode_GetOutDegree(node) > maxByPassOutDegree)
        {
            selfExcluded = gcvTRUE;
#if DEBUG_FINDING_DETOUR
            gcoOS_Print("as a bypass node, node %d has outdegree %d and is excluded.\n", VSC_IS_DepDagNode_GetID(node), VSC_IS_DepDagNode_GetOutDegree(node));
#endif
        }
    }

    {
        VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(node);
        VSC_UL_ITERATOR pred_edge_iter;
        VSC_IS_DepDagEdge* pred_edge;
        gctBOOL predExcluded = gcvFALSE;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&pred_edge_iter, pred_edge_list);
        for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&pred_edge_iter);
            pred_edge != gcvNULL;   /*??*/
            pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&pred_edge_iter))
        {
            if(vscBV_TestBit(sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge)))
            {
                VSC_IS_DepDagNode* pred_edge_to = VSC_IS_DepDagEdge_GetToNode(pred_edge);
                gctBOOL result;

                if(selfExcluded || succExcluded)
                {
#if DEBUG_FINDING_DETOUR
                    gcoOS_Print("remove sub-tree edge from %d to %d.\n", VSC_IS_DepDagNode_GetID(node), VSC_IS_DepDagNode_GetID(pred_edge_to));
#endif
                    vscBV_ClearBit(sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge));
                }
                else
                {
                    vscBV_SetBit(visited_sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge));
                }
                result = _VSC_IS_FindDetourTopNode(pred_edge_to, sub_tree_edges_bv, maxTopOutDegree, maxByPassOutDegree, selfExcluded || succExcluded, visited_sub_tree_edges_bv, head, tail);
                predExcluded = predExcluded || result;
            }
        }

        return predExcluded || selfExcluded;
    }
}

static gctBOOL _VSC_IS_FindDetourTopNodes(/* if node is excluded, return true, otherwise return false */
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* sub_root,
    IN VSC_BIT_VECTOR* sub_tree_edges_bv,
    IN gctUINT maxDetourInDegree,
    IN gctUINT maxByPassOutDegree,
    OUT VSC_IS_DepDagNode** head,
    OUT VSC_IS_DepDagNode** tail
    )
{
    VSC_BIT_VECTOR* visited_sub_tree_edges_bv = _VSC_IS_DepDag_RentAEdgesBV(dag);
    VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(sub_root);
    VSC_UL_ITERATOR pred_edge_iter;
    VSC_IS_DepDagEdge* pred_edge;
    gctBOOL hasExcluded = gcvFALSE;

#if DEBUG_FINDING_DETOUR
    gcoOS_Print("sub root: %d\n", VSC_IS_DepDagNode_GetID(sub_root));
#endif

    VSC_ADJACENT_LIST_ITERATOR_INIT(&pred_edge_iter, pred_edge_list);
    for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&pred_edge_iter);
        pred_edge != gcvNULL;   /*??*/
        pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&pred_edge_iter))
    {
        if(vscBV_TestBit(sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge)))
        {
            VSC_IS_DepDagNode* pred_edge_to = VSC_IS_DepDagEdge_GetToNode(pred_edge);
            gctBOOL result;

            vscBV_SetBit(visited_sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge));
            result = _VSC_IS_FindDetourTopNode(pred_edge_to, sub_tree_edges_bv, maxDetourInDegree, maxByPassOutDegree, gcvFALSE, visited_sub_tree_edges_bv, head, tail);
            hasExcluded = hasExcluded || result;
        }
    }

    _VSC_IS_DepDag_ReturnAEdgesBV(dag, visited_sub_tree_edges_bv);
    return hasExcluded;
}

static VSC_IS_DepDagNode* _VSC_IS_MergePredsOrderly(
    VSC_IS_DepDag* dag,
    VSC_IS_DepDagNode* rootNode,
    VSC_IS_DepDagNode* startNode0,
    VSC_IS_DepDagNode* startNode1,
    VSC_IS_DepDagNode* endNode0,
    VSC_IS_DepDagNode* endNode1,
    VSC_BIT_VECTOR* pathEdges,
    OUT VSC_IS_DepDagEdge** endNode0PredEdge,
    OUT VSC_IS_DepDagEdge** endNode1PredEdge
    )
{
    VSC_IS_DepDagEdge* newEdge;
    VSC_IS_DepDagNode* currNode0 = startNode0;
    VSC_IS_DepDagNode* currNode1 = startNode1;

    gcmASSERT(rootNode && startNode0 && startNode1 && endNode0 && endNode1);
    if(VSC_IS_DepDagNode_GetID(startNode0) < VSC_IS_DepDagNode_GetID(startNode1))
    {
        VSC_IS_DepDagEdge* currNode1Edge = gcvNULL;

        while(currNode1 != endNode1 && VSC_IS_DepDagNode_GetID(currNode1) > VSC_IS_DepDagNode_GetID(startNode0))
        {
            currNode1 = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(currNode1, gcvFALSE, pathEdges, &currNode1Edge);
        }

        if(currNode1 == endNode1 && VSC_IS_DepDagNode_GetID(currNode1) > VSC_IS_DepDagNode_GetID(startNode0))
        {
            newEdge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, startNode0, rootNode, endNode1);
            if(endNode1PredEdge)
            {
                *endNode1PredEdge = newEdge + 1;
            }
            return endNode0;
        }
        else
        {
            VSC_IS_DepDagNode* newRoot = VSC_IS_DepDagEdge_GetFromNode(currNode1Edge);

            newEdge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, startNode0, rootNode, newRoot);
            return _VSC_IS_MergePredsOrderly(dag, newRoot, startNode0, currNode1, endNode0, endNode1, pathEdges, endNode0PredEdge, endNode1PredEdge);
        }
    }
    else
    {
        VSC_IS_DepDagEdge* currNode0Edge = gcvNULL;

        while(currNode0 != endNode0 && VSC_IS_DepDagNode_GetID(currNode0) > VSC_IS_DepDagNode_GetID(startNode1))
        {
            currNode0 = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(currNode0, gcvFALSE, pathEdges, &currNode0Edge);
        }

        if(currNode0 == endNode0 && VSC_IS_DepDagNode_GetID(currNode0) > VSC_IS_DepDagNode_GetID(startNode1))
        {
            newEdge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, startNode1, rootNode, endNode0);
            if(endNode0PredEdge)
            {
                *endNode0PredEdge = newEdge + 1;
            }
            return endNode1;
        }
        else
        {
            VSC_IS_DepDagNode* newRoot = VSC_IS_DepDagEdge_GetFromNode(currNode0Edge);

            newEdge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, startNode1, rootNode, newRoot);
            return _VSC_IS_MergePredsOrderly(dag, newRoot, currNode0, startNode1, endNode0, endNode1, pathEdges, endNode0PredEdge, endNode1PredEdge);
        }
    }
}


static VSC_ErrCode _VSC_IS_MergeDetour(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* top_node,
    IN OUT VSC_IS_DepDagEdge* edge0,
    IN OUT VSC_IS_DepDagEdge* edge1,
    IN OUT VSC_IS_DepDagNode* bottom_node,
    IN OUT VSC_BIT_VECTOR* detour_edges_bv,
    IN gctBOOL recursive_call
    )
{
    VSC_ErrCode err_code = VSC_ERR_NONE;
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);
    VSC_IS_DepDagNode* node0 = VSC_IS_DepDagEdge_GetToNode(edge0);
    VSC_IS_DepDagNode* node1 = VSC_IS_DepDagEdge_GetToNode(edge1);
    gctUINT32 bubble0 = VSC_IS_DepDagEdge_GetBubble(edge0);
    gctUINT32 bubble1 = VSC_IS_DepDagEdge_GetBubble(edge1);
    VSC_IS_DepDagEdge* new_edge;

    gcmASSERT(bottom_node);
    gcmASSERT(edge0);
    gcmASSERT(edge1);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED) &&
       (VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED_INCLUDE_RECUR) || !recursive_call))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);

        VIR_LOG(dumper, "before merge detour:\n");

        VIR_LOG(dumper, "tour0:\n");
        _VSC_IS_DepDagNode_Dump(bottom_node, dumper);
        _VSC_IS_DepDagEgde_Dump(edge0, dumper);
        _VSC_IS_DepDagNode_DumpList(node0, top_node, gcvFALSE, detour_edges_bv, dumper);

        VIR_LOG(dumper, "tour1:\n");
        _VSC_IS_DepDagNode_Dump(bottom_node, dumper);
        _VSC_IS_DepDagEgde_Dump(edge1, dumper);
        _VSC_IS_DepDagNode_DumpList(node1, top_node, gcvFALSE, detour_edges_bv, dumper);
    }

    if(node0 == top_node)
    {
        _VSC_IS_DepDag_RemoveEdge(dag, top_node, bottom_node);
        if(bubble0)
        {
            gctUINT32 final_distance = 0;
            gctUINT32 bubble_sum = 0;
            gctUINT32 total_len;
            _VSC_IS_DepDagNode_GetNodeOnList(node1, gcvFALSE, detour_edges_bv, gcvMAXUINT32, gcvFALSE, gcvMAXUINT32, gcvMAXUINT32, 0, gcvMAXUINT32, top_node, &final_distance, &bubble_sum, gcvNULL);
            total_len = final_distance + bubble_sum + bubble1;
            if(bubble0 > total_len)
            {
                VSC_IS_DepDagEdge* endEdge1;
                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(top_node, gcvTRUE, detour_edges_bv, &endEdge1);
                VSC_IS_DepDagEdge_IncBubble(edge0 - 1, bubble0 - total_len);
            }
        }
    }
    else if(node1 == top_node)
    {
        _VSC_IS_DepDag_RemoveEdge(dag, top_node, bottom_node);
        if(bubble1)
        {
            gctUINT32 final_distance = 0;
            gctUINT32 bubble_sum = 0;
            gctUINT32 total_len;

            _VSC_IS_DepDagNode_GetNodeOnList(node0, gcvFALSE, detour_edges_bv, gcvMAXUINT32, gcvFALSE, gcvMAXUINT32, gcvMAXUINT32, 0, gcvMAXUINT32, top_node, &final_distance, &bubble_sum, gcvNULL);
            total_len = final_distance + bubble_sum + bubble0;
            if(bubble1 > total_len)
            {
                VSC_IS_DepDagEdge* endEdge0;
                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(top_node, gcvTRUE, detour_edges_bv, &endEdge0);
                VSC_IS_DepDagEdge_IncBubble(endEdge0, bubble1 - total_len);
            }
        }
    }
    else
    {
        /* handle kill priority nodes */
        if(VSC_IS_DepDagNode_GetKillPriority(node0) > VSC_IS_DepDagNode_GetKillPriority(node1))
        {
            VSC_IS_DepDagEdge* node1_prev_edge = gcvNULL;

            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, detour_edges_bv, &node1_prev_edge);
            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, node1);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 > bubble1 ? bubble0 - bubble1 - 1 : 0);
            err_code = _VSC_IS_MergeDetour(is, top_node, node1_prev_edge, new_edge + 1, node1, detour_edges_bv, gcvTRUE);
        }
        else if(VSC_IS_DepDagNode_GetKillPriority(node1) > VSC_IS_DepDagNode_GetKillPriority(node0))
        {
            VSC_IS_DepDagEdge* node0_prev_edge = gcvNULL;

            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, detour_edges_bv, &node0_prev_edge);
            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, node0);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 < bubble1 ? bubble1 - bubble0 - 1 : 0);
            err_code = _VSC_IS_MergeDetour(is, top_node, node0_prev_edge, new_edge + 1, node0, detour_edges_bv, gcvTRUE);
        }
        /* kill priorities equal here. handle indegree nodes */
        else if(VSC_IS_DepDagNode_GetInDegree(node0) < VSC_IS_DepDagNode_GetInDegree(node1) && bubble0 >= bubble1 && VSC_IS_DepDagNode_GetID(node0) < VSC_IS_DepDagNode_GetID(node1))
        {
            VSC_IS_DepDagEdge* node1_prev_edge = gcvNULL;

            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, detour_edges_bv, &node1_prev_edge);
            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, node1);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 > bubble1 ? bubble0 - bubble1 - 1 : 0);
            err_code = _VSC_IS_MergeDetour(is, top_node, node1_prev_edge, new_edge + 1, node1, detour_edges_bv, gcvTRUE);
        }
        else if(VSC_IS_DepDagNode_GetInDegree(node1) < VSC_IS_DepDagNode_GetInDegree(node0) && bubble0 <= bubble1 && VSC_IS_DepDagNode_GetID(node1) < VSC_IS_DepDagNode_GetID(node0))
        {
            VSC_IS_DepDagEdge* node0_prev_edge = gcvNULL;

            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, detour_edges_bv, &node0_prev_edge);
            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, node0);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 < bubble1 ? bubble1 - bubble0 - 1 : 0);
            err_code = _VSC_IS_MergeDetour(is, top_node, node0_prev_edge, new_edge + 1, node0, detour_edges_bv, gcvTRUE);
        }
        /* handle others */
        else if(bubble0 && bubble1)
        {
            if(bubble0 > bubble1)
            {
                VSC_IS_DepDagEdge* node1_pred_edge;

                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, detour_edges_bv, &node1_pred_edge);
                new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, node1);
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - bubble1 - 1);
                gcmASSERT(node1_pred_edge);
                err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node1_pred_edge, node1, detour_edges_bv, gcvTRUE);
            }
            else if(bubble0 < bubble1)
            {
                VSC_IS_DepDagEdge* node0_pred_edge;

                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, detour_edges_bv, &node0_pred_edge);
                new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, node0);
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - bubble0 - 1);
                gcmASSERT(node0_pred_edge);
                err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node0_pred_edge, node0, detour_edges_bv, gcvTRUE);
            }
            else
            {
                /* bubble0 equals to bubble1, schedule node0 and node1 orderly. we may have better way to schedule them here */
                if(VSC_IS_DepDagNode_GetID(node0) < VSC_IS_DepDagNode_GetID(node1))
                {
                    VSC_IS_DepDagEdge* node1_pred_edge;

                    _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, detour_edges_bv, &node1_pred_edge);
                    new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, node1);
                    gcmASSERT(node1_pred_edge);
                    err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node1_pred_edge, node1, detour_edges_bv, gcvTRUE);
                }
                else
                {
                    VSC_IS_DepDagEdge* node0_pred_edge;

                    _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, detour_edges_bv, &node0_pred_edge);
                    new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, node0);
                    gcmASSERT(node0_pred_edge);
                    err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node0_pred_edge, node0, detour_edges_bv, gcvTRUE);
                }
            }
        }
        else
        {
            /* not both edges have bubble */
            if(bubble0)
            {
                VSC_IS_DepDagNode* end_node1;
                gctUINT32 final_distance = 0;
                VSC_IS_DepDagEdge* end_node1_edge = gcvNULL;
                end_node1 = _VSC_IS_DepDagNode_GetNodeOnList(node1,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             bubble0 - 1,
                                                             gcvTRUE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node0),
                                                             gcvMAXUINT32,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             &final_distance,
                                                             gcvNULL,
                                                             &end_node1_edge);
                if(end_node1 == top_node)
                {
                    VSC_IS_DepDagNode* end_node1_edge_from = VSC_IS_DepDagEdge_GetFromNode(end_node1_edge);

                    new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, end_node1_edge_from);
                    _VSC_IS_DepDag_RemoveEdge(dag, top_node, end_node1_edge_from);
                    VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - final_distance - 1);
                }
                else
                {
                    if(VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node0))
                    {
                        VSC_IS_DepDagNode* end_node1_edge_from = VSC_IS_DepDagEdge_GetFromNode(end_node1_edge);
                        VSC_IS_DepDagEdge* node0_prev_edge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, detour_edges_bv, &node0_prev_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, end_node1_edge_from);
                        if(bubble0 > final_distance)
                        {
                            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - final_distance);
                        }
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, end_node1, end_node1_edge_from, node0);
                        err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node0_prev_edge, node0, detour_edges_bv, gcvTRUE);
                    }
                    else
                    {
                        VSC_IS_DepDagEdge* end_node1_bubble_edge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &end_node1_bubble_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, end_node1);
                        VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - final_distance - 1);
                        err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, end_node1_bubble_edge, end_node1, detour_edges_bv, gcvTRUE);
                    }
                }
            }
            else if(bubble1)
            {
                VSC_IS_DepDagNode* end_node0;
                gctUINT32 final_distance = 0;
                VSC_IS_DepDagEdge* end_node0_edge = gcvNULL;
                end_node0 = _VSC_IS_DepDagNode_GetNodeOnList(node0,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             bubble1 - 1,
                                                             gcvTRUE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node1),
                                                             gcvMAXUINT32,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             &final_distance,
                                                             gcvNULL,
                                                             &end_node0_edge);
                if(end_node0 == top_node)
                {
                    VSC_IS_DepDagNode* end_node0_edge_from = VSC_IS_DepDagEdge_GetFromNode(end_node0_edge);

                    new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, end_node0_edge_from);
                    _VSC_IS_DepDag_RemoveEdge(dag, top_node, end_node0_edge_from);
                    VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - final_distance);
                }
                else
                {
                    if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node1))
                    {
                        VSC_IS_DepDagNode* end_node0_edge_from = VSC_IS_DepDagEdge_GetFromNode(end_node0_edge);
                        VSC_IS_DepDagEdge* node1_prev_edge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, detour_edges_bv, &node1_prev_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, end_node0_edge_from);
                        if(bubble1 > final_distance)
                        {
                            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - final_distance);
                        }
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, end_node0, end_node0_edge_from, node1);
                        err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, node1_prev_edge, node1, detour_edges_bv, gcvTRUE);
                    }
                    else
                    {
                        VSC_IS_DepDagEdge* end_node0_bubble_edge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &end_node0_bubble_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, end_node0);
                        VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - final_distance - 1);
                        err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, end_node0_bubble_edge, end_node0, detour_edges_bv, gcvTRUE);
                    }
                }
            }
            else
            {
                /* bubble0 and bubble1 are both zero */
                gctUINT32 end_node0_distance = 0;
                gctUINT32 end_node1_distance = 0;
                VSC_IS_DepDagNode* end_node0;
                VSC_IS_DepDagNode* end_node1;
                VSC_IS_DepDagEdge* prev_edge0 = gcvNULL;
                VSC_IS_DepDagEdge* prev_edge1 = gcvNULL;

                end_node0 = _VSC_IS_DepDagNode_GetNodeOnList(node0,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             gcvMAXUINT32,
                                                             gcvTRUE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node0),
                                                             1,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             &end_node0_distance,
                                                             gcvNULL,
                                                             &prev_edge0);
                end_node1 = _VSC_IS_DepDagNode_GetNodeOnList(node1,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             gcvMAXUINT32,
                                                             gcvTRUE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node1),
                                                             1,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             &end_node1_distance,
                                                             gcvNULL,
                                                             &prev_edge1);

                if(end_node0 == top_node && end_node1 == top_node)
                {
                    VSC_IS_DepDagNode* endNode0PredNode = VSC_IS_DepDagEdge_GetFromNode(prev_edge0);
                    VSC_IS_DepDagNode* endNode1PredNode = VSC_IS_DepDagEdge_GetFromNode(prev_edge1);
                    VSC_IS_DepDagNode* tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, endNode0PredNode, endNode1PredNode, detour_edges_bv, gcvNULL, gcvNULL);

                    if(tailNode == endNode0PredNode)
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, top_node, endNode1PredNode);
                    }
                    else
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, top_node, endNode0PredNode);
                    }
                }
                else if(end_node0 == top_node)
                {
                    if(VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
                    {
                        /* the second path has higher priority */
                        VSC_IS_DepDagNode* prev_edge0_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge0);
                        VSC_IS_DepDagNode* prev_edge1_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge1);
                        VSC_IS_DepDagNode* tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, prev_edge0_from, prev_edge1_from, detour_edges_bv, gcvNULL, gcvNULL);

                        _VSC_IS_DepDag_RemoveEdge(dag, top_node, prev_edge0_from);
                        if(tailNode == prev_edge0_from)
                        {
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, end_node1, prev_edge1_from, tailNode);
                        }
                    }
                    else if(VSC_IS_DepDagNode_GetInDegree(end_node1) > 1)
                    {
                        /* the second path has a high indegree node */
                        VSC_IS_DepDagNode* endNode0PrevNode = VSC_IS_DepDagEdge_GetFromNode(prev_edge0);
                        VSC_IS_DepDagEdge* endNode1PredEdge;
                        VSC_IS_DepDagNode* tailNode;
                        VSC_IS_DepDagEdge* newEndNode1PredEdge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &endNode1PredEdge);
                        tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, endNode0PrevNode, end_node1, detour_edges_bv, gcvNULL, &newEndNode1PredEdge);

                        if(tailNode == endNode0PrevNode)
                        {
                            err_code = _VSC_IS_MergeDetour(is, top_node, newEndNode1PredEdge, endNode1PredEdge, end_node1, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            _VSC_IS_DepDag_RemoveEdge(dag, top_node, endNode0PrevNode);
                        }
                    }
                    else
                    {
                        /* the second path has a bubble */
                        gctUINT32 bubbleSum;

                        _VSC_IS_DepDagNode_GetNodeOnList(end_node1,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             gcvMAXUINT32,
                                                             gcvFALSE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node1),
                                                             1,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             gcvNULL,
                                                             &bubbleSum,
                                                             gcvNULL);

                        if(bubbleSum >= end_node0_distance)
                        {
                            /* all nodes on the first path need to be used to fill bubbles on the second path */
                            VSC_IS_DepDagEdge* nextDetourEdge = gcvNULL;
                            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &nextDetourEdge);

                            gcmASSERT(nextDetourEdge);
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, end_node1);
                            err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, nextDetourEdge, end_node1, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            /* use some upper nodes on the first path to fill bubbles on the second path */
                            VSC_IS_DepDagNode* newNode0;
                            VSC_IS_DepDagEdge* prevNewNode0Edge;
                            VSC_IS_DepDagNode* prevNewNode0Node;
                            VSC_IS_DepDagEdge* nextDetourEdge;
                            VSC_IS_DepDagNode* tailNode;
                            VSC_IS_DepDagEdge* newEndNode1SuccEdge;

                            newNode0 = _VSC_IS_DepDagNode_GetNodeOnList(node0,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             end_node0_distance - bubbleSum,
                                                             gcvFALSE,
                                                             gcvMAXUINT32,
                                                             gcvMAXUINT32,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             gcvNULL,
                                                             gcvNULL,
                                                             &prevNewNode0Edge);

                            /* Orderly merge the lower part */
                            prevNewNode0Node = VSC_IS_DepDagEdge_GetFromNode(prevNewNode0Edge);
                            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &nextDetourEdge);
                            tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, prevNewNode0Node, end_node1, detour_edges_bv, gcvNULL, &newEndNode1SuccEdge);

                            /* recursively merge the upper detour */
                            if(tailNode == prevNewNode0Node)
                            {
                                err_code = _VSC_IS_MergeDetour(is, top_node, newEndNode1SuccEdge, nextDetourEdge, end_node1, detour_edges_bv, gcvTRUE);
                            }
                            else
                            {
                                new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, newNode0, prevNewNode0Node, end_node1);
                                err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, nextDetourEdge, end_node1, detour_edges_bv, gcvTRUE);
                            }
                        }
                    }
                }
                else if(end_node1 == top_node)
                {
                    if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0))
                    {
                        /* the first path has higher priority */
                        VSC_IS_DepDagNode* prev_edge0_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge0);
                        VSC_IS_DepDagNode* prev_edge1_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge1);
                        VSC_IS_DepDagNode* tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, prev_edge0_from, prev_edge1_from, detour_edges_bv, gcvNULL, gcvNULL);

                        _VSC_IS_DepDag_RemoveEdge(dag, top_node, prev_edge1_from);
                        if(tailNode == prev_edge1_from)
                        {
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, end_node0, prev_edge0_from, tailNode);
                        }
                    }
                    else if(VSC_IS_DepDagNode_GetInDegree(end_node0) > 1)
                    {
                        /* the first path has a high indegree node */
                        VSC_IS_DepDagNode* endNode1PrevNode = VSC_IS_DepDagEdge_GetFromNode(prev_edge1);
                        VSC_IS_DepDagEdge* endNode0SuccEdge;
                        VSC_IS_DepDagNode* tailNode;
                        VSC_IS_DepDagEdge* newEndNode0PredEdge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &endNode0SuccEdge);
                        tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, end_node0, endNode1PrevNode, detour_edges_bv, &newEndNode0PredEdge, gcvNULL);

                        if(tailNode == endNode1PrevNode)
                        {
                            err_code = _VSC_IS_MergeDetour(is, top_node, newEndNode0PredEdge, endNode0SuccEdge, end_node0, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            _VSC_IS_DepDag_RemoveEdge(dag, top_node, endNode1PrevNode);
                        }
                    }
                    else
                    {
                        /* the first path has a bubble */
                        gctUINT32 bubbleSum;

                        _VSC_IS_DepDagNode_GetNodeOnList(end_node0,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             gcvMAXUINT32,
                                                             gcvFALSE,
                                                             VSC_IS_DepDagNode_GetKillPriority(node0),
                                                             1,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             gcvNULL,
                                                             &bubbleSum,
                                                             gcvNULL);

                        if(bubbleSum >= end_node1_distance)
                        {
                            /* all nodes on the first path need to be used to fill bubbles on the second path */
                            VSC_IS_DepDagEdge* nextDetourEdge = gcvNULL;
                            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &nextDetourEdge);

                            gcmASSERT(nextDetourEdge);
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, end_node0);
                            err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, nextDetourEdge, end_node0, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            /* use some upper nodes on the first path to fill bubbles on the second path */
                            VSC_IS_DepDagNode* newNode1;
                            VSC_IS_DepDagEdge* prevNewNode1Edge;
                            VSC_IS_DepDagNode* prevNewNode1Node;
                            VSC_IS_DepDagEdge* nextDetourEdge;
                            VSC_IS_DepDagNode* tailNode;
                            VSC_IS_DepDagEdge* newEndNode0SuccEdge;

                            newNode1 = _VSC_IS_DepDagNode_GetNodeOnList(node1,
                                                             gcvFALSE,
                                                             detour_edges_bv,
                                                             end_node1_distance - bubbleSum,
                                                             gcvFALSE,
                                                             gcvMAXUINT32,
                                                             gcvMAXUINT32,
                                                             0,
                                                             gcvMAXUINT32,
                                                             top_node,
                                                             gcvNULL,
                                                             gcvNULL,
                                                             &prevNewNode1Edge);

                            /* Orderly merge the lower part */
                            prevNewNode1Node = VSC_IS_DepDagEdge_GetFromNode(prevNewNode1Edge);
                            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &nextDetourEdge);
                            tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node1, node0, prevNewNode1Node, end_node0, detour_edges_bv, gcvNULL, &newEndNode0SuccEdge);

                            /* recursively merge the upper detour */
                            if(tailNode == prevNewNode1Node)
                            {
                                err_code = _VSC_IS_MergeDetour(is, top_node, newEndNode0SuccEdge, nextDetourEdge, end_node0, detour_edges_bv, gcvTRUE);
                            }
                            else
                            {
                                new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, newNode1, prevNewNode1Node, end_node0);
                                err_code = _VSC_IS_MergeDetour(is, top_node, new_edge + 1, nextDetourEdge, end_node0, detour_edges_bv, gcvTRUE);
                            }
                        }
                    }
                }
                else
                {
                    /* both stopped by lower_priority/bubble/indegree node */
                    if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0) &&
                       VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
                    {
                        /* both stopped by lower_priority node */
                        VSC_IS_DepDagNode* prev_edge0_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge0);
                        VSC_IS_DepDagNode* prev_edge1_from = VSC_IS_DepDagEdge_GetFromNode(prev_edge1);
                        VSC_IS_DepDagNode* tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, prev_edge0_from, prev_edge1_from, detour_edges_bv, gcvNULL, gcvNULL);

                        if(tailNode == prev_edge0_from)
                        {
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, end_node1, prev_edge1_from, prev_edge0_from);
                            err_code = _VSC_IS_MergeDetour(is, top_node, prev_edge0, new_edge + 1, prev_edge0_from, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, prev_edge0_from, prev_edge1_from);
                            err_code = _VSC_IS_MergeDetour(is, top_node, prev_edge0, new_edge + 1, prev_edge0_from, detour_edges_bv, gcvTRUE);
                        }
                    }
                    else if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0))
                    {
                        /* one stopped by lower priority node, the other stopped by indegree/bubble */
                        VSC_IS_DepDagEdge* bubble1_edge = gcvNULL;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &bubble1_edge);
                        gcmASSERT(bubble1_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node0, bottom_node, end_node1);
                        err_code = _VSC_IS_MergeDetour(is, top_node, bubble1_edge, new_edge + 1, end_node1, detour_edges_bv, gcvTRUE);
                    }
                    else if(VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
                    {
                        /* one stopped by lower priority node, the other stopped by indegree/bubble */
                        VSC_IS_DepDagEdge* bubble0_edge = gcvNULL;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &bubble0_edge);
                        gcmASSERT(bubble0_edge);
                        new_edge = _VSC_IS_DepDag_ReplaceEdgeToNode(dag, node1, bottom_node, end_node0);
                        err_code = _VSC_IS_MergeDetour(is, top_node, bubble0_edge, new_edge + 1, end_node0, detour_edges_bv, gcvTRUE);
                    }
                    else
                    {
                        VSC_IS_DepDagEdge* end_node0_prev_edge;
                        VSC_IS_DepDagEdge* end_node1_prev_edge;
                        VSC_IS_DepDagNode* tailNode;
                        VSC_IS_DepDagEdge* endNode0SuccEdge;
                        VSC_IS_DepDagEdge* endNode1SuccEdge;

                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, detour_edges_bv, &end_node0_prev_edge);
                        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, detour_edges_bv, &end_node1_prev_edge);
                        tailNode = _VSC_IS_MergePredsOrderly(dag, bottom_node, node0, node1, end_node0, end_node1, detour_edges_bv, &endNode0SuccEdge, &endNode1SuccEdge);
                        if(tailNode == end_node0)
                        {
                            err_code = _VSC_IS_MergeDetour(is, top_node, endNode1SuccEdge, end_node1_prev_edge, end_node1, detour_edges_bv, gcvTRUE);
                        }
                        else
                        {
                            err_code = _VSC_IS_MergeDetour(is, top_node, endNode0SuccEdge, end_node0_prev_edge, end_node0, detour_edges_bv, gcvTRUE);
                        }
                    }
                }
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED) &&
       (VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED_INCLUDE_RECUR) || !recursive_call))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        gctUINT bubble_sum;

        VIR_LOG(dumper, "after merge detour:\n");
        VIR_LOG(dumper, "list:\n");
        _VSC_IS_DepDagNode_DumpList(top_node, bottom_node, gcvTRUE, detour_edges_bv, dumper);
        _VSC_IS_DepDagNode_GetNodeOnList(bottom_node, gcvFALSE, detour_edges_bv, gcvMAXUINT32, gcvFALSE, gcvMAXUINT32, gcvMAXUINT32, 0, gcvMAXUINT32, top_node, gcvNULL, &bubble_sum, gcvNULL);
        VIR_LOG(dumper, "bubble_sum: %d\n", bubble_sum);
        VIR_LOG_FLUSH(dumper);
    }

    return err_code;
}


static VSC_IS_DepDagNode* _VSC_IS_FindFromEdges(
    IN VSC_IS_DepDag* dag,
    IN VSC_IS_DepDagNode* top_node,
    IN VSC_IS_DepDagEdge* first_succ_edge,
    IN VSC_IS_DepDagEdge* second_succ_edge,
    IN VSC_IS_DepDagNode* sub_root,
    IN VSC_BIT_VECTOR* sub_tree_edges_bv,
    OUT VSC_IS_DepDagEdge** first_from_edge,
    OUT VSC_IS_DepDagEdge** second_from_edge,
    OUT VSC_BIT_VECTOR* detour_edges_bv/*,
    OUT VSC_BIT_VECTOR* excluded_edges_bv*/
    )
{
    /* the result may be diffenrent from sub_root. For example, A->C, A->D, B->C, B->D, C->E, D->E. After one detour is merged, the other will not have E as root any more */
    VSC_IS_DepDagNode* result = gcvNULL;
    VSC_BIT_VECTOR* nodes_bv = _VSC_IS_DepDag_RentANodesBV(dag);
    VSC_IS_DepDagNode* nodeOnFirstPath = VSC_IS_DepDagEdge_GetToNode(first_succ_edge);
    VSC_IS_DepDagNode* nodeOnSecondPath = VSC_IS_DepDagEdge_GetToNode(second_succ_edge);
    VSC_IS_DepDagEdge* from_edge0;
    VSC_IS_DepDagEdge* from_edge1;
    VSC_IS_DepDagEdge* adj_edge;

    /* first step: reach down to sub_root from the first path. mark those nodes on the path. record the first breaking node if it exists */
    vscBV_SetBit(detour_edges_bv, VSC_IS_DepDagEdge_GetID(first_succ_edge));
    from_edge0 = first_succ_edge;
    while(nodeOnFirstPath != sub_root)
    {
        vscBV_SetBit(nodes_bv, VSC_IS_DepDagNode_GetID(nodeOnFirstPath));
        nodeOnFirstPath = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(nodeOnFirstPath, gcvTRUE, sub_tree_edges_bv, &adj_edge);
        gcmASSERT(adj_edge);
        from_edge0 = adj_edge;
        vscBV_SetBit(detour_edges_bv, VSC_IS_DepDagEdge_GetID(adj_edge));
    }
    vscBV_SetBit(nodes_bv, VSC_IS_DepDagNode_GetID(sub_root));

    /* second step: reach down to sub_root from the second path. If a breaking node is met, record and mark it. it will meet a node on the first path for sure */
    vscBV_SetBit(detour_edges_bv, VSC_IS_DepDagEdge_GetID(second_succ_edge));
    from_edge1 = second_succ_edge;
    while(!vscBV_TestBit(nodes_bv, VSC_IS_DepDagNode_GetID(nodeOnSecondPath)))
    {
        nodeOnSecondPath = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(nodeOnSecondPath, gcvTRUE, sub_tree_edges_bv, &adj_edge);
        gcmASSERT(adj_edge);
        from_edge1 = adj_edge;
        vscBV_SetBit(detour_edges_bv, VSC_IS_DepDagEdge_GetID(adj_edge));
    }
    /* -- now nodeOnSecondPath is the new sub root */

    /* third step: */
    if(nodeOnSecondPath == sub_root)
    {
        *first_from_edge = from_edge0 + 1;
    }
    else
    {
        VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(nodeOnSecondPath);
        VSC_UL_ITERATOR pred_edge_iter;
        VSC_IS_DepDagEdge* pred_edge;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&pred_edge_iter, pred_edge_list);
        for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&pred_edge_iter);
            pred_edge != gcvNULL;   /*??*/
            pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&pred_edge_iter))
        {
            if(vscBV_TestBit(detour_edges_bv, VSC_IS_DepDagEdge_GetID(pred_edge)) &&
               pred_edge - 1 != from_edge1)
            {
                *first_from_edge = pred_edge;
                break;
            }
        }
        gcmASSERT(pred_edge);
    }
    *second_from_edge = from_edge1 + 1;
    result = nodeOnSecondPath;

    _VSC_IS_DepDag_ReturnANodesBV(dag, nodes_bv);

    return result;
}

static VSC_ErrCode _VSC_IS_MergeDetours(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root,
    IN gctUINT maxDetourInDegree,
    IN gctUINT maxByPassOutDegree,
    OUT gctBOOL* detourLeft
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VSC_BIT_VECTOR* nodes_bv;
    VSC_BIT_VECTOR* sub_tree_edges_bv;
    gctBOOL hasExcluded;
    VSC_IS_DepDagNode* top_nodes_head = gcvNULL;
    VSC_IS_DepDagNode* top_nodes_tail = gcvNULL;

    gcmASSERT(detourLeft);

    if(VSC_IS_DepDagNode_GetInDegree(sub_root) <= 1)
    {
        *detourLeft = gcvFALSE;
        return errCode;
    }

    nodes_bv = _VSC_IS_DepDag_RentANodesBV(dag);
    sub_tree_edges_bv = _VSC_IS_DepDag_RentAEdgesBV(dag);
    _VSC_IS_DepDagNode_MarkSubTree(sub_root, gcvFALSE, VSC_IS_DEPDAGNODE_FLAG_NONE, VSC_IS_DEPDAGEDGE_FLAG_NONE, gcvNULL, nodes_bv, sub_tree_edges_bv);
    hasExcluded = _VSC_IS_FindDetourTopNodes(dag, sub_root, sub_tree_edges_bv, maxDetourInDegree, maxByPassOutDegree, &top_nodes_head, &top_nodes_tail);
    _VSC_IS_DepDag_ReturnANodesBV(dag, nodes_bv);

    if(top_nodes_head)
    {
        while(top_nodes_head)
        {
            VSC_ADJACENT_LIST* succ_edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(top_nodes_head);
            VSC_UL_ITERATOR succ_edge_iter;
            VSC_IS_DepDagEdge* succ_edge;
            VSC_IS_DepDagEdge* first_succ_edge = gcvNULL;
            VSC_IS_DepDagEdge* second_succ_edge = gcvNULL;

            VSC_ADJACENT_LIST_ITERATOR_INIT(&succ_edge_iter, succ_edge_list);
            for(succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succ_edge_iter);
                succ_edge != gcvNULL;   /*??*/
                succ_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succ_edge_iter))
            {
                if(vscBV_TestBit(sub_tree_edges_bv, VSC_IS_DepDagEdge_GetID(succ_edge)))
                {
                    if(!first_succ_edge)
                    {
                        first_succ_edge = succ_edge;
                    }
                    else if(!second_succ_edge)
                    {
                        second_succ_edge = succ_edge;
                    }
                }
            }

            if(first_succ_edge && second_succ_edge)
            {
                VSC_BIT_VECTOR* detour_edges_bv = _VSC_IS_DepDag_RentAEdgesBV(dag);

                VSC_IS_DepDagEdge* first_pred_edge = gcvNULL;
                VSC_IS_DepDagEdge* second_pred_edge = gcvNULL;

                VSC_IS_DepDagNode* new_sub_root =  _VSC_IS_FindFromEdges(dag,
                                                                            top_nodes_head,
                                                                            first_succ_edge,
                                                                            second_succ_edge,
                                                                            sub_root,
                                                                            sub_tree_edges_bv,
                                                                            &first_pred_edge,
                                                                            &second_pred_edge,
                                                                            detour_edges_bv/*,
                                                                            excluded_edges_bv*/);

                gcmASSERT(new_sub_root && first_pred_edge && second_pred_edge);

                errCode = _VSC_IS_MergeDetour(is, top_nodes_head, first_pred_edge, second_pred_edge, new_sub_root, detour_edges_bv, gcvFALSE);
                _VSC_IS_DepDag_ReturnAEdgesBV(dag, detour_edges_bv);

                if(errCode != VSC_ERR_NONE)
                {
                    break;
                }
            }
            else
            {
                top_nodes_head = VSC_IS_DepDagNode_GetNext(top_nodes_head);
            }
        }
    }

    _VSC_IS_DepDag_ReturnAEdgesBV(dag, sub_tree_edges_bv);

    if(hasExcluded)
    {
        *detourLeft = gcvTRUE;
    }
    else
    {
        *detourLeft = gcvFALSE;
    }

    return errCode;
}

static VSC_ErrCode _VSC_IS_DepDag_RecursivelyMergeDetours(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root,
    IN gctUINT maxDetourInDegree, /* detour in degree is the path count betwenn A and B in detour A-B */
    IN gctUINT maxByPassOutDegree, /* Bypass out degree is the out degree of a node on the path between A and B in detour A-B, excluding A & B */
    IN VSC_IS_DepDagNode_Flag triedFlag,
    OUT gctBOOL* detourLeft
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctBOOL predsDetourLeft = gcvFALSE;
    gctBOOL selfDetourLeft;

    VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(sub_root);
    VSC_UL_ITERATOR edge_iter;
    VSC_IS_DepDagEdge* pred_edge;

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter, pred_edge_list);
    for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter);
        pred_edge != gcvNULL;   /*??*/
        pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter))
    {
        VSC_IS_DepDagNode* pred = VSC_IS_DepDagEdge_GetToNode(pred_edge);

        if(!VSC_IS_DepDagNode_HasFlag(pred, triedFlag)
          )
        {
            gctBOOL result;

            _VSC_IS_DepDag_RecursivelyMergeDetours(is, pred, maxDetourInDegree, maxByPassOutDegree, triedFlag, &result);
            predsDetourLeft = predsDetourLeft || result;
        }
    }

    _VSC_IS_MergeDetours(is, sub_root, maxDetourInDegree, maxByPassOutDegree, &selfDetourLeft);

    if(predsDetourLeft || selfDetourLeft)
    {
        *detourLeft = gcvTRUE;
    }
    else
    {
        *detourLeft = gcvFALSE;
    }

    VSC_IS_DepDagNode_AddFlag(sub_root, triedFlag);

    return errCode;
}

static VSC_ErrCode _VSC_IS_DepDagNode_MergeBranch(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root,
    IN OUT VSC_IS_DepDagEdge* edge0,
    IN OUT VSC_IS_DepDagEdge* edge1,
    IN gctBOOL recursive_call
    )
{
    VSC_ErrCode err_code = VSC_ERR_NONE;
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);
    VSC_IS_DepDagNode* node0 = VSC_IS_DepDagEdge_GetToNode(edge0);
    VSC_IS_DepDagNode* node1 = VSC_IS_DepDagEdge_GetToNode(edge1);
    gctUINT32 bubble0 = VSC_IS_DepDagEdge_GetBubble(edge0);
    gctUINT32 bubble1 = VSC_IS_DepDagEdge_GetBubble(edge1);
    VSC_IS_DepDagEdge* new_edge;

    gcmASSERT(sub_root);
    gcmASSERT(edge0);
    gcmASSERT(edge1);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED) &&
       (VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED_INCLUDE_RECUR) || !recursive_call))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);

        VIR_LOG(dumper, "before merge branch:\n");

        VIR_LOG(dumper, "branch0:\n");
        _VSC_IS_DepDagNode_Dump(sub_root, dumper);
        _VSC_IS_DepDagEgde_Dump(edge0, dumper);
        _VSC_IS_DepDagNode_DumpList(VSC_IS_DepDagEdge_GetToNode(edge0), gcvNULL, gcvFALSE, gcvNULL, dumper);

        VIR_LOG(dumper, "branch1:\n");
        _VSC_IS_DepDagNode_Dump(sub_root, dumper);
        _VSC_IS_DepDagEgde_Dump(edge1, dumper);
        _VSC_IS_DepDagNode_DumpList(VSC_IS_DepDagEdge_GetToNode(edge1), gcvNULL, gcvFALSE, gcvNULL, dumper);
    }

    if(VSC_IS_DepDagNode_GetKillPriority(node0) > VSC_IS_DepDagNode_GetKillPriority(node1))
    {
        VSC_IS_DepDagEdge* node1_pred_edge;
        _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, gcvNULL, &node1_pred_edge);
        new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, node1);
        VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 > bubble1 ? bubble0 - bubble1 - 1 : 0);
        if(node1_pred_edge)
        {
            err_code = _VSC_IS_DepDagNode_MergeBranch(is, node1, new_edge + 1, node1_pred_edge, gcvTRUE);
        }
    }
    else if(VSC_IS_DepDagNode_GetKillPriority(node0) < VSC_IS_DepDagNode_GetKillPriority(node1))
    {
        VSC_IS_DepDagEdge* node0_pred_edge;
        _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
        _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, gcvNULL, &node0_pred_edge);
        new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, node0);
        VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 > bubble0 ? bubble1 - bubble0 - 1 : 0);
        if(node0_pred_edge)
        {
            err_code = _VSC_IS_DepDagNode_MergeBranch(is, node0, new_edge + 1, node0_pred_edge, gcvTRUE);
        }
    }
    else if(bubble0 && bubble1)
    {
        if(bubble0 >= bubble1)
        {
            VSC_IS_DepDagEdge* node1_pred_edge;
            _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, gcvNULL, &node1_pred_edge);
            new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, node1);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 > bubble1 ? bubble0 - bubble1 - 1 : 0);
            if(node1_pred_edge)
            {
                err_code = _VSC_IS_DepDagNode_MergeBranch(is, node1, new_edge + 1, node1_pred_edge, gcvTRUE);
            }
        }
        else
        {
            VSC_IS_DepDagEdge* node0_pred_edge;
            _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
            _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, gcvNULL, &node0_pred_edge);
            new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, node0);
            VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - bubble0 - 1);
            if(node0_pred_edge)
            {
                err_code = _VSC_IS_DepDagNode_MergeBranch(is, node0, new_edge + 1, node0_pred_edge, gcvTRUE);
            }
        }
    }
    else
    {
        if(bubble0)
        {
            VSC_IS_DepDagNode* end_node1;
            VSC_IS_DepDagEdge* end_node1_pred_edge;
            gctUINT32 final_distance = 0;
            end_node1 = _VSC_IS_DepDagNode_GetNodeOnList(node1,
                                                        gcvFALSE,
                                                        gcvNULL,
                                                        bubble0 - 1,
                                                        gcvTRUE,
                                                        VSC_IS_DepDagNode_GetKillPriority(node1),
                                                        gcvMAXUINT32,
                                                        0,
                                                        gcvMAXUINT32,
                                                        gcvNULL,
                                                        &final_distance,
                                                        gcvNULL,
                                                        &end_node1_pred_edge);

            if(VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
            {
                VSC_IS_DepDagEdge* node0_pred_edge;

                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node0, gcvFALSE, gcvNULL, &node0_pred_edge);
                _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, VSC_IS_DepDagEdge_GetFromNode(end_node1_pred_edge));
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - final_distance - 1);
                _VSC_IS_DepDag_RemoveEdge(dag, end_node1, VSC_IS_DepDagEdge_GetFromNode(end_node1_pred_edge));
                new_edge = _VSC_IS_DepDag_AddEdge(dag, end_node1, node0);
                if(node0_pred_edge)
                {
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, node0, new_edge + 1, node0_pred_edge, gcvTRUE);
                }
            }
            else
            {
                VSC_IS_DepDagEdge* end_node1_succ_edge;
                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, gcvNULL, &end_node1_succ_edge);
                _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, end_node1);
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble0 - final_distance - 1);
                if(end_node1_succ_edge)
                {
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node1, new_edge + 1, end_node1_succ_edge, gcvTRUE);
                }
            }
        }
        else if(bubble1)
        {
            VSC_IS_DepDagNode* end_node0;
            VSC_IS_DepDagEdge* end_node0_pred_edge;
            gctUINT32 final_distance = 0;
            end_node0 = _VSC_IS_DepDagNode_GetNodeOnList(node0,
                                                        gcvFALSE,
                                                        gcvNULL,
                                                        bubble1 - 1,
                                                        gcvTRUE,
                                                        VSC_IS_DepDagNode_GetKillPriority(node0),
                                                        gcvMAXUINT32,
                                                        0,
                                                        gcvMAXUINT32,
                                                        gcvNULL,
                                                        &final_distance,
                                                        gcvNULL,
                                                        &end_node0_pred_edge);

            if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0))
            {
                VSC_IS_DepDagEdge* node1_pred_edge;

                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node1, gcvFALSE, gcvNULL, &node1_pred_edge);
                _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, VSC_IS_DepDagEdge_GetFromNode(end_node0_pred_edge));
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - final_distance - 1);
                _VSC_IS_DepDag_RemoveEdge(dag, end_node0, VSC_IS_DepDagEdge_GetFromNode(end_node0_pred_edge));
                new_edge = _VSC_IS_DepDag_AddEdge(dag, end_node0, node1);
                if(node1_pred_edge)
                {
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, node1, new_edge + 1, node1_pred_edge, gcvTRUE);
                }
            }
            else
            {
                VSC_IS_DepDagEdge* end_node0_succ_edge;
                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, gcvNULL, &end_node0_succ_edge);
                _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, end_node0);
                VSC_IS_DepDagEdge_SetBubble(new_edge, bubble1 - final_distance - 1);
                if(end_node0_succ_edge)
                {
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node0, new_edge + 1, end_node0_succ_edge, gcvTRUE);
                }
            }
        }
        else
        {
            gctUINT32 end_node0_distance = 0;
            gctUINT32 end_node1_distance = 0;
            VSC_IS_DepDagEdge* prev_edge0 = gcvNULL;
            VSC_IS_DepDagEdge* prev_edge1 = gcvNULL;
            VSC_IS_DepDagNode* end_node0;
            VSC_IS_DepDagNode* end_node1;

            end_node0 = _VSC_IS_DepDagNode_GetNodeOnList(node0,
                                                         gcvFALSE,
                                                         gcvNULL,
                                                         gcvMAXUINT32,
                                                         gcvTRUE,
                                                         VSC_IS_DepDagNode_GetKillPriority(node0),
                                                         gcvMAXUINT32,
                                                         0,
                                                         gcvMAXUINT32,
                                                         gcvNULL,
                                                         &end_node0_distance,
                                                         gcvNULL,
                                                         &prev_edge0);
            end_node1 = _VSC_IS_DepDagNode_GetNodeOnList(node1,
                                                         gcvFALSE,
                                                         gcvNULL,
                                                         gcvMAXUINT32,
                                                         gcvTRUE,
                                                         VSC_IS_DepDagNode_GetKillPriority(node1),
                                                         gcvMAXUINT32,
                                                         0,
                                                         gcvMAXUINT32,
                                                         gcvNULL,
                                                         &end_node1_distance,
                                                         gcvNULL,
                                                         &prev_edge1);

            if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0) &&
               VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
            {
                if(end_node0_distance < end_node1_distance)
                {
                    _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                    new_edge = _VSC_IS_DepDag_AddEdge(dag, end_node1, VSC_IS_DepDagEdge_GetFromNode(prev_edge0));
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, VSC_IS_DepDagEdge_GetFromNode(prev_edge0), new_edge + 1, prev_edge0, gcvTRUE);
                }
                else
                {
                    _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                    new_edge = _VSC_IS_DepDag_AddEdge(dag, end_node0, VSC_IS_DepDagEdge_GetFromNode(prev_edge1));
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, VSC_IS_DepDagEdge_GetFromNode(prev_edge1), new_edge + 1, prev_edge1, gcvTRUE);
                }
            }
            else if(VSC_IS_DepDagNode_GetKillPriority(end_node0) > VSC_IS_DepDagNode_GetKillPriority(node0))
            {
                if(VSC_IS_DepDagNode_GetInDegree(end_node1))
                {
                    VSC_IS_DepDagEdge* end_node1_succ_edge;

                    _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, gcvNULL, &end_node1_succ_edge);
                    gcmASSERT(VSC_IS_DepDagEdge_GetBubble(end_node1_succ_edge));
                    _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                    new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, end_node1);
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node1, new_edge + 1, end_node1_succ_edge, gcvTRUE);
                }
                else
                {
                    if(end_node0_distance <= end_node1_distance)
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                        _VSC_IS_DepDag_RemoveEdge(dag, end_node0, VSC_IS_DepDagEdge_GetFromNode(prev_edge0));
                        _VSC_IS_DepDag_AddEdge(dag, node1, VSC_IS_DepDagEdge_GetFromNode(prev_edge0));
                        _VSC_IS_DepDag_AddEdge(dag, end_node0, end_node1);
                    }
                    else
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                        _VSC_IS_DepDag_AddEdge(dag, node0, end_node1);
                    }
                }
            }
            else if(VSC_IS_DepDagNode_GetKillPriority(end_node1) > VSC_IS_DepDagNode_GetKillPriority(node1))
            {
                if(VSC_IS_DepDagNode_GetInDegree(end_node0))
                {
                    VSC_IS_DepDagEdge* end_node0_succ_edge;

                    _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, gcvNULL, &end_node0_succ_edge);
                    gcmASSERT(VSC_IS_DepDagEdge_GetBubble(end_node0_succ_edge));
                    _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                    new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, end_node0);
                    err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node0, new_edge + 1, end_node0_succ_edge, gcvTRUE);
                }
                else
                {
                    if(end_node1_distance <= end_node0_distance)
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                        _VSC_IS_DepDag_RemoveEdge(dag, end_node1, VSC_IS_DepDagEdge_GetFromNode(prev_edge1));
                        _VSC_IS_DepDag_AddEdge(dag, node0, VSC_IS_DepDagEdge_GetFromNode(prev_edge1));
                        _VSC_IS_DepDag_AddEdge(dag, end_node1, end_node0);
                    }
                    else
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                        _VSC_IS_DepDag_AddEdge(dag, node1, end_node0);
                    }
                }
            }
            else
            {
                VSC_IS_DepDagEdge* bubble0_edge = gcvNULL;
                VSC_IS_DepDagEdge* bubble1_edge = gcvNULL;

                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node0, gcvFALSE, gcvNULL, &bubble0_edge);
                _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(end_node1, gcvFALSE, gcvNULL, &bubble1_edge);
                if(bubble0_edge)
                {
                    gcmASSERT(VSC_IS_DepDagEdge_GetBubble(bubble0_edge));
                    bubble0 = VSC_IS_DepDagEdge_GetBubble(bubble0_edge);
                }
                if(bubble1_edge)
                {
                    gcmASSERT(VSC_IS_DepDagEdge_GetBubble(bubble1_edge));
                    bubble1 = VSC_IS_DepDagEdge_GetBubble(bubble1_edge);
                }

                if(bubble0 == 0 && bubble1 == 0)
                {
                    if(end_node0_distance >= end_node1_distance)
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                        _VSC_IS_DepDag_AddEdge(dag, node0, end_node1);
                    }
                    else
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                        _VSC_IS_DepDag_AddEdge(dag, node1, end_node0);
                    }
                }
                else
                {
                    gctUINT32 following_distance0;
                    gctUINT32 following_distance1;
                    gctUINT32 bubble_sum0;
                    gctUINT32 bubble_sum1;
                    gctUINT32 total_length0;
                    gctUINT32 total_length1;
                    gctUINT32 filled_bubble0;
                    gctUINT32 filled_bubble1;

                    _VSC_IS_DepDagNode_GetNodeOnList(end_node0,
                                                            gcvFALSE,
                                                            gcvNULL,
                                                            gcvMAXUINT32,
                                                            gcvFALSE,
                                                            VSC_IS_DepDagNode_GetKillPriority(node0),
                                                            gcvMAXUINT32,
                                                            0,
                                                            gcvMAXUINT32,
                                                            gcvNULL,
                                                            &following_distance0,
                                                            &bubble_sum0,
                                                            gcvNULL);
                    _VSC_IS_DepDagNode_GetNodeOnList(end_node1,
                                                            gcvFALSE,
                                                            gcvNULL,
                                                            gcvMAXUINT32,
                                                            gcvFALSE,
                                                            VSC_IS_DepDagNode_GetKillPriority(node1),
                                                            gcvMAXUINT32,
                                                            0,
                                                            gcvMAXUINT32,
                                                            gcvNULL,
                                                            &following_distance1,
                                                            &bubble_sum1,
                                                            gcvNULL);

                    total_length0 = end_node0_distance + 1 + following_distance0 + bubble_sum0;
                    total_length1 = end_node1_distance + 1 + following_distance1 + bubble_sum1;
                    filled_bubble0 = bubble_sum0 > total_length0 ? total_length0 : bubble_sum0;
                    filled_bubble1 = bubble_sum1 > total_length1 ? total_length1 : bubble_sum1;

                    if(filled_bubble0 >= filled_bubble1)
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node1, sub_root);
                        new_edge = _VSC_IS_DepDag_AddEdge(dag, node1, end_node0);
                        err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node0, new_edge + 1, bubble0_edge, gcvTRUE);
                    }
                    else
                    {
                        _VSC_IS_DepDag_RemoveEdge(dag, node0, sub_root);
                        new_edge = _VSC_IS_DepDag_AddEdge(dag, node0, end_node1);
                        err_code = _VSC_IS_DepDagNode_MergeBranch(is, end_node1, new_edge + 1, bubble1_edge, gcvTRUE);
                    }
                }
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED) &&
       (VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED_INCLUDE_RECUR) || !recursive_call))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        gctUINT bubble_sum;

        VIR_LOG(dumper, "after merge branch:\n");
        VIR_LOG(dumper, "list:\n");
        _VSC_IS_DepDagNode_Dump(sub_root, dumper);
        if(_VSC_IS_DepDag_GetEdge(dag, node0, sub_root))
        {
            _VSC_IS_DepDagEgde_Dump(edge0, dumper);
            _VSC_IS_DepDagNode_DumpList(node0, gcvNULL, gcvFALSE, gcvNULL, dumper);
        }
        else
        {
            gcmASSERT(_VSC_IS_DepDag_GetEdge(dag, node1, sub_root));
            _VSC_IS_DepDagEgde_Dump(edge1, dumper);
            _VSC_IS_DepDagNode_DumpList(node1, gcvNULL, gcvFALSE, gcvNULL, dumper);
        }
        _VSC_IS_DepDagNode_GetNodeOnList(sub_root, gcvFALSE, gcvNULL, gcvMAXUINT32, gcvFALSE, gcvMAXUINT32, gcvMAXUINT32, 0, gcvMAXUINT32, gcvNULL, gcvNULL, &bubble_sum, gcvNULL);
        VIR_LOG(dumper, "bubble_sum: %d\n", bubble_sum);
        VIR_LOG_FLUSH(dumper);
    }

    return err_code;
}

static VSC_ErrCode _VSC_IS_MergeFork(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root
    )
{
    VSC_ErrCode err_code = VSC_ERR_NONE;
    VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(sub_root);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);

    if(AJLST_GET_EDGE_COUNT(pred_edge_list) <= 1)
    {
        return err_code;
    }

    while(AJLST_GET_EDGE_COUNT(pred_edge_list) != 1)
    {
        VSC_UL_ITERATOR edge_iter0;
        VSC_IS_DepDagEdge* pred_edge0;
        VSC_IS_DepDagEdge* pred_edge1;
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter0, pred_edge_list);
        pred_edge0 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter0);
        pred_edge1 = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter0);
        err_code = _VSC_IS_DepDagNode_MergeBranch(is, sub_root, pred_edge0, pred_edge1, gcvFALSE);
        if(err_code != VSC_ERR_NONE)
        {
            return err_code;
        }
    }

    gcmASSERT(AJLST_GET_EDGE_COUNT(pred_edge_list) == 1);

    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BUBBLESCHED))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);

        VIR_LOG(dumper, "after fork merged on node:\n");
        _VSC_IS_DepDagNode_DumpList(sub_root, gcvNULL, gcvFALSE, gcvNULL, dumper);
    }

    return err_code;
}

static VSC_ErrCode _VSC_IS_RecursivelyMergeFork(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;

    VSC_ADJACENT_LIST* pred_edge_list = VSC_IS_DepDagNode_GetPredEdgeList(sub_root);
    VSC_UL_ITERATOR edge_iter;
    VSC_IS_DepDagEdge* pred_edge;

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edge_iter, pred_edge_list);
    for(pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edge_iter);
        pred_edge != gcvNULL;   /*??*/
        pred_edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edge_iter))
    {
        VSC_IS_DepDagNode* pred = VSC_IS_DepDagEdge_GetToNode(pred_edge);

        if(!VSC_IS_DepDagNode_HasFlag(pred, VSC_IS_DEPDAGNODE_FLAG_FORK_MERGED))
        {
            err_code = _VSC_IS_RecursivelyMergeFork(is, pred);
        }
    }

    err_code = _VSC_IS_MergeFork(is, sub_root);

    VSC_IS_DepDagNode_AddFlag(sub_root, VSC_IS_DEPDAGNODE_FLAG_FORK_MERGED);

    return err_code;
}

static VSC_ErrCode _VSC_IS_MergeOnNode(
    IN VSC_IS_InstSched* is,
    IN OUT VSC_IS_DepDagNode* sub_root
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;
    gctBOOL detourLeft;

    err_code = _VSC_IS_DepDag_RecursivelyMergeDetours(is, sub_root, 4, 4, VSC_IS_DEPDAGNODE_FLAG_1ST_TIME_DETOURS_TRIED, &detourLeft);
    if(detourLeft)
    {
        err_code = _VSC_IS_DepDag_RecursivelyMergeDetours(is, sub_root, gcvMAXUINT32, gcvMAXUINT32, VSC_IS_DEPDAGNODE_FLAG_2nd_DETOURS_TRIED, &detourLeft);
    }
    gcmASSERT(!detourLeft);
    err_code = _VSC_IS_RecursivelyMergeFork(is, sub_root);

    return err_code;
}

static VSC_ErrCode _VSC_IS_DoBubbleScheduling(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode err_code  = VSC_ERR_NONE;
    VSC_IS_DepDag* dag = VSC_IS_InstSched_GetCurrDepDag(is);
    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VSC_IS_DepDagNode* tail_node;
    /* VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is); */

    VIR_Instruction *prev, *next;

    _VSC_IS_GetBBEssence(bb, &prev, &next);
    prev = VIR_Inst_GetPrev(prev);
    next = VIR_Inst_GetNext(next);

    /* merge */
    {
        VSC_SIMPLE_RESIZABLE_ARRAY* tail_array;

        tail_array = DG_GET_TAIL_ARRAY_P(VSC_IS_DepDag_GetDGraph(dag));
        gcmASSERT(vscSRARR_GetElementCount(tail_array) == 1);       /* a pseudo tail node was added */
        tail_node = *(VSC_IS_DepDagNode**)vscSRARR_GetElement(tail_array, 0);

        err_code = _VSC_IS_MergeOnNode(is, tail_node);
    }

    {
        VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);

        if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_BB_BUBBLE_SUM))
        {
            gctUINT bubble_sum;
            VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);

            _VSC_IS_DepDagNode_GetNodeOnList(tail_node, gcvFALSE, gcvNULL, gcvMAXUINT32, gcvFALSE, gcvMAXUINT32, gcvMAXUINT32, 0, gcvMAXUINT32, gcvNULL, gcvNULL, &bubble_sum, gcvNULL);
            VIR_LOG(dumper, "BB(%d)'s bubble sum: %d.\n", BB_GET_ID(bb), bubble_sum);
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* relink instructions */
    {
        gctUINT32 i;
        VSC_IS_DepDagNode* end = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(tail_node, gcvFALSE, gcvNULL, gcvNULL);
        VIR_Instruction* end_inst = VSC_IS_DepDagNode_GetInst(end);
        VSC_IS_DepDagNode* iter_last = end;
        VIR_Instruction* iter_last_inst = end_inst;
        VSC_IS_DepDagNode* iter;
        VIR_Instruction* iter_inst;

        for(i = 2; i < VSC_IS_DepDag_GetNodeCount(dag); i++)
        {
            iter = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(iter_last, gcvFALSE, gcvNULL, gcvNULL);
            iter_inst = VSC_IS_DepDagNode_GetInst(iter);
            VIR_Inst_SetPrev(iter_last_inst, iter_inst);
            VIR_Inst_SetNext(iter_inst, iter_last_inst);
            iter_last = iter;
            iter_last_inst = iter_inst;
        }

        VIR_Inst_SetPrev(iter_last_inst, prev);
        if(prev)
        {
            VIR_Inst_SetNext(prev, iter_last_inst);
        }
        VIR_Inst_SetNext(end_inst, next);
        if(next)
        {
            VIR_Inst_SetPrev(next, end_inst);
        }
        if(!VIR_OPCODE_isBBPrefix(VIR_Inst_GetOpcode(BB_GET_START_INST(bb))))
        {
            BB_SET_START_INST(bb, iter_last_inst);
        }
        if(!VIR_OPCODE_isBBSuffix(VIR_Inst_GetOpcode(BB_GET_END_INST(bb))))
        {
            BB_SET_END_INST(bb, end_inst);
        }
    }

    return err_code;
}

static gctBOOL _VSC_IS_IsBandwidthRatioMetForBB(
    IN OUT VSC_IS_InstSched* is
    )
{
    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VIR_Instruction *start_inst, *end_inst;
    VIR_Instruction* inst;
    gctUINT32 i, len, texld_count = 0, memld_count = 0;

    len = _VSC_IS_GetBBEssence(bb, &start_inst, &end_inst);

    for(i = 0, inst = start_inst; i < len; i++, inst = VIR_Inst_GetNext(inst))
    {
        if(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)))
        {
            ++texld_count;
        }
        if(VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(inst)))
        {
            ++memld_count;
        }
    }

    return len > (texld_count * (VSC_IS_InstSched_GetTexldInterfaceBubble(is) + 1)) + 1 &&
        len > (memld_count * (VSC_IS_InstSched_GetMemldInterfaceBubble(is) + 1)) + 1;
}

static gctBOOL _VSC_IS_ContainsVXInBB(
    IN OUT VSC_IS_InstSched* is
    )
{
    VIR_BASIC_BLOCK* bb = VSC_IS_InstSched_GetCurrBB(is);
    VIR_Instruction *start_inst, *end_inst;
    VIR_Instruction* inst;
    gctUINT32 i, len;

    len = _VSC_IS_GetBBEssence(bb, &start_inst, &end_inst);

    for(i = 0, inst = start_inst; i < len; i++, inst = VIR_Inst_GetNext(inst))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

        if(VIR_OPCODE_isVX(opcode) &&
           opcode != VIR_OP_VX_IMG_LOAD &&
           opcode != VIR_OP_VX_IMG_LOAD_3D)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode _VSC_IS_DoInstructionSchedulingForBB(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode            err_code  = VSC_ERR_NONE;
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);

    if(!VSC_OPTN_InRange(VSC_IS_InstSched_GetCurrBB(is)->dgNode.id, VSC_OPTN_ISOptions_GetBeforeBB(options), VSC_OPTN_ISOptions_GetAfterBB(options)))
    {
        if(VSC_OPTN_ISOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
            VIR_LOG(dumper, "Instruction Scheduling skips BB(%d)\n", VSC_IS_InstSched_GetCurrBB(is)->dgNode.id);
            VIR_LOG_FLUSH(dumper);
        }
        return err_code;
    }

    /* dump input bb */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_INPUT_BB))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling Start for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, VSC_IS_InstSched_GetCurrBB(is)->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, VSC_IS_InstSched_GetCurrBB(is), gcvTRUE);
    }

    if(VSC_OPTN_ISOptions_GetBandwidthOnly(options) && !_VSC_IS_IsBandwidthRatioMetForBB(is))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_OUTPUT_BB))
        {
            VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
            VIR_LOG(dumper, "%s\nInstruction Scheduling End for BB %d\n%s\n",
                VSC_TRACE_STAR_LINE, VSC_IS_InstSched_GetCurrBB(is)->dgNode.id, VSC_TRACE_STAR_LINE);
            VIR_LOG(dumper, "Not scheduled because the bandwidth ratio is not met.\n");
        }
        return err_code;
    }

    if(!VSC_OPTN_ISOptions_GetVXOn(options) && _VSC_IS_ContainsVXInBB(is))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_OUTPUT_BB))
        {
            VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
            VIR_LOG(dumper, "%s\nInstruction Scheduling End for BB %d\n%s\n",
                VSC_TRACE_STAR_LINE, VSC_IS_InstSched_GetCurrBB(is)->dgNode.id, VSC_TRACE_STAR_LINE);
            VIR_LOG(dumper, "Not scheduled because it contains VX instructions.\n");
        }
        return err_code;
    }

    _VSC_IS_InstSched_NewDepDag(is);
    _VSC_IS_BuildDAGForBB(is);

    switch(VSC_OPTN_ISOptions_GetAlgorithm(options))
    {
        case VSC_OPTN_ISOptions_ALGORITHM_LISTSCHEDULING:
            _VSC_IS_DoListScheduling(is);
            break;
        case VSC_OPTN_ISOptions_ALGORITHM_BUBBLESCHEDULING:
            _VSC_IS_DoBubbleScheduling(is);
            break;
        default:
            gcmASSERT(0);
    }

    /* dump output bb */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_OUTPUT_BB))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling End for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, VSC_IS_InstSched_GetCurrBB(is)->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, VSC_IS_InstSched_GetCurrBB(is), gcvTRUE);
    }

    _VSC_IS_InstSched_DeleteDepDag(is);

    return err_code;
}

static VSC_ErrCode _VSC_IS_InstSched_PerformOnFunction(
    IN OUT VSC_IS_InstSched* is
    )
{
    VSC_ErrCode            err_code  = VSC_ERR_NONE;
    VIR_Shader* shader = VSC_IS_InstSched_GetShader(is);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VSC_OPTN_ISOptions* options = VSC_IS_InstSched_GetOptions(is);
    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfg_iter;
    VIR_BASIC_BLOCK* bb;
    gctBOOL scheduled = gcvFALSE;
    static gctUINT32 counter = 0;

    if(!VSC_OPTN_InRange(counter, VSC_OPTN_ISOptions_GetBeforeFunc(options), VSC_OPTN_ISOptions_GetAfterFunc(options)))
    {
        if(VSC_OPTN_ISOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
            VIR_LOG(dumper, "Instruction Scheduling skips function(%d)\n", counter);
            VIR_LOG_FLUSH(dumper);
        }
        counter++;
        return err_code;
    }

    if(VSC_OPTN_ISOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling Start for function %s(%d)\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), counter, VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    cfg = VIR_Function_GetCFG(func);

    /* dump input cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_INPUT_CFG))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling: input cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    /* Only inst count GT 1 is meaningful for inst sched */
    if (VIR_Inst_Count(&func->instList) > 1)
    {
        CFG_ITERATOR_INIT(&cfg_iter, cfg);
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            gctUINT32 bb_length = _VSC_IS_GetBBEssence(bb, gcvNULL, gcvNULL);
            if(bb_length > 1 &&
               bb_length <= VSC_OPTN_ISOptions_GetBBCeiling(options) &&
               (!VSC_OPTN_ISOptions_GetLLIOnly(options) || BB_FLAGS_GET_LLI(bb))
              )
            {
                VSC_IS_InstSched_SetCurrBB(is, bb);
                err_code = _VSC_IS_DoInstructionSchedulingForBB(is);
                scheduled = gcvTRUE;
            }

            if(err_code)
            {
                return err_code;
            }
        }

        func->instList.pHead = func->instList.pHead->parent.BB->pStartInst;
        func->instList.pTail = func->instList.pTail->parent.BB->pEndInst;
    }

    /* dump output cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_ISOptions_GetTrace(options), VSC_OPTN_ISOptions_TRACE_OUTPUT_CFG))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling: output cfg of function %s: %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)),
            scheduled ? "scheduled" : "not scheduled", VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    if(VSC_OPTN_ISOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_IS_InstSched_GetDumper(is);
        VIR_LOG(dumper, "%s\nInstruction Scheduling End for function %s(%d)\n%s\n",
            VSC_TRACE_BAR_LINE, VIR_Shader_GetSymNameString(shader, VIR_Function_GetSymbol(func)), counter, VSC_TRACE_BAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }
    counter++;

    return err_code;
}

DEF_QUERY_PASS_PROP(VSC_IS_InstSched_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_IS;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

VSC_ErrCode VSC_IS_InstSched_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG* hwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_DEF_USAGE_INFO* du_info = pPassWorker->pDuInfo;
    VSC_OPTN_ISOptions* options = (VSC_OPTN_ISOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_IS_InstSched  is;

    if (!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_ISOptions_GetBeforeShader(options),
                          VSC_OPTN_ISOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_ISOptions_GetTrace(options))
        {
            VIR_LOG(dumper, "Instruction Scheduling skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_ISOptions_GetTrace(options))
        {
            VIR_LOG(dumper, "Instruction Scheduling starts for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    /* do IS only if the shader has instructions */
    if(VIR_Shader_GetTotalInstructionCount(shader))
    {
        _VSC_IS_InstSched_Init(&is, shader, hwCfg, du_info, options, dumper, pPassWorker->basePassWorker.pMM);

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
        for(func_node = VIR_FuncIterator_First(&func_iter);
            func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function* func = func_node->function;

            VIR_Shader_SetCurrentFunction(shader, func);
            errcode = _VSC_IS_InstSched_PerformOnFunction(&is);
            if(errcode)
            {
                break;
            }
        }

        _VSC_IS_InstSched_Final(&is);
    }

    if(VSC_OPTN_ISOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "Instruction Scheduling ends for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Instruction Scheduling.", shader, gcvTRUE);
    }

    return errcode;
}

/* dump functions */
void _VSC_IS_DepDagNode_Dump(
    IN VSC_IS_DepDagNode* node,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "[%d]", VSC_IS_DepDagNode_GetID(node));
    if(VSC_IS_DepDagNode_GetInst(node))
    {
        VIR_Inst_Dump(dumper, VSC_IS_DepDagNode_GetInst(node));
    }
    else
    {
        VIR_LOG(dumper, "pseudo end\n");
    }
    if(VSC_IS_DepDagNode_GetFlags(node))
    {
        VIR_LOG(dumper, "flags [ ");
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_PRED))
        {
            VIR_LOG(dumper, "Has_Binding_Pred ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_HAS_BINDING_SUCC))
        {
            VIR_LOG(dumper, "Has_Binding_Succ ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_DEPENDING_MOVA))
        {
            VIR_LOG(dumper, "Depending_MOVA ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_1ST_TIME_DETOURS_TRIED))
        {
            VIR_LOG(dumper, "4_Detours_Tried ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_2nd_DETOURS_TRIED))
        {
            VIR_LOG(dumper, "All_Detours_Tried ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_ALL_DETOURS_DONE))
        {
            VIR_LOG(dumper, "All_Detours_Done ");
        }
        if(VSC_IS_DepDagNode_HasFlag(node, VSC_IS_DEPDAGNODE_FLAG_FORK_MERGED))
        {
            VIR_LOG(dumper, "Fork_Merged ");
        }
        VIR_LOG(dumper, "] ");
    }
    VIR_LOG(dumper, "kill_priority: %d \n", VSC_IS_DepDagNode_GetKillPriority(node));
    VIR_LOG_FLUSH(dumper);
}

VSC_IS_DepDagNode* _VSC_IS_DepDagNode_DumpWithEdge(
    IN VSC_IS_DepDagNode* node,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    IN VIR_Dumper* dumper
    )
{
    VSC_IS_DepDagEdge* edge;
    VSC_IS_DepDagNode* to_node;

    _VSC_IS_DepDagNode_Dump(node, dumper);
    to_node = _VSC_IS_DepDagNode_GetAdjacentNodeAndEdge(node, succ, edges_bv, &edge);
    if(edge)
    {
        _VSC_IS_DepDagEgde_Dump(edge, dumper);
        return to_node;
    }
    else
    {
        return gcvNULL;
    }
}

void _VSC_IS_DepDagNode_DumpList(
    IN VSC_IS_DepDagNode* start,
    IN VSC_IS_DepDagNode* end,
    IN gctBOOL succ,
    IN VSC_BIT_VECTOR* edges_bv,
    IN VIR_Dumper* dumper
    )
{
    VSC_IS_DepDagNode* iter = start;

    gcmASSERT(start);
    while(iter && iter != end)
    {
        iter = _VSC_IS_DepDagNode_DumpWithEdge(iter, succ, edges_bv, dumper);
    }
    if(end)
    {
        _VSC_IS_DepDagNode_Dump(end, dumper);
    }
}

void _VSC_IS_DepDagNode_DumpWithPredSucc(
    IN VSC_IS_DepDagNode* node,
    IN VIR_Dumper* dumper
    )
{
    VSC_ADJACENT_LIST* edge_list;
    VSC_UL_ITERATOR iter;
    VSC_IS_DepDagEdge* edge;

    _VSC_IS_DepDagNode_Dump(node, dumper);

    edge_list = VSC_IS_DepDagNode_GetSuccEdgeList(node);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    if(VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter))
    {
        VIR_LOG(dumper, "has dependence: ");
        VIR_LOG_FLUSH(dumper);
    }
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        _VSC_IS_DepDagEgde_DumpWithNode(edge, gcvTRUE, dumper);
    }

    edge_list = VSC_IS_DepDagNode_GetPredEdgeList(node);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    if(VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter))
    {
        VIR_LOG(dumper, "depends on: ");
        VIR_LOG_FLUSH(dumper);
    }
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        _VSC_IS_DepDagEgde_DumpWithNode(edge, gcvTRUE, dumper);
    }

    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

gctBOOL _VSC_IS_DepDagNode_DepandsOnNode(
    IN VSC_IS_DepDagNode* node1,
    IN VSC_IS_DepDagNode* node2
    )
{
    VSC_ADJACENT_LIST* edge_list;
    VSC_UL_ITERATOR iter;
    VSC_IS_DepDagEdge* edge;

    edge_list = VSC_IS_DepDagNode_GetPredEdgeList(node1);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&iter, edge_list);
    for(edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_FIRST(&iter);
        edge != gcvNULL; edge = (VSC_IS_DepDagEdge*)VSC_ADJACENT_LIST_ITERATOR_NEXT(&iter))
    {
        if(VSC_IS_DepDagEdge_GetToNode(edge) == node2)
        {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

gctUINT32 _VSC_IS_DepDagNode_DepandsOnBubbleSet(
    IN VSC_IS_DepDagNode* node,
    IN VSC_HASH_TABLE* set
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;
    gctUINT32 max_bubble = 0;

    vscHTBLIterator_Init(&iter, set);
    for(pair = vscHTBLIterator_DirectFirst(&iter); IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* dep = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        if(_VSC_IS_DepDagNode_DepandsOnNode(node, dep))
        {
            gctUINT32 bubble = (gctUINT32)(gctUINTPTR_T)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
            if(bubble > max_bubble)
            {
                max_bubble = bubble;
            }
        }
    }
    return max_bubble;
}

static void _VSC_IS_ConflictType_Dump(
    IN gctUINT32 conflict_type,
    IN VIR_Dumper* dumper
    )
{
    if(VSC_IS_ConflictType_IsLOOPCARRIED(conflict_type))
    {
        VIR_LOG(dumper, "LOOP_CARRIED ");
    }
    if(VSC_IS_ConflictType_HasTLRS_RU(conflict_type))
    {
        VIR_LOG(dumper, "TLRS_RU ");
    }
    if(VSC_IS_ConflictType_HasTLRS_RS(conflict_type))
    {
        VIR_LOG(dumper, "TLRS_RS ");
    }
    if(VSC_IS_ConflictType_HasMLRS_RU(conflict_type))
    {
        VIR_LOG(dumper, "MLRS_RU ");
    }
    if(VSC_IS_ConflictType_HasMLRS_RS(conflict_type))
    {
        VIR_LOG(dumper, "MLRS_RS ");
    }
    if(VSC_IS_ConflictType_HasML_MS(conflict_type))
    {
        VIR_LOG(dumper, "ML_MS ");
    }
    if(VSC_IS_ConflictType_HasMS_ML(conflict_type))
    {
        VIR_LOG(dumper, "MS_ML ");
    }
    if(VSC_IS_ConflictType_HasMS_MS(conflict_type))
    {
        VIR_LOG(dumper, "MS_MS ");
    }
    if(VSC_IS_ConflictType_HasCLRS_RU(conflict_type))
    {
        VIR_LOG(dumper, "CLRS_RU ");
    }
    if(VSC_IS_ConflictType_HasCLRS_RS(conflict_type))
    {
        VIR_LOG(dumper, "CLRS_RS ");
    }
    if(VSC_IS_ConflictType_HasCL_CS(conflict_type))
    {
        VIR_LOG(dumper, "CL_CS ");
    }
    if(VSC_IS_ConflictType_HasCS_CL(conflict_type))
    {
        VIR_LOG(dumper, "CS_CL ");
    }
    if(VSC_IS_ConflictType_HasCS_CS(conflict_type))
    {
        VIR_LOG(dumper, "CS_CS ");
    }
    if(VSC_IS_ConflictType_HasRS_RU(conflict_type))
    {
        VIR_LOG(dumper, "RS_RU ");
    }
    if(VSC_IS_ConflictType_HasRU_RS(conflict_type))
    {
        VIR_LOG(dumper, "RU_RS ");
    }
    if(VSC_IS_ConflictType_HasRS_RS(conflict_type))
    {
        VIR_LOG(dumper, "RS_RS ");
    }
    if(VSC_IS_ConflictType_HasCOND(conflict_type))
    {
        VIR_LOG(dumper, "COND ");
    }
    if(VSC_IS_ConflictType_HasContinuousBinding(conflict_type))
    {
        VIR_LOG(dumper, "CONTINUOUS_BINDING ");
    }
    if(VSC_IS_ConflictType_HasLooseBindingLDARR(conflict_type))
    {
        VIR_LOG(dumper, "LOOSE_BINDING_LDARR");
    }
    if(VSC_IS_ConflictType_HasLooseBindingMOVA(conflict_type))
    {
        VIR_LOG(dumper, "LOOSE_BINDING_MOVA");
    }
    if(VSC_IS_ConflictType_HasDodging(conflict_type))
    {
        VIR_LOG(dumper, "DODGING ");
    }
    if(VSC_IS_ConflictType_UseReturnValue(conflict_type))
    {
        VIR_LOG(dumper, "UseReturnValue ");
    }
    if(VSC_IS_ConflictType_HasBarrier(conflict_type))
    {
        VIR_LOG(dumper, "Barrier ");
    }
    if(VSC_IS_ConflictType_HasAtomic(conflict_type))
    {
        VIR_LOG(dumper, "Atomic ");
    }
}

static void _VSC_IS_DepDagEgde_Dump(
    IN VSC_IS_DepDagEdge* edge,
    IN VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "conflict type: ");
    _VSC_IS_ConflictType_Dump(VSC_IS_DepDagEdge_GetConflictType(edge), dumper);
    VIR_LOG(dumper, " bubble: %d ", VSC_IS_DepDagEdge_GetBubble(edge));
    VIR_LOG_FLUSH(dumper);
}

static VSC_IS_DepDagNode* _VSC_IS_DepDagEgde_DumpWithNode(
    IN VSC_IS_DepDagEdge* edge,
    IN gctBOOL with_to_node,
    IN VIR_Dumper* dumper
    )
{
    VSC_IS_DepDagNode* ret = gcvNULL;
    if(!with_to_node)
    {
        ret = VSC_IS_DepDagEdge_GetFromNode(edge);
        _VSC_IS_DepDagNode_Dump(ret, dumper);
    }

    _VSC_IS_DepDagEgde_Dump(edge, dumper);

    if(with_to_node)
    {
        ret = VSC_IS_DepDagEdge_GetToNode(edge);
        _VSC_IS_DepDagNode_Dump(ret, dumper);
    }
    VIR_LOG_FLUSH(dumper);
    return ret;
}

static void _VSC_IS_DepDag_Dump(
    IN VSC_IS_DepDag* dag,
    IN VIR_Dumper* dumper
    )
{
    VSC_DG_ITERATOR iter;
    VSC_IS_DepDagNode* node;

    VIR_LOG(dumper, "dependence graph:\n");

    vscDG_ITERATOR_Initialize(&iter, VSC_IS_DepDag_GetDGraph(dag),
        VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE, VSC_GRAPH_TRAVERSAL_ORDER_PREV, gcvFALSE);

    for(node = (VSC_IS_DepDagNode*)vscDG_ITERATOR_Begin(&iter);
        node != gcvNULL; node = (VSC_IS_DepDagNode*)vscDG_ITERATOR_Next(&iter))
    {
        _VSC_IS_DepDagNode_DumpWithPredSucc(node, dumper);
    }

    vscDG_ITERATOR_End(&iter);
}

static void _VSC_IS_DumpInstSet(
    IN VSC_HASH_TABLE* inst_set,
    IN VIR_Dumper* dumper
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;

    vscHTBLIterator_Init(&iter, inst_set);

    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair); pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_IS_DepDagNode* node = (VSC_IS_DepDagNode*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        _VSC_IS_DepDagNode_Dump(node, dumper);
        if(VSC_DIRECT_HNODE_PAIR_SECOND(&pair))
        {
            VIR_LOG(dumper, " value:%d\n", (gctUINT32)(gctUINTPTR_T)VSC_DIRECT_HNODE_PAIR_SECOND(&pair));
            VIR_LOG_FLUSH(dumper);
        }
    }
}

