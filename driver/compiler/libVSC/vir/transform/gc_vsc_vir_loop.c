/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"
#include "vir/transform/gc_vsc_vir_loop.h"

/* Only support VSC_UNI_LIST_NODE_EXT node */
static void _CommonFreeList(VSC_UNI_LIST* pList, VSC_MM* pMM)
{
    VSC_UNI_LIST_NODE*  pThisNode;

    for (pThisNode = vscUNILST_GetHead(pList);
         pThisNode != gcvNULL;
         pThisNode = vscUNILST_GetHead(pList))
    {
        /* Remove it from list */
        vscUNILST_Remove(pList, pThisNode);

        /* Delete it now */
        vscMM_Free(pMM, pThisNode);
    }

    vscUNILST_Finalize(pList);
}

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

/************************************************************************************/
/* VIR_LoopInfo related code */
/************************************************************************************/

typedef struct VIR_DEFINLOOP
{
    VSC_UNI_LIST_NODE node;
    VIR_Instruction* inst;
    VIR_Swizzle swizzle;
} VIR_DefInLoop;

#define VIR_DefInLoop_GetInst(d)            ((d)->inst)
#define VIR_DefInLoop_SetInst(d, i)         ((d)->inst = (i))
#define VIR_DefInLoop_GetSwizzle(d)         ((d)->swizzle)
#define VIR_DefInLoop_SetSwizzle(d, s)      ((d)->swizzle = (s))

static void
_VIR_DefInLoop_Init(
    VIR_DefInLoop* def,
    VIR_Instruction* inst,
    VIR_Swizzle swizzle
    )
{
    VIR_DefInLoop_SetInst(def, inst);
    VIR_DefInLoop_SetSwizzle(def, swizzle);
}

typedef struct VIR_LoopDU
{
    VSC_HASH_TABLE symToDefTable;
    VSC_MM* mm;
} VIR_LoopDU;

#define VIR_LoopDU_GetSymToDefTable(du)         (&(du)->symToDefTable)
#define VIR_LoopDU_GetMM(du)                    ((du)->mm)
#define VIR_LoopDU_SetMM(du, m)                 ((du)->mm = (m))

static void
_VIR_LoopDU_Init(
    VIR_LoopDU* du,
    VSC_MM* mm
    )
{
    vscHTBL_Initialize(VIR_LoopDU_GetSymToDefTable(du), mm, vscHFUNC_Default, vscHKCMP_Default, 256);
    VIR_LoopDU_SetMM(du, mm);
}

static void
_VIR_LoopDU_Final(
    VIR_LoopDU* du
    )
{
    VSC_HASH_ITERATOR iter;
    VSC_DIRECT_HNODE_PAIR pair;


    vscHTBLIterator_Init(&iter, VIR_LoopDU_GetSymToDefTable(du));
    for(pair = vscHTBLIterator_DirectFirst(&iter);
        IS_VALID_DIRECT_HNODE_PAIR(&pair);
        pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_UNI_LIST* list = (VSC_UNI_LIST*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        _CommonFreeList(list, VIR_LoopDU_GetMM(du));
        vscMM_Free(VIR_LoopDU_GetMM(du), list);
    }

    vscHTBL_Finalize(VIR_LoopDU_GetSymToDefTable(du));
}

static VSC_ErrCode
_VIR_LoopDU_AddDef(
    VIR_LoopDU* du,
    VIR_Symbol* sym,
    VIR_Swizzle swizzle,
    VIR_Instruction* inst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_TABLE* symToDefTable = VIR_LoopDU_GetSymToDefTable(du);
    VSC_UNI_LIST* list;
    VIR_DefInLoop* def;

    gcmASSERT(swizzle < VIR_CHANNEL_NUM);
    if(!vscHTBL_DirectTestAndGet(symToDefTable, sym, (void**)&list))
    {
        list = (VSC_UNI_LIST*)vscMM_Alloc(VIR_LoopDU_GetMM(du), sizeof(VSC_UNI_LIST));
        if(list == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            return errCode;
        }
        vscUNILST_Initialize(list, gcvFALSE);
        vscHTBL_DirectSet(symToDefTable, sym, list);
    }

    def = (VIR_DefInLoop*)vscMM_Alloc(VIR_LoopDU_GetMM(du), sizeof(VIR_DefInLoop));
    if(def == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }
    _VIR_DefInLoop_Init(def, inst, swizzle);
    vscUNILST_Append(list, (VSC_UNI_LIST_NODE*)def);

    return errCode;
}

static gctUINT
_VIR_LoopDU_SymDefCountInLoop(
    VIR_LoopDU* du,
    VIR_Symbol* sym,
    VIR_Swizzle swizzle
    )
{
    gctUINT result = 0;
    VSC_HASH_TABLE* symToDefTable = VIR_LoopDU_GetSymToDefTable(du);
    VSC_UNI_LIST* list;

    gcmASSERT(swizzle <= 3);

    if(vscHTBL_DirectTestAndGet(symToDefTable, sym, (void**)&list))
    {
        VIR_Enable enable = VIR_Swizzle_2_Enable(swizzle);
        VSC_UL_ITERATOR iter;
        VIR_DefInLoop* def;

    vscULIterator_Init(&iter, list);
    for(def = (VIR_DefInLoop*)vscULIterator_First(&iter);
        def != gcvNULL;
        def = (VIR_DefInLoop*)vscULIterator_Next(&iter))
    {
            VIR_Swizzle swizzle = VIR_DefInLoop_GetSwizzle(def);

            if(enable & (1 << (gctUINT)swizzle))
        {
                result++;
            }
        }
    }

    return result;
}

static gctBOOL
_VIR_LoopDU_SymIsDefinedInLoop(
    VIR_LoopDU* du,
    VIR_Symbol* sym,
    VIR_Swizzle swizzle
    )
{
    VSC_HASH_TABLE* symToDefTable = VIR_LoopDU_GetSymToDefTable(du);
    VSC_UNI_LIST* list;

    if(vscHTBL_DirectTestAndGet(symToDefTable, sym, (void**)&list))
    {
        VIR_Enable enable = VIR_Swizzle_2_Enable(swizzle);
        VSC_UL_ITERATOR iter;
        VIR_DefInLoop* def;

        vscULIterator_Init(&iter, list);
        for(def = (VIR_DefInLoop*)vscULIterator_First(&iter);
            def != gcvNULL;
            def = (VIR_DefInLoop*)vscULIterator_Next(&iter))
        {
            VIR_Instruction* defInst = VIR_DefInLoop_GetInst(def);
            VIR_Swizzle swizzle = VIR_DefInLoop_GetSwizzle(def);

            if(enable & (1 << (gctUINT)swizzle) && !VIR_Inst_IsLoopInvariant(defInst))
            {
                return gcvTRUE;
                }
            }
        }

    return gcvFALSE;
}

typedef enum VIR_IV_FLAGS
{
    VIR_IV_Flags_None        = 0x0,
    VIR_IV_Flags_Invalid     = 0x1,
    VIR_IV_Flags_Basic       = 0x2,
    VIR_IV_Flags_LoopIndex   = 0x4
} VIR_IV_Flags;

typedef struct VIR_INDUCTIONVARIABLE
{
    VSC_UNI_LIST_NODE               node;
    VIR_Symbol*                     sym;
    gctUINT                         channel;
    VIR_Instruction*                updateInst;
    VIR_Const                       factor;
    struct VIR_INDUCTIONVARIABLE*   basis;
    VIR_Const                       diff;
    VIR_IV_Flags                    flags;
} VIR_InductionVariable;

typedef VIR_InductionVariable VIR_IV;

#define VIR_IV_GetSym(iv)                           ((iv)->sym)
#define VIR_IV_SetSym(iv, s)                        ((iv)->sym = (s))
#define VIR_IV_GetChannel(iv)                       ((iv)->channel)
#define VIR_IV_SetChannel(iv, c)                    ((iv)->channel = (c))
#define VIR_IV_GetUpdateInst(iv)                    ((iv)->updateInst)
#define VIR_IV_SetUpdateInst(iv, i)                 ((iv)->updateInst = (i))
#define VIR_IV_GetFactor(iv)                        (&(iv)->factor)
#define VIR_IV_SetFactor(iv, f)                     ((iv)->factor = (f))
#define VIR_IV_GetBasis(iv)                         ((iv)->basis)
#define VIR_IV_SetBasis(iv, b)                      ((iv)->basis = (b))
#define VIR_IV_GetDiff(iv)                          (&(iv)->diff)
#define VIR_IV_SetDiff(iv, d)                       ((iv)->diff = (d))
#define VIR_IV_GetFlags(iv)                         ((iv)->flags)
#define VIR_IV_SetFlags(iv, f)                      ((iv)->flags = (f))
#define VIR_IV_AddFlag(iv, f)                       ((iv)->flags |= (f))
#define VIR_IV_HasFlag(iv, f)                       ((iv)->flags & (f))

static void
_VIR_IV_Init(
    VIR_IV* iv,
    VIR_Symbol* sym,
    gctUINT channel,
    VIR_Instruction* updateInst
    )
{
    VIR_Const* factor = VIR_IV_GetFactor(iv);
    VIR_Const* diff = VIR_IV_GetDiff(iv);
    VIR_TypeId symTypeId = VIR_Symbol_GetTypeId(sym);

    VIR_IV_SetSym(iv, sym);
    VIR_IV_SetChannel(iv, channel);
    VIR_IV_SetUpdateInst(iv, updateInst);
    VIR_IV_SetBasis(iv, iv);
    VIR_IV_SetFlags(iv, VIR_IV_Flags_None);
    if(VIR_TypeId_isFloat(symTypeId))
    {
        factor->type = VIR_TYPE_FLOAT32;
        factor->value.scalarVal.fValue = 1.0;
        diff->type = VIR_TYPE_FLOAT32;
        diff->value.scalarVal.fValue = 0.0;
    }
    else if(VIR_TypeId_isSignedInteger(symTypeId))
    {
        factor->type = VIR_TYPE_INT32;
        factor->value.scalarVal.iValue = 1;
        diff->type = VIR_TYPE_INT32;
        diff->value.scalarVal.iValue = 0;
    }
    else if(VIR_TypeId_isUnSignedInteger(symTypeId))
    {
        factor->type = VIR_TYPE_UINT32;
        factor->value.scalarVal.uValue = 1;
        diff->type = VIR_TYPE_UINT32;
        diff->value.scalarVal.uValue = 0;
    }
    else
    {
        gcmASSERT(0);
    }
}

static void
_VIR_IV_Dump(
    VIR_IV* iv,
    VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "iv channel: %d\n", VIR_IV_GetChannel(iv));
    VIR_LOG(dumper, "iv update inst:\n");
    VIR_Inst_Dump(dumper, VIR_IV_GetUpdateInst(iv));
    if(VIR_IV_HasFlag(iv, VIR_IV_Flags_Invalid))
    {
        VIR_LOG(dumper, "invaild ");
    }
    if(VIR_IV_HasFlag(iv, VIR_IV_Flags_Basic))
    {
        VIR_LOG(dumper, "basic ");
    }
    if(VIR_IV_HasFlag(iv, VIR_IV_Flags_LoopIndex))
    {
        VIR_LOG(dumper, "loop_index ");
    }
    VIR_LOG(dumper, "\n");
    VIR_LOG_FLUSH(dumper);
}

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

static void
_VIR_IVMgr_Init(
    VIR_IVMgr* ivMgr,
    VSC_MM* mm
    )
{
    vscUNILST_Initialize(VIR_IVMgr_GetIVList(ivMgr), gcvFALSE);
    VIR_IVMgr_SetBasicIdentified(ivMgr, gcvFALSE);
    VIR_IVMgr_SetMM(ivMgr, mm);
}

static void
_VIR_IVMgr_Final(
    VIR_IVMgr* ivMgr
    )
{
    _CommonFreeList(VIR_IVMgr_GetIVList(ivMgr), VIR_IVMgr_GetMM(ivMgr));
}

static VIR_IV*
_VIR_IVMgr_AddIV(
    VIR_IVMgr* ivMgr
    )
{
    VIR_IV* iv = (VIR_IV*)vscMM_Alloc(VIR_IVMgr_GetMM(ivMgr), sizeof(VIR_IV));

    if(iv != gcvNULL)
    {
        vscUNILST_Append(VIR_IVMgr_GetIVList(ivMgr), (VSC_UNI_LIST_NODE*)iv);
    }

    return iv;
}

static VIR_IV*
_VIR_IVMgr_FindIVAccordingToSymChannel(
    VIR_IVMgr* ivMgr,
    VIR_Symbol* sym,
    gctUINT channel
    )
{
    VSC_UL_ITERATOR iter;
    VIR_IV* iv;

    vscULIterator_Init(&iter, VIR_IVMgr_GetIVList(ivMgr));
    for(iv = (VIR_IV*)vscULIterator_First(&iter);
        iv != gcvNULL;
        iv = (VIR_IV*)vscULIterator_Next(&iter))
    {
        if(VIR_IV_GetSym(iv) == sym && VIR_IV_GetChannel(iv) == channel)
        {
            return iv;
        }
    }

    return gcvNULL;
}


static void
_VIR_IVMgr_Dump(
    VIR_IVMgr* ivMgr,
    VIR_Dumper* dumper
    )
{
    VSC_UNI_LIST* ivList = VIR_IVMgr_GetIVList(ivMgr);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE* node;

    vscULIterator_Init(&iter, ivList);
    for(node = vscULIterator_First(&iter);
        node != gcvNULL;
        node = vscULIterator_Next(&iter))
    {
        VIR_IV* iv = (VIR_IV*)node;

        _VIR_IV_Dump(iv, dumper);
    }
}

typedef struct VIR_LOOPUPBOUND
{
    VIR_IV*                 iv;
    VIR_ConditionOp         cop;
    VIR_Symbol*             upboundSym;
    gctUINT                 upboundSymChannel;
    VIR_Const               upboundConst;
} VIR_LoopUpbound;

#define VIR_LoopUpbound_GetIV(lu)                           ((lu)->iv)
#define VIR_LoopUpbound_SetIV(lu, i)                        ((lu)->iv = (i))
#define VIR_LoopUpbound_GetCOP(lu)                          ((lu)->cop)
#define VIR_LoopUpbound_SetCOP(lu, c)                       ((lu)->cop = (c))
#define VIR_LoopUpbound_GetUpboundSym(lu)                   ((lu)->upboundSym)
#define VIR_LoopUpbound_SetUpboundSym(lu, u)                ((lu)->upboundSym = (u))
#define VIR_LoopUpbound_GetUpboundSymChannel(lu)            ((lu)->upboundSymChannel)
#define VIR_LoopUpbound_SetUpboundSymChannel(lu, u)         ((lu)->upboundSymChannel = (u))
#define VIR_LoopUpbound_GetUpboundConst(lu)                 (&(lu)->upboundConst)

static void
_VIR_LoopUpbound_Dump(
    VIR_LoopUpbound* upbound,
    VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "upbound:\n");
    _VIR_IV_Dump(VIR_LoopUpbound_GetIV(upbound), dumper);
    VIR_LOG(dumper, "cop: %d\n", VIR_LoopUpbound_GetCOP(upbound));

    if(VIR_LoopUpbound_GetUpboundSym(upbound))
    {
        VIR_Symbol_Dump(dumper, VIR_LoopUpbound_GetUpboundSym(upbound), gcvFALSE);
        VIR_LOG(dumper, "channel: %d\n", VIR_LoopUpbound_GetUpboundSymChannel(upbound));
    }
    else
    {
        VIR_Const* constVal = VIR_LoopUpbound_GetUpboundConst(upbound);

        if(constVal->type == VIR_TYPE_FLOAT32)
        {
            VIR_LOG(dumper, "float %f\n", constVal->value.scalarVal.fValue);
        }
        else if(constVal->type == VIR_TYPE_INT32)
        {
            VIR_LOG(dumper, "int %d\n", constVal->value.scalarVal.iValue);
        }
        else if(constVal->type == VIR_TYPE_UINT32)
        {
            VIR_LOG(dumper, "uint %u\n", constVal->value.scalarVal.uValue);
        }
        else
        {
            gcmASSERT(0);
        }
    }

    VIR_LOG_FLUSH(dumper);
}

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

