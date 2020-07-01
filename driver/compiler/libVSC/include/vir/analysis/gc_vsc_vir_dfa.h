/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_vir_dfa_h_
#define __gc_vsc_vir_dfa_h_

/* Here we define all data flow related stuff based on VIR, such as reach-def/liveness/...

   IMPORTANT:
   Even there are 3 data flow analysis, forward/backward/bi-direction, we always call flow
   at top of basic block as inFlow, and flow at bottom of basic block as outFlow. Don't
   confuse yourself when doing backward analysis.

   The PROCEDURE of doing global DFA is as following:
   a. Calc how many of flow size. For example, for reach-def, flow size is def count of whole
      shader, and for live variable, it can be selected as the number of webs or def count.
   b. Design a structure which must be derived from VIR_BASE_TS_DFA or VIR_BASE_MS_DFA. It saves
      all corresponding data that are true data analyzed by block-flow at client side function.
      For example, after doing global reach-def, DU/UD/WEB can be built based on block-flow
      info, and after doing live variable, adjacent matrix (UDG) can be built for RA.
   c. Determine which mem pool is used for current DFA, for reach-def, preferring to BMS, and
      for others, preferring to AMS because they don't existed at same time.
   d. Initialize structure that designed at step b. vscVIR_InitializeBaseTsDFA must be called
      (or vscVIR_InitializeBaseMsDFA for multi-states version).
   e. Call General iterative DFA routine by passing VIR_BASE_TS_DFA or VIR_BASE_MS_DFA.
   f. At destroy time, vscVIR_FinalizeBaseTsDFA or vscVIR_FinalizeBaseMsDFA must be called.
*/

BEGIN_EXTERN_C()

/*
 *  DFA base
 */

/* DFA types VSC supports */
typedef enum _VIR_DFA_TYPE
{
    VIR_DFA_TYPE_REACH_DEF = 0, /* Forward */
    VIR_DFA_TYPE_LIVE_VAR, /* Backward */
    VIR_DFA_TYPE_HELPER, /* Any direction, not for real flow */
    VIR_DFA_TYPE_SCCP, /* Constant propagation */
    VIR_DFA_TYPE_COUNT
}VIR_DFA_TYPE;

typedef struct _VIR_DFA_COMMON_FLAGS
{
    gctUINT                      bFlowBuilt        : 1;  /* Means DFA flow has been built */
    gctUINT                      bFlowInvalidated  : 1;  /* See comments for VIR_BASE_DFA */
    gctUINT                      reserved          : 30;
}VIR_DFA_COMMON_FLAGS;

/* A base DFA structure from which all other different types of DFA will derive.

   NOTE after bFlowInvalidated in common flag is set, flowsize and all related flows (func-
   flow and block-flow) of this DFA is not VALID anymore because for some partially updating,
   we may not update them easily!!! So if bFlowInvalidated has been set, and you still want
   to access flowsize and all related flows (func-flow and block-flow) of this DFA, you must
   redo such global DFA again */
typedef struct _VIR_BASE_DFA
{
    VIR_DFA_TYPE                 dfaType;
    gctINT                       flowSize;

    /* Common flags that independent on concrete dfa info */
    VIR_DFA_COMMON_FLAGS         cmnDfaFlags;

    /* This dfa belongs to which call-graph */
    VIR_CALL_GRAPH*              pOwnerCG;

    /* What type of MM are this DFA built on? */
    VSC_MM*                      pMM;
    /* scratch mem pool can reuse memory be freed */
    VSC_MM*                      pScratchMemPool;
} VIR_BASE_DFA;

void vscVIR_InitializeBaseDFA(VIR_BASE_DFA* pBaseDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                              gctINT flowSize, VSC_MM* pMM, VSC_MM* pScratchMemPool);
void vscVIR_UpdateBaseDFAFlowSize(VIR_BASE_DFA* pBaseDFA, gctINT newFlowSize);
void vscVIR_FinalizeBaseDFA(VIR_BASE_DFA* pBaseDFA);

void vscVIR_SetDFAFlowBuilt(VIR_BASE_DFA* pBaseDFA, gctBOOL bFlowBuilt);
gctBOOL vscVIR_CheckDFAFlowBuilt(VIR_BASE_DFA* pBaseDFA);

/*
 *  Generic two-states (TS) DFA interface (fast version with bit-vector)
 */

/* Flow that one basic block maintains */
typedef struct _VIR_TS_BLOCK_FLOW
{
    /* This block-flow belongs to which BB */
    VIR_BASIC_BLOCK*             pOwnerBB;

    /* 2 local flows that need to be resolved by local analysis done by
       PFN_TS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER */
    VSC_BIT_VECTOR               genFlow;
    VSC_BIT_VECTOR               killFlow;

    /* 2 global flows that need to be resolved by global iterative analysis
       done by PFN_TS_BLOCK_FLOW_ITERATE_RESOLVER and PFN_TS_BLOCK_FLOW_COMBINE_RESOLVER */
    VSC_BIT_VECTOR               inFlow;
    VSC_BIT_VECTOR               outFlow;
}VIR_TS_BLOCK_FLOW;

void vscVIR_InitializeTsBlockFlow(VIR_TS_BLOCK_FLOW* pTsBlkFlow, VIR_BASIC_BLOCK* pOwnerBB, VSC_MM* pMM, gctINT flowSize);
void vscVIR_UpdateTsBlockFlowSize(VIR_TS_BLOCK_FLOW* pTsBlkFlow, gctINT newFlowSize);
void vscVIR_FinalizeTsBlockFlow(VIR_TS_BLOCK_FLOW* pTsBlkFlow);

typedef struct _VIR_TS_FUNC_FLOW
{
    /* This func-flow belongs to which FB */
    VIR_FUNC_BLOCK*              pOwnerFB;

    /* In and out flows of this function */
    VSC_BIT_VECTOR               inFlow;
    VSC_BIT_VECTOR               outFlow;

    /* Array of VIR_TS_BLOCK_FLOW in this func flow. It is maintained only by iterative analyzer.
       !!!!!Note that this array is indexed by id of BB!!!!! */
    VSC_SIMPLE_RESIZABLE_ARRAY   tsBlkFlowArray;
}VIR_TS_FUNC_FLOW;

