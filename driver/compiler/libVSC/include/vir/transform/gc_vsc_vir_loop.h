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


#ifndef __gc_vsc_vir_loop_h_
#define __gc_vsc_vir_loop_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct VIR_LOOPINFOMGR VIR_LoopInfoMgr;

typedef struct VIR_LOOPOPTS
{
    VIR_Shader* shader;
    VIR_DEF_USAGE_INFO* pDuInfo;
    VIR_Function* func;
    VIR_LoopInfoMgr* loopInfoMgr;
    VSC_HASH_TABLE* processedLoopInfos;
    VSC_OPTN_LoopOptsOptions* options;
    VIR_Dumper* dumper;
    VSC_MM* mm;
    VSC_HW_CONFIG* pHwCfg;
    gctUINT allowedInstNumAfterUnroll;
    gctUINT maxInvariantCodeMotionCount;
    gctUINT curInvariantCodeMotionCount;
    gctBOOL hwsupportPerCompDepForLS;
    gctBOOL outerLoopFirst;
} VIR_LoopOpts;

#define VIR_LoopOpts_GetShader(lo)                        ((lo)->shader)
#define VIR_LoopOpts_SetShader(lo, s)                     ((lo)->shader = (s))
#define VIR_LoopOpts_GetDuInfo(lo)                        ((lo)->pDuInfo)
#define VIR_LoopOpts_SetDuInfo(lo, s)                     ((lo)->pDuInfo = (s))
#define VIR_LoopOpts_GetFunc(lo)                          ((lo)->func)
#define VIR_LoopOpts_SetFunc(lo, f)                       ((lo)->func = (f))
#define VIR_LoopOpts_GetLoopInfoMgr(lo)                   ((lo)->loopInfoMgr)
#define VIR_LoopOpts_SetLoopInfoMgr(lo, l)                ((lo)->loopInfoMgr = (l))
#define VIR_LoopOpts_GetProcessedLoopInfos(lo)            ((lo)->processedLoopInfos)
#define VIR_LoopOpts_SetProcessedLoopInfos(lo, l)         ((lo)->processedLoopInfos = (l))
#define VIR_LoopOpts_GetOptions(lo)                       ((lo)->options)
#define VIR_LoopOpts_SetOptions(lo, o)                    ((lo)->options = (o))
#define VIR_LoopOpts_GetDumper(lo)                        ((lo)->dumper)
#define VIR_LoopOpts_SetDumper(lo, d)                     ((lo)->dumper = (d))
#define VIR_LoopOpts_GetMM(lo)                            ((lo)->mm)
#define VIR_LoopOpts_SetMM(lo, m)                         ((lo)->mm = (m))
#define VIR_LoopOpts_GetHwCfg(lo)                         ((lo)->pHwCfg)
#define VIR_LoopOpts_SetHwCfg(lo, m)                      ((lo)->pHwCfg = (m))
#define VIR_LoopOpts_GetAllowedInstNumAfterUnroll(lo)     ((lo)->allowedInstNumAfterUnroll)
#define VIR_LoopOpts_SetAllowedInstNumAfterUnroll(lo, n)  ((lo)->allowedInstNumAfterUnroll = (n))
#define VIR_LoopOpts_GetMaxInvariantCodeMotionCount(lo)   ((lo)->maxInvariantCodeMotionCount)
#define VIR_LoopOpts_SetMaxInvariantCodeMotionCount(lo, n)((lo)->maxInvariantCodeMotionCount = (n))
#define VIR_LoopOpts_GetCurInvariantCodeMotionCount(lo)   ((lo)->curInvariantCodeMotionCount)
#define VIR_LoopOpts_SetCurInvariantCodeMotionCount(lo, n)((lo)->curInvariantCodeMotionCount = (n))
#define VIR_LoopOpts_HWsupportPerCompDepForLS(lo)         ((lo)->hwsupportPerCompDepForLS)
#define VIR_LoopOpts_SetHWsupportPerCompDepForLS(lo, b)   ((lo)->hwsupportPerCompDepForLS = (b))
#define VIR_LoopOpts_SetOuterLoopFirst(lo, b)             ((lo)->outerLoopFirst = (b))
#define VIR_LoopOpts_GetOuterLoopFirst(lo)                ((lo)->outerLoopFirst)