static void
_VIR_LoopLowbound_Dump(
    VIR_LoopLowbound* lowbound,
    VIR_Dumper* dumper
    )
{
    VIR_LOG(dumper, "lowbound:\n");
    _VIR_IV_Dump(VIR_LoopLowbound_GetIV(lowbound), dumper);

    if(VIR_LoopLowbound_GetLowboundSym(lowbound))
    {
        VIR_Symbol_Dump(dumper, VIR_LoopLowbound_GetLowboundSym(lowbound), gcvFALSE);
        VIR_LOG(dumper, "channel: %d\n", VIR_LoopLowbound_GetLowboundSymChannel(lowbound));
    }
    else
    {
        VIR_Const* constVal = VIR_LoopLowbound_GetLowboundConst(lowbound);

        if(constVal->type == VIR_TYPE_FLOAT32)
        {
            VIR_LOG(dumper, "%f\n", constVal->value.scalarVal.fValue);
        }
        else
        {
            VIR_LOG(dumper, "%d\n", constVal->value.scalarVal.iValue);
        }
    }

    VIR_LOG_FLUSH(dumper);
}

/*
For the loops we are talking about here, they have the following attributes:
    1. break bbs are those whose successors are not all in loop
    2. continue bbs are those whose successors contain loop head
    3. loop body bbs do not have to be continuours in IR. spir-v assembly can generate this kind of loop(dEQP-VK.glsl.functions.control_flow.return_in_nested_loop_vertex).
*/
typedef struct VIR_LOOPINFO
{
    VSC_UNI_LIST_NODE       node;
    gctUINT                 id;
    VIR_LoopInfoMgr*        loopinfoMgr;
    VIR_BASIC_BLOCK*        loopHead;
    VIR_BASIC_BLOCK*        loopEnd;
    struct VIR_LOOPINFO*    parentLoop;
    VSC_UNI_LIST            childLoopSet;
    VSC_UNI_LIST            bbSet;
    VSC_UNI_LIST            breakBBSet;
    VSC_UNI_LIST            continueBBSet;
    VSC_UNI_LIST            backBoneSet;
    VIR_LoopDU*             du;
    VIR_IVMgr*              ivMgr;
    VIR_LoopUpbound*        upbound;
    VIR_LoopLowbound*       lowbound;
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
#define VIR_LoopInfo_GetParentLoop(l)               ((l)->parentLoop)
#define VIR_LoopInfo_SetParentLoop(l, p)            ((l)->parentLoop = (p))
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
#define VIR_LoopInfo_GetDU(l)                       ((l)->du)
#define VIR_LoopInfo_SetDU(l, d)                    ((l)->du = (d))
#define VIR_LoopInfo_GetIVMGR(l)                    ((l)->ivMgr)
#define VIR_LoopInfo_SetIVMGR(l, i)                 ((l)->ivMgr = (i))
#define VIR_LoopInfo_GetIVCount(l)                  (VIR_LoopInfo_GetIVMGR(l) ? VIR_IVMgr_GetIVCount(VIR_LoopInfo_GetIVMGR(l)) : 0)
#define VIR_LoopInfo_GetUpbound(l)                  ((l)->upbound)
#define VIR_LoopInfo_SetUpbound(l, u)               ((l)->upbound = (u))
#define VIR_LoopInfo_GetLowbound(l)                 ((l)->lowbound)
#define VIR_LoopInfo_SetLowbound(l, u)              ((l)->lowbound = (u))
#define VIR_LoopInfo_GetOptions(l)                  VIR_LoopInfoMgr_GetOptions(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetDumper(l)                   VIR_LoopInfoMgr_GetDumper(VIR_LoopInfo_GetLoopInfoMgr(l))
#define VIR_LoopInfo_GetMM(l)                       VIR_LoopInfoMgr_GetMM(VIR_LoopInfo_GetLoopInfoMgr(l))

static void
_VIR_LoopInfo_Init(
    VIR_LoopInfo* loopInfo,
    VIR_LoopInfoMgr* loopInfoMgr,
    gctUINT id,
    VIR_BASIC_BLOCK* loopHead,
    VIR_BASIC_BLOCK* loopEnd
    )
{
    memset(loopInfo, 0, sizeof(VIR_LoopInfo));
    VIR_LoopInfo_SetLoopInfoMgr(loopInfo, loopInfoMgr);
    vscUNILST_Initialize(VIR_LoopInfo_GetBBSet(loopInfo), gcvFALSE);
    vscUNILST_Initialize(VIR_LoopInfo_GetBreakBBSet(loopInfo), gcvFALSE);
    vscUNILST_Initialize(VIR_LoopInfo_GetContinueBBSet(loopInfo), gcvFALSE);
    vscUNILST_Initialize(VIR_LoopInfo_GetBackBoneBBSet(loopInfo), gcvFALSE);
    vscUNILST_Initialize(VIR_LoopInfo_GetChildLoopSet(loopInfo), gcvFALSE);
    VIR_LoopInfo_SetId(loopInfo, id);
    VIR_LoopInfo_SetLoopHead(loopInfo, loopHead);
    VIR_LoopInfo_SetLoopEnd(loopInfo, loopEnd);
}

static void
_VIR_LoopInfo_Final(
    VIR_LoopInfo* loopInfo
    )
{
    _CommonFreeList(VIR_LoopInfo_GetBBSet(loopInfo), VIR_LoopInfo_GetMM(loopInfo));
    _CommonFreeList(VIR_LoopInfo_GetBreakBBSet(loopInfo), VIR_LoopInfo_GetMM(loopInfo));
    _CommonFreeList(VIR_LoopInfo_GetContinueBBSet(loopInfo), VIR_LoopInfo_GetMM(loopInfo));
    _CommonFreeList(VIR_LoopInfo_GetBackBoneBBSet(loopInfo), VIR_LoopInfo_GetMM(loopInfo));
    _CommonFreeList(VIR_LoopInfo_GetChildLoopSet(loopInfo), VIR_LoopInfo_GetMM(loopInfo));
    if(VIR_LoopInfo_GetDU(loopInfo))
    {
        _VIR_LoopDU_Final(VIR_LoopInfo_GetDU(loopInfo));
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetDU(loopInfo));
    }
    if(VIR_LoopInfo_GetIVMGR(loopInfo))
    {
        _VIR_IVMgr_Final(VIR_LoopInfo_GetIVMGR(loopInfo));
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetIVMGR(loopInfo));
    }
    if(VIR_LoopInfo_GetUpbound(loopInfo))
    {
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetUpbound(loopInfo));
    }
    if(VIR_LoopInfo_GetLowbound(loopInfo))
    {
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetLowbound(loopInfo));
    }
}

static gctBOOL
_VIR_LoopInfo_BBIsInLoop(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_UNI_LIST* bbSet = VIR_LoopInfo_GetBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, bbSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* loopBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == loopBB)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode
_VIR_LoopInfo_AddBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb,
    gctBOOL* newlyAdded
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if(!_VIR_LoopInfo_BBIsInLoop(loopInfo, bb))
    {
        VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

        if(node == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            return errCode;
        }
        vscULNDEXT_Initialize(node, (void*)bb);
        vscUNILST_Append(VIR_LoopInfo_GetBBSet(loopInfo), CAST_ULEN_2_ULN(node));

        if(VIR_LoopInfo_GetParentLoop(loopInfo))
        {
            errCode = _VIR_LoopInfo_AddBB(VIR_LoopInfo_GetParentLoop(loopInfo), bb, gcvNULL);
        }

        if(newlyAdded)
        {
            *newlyAdded = gcvTRUE;
        }
    }
    else
    {
        if(newlyAdded)
        {
            *newlyAdded = gcvFALSE;
        }
    }

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_RemoveBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* bbSet = VIR_LoopInfo_GetBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, bbSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* loopBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == loopBB)
        {
            vscUNILST_Remove(bbSet, (VSC_UNI_LIST_NODE*)node);
            vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), node);
            break;
        }
    }

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UNI_LIST* childLoopSet = VIR_LoopInfo_GetChildLoopSet(loopInfo);

        vscULIterator_Init(&iter, childLoopSet);
        for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            node != gcvNULL;
            node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_LoopInfo* childLoop = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(node);

            _VIR_LoopInfo_RemoveBB(childLoop, bb);
        }
    }

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_AddLoopBBs(
    VIR_LoopInfo* loopInfo0,
    VIR_LoopInfo* loopInfo1
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* bbSet = VIR_LoopInfo_GetBBSet(loopInfo1);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, bbSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* loop1BB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        errCode = _VIR_LoopInfo_AddBB(loopInfo0, loop1BB, gcvNULL);
        if(errCode)
        {
            return errCode;
        }
    }

    return errCode;
}

static
gctBOOL
_VIR_LoopInfo_BBIsBreak(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_UNI_LIST* breakBBSet = VIR_LoopInfo_GetBreakBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, breakBBSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* breakBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == breakBB)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static
VSC_ErrCode
_VIR_LoopInfo_AddBreakBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* breakBB
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if(!_VIR_LoopInfo_BBIsBreak(loopInfo, breakBB))
    {
        VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

        vscULNDEXT_Initialize(node, (void*)breakBB);
        vscUNILST_Append(VIR_LoopInfo_GetBreakBBSet(loopInfo), CAST_ULEN_2_ULN(node));
    }

    return errCode;
}

static
VSC_ErrCode
_VIR_LoopInfo_RemoveBreakBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* breakBBSet = VIR_LoopInfo_GetBreakBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, breakBBSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* breakBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == breakBB)
        {
            vscUNILST_Remove(breakBBSet, (VSC_UNI_LIST_NODE*)node);
            vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), node);
            break;
        }
    }

    gcmASSERT(node);

    return errCode;
}

static
gctBOOL
_VIR_LoopInfo_BBIsContinue(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_UNI_LIST* continueBBSet = VIR_LoopInfo_GetContinueBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, continueBBSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* continueBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == continueBB)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static
VSC_ErrCode
_VIR_LoopInfo_AddContinueBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* continueBB
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if(!_VIR_LoopInfo_BBIsContinue(loopInfo, continueBB))
    {
        VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

        vscULNDEXT_Initialize(node, (void*)continueBB);
        vscUNILST_Append(VIR_LoopInfo_GetContinueBBSet(loopInfo), CAST_ULEN_2_ULN(node));
    }

    return errCode;
}

static
VSC_ErrCode
_VIR_LoopInfo_RemoveContinueBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* continueBBSet = VIR_LoopInfo_GetContinueBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, continueBBSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* continueBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == continueBB)
        {
            vscUNILST_Remove(continueBBSet, (VSC_UNI_LIST_NODE*)node);
            vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), node);
            break;
        }
    }

    gcmASSERT(node);

    return errCode;
}

static
gctBOOL
_VIR_LoopInfo_BBIsBackBone(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* bb
    )
{
    VSC_UNI_LIST* backBoneBBSet = VIR_LoopInfo_GetBackBoneBBSet(loopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, backBoneBBSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* backBoneBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb == backBoneBB)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static
VSC_ErrCode
_VIR_LoopInfo_AddBackBoneBB(
    VIR_LoopInfo* loopInfo,
    VIR_BASIC_BLOCK* backBoneBB
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if(!_VIR_LoopInfo_BBIsBackBone(loopInfo, backBoneBB))
    {
        VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

        vscULNDEXT_Initialize(node, (void*)backBoneBB);
        vscUNILST_Append(VIR_LoopInfo_GetBackBoneBBSet(loopInfo), CAST_ULEN_2_ULN(node));
    }

    return errCode;
}


static
VSC_ErrCode
_VIR_LoopInfo_AddChildLoop(
    VIR_LoopInfo* loopInfo,
    VIR_LoopInfo* childLoopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(node, (void*)childLoopInfo);
    vscUNILST_Append(VIR_LoopInfo_GetChildLoopSet(loopInfo), CAST_ULEN_2_ULN(node));

    return errCode;
}

static
VSC_ErrCode
_VIR_LoopInfo_RemoveChildLoop(
    VIR_LoopInfo* parentLoopInfo,
    VIR_LoopInfo* childLoopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* childLoopSet = VIR_LoopInfo_GetChildLoopSet(parentLoopInfo);
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, childLoopSet);
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_LoopInfo* loopInfo = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(node);

        if(childLoopInfo == loopInfo)
        {
            vscUNILST_Remove(childLoopSet, (VSC_UNI_LIST_NODE*)node);
            vscMM_Free(VIR_LoopInfo_GetMM(parentLoopInfo), node);
            break;
        }
    }

    gcmASSERT(node);

    return errCode;
}

static VIR_BB*
_VIR_LoopInfo_GetLowestBB(
    VIR_LoopInfo* loopInfo,
    gctUINT* coveringBBCount
    )
{
    gctUINT bbCount = VIR_LoopInfo_GetBBCount(loopInfo);
    gctUINT i = 0;
    VIR_BB* bb = VIR_LoopInfo_GetLoopHead(loopInfo);
    gctUINT coveringCount = 0;

    while(gcvTRUE)
    {
        if(_VIR_LoopInfo_BBIsInLoop(loopInfo, bb))
        {
            i++;
        }
        coveringCount++;

        if(i == bbCount)
        {
            break;
        }
        else
        {
            bb = VIR_BB_GetFollowingBB(bb);
        }
    }

    if(coveringBBCount)
    {
        *coveringBBCount = coveringCount;
    }

    gcmASSERT(bb);

    return bb;
}