void vscVIR_InitializeTsFuncFlow(VIR_TS_FUNC_FLOW* pTsFuncFlow, VIR_FUNC_BLOCK* pOwnerFB, VSC_MM* pMM, gctINT flowSize);
void vscVIR_UpdateTsFuncFlowSize(VIR_TS_FUNC_FLOW* pTsFuncFlow, gctINT newFlowSize);
void vscVIR_FinalizeTsFuncFlow(VIR_TS_FUNC_FLOW* pTsFuncFlow);

typedef struct _VIR_BASE_TS_DFA VIR_BASE_TS_DFA;

/* Callbacks for doing real specific ts-DFA, boolean return value means whether flow is changed after
   resolved, if changed return TRUE, otherwise, return FALSE */
typedef void (*PFN_TS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow);
typedef void (*PFN_TS_BLOCK_FLOW_INIT_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow);
typedef gctBOOL (*PFN_TS_BLOCK_FLOW_ITERATE_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow);
typedef gctBOOL (*PFN_TS_BLOCK_FLOW_COMBINE_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow); /* n-to-1 */
typedef gctBOOL (*PFN_TS_BLOCK_FLOW_COMBINE_FROM_CALLEE_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pCallerTsBlockFlow); /* 1-to-1 */
typedef gctBOOL (*PFN_TS_FUNC_FLOW_COMBINE_FROM_CALLERS_RESOLVER)(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_FUNC_FLOW* pCalleeTsFuncFlow); /* n-to-1 */

/* Resolvers for iterative ts-DFA */
typedef struct _VIR_TS_DFA_RESOLVERS
{
    PFN_TS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER       ts_localGenKill_resolver;
    PFN_TS_BLOCK_FLOW_INIT_RESOLVER                ts_initBlockFlow_resolver;
    PFN_TS_BLOCK_FLOW_ITERATE_RESOLVER             ts_iterateBlockFlow_resolver;
    PFN_TS_BLOCK_FLOW_COMBINE_RESOLVER             ts_combineBlockFlow_resolver;
    PFN_TS_BLOCK_FLOW_COMBINE_FROM_CALLEE_RESOLVER ts_combineBlockFlowFromCallee_resolver;
    PFN_TS_FUNC_FLOW_COMBINE_FROM_CALLERS_RESOLVER ts_combineFuncFlowFromCallers_resolver;
}VIR_TS_DFA_RESOLVERS;

/* ts-DFA base, all concrete ts-DFA must be derived from this structure. The general iterative ts-DFA
   routines only recongnizes this structure.

   NOTE that ts-DFA can only process states {0, 1}. */
struct _VIR_BASE_TS_DFA
{
    VIR_BASE_DFA                 baseDFA;

    /* Resolvers that iterative analyzer needs */
    VIR_TS_DFA_RESOLVERS         tsDfaResolvers;

    /* Array of VIR_TS_FUNC_FLOW in this func flow. It is maintained only by iterative analyzer.
    !!!!!Note that this array is indexed by id of FB!!!!! */
    VSC_SIMPLE_RESIZABLE_ARRAY   tsFuncFlowArray;
};

void vscVIR_InitializeBaseTsDFA(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                                gctINT flowSize, VSC_MM* pMM, VIR_TS_DFA_RESOLVERS* pTsDfaResolvers);
void vscVIR_UpdateBaseTsDFAFlowSize(VIR_BASE_TS_DFA* pBaseTsDFA, gctINT newFlowSize);
void vscVIR_FinalizeBaseTsDFA(VIR_BASE_TS_DFA* pBaseTsDFA);
void vscVIR_UpdateTsFlow(VSC_BIT_VECTOR* pTsFlow, VSC_BIT_VECTOR* pDeltaTsFlow, gctBOOL bClearFlow);

/* General iterative ts-DFA routines */
VSC_ErrCode vscVIR_DoForwardIterativeTsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_TS_DFA* pTsDFA, gctBOOL bIPA);
VSC_ErrCode vscVIR_DoBackwardIterativeTsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_TS_DFA* pTsDFA, gctBOOL bIPA);

/*
 *  Generic multi-states (MS) DFA interface (general version with state-vector)
 */

/* Flow that one basic block maintains */
typedef struct _VIR_MS_BLOCK_FLOW
{
    /* This block-flow belongs to which BB */
    VIR_BASIC_BLOCK*             pOwnerBB;

    /* 2 local flows that need to be resolved by local analysis done by
       PFN_MS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER */
    VSC_STATE_VECTOR             genFlow;
    VSC_STATE_VECTOR             killFlow;

    /* 2 global flows that need to be resolved by global iterative analysis
       done by PFN_MS_BLOCK_FLOW_ITERATE_RESOLVER and PFN_MS_BLOCK_FLOW_COMBINE_RESOLVER */
    VSC_STATE_VECTOR             inFlow;
    VSC_STATE_VECTOR             outFlow;
}VIR_MS_BLOCK_FLOW;

void vscVIR_InitializeMsBlockFlow(VIR_MS_BLOCK_FLOW* pMsBlkFlow, VIR_BASIC_BLOCK* pOwnerBB,
                                  VSC_MM* pMM, gctINT flowSize, gctUINT stateCount);
void vscVIR_UpdateMsBlockFlowSize(VIR_MS_BLOCK_FLOW* pMsBlkFlow, gctINT newFlowSize);
void vscVIR_FinalizeMsBlockFlow(VIR_MS_BLOCK_FLOW* pMsBlkFlow);

typedef struct _VIR_MS_FUNC_FLOW
{
    /* This func-flow belongs to which FB */
    VIR_FUNC_BLOCK*              pOwnerFB;

    /* In and out flows of this function */
    VSC_STATE_VECTOR             inFlow;
    VSC_STATE_VECTOR             outFlow;

    /* Array of VIR_MS_BLOCK_FLOW in this func flow. It is maintained only by iterative analyzer.
       !!!!!Note that this array is indexed by id of BB!!!!! */
    VSC_SIMPLE_RESIZABLE_ARRAY   msBlkFlowArray;
}VIR_MS_FUNC_FLOW;

