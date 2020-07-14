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


#ifndef __gc_vsc_vir_inline_h_
#define __gc_vsc_vir_inline_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

#define VSC_IL_MEM_BLK_SIZE         2048

/* Right now, all the chips have the same call stack limitation, 4. */
#define VSC_MAX_CALL_STACK_DEPTH    3

typedef struct _VSC_IL_PASS_DATA
{
    gctUINT32                   passIndex;
    gctBOOL                     bCheckAlwaysInlineOnly;
} VSC_IL_PASS_DATA;

typedef struct _VSC_IL_INST_LIST_NODE
{
    VSC_UNI_LIST_NODE           node;
    VIR_Instruction             *inst;
}VSC_IL_INST_LIST_NODE;

#define CAST_INST_NODE_2_ULN(pInstNode)            (VSC_UNI_LIST_NODE*)(pInstNode)
#define CAST_ULN_2_INST_NODE(pUln)                 (VSC_IL_INST_LIST_NODE*)(pUln)

#define INST_NODE_GET_NEXT(pInstNode)               CAST_ULN_2_USGN(vscULN_GetNextNode(CAST_USGN_2_ULN(pInstNode)))
#define INST_NODE_GET_INST(pInstNode)              ((pInstNode)->inst)

typedef VSC_UNI_LIST                   VSC_IL_INST_LIST;
#define INST_LIST_INITIALIZE(pInstList)             vscUNILST_Initialize((pInstList), gcvFALSE)
#define INST_LIST_FINALIZE(pInstList)               vscUNILST_Finalize((pInstList))
#define INST_LIST_ADD_NODE(pInstList, pInstNode)    vscUNILST_Append((pInstList), CAST_USGN_2_ULN((pInstNode)))

typedef VSC_UL_ITERATOR VSC_IL_INST_LIST_ITERATOR;
#define VSC_IL_INST_LIST_ITERATOR_INIT(iter, pInstList)       vscULIterator_Init((iter), pInstList)
#define VSC_IL_INST_LIST_ITERATOR_FIRST(iter)                 (VSC_IL_INST_LIST_NODE*)vscULIterator_First((iter))
#define VSC_IL_INST_LIST_ITERATOR_NEXT(iter)                  (VSC_IL_INST_LIST_NODE*)vscULIterator_Next((iter))
#define VSC_IL_INST_LIST_ITERATOR_LAST(iter)                  (VSC_IL_INST_LIST_NODE*)vscULIterator_Last((iter))

/* inliner define */
typedef struct VIR_INLINER
{
    VIR_Shader                  *pShader;
    VIR_Dumper                  *pDumper;
    VSC_OPTN_ILOptions          *pOptions;
    VSC_MM                      *pMM;
    VSC_HW_CONFIG               *pHwCfg;
    VIR_CALL_GRAPH              *pCG;
    VSC_HASH_TABLE              *pCandidates; /* inline candidate */

    VSC_IL_PASS_DATA            *pILPassData;
    gctBOOL                     bCheckAlwaysInlineOnly;
    gctBOOL                     bRemoveUnusedFunctions;
    gctINT                      inlineBudget;

} VIR_Inliner;


#define VSC_IL_GetShader(inliner)          ((inliner)->pShader)
#define VSC_IL_SetShader(inliner, s)       ((inliner)->pShader = (s))
#define VSC_IL_GetDumper(inliner)          ((inliner)->pDumper)
#define VSC_IL_SetDumper(inliner, d)       ((inliner)->pDumper = (d))
#define VSC_IL_GetOptions(inliner)         ((inliner)->pOptions)
#define VSC_IL_SetOptions(inliner, o)      ((inliner)->pOptions = (o))
#define VSC_IL_GetMM(inliner)              ((inliner)->pMM)
#define VSC_IL_GetHwCfg(inliner)           ((inliner)->pHwCfg)
#define VSC_IL_SetHwCfg(inliner, hc)       ((inliner)->pHwCfg = (hc))
#define VSC_IL_GetCallGraph(inliner)       ((inliner)->pCG)
#define VSC_IL_SetCallGraph(inliner, cg)   ((inliner)->pCG = (cg))
#define VSC_IL_GetCandidates(inliner)      ((inliner)->pCandidates)
#define VSC_IL_GetPassData(inliner)        ((inliner)->pILPassData)
#define VSC_IL_SetPassData(inliner, data)  ((inliner)->pILPassData = (data))
#define VSC_IL_GetCheckAlwaysInlineOnly(inliner)    ((inliner)->bCheckAlwaysInlineOnly)
#define VSC_IL_SetCheckAlwaysInlineOnly(inliner, V) ((inliner)->bCheckAlwaysInlineOnly = (V))
#define VSC_IL_GetRemoveUnusedFunctions(inliner)    ((inliner)->bRemoveUnusedFunctions)
#define VSC_IL_SetRemoveUnusedFunctions(inliner, V) ((inliner)->bRemoveUnusedFunctions = (V))
#define VSC_IL_GetInlineBudget(inliner)     ((inliner)->inlineBudget)
#define VSC_IL_SetInlineBudget(inliner, bg) ((inliner)->inlineBudget = (bg))

extern VSC_ErrCode VSC_IL_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(VSC_IL_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VSC_IL_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_inline_h_ */