static VIR_BB*
_VIR_LoopInfo_GetLowerNeighbour(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_BB* result = VIR_BB_GetFollowingBB(_VIR_LoopInfo_GetLowestBB(loopInfo, gcvNULL));

    gcmASSERT(result);

    return result;

}

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

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_InitArbitrary(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo = VIR_LoopInfo_BBIterator_GetLoopInfo(iter);
    VIR_BB** bbArray;
    gctUINT bbCount = VIR_LoopInfo_GetBBCount(loopInfo);
    VSC_UL_ITERATOR ulIter;
    VSC_UNI_LIST_NODE_EXT* node;
    gctUINT i = 0;

    bbArray = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    if(bbArray == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    vscULIterator_Init(&ulIter, VIR_LoopInfo_GetBBSet(loopInfo));
    for(node = CAST_ULN_2_ULEN(vscULIterator_First(&ulIter));
        node != gcvNULL;
        node = CAST_ULN_2_ULEN(vscULIterator_Next(&ulIter)))
    {
        VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        bbArray[i++] = bb;
    }

    gcmASSERT(i == bbCount);

    VIR_LoopInfo_BBIterator_SetBBArray(iter, bbArray);
    VIR_LoopInfo_BBIterator_SetBBCount(iter, bbCount);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_InitBreadthFirst(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo = VIR_LoopInfo_BBIterator_GetLoopInfo(iter);
    VIR_BB** bbArray;
    gctUINT bbCount = VIR_LoopInfo_GetBBCount(loopInfo);
    gctUINT levelStart, levelEnd, last;

    bbArray = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    if(bbArray == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    bbArray[0] = VIR_LoopInfo_GetLoopHead(loopInfo);
    levelStart = 0;
    levelEnd = 0;
    last = 0;

    while(last != bbCount - 1)
    {
        gctUINT i;

        gcmASSERT(levelStart <= levelEnd);
        gcmASSERT(levelEnd <= last);

        for(i = levelStart; i <= levelEnd; i++)
        {
            VIR_BB* bb = bbArray[i];
            VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
            VIR_CFG_EDGE* succEdge;

            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &bb->dgNode.succList);
            succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
            for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
            {
                VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);
                gctUINT j;

                if(!_VIR_LoopInfo_BBIsInLoop(loopInfo, succBB))
                {
                    continue;
                }

                for(j = 0; j <= last; j++)
                {
                    if(bbArray[j] == succBB)
                    {
                        break;
                    }
                }
                if(j <= last)
                {
                    continue;
                }

                bbArray[++last] = succBB;
            }
        }

        levelStart = levelEnd + 1;
        levelEnd = last;
    }

    gcmASSERT(last == bbCount - 1);

    VIR_LoopInfo_BBIterator_SetBBArray(iter, bbArray);
    VIR_LoopInfo_BBIterator_SetBBCount(iter, bbCount);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_InitDepthFirst(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo = VIR_LoopInfo_BBIterator_GetLoopInfo(iter);
    VIR_BB** bbArray;
    gctUINT curBBCount;
    VIR_BB** bbStack;
    gctINT bbStackTop;
    gctUINT bbCount = VIR_LoopInfo_GetBBCount(loopInfo);

    bbArray = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    bbStack = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    if(bbArray == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    bbArray[0] = VIR_LoopInfo_GetLoopHead(loopInfo);
    curBBCount = 1;
    bbStack[0] = VIR_LoopInfo_GetLoopHead(loopInfo);
    bbStackTop = 0;

    while(bbStackTop >= 0)
    {
        VIR_BB* topBB = bbStack[bbStackTop];
        VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
        VIR_CFG_EDGE* succEdge;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &topBB->dgNode.succList);
        succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);
            gctUINT j;

            if(!_VIR_LoopInfo_BBIsInLoop(loopInfo, succBB))
            {
                continue;
            }

            for(j = 0; j < curBBCount; j++)
            {
                if(bbArray[j] == succBB)
                {
                    break;
                }
            }
            if(j < curBBCount)
            {
                continue;
            }

            bbArray[curBBCount++] = succBB;
            bbStack[++bbStackTop] = succBB;
            break;
        }

        if(succEdge == gcvNULL)
        {
            --bbStackTop;
        }
    }

    gcmASSERT(curBBCount == bbCount);

    VIR_LoopInfo_BBIterator_SetBBArray(iter, bbArray);
    VIR_LoopInfo_BBIterator_SetBBCount(iter, bbCount);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_InitIRSequence(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo = VIR_LoopInfo_BBIterator_GetLoopInfo(iter);
    VIR_BB** bbArray;
    gctUINT curBBCount;
    gctUINT bbCount = VIR_LoopInfo_GetBBCount(loopInfo);

    bbArray = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    if(bbArray == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    bbArray[0] = VIR_LoopInfo_GetLoopHead(loopInfo);
    curBBCount = 1;

    while(gcvTRUE)
    {
        VIR_BB* followingBB = VIR_BB_GetFollowingBB(bbArray[curBBCount - 1]);

        if(_VIR_LoopInfo_BBIsInLoop(loopInfo, followingBB))
        {
            bbArray[curBBCount++] = followingBB;
        }
        else
        {
            break;
        }
    }

    gcmASSERT(curBBCount == bbCount);

    VIR_LoopInfo_BBIterator_SetBBArray(iter, bbArray);
    VIR_LoopInfo_BBIterator_SetBBCount(iter, bbCount);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_InitCoveringIRSequence(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo = VIR_LoopInfo_BBIterator_GetLoopInfo(iter);
    VIR_BB** bbArray;
    gctUINT curBBCount;
    gctUINT bbCount;
    VIR_BB* lowestBB = _VIR_LoopInfo_GetLowestBB(loopInfo, &bbCount);

    bbArray = (VIR_BB**)vscMM_Alloc(VIR_LoopInfo_BBIterator_GetMM(iter), sizeof(VIR_BB*) * bbCount);
    if(bbArray == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    bbArray[0] = VIR_LoopInfo_GetLoopHead(loopInfo);
    curBBCount = 1;

    while(bbArray[curBBCount - 1] != lowestBB)
    {
        VIR_BB* followingBB = VIR_BB_GetFollowingBB(bbArray[curBBCount - 1]);
        bbArray[curBBCount++] = followingBB;
    }

    gcmASSERT(curBBCount == bbCount);

    VIR_LoopInfo_BBIterator_SetBBArray(iter, bbArray);
    VIR_LoopInfo_BBIterator_SetBBCount(iter, bbCount);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_BBIterator_Init(
    VIR_LoopInfo_BBIterator* iter,
    VIR_LoopInfo* loopInfo,
    VIR_LoopInfo_BBIterator_Type type
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VIR_LoopInfo_BBIterator_SetLoopInfo(iter, loopInfo);
    VIR_LoopInfo_BBIterator_SetMM(iter, VIR_LoopInfo_GetMM(loopInfo));

    switch(type)
    {
        case VIR_LoopInfo_BBIterator_Type_Arbitrary:
        {
            errCode = _VIR_LoopInfo_BBIterator_InitArbitrary(iter);
            break;
        }
        case VIR_LoopInfo_BBIterator_Type_BreadthFirst:
        {
            errCode = _VIR_LoopInfo_BBIterator_InitBreadthFirst(iter);
            break;
        }
        case VIR_LoopInfo_BBIterator_Type_DepthFirst:
        {
            errCode = _VIR_LoopInfo_BBIterator_InitDepthFirst(iter);
            break;
        }
        case VIR_LoopInfo_BBIterator_Type_IRSequence:
        {
            errCode = _VIR_LoopInfo_BBIterator_InitIRSequence(iter);
            break;
        }
        case VIR_LoopInfo_BBIterator_Type_CoveringIRSequence:
        {
            errCode = _VIR_LoopInfo_BBIterator_InitCoveringIRSequence(iter);
            break;
        }
        default:
            gcmASSERT(0);
    }

    return errCode;
}

static void
_VIR_LoopInfo_BBIterator_Final(
    VIR_LoopInfo_BBIterator* iter
    )
{
    vscMM_Free(VIR_LoopInfo_BBIterator_GetMM(iter), VIR_LoopInfo_BBIterator_GetBBArray(iter));
}

static VIR_BB*
_VIR_LoopInfo_BBIterator_First(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VIR_LoopInfo_BBIterator_SetCurIndex(iter, 0);

    return VIR_LoopInfo_BBIterator_GetCurBB(iter);
}

static VIR_BB*
_VIR_LoopInfo_BBIterator_Next(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VIR_LoopInfo_BBIterator_IncCurIndex(iter);

    return VIR_LoopInfo_BBIterator_GetCurBB(iter);
}

static VIR_BB*
_VIR_LoopInfo_BBIterator_Last(
    VIR_LoopInfo_BBIterator* iter
    )
{
    VIR_LoopInfo_BBIterator_SetCurIndex(iter, VIR_LoopInfo_BBIterator_GetBBCount(iter) - 1);

    return VIR_LoopInfo_BBIterator_GetCurBB(iter);
}


static VIR_LoopDU*
_VIR_LoopInfo_NewDU(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_LoopDU* du = VIR_LoopInfo_GetDU(loopInfo);

    if(du)
    {
        _VIR_LoopDU_Final(du);
        _VIR_LoopDU_Init(du, VIR_LoopInfo_GetMM(loopInfo));
    }
    else
    {
        du = (VIR_LoopDU*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VIR_LoopDU));
        if(du)
        {
            _VIR_LoopDU_Init(du, VIR_LoopInfo_GetMM(loopInfo));
        }
    }

    VIR_LoopInfo_SetDU(loopInfo, du);

    return du;
}

static VSC_ErrCode
_VIR_LoopInfo_BuildDU(
    VIR_LoopInfo* loopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopDU* du = VIR_LoopInfo_GetDU(loopInfo);
    VIR_LoopInfo_BBIterator bbIter = {gcvNULL, 0, gcvNULL, 0, gcvNULL};
    VIR_BB* bb;

    gcmASSERT(du);

    _VIR_LoopInfo_BBIterator_Init(&bbIter, loopInfo, VIR_LoopInfo_BBIterator_Type_Arbitrary);
    for(bb = _VIR_LoopInfo_BBIterator_First(&bbIter);
        bb != gcvNULL;
        bb = _VIR_LoopInfo_BBIterator_Next(&bbIter))
    {
        VIR_Instruction* inst = BB_GET_START_INST(bb);

        while(gcvTRUE)
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

            if(VIR_OPCODE_hasDest(opcode) && VIR_Operand_isSymbol(VIR_Inst_GetDest(inst)))
            {
                VIR_Operand* dest = VIR_Inst_GetDest(inst);
                VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
                VIR_Enable enable = VIR_Operand_GetEnable(dest);
                gctUINT i;

                for(i = 0; i < VIR_CHANNEL_NUM; i++)
                {
                    if(enable & 1 << i)
                {
                        errCode = _VIR_LoopDU_AddDef(du, destSym, (VIR_Swizzle)i, inst);
                if(errCode)
                {
                    _VIR_LoopInfo_BBIterator_Final(&bbIter);
                    return errCode;
                }
            }
                }
            }
            if(VIR_OPCODE_isMemSt(opcode) || VIR_OPCODE_isAtom(opcode))
            {
                VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
                VIR_Symbol* src0Sym = VIR_Operand_GetSymbol(src0);
                VIR_Enable enable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(src0));
                gctUINT i;

                for(i = 0; i < VIR_CHANNEL_NUM; i++)
                {
                    if(enable & 1 << i)
                {
                        errCode = _VIR_LoopDU_AddDef(du, src0Sym, (VIR_Swizzle)i, inst);
                if(errCode)
                {
                    _VIR_LoopInfo_BBIterator_Final(&bbIter);
                    return errCode;
                }
            }
                }
            }

            if(inst == BB_GET_END_INST(bb))
            {
                break;
            }
            else
            {
                inst = VIR_Inst_GetNext(inst);
            }
        }
    }
    _VIR_LoopInfo_BBIterator_Final(&bbIter);

    return errCode;
}

static void
_VIR_LoopInfo_DestroyDU(
    VIR_LoopInfo* loopInfo
    )
{
    if(VIR_LoopInfo_GetDU(loopInfo))
    {
        _VIR_LoopDU_Final(VIR_LoopInfo_GetDU(loopInfo));
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetDU(loopInfo));
        VIR_LoopInfo_SetDU(loopInfo, gcvNULL);
    }
}

static VIR_IVMgr*
_VIR_LoopInfo_NewIVMgr(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_IVMgr* ivMgr = VIR_LoopInfo_GetIVMGR(loopInfo);

    if(ivMgr)
    {
        _VIR_IVMgr_Final(ivMgr);
        _VIR_IVMgr_Init(ivMgr, VIR_LoopInfo_GetMM(loopInfo));
    }
    else
    {
        ivMgr = (VIR_IVMgr*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VIR_IVMgr));
        if(ivMgr)
        {
            _VIR_IVMgr_Init(ivMgr, VIR_LoopInfo_GetMM(loopInfo));
        }
    }

    VIR_LoopInfo_SetIVMGR(loopInfo, ivMgr);

    return ivMgr;
}

static VIR_LoopUpbound*
_VIR_LoopInfo_NewUpbound(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_LoopUpbound* upbound;

    if(VIR_LoopInfo_GetUpbound(loopInfo))
    {
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetUpbound(loopInfo));
    }

    upbound = (VIR_LoopUpbound*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VIR_LoopUpbound));
    if(upbound)
    {
        gcoOS_ZeroMemory(upbound, sizeof(VIR_LoopUpbound));
    }
    VIR_LoopInfo_SetUpbound(loopInfo, upbound);

    return upbound;
}

static VIR_LoopLowbound*
_VIR_LoopInfo_NewLowbound(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_LoopLowbound* lowbound;

    if(VIR_LoopInfo_GetLowbound(loopInfo))
    {
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), VIR_LoopInfo_GetLowbound(loopInfo));
    }

    lowbound = (VIR_LoopLowbound*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VIR_LoopLowbound));
    if(lowbound)
    {
        gcoOS_ZeroMemory(lowbound, sizeof(VIR_LoopLowbound));
    }
    VIR_LoopInfo_SetLowbound(loopInfo, lowbound);

    return lowbound;
}

static void
_VIR_LoopInfo_IdentifyBasicIVs(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_IVMgr* ivMgr = _VIR_LoopInfo_NewIVMgr(loopInfo);
    VIR_LoopInfo_BBIterator iter = {gcvNULL, 0, gcvNULL, 0, gcvNULL};
    VIR_BB* bb;

    _VIR_LoopInfo_BBIterator_Init(&iter, loopInfo, VIR_LoopInfo_BBIterator_Type_DepthFirst);
    for(bb = _VIR_LoopInfo_BBIterator_First(&iter);
        bb != gcvNULL;
        bb = _VIR_LoopInfo_BBIterator_Next(&iter))
    {
        VIR_Instruction* inst;

        if(BB_GET_LENGTH(bb) == 0)
        {
            continue;
        }

        inst = BB_GET_START_INST(bb);

        while(gcvTRUE)
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

            if(opcode == VIR_OP_ADD)
            {
                VIR_Operand* addDest = VIR_Inst_GetDest(inst);
                VIR_Enable enable = VIR_Operand_GetEnable(addDest);
                VIR_Operand* addSrc0 = VIR_Inst_GetSource(inst, 0);
                VIR_Operand* addSrc1 = VIR_Inst_GetSource(inst, 1);

                if(VIR_Operand_isSymbol(addSrc0) &&
                   (VIR_Operand_isImm(addSrc1) || VIR_Operand_isConst(addSrc1)))
                {
                    gctUINT i;

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                    {
                        if(enable & (1 << i))
                {
                            if(VIR_Operand_GetSymbol(addDest) == VIR_Operand_GetSymbol(addSrc0) &&
                               VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(addSrc0), i) == (VIR_Swizzle)i)
                {
                                VIR_IV* iv = _VIR_IVMgr_FindIVAccordingToSymChannel(ivMgr, VIR_Operand_GetSymbol(addDest), i);

                            if(iv)
                            {
                                VIR_IV_AddFlag(iv, VIR_IV_Flags_Invalid);
                            }
                                else
                {
                                    VIR_Const* diff;

                                    iv = _VIR_IVMgr_AddIV(ivMgr);
                                    _VIR_IV_Init(iv, VIR_Operand_GetSymbol(addDest), i, inst);
                                    diff = VIR_IV_GetDiff(iv);
                                        if(VIR_Operand_isImm(addSrc1))
                                        {
                                        if(VIR_TypeId_isFloat(VIR_Operand_GetType(addSrc1)))
                                            {
                                            diff->type = VIR_TYPE_FLOAT32;
                                            diff->value.scalarVal.fValue = VIR_Operand_GetImmediateFloat(addSrc1);
                                            }
                                        else if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(addSrc1)))
                                            {
                                            diff->type = VIR_TYPE_INT32;
                                            diff->value.scalarVal.iValue = VIR_Operand_GetImmediateInt(addSrc1);
                                            }
                                        else if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(addSrc1)))
                                            {
                                            diff->type = VIR_TYPE_UINT32;
                                            diff->value.scalarVal.uValue = VIR_Operand_GetImmediateUint(addSrc1);
                                            }
                                            else
                                            {
                                                gcmASSERT(0);
                                            }
                                        }
                                        else
                                        {
                                            /* TBD */
                                        }
                                        VIR_IV_AddFlag(iv, VIR_IV_Flags_Basic);
                                    }
                            }
                        }
                    }
                }
                                    else if(VIR_Operand_isSymbol(addSrc1) &&
                        (VIR_Operand_isImm(addSrc0) || VIR_Operand_isConst(addSrc0)))
                {
                    gctUINT i;

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                    {
                        if(enable & (1 << i))
                        {
                            if(VIR_Operand_GetSymbol(addDest) == VIR_Operand_GetSymbol(addSrc1) &&
                                            VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(addSrc1), i) == (VIR_Swizzle)i)
                                    {
                                VIR_IV* iv = _VIR_IVMgr_FindIVAccordingToSymChannel(ivMgr, VIR_Operand_GetSymbol(addDest), i);

                                if(iv)
                                {
                                    VIR_IV_AddFlag(iv, VIR_IV_Flags_Invalid);
                                }
                                else
                                {
                                    VIR_Const* diff;

                                    iv = _VIR_IVMgr_AddIV(ivMgr);
                                    _VIR_IV_Init(iv, VIR_Operand_GetSymbol(addDest), i, inst);
                                    diff = VIR_IV_GetDiff(iv);
                                        if(VIR_Operand_isImm(addSrc0))
                                        {
                                        if(VIR_TypeId_isFloat(VIR_Operand_GetType(addSrc0)))
                                            {
                                            diff->type = VIR_TYPE_FLOAT32;
                                            diff->value.scalarVal.fValue = VIR_Operand_GetImmediateFloat(addSrc0);
                                            }
                                        else if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(addSrc0)))
                                            {
                                            diff->type = VIR_TYPE_INT32;
                                            diff->value.scalarVal.iValue = VIR_Operand_GetImmediateInt(addSrc0);
                                            }
                                        else if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(addSrc0)))
                                            {
                                            diff->type = VIR_TYPE_UINT32;
                                            diff->value.scalarVal.uValue = VIR_Operand_GetImmediateUint(addSrc0);
                                            }
                                            else
                                            {
                                                gcmASSERT(0);
                                            }
                                        }
                                        else
                                        {
                                            /* TBD */
                                        }
                                        VIR_IV_AddFlag(iv, VIR_IV_Flags_Basic);
                                    }
                            }
                        }
                    }
                }
                                }
            else if(opcode == VIR_OP_SUB)
                                {
                VIR_Operand* subDest = VIR_Inst_GetDest(inst);
                VIR_Enable enable = VIR_Operand_GetEnable(subDest);
                                    VIR_Operand* subSrc0 = VIR_Inst_GetSource(inst, 0);
                                    VIR_Operand* subSrc1 = VIR_Inst_GetSource(inst, 1);

                                    if(VIR_Operand_isSymbol(subSrc0) &&
                   (VIR_Operand_isImm(subSrc1) || VIR_Operand_isConst(subSrc1)))
                                    {
                    gctUINT i;

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                                        {
                        if(enable & (1 << i))
                                            {
                            if(VIR_Operand_GetSymbol(subDest) == VIR_Operand_GetSymbol(subSrc0) &&
                               VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(subSrc0), i) == (VIR_Swizzle)i)
                                            {
                                VIR_IV* iv = _VIR_IVMgr_FindIVAccordingToSymChannel(ivMgr, VIR_Operand_GetSymbol(subDest), i);

                                if(iv)
                                            {
                                    VIR_IV_AddFlag(iv, VIR_IV_Flags_Invalid);
                                        }
                                        else
                                        {
                                    VIR_Const* diff;

                                    iv = _VIR_IVMgr_AddIV(ivMgr);
                                    _VIR_IV_Init(iv, VIR_Operand_GetSymbol(subDest), i, inst);
                                    diff = VIR_IV_GetDiff(iv);
                                    if(VIR_Operand_isImm(subSrc1))
                                    {
                                        if(VIR_TypeId_isFloat(VIR_Operand_GetType(subSrc1)))
                                        {
                                            diff->type = VIR_TYPE_FLOAT32;
                                            diff->value.scalarVal.fValue = -VIR_Operand_GetImmediateFloat(subSrc1);
                                            }
                                        else if(VIR_TypeId_isInteger(VIR_Operand_GetType(subSrc1)))
                                            {
                                            diff->type = VIR_TYPE_INT32;
                                            diff->value.scalarVal.iValue = -VIR_Operand_GetImmediateInt(subSrc1);
                                            }
                                            else
                                            {
                                                gcmASSERT(0);
                                            }
                                        }
                                        else
                                        {
                                            /* TBD */
                                        }
                                        VIR_IV_AddFlag(iv, VIR_IV_Flags_Basic);
                                    }
                                }
                        }
                    }
                }
            }

            if(inst == BB_GET_END_INST(bb))
            {
                break;
            }
            else
            {
                inst = VIR_Inst_GetNext(inst);
            }
        }
    }
    _VIR_LoopInfo_BBIterator_Final(&iter);

    VIR_IVMgr_SetBasicIdentified(ivMgr, gcvTRUE);

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
    {
        _VIR_IVMgr_Dump(ivMgr, VIR_LoopInfo_GetDumper(loopInfo));
    }
}