void vscVIR_InitializeMsFuncFlow(VIR_MS_FUNC_FLOW* pMsFuncFlow, VIR_FUNC_BLOCK* pOwnerFB,
                                 VSC_MM* pMM, gctINT flowSize, gctUINT stateCount);
void vscVIR_UpdateMsFuncFlowSize(VIR_MS_FUNC_FLOW* pMsFuncFlow, gctINT newFlowSize);
void vscVIR_FinalizeMsFuncFlow(VIR_MS_FUNC_FLOW* pMsFuncFlow);

typedef struct _VIR_BASE_MS_DFA VIR_BASE_MS_DFA;

/* Callbacks for doing real specific ms-DFA, boolean return value means whether flow is changed after
   resolved, if changed return TRUE, otherwise, return FALSE */
typedef void (*PFN_MS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_BLOCK_FLOW* pMsBlockFlow);
typedef void (*PFN_MS_BLOCK_FLOW_INIT_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_BLOCK_FLOW* pMsBlockFlow);
typedef gctBOOL (*PFN_MS_BLOCK_FLOW_ITERATE_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_BLOCK_FLOW* pMsBlockFlow);
typedef gctBOOL (*PFN_MS_BLOCK_FLOW_COMBINE_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_BLOCK_FLOW* pMsBlockFlow); /* n-to-1 */
typedef gctBOOL (*PFN_MS_BLOCK_FLOW_COMBINE_FROM_CALLEE_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_BLOCK_FLOW* pCallerMsBlockFlow); /* 1-to-1 */
typedef gctBOOL (*PFN_MS_FUNC_FLOW_COMBINE_FROM_CALLERS_RESOLVER)(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_MS_FUNC_FLOW* pCalleeMsFuncFlow); /* n-to-1 */

/* Resolvers for iterative ms-DFA */
typedef struct _VIR_MS_DFA_RESOLVERS
{
    PFN_MS_BLOCK_FLOW_LOCAL_GENKILL_RESOLVER       ms_localGenKill_resolver;
    PFN_MS_BLOCK_FLOW_INIT_RESOLVER                ms_initBlockFlow_resolver;
    PFN_MS_BLOCK_FLOW_ITERATE_RESOLVER             ms_iterateBlockFlow_resolver;
    PFN_MS_BLOCK_FLOW_COMBINE_RESOLVER             ms_combineBlockFlow_resolver;
    PFN_MS_BLOCK_FLOW_COMBINE_FROM_CALLEE_RESOLVER ms_combineBlockFlowFromCallee_resolver;
    PFN_MS_FUNC_FLOW_COMBINE_FROM_CALLERS_RESOLVER ms_combineFuncFlowFromCallers_resolver;
}VIR_MS_DFA_RESOLVERS;

/* ms-DFA base, all concrete ms-DFA must be derived from this structure. The general iterative ms-DFA
   routines only recongnizes this structure.

   NOTE that ms-DFA can only process states {0, 1, ..., stateCount -1} */
struct _VIR_BASE_MS_DFA
{
    VIR_BASE_DFA                 baseDFA;

    /* How many states this DFA can process. */
    gctUINT                      stateCount;

    /* Resolvers that iterative analyzer needs */
    VIR_MS_DFA_RESOLVERS         msDfaResolvers;

    /* Array of VIR_MS_FUNC_FLOW in this func flow. It is maintained only by iterative analyzer.
    !!!!!Note that this array is indexed by id of FB!!!!! */
    VSC_SIMPLE_RESIZABLE_ARRAY   msFuncFlowArray;
};

void vscVIR_InitializeBaseMsDFA(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                                gctINT flowSize, gctUINT stateCount, VSC_MM* pMM, VIR_MS_DFA_RESOLVERS* pMsDfaResolvers);
void vscVIR_UpdateBaseMsDFAFlowSize(VIR_BASE_MS_DFA* pBaseMsDFA, gctINT newFlowSize);
void vscVIR_FinalizeBaseMsDFA(VIR_BASE_MS_DFA* pBaseMsDFA);

/* General iterative ms-DFA routines */
VSC_ErrCode vscVIR_DoForwardIterativeMsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_MS_DFA* pMsDFA, gctBOOL bIPA);
VSC_ErrCode vscVIR_DoBackwardIterativeMsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_MS_DFA* pMsDFA, gctBOOL bIPA);

/*
 *  DU analysis
 */

/* DU/UD/web build. Note that if we have correct SSA built, we don't need this actually. But
   with this provision, we can provide more choice for opts even when SSA can be enabled. */

#define VIR_INVALID_DEF_INDEX         INVALID_BT_ENTRY_ID
#define VIR_INVALID_USAGE_INDEX       INVALID_BT_ENTRY_ID
#define VIR_INVALID_WEB_INDEX         INVALID_BT_ENTRY_ID

#define VIR_INVALID_REG_NO            VIR_INVALID_ID

#define VIR_ANY_DEF_INST              (VIR_Instruction*)(-1) /* For def-key search only */
#define VIR_UNDEF_INST                (VIR_Instruction*)(-2) /* For undefined usage */
#define VIR_HW_SPECIAL_DEF_INST       (VIR_Instruction*)(-3) /* Some regs are defined by HW like input, but they are not true input */
#define VIR_INPUT_DEF_INST            (VIR_Instruction*)(-4) /* Input define from FFU or pre-shader-stage */
#define VIR_OUTPUT_USAGE_INST         (VIR_Instruction*)(-5) /* Output usage to FFU or next-shader-stage */

#define VIR_IS_IMPLICIT_DEF_INST(pDefInst)     ((pDefInst) == VIR_HW_SPECIAL_DEF_INST ||           \
                                                (pDefInst) == VIR_INPUT_DEF_INST)

#define VIR_IS_OUTPUT_USAGE_INST(pUsageInst)   ((pUsageInst) == VIR_OUTPUT_USAGE_INST               || \
                                                VIR_Inst_GetOpcode((pUsageInst)) == VIR_OP_EMIT0    || \
                                                VIR_Inst_GetOpcode((pUsageInst)) == VIR_OP_EMIT     || \
                                                VIR_Inst_GetOpcode((pUsageInst)) == VIR_OP_EMIT_STREAM)