/* Loop info mgr. */
struct VIR_LOOPINFOMGR
{
    VIR_LoopOpts*                   loopOpts;
    gctUINT                         nextLoopId;
    VSC_UNI_LIST                    loopInfos;
};

#define VIR_LoopInfoMgr_GetLoopOpts(lim)                ((lim)->loopOpts)
#define VIR_LoopInfoMgr_SetLoopOpts(lim, l)             ((lim)->loopOpts = (l))
#define VIR_LoopInfoMgr_GetShader(lim)                  VIR_LoopOpts_GetShader(VIR_LoopInfoMgr_GetLoopOpts(lim))
#define VIR_LoopInfoMgr_GetFunc(lim)                    VIR_LoopOpts_GetFunc(VIR_LoopInfoMgr_GetLoopOpts(lim))
#define VIR_LoopInfoMgr_GetNextLoopId(lim)              ((lim)->nextLoopId)
#define VIR_LoopInfoMgr_SetNextLoopId(lim, n)           ((lim)->nextLoopId = (n))
#define VIR_LoopInfoMgr_IncNextLoopId(lim)              ((lim)->nextLoopId++)
#define VIR_LoopInfoMgr_GetLoopInfos(lim)               (&(lim)->loopInfos)
#define VIR_LoopInfoMgr_GetLoopInfoCount(lim)           (vscUNILST_GetNodeCount(VIR_LoopInfoMgr_GetLoopInfos(lim)))
#define VIR_LoopInfoMgr_GetMM(lim)                      VIR_LoopOpts_GetMM(VIR_LoopInfoMgr_GetLoopOpts(lim))
#define VIR_LoopInfoMgr_GetOptions(lim)                 VIR_LoopOpts_GetOptions(VIR_LoopInfoMgr_GetLoopOpts(lim))
#define VIR_LoopInfoMgr_GetDumper(lim)                  VIR_LoopOpts_GetDumper(VIR_LoopInfoMgr_GetLoopOpts(lim))

/* Loop DU. */
typedef struct VIR_LoopDU
{
    VSC_HASH_TABLE symToDefListTable;
    gctBOOL isValid;
    VSC_MM* mm;
} VIR_LoopDU;

#define VIR_LoopDU_GetSymToDefListTable(du)     (&(du)->symToDefListTable)
#define VIR_LoopDU_IsValid(du)                  ((du)->isValid)
#define VIR_LoopDU_SetValid(du)                 ((du)->isValid = gcvTRUE)
#define VIR_LoopDU_SetInValid(du)               ((du)->isValid = gcvFALSE)
#define VIR_LoopDU_GetMM(du)                    ((du)->mm)
#define VIR_LoopDU_SetMM(du, m)                 ((du)->mm = (m))

/* Loop IV flag. */
typedef enum VIR_IV_FLAGS
{
    VIR_IV_Flags_None        = 0x0,
    VIR_IV_Flags_Invalid     = 0x1,
    VIR_IV_Flags_Basic       = 0x2,
    VIR_IV_Flags_LoopIndex   = 0x4
} VIR_IV_Flags;

/* Loop IV. */
typedef struct VIR_INDUCTIONVARIABLE
{
    VSC_UNI_LIST_NODE               node;
    VIR_Symbol*                     sym;
    gctUINT                         channel;
    VIR_Instruction*                updateInst;
    VIR_Const                       factor;
    struct VIR_INDUCTIONVARIABLE*   basis;
    VIR_Const                       constFactor;
    VIR_IV_Flags                    flags;
} VIR_InductionVariable;

typedef VIR_InductionVariable VIR_IV;