static void
_VIR_LoopInfo_Dump(
    VIR_LoopInfo* loopInfo,
    gctBOOL dumpIR
    )
{
    VIR_Dumper* dumper = VIR_LoopInfo_GetDumper(loopInfo);

    VIR_LOG(dumper, "loop info id: %d\n", VIR_LoopInfo_GetId(loopInfo));
    VIR_LOG(dumper, "loop head id: %d\n", BB_GET_ID(VIR_LoopInfo_GetLoopHead(loopInfo)));
    VIR_LOG(dumper, "loop end id: %d\n", BB_GET_ID(VIR_LoopInfo_GetLoopEnd(loopInfo)));

    if(VIR_LoopInfo_GetParentLoop(loopInfo))
    {
        VIR_LOG(dumper, "parent loop id: %d\n", VIR_LoopInfo_GetId(VIR_LoopInfo_GetParentLoop(loopInfo)));
    }

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* nodeExt;

        VIR_LOG(dumper, "child loop ids: ");
        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(loopInfo));
        for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            nodeExt != gcvNULL;
            nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_LoopInfo* childLoop = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(nodeExt);

            VIR_LOG(dumper, "%d ", VIR_LoopInfo_GetId(childLoop));
        }
        VIR_LOG(dumper, "\n");
    }

    if(VIR_LoopInfo_GetBBCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* nodeExt;

        VIR_LOG(dumper, "bb ids: ");
        vscULIterator_Init(&iter, VIR_LoopInfo_GetBBSet(loopInfo));
        for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            nodeExt != gcvNULL;
            nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(nodeExt);

            VIR_LOG(dumper, "%d ", BB_GET_ID(bb));
        }
        VIR_LOG(dumper, "\n");
    }

    if(VIR_LoopInfo_GetBreakBBCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* nodeExt;

        VIR_LOG(dumper, "break bb ids: ");
        vscULIterator_Init(&iter, VIR_LoopInfo_GetBreakBBSet(loopInfo));
        for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            nodeExt != gcvNULL;
            nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(nodeExt);

            VIR_LOG(dumper, "%d ", BB_GET_ID(bb));
        }
        VIR_LOG(dumper, "\n");
    }

    if(VIR_LoopInfo_GetContinueBBCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* nodeExt;

        VIR_LOG(dumper, "continue bb ids: ");
        vscULIterator_Init(&iter, VIR_LoopInfo_GetContinueBBSet(loopInfo));
        for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            nodeExt != gcvNULL;
            nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(nodeExt);

            VIR_LOG(dumper, "%d ", BB_GET_ID(bb));
        }
        VIR_LOG(dumper, "\n");
    }

    if(VIR_LoopInfo_GetBackBoneBBCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* nodeExt;

        VIR_LOG(dumper, "back bone bb ids: ");
        vscULIterator_Init(&iter, VIR_LoopInfo_GetBackBoneBBSet(loopInfo));
        for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
            nodeExt != gcvNULL;
            nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
        {
            VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(nodeExt);

            VIR_LOG(dumper, "%d ", BB_GET_ID(bb));
        }
        VIR_LOG(dumper, "\n");
    }

    if(VIR_LoopInfo_GetIVCount(loopInfo))
    {
        _VIR_IVMgr_Dump(VIR_LoopInfo_GetIVMGR(loopInfo), VIR_LoopInfo_GetDumper(loopInfo));
    }

    if(VIR_LoopInfo_GetUpbound(loopInfo))
    {
        _VIR_LoopUpbound_Dump(VIR_LoopInfo_GetUpbound(loopInfo), VIR_LoopInfo_GetDumper(loopInfo));
    }

    VIR_LOG(dumper, "\n");

    if(dumpIR)
    {
        VIR_BB* bb = VIR_LoopInfo_GetLoopHead(loopInfo);
        gctUINT count = 0;

        while(gcvTRUE)
        {
            VIR_BasicBlock_Dump(VIR_LoopInfo_GetDumper(loopInfo), bb, gcvTRUE);
            if(_VIR_LoopInfo_BBIsInLoop(loopInfo, bb))
            {
                count++;
            }

            if(count == VIR_LoopInfo_GetBBCount(loopInfo))
            {
                break;
            }
            else
            {
                bb = VIR_BB_GetFollowingBB(bb);
            }
        }
    }
    VIR_LOG_FLUSH(dumper);
}

/************************************************************************************/
/* VIR_LoopInfoMgr related code */
/************************************************************************************/

void
VIR_LoopInfoMgr_Init(
    VIR_LoopInfoMgr* loopInfoMgr,
    VIR_LoopOpts* loopOpts
    )
{
    VIR_LoopInfoMgr_SetLoopOpts(loopInfoMgr, loopOpts);
    VIR_LoopInfoMgr_SetNextLoopId(loopInfoMgr, 0);
    vscUNILST_Initialize(VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr), gcvFALSE);
}

VSC_ErrCode
VIR_LoopInfoMgr_InsertLoopInfo(
    VIR_LoopInfoMgr*        loopInfoMgr,
    VIR_LoopInfo*           loopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    vscUNILST_Append(VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr), (VSC_UNI_LIST_NODE*)loopInfo);
    return errCode;
}

VSC_ErrCode
VIR_LoopInfoMgr_RemoveLoopInfo(
    VIR_LoopInfoMgr*        loopInfoMgr,
    VIR_LoopInfo*           loopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    if(VIR_LoopInfo_GetParentLoop(loopInfo))
    {
        _VIR_LoopInfo_RemoveChildLoop(VIR_LoopInfo_GetParentLoop(loopInfo), loopInfo);
    }

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VIR_LoopInfo* childLoopInfo;

        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(loopInfo));
        for(childLoopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
            childLoopInfo != gcvNULL;
            childLoopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
        {
            VIR_LoopInfo_SetParentLoop(childLoopInfo, VIR_LoopInfo_GetParentLoop(loopInfo));
        }
    }

    vscUNILST_Remove(VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr), (VSC_UNI_LIST_NODE*)loopInfo);
    _VIR_LoopInfo_Final(loopInfo);
    vscMM_Free(VIR_LoopInfoMgr_GetMM(loopInfoMgr), loopInfo);
    return errCode;
}

VSC_ErrCode
VIR_LoopInfoMgr_ClearAllLoopInfos(
    VIR_LoopInfoMgr*        loopInfoMgr
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_UNI_LIST* list = VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr);
    VIR_LoopInfo* loopInfo;

    for (loopInfo = (VIR_LoopInfo*)vscUNILST_GetHead(list);
         loopInfo != gcvNULL;
         loopInfo = (VIR_LoopInfo*)vscUNILST_GetHead(list))
    {
        /* Remove it from list */
        vscUNILST_Remove(list, &loopInfo->node);
        vscULN_Finalize(&loopInfo->node);

        _VIR_LoopInfo_Final(loopInfo);

        /* Delete it now */
        vscMM_Free(VIR_LoopInfoMgr_GetMM(loopInfoMgr), loopInfo);
    }

    vscUNILST_Finalize(list);

    return errCode;
}

VSC_ErrCode
VIR_LoopInfoMgr_NewLoopInfo(
    VIR_LoopInfoMgr* loopInfoMgr,
    VIR_BASIC_BLOCK* loopHead,
    VIR_BASIC_BLOCK* loopEnd,
    VIR_LoopInfo** newLoopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo* loopInfo;

    loopInfo = (VIR_LoopInfo*)vscMM_Alloc(VIR_LoopInfoMgr_GetMM(loopInfoMgr), sizeof(VIR_LoopInfo));
    if(loopInfo == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        _VIR_LoopInfo_Init(loopInfo,
                          loopInfoMgr,
                          VIR_LoopInfoMgr_GetNextLoopId(loopInfoMgr),
                          loopHead,
                          loopEnd);
        VIR_LoopInfoMgr_IncNextLoopId(loopInfoMgr);
        VIR_LoopInfoMgr_InsertLoopInfo(loopInfoMgr, loopInfo);
        if(newLoopInfo)
        {
            *newLoopInfo = loopInfo;
        }
    }

    return errCode;
}

void
VIR_LoopInfoMgr_Final(
    VIR_LoopInfoMgr*        loopInfoMgr
    )
{
    VIR_LoopInfoMgr_ClearAllLoopInfos(loopInfoMgr);
}

VIR_LoopInfo*
VIR_LoopInfoMgr_GetLoopInfoByHeadBB(
    VIR_LoopInfoMgr*        loopInfoMgr,
    VIR_BB* headBB
    )
{
    VIR_LoopInfo* loopInfo;
    VSC_UL_ITERATOR iter;

    vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));

    for(loopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
        loopInfo != gcvNULL;
        loopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
    {
        if(VIR_LoopInfo_GetLoopHead(loopInfo) == headBB)
        {
            return loopInfo;
        }
    }

    return gcvNULL;
}

void
VIR_LoopInfoMgr_Dump(
    VIR_LoopInfoMgr* loopInfoMgr,
    gctBOOL dumpIR
    )
{
    VIR_LoopInfo* loopInfo;
    VSC_UL_ITERATOR iter;

    vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));

    for(loopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
        loopInfo != gcvNULL;
        loopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
    {
        _VIR_LoopInfo_Dump(loopInfo, dumpIR);
    }
}

/************************************************************************************/
/* VIR_LoopOpts methods */
/************************************************************************************/

void
VIR_LoopOpts_Init(
    VIR_LoopOpts* loopOpts,
    VIR_Shader* shader,
    VIR_Function* func,
    VSC_OPTN_LoopOptsOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    )
{
    VIR_LoopOpts_SetShader(loopOpts, shader);
    VIR_LoopOpts_SetFunc(loopOpts, func);
    VIR_LoopOpts_SetLoopInfoMgr(loopOpts, gcvNULL);
    VIR_LoopOpts_SetOptions(loopOpts, options);
    VIR_LoopOpts_SetDumper(loopOpts, dumper);
    VIR_LoopOpts_SetMM(loopOpts, mm);
}

void
VIR_LoopOpts_Final(
    VIR_LoopOpts* loopOpts
    )
{
    if(VIR_LoopOpts_GetLoopInfoMgr(loopOpts))
    {
        vscMM_Free(VIR_LoopOpts_GetMM(loopOpts), VIR_LoopOpts_GetLoopInfoMgr(loopOpts));
    }
}

static VIR_LoopInfoMgr*
_VIR_LoopOpts_NewLoopInfoMgr(
    VIR_LoopOpts* loopOpts
    )
{
    VIR_LoopInfoMgr* loopInfoMgr = (VIR_LoopInfoMgr*)vscMM_Alloc(VIR_LoopOpts_GetMM(loopOpts), sizeof(VIR_LoopInfoMgr));

    gcmASSERT(VIR_LoopOpts_GetLoopInfoMgr(loopOpts) == gcvNULL);

    VIR_LoopOpts_SetLoopInfoMgr(loopOpts, loopInfoMgr);
    VIR_LoopInfoMgr_Init(loopInfoMgr,loopOpts);

    return loopInfoMgr;
}

static void
_VIR_LoopOpts_DeleteLoopInfoMgr(
    VIR_LoopOpts* loopOpts
    )
{
    VIR_LoopInfoMgr* loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);

    if(loopInfoMgr)
    {
        VIR_LoopInfoMgr_Final(loopInfoMgr);
        VIR_LoopOpts_SetLoopInfoMgr(loopOpts, gcvNULL);
    }

    vscMM_Free(VIR_LoopOpts_GetMM(loopOpts), loopInfoMgr);
}

/* Parameter passing into traversal callbacks of CFG */
typedef struct _VSC_CFG_DFS_VISIT_ORDER
{
    gctUINT     preOrderIdx;
    gctUINT     postOrderIdx;
}VSC_CFG_DFS_VISIT_ORDER;

/* Following 3 functions are callbacks for DFS traversal of CFG, they are for determining dfs edge type */
static gctBOOL _OwnBasicBlkHandlerDFSPre(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_BASIC_BLOCK* pBasicBlk, VSC_CFG_DFS_VISIT_ORDER* pVisitOrder)
{
    pBasicBlk->dfsPreVisitOrderIdx = (pVisitOrder->preOrderIdx) ++;

    return gcvFALSE;
}