#define VIR_IS_IMPLICIT_USAGE_INST(pUsageInst) ((pUsageInst) == VIR_OUTPUT_USAGE_INST)
#define VIR_IS_SPECIAL_INST(pDefInst)          ((pDefInst) == VIR_ANY_DEF_INST        || \
                                                (pDefInst) == VIR_UNDEF_INST          || \
                                                (pDefInst) == VIR_HW_SPECIAL_DEF_INST || \
                                                (pDefInst) == VIR_INPUT_DEF_INST      || \
                                                (pDefInst) == VIR_OUTPUT_USAGE_INST)

#define VIR_HALF_CHANNEL_MASK_NONE    0x0
#define VIR_HALF_CHANNEL_MASK_LOW     0x1 /* T0 */
#define VIR_HALF_CHANNEL_MASK_HIGH    0x2 /* T1 */
#define VIR_HALF_CHANNEL_MASK_FULL    (VIR_HALF_CHANNEL_MASK_LOW | VIR_HALF_CHANNEL_MASK_HIGH)

typedef struct VIR_NATIVE_DEF_FLAGS
{
    gctUINT                      bIsInput             : 1; /* Attributes/inputs/varying inputs */
    gctUINT                      bIsOutput            : 1; /* Output/varyings output */
    gctUINT                      bIsPerVtxCp          : 1; /* Per vertex/control-point */
    gctUINT                      bIsPerPrim           : 1; /* Per prim */
    gctUINT                      bHwSpecialInput      : 1; /* A hw special input,
                                                            * so far we only have output sample-mask,
                                                            * check needHwSpecialDef in operandInfo.
                                                            */

    gctUINT                      reserved             : 27;
}VIR_NATIVE_DEF_FLAGS;

typedef struct VIR_DEDUCED_DEF_FLAGS
{
    gctUINT         bNoUsageCrossRoutine       : 1; /* Means all usages are in same routine as this def */
    gctUINT         bDynIndexed                : 1; /* This def will be dynamically indexed access by indexing reg */
    gctUINT         bIndexingReg               : 1; /* This def is for indexing reg Ro of Rb[Ro.single_channel] */
    gctUINT         bHasUsageOnNoSwizzleInst   : 1; /* This def has an usage on no swizzle support instruction (for example,
                                                       EVIS instruction, swizzle instruction).
                                                       NOTE it will be removed after bin is considered as bin implies
                                                       concept of swizzle!!! */
    gctUINT         bHasUsageOnFalseDepInst    : 1; /* This def has an usage on ST as src2.
                                                       On some HW, there is no per-component dependency support
                                                       for LD/ST def. For ST, def is src2, if it is temp. */

    gctUINT         reserved                   : 27;

}VIR_DEDUCED_DEF_FLAGS;

typedef struct _VIR_DEF_FLAGS
{
    /* These must be specified by shader itself */
    VIR_NATIVE_DEF_FLAGS         nativeDefFlags;

    /* These can be deduced by analysis */
    VIR_DEDUCED_DEF_FLAGS        deducedDefFlags;
}VIR_DEF_FLAGS;

typedef struct _VIR_DEF_KEY
{
    VIR_Instruction*             pDefInst;

    /* Note that for indexing access, reg number in this indexing range shares same pDefInst.
       That's why regNo is as part of key */
    gctUINT                      regNo;

    /* Which channel on regNo in this def inst */
    gctUINT8                     channel;
}VIR_DEF_KEY;

typedef struct _VIR_DU_CHAIN_USAGE_NODE
{
    /* Next usage in DU chain. It must be put at FIRST place!!!! */
    VSC_UNI_LIST_NODE         uniLstNode;

    gctUINT                   usageIdx;
}VIR_DU_CHAIN_USAGE_NODE;

typedef struct _VIR_1st_DEF_INFO
{
    gctUINT                 firstDefIndex;
    gctUINT                 lastDefIndex;
} VIR_1st_DEF_INFO;

void vscUSGN_Initialize(VIR_DU_CHAIN_USAGE_NODE* pUsageNode, gctUINT usageIndex);
void vscUSGN_Finalize(VIR_DU_CHAIN_USAGE_NODE* pUsageNode);
#define CAST_USGN_2_ULN(pUsageNode)                 (VSC_UNI_LIST_NODE*)(pUsageNode)
#define CAST_ULN_2_USGN(pUln)                       (VIR_DU_CHAIN_USAGE_NODE*)(pUln)
#define USGN_GET_NEXT_USAGE_NODE(pUsageNode)        CAST_ULN_2_USGN(vscULN_GetNextNode(CAST_USGN_2_ULN(pUsageNode)))
#define USGN_GET_USAGE_INDEX(pUsageNode)            ((pUsageNode)->usageIdx)

typedef VSC_UNI_LIST                   VIR_DU_CHAIN;
#define DU_CHAIN_INITIALIZE(pDuChain)               vscUNILST_Initialize((pDuChain), gcvFALSE)
#define DU_CHAIN_FINALIZE(pDuChain)                 vscUNILST_Finalize((pDuChain))
#define DU_CHAIN_ADD_USAGE(pDuChain, pUsageNode)    vscUNILST_Append((pDuChain), CAST_USGN_2_ULN((pUsageNode)))
#define DU_CHAIN_GET_FIRST_USAGE(pDuChain)          CAST_ULN_2_USGN(vscUNILST_GetHead((pDuChain)))
#define DU_CHAIN_REMOVE_USAGE(pDuChain, pUsageNode) vscUNILST_Remove((pDuChain), CAST_USGN_2_ULN((pUsageNode)))
#define DU_CHAIN_CHECK_EMPTY(pDuChain)              vscUNILST_IsEmpty((pDuChain))
#define DU_CHAIN_GET_USAGE_COUNT(pDuChain)          vscUNILST_GetNodeCount((pDuChain))