#define VIR_IV_GetSym(iv)                           ((iv)->sym)
#define VIR_IV_SetSym(iv, s)                        ((iv)->sym = (s))
#define VIR_IV_GetChannel(iv)                       ((iv)->channel)
#define VIR_IV_SetChannel(iv, c)                    ((iv)->channel = (c))
#define VIR_IV_GetUpdateInst(iv)                    ((iv)->updateInst)
#define VIR_IV_SetUpdateInst(iv, i)                 ((iv)->updateInst = (i))
#define VIR_IV_GetUpdateOpcode(iv)                  (VIR_Inst_GetOpcode((iv)->updateInst))
#define VIR_IV_GetFactor(iv)                        (&(iv)->factor)
#define VIR_IV_SetFactor(iv, f)                     ((iv)->factor = (f))
#define VIR_IV_GetBasis(iv)                         ((iv)->basis)
#define VIR_IV_SetBasis(iv, b)                      ((iv)->basis = (b))
#define VIR_IV_GetConstFactor(iv)                   (&(iv)->constFactor)
#define VIR_IV_SetConstFactor(iv, c)                ((iv)->constFactor = (c))
#define VIR_IV_GetFlags(iv)                         ((iv)->flags)
#define VIR_IV_SetFlags(iv, f)                      ((iv)->flags = (f))
#define VIR_IV_AddFlag(iv, f)                       ((iv)->flags |= (f))
#define VIR_IV_HasFlag(iv, f)                       ((iv)->flags & (f))

/* LOOP IV manager. */
typedef struct VIR_IVMGR
{
    VSC_UNI_LIST                ivList;
    VSC_MM*                     mm;
    gctBOOL                     basicIdentified;
} VIR_IVMgr;

#define VIR_IVMgr_GetIVList(i)                  (&(i)->ivList)
#define VIR_IVMgr_GetIVCount(i)                 vscUNILST_GetNodeCount(VIR_IVMgr_GetIVList(i))
#define VIR_IVMgr_GetBasicIdentified(i)         ((i)->basicIdentified)
#define VIR_IVMgr_SetBasicIdentified(i, b)      ((i)->basicIdentified = (b))
#define VIR_IVMgr_GetMM(i)                      ((i)->mm)
#define VIR_IVMgr_SetMM(i, m)                   ((i)->mm = (m))

/* Loop up bound. */
typedef struct VIR_LOOPUPBOUND
{
    VIR_IV*                 iv;
    VIR_Instruction*        cmpInst;
    VIR_Symbol*             upboundSym;
    gctUINT                 upboundSymChannel;
    VIR_TypeId              upboundOpndTypeId;
    VIR_Const               upboundConst;
} VIR_LoopUpbound;

#define VIR_LoopUpbound_GetIV(lu)                           ((lu)->iv)
#define VIR_LoopUpbound_SetIV(lu, i)                        ((lu)->iv = (i))
#define VIR_LoopUpbound_GetCmpInst(lu)                      ((lu)->cmpInst)
#define VIR_LoopUpbound_SetCmpInst(lu, c)                   ((lu)->cmpInst = (c))
#define VIR_LoopUpbound_GetCOP(lu)                          VIR_Inst_GetConditionOp((lu)->cmpInst)
#define VIR_LoopUpbound_GetUpboundSym(lu)                   ((lu)->upboundSym)
#define VIR_LoopUpbound_SetUpboundSym(lu, u)                ((lu)->upboundSym = (u))
#define VIR_LoopUpbound_GetUpboundSymChannel(lu)            ((lu)->upboundSymChannel)
#define VIR_LoopUpbound_SetUpboundSymChannel(lu, u)         ((lu)->upboundSymChannel = (u))
#define VIR_LoopUpbound_GetUpboundOpndTypeId(lu)            ((lu)->upboundOpndTypeId)
#define VIR_LoopUpbound_SetUpboundOpndTypeId(lu, u)         ((lu)->upboundOpndTypeId = (u))
#define VIR_LoopUpbound_GetUpboundConst(lu)                 (&(lu)->upboundConst)
#define VIR_LoopUpbound_IsConst(lu)                         ((lu)->upboundSym == gcvNULL)

/* Loop low bound. */
typedef struct VIR_LOOPLOWBOUND
{
    VIR_IV*                 iv;
    VIR_Symbol*             lowboundSym;
    gctUINT                 lowboundSymChannel;
    VIR_Const               lowboundConst;
} VIR_LoopLowbound;