static gctBOOL _OwnBasicBlkHandlerDFSPost(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_BASIC_BLOCK* pBasicBlk, VSC_CFG_DFS_VISIT_ORDER* pVisitOrder)
{
    pBasicBlk->dfsPostVisitOrderIdx = (pVisitOrder->postOrderIdx) ++;

    return gcvFALSE;
}

static void _EdgeHandlerDFSOnRevisit(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_CFG_EDGE* pEdge, void* pNonParam)
{
    VIR_BASIC_BLOCK*  pFromBB = CFG_EDGE_GET_FROM_BB(pEdge);
    VIR_BASIC_BLOCK*  pToBB = CFG_EDGE_GET_TO_BB(pEdge);

    if (pFromBB->dfsPreVisitOrderIdx < pToBB->dfsPreVisitOrderIdx)
    {
        pEdge->dfsType = VIR_CFG_DFS_EDGE_TYPE_FORWARD;
    }
    else if (pToBB->dfsPostVisitOrderIdx == NOT_ASSIGNED)
    {
        pEdge->dfsType = VIR_CFG_DFS_EDGE_TYPE_BACKWARD;
    }
    else
    {
        pEdge->dfsType = VIR_CFG_DFS_EDGE_TYPE_CROSS;
    }
}

static gctBOOL
_VIR_LoopOpts_DetectNatrualLoops(
    VIR_LoopOpts* loopOpts
    )
{
    VIR_Function*                func = VIR_LoopOpts_GetFunc(loopOpts);
    VIR_CONTROL_FLOW_GRAPH*      cfg = VIR_Function_GetCFG(func);
    VIR_LoopInfoMgr*             loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);
    CFG_ITERATOR                 basicBlkIter;
    VIR_BASIC_BLOCK*             pTailBlock;
    VIR_BASIC_BLOCK*             pHeadBlock;
    VIR_BASIC_BLOCK*             pThisBlock;
    VSC_ADJACENT_LIST_ITERATOR   succEdgeIter;
    VIR_CFG_EDGE*                pSuccEdge;
    VSC_CFG_DFS_VISIT_ORDER      visitOrder = {0};
    gctBOOL                      bLoopDetected = gcvFALSE;

    /* Build DOM tree */
    vscVIR_BuildDOMTreePerCFG(cfg);

    /* Use DFS to build spanning tree, so we can get dfs edge type */
    vscDG_TraversalCB(&cfg->dgGraph,
                      VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                      gcvFALSE,
                      gcvNULL,
                      (PFN_DG_NODE_HANLDER)_OwnBasicBlkHandlerDFSPre,
                      (PFN_DG_NODE_HANLDER)_OwnBasicBlkHandlerDFSPost,
                      gcvNULL,
                      gcvNULL,
                      (PFN_DG_EDGE_HANLDER)_EdgeHandlerDFSOnRevisit,
                      &visitOrder);

    /* Check back-edge t->h, t is the loop tail and h is the loop head */
    CFG_ITERATOR_INIT(&basicBlkIter, cfg);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBlock->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            if (pSuccEdge->dfsType != VIR_CFG_DFS_EDGE_TYPE_BACKWARD)
            {
                continue;
            }

            pHeadBlock = CFG_EDGE_GET_TO_BB(pSuccEdge);
            if(VIR_Function_HasFlag(func, VIR_FUNCFLAG_HAS_GOTO) &&
               !vscBV_TestBit(&pThisBlock->domSet, pHeadBlock->dgNode.id))
            {
                continue;
            }

            pHeadBlock = CFG_EDGE_GET_TO_BB(pSuccEdge);
            pTailBlock = pThisBlock;
            VIR_LoopInfoMgr_NewLoopInfo(loopInfoMgr, pHeadBlock, pTailBlock, gcvNULL);

            bLoopDetected = gcvTRUE;
        }
    }

    /* No need to keep DOM tree */
    vscVIR_DestroyDOMTreePerCFG(cfg);

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopOpts_GetOptions(loopOpts)), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_LOOP_ANALYSIS))
    {
        VIR_LOG(VIR_LoopOpts_GetDumper(loopOpts), "after natual loop detection:\n");
        VIR_LoopInfoMgr_Dump(loopInfoMgr, gcvFALSE);
    }
    return bLoopDetected;
}

static void
_VIR_LoopInfo_ComputeLoopBody(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_BB* loopHead = VIR_LoopInfo_GetLoopHead(loopInfo);
    VIR_BB* loopEnd = VIR_LoopInfo_GetLoopEnd(loopInfo);

    if(loopHead == loopEnd)
    {
        _VIR_LoopInfo_AddBB(loopInfo, loopHead, gcvNULL);

        return;
    }
    else
    {
        VSC_SIMPLE_STACK stack;
        VSC_UNI_LIST_NODE_EXT* node;
        VIR_BB* bb;

        STACK_INITIALIZE(&stack);

        _VIR_LoopInfo_AddBB(loopInfo, loopHead, gcvNULL);
        _VIR_LoopInfo_AddBB(loopInfo, loopEnd, gcvNULL);

        node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));
        vscULNDEXT_Initialize(node, loopEnd);
        STACK_PUSH_ENTRY(&stack, node);

        while(!STACK_CHECK_EMPTY(&stack))
        {
            VSC_ADJACENT_LIST_ITERATOR predEdgeIter;
            VIR_CFG_EDGE* predEdge;

            node = STACK_POP_ENTRY(&stack);
            bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);
            vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), node);

            VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &bb->dgNode.predList);
            predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
            for (; predEdge != gcvNULL; predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
            {
                gctBOOL newlyAdded;
                VIR_BB* predBB = CFG_EDGE_GET_TO_BB(predEdge);

                _VIR_LoopInfo_AddBB(loopInfo, predBB, &newlyAdded);
                if(newlyAdded)
                {
                    node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));
                    vscULNDEXT_Initialize(node, predBB);
                    STACK_PUSH_ENTRY(&stack, node);
                }
            }
        }
    }
}

static void
_VIR_LoopOpts_ComputeLoopBodies(
    VIR_LoopOpts* loopOpts
    )
{
    VIR_LoopInfoMgr* loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);
    VIR_LoopInfo* loopInfo;
    VSC_UL_ITERATOR iter;

    gcmASSERT(loopInfoMgr && VIR_LoopInfoMgr_GetLoopInfoCount(loopInfoMgr));
    vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));

    for(loopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
        loopInfo != gcvNULL;
        loopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
    {
        _VIR_LoopInfo_ComputeLoopBody(loopInfo);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopOpts_GetOptions(loopOpts)), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_LOOP_ANALYSIS))
    {
        VIR_LOG(VIR_LoopOpts_GetDumper(loopOpts), "after compute loop bodies:\n");
        VIR_LoopInfoMgr_Dump(loopInfoMgr, gcvTRUE);
    }
}

static gctBOOL
_VIR_LoopInfo_IsContinueOf(
    VIR_LoopInfo* loopInfo0,
    VIR_LoopInfo* loopInfo1
    )
{
    if(VIR_LoopInfo_GetLoopHead(loopInfo0) == VIR_LoopInfo_GetLoopHead(loopInfo1))
    {
        VIR_BB* loopEnd0 = VIR_LoopInfo_GetLoopEnd(loopInfo0);

        if(BB_GET_OUT_DEGREE(loopEnd0) == 1)
        {
            VSC_UL_ITERATOR iter0;
            VSC_UNI_LIST_NODE_EXT* nodeExt;

            vscULIterator_Init(&iter0, VIR_LoopInfo_GetBBSet(loopInfo0));
            for(nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter0);
                nodeExt != gcvNULL;
                nodeExt = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter0))
            {
                VIR_BB* bb0 = (VIR_BB*)vscULNDEXT_GetContainedUserData(nodeExt);

                if(bb0 != loopEnd0)
                {
                    VIR_BB* bb0Succ = VIR_BB_GetFirstSuccBB(bb0);

                    gcmASSERT(bb0Succ);
                    if(!_VIR_LoopInfo_BBIsInLoop(loopInfo0, bb0Succ) && _VIR_LoopInfo_BBIsInLoop(loopInfo1, bb0Succ))
                    {
                        return gcvTRUE;
                    }

                    bb0Succ = VIR_BB_GetSecondSuccBB(bb0);
                    if(bb0Succ && !_VIR_LoopInfo_BBIsInLoop(loopInfo0, bb0Succ) && _VIR_LoopInfo_BBIsInLoop(loopInfo1, bb0Succ))
                    {
                        return gcvTRUE;
                    }
                }
            }
        }
        else
        {
            VIR_BB* loopEnd0Following = VIR_BB_GetFollowingBB(loopEnd0);

            gcmASSERT(BB_GET_OUT_DEGREE(loopEnd0) == 2);
            gcmASSERT(VIR_BB_GetJumpToBB(loopEnd0) == VIR_LoopInfo_GetLoopHead(loopInfo1));

            return _VIR_LoopInfo_BBIsInLoop(loopInfo1, loopEnd0Following);
        }
    }

    return gcvFALSE;
}

static gctBOOL
_VIR_LoopInfo_IncludesHead(
    VIR_LoopInfo* loopInfo0,
    VIR_LoopInfo* loopInfo1
    )
{
    return _VIR_LoopInfo_BBIsInLoop(loopInfo0, VIR_LoopInfo_GetLoopHead(loopInfo1));
}

static void
_VIR_LoopOpts_ComputeLoopTree(
    VIR_LoopOpts* loopOpts
    )
{
    VIR_LoopInfoMgr* loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);
    VIR_LoopInfo* loopInfo0;
    VSC_UL_ITERATOR iter0, lastIter0;

    gcmASSERT(loopInfoMgr && VIR_LoopInfoMgr_GetLoopInfoCount(loopInfoMgr));

    vscULIterator_Init(&iter0, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));
    vscULIterator_First(&iter0);
    lastIter0 = iter0;

    for(loopInfo0 = (VIR_LoopInfo*)vscULIterator_Next(&iter0);
        loopInfo0 != gcvNULL;
        loopInfo0 = (VIR_LoopInfo*)vscULIterator_Next(&iter0))
    {
        VIR_LoopInfo* loopInfo1;
        VSC_UL_ITERATOR iter1, lastIter1 = {0};
        gctBOOL lastIter1Valid = gcvFALSE;

        vscULIterator_Init(&iter1, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));
        loopInfo1 = (VIR_LoopInfo*)vscULIterator_First(&iter1);

        while(loopInfo1 != loopInfo0)
        {
            if(VIR_LoopInfo_GetLoopHead(loopInfo0) == VIR_LoopInfo_GetLoopHead(loopInfo1))
            {
                if(_VIR_LoopInfo_IsContinueOf(loopInfo1, loopInfo0))
                {
                    if(BB_GET_OUT_DEGREE(VIR_LoopInfo_GetLoopEnd(loopInfo1)) == 1)
                    {
                        _VIR_LoopInfo_AddLoopBBs(loopInfo0, loopInfo1);
                    }
                    VIR_LoopInfoMgr_RemoveLoopInfo(loopInfoMgr, loopInfo1);
                    if(!lastIter1Valid)
                    {
                        vscULIterator_Init(&iter1, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));
                        loopInfo1 = (VIR_LoopInfo*)vscULIterator_First(&iter1);
                        continue;
                    }
                    else
                    {
                        iter1 = lastIter1;
                    }
                }
                else
                {
                    gcmASSERT(_VIR_LoopInfo_IsContinueOf(loopInfo0, loopInfo1));
                    if(BB_GET_OUT_DEGREE(VIR_LoopInfo_GetLoopEnd(loopInfo0)) == 1)
                    {
                        _VIR_LoopInfo_AddLoopBBs(loopInfo1, loopInfo0);
                    }
                    VIR_LoopInfoMgr_RemoveLoopInfo(loopInfoMgr, loopInfo0);
                    iter0 = lastIter0;
                    break;
                }
            }

            lastIter1 = iter1;
            lastIter1Valid = gcvTRUE;
            loopInfo1 = (VIR_LoopInfo*)vscULIterator_Next(&iter1);
        }

        lastIter0 = iter0;
    }

    vscULIterator_First(&iter0);

    for(loopInfo0 = (VIR_LoopInfo*)vscULIterator_Next(&iter0);
        loopInfo0 != gcvNULL;
        loopInfo0 = (VIR_LoopInfo*)vscULIterator_Next(&iter0))
    {
        VIR_LoopInfo* loopInfo1;
        VSC_UL_ITERATOR iter1;

        vscULIterator_Init(&iter1, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));
        for(loopInfo1 = (VIR_LoopInfo*)vscULIterator_First(&iter1);
            loopInfo1 != loopInfo0;
            loopInfo1 = (VIR_LoopInfo*)vscULIterator_Next(&iter1))
        {
            if(_VIR_LoopInfo_IncludesHead(loopInfo1, loopInfo0) &&
               (VIR_LoopInfo_GetParentLoop(loopInfo0) == gcvNULL ||
                _VIR_LoopInfo_IncludesHead(VIR_LoopInfo_GetParentLoop(loopInfo0), loopInfo1)))
            {
                VIR_LoopInfo_SetParentLoop(loopInfo0, loopInfo1);
            }
            else if(_VIR_LoopInfo_IncludesHead(loopInfo0, loopInfo1) &&
                    (VIR_LoopInfo_GetParentLoop(loopInfo1) == gcvNULL ||
                     _VIR_LoopInfo_IncludesHead(VIR_LoopInfo_GetParentLoop(loopInfo1), loopInfo0)))
            {
                VIR_LoopInfo_SetParentLoop(loopInfo1, loopInfo0);
            }
        }
    }

    for(loopInfo0 = (VIR_LoopInfo*)vscULIterator_First(&iter0);
        loopInfo0 != gcvNULL;
        loopInfo0 = (VIR_LoopInfo*)vscULIterator_Next(&iter0))
    {
        if(VIR_LoopInfo_GetParentLoop(loopInfo0))
        {
            _VIR_LoopInfo_AddChildLoop(VIR_LoopInfo_GetParentLoop(loopInfo0), loopInfo0);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopOpts_GetOptions(loopOpts)), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_LOOP_ANALYSIS))
    {
        VIR_LOG(VIR_LoopOpts_GetDumper(loopOpts), "after compute loop tree:\n");
        VIR_LoopInfoMgr_Dump(loopInfoMgr, gcvFALSE);
    }
}

static void
_VIR_LoopInfo_IdentifyBreakContinues(VIR_LoopInfo* loopInfo)
{
    VSC_UL_ITERATOR iter;
    VSC_UNI_LIST_NODE_EXT* node;

    vscULIterator_Init(&iter, VIR_LoopInfo_GetBBSet(loopInfo));
    for(node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_First(&iter);
        node != gcvNULL;
        node = (VSC_UNI_LIST_NODE_EXT*)vscULIterator_Next(&iter))
    {
        VIR_BB* bb = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

        if(bb != VIR_LoopInfo_GetLoopEnd(loopInfo))
        {
            VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
            VIR_CFG_EDGE* succEdge;

            /* update bb's succ bb */
            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &bb->dgNode.succList);
            succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
            for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
            {
                VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);

                if(succBB == VIR_LoopInfo_GetLoopHead(loopInfo))
                {
                    _VIR_LoopInfo_AddContinueBB(loopInfo, bb);
                }
                else if(!_VIR_LoopInfo_BBIsInLoop(loopInfo, succBB))
                {
                    _VIR_LoopInfo_AddBreakBB(loopInfo, bb);
                }
            }
        }
    }
}

static void
_VIR_LoopOpts_IdentifyBreakContinues(VIR_LoopOpts* loopOpts)
{
    VIR_LoopInfoMgr* loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);
    VIR_LoopInfo* loopInfo;
    VSC_UL_ITERATOR iter;

    gcmASSERT(loopInfoMgr && VIR_LoopInfoMgr_GetLoopInfoCount(loopInfoMgr));

    vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));

    for(loopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
        loopInfo != gcvNULL;
        loopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
    {
        _VIR_LoopInfo_IdentifyBreakContinues(loopInfo);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopOpts_GetOptions(loopOpts)), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_LOOP_ANALYSIS))
    {
        VIR_LOG(VIR_LoopOpts_GetDumper(loopOpts), "after identifying breaks & continues:\n");
        VIR_LoopInfoMgr_Dump(loopInfoMgr, gcvFALSE);
    }
}