typedef VSC_UL_ITERATOR VSC_DU_ITERATOR;
#define VSC_DU_ITERATOR_INIT(iter, pDuChain)        vscULIterator_Init((iter), pDuChain)
#define VSC_DU_ITERATOR_FIRST(iter)                 (VIR_DU_CHAIN_USAGE_NODE*)vscULIterator_First((iter))
#define VSC_DU_ITERATOR_NEXT(iter)                  (VIR_DU_CHAIN_USAGE_NODE*)vscULIterator_Next((iter))
#define VSC_DU_ITERATOR_LAST(iter)                  (VIR_DU_CHAIN_USAGE_NODE*)vscULIterator_Last((iter))

typedef VSC_SIMPLE_RESIZABLE_ARRAY     VIR_UD_CHAIN;
#define UD_CHAIN_INITIALIZE(pUdChain)               vscSRARR_Initialize((pUdChain), pMM, VIR_CHANNEL_NUM, sizeof(gctUINT), DEF_INDEX_CMP)
#define UD_CHAIN_FINALIZE(pUdChain)                 vscSRARR_Finalize((pUdChain))
#define UD_CHAIN_ADD_DEF(pUdChain, defIdx)          vscSRARR_AddElement((pUdChain), &(defIdx))
#define UD_CHAIN_GET_DEF(pUdChain, idx)             (vscSRARR_GetElement((pUdChain), (idx)) == gcvNULL) ? VIR_INVALID_DEF_INDEX : \
                                                    *(gctUINT*)vscSRARR_GetElement((pUdChain), (idx));
#define UD_CHAIN_GET_FIRST_DEF(pUdChain)            UD_CHAIN_GET_DEF((pUdChain), 0)
#define UD_CHAIN_REMOVE_DEF(pUdChain, defIdx)       vscSRARR_RemoveElementByContent((pUdChain), &(defIdx))
#define UD_CHAIN_CHECK_EMPTY(pUdChain)              (vscSRARR_GetElementCount((pUdChain)) == 0)
#define UD_CHAIN_GET_DEF_COUNT(pUdChain)            vscSRARR_GetElementCount((pUdChain))
#define UD_CHAIN_CHECK_DEF(pUdChain, defIdx)        (vscSRARR_GetElementIndexByContent((pUdChain), &(defIdx)) != VSC_INVALID_ARRAY_INDEX)

typedef struct _VIR_DEF
{
    /* Def key which is as hash key */
    VIR_DEF_KEY                  defKey;

    /* Def flags */
    VIR_DEF_FLAGS                flags;

    /* Orignal write mask that inst of def natively should hold */
    VIR_Enable                   OrgEnableMask;

    /* It is only used for single-t when dual16 is on. For other cases, it
       must set to VIR_HALF_CHANNEL_MASK_FULL to indicate full channel is
       touched */
    gctUINT8                     halfChannelMask;

    /* To accelerate analysis speed, make all def with same regNo are linked.
       Note that seq of such link is reversed-ordered by occurrence of inst
       that defines this def.  */
    gctUINT                      nextDefIdxOfSameRegNo;

    /* Web related info */
    gctUINT                      webIdx;
    gctUINT                      nextDefInWebIdx;

    /* DU-chain */
    VIR_DU_CHAIN                 duChain;
}VIR_DEF;

#define IS_VALID_DEF(pDef)                                               \
    ((pDef)->defKey.pDefInst != gcvNULL &&                               \
     (pDef)->defKey.regNo != VIR_INVALID_ID &&                           \
     (pDef)->defKey.channel != VIR_CHANNEL_ANY)

#define VIR_ANY_USAGE_OPERAND         (VIR_Operand*)(-1) /* For usage-key search only */
#define VIR_INVALID_USAGE_OPERAND     (VIR_Operand*)(-2)


typedef struct _VIR_USAGE_KEY
{
    VIR_Instruction*             pUsageInst;

    /* For output (pUsageInst must be set to VIR_OUTPUT_USAGE_INST or EMIT), use
       output regNo as operand, so we can make a diff with different output usages */
    VIR_Operand*                 pOperand;

    /* Because we use operand to differiate usages of same inst, for the case of
       Rb[Ro.single_channel] where Ro share same operand with host operand, to
       differiate Ro usage with usage of host operand, bIsIndexingRegUsage must be
       TRUE if this usage is for Ro. So we can regard bIsIndexingRegUsage as part
       of pOperand!! */
    gctBOOL                      bIsIndexingRegUsage;

}VIR_USAGE_KEY;

typedef struct _VIR_USAGE
{
    /* Usage key which is as hash key */
    VIR_USAGE_KEY                usageKey;

    /* Real usage channel-mask. Sometimes, swizzle of operand may be superset of
       real used channel-mask (swizzle). We mark this diff here, so client won't
       need check it anywhere */
    gctUINT                      realChannelMask : 8;

    /* It is only used for single-t when dual16 is on. For other cases, it
       must set to VIR_HALF_CHANNEL_MASK_FULL to indicate full channel is
       touched */
    gctUINT                      halfChannelMask : 8;

    /* Web related info, we only record ONCE for each usage who shares several defs */
    gctUINT                      webIdx;
    gctUINT                      nextWebUsageIdx;

    /* UD-chain */
    VIR_UD_CHAIN                 udChain;
}VIR_USAGE;

#define IS_VALID_USAGE(pUsage)                                              \
    ((pUsage)->usageKey.pUsageInst != gcvNULL &&                            \
     (pUsage)->usageKey.pOperand != VIR_INVALID_USAGE_OPERAND)

typedef enum _VIR_WEB_TYPE
{
    VIR_WEB_TYPE_UNKNOWN = 0,
    VIR_WEB_TYPE_TEMP,
    VIR_WEB_TYPE_ADDR,
    VIR_WEB_TYPE_PRED
}VIR_WEB_TYPE;

typedef struct _VIR_WEB
{
    VIR_WEB_TYPE                 webType     : 4;
    /* Channel mask for this web. This channel mask is mask of
       channel of all defs belonging to this web */
    gctINT                       channelMask : 8;
    gctINT                       numOfDef    : 20 ;

    gctUINT                      firstDefIdx;
    gctUINT                      firstUsageIdx;
}VIR_WEB;