#define VIR_LoopLowbound_GetIV(ll)                          ((ll)->iv)
#define VIR_LoopLowbound_SetIV(ll, i)                       ((ll)->iv = (i))
#define VIR_LoopLowbound_GetLowboundSym(ll)                 ((ll)->lowboundSym)
#define VIR_LoopLowbound_SetLowboundSym(ll, l)              ((ll)->lowboundSym = (l))
#define VIR_LoopLowbound_GetLowboundSymChannel(ll)          ((ll)->lowboundSymChannel)
#define VIR_LoopLowbound_SetLowboundSymChannel(ll, l)       ((ll)->lowboundSymChannel = (l))
#define VIR_LoopLowbound_GetLowboundConst(ll)               (&(ll)->lowboundConst)
#define VIR_LoopLowbound_IsConst(ll)                        ((ll)->lowboundSym == gcvNULL)

/*
For the loops we are talking about here, they have the following attributes:
    1. break bbs are those whose successors are not all in loop
    2. continue bbs are those whose successors contain loop head
    3. loop body bbs do not have to be continuours in IR. spir-v assembly can generate this kind of loop(dEQP-VK.glsl.functions.control_flow.return_in_nested_loop_vertex).
*/

typedef enum
{
    VIR_LoopInfo_Flags_None             = 0x0000,
    VIR_LoopInfo_Flags_HasEmit          = 0x0001,
    VIR_LoopInfo_Flags_HasStore         = 0x0002,
    VIR_LoopInfo_Flags_HasBarrier       = 0x0004,
} VIR_LoopInfo_Flags;

typedef struct VIR_LOOPINFO
{
    VSC_UNI_LIST_NODE       node;
    gctUINT                 id;
    VIR_LoopInfoMgr*        loopinfoMgr;
    /*
    ** Note that the "head" and "end" is in the graph level, but in the instruction set level,
    ** the loopHead may be in the front and the loopEnd may be behind it.
    */
    VIR_BASIC_BLOCK*        loopHead;
    VIR_BASIC_BLOCK*        loopEnd;
    gctBOOL                 bBackJmpInInstSet;
    struct VIR_LOOPINFO*    parentLoop;
    gctINT                  parentIterationCount;   /* If parent loop have been statically unrolling, save the iteration count. */
    VSC_UNI_LIST            childLoopSet;
    VSC_UNI_LIST            bbSet;
    VSC_UNI_LIST            breakBBSet;
    VSC_UNI_LIST            continueBBSet;
    VSC_UNI_LIST            backBoneSet;
    VSC_UNI_LIST            loopEndDominatorSet;
    VIR_LoopDU*             du;
    VIR_LoopInfo_Flags      flags;
    VIR_IVMgr*              ivMgr;
    VIR_LoopUpbound*        upbound;
    VIR_LoopLowbound*       lowbound;
    gctUINT                 moveInvariantCodeCount;
} VIR_LoopInfo;