VSC_ErrCode
_VIR_LoopInfo_PerformLoopInversionOnLoop(
    VIR_LoopInfo* loopInfo,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_BASIC_BLOCK* loopHead;
    VIR_BASIC_BLOCK* loopEnd;

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* node;

        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(loopInfo));
        for(node = CAST_ULN_2_ULEN(vscULIterator_First(&iter));
            node != gcvNULL;
            node = CAST_ULN_2_ULEN(vscULIterator_Next(&iter)))
        {
            VIR_LoopInfo* childLoopInfo = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(node);

            _VIR_LoopInfo_PerformLoopInversionOnLoop(childLoopInfo, changed);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVERSION))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop inversion input loop:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvTRUE);
    }

    loopHead = VIR_LoopInfo_GetLoopHead(loopInfo);
    loopEnd = VIR_LoopInfo_GetLoopEnd(loopInfo);

    if(_VIR_LoopInfo_BBIsBreak(loopInfo, loopHead) &&
       _VIR_LoopInfo_BBIsInLoop(loopInfo, VIR_BB_GetFollowingBB(loopHead)) &&
       VIR_BB_GetJumpToBB(loopHead) == VIR_BB_GetFollowingBB(loopEnd) &&
       BB_GET_OUT_DEGREE(loopEnd) == 1)
    {
        VIR_BB* loopHeadFallThrough = VIR_BB_GetFollowingBB(loopHead);
        VIR_BB* loopEndFollowing = VIR_BB_GetFollowingBB(loopEnd);
        VIR_BB* newBB;
        VIR_Instruction* jmpcInst;
        VIR_ConditionOp cop;

        VIR_BB_CopyBBAfter(loopHead, loopEnd, &newBB);
        _VIR_LoopInfo_AddBB(loopInfo, newBB, gcvNULL);
        VIR_BB_ChangeSuccBBs(loopEnd, newBB, gcvNULL);
        jmpcInst = BB_GET_END_INST(newBB);
        cop = VIR_Inst_GetConditionOp(jmpcInst);
        VIR_Inst_SetConditionOp(jmpcInst, VIR_ConditionOp_Reverse(cop));
        _VIR_LoopInfo_RemoveBreakBB(loopInfo, loopHead);
        VIR_LoopInfo_SetLoopHead(loopInfo, loopHeadFallThrough);

        while(VIR_LoopInfo_GetContinueBBCount(loopInfo))
        {
            VSC_UNI_LIST_NODE_EXT* node;
            VIR_BB* continueBB;

            node = CAST_ULN_2_ULEN(vscUNILST_GetHead(VIR_LoopInfo_GetContinueBBSet(loopInfo)));
            continueBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

            VIR_BB_ChangeSuccBBs(continueBB, newBB, gcvNULL);
            _VIR_LoopInfo_RemoveContinueBB(loopInfo, continueBB);
        }

        if(VIR_BB_GetJumpToBB(loopHead) == loopEndFollowing)
        {
            VIR_BB_ChangeSuccBBs(newBB, loopHeadFallThrough, loopEndFollowing);
        }
        else
        {
            VIR_BB* jmpBB;
            VIR_LoopInfo* parentLoopInfo = VIR_LoopInfo_GetParentLoop(loopInfo);

            gcmASSERT(parentLoopInfo);

            VIR_BB_InsertBBAfter(newBB, VIR_OP_JMP, &jmpBB);
            VIR_BB_ChangeSuccBBs(jmpBB, VIR_BB_GetJumpToBB(loopHead), gcvNULL);
            VIR_BB_ChangeSuccBBs(newBB, loopHeadFallThrough, jmpBB);
            _VIR_LoopInfo_AddContinueBB(parentLoopInfo, loopHead);
            _VIR_LoopInfo_AddBB(parentLoopInfo, jmpBB, gcvNULL);
            VIR_LoopInfo_SetLoopEnd(parentLoopInfo, jmpBB);
        }
        VIR_LoopInfo_SetLoopEnd(loopInfo, newBB);
        _VIR_LoopInfo_RemoveBB(loopInfo, loopHead);

        if(changed)
        {
            *changed = gcvTRUE;
        }
    }
    else
    {
        if(changed && *changed != gcvTRUE)
        {
            *changed = gcvFALSE;
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVERSION))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop inversion output loop:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_GetPreHead(
    VIR_LoopInfo* loopInfo,
    VIR_BB** preHead
    )
{
    VSC_ErrCode errCode;
    VIR_BB* loopHead = VIR_LoopInfo_GetLoopHead(loopInfo);
    VIR_BB* result;
    VSC_ADJACENT_LIST_ITERATOR predEdgeIter;
    VIR_CFG_EDGE* predEdge;

    errCode = VIR_BB_InsertBBBefore(loopHead, VIR_OP_NOP, &result);
    if(errCode)
    {
        return errCode;
    }
    if(VIR_LoopInfo_GetParentLoop(loopInfo))
    {
        errCode = _VIR_LoopInfo_AddBB(VIR_LoopInfo_GetParentLoop(loopInfo), result, gcvNULL);
        if(errCode)
        {
            return errCode;
        }
    }

    VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &loopHead->dgNode.predList);
    predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
    for (; predEdge != gcvNULL; predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
    {
        VIR_BB* predBB = CFG_EDGE_GET_TO_BB(predEdge);

        if(predBB != VIR_LoopInfo_GetLoopEnd(loopInfo) && !_VIR_LoopInfo_BBIsContinue(loopInfo, predBB))
        {
            switch(CFG_EDGE_GET_TYPE(predEdge))
            {
                case VIR_CFG_EDGE_TYPE_ALWAYS:
                {
                    if(BB_GET_FLOWTYPE(predBB) == VIR_FLOW_TYPE_JMP)
                    {
                        VIR_BB_ChangeSuccBBs(predBB, result, gcvNULL);
                    }
                    else
                    {
                        VIR_BB_ChangeSuccBBs(predBB, gcvNULL, result);
                    }
                    break;
                }
                case VIR_CFG_EDGE_TYPE_TRUE:
                {
                    gcmASSERT(BB_GET_FLOWTYPE(predBB) == VIR_FLOW_TYPE_JMPC);
                    VIR_BB_ChangeSuccBBs(predBB, gcvNULL, result);
                    break;
                }
                case VIR_CFG_EDGE_TYPE_FALSE:
                {
                    gcmASSERT(BB_GET_FLOWTYPE(predBB) == VIR_FLOW_TYPE_JMPC);
                    VIR_BB_ChangeSuccBBs(predBB, result, gcvNULL);
                    break;
                }
                default:
                    gcmASSERT(0);
            }
        }
    }

        errCode = vscVIR_AddEdgeToCFG(BB_GET_CFG(loopHead),
                                      result,
                                      loopHead,
                                      VIR_CFG_EDGE_TYPE_ALWAYS);

    if(preHead)
    {
        *preHead = result;
    }

    return errCode;
}

VSC_ErrCode
_VIR_LoopInfo_BuildBackBoneSet(
    VIR_LoopInfo* loopInfo
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* cfg = VIR_LoopInfo_GetCFG(loopInfo);
    VIR_LoopInfo_BBIterator bbIter = {0};
    VSC_UL_ITERATOR breakBBIter;
    VIR_BB* bb;

    errCode = vscVIR_BuildDOMTreePerCFG(cfg);
    if(errCode)
    {
        return errCode;
    }

    errCode = _VIR_LoopInfo_BBIterator_Init(&bbIter, loopInfo, VIR_LoopInfo_BBIterator_Type_Arbitrary);
    if(errCode)
    {
        return errCode;
    }

    vscULIterator_Init(&breakBBIter, VIR_LoopInfo_GetBreakBBSet(loopInfo));

    for(bb = _VIR_LoopInfo_BBIterator_First(&bbIter);
        bb != gcvNULL;
        bb = _VIR_LoopInfo_BBIterator_Next(&bbIter))
    {
        VSC_UNI_LIST_NODE_EXT* node;
        gctBOOL isBackBone = gcvTRUE;

        for(node = CAST_ULN_2_ULEN(vscULIterator_First(&breakBBIter));
            node != gcvNULL;
            node = CAST_ULN_2_ULEN(vscULIterator_Next(&breakBBIter)))
        {
            VIR_BB* breakBB = (VIR_BB*)vscULNDEXT_GetContainedUserData(node);

            if(!BB_IS_DOM(bb, breakBB))
            {
                isBackBone = gcvFALSE;
                break;
            }
        }

        if(isBackBone && !BB_IS_DOM(bb, VIR_LoopInfo_GetLoopEnd(loopInfo)))
        {
            isBackBone = gcvFALSE;
        }

        if(isBackBone)
        {
            errCode = _VIR_LoopInfo_AddBackBoneBB(loopInfo, bb);
            if(errCode)
            {
                return errCode;
            }
        }
    }

    _VIR_LoopInfo_BBIterator_Final(&bbIter);
    errCode = vscVIR_DestroyDOMTreePerCFG(cfg);
    if(errCode)
    {
        return errCode;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "after building back bone bb set:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvFALSE);
    }

    return errCode;
}

VSC_ErrCode
_VIR_LoopInfo_PerformLoopInvariantCodeMotionOnLoop(
    VIR_LoopInfo* loopInfo,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_LoopInfo_BBIterator bbIter = {gcvNULL, 0, gcvNULL, 0, gcvNULL};
    VIR_BASIC_BLOCK* bb;
    VIR_LoopDU* du;
    gctBOOL repeat;
    VSC_UNI_LIST invariantInsts;

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* node;

        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(loopInfo));
        for(node = CAST_ULN_2_ULEN(vscULIterator_First(&iter));
            node != gcvNULL;
            node = CAST_ULN_2_ULEN(vscULIterator_Next(&iter)))
        {
            VIR_LoopInfo* childLoopInfo = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(node);

            _VIR_LoopInfo_PerformLoopInvariantCodeMotionOnLoop(childLoopInfo, changed);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop invariant code motion input loop:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvTRUE);
    }

    /* build du */
    _VIR_LoopInfo_NewDU(loopInfo);
    _VIR_LoopInfo_BuildDU(loopInfo);
    du = VIR_LoopInfo_GetDU(loopInfo);
    gcmASSERT(du);

    /* initialize all instructions in loop to loop variant */
    _VIR_LoopInfo_BBIterator_Init(&bbIter, loopInfo, VIR_LoopInfo_BBIterator_Type_BreadthFirst);
    for(bb = _VIR_LoopInfo_BBIterator_First(&bbIter);
        bb != gcvNULL;
        bb = _VIR_LoopInfo_BBIterator_Next(&bbIter))
    {
        VIR_Instruction* inst = BB_GET_START_INST(bb);

        while(gcvTRUE)
        {
            VIR_Inst_SetLoopInvariant(inst, gcvFALSE);

            if(inst == BB_GET_END_INST(bb))
            {
                break;
            }
            else
            {
                inst = VIR_Inst_GetNext(inst);
            }
        }
    }
    vscUNILST_Initialize(&invariantInsts, gcvFALSE);

    /* compute and mark loop invariant instructions */
    _VIR_LoopInfo_BuildBackBoneSet(loopInfo);
    do
    {
        repeat = gcvFALSE;
        for(bb = _VIR_LoopInfo_BBIterator_First(&bbIter);
            bb != gcvNULL;
            bb = _VIR_LoopInfo_BBIterator_Next(&bbIter))
        {
            VIR_Instruction* inst = BB_GET_START_INST(bb);

            if(!_VIR_LoopInfo_BBIsBackBone(loopInfo, bb))
            {
                continue;
            }

            while(gcvTRUE)
            {
                if(VIR_OPCODE_IsExpr(VIR_Inst_GetOpcode(inst)) && !VIR_Inst_IsLoopInvariant(inst))
                {
                    VIR_Operand* dest = VIR_Inst_GetDest(inst);
                    VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
                    VIR_Enable enable = VIR_Operand_GetEnable(dest);
                    gctUINT i;

                    for(i = 0; i < VIR_CHANNEL_NUM; i++)
                    {
                        if(enable & (1 << i))
                    {
                            gctUINT defCount = _VIR_LoopDU_SymDefCountInLoop(du, destSym, (VIR_Swizzle)i);

                        if(defCount > 1)
                        {
                                break;
                        }
                    }
                    }

                    if(i >= VIR_CHANNEL_NUM)
                    {
                        gctUINT j;
                        for(j = 0; j < VIR_Inst_GetSrcNum(inst); j++)
                        {
                            VIR_Operand* src = VIR_Inst_GetSource(inst, j);

                            if(VIR_Operand_isImm(src) || VIR_Operand_isConst(src))
                            {
                                continue;
                            }

                            if(VIR_Operand_isSymbol(src))
                            {
                                VIR_Symbol* srcSym = VIR_Operand_GetSymbol(src);
                                VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(src);
                                VIR_Enable enable = VIR_Swizzle_2_Enable(swizzle);
                                gctUINT k;

                                for(k = 0; k < VIR_CHANNEL_NUM; k++)
                                {
                                    if((enable & (1 << k)) && _VIR_LoopDU_SymIsDefinedInLoop(du, srcSym, (VIR_Swizzle)k))
                                {
                                        break;
                                    }
                                }
                                if(k < VIR_CHANNEL_NUM)
                                {
                                    break;
                                }
                            }
                        }
                        if(j >= VIR_Inst_GetSrcNum(inst))
                        {
                            VSC_UNI_LIST_NODE_EXT* node = (VSC_UNI_LIST_NODE_EXT*)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_UNI_LIST_NODE_EXT));

                            vscULNDEXT_Initialize(node, inst);
                            vscUNILST_Append(&invariantInsts, CAST_ULEN_2_ULN(node));
                            VIR_Inst_SetLoopInvariant(inst, gcvTRUE);
                            repeat = gcvTRUE;

                            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT))
                            {
                                VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "the following instruction is loop invariant:\n");
                                VIR_Inst_Dump(VIR_LoopInfo_GetDumper(loopInfo), inst);
                            }
                        }
                    }
                }

                if(inst == BB_GET_END_INST(bb))
                {
                    break;
                }
                else
                {
                    inst = VIR_Inst_GetNext(inst);
                }
            }
        }
    } while(repeat);

    /* move loop invariant instructions out */
    if(vscUNILST_GetNodeCount(&invariantInsts))
    {
        VIR_BB* preHead = gcvNULL;
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* node = gcvNULL;
        VIR_Instruction* insertAfter = gcvNULL;

        errCode = _VIR_LoopInfo_GetPreHead(loopInfo, &preHead);
        if(errCode)
        {
            return errCode;
        }
        insertAfter = BB_GET_END_INST(preHead);

        vscULIterator_Init(&iter, &invariantInsts);
        for(node = CAST_ULN_2_ULEN(vscULIterator_First(&iter));
            node != gcvNULL;
            node = CAST_ULN_2_ULEN(vscULIterator_Next(&iter)))
        {
            VIR_Instruction* invariantInst = (VIR_Instruction*)vscULNDEXT_GetContainedUserData(node);

            VIR_Function_AddCopiedInstructionAfter(VIR_LoopInfo_GetFunc(loopInfo), invariantInst, insertAfter, gcvTRUE, &insertAfter);
            VIR_Function_ChangeInstToNop(VIR_LoopInfo_GetFunc(loopInfo), invariantInst);
        }

        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT))
        {
            VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "new generated preHead:\n");
            VIR_BasicBlock_Dump(VIR_LoopInfo_GetDumper(loopInfo), preHead, gcvTRUE);
        }
    }
    _CommonFreeList(&invariantInsts, VIR_LoopInfo_GetMM(loopInfo));
    vscUNILST_Finalize(&invariantInsts);

    _VIR_LoopInfo_DestroyDU(loopInfo);

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop invariant code motion output loop:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvTRUE);
    }

    return errCode;
}