typedef struct _VIR_DEF_USAGE_INFO
{
    VIR_BASE_TS_DFA              baseTsDFA;  /* must be the first field, some place use pointer
                                              * to this field as pointer to VIR_DEF_USAGE_INFO !
                                              */

    /* All defs in shader */
    VSC_BLOCK_TABLE              defTable;

    /* hash table for first def index of a regNo in defTable  */
    VSC_HASH_TABLE             * pFirstDefTable;

    /* All usages in shader */
    VSC_BLOCK_TABLE              usageTable;

    /* All webs in shader */
    VSC_BLOCK_TABLE              webTable;

    /* Maximum virtual reg number that this DU-info maintains */
    gctUINT                      maxVirRegNo;

    /* Client can make DU-chain (as well as UD-chain) built not
       along with building of reach-def flow DFA. This is for
       CPU performance consideration. So we need trace its status */
    gctBOOL                      bDUUDChainBuilt;

    /* Client can make web table built not along with building
       of def table and usage table. This is for CPU performance
       consideration. So we need trace its status */
    gctBOOL                      bWebTableBuilt;

    /* compute hash key with regNo and Inst if true,
       otherwise only hash regNo */
    gctBOOL                      bHashRegNoInst;

    /* Memory pool that this DU-info is built on */
    VSC_PRIMARY_MEM_POOL         pmp;
}VIR_DEF_USAGE_INFO;

#define GET_DEF_BY_IDX(pDefTable, defIdx)          (VIR_DEF*)BT_GET_ENTRY_DATA(pDefTable, (defIdx))
#define GET_USAGE_BY_IDX(pUsageTable, usageIdx)    (VIR_USAGE*)BT_GET_ENTRY_DATA(pUsageTable, (usageIdx))
#define GET_WEB_BY_IDX(pWebTable, webIdx)          (VIR_WEB*)BT_GET_ENTRY_DATA(pWebTable, (webIdx))

/* Utilities for d-u related analysis */

gctBOOL vscVIR_QueryRealWriteVirRegInfo(VIR_Shader* pShader,
                                        VIR_Instruction* pInst,
                                        VIR_Enable *pDefEnableMask,
                                        gctUINT8 *pHalfChannelMask,
                                        gctUINT* pFirstRegNo,
                                        gctUINT* pRegNoRange,
                                        VIR_NATIVE_DEF_FLAGS* pNativeDefFlags,
                                        gctBOOL* pIsIndexing);

#define MAKE_DEAD_DEF_IN_SEPERATED_WEB     0

/* Build routines */

VSC_ErrCode vscVIR_BuildDefUsageInfo(VIR_CALL_GRAPH*     pCg,
                                     VIR_DEF_USAGE_INFO* pDuInfo,
                                     gctBOOL             bBuildDUUDChain, /* DUUD-chain will also be built if TRUE */
                                     gctBOOL             bBuildWeb /* Web-table will also be built if TRUE */
                                     );
VSC_ErrCode vscVIR_BuildDUUDChain(VIR_CALL_GRAPH*     pCg,
                                  VIR_DEF_USAGE_INFO* pDuInfo,
                                  gctBOOL             bForceToBuild  /* If set to TRUE, previous DUUD
                                                                        chain will be destroyed firstly */
                                  );
VSC_ErrCode vscVIR_BuildWebs(VIR_CALL_GRAPH*     pCg,
                             VIR_DEF_USAGE_INFO* pDuInfo,
                             gctBOOL             bForceToBuild  /* If set to TRUE, previous web
                                                                   table will be destroyed firstly */
                             );

VSC_ErrCode vscVIR_DestroyDefUsageInfo(VIR_DEF_USAGE_INFO* pDuInfo); /* All du info including web will be destroyed */
VSC_ErrCode vscVIR_DestoryDUUDChain(VIR_DEF_USAGE_INFO* pDuInfo); /* Only DUUD chain will be destroyed */
VSC_ErrCode vscVIR_DestoryWebs(VIR_DEF_USAGE_INFO* pDuInfo); /* Only web table will be destroyed */

/* Partially update routines */

void vscVIR_AddNewDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      VIR_NATIVE_DEF_FLAGS* pNativeDefFlags, /* If NULL, default native-def-flags will be used */
                      gctUINT* pRetDefIdxArray  /* VIR_CHANNEL_NUM*defRegNoRange sized array to
                                                   return def index for each enabled channel that
                                                   is new added into def-table */
                     );

void vscVIR_DeleteDef(VIR_DEF_USAGE_INFO* pDuInfo,
                      VIR_Instruction* pDefInst,
                      gctUINT firstDefRegNo,
                      gctUINT defRegNoRange,
                      VIR_Enable defEnableMask,
                      gctUINT8 halfChannelMask,
                      gctUINT* pRetDefIdxArray  /* VIR_CHANNEL_NUM*defRegNoRange sized array to
                                                   return def index for each enabled channel that
                                                   is just deleted from def-table */
                     );

void vscVIR_AddNewUsageToDef(VIR_DEF_USAGE_INFO* pDuInfo,
                             VIR_Instruction* pDefInst,
                             VIR_Instruction* pUsageInst,
                             VIR_Operand* pOperand,
                             gctBOOL bIsIndexingRegUsage,
                             gctUINT firstUsageRegNo,
                             gctUINT usageRegNoRange,
                             VIR_Enable defEnableMask,
                             gctUINT8 halfChannelMask,
                             gctUINT* pRetUsageIdx);

void vscVIR_DeleteUsage(VIR_DEF_USAGE_INFO* pDuInfo,
                        VIR_Instruction* pDefInst, /* Can be set as VIR_ANY_DEF_INST */
                        VIR_Instruction* pUsageInst,
                        VIR_Operand* pOperand,
                        gctBOOL bIsIndexingRegUsage,
                        gctUINT firstUsageRegNo,
                        gctUINT usageRegNoRange,
                        VIR_Enable defEnableMask,
                        gctUINT8 halfChannelMask,
                        gctUINT* pRetUsageIdx);

void vscVIR_MergeTwoWebs(VIR_DEF_USAGE_INFO* pDuInfo,
                         gctUINT dstWebIdx,
                         gctUINT srcWebIdx);

gctUINT vscVIR_FindFirstDefIndex(VIR_DEF_USAGE_INFO* pDuInfo,
                                            gctUINT firstDefRegNo);