#define VIR_LoopInfo_GetId(l)                       ((l)->id)
#define VIR_LoopInfo_SetId(l, i)                    ((l)->id = (i))
#define VIR_LoopInfo_GetLoopInfoMgr(l)              ((l)->loopinfoMgr)
#define VIR_LoopInfo_SetLoopInfoMgr(l, lim)         ((l)->loopinfoMgr = (lim))
#define VIR_LoopInfo_GetShader(l)                   VIR_LoopInfoMgr_GetShader(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetFunc(l)                     VIR_LoopInfoMgr_GetFunc(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetCFG(l)                      (VIR_Function_GetCFG(VIR_LoopInfo_GetFunc(l)))
#define VIR_LoopInfo_GetLoopHead(l)                 ((l)->loopHead)
#define VIR_LoopInfo_SetLoopHead(l, lh)             ((l)->loopHead = (lh))
#define VIR_LoopInfo_GetLoopEnd(l)                  ((l)->loopEnd)
#define VIR_LoopInfo_SetLoopEnd(l, le)              ((l)->loopEnd = (le))
#define VIR_LoopInfo_GetBackJmpInInstSet(l)         ((l)->bBackJmpInInstSet)
#define VIR_LoopInfo_SetBackJmpInInstSet(l, le)     ((l)->bBackJmpInInstSet = (le))
#define VIR_LoopInfo_GetParentLoop(l)               ((l)->parentLoop)
#define VIR_LoopInfo_SetParentLoop(l, p)            ((l)->parentLoop = (p))
#define VIR_LoopInfo_GetParentIterationCount(l)     ((l)->parentIterationCount)
#define VIR_LoopInfo_SetParentIterationCount(l, p)  ((l)->parentIterationCount = (p))
#define VIR_LoopInfo_GetChildLoopSet(l)             (&(l)->childLoopSet)
#define VIR_LoopInfo_GetChildLoopCount(l)           (vscUNILST_GetNodeCount(&(l)->childLoopSet))
#define VIR_LoopInfo_GetBBSet(l)                    (&(l)->bbSet)
#define VIR_LoopInfo_GetBBCount(l)                  (vscUNILST_GetNodeCount(&(l)->bbSet))
#define VIR_LoopInfo_GetBreakBBSet(l)               (&(l)->breakBBSet)
#define VIR_LoopInfo_GetBreakBBCount(l)             (vscUNILST_GetNodeCount(&(l)->breakBBSet))
#define VIR_LoopInfo_GetContinueBBSet(l)            (&(l)->continueBBSet)
#define VIR_LoopInfo_GetContinueBBCount(l)          (vscUNILST_GetNodeCount(&(l)->continueBBSet))
#define VIR_LoopInfo_GetBackBoneBBSet(l)            (&(l)->backBoneSet)
#define VIR_LoopInfo_GetBackBoneBBCount(l)          (vscUNILST_GetNodeCount(&(l)->backBoneSet))
#define VIR_LoopInfo_GetLoopEndDominatorSet(l)      (&(l)->loopEndDominatorSet)
#define VIR_LoopInfo_GetLoopEndDominatorCount(l)    (vscUNILST_GetNodeCount(&(l)->loopEndDominatorSet))
#define VIR_LoopInfo_GetDU(l)                       ((l)->du)
#define VIR_LoopInfo_SetDU(l, d)                    ((l)->du = (d))
#define VIR_LoopInfo_GetFlags(l)                    ((l)->flags)
#define VIR_LoopInfo_SetFlags(l, f)                 ((l)->flags = (f))
#define VIR_LoopInfo_HasFlag(l, f)                  ((l)->flags & (f))
#define VIR_LoopInfo_AddFlag(l, f)                  ((l)->flags |= (f))
#define VIR_LoopInfo_GetIVMGR(l)                    ((l)->ivMgr)
#define VIR_LoopInfo_SetIVMGR(l, i)                 ((l)->ivMgr = (i))
#define VIR_LoopInfo_GetIVCount(l)                  (VIR_LoopInfo_GetIVMGR(l) ? VIR_IVMgr_GetIVCount(VIR_LoopInfo_GetIVMGR(l)) : 0)
#define VIR_LoopInfo_GetUpbound(l)                  ((l)->upbound)
#define VIR_LoopInfo_SetUpbound(l, u)               ((l)->upbound = (u))
#define VIR_LoopInfo_GetLowbound(l)                 ((l)->lowbound)
#define VIR_LoopInfo_SetLowbound(l, u)              ((l)->lowbound = (u))
#define VIR_LoopInfo_GetMoveInvariantCodeCount(l)   ((l)->moveInvariantCodeCount)
#define VIR_LoopInfo_SetMoveInvariantCodeCount(l, u)((l)->moveInvariantCodeCount = (u))
#define VIR_LoopInfo_GetOptions(l)                  VIR_LoopInfoMgr_GetOptions(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetDumper(l)                   VIR_LoopInfoMgr_GetDumper(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetMM(l)                       VIR_LoopInfoMgr_GetMM(VIR_LoopInfo_GetLoopInfoMgr(l))

void
VIR_LoopOpts_Init(
    VIR_LoopOpts* loopOpts,
    VIR_DEF_USAGE_INFO* pDuInfo,
    VIR_Shader* shader,
    VIR_Function* func,
    VSC_OPTN_LoopOptsOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm,
    VSC_HW_CONFIG* pHwCfg
    );

void
VIR_LoopOpts_Final(
    VIR_LoopOpts* loopOpts
    );

VIR_LoopInfoMgr*
VIR_LoopOpts_NewLoopInfoMgr(
    VIR_LoopOpts* loopOpts
    );

void
VIR_LoopOpts_DeleteLoopInfoMgr(
    VIR_LoopOpts* loopOpts
    );

gctBOOL
VIR_LoopOpts_DetectNaturalLoops(
    VIR_LoopOpts* loopOpts
    );