static void
_VIR_LoopInfo_DetectLoopUpbound(
    VIR_LoopInfo* loopInfo
    )
{
    VIR_BB* loopEnd = VIR_LoopInfo_GetLoopEnd(loopInfo);
    VIR_Instruction* lastInst = BB_GET_END_INST(loopEnd);

    if(VIR_Inst_GetOpcode(lastInst) == VIR_OP_JMPC)
    {
        if(VIR_Inst_GetPrev(lastInst) &&
           VIR_Inst_GetOpcode(VIR_Inst_GetPrev(lastInst)) == VIR_OP_CSELECT &&
           VIR_Inst_GetPrev(VIR_Inst_GetPrev(lastInst)) &&
           VIR_Inst_GetOpcode(VIR_Inst_GetPrev(VIR_Inst_GetPrev(lastInst))) == VIR_OP_COMPARE)
        {
            VIR_Instruction* selectInst = VIR_Inst_GetPrev(lastInst);
            VIR_Instruction* cmpInst = VIR_Inst_GetPrev(selectInst);

            if(VIR_Inst_GetEnable(cmpInst) == VIR_ENABLE_X &&
               VIR_Operand_Identical(VIR_Inst_GetDest(cmpInst), VIR_Inst_GetSource(selectInst, 0), VIR_LoopInfo_GetShader(loopInfo)) &&
               VIR_Operand_Identical(VIR_Inst_GetDest(selectInst), VIR_Inst_GetSource(lastInst, 0), VIR_LoopInfo_GetShader(loopInfo)) &&
               VIR_Inst_GetConditionOp(selectInst) == VIR_COP_SELMSB &&
               VIR_Operand_isImm(VIR_Inst_GetSource(selectInst, 1)) &&
               VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(selectInst, 1)) == 1 &&
               VIR_Operand_isImm(VIR_Inst_GetSource(selectInst, 2)) &&
               VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(selectInst, 2)) == 0 &&
               VIR_Operand_isImm(VIR_Inst_GetSource(lastInst, 1)) &&
               VIR_Operand_GetImmediateUint(VIR_Inst_GetSource(lastInst, 1)) == 0)
            {
                VIR_Operand* cmpSrc0 = VIR_Inst_GetSource(cmpInst, 0);
                VIR_Operand* cmpSrc1 = VIR_Inst_GetSource(cmpInst, 1);
                VIR_Symbol* cmpSym0 = gcvNULL;
                VIR_Symbol* cmpSym1 = gcvNULL;
                VIR_IV* iv0 = gcvNULL;
                VIR_IV* iv1 = gcvNULL;

                if(VIR_Operand_isSymbol(cmpSrc0))
                {
                    cmpSym0 = VIR_Operand_GetSymbol(cmpSrc0);
                    iv0 = _VIR_IVMgr_FindIVAccordingToSymChannel(VIR_LoopInfo_GetIVMGR(loopInfo), cmpSym0, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(cmpSrc0), 0));

                    if(iv0 && !VIR_IV_HasFlag(iv0, VIR_IV_Flags_Basic))
                    {
                        iv0 = gcvNULL;
                    }
                }
                if(VIR_Operand_isSymbol(cmpSrc1))
                {
                    cmpSym1 = VIR_Operand_GetSymbol(cmpSrc1);
                    iv1 = _VIR_IVMgr_FindIVAccordingToSymChannel(VIR_LoopInfo_GetIVMGR(loopInfo), cmpSym1, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(cmpSrc1), 0));

                    if(iv1 && !VIR_IV_HasFlag(iv1, VIR_IV_Flags_Basic))
                    {
                        iv1 = gcvNULL;
                    }
                }

                    if(iv0 && iv1 == gcvNULL)
                    {
                        VIR_LoopUpbound* upbound = _VIR_LoopInfo_NewUpbound(loopInfo);

                        VIR_LoopUpbound_SetIV(upbound, iv0);
                    VIR_LoopUpbound_SetCOP(upbound, VIR_Inst_GetConditionOp(cmpInst));
                        if(VIR_Operand_isSymbol(cmpSrc1))
                        {
                            VIR_LoopUpbound_SetUpboundSym(upbound, cmpSym1);
                            VIR_LoopUpbound_SetUpboundSymChannel(upbound, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(cmpSrc1), 0));
                        }
                        else if(VIR_Operand_isImm(cmpSrc1))
                        {
                            VIR_Const* upboundConst = VIR_LoopUpbound_GetUpboundConst(upbound);

                        if(VIR_TypeId_isFloat(VIR_Operand_GetType(cmpSrc1)))
                            {
                                upboundConst->type = VIR_TYPE_FLOAT32;
                                upboundConst->value.scalarVal.fValue = VIR_Operand_GetImmediateFloat(cmpSrc1);
                            }
                        else if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(cmpSrc1)))
                            {
                                upboundConst->type = VIR_TYPE_INT32;
                                upboundConst->value.scalarVal.iValue = VIR_Operand_GetImmediateInt(cmpSrc1);
                            }
                        else if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(cmpSrc1)))
                            {
                                upboundConst->type = VIR_TYPE_UINT32;
                                upboundConst->value.scalarVal.uValue = VIR_Operand_GetImmediateUint(cmpSrc1);
                            }
                            else
                            {
                                gcmASSERT(0);
                            }
                        }
                        else if(VIR_Operand_isConst(cmpSrc1))
                        {
                            /* TBD */
                            gcmASSERT(0);
                        }
                        else
                        {
                            gcmASSERT(0);
                        }

                        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
                        {
                            _VIR_LoopUpbound_Dump(upbound, VIR_LoopInfo_GetDumper(loopInfo));
                        }
                    }
                else if(iv1 && iv0 == gcvNULL)
                    {
                    /* TBD */
                }
            }
        }
        else
        {
            VIR_Operand* jmpcSrc0 = VIR_Inst_GetSource(lastInst, 0);
            VIR_Operand* jmpcSrc1 = VIR_Inst_GetSource(lastInst, 1);
            VIR_Symbol* jmpcSym0 = gcvNULL;
            VIR_Symbol* jmpcSym1 = gcvNULL;
            VIR_IV* iv0 = gcvNULL;
            VIR_IV* iv1 = gcvNULL;

            if(VIR_Operand_isSymbol(jmpcSrc0))
            {
                jmpcSym0 = VIR_Operand_GetSymbol(jmpcSrc0);
                iv0 = _VIR_IVMgr_FindIVAccordingToSymChannel(VIR_LoopInfo_GetIVMGR(loopInfo), jmpcSym0, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(jmpcSrc0), 0));

                if(iv0 && !VIR_IV_HasFlag(iv0, VIR_IV_Flags_Basic))
                {
                    iv0 = gcvNULL;
                }
            }
            if(VIR_Operand_isSymbol(jmpcSrc1))
            {
                jmpcSym1 = VIR_Operand_GetSymbol(jmpcSrc1);
                iv1 = _VIR_IVMgr_FindIVAccordingToSymChannel(VIR_LoopInfo_GetIVMGR(loopInfo), jmpcSym1, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(jmpcSrc1), 0));

                if(iv1 && !VIR_IV_HasFlag(iv1, VIR_IV_Flags_Basic))
                {
                    iv1 = gcvNULL;
                }
            }

                if(iv0 && iv1 == gcvNULL)
                {
                    VIR_LoopUpbound* upbound = _VIR_LoopInfo_NewUpbound(loopInfo);

                    VIR_LoopUpbound_SetIV(upbound, iv0);
                VIR_LoopUpbound_SetCOP(upbound, VIR_Inst_GetConditionOp(lastInst));
                    if(VIR_Operand_isSymbol(jmpcSrc1))
                    {
                        VIR_LoopUpbound_SetUpboundSym(upbound, jmpcSym1);
                        VIR_LoopUpbound_SetUpboundSymChannel(upbound, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(jmpcSrc1), 0));
                    }
                    else if(VIR_Operand_isImm(jmpcSrc1))
                    {
                        VIR_Const* upboundConst = VIR_LoopUpbound_GetUpboundConst(upbound);

                    if(VIR_TypeId_isFloat(VIR_Operand_GetType(jmpcSrc1)))
                        {
                            upboundConst->type = VIR_TYPE_FLOAT32;
                            upboundConst->value.scalarVal.fValue = VIR_Operand_GetImmediateFloat(jmpcSrc1);
                        }
                    else if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(jmpcSrc1)))
                        {
                            upboundConst->type = VIR_TYPE_INT32;
                            upboundConst->value.scalarVal.iValue = VIR_Operand_GetImmediateInt(jmpcSrc1);
                        }
                    else if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(jmpcSrc1)))
                        {
                            upboundConst->type = VIR_TYPE_UINT32;
                            upboundConst->value.scalarVal.uValue = VIR_Operand_GetImmediateUint(jmpcSrc1);
                        }
                        else
                        {
                            gcmASSERT(0);
                        }
                    }
                    else if(VIR_Operand_isConst(jmpcSrc1))
                    {
                        /* TBD */
                        gcmASSERT(0);
                    }
                    else
                    {
                        gcmASSERT(0);
                    }

                    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
                    {
                        _VIR_LoopUpbound_Dump(upbound, VIR_LoopInfo_GetDumper(loopInfo));
                    }
                }
            else if(iv1 && iv0 == gcvNULL)
                {
                /* TBD */
                }
                }
            }
}

static void
_VIR_LoopInfo_DetectLoopLowbound(
    VIR_LoopInfo* loopInfo,
    VIR_IV* iv
    )
{
    VIR_Symbol* ivSym = VIR_IV_GetSym(iv);
    gctUINT ivSymChannel = VIR_IV_GetChannel(iv);
    VIR_BB* bb = VIR_LoopInfo_GetLoopHead(loopInfo);

    if(BB_GET_IN_DEGREE(bb) != 2)
    {
        return;
    }
    else
    {
        VSC_ADJACENT_LIST_ITERATOR predEdgeIter;
        VIR_CFG_EDGE* predEdge;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &bb->dgNode.predList);
        predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for(;predEdge != gcvNULL; predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            if(bb != CFG_EDGE_GET_TO_BB(predEdge))
            {
                bb = CFG_EDGE_GET_TO_BB(predEdge);
                break;
            }
        }
    }

    while(gcvTRUE)
    {
        gctBOOL foundDef = gcvFALSE;
        VIR_Instruction* inst = BB_GET_END_INST(bb);

        while(gcvTRUE)
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

            if(VIR_OPCODE_hasDest(opcode) && VIR_Operand_isSymbol(VIR_Inst_GetDest(inst)))
            {
                VIR_Operand* dest = VIR_Inst_GetDest(inst);
                VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
                VIR_Enable enable = VIR_Operand_GetEnable(dest);

                if(destSym == ivSym &&
                   enable & (1 << ivSymChannel))
                {
                    foundDef = gcvTRUE;

                    if(opcode == VIR_OP_MOV)
                    {
                        VIR_LoopLowbound* lowbound = _VIR_LoopInfo_NewLowbound(loopInfo);
                        VIR_Operand* src = VIR_Inst_GetSource(inst, 0);

                        VIR_LoopLowbound_SetIV(lowbound, iv);
                        if(VIR_Operand_isSymbol(src))
                        {
                            VIR_LoopLowbound_SetLowboundSym(lowbound, VIR_Operand_GetSymbol(src));
                            VIR_LoopLowbound_SetLowboundSymChannel(lowbound, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(src), ivSymChannel));
                        }
                        else if(VIR_Operand_isImm(src))
                        {
                            VIR_Const* lowboundConst = VIR_LoopLowbound_GetLowboundConst(lowbound);

                            if(VIR_TypeId_isFloat(VIR_Operand_GetType(src)))
                            {
                                lowboundConst->type = VIR_TYPE_FLOAT32;
                                lowboundConst->value.scalarVal.fValue = VIR_Operand_GetImmediateFloat(src);
                            }
                            else if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(src)))
                            {
                                lowboundConst->type = VIR_TYPE_INT32;
                                lowboundConst->value.scalarVal.iValue = VIR_Operand_GetImmediateInt(src);
                            }
                            else if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(src)))
                            {
                                lowboundConst->type = VIR_TYPE_UINT32;
                                lowboundConst->value.scalarVal.uValue = VIR_Operand_GetImmediateUint(src);
                            }
                            else
                            {
                                gcmASSERT(0);
                            }
                        }
                        else if(VIR_Operand_isConst(src))
                        {
                            /* TBD */
                            gcmASSERT(0);
                        }

                        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
                        {
                            _VIR_LoopLowbound_Dump(lowbound, VIR_LoopInfo_GetDumper(loopInfo));
                        }
                    }

                    break;
                }
            }

            if(inst == BB_GET_START_INST(bb))
            {
                break;
            }
            else
            {
                inst = VIR_Inst_GetPrev(inst);
            }
        }

        if(foundDef)
        {
            break;
        }

        if(BB_GET_IN_DEGREE(bb) != 1)
        {
            break;
        }
        else
        {
            VSC_ADJACENT_LIST_ITERATOR predEdgeIter;
            VIR_CFG_EDGE* predEdge;

            VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &bb->dgNode.predList);
            predEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
            bb = CFG_EDGE_GET_TO_BB(predEdge);
        }
    }

}

static void
_VIR_LoopInfo_DetectLoopBound(
    VIR_LoopInfo* loopInfo
    )
{
    _VIR_LoopInfo_DetectLoopUpbound(loopInfo);
    if(VIR_LoopInfo_GetUpbound(loopInfo))
    {
        _VIR_LoopInfo_DetectLoopLowbound(loopInfo, VIR_LoopUpbound_GetIV(VIR_LoopInfo_GetUpbound(loopInfo)));
    }
}

static gctINT
_VIR_LoopInfo_ComputeConstLoopIterations(
    VIR_LoopInfo* loopInfo
    )
{
    gctINT iterations = -1;
    VIR_LoopUpbound* upbound = VIR_LoopInfo_GetUpbound(loopInfo);
    VIR_LoopLowbound* lowbound = VIR_LoopInfo_GetLowbound(loopInfo);

    if(VIR_LoopUpbound_GetUpboundSym(upbound) == gcvNULL &&
       VIR_LoopLowbound_GetLowboundSym(lowbound) == gcvNULL)
    {
        /* constant upperbound and lowerbound */
        VIR_Const* upConst = VIR_LoopUpbound_GetUpboundConst(upbound);
        VIR_Const* lowConst = VIR_LoopLowbound_GetLowboundConst(lowbound);
        VIR_IV* iv = VIR_LoopUpbound_GetIV(upbound);
        VIR_Symbol* ivSym = VIR_IV_GetSym(iv);
        VIR_TypeId ivSymTypeId = VIR_Symbol_GetTypeId(ivSym);
        VIR_Const* diff = VIR_IV_GetDiff(iv);
        VIR_ConditionOp cop = VIR_LoopUpbound_GetCOP(upbound);

        gcmASSERT(iv == VIR_LoopLowbound_GetIV(lowbound));
        gcmASSERT(VIR_GetTypeComponentType(ivSymTypeId) == lowConst->type &&
                  lowConst->type == upConst->type);

        if(VIR_TypeId_isFloat(ivSymTypeId))
        {
            /* TBD */
                                return -1;
                            }
        else if(VIR_TypeId_isInteger(ivSymTypeId))
        {
            gctINT gap;

            switch (cop)
            {
                case VIR_COP_LESS:
                {
                    if(VIR_GetTypeComponentType(ivSymTypeId) == VIR_TYPE_INT32)
                    {
                        if(lowConst->value.scalarVal.iValue < upConst->value.scalarVal.iValue)
                        {
                            if(diff->type == VIR_TYPE_UINT32 || diff->value.scalarVal.iValue > 0)
                            {
                                gap = upConst->value.scalarVal.iValue - lowConst->value.scalarVal.iValue;
                                iterations = gap / diff->value.scalarVal.iValue;
                                return iterations == 0 ? 1 : iterations;
                            }
                            else
                            {
                                return -1;
                            }
                        }
                    else
                    {
                        return -1;
                    }
                }
                    else if(VIR_GetTypeComponentType(ivSymTypeId) == VIR_TYPE_UINT32)
                            {
                            if(lowConst->value.scalarVal.uValue < upConst->value.scalarVal.uValue)
                            {
                            if(diff->type == VIR_TYPE_UINT32 || diff->value.scalarVal.iValue > 0)
                                {
                                    gap = upConst->value.scalarVal.uValue - lowConst->value.scalarVal.uValue;
                                iterations = gap / diff->value.scalarVal.iValue;
                                    return iterations == 0 ? 1 : iterations;
                                }
                                else
                                {
                                    return -1;
                                }
                            }
                            else
                            {
                                return -1;
                            }
                        }
                }
                default:
                    return -1;
                    break;
            }
        }
    }

    return iterations;
}