gctUINT vscVIR_FindFirstDefIndexWithChannel(VIR_DEF_USAGE_INFO* pDuInfo,
                                                       gctUINT             FirstDefRegNo,
                                                       gctUINT8            Channel);

/* Query routines */

VIR_DEF* vscVIR_GetDef(VIR_DEF_USAGE_INFO*        pDuInfo,
                       VIR_Instruction*           pDefInst,
                       gctUINT                    defRegNo,
                       gctUINT8                   defChannel,
                       VIR_WEB**                  ppWeb);

VIR_USAGE* vscVIR_GetUsage(VIR_DEF_USAGE_INFO*    pDuInfo,
                           VIR_Instruction*       pUsageInst,
                           VIR_Operand*           pUsageOperand,
                           gctBOOL                bIsIndexingRegUsage,
                           VIR_WEB**              ppWeb);

typedef struct _VIR_GENERAL_DU_ITERATOR
{
    VIR_DEF_USAGE_INFO*          pDuInfo;
    gctBOOL                      bSameBBOnly;
    VSC_DU_ITERATOR              duIter;
    VIR_DEF_KEY                  defKey;
}VIR_GENERAL_DU_ITERATOR;

void vscVIR_InitGeneralDuIterator(VIR_GENERAL_DU_ITERATOR* pIter,
                                  VIR_DEF_USAGE_INFO*      pDuInfo,
                                  VIR_Instruction*         pDefInst,
                                  gctUINT                  defRegNo,
                                  gctUINT8                 defChannel,
                                  gctBOOL                  bSameBBOnly);
VIR_USAGE* vscVIR_GeneralDuIterator_First(VIR_GENERAL_DU_ITERATOR* pIter);
VIR_USAGE* vscVIR_GeneralDuIterator_Next(VIR_GENERAL_DU_ITERATOR* pIter);

typedef struct _VIR_GENERAL_UD_ITERATOR
{
    VIR_DEF_USAGE_INFO*          pDuInfo;
    gctBOOL                      bSameBBOnly;
    VIR_USAGE_KEY                usageKey;
    VIR_UD_CHAIN*                pUdChain;
    gctUINT                      curIdx;
}VIR_GENERAL_UD_ITERATOR;

void vscVIR_InitGeneralUdIterator(VIR_GENERAL_UD_ITERATOR*  pIter,
                                  VIR_DEF_USAGE_INFO*       pDuInfo,
                                  VIR_Instruction*          pUsageInst,
                                  VIR_Operand*              pUsageOperand,
                                  gctBOOL                   bIsIndexingRegUsage,
                                  gctBOOL                   bSameBBOnly);
VIR_DEF* vscVIR_GeneralUdIterator_First(VIR_GENERAL_UD_ITERATOR* pIter);
VIR_DEF* vscVIR_GeneralUdIterator_Next(VIR_GENERAL_UD_ITERATOR* pIter);

/* That a def has homonymy defs means there is at least another def that has same defRegNo
   and defChannel as ones in requested def */
gctBOOL vscVIR_HasHomonymyDefs(VIR_DEF_USAGE_INFO*          pDuInfo,
                               VIR_Instruction*             pDefInst,
                               gctUINT                      defRegNo,
                               gctUINT8                     defChannel,
                               gctBOOL                      bSameBBOnly);

VIR_DEF* vscVIR_GetNextHomonymyDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                                   VIR_Instruction*         pDefInst,
                                   gctUINT                  defRegNo,
                                   gctUINT8                 defChannel,
                                   gctBOOL                  bSameBBOnly);
VIR_DEF* vscVIR_GetPrevHomonymyDef(VIR_DEF_USAGE_INFO*      pDuInfo,
                                   VIR_Instruction*         pDefInst,
                                   gctUINT                  defRegNo,
                                   gctUINT8                 defChannel,
                                   gctBOOL                  bSameBBOnly);

typedef struct _VIR_HOMONYMY_DEF_ITERATOR
{
    VIR_DEF_USAGE_INFO*          pDuInfo;
    gctBOOL                      bSameBBOnly;
    gctBOOL                      bBackward;
    VIR_DEF_KEY                  defKey;
}VIR_HOMONYMY_DEF_ITERATOR;

void vscVIR_InitHomonymyDefIterator(VIR_HOMONYMY_DEF_ITERATOR* pIter,
                                    VIR_DEF_USAGE_INFO*        pDuInfo,
                                    VIR_Instruction*           pDefInst,
                                    gctUINT                    defRegNo,
                                    gctUINT8                   defChannel,
                                    gctBOOL                    bSameBBOnly,
                                    gctBOOL                    bBackward);
VIR_DEF* vscVIR_HomonymyDefIterator_First(VIR_HOMONYMY_DEF_ITERATOR* pIter);
VIR_DEF* vscVIR_HomonymyDefIterator_Next(VIR_HOMONYMY_DEF_ITERATOR* pIter);

/* Given an expected unique def (or usage inst/usage operand), check whether it is
   really the unique one that usage (or def) has, if yes, return TRUE, otherwise,
   return FALSE. */
gctBOOL vscVIR_IsUniqueUsageInstOfDefInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_Instruction*    pDefInst,
                                          VIR_Instruction*    pExpectedUniqueUsageInst, /* Expected unique usage inst */
                                          VIR_Operand*        pExpectedUniqueUsageOperand, /* If not NULL, means usage-operand also needs to be unique */
                                          gctBOOL             bIsIdxingRegForExpectedUniqueUsage,/* Valid only when pExpectedUniqueUsageOperand is not NULL */
                                          VIR_Instruction**   ppFirstOtherUsageInst, /* If not unique, it returns first other usage inst when searching */
                                          VIR_Operand**       ppFirstOtherUsageOperand, /* Valid only when pExpectedUniqueUsageOperand is not NULL */
                                          gctBOOL*            pIsIdxingRegForFirstOtherUsage);   /* Valid only when pExpectedUniqueUsageOperand is not NULL */