VSC_ErrCode
VIR_LoopOpts_ComputeLoopBodies(
    VIR_LoopOpts* loopOpts
    );

void
VIR_LoopOpts_ComputeLoopTree(
    VIR_LoopOpts* loopOpts
    );

void
VIR_LoopOpts_IdentifyBreakContinues(
    VIR_LoopOpts* loopOpts
    );

/**************************loop info BB iterator**************************/
typedef enum VIR_LOOPINFO_BBITERATOR_TYPE
{
    VIR_LoopInfo_BBIterator_Type_Arbitrary,
    VIR_LoopInfo_BBIterator_Type_BreadthFirst,
    VIR_LoopInfo_BBIterator_Type_DepthFirst,
    VIR_LoopInfo_BBIterator_Type_IRSequence,
    VIR_LoopInfo_BBIterator_Type_CoveringIRSequence,
} VIR_LoopInfo_BBIterator_Type;

typedef struct VIR_LOOPINFO_BBITERATOR
{
    VIR_LoopInfo*           loopInfo;
    gctUINT                 bbCount;
    VIR_BB**                bbArray;
    gctUINT                 curIndex;
    VSC_MM*                 mm;
} VIR_LoopInfo_BBIterator;

#define VIR_LoopInfo_BBIterator_GetLoopInfo(i)                  ((i)->loopInfo)
#define VIR_LoopInfo_BBIterator_SetLoopInfo(i, l)               ((i)->loopInfo = (l))
#define VIR_LoopInfo_BBIterator_GetBBCount(i)                   ((i)->bbCount)
#define VIR_LoopInfo_BBIterator_SetBBCount(i, b)                ((i)->bbCount = (b))
#define VIR_LoopInfo_BBIterator_GetBBArray(i)                   ((i)->bbArray)
#define VIR_LoopInfo_BBIterator_SetBBArray(i, b)                ((i)->bbArray = (b))
#define VIR_LoopInfo_BBIterator_GetCurIndex(i)                  ((i)->curIndex)
#define VIR_LoopInfo_BBIterator_SetCurIndex(i, c)               ((i)->curIndex = (c))
#define VIR_LoopInfo_BBIterator_IncCurIndex(i)                  ((i)->curIndex += 1)
#define VIR_LoopInfo_BBIterator_DecCurIndex(i)                  ((i)->curIndex -= 1)
#define VIR_LoopInfo_BBIterator_GetCurBB(i)                     ((i)->curIndex != gcvMAXUINT32 && (i)->curIndex < (i)->bbCount ? (i)->bbArray[(i)->curIndex] : gcvNULL)
#define VIR_LoopInfo_BBIterator_GetMM(i)                        ((i)->mm)
#define VIR_LoopInfo_BBIterator_SetMM(i, m)                     ((i)->mm = (m))

VSC_ErrCode
VIR_LoopInfo_BBIterator_Init(
    VIR_LoopInfo_BBIterator* iter,
    VIR_LoopInfo* loopInfo,
    VIR_LoopInfo_BBIterator_Type type
    );

void
VIR_LoopInfo_BBIterator_Final(
    VIR_LoopInfo_BBIterator* iter
    );

VIR_BB*
VIR_LoopInfo_BBIterator_First(
    VIR_LoopInfo_BBIterator* iter
    );

VIR_BB*
VIR_LoopInfo_BBIterator_Next(
    VIR_LoopInfo_BBIterator* iter
    );

gctBOOL
VIR_LoopOpts_IsBBHeadBlockOfOneLoop(
    VIR_LoopOpts*       pLoopOpts,
    VIR_BASIC_BLOCK*    pBB,
    VIR_LoopInfo**      ppLoopInfo
    );

VSC_ErrCode
VIR_LoopOpts_PerformLoopInversionOnShader(
    VIR_LoopOpts* loopOpts,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_LoopOpts_PerformLoopUnrollingOnShader(
    VIR_LoopOpts* loopOpts,
    gctBOOL* changed
    );

VSC_ErrCode
VIR_LoopOpts_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_LoopOpts_PerformOnShader);
DECLARE_SH_NECESSITY_CHECK(VIR_LoopOpts_PerformOnShader);

END_EXTERN_C()

#endif /* __gc_vsc_vir_loop_h_ */