static VSC_ErrCode
_VIR_LoopInfo_CopyLoop(
    VIR_LoopInfo* loopInfo,
    gctUINT copyId,
    VSC_HASH_TABLE* bbToNewBBMap
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = VIR_LoopInfo_GetFunc(loopInfo);
    VIR_LoopInfo_BBIterator bbIter = {0};
    VIR_BB* lowerNeighbour = _VIR_LoopInfo_GetLowerNeighbour(loopInfo);
    VIR_BB* bb;
    VIR_BB* newBB;
    VIR_CFG* cfg = VIR_LoopInfo_GetCFG(loopInfo);
    VSC_HASH_TABLE* labelToNewLabelMap = vscHTBL_Create(VIR_LoopInfo_GetMM(loopInfo), vscHFUNC_Default, vscHKCMP_Default, 256);

    /* insert BBs */
    _VIR_LoopInfo_BBIterator_Init(&bbIter, loopInfo, VIR_LoopInfo_BBIterator_Type_CoveringIRSequence);
    for(bb = _VIR_LoopInfo_BBIterator_First(&bbIter); bb != gcvNULL; bb = _VIR_LoopInfo_BBIterator_Next(&bbIter))
    {
        VIR_Instruction* bbInst;
        VIR_Instruction* newBBInst;
        gctUINT i;

        errCode = VIR_BB_CopyBBBefore(bb, lowerNeighbour, &newBB);
        if(errCode)
        {
            return errCode;
        }
        vscHTBL_DirectSet(bbToNewBBMap, (void*)bb, (void*)newBB);
        if(VIR_LoopInfo_GetParentLoop(loopInfo))
        {
            errCode = _VIR_LoopInfo_AddBB(VIR_LoopInfo_GetParentLoop(loopInfo), newBB, gcvNULL);
            if(errCode)
            {
                return errCode;
            }
        }

        for(i = 0, bbInst = BB_GET_START_INST(bb), newBBInst = BB_GET_START_INST(newBB);
            i < BB_GET_LENGTH(bb);
            i++, bbInst = VIR_Inst_GetNext(bbInst), newBBInst = VIR_Inst_GetNext(newBBInst))
        {
            VIR_Label* bbLabel;
            VIR_Label* newBBLabel;

            if(VIR_Inst_GetOpcode(bbInst) != VIR_OP_LABEL)
            {
                gcmASSERT(VIR_Inst_GetOpcode(newBBInst) != VIR_OP_LABEL);
                break;
            }
            gcmASSERT(VIR_Inst_GetOpcode(newBBInst) == VIR_OP_LABEL);

            bbLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(bbInst));
            newBBLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(newBBInst));

            vscHTBL_DirectSet(labelToNewLabelMap, bbLabel, newBBLabel);
        }
    }

    /* update cfg edges */
    bb = VIR_LoopInfo_GetLoopHead(loopInfo);
    newBB = VIR_BB_GetFollowingBB(_VIR_LoopInfo_BBIterator_Last(&bbIter));
    while(gcvTRUE)
    {
        VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
        VIR_CFG_EDGE* succEdge;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &bb->dgNode.succList);
        succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);
            VIR_BB* succBBMapping;

            succBBMapping = (VIR_BB*)vscHTBL_DirectGet(bbToNewBBMap, (void*)succBB);
            if(succBBMapping == gcvNULL)
            {
                succBBMapping = succBB;
            }
            vscVIR_AddEdgeToCFG(cfg, newBB, succBBMapping, CFG_EDGE_GET_TYPE(succEdge));
        }

        /* update labels on branch instructions */
        {
            VIR_Instruction* newBBEnd = BB_GET_END_INST(newBB);

            if(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(newBBEnd)))
            {
                VIR_Operand* newBBEndDest = VIR_Inst_GetDest(newBBEnd);
                VIR_Label* label = VIR_Operand_GetLabel(newBBEndDest);
                VIR_Label* newLabel = (VIR_Label*)vscHTBL_DirectGet(labelToNewLabelMap, label);

                if(newLabel)        /* break bb does not need to be modified */
                {
                    VIR_Link* link;

                    errCode = VIR_Function_NewLink(func, &link);
                    if(errCode)
                    {
                        return errCode;
                    }
                    VIR_Link_SetReference(link, (gctUINTPTR_T)newBBEnd);
                    VIR_Link_AddLink(VIR_Label_GetReferenceAddr(newLabel), link);
                    VIR_Operand_SetLabel(newBBEndDest, newLabel);
                }
            }
        }

        if(bb == _VIR_LoopInfo_BBIterator_Last(&bbIter))
        {
            break;
        }
        else
        {
            bb = VIR_BB_GetFollowingBB(bb);
            newBB = VIR_BB_GetFollowingBB(newBB);
        }
    }
    vscHTBL_Destroy(labelToNewLabelMap);

    return errCode;
}

static VSC_ErrCode
_VIR_LoopInfo_StaticallyUnroll(
    VIR_LoopInfo* loopInfo,
    gctUINT copyCount
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_HASH_TABLE** bbToNewBBMaps = (VSC_HASH_TABLE**)vscMM_Alloc(VIR_LoopInfo_GetMM(loopInfo), sizeof(VSC_HASH_TABLE*) * copyCount);

    VIR_BB* loopHead = VIR_LoopInfo_GetLoopHead(loopInfo);
    VIR_BB* loopEnd = VIR_LoopInfo_GetLoopEnd(loopInfo);
    gctUINT i;

    gcmASSERT(copyCount > 0);

    for(i = 0; i < copyCount; i++)
    {
        bbToNewBBMaps[i] = vscHTBL_Create(VIR_LoopInfo_GetMM(loopInfo), vscHFUNC_Default, vscHKCMP_Default, 256);
        _VIR_LoopInfo_CopyLoop(loopInfo, i, bbToNewBBMaps[i]);
    }

    /* orgnize unrolled loop */
    {
        VIR_BB* bb = VIR_LoopInfo_GetLoopHead(loopInfo);

        while(gcvTRUE)
        {
            if(_VIR_LoopInfo_BBIsBreak(loopInfo, bb))
            {
                /* nothing to do */
            }
            else if(_VIR_LoopInfo_BBIsContinue(loopInfo, bb))
            {
                VIR_BB_ChangeSuccBBs(bb, (VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[0], loopHead), gcvNULL);

                /* update copied loops except the last one */
                {
                    gctUINT i;
                    for(i = 0; i < copyCount - 1; i++)
                    {
                        VIR_BB_ChangeSuccBBs((VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[i], bb), (VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[i + 1], loopHead), gcvNULL);
                    }
                }

                /* update the last copied loop */
                VIR_BB_ChangeSuccBBs((VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[copyCount - 1], bb), _VIR_LoopInfo_GetLowerNeighbour(loopInfo), gcvNULL);
            }
            else if(bb == loopEnd)
            {
                VIR_BB_RemoveBranch(bb);

                /* update copied loops except the last one */
                {
                    gctUINT i;
                    for(i = 0; i < copyCount - 1; i++)
                    {
                        VIR_BB_RemoveBranch((VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[i], bb));
                    }
                }

                /* update the last copied loop */
                VIR_BB_RemoveBranch((VIR_BB*)vscHTBL_DirectGet(bbToNewBBMaps[copyCount - 1], bb));
            }

            if(bb == VIR_LoopInfo_GetLoopEnd(loopInfo))
    {
    }

            if(bb == VIR_LoopInfo_GetLoopEnd(loopInfo))
        {
                break;
            }
            else
            {
                bb = VIR_BB_GetFollowingBB(bb);
                }
            }
        }

    for(i = 0; i < copyCount; i++)
        {
            vscHTBL_Destroy(bbToNewBBMaps[i]);
        }
        vscMM_Free(VIR_LoopInfo_GetMM(loopInfo), bbToNewBBMaps);
    return errCode;
}

VSC_ErrCode
_VIR_LoopInfo_PerformLoopUnrollingOnLoop(
    VIR_LoopInfo* loopInfo,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_BB* loopHead = VIR_LoopInfo_GetLoopHead(loopInfo);
    VIR_BB* loopLowerNeighbour = _VIR_LoopInfo_GetLowerNeighbour(loopInfo);
    VIR_Dumper* dumper = VIR_LoopInfo_GetDumper(loopInfo);

    if(VIR_LoopInfo_GetChildLoopCount(loopInfo))
    {
        VSC_UL_ITERATOR iter;
        VSC_UNI_LIST_NODE_EXT* node;

        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(loopInfo));
        for(node = CAST_ULN_2_ULEN(vscULIterator_First(&iter));
            node != gcvNULL;
            node = CAST_ULN_2_ULEN(vscULIterator_Next(&iter)))
        {
            VIR_LoopInfo* childLoopInfo = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(node);

            _VIR_LoopInfo_PerformLoopUnrollingOnLoop(childLoopInfo, changed);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
    {
        VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop unrolling input loop:\n");
        _VIR_LoopInfo_Dump(loopInfo, gcvTRUE);
    }

    _VIR_LoopInfo_IdentifyBasicIVs(loopInfo);
    if(VIR_LoopInfo_GetIVCount(loopInfo) == 0)
    {
        return errCode;
    }

    _VIR_LoopInfo_DetectLoopBound(loopInfo);
    if(VIR_LoopInfo_GetUpbound(loopInfo) != gcvNULL &&
       VIR_LoopInfo_GetLowbound(loopInfo) != gcvNULL)
    {
        gctINT iterations = _VIR_LoopInfo_ComputeConstLoopIterations(loopInfo);

        if(iterations >= 0 && iterations <= 12)
        {
            if(iterations <= 1)
            {
                VIR_BB_RemoveBranch(VIR_LoopInfo_GetLoopEnd(loopInfo));
            }
            else
            {
                    errCode = _VIR_LoopInfo_StaticallyUnroll(loopInfo, (gctUINT)(iterations - 1));
                }
            VIR_LoopInfoMgr_RemoveLoopInfo(VIR_LoopInfo_GetLoopInfoMgr(loopInfo), loopInfo);

            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(VIR_LoopInfo_GetOptions(loopInfo)), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL))
            {
                VIR_LOG(VIR_LoopInfo_GetDumper(loopInfo), "loop unrolling output:\n");
                    VIR_BasicBlock_DumpRange(dumper, loopHead, loopLowerNeighbour, gcvTRUE);
                }
            }
        }

    return errCode;
}

typedef VSC_ErrCode (*_VIR_LoopInfo_OptFuncP)(VIR_LoopInfo* loopInfo, gctBOOL* changed);

VSC_ErrCode
_VIR_LoopOpts_PerformSpecOptOnLoops(
    VIR_LoopOpts* loopOpts,
    _VIR_LoopInfo_OptFuncP optFunc,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    /*VIR_Shader* shader = VIR_LoopOpts_GetShader(loopOpts);
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VIR_CONTROL_FLOW_GRAPH* cfg = VIR_Function_GetCFG(func);*/
    VIR_LoopInfoMgr* loopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(loopOpts);
    VIR_LoopInfo* loopInfo;
    VSC_UL_ITERATOR iter;

    vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(loopInfoMgr));

    for(loopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
        loopInfo != gcvNULL;
        loopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
    {
        if(VIR_LoopInfo_GetParentLoop(loopInfo))
        {
            continue;
        }

        errCode = (*optFunc)(loopInfo, changed);

        if(errCode)
        {
            break;
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_LoopOpts_PerformOnFunction(
    VIR_LoopOpts* loopOpts
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = VIR_LoopOpts_GetFunc(loopOpts);
    VSC_OPTN_LoopOptsOptions* options = VIR_LoopOpts_GetOptions(loopOpts);

    if(VSC_OPTN_LoopOptsOptions_GetOpts(options) == VSC_OPTN_LoopOptsOptions_OPTS_NONE)
    {
        return errCode;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_INPUT))
    {
        VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
        VIR_LOG(dumper, "Loop optimizations start for function\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
    }

    _VIR_LoopOpts_NewLoopInfoMgr(loopOpts);
    if(_VIR_LoopOpts_DetectNatrualLoops(loopOpts))
    {
        _VIR_LoopOpts_ComputeLoopBodies(loopOpts);
        _VIR_LoopOpts_ComputeLoopTree(loopOpts);
        _VIR_LoopOpts_IdentifyBreakContinues(loopOpts);

        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetOpts(options), VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVERSION))
        {
            gctBOOL localChanged;

            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_INVERSION_FUNC_INPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop inversion starts for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
            _VIR_LoopOpts_PerformSpecOptOnLoops(loopOpts, _VIR_LoopInfo_PerformLoopInversionOnLoop, &localChanged);
            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_INVERSION_FUNC_OUTPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop inversion ends for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
        }

        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetOpts(options), VSC_OPTN_LoopOptsOptions_OPTS_LOOP_INVARIANT))
        {
            gctBOOL localChanged;

            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT_FUNC_INPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop invariant code motion starts for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
            _VIR_LoopOpts_PerformSpecOptOnLoops(loopOpts, _VIR_LoopInfo_PerformLoopInvariantCodeMotionOnLoop, &localChanged);
            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_INVARIANT_FUNC_OUTPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop invariant code motion ends for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
        }

        if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetOpts(options), VSC_OPTN_LoopOptsOptions_OPTS_LOOP_UNROLLING))
        {
            gctBOOL localChanged;

            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_INPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop unrolling starts for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
            _VIR_LoopOpts_PerformSpecOptOnLoops(loopOpts, _VIR_LoopInfo_PerformLoopUnrollingOnLoop, &localChanged);
            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_UNROLL_FUNC_OUTPUT))
            {
                VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
                VIR_LOG(dumper, "Loop unrolling ends for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
        }
    }
    _VIR_LoopOpts_DeleteLoopInfoMgr(loopOpts);

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_FUNC_OUTPUT))
    {
        VIR_Dumper* dumper = VIR_LoopOpts_GetDumper(loopOpts);
        VIR_LOG(dumper, "Loop optimizations end for function\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_LoopOpts_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_LOOPOPTS;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
}

VSC_ErrCode
VIR_LoopOpts_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    VSC_OPTN_LoopOptsOptions* options = (VSC_OPTN_LoopOptsOptions*)pPassWorker->basePassWorker.pBaseOption;

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_LoopOptsOptions_GetBeforeShader(options), VSC_OPTN_LoopOptsOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_LoopOptsOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Loop optimizations skip shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_LoopOptsOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Loop optimizations start for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_SHADER_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Before Loop optimizations.", shader, gcvTRUE);
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_LoopOpts loopOpts;
        VIR_Function* func = func_node->function;

        VIR_LoopOpts_Init(&loopOpts, shader, func, options, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);
        errcode = VIR_LoopOpts_PerformOnFunction(&loopOpts);
        VIR_LoopOpts_Final(&loopOpts);

        if(errcode)
        {
            break;
        }
    }

    if(VSC_OPTN_LoopOptsOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
        VIR_LOG(dumper, "Loop optimizations end for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE) ||
        VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_LoopOptsOptions_TRACE_SHADER_OUTPUT))
    {
        VIR_Shader_Dump(gcvNULL, "After Loop optimizations.", shader, gcvTRUE);
    }

    return errcode;
}