gctBOOL vscVIR_IsUniqueDefInstOfUsageInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_Instruction*    pUsageInst,
                                          VIR_Operand*        pUsageOperand,
                                          gctBOOL             bIsIndexingRegUsage,
                                          VIR_Instruction*    pExpectedUniqueDefInst, /* Expected unique def inst */
                                          VIR_Instruction**   ppFirstDefInstOrFirstOtherDefInst);/* When the expected unique def instruction is not NULL,
                                                                                                 ** then it returns first other def inst when searching.
                                                                                                 ** Otherwise it returns the first def instruction we meet when searching. */

/* Given a register no, check whether it has a unique def instruction, if yes, return TRUE and the def instruction. */
gctBOOL vscVIR_IsRegNoHasUniqueDefInst(VIR_DEF_USAGE_INFO*      pDuInfo,
                                       VIR_VirRegId             regNo,
                                       VIR_Instruction**        ppUniqueDefInst);

/* Given a register no, if one of its DEF is conditional write, check whether all its def instructions have the same writeMask. */
gctBOOL vscVIR_IsRegAllDefHaveSameWriteMask(VIR_DEF_USAGE_INFO*      pDuInfo,
                                            VIR_VirRegId             regNo);

/*
** Check whether this instruction is a definite write.
**      1) non-conditional write.
**      2) conditional write, but use the fixed writeMask.
**      3) this DEF is the unique DEF.
*/
gctBOOL vscVIR_IsInstDefiniteWrite(VIR_DEF_USAGE_INFO*   pDuInfo,
                                   VIR_Instruction*      pInst,
                                   VIR_VirRegId          regNo,
                                   gctBOOL               bCheckDef);

gctBOOL vscVIR_IsDefInstAndUsageInstSameBranch(VIR_DEF_USAGE_INFO* pDuInfo,
                                               VIR_Instruction*    pUsageInst,
                                               VIR_Instruction*    pDefInst);

/* Check whether a def (or usage) has unique usage (or def), if yes, just return this
   unique one. */
gctBOOL vscVIR_DoesDefInstHaveUniqueUsageInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                              VIR_Instruction*    pDefInst,
                                              gctBOOL             bUniqueOperand, /* If TRUE, operand of usage must be also unique even if usage-inst is unique */
                                              VIR_Instruction**   ppUniqueUsageInst, /* If unique, it returns this unique usage inst */
                                              VIR_Operand**       ppUniqueUsageOperand, /* Valid only when bUniqueOperand is TRUE */
                                              gctBOOL*            pIsIdxingRegForUniqueUsage);   /* Valid only when bUniqueOperand is TRUE */
gctBOOL vscVIR_DoesUsageInstHaveUniqueDefInst(VIR_DEF_USAGE_INFO* pDuInfo,
                                              VIR_Instruction*    pUsageInst,
                                              VIR_Operand*        pUsageOperand,
                                              gctBOOL             bIsIndexingRegUsage,
                                              VIR_Instruction**   ppUniqueDefInst);              /* If unique, it returns this unique def inst */

/* Check an usage (or def) is the unique one for each of defs in its UD chain (or each of
   usages in its DU chain) */
gctBOOL vscVIR_IsUniqueUsageInstOfDefsInItsUDChain(VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_Instruction*    pUsageInst,
                                                   VIR_Operand*        pUsageOperand,
                                                   gctBOOL             bIsIndexingRegUsage,
                                                   VIR_Instruction**   ppFirstMultiUsageDefInst,
                                                   VIR_Instruction**   ppFirstOtherUsageInst);
gctBOOL vscVIR_IsUniqueDefInstOfUsagesInItsDUChain(VIR_DEF_USAGE_INFO* pDuInfo,
                                                   VIR_Instruction*    pDefInst,
                                                   VIR_Instruction**   ppFirstOtherDefInst,
                                                   VIR_Instruction**   ppFirstMultiDefUsageInst);

gctBOOL vscVIR_RedefineBetweenInsts(IN VSC_MM                   *pMM,
                                    IN VIR_DEF_USAGE_INFO       *duInfo,
                                    IN VIR_Instruction          *startInst,
                                    IN VIR_Instruction          *endInst,
                                    IN VIR_Operand              *srcOpndOfStartInst,
                                    OUT VIR_Instruction         **redefInst);

/* Find the unique nearest defined instruction. */
typedef gctBOOL (*PFN_VSC_DEF_CMP)(VIR_Instruction* pDefInst);

gctBOOL vscVIR_FindUniqueNearestDefInst(
    IN VIR_DEF_USAGE_INFO*          pDuInfo,
    IN VIR_Instruction*             pUsageInst,
    IN VIR_Operand*                 pUsageOpnd,
    IN VIR_Instruction*             pStartSearchInst,
    IN PFN_VSC_DEF_CMP              pDefCmpFunc,
    INOUT VIR_Instruction**         ppNearestDefInst
    );

/*
 *  LV analysis
 */

typedef struct _VIR_LIVENESS_INFO
{
    VIR_BASE_TS_DFA              baseTsDFA;

    /* D-U info that lv needs */
    VIR_DEF_USAGE_INFO*          pDuInfo;

    /* Memory pool that this LV-info is built on */
    VSC_PRIMARY_MEM_POOL         pmp;

    /* scratch mem pool can reuse memory be freed */
    VSC_MM                       scratchMemPool;
}VIR_LIVENESS_INFO;

/* Do aggressive liveness IPA.
   We hope use it to get accurate liveness at any spot. Since when a function is called from
   multiple call sites, at each time of calling, live info should be diff based on liveness
   that're surrounding the 'CALL' instruction. So later, for any user, especially RA which
   is trustily audience of LV, need do live info check at any call site with extra live info
   that is the !DELTA! of outflow of 'CALL' block and outflow of callee.*/
#define ENABLE_AGGRESSIVE_IPA_LIVENESS     (SUPPORT_IPA_DFA && 0)

/* Build routines */
VSC_ErrCode vscVIR_BuildLivenessInfo(VIR_CALL_GRAPH* pCg,
                                     VIR_LIVENESS_INFO* pLvInfo,
                                     VIR_DEF_USAGE_INFO* pDuInfo);
VSC_ErrCode vscVIR_DestroyLivenessInfo(VIR_LIVENESS_INFO* pLvInfo);

END_EXTERN_C()

#endif /* __gc_vsc_vir_dfa_h_ */

