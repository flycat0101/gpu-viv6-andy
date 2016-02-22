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


#include "vir/transform/gc_vsc_vir_cpf.h"

/******************************************************************************
            constant propagation and folding
    Conditional constant propagtion worklist algorithm is uesd. The pesudo
    alogorithm:

    while worklist is not empty
      get n from worklist
      IN[n] = AND OUT[p] p is all the executable predcessors of n
      if n is an assignment (n: A = B op C)
        OUT[n] = fn (IN[n])
        mark outgoing edges as excutable
        if (OUT[n]) changed then add executable Succ(n) to the worklist
      else
       evaluate the condition (true/false/not constant)
       OUT[n] = fn (IN[n])
       mark appropriate edge as executable/not executable
       if (OUT[n]) changed then add executable Succ(n) to the worklist

    This phase keeps its own constant data flow. It does not update
    the du info. Thus it should be called before du is built
    (or rebuilt du afterward).

******************************************************************************/

/* VSC_CPF_DF_VECTOR utility functions */
/* Creation, resize and destroy */
void vscCPF_DV_Initialize(VSC_CPF_DF_VECTOR* pCPF_DV, VSC_MM* pMM, gctINT dvSize)
{
    vscBV_Initialize(&pCPF_DV->hi, pMM, dvSize);
    vscBV_Initialize(&pCPF_DV->low, pMM, dvSize);
}

VSC_CPF_DF_VECTOR* vscCPF_DV_Create(VSC_MM* pMM, gctINT dvSize)
{
    VSC_CPF_DF_VECTOR*   pCPF_DV = gcvNULL;

    pCPF_DV = (VSC_CPF_DF_VECTOR*)vscMM_Alloc(pMM, sizeof(VSC_CPF_DF_VECTOR));

    vscCPF_DV_Initialize(pCPF_DV, pMM, dvSize);

    return pCPF_DV;
}

void vscCPF_DV_Destroy(VSC_CPF_DF_VECTOR* pCPF_DV)
{
    vscBV_Destroy(&pCPF_DV->hi);
    vscBV_Destroy(&pCPF_DV->low);
}

void vscCPF_DV_Finalize(VSC_CPF_DF_VECTOR* pCPF_DV)
{
    vscBV_Finalize(&pCPF_DV->hi);
    vscBV_Finalize(&pCPF_DV->low);
}

gctUINT vscCPF_DV_BitCount(VSC_CPF_DF_VECTOR* pCPF_DV)
{
    gcmASSERT(pCPF_DV->hi.bitCount == pCPF_DV->low.bitCount);

    return (gctUINT) pCPF_DV->hi.bitCount;
}

/* we need to save const value for each const variable at each block,
   since the variable is changed inside the block (from const -> non-const,
   from value1->value2), we need to save the value coming to the block,
   which is got from its predecessors */
typedef struct _VSC_CPF_CONSTKEY
{
    gctUINT     bbId;
    gctUINT     index; /* index = regNo * 4 + channel */
    gctBOOL     isIN;  /* whether it is the const value at the begining of the block */
} VSC_CPF_CONSTKEY;

static gctUINT _HFUNC_CPF_CONSTKEY(const void* ptr)
{
    gctUINT hashVal = (((gctUINT) ((VSC_CPF_CONSTKEY*)ptr)->index) << 4) |
                        (((VSC_CPF_CONSTKEY*)ptr)->bbId & 0xf);

    return hashVal;
}

static gctBOOL _HKCMP_CPF_CONSTKEY(const void* pHashKey1, const void* pHashKey2)
{
    return (((VSC_CPF_CONSTKEY*)pHashKey1)->bbId == ((VSC_CPF_CONSTKEY*)pHashKey2)->bbId)
        && (((VSC_CPF_CONSTKEY*)pHashKey1)->index == ((VSC_CPF_CONSTKEY*)pHashKey2)->index)
        && (((VSC_CPF_CONSTKEY*)pHashKey1)->isIN == ((VSC_CPF_CONSTKEY*)pHashKey2)->isIN);
}

static VSC_CPF_CONSTKEY* _VSC_CPF_NewConstKey(
    VSC_MM      *pMM,
    gctUINT     bbId,
    gctUINT     index,
    gctBOOL     isIN
    )
{
    VSC_CPF_CONSTKEY* constKey = (VSC_CPF_CONSTKEY*)vscMM_Alloc(pMM, sizeof(VSC_CPF_CONSTKEY));
    constKey->bbId = bbId;
    constKey->index = index;
    constKey->isIN = isIN;
    return constKey;
}

static VSC_CPF_Const* _VSC_CPF_NewConstVal(
    VSC_MM              *pMM,
    gctUINT             constValue,
    VIR_PrimitiveTypeId constType
    )
{
    VSC_CPF_Const* constVal = (VSC_CPF_Const*)vscMM_Alloc(pMM, sizeof(VSC_CPF_Const));
    constVal->value = constValue;
    constVal->type = constType;
    return constVal;
}

/* channel name const */
static const char* _VSC_CPF_ChannelName[4] =
{
    "x",
    "y",
    "z",
    "w",
};

#define _VSC_CPF_GetChannelName(channel)     (_VSC_CPF_ChannelName[channel])

static gctBOOL
_VSC_CPF_typeToChannelType(
    VIR_PrimitiveTypeId fromType,
    VIR_PrimitiveTypeId *toType
    )
{
    gctBOOL     retValue = gcvTRUE;
    switch (fromType)
    {
    case VIR_TYPE_FLOAT32:
    case VIR_TYPE_FLOAT_X2:
    case VIR_TYPE_FLOAT_X3:
    case VIR_TYPE_FLOAT_X4:
        *toType = VIR_TYPE_FLOAT32;
        break;
    case VIR_TYPE_INT32:
    case VIR_TYPE_INTEGER_X2:
    case VIR_TYPE_INTEGER_X3:
    case VIR_TYPE_INTEGER_X4:
        *toType = VIR_TYPE_INT32;
        break;
    case VIR_TYPE_UINT32:
    case VIR_TYPE_UINT_X2:
    case VIR_TYPE_UINT_X3:
    case VIR_TYPE_UINT_X4:
        *toType = VIR_TYPE_UINT32;
        break;
    case VIR_TYPE_INT16:
    case VIR_TYPE_INT16_X2:
    case VIR_TYPE_INT16_X3:
    case VIR_TYPE_INT16_X4:
    case VIR_TYPE_INT16_X8:
        *toType = VIR_TYPE_INT16;
        break;
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT16_X2:
    case VIR_TYPE_UINT16_X3:
    case VIR_TYPE_UINT16_X4:
    case VIR_TYPE_UINT16_X8:
        *toType = VIR_TYPE_UINT16;
        break;
    case VIR_TYPE_INT8:
    case VIR_TYPE_INT8_X2:
    case VIR_TYPE_INT8_X3:
    case VIR_TYPE_INT8_X4:
    case VIR_TYPE_INT8_X8:
    case VIR_TYPE_INT8_X16:
        *toType = VIR_TYPE_INT8;
        break;
    case VIR_TYPE_UINT8:
    case VIR_TYPE_UINT8_X2:
    case VIR_TYPE_UINT8_X3:
    case VIR_TYPE_UINT8_X4:
    case VIR_TYPE_UINT8_X8:
    case VIR_TYPE_UINT8_X16:
        *toType = VIR_TYPE_UINT8;
        break;
    case VIR_TYPE_BOOLEAN:
    case VIR_TYPE_BOOLEAN_X2:
    case VIR_TYPE_BOOLEAN_X3:
    case VIR_TYPE_BOOLEAN_X4:
        *toType = VIR_TYPE_BOOLEAN;
        break;
    default:
        retValue = gcvFALSE;
        *toType = VIR_TYPE_FLOAT32;
        break;
    }

    /* this is a basic assumption during CPF: const could only be of the following types */
    gcmASSERT(*toType == VIR_TYPE_FLOAT32 ||
              *toType == VIR_TYPE_INT32 ||
              *toType == VIR_TYPE_INT16 ||
              *toType == VIR_TYPE_INT8 ||
              *toType == VIR_TYPE_UINT32 ||
              *toType == VIR_TYPE_UINT16 ||
              *toType == VIR_TYPE_UINT8 ||
              *toType == VIR_TYPE_BOOLEAN);

    return retValue;
}

static gctBOOL
_VSC_CPF_isTypeFloat(
    VIR_PrimitiveTypeId type
    )
{
    if (type == VIR_TYPE_FLOAT32)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_VSC_CPF_isTypeINT(
    VIR_PrimitiveTypeId type
    )
{
    if (type == VIR_TYPE_INT32 ||
        type == VIR_TYPE_INT16 ||
        type == VIR_TYPE_INT8)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_VSC_CPF_isTypeUINT(
    VIR_PrimitiveTypeId type
    )
{
    if (type == VIR_TYPE_UINT32 ||
        type == VIR_TYPE_UINT16 ||
        type == VIR_TYPE_UINT8)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_VSC_CPF_isTypeBOOL(
    VIR_PrimitiveTypeId type
    )
{
    if (type == VIR_TYPE_BOOLEAN)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/* put the BB into the tail of worklist */
static void
_VSC_CPF_WorkListQueue(
    IN VSC_CPF         *pCPF,
    IN VIR_BASIC_BLOCK *pBB
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)
        vscMM_Alloc(VSC_CPF_GetMM(pCPF), sizeof(VSC_UNI_LIST_NODE_EXT));

    VSC_OPTN_CPFOptions  *pOptions      = VSC_CPF_GetOptions(pCPF);

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);

        VIR_LOG(pDumper, "Adding BB[%d]", pBB->dgNode.id);
        VIR_LOG_FLUSH(pDumper);
    }
    vscULNDEXT_Initialize(worklistNode, pBB);
    QUEUE_PUT_ENTRY(VSC_CPF_GetWorkList(pCPF), worklistNode);
}

/* get the BB from the head of worklist */
static void
_VSC_CPF_WorkListDequeue(
    IN  VSC_CPF            *pCPF,
    OUT VIR_BASIC_BLOCK   **ppBB
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode =
        (VSC_UNI_LIST_NODE_EXT *) QUEUE_GET_ENTRY(VSC_CPF_GetWorkList(pCPF));

    *ppBB = (VIR_BASIC_BLOCK *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(VSC_CPF_GetMM(pCPF), worklistNode);
}

/* whether the BB is inside the worklist, to avoid
   add the existing BB to the worklist*/
static gctBOOL
_VSC_CPF_InWorkList(
    IN VSC_CPF         *pCPF,
    IN VIR_BASIC_BLOCK *pBB
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode =
        (VSC_UNI_LIST_NODE_EXT *) QUEUE_PEEK_HEAD_ENTRY(VSC_CPF_GetWorkList(pCPF));

    while (worklistNode != gcvNULL)
    {
        VIR_BASIC_BLOCK *worklistBB = (VIR_BASIC_BLOCK *)
            vscULNDEXT_GetContainedUserData(worklistNode);
        if (worklistBB == pBB)
        {
            return gcvTRUE;
        }

        worklistNode = vscULNDEXT_GetNextNode(worklistNode);
    }
    return gcvFALSE;
}

static void
_VSC_CPF_Init(
    IN OUT VSC_CPF          *pCPF,
    IN VIR_Shader           *pShader,
    IN VSC_OPTN_CPFOptions  *pOptions,
    IN VIR_Dumper           *pDumper
    )
{
    VSC_CPF_SetShader(pCPF, pShader);
    VSC_CPF_SetOptions(pCPF, pOptions);
    VSC_CPF_SetDumper(pCPF, pDumper);

    vscPMP_Intialize(VSC_CPF_GetPmp(pCPF), gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);
}

static void
_VSC_CPF_Final(
    IN OUT VSC_CPF  *pCPF
    )
{
    VSC_CPF_SetShader(pCPF, gcvNULL);
    VSC_CPF_SetOptions(pCPF, gcvNULL);
    VSC_CPF_SetDumper(pCPF, gcvNULL);
    vscPMP_Finalize(VSC_CPF_GetPmp(pCPF));
}

/* initialize the pBlkFlowArray, which is indexed by BB id.
   flowSize is the number of vreg. */

static VSC_ErrCode
_VSC_CPF_InitializeBlkFlow(
    VIR_FUNC_BLOCK              *pOwnerFB,
    VSC_SIMPLE_RESIZABLE_ARRAY  *pBlkFlowArray,
    VSC_MM                      *pMM,
    gctINT                      flowSize)
{
    VSC_ErrCode        errCode     = VSC_ERR_NONE;
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    vscSRARR_Initialize(pBlkFlowArray,
                        pMM,
                        vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph),
                        sizeof(VSC_CPF_BLOCK_FLOW),
                        gcvNULL);

    /* pBlkFlowArray's element count is the number of BB in CFG */
    vscSRARR_SetElementCount(pBlkFlowArray,
        vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph));

    CFG_ITERATOR_INIT(&basicBlkIter, &pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL;
        pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VSC_CPF_BLOCK_FLOW  *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
            vscSRARR_GetElement(pBlkFlowArray, pBasicBlk->dgNode.id);

        /* every vreg has 4 components. */
        vscCPF_DV_Initialize(&pBlkFlow->inFlow, pMM, flowSize * 4);
        vscCPF_DV_Initialize(&pBlkFlow->outFlow, pMM, flowSize * 4);
    }

    return errCode;
}

static void
_VSC_CPF_FinallizeBlkFlow(
    VIR_FUNC_BLOCK              *pOwnerFB,
    VSC_SIMPLE_RESIZABLE_ARRAY  *pBlkFlowArray
    )
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    CFG_ITERATOR_INIT(&basicBlkIter, &pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL;
        pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VSC_CPF_BLOCK_FLOW *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
            vscSRARR_GetElement(pBlkFlowArray, pBasicBlk->dgNode.id);

        vscCPF_DV_Finalize(&pBlkFlow->inFlow);
        vscCPF_DV_Finalize(&pBlkFlow->outFlow);
    }

    vscSRARR_Finalize(pBlkFlowArray);
}

static VSC_ErrCode
_VSC_CPF_InitFunction(
    VSC_CPF         *pCPF,
    VIR_Function    *pFunc
    )
{
    VSC_ErrCode     errCode     = VSC_ERR_NONE;
    VIR_Shader      *pShader    = VSC_CPF_GetShader(pCPF);
    VIR_CFG         *pCfg       = VIR_Function_GetCFG(pFunc);
    gctUINT         tempCount   = VIR_Shader_GetVirRegCount(pShader);

    gcmASSERT(tempCount > 256);
    tempCount = tempCount - 256;

    /* init the constant data flow*/
    errCode = _VSC_CPF_InitializeBlkFlow(
                pFunc->pFuncBlock,
                VSC_CPF_GetBlkFlowArray(pCPF),
                VSC_CPF_GetMM(pCPF),
                tempCount);

    /* init the worklist */
    QUEUE_INITIALIZE(VSC_CPF_GetWorkList(pCPF));

    /* add the entry bb to the worklist */
    _VSC_CPF_WorkListQueue(pCPF, CFG_GET_ENTRY_BB(pCfg));

    /* initialize the const hash table */
    vscHTBL_Initialize(VSC_CPF_GetConstTable(pCPF),
                    VSC_CPF_GetMM(pCPF),
                    (PFN_VSC_HASH_FUNC) _HFUNC_CPF_CONSTKEY,
                    (PFN_VSC_KEY_CMP) _HKCMP_CPF_CONSTKEY,
                    1024);

    return errCode;
};

static VSC_ErrCode
_VSC_CPF_FinalizeFunction(
    VSC_CPF         *pCPF,
    VIR_Function    *pFunc
    )
{
    VSC_ErrCode     errCode     = VSC_ERR_NONE;

    /* finalize the constant data flow*/
    _VSC_CPF_FinallizeBlkFlow(
        pFunc->pFuncBlock,
        VSC_CPF_GetBlkFlowArray(pCPF));

    /* finalize the worklist */
    QUEUE_FINALIZE(VSC_CPF_GetWorkList(pCPF));

    vscHTBL_Finalize(VSC_CPF_GetConstTable(pCPF));

    return errCode;
};

/* lattice data structure utility functions */
static VSC_CPF_LATTICE
_VSC_CPF_DF_GetLattice(
    VSC_CPF_DF_VECTOR           *tmpFlow,
    gctUINT                     index
    )
{
    VSC_CPF_LATTICE value = VSC_CPF_UNDEFINE;
    if (vscBV_TestBit(&tmpFlow->hi, index) && vscBV_TestBit(&tmpFlow->low, index))
    {
        value = VSC_CPF_NOT_CONSTANT;
    }
    else if (!vscBV_TestBit(&tmpFlow->hi, index) && vscBV_TestBit(&tmpFlow->low, index))
    {
        value = VSC_CPF_CONSTANT;
    }

    return value;
}

static void
_VSC_CPF_DF_SetLatticeNotConst(
    VSC_CPF_DF_VECTOR           *tmpFlow,
    gctUINT                     index
    )
{
    vscBV_SetBit(&tmpFlow->hi, index);
    vscBV_SetBit(&tmpFlow->low, index);
}

static void
_VSC_CPF_DF_SetLatticeConst(
    VSC_CPF_DF_VECTOR           *tmpFlow,
    gctUINT                     index
    )
{
    vscBV_SetBit(&tmpFlow->low, index);
    vscBV_ClearBit(&tmpFlow->hi, index);
}

static VSC_CPF_Const*
_VSC_CPF_GetConstVal(
    VSC_CPF         *pCPF,
    gctUINT         bbId,
    gctUINT         index,
    gctBOOL         isIN
    )
{
    VSC_CPF_Const       *retConst = gcvNULL;
    VSC_CPF_CONSTKEY    constKey;

    constKey.bbId = bbId;
    constKey.index = index;
    constKey.isIN = isIN;
    vscHTBL_DirectTestAndGet(VSC_CPF_GetConstTable(pCPF),
                             &constKey,
                             (void**)&retConst);

    return retConst;
}

static gctBOOL
_VSC_CPF_SetConstVal(
    VSC_CPF             *pCPF,
    gctUINT             bbId,
    gctUINT             index,
    gctBOOL             isIN,
    gctUINT             constValue,
    VIR_PrimitiveTypeId constType
    )
{
    VSC_CPF_Const *dstConstVal = _VSC_CPF_GetConstVal(pCPF, bbId, index, isIN);
    gctBOOL changed = gcvFALSE;

    if (dstConstVal != gcvNULL)
    {
        if(dstConstVal->value != constValue)
        {
            dstConstVal->value = constValue;
            changed = gcvTRUE;
        }

        /*
        ** We need to update type, otherwise it will cause error:
        ** For example:
        ** 1) F2I temp(1), 5.5
        ** 2) I2F temp(1), temp(1)
        */
        if(dstConstVal->type != constType)
        {
            dstConstVal->type = constType;
            changed = gcvTRUE;
        }
    }
    else
    {
        vscHTBL_DirectSet(VSC_CPF_GetConstTable(pCPF),
                          _VSC_CPF_NewConstKey(VSC_CPF_GetMM(pCPF),
                            bbId,
                            index,
                            isIN),
                          _VSC_CPF_NewConstVal(VSC_CPF_GetMM(pCPF),
                            constValue,
                            constType));
        changed = gcvTRUE;
    }

    return changed;
}

static void
_VSC_CPF_RemoveConstVal(
    VSC_CPF         *pCPF,
    gctUINT         bbId,
    gctUINT         index,
    gctBOOL         isIN
    )
{
    VSC_CPF_CONSTKEY    constKey;
    constKey.bbId = bbId;
    constKey.index = index;
    constKey.isIN = isIN;

    vscHTBL_DirectRemove(VSC_CPF_GetConstTable(pCPF), &constKey);
}

static void
_VSC_CPF_SetConst(
    VSC_CPF             *pCPF,
    VSC_CPF_DF_VECTOR   *tmpFlow,
    gctUINT             bbId,
    gctUINT             index,
    gctBOOL             isIN,
    gctUINT             constValue,
    VIR_PrimitiveTypeId constType
    )
{
    /* set the lattice */
    _VSC_CPF_DF_SetLatticeConst(tmpFlow, index);

    /* set the const val in the hash table */
    _VSC_CPF_SetConstVal(pCPF, bbId, index, isIN, constValue, constType);
}

static void
_VSC_CPF_SetNotConst(
    VSC_CPF             *pCPF,
    VSC_CPF_DF_VECTOR   *tmpFlow,
    gctUINT             bbId,
    gctUINT             index,
    gctBOOL             isIN
    )
{
    /* set the lattice to be not-const */
    _VSC_CPF_DF_SetLatticeNotConst(tmpFlow, index);

    /* remove the const val in the hash table */
    _VSC_CPF_RemoveConstVal(pCPF, bbId, index, isIN);
}

static void
_VSC_CPF_CopyConstKey(
    VSC_CPF             *pCPF,
    gctUINT             bbId)
{
    VSC_CPF_BLOCK_FLOW  *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
        vscSRARR_GetElement(VSC_CPF_GetBlkFlowArray(pCPF), bbId);

    VSC_CPF_DF_VECTOR       *tmpFlow = &pBlkFlow->inFlow;
    gctUINT                 i = 0, constIdx;

    /* copy the const hash table */
    while ((constIdx = vscBV_FindSetBitForward(&tmpFlow->low, i)) != (gctUINT)INVALID_BIT_LOC)
    {
        /* VSC_CPF_CONSTANT */
        if (!vscBV_TestBit(&tmpFlow->hi, constIdx))
        {
            VSC_CPF_Const   *constVal = _VSC_CPF_GetConstVal(pCPF, bbId, constIdx, gcvTRUE);
            gcmASSERT(constVal);
            _VSC_CPF_SetConstVal(pCPF, bbId, constIdx, gcvFALSE, constVal->value, constVal->type);
        }

        i = constIdx + 1;
    }
}

/* Dump the constant data flow. */
static void
_VSC_CPF_DataFlow_Dump(
    VSC_CPF                 *pCPF,
    gctUINT                 bbId,
    VSC_CPF_DF_VECTOR       *tmpFlow,
    gctBOOL                 isIN
    )
{
    VIR_Dumper      *pDumper = VSC_CPF_GetDumper(pCPF);
    gctUINT          i = 0, constIdx;

    gcmASSERT(BV_IS_VALID(&tmpFlow->hi));
    gcmASSERT(BV_IS_VALID(&tmpFlow->low));

    /* copy the const hash table */
    while ((constIdx = vscBV_FindSetBitForward(&tmpFlow->low, i)) != (gctUINT)INVALID_BIT_LOC)
    {
        /* VSC_CPF_CONSTANT */
        gctUINT         regNo = constIdx / 4;
        gctUINT         channel = constIdx % 4;

        if (!vscBV_TestBit(&tmpFlow->hi, constIdx))
        {
            VSC_CPF_Const   *constVal = _VSC_CPF_GetConstVal(pCPF, bbId, constIdx, isIN);

            if (constVal != gcvNULL)
            {
                if (_VSC_CPF_isTypeFloat(constVal->type))
                {
                    gctFLOAT f = *(gctFLOAT*)&(constVal->value);
                    VIR_LOG(pDumper, "\ttemp[%d].%s(%f)",
                        regNo,
                        _VSC_CPF_GetChannelName(channel),
                        f);
                }
                else if (_VSC_CPF_isTypeINT(constVal->type))
                {
                    VIR_LOG(pDumper, "\ttemp[%d].%s(%d)",
                        regNo,
                        _VSC_CPF_GetChannelName(channel),
                        constVal->value);
                }
                else if (_VSC_CPF_isTypeUINT(constVal->type))
                {
                    VIR_LOG(pDumper, "\ttemp[%d].%s(%u)",
                        regNo,
                        _VSC_CPF_GetChannelName(channel),
                        constVal->value);
                }
                else if (_VSC_CPF_isTypeBOOL(constVal->type))
                {
                    VIR_LOG(pDumper, "\ttemp[%d].%s(%u)",
                        regNo,
                        _VSC_CPF_GetChannelName(channel),
                        constVal->value);
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

            }
            else
            {
                /* the const is changed to non-const in this block, thus constVal is removed */
                VIR_LOG(pDumper, "\ttemp[%d].%s(changed to non-const)",
                        regNo,
                        _VSC_CPF_GetChannelName(channel));
            }
        }

        i = constIdx + 1;
    }

    VIR_LOG_FLUSH(pDumper);
}

/* Dump the constant data flow for a basic block. */
static void
_VSC_CPF_BB_DataFlow_Dump(
    VSC_CPF                 *pCPF,
    VIR_BB                  *pBB
    )
{
    VIR_Dumper       *pDumper = VSC_CPF_GetDumper(pCPF);

    VSC_CPF_BLOCK_FLOW  *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
        vscSRARR_GetElement(VSC_CPF_GetBlkFlowArray(pCPF),
                            pBB->dgNode.id);

    VIR_LOG(pDumper, "BB[%d] IN FLOW", pBB->dgNode.id);
    VIR_LOG_FLUSH(pDumper);

    _VSC_CPF_DataFlow_Dump(pCPF, pBB->dgNode.id, &pBlkFlow->inFlow, gcvTRUE);

    VIR_LOG(pDumper, "BB[%d] OUT FLOW", pBB->dgNode.id);
    VIR_LOG_FLUSH(pDumper);

    _VSC_CPF_DataFlow_Dump(pCPF, pBB->dgNode.id, &pBlkFlow->outFlow, gcvFALSE);

    VIR_LOG(pDumper, "\n");
    VIR_LOG_FLUSH(pDumper);
}

/* Dump the constant data flow for CFG. */
static void
_VSC_CPF_CFG_DataFlow_Dump(
    VSC_CPF                 *pCPF,
    VIR_CONTROL_FLOW_GRAPH  *pCFG
    )
{
    CFG_ITERATOR     cfg_iter;
    VIR_BB           *pBB;
    VIR_Dumper       *pDumper = VSC_CPF_GetDumper(pCPF);

    VIR_LOG(pDumper, "\n=== Control Flow Graph with constant data flow information ===\n");
    VIR_LOG_FLUSH(pDumper);

    /* Dump basic blocks. */
    CFG_ITERATOR_INIT(&cfg_iter, pCFG);
    for (pBB = CFG_ITERATOR_FIRST(&cfg_iter); pBB != gcvNULL;
         pBB = CFG_ITERATOR_NEXT(&cfg_iter))
    {
        _VSC_CPF_BB_DataFlow_Dump(pCPF, pBB);
    }

    VIR_LOG(pDumper, "\n");
    VIR_LOG_FLUSH(pDumper);
}

/*   dstFlow = srcFlow: only copy the lattice state */
static void
_VSC_CPF_CopyFlowLattice(
    VSC_CPF_DF_VECTOR *dstFlow,
    VSC_CPF_DF_VECTOR *srcFlow)
{
    vscBV_Copy(&dstFlow->hi, &srcFlow->hi);
    vscBV_Copy(&dstFlow->low, &srcFlow->low);
}

/*   dstFlow = srcFlow */
static void
_VSC_CPF_CopyFlow(
    VSC_CPF           *pCPF,
    gctUINT           dstBBId,
    VSC_CPF_DF_VECTOR *dstFlow,
    gctUINT            srcBBId,
    VSC_CPF_DF_VECTOR *srcFlow)
{
    gctUINT i = 0, constIdx;

    _VSC_CPF_CopyFlowLattice(dstFlow, srcFlow);

    /* copy the const hash table */
    while ((constIdx = vscBV_FindSetBitForward(&srcFlow->low, i)) != (gctUINT)INVALID_BIT_LOC)
    {
        /* VSC_CPF_CONSTANT */
        if (!vscBV_TestBit(&srcFlow->hi, constIdx))
        {
            VSC_CPF_Const   *constVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, constIdx, gcvFALSE);

            if (constVal != gcvNULL)
            {
                /* we have two const key to the hash table,
                   one is to save the IN const value,
                   the other is to be changed inside the block.
                   when merging the data flow, we only need to change IN const key,
                   the other one is copy from IN const key at the beginning of BB
                   analysis and changed inside the block. */
                _VSC_CPF_SetConstVal(pCPF, dstBBId, constIdx, gcvTRUE, constVal->value, constVal->type);
            }
        }
        i = constIdx + 1;
    }
}

/* this is AND operation of all predecessors' OUT to get IN
   dstFlow = dstFlow ^ srcFlow */
static gctBOOL
_VSC_CPF_AndFlow(
    VSC_CPF           *pCPF,
    gctUINT           dstBBId,
    VSC_CPF_DF_VECTOR *dstFlow,
    gctUINT            srcBBId,
    VSC_CPF_DF_VECTOR *srcFlow)
{
    gctUINT i;
    gctBOOL changed = gcvFALSE;

    /* to-do optimize this case */
    for (i = 0; i < vscCPF_DV_BitCount(srcFlow); i ++)
    {
        if (_VSC_CPF_DF_GetLattice(dstFlow, i) == VSC_CPF_NOT_CONSTANT)
        {
            continue;
        }
        else if (_VSC_CPF_DF_GetLattice(srcFlow, i) == VSC_CPF_NOT_CONSTANT)
        {
            _VSC_CPF_DF_SetLatticeNotConst(dstFlow, i);
            changed = gcvTRUE;
        }
        else if (_VSC_CPF_DF_GetLattice(dstFlow, i) == VSC_CPF_CONSTANT &&
                 _VSC_CPF_DF_GetLattice(srcFlow, i) == VSC_CPF_CONSTANT)
        {
            /* this is "AND" all predecessors' (src) OUT to get IN (dst),
               thus src is not isIN, dst is isIN */
            VSC_CPF_Const   *dstConstVal = _VSC_CPF_GetConstVal(pCPF, dstBBId, i, gcvTRUE);
            VSC_CPF_Const   *srcConstVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, i, gcvFALSE);

            gcmASSERT(dstConstVal != gcvNULL && srcConstVal != gcvNULL);
            if (dstConstVal->value != srcConstVal->value)
            {
                _VSC_CPF_SetNotConst(pCPF, dstFlow, dstBBId, i, gcvTRUE);
                changed = gcvTRUE;
            }
        }
        else if (_VSC_CPF_DF_GetLattice(srcFlow, i) == VSC_CPF_UNDEFINE)
        {
            continue;
        }
        else if (_VSC_CPF_DF_GetLattice(dstFlow, i) == VSC_CPF_UNDEFINE)
        {
            VSC_CPF_Const   *srcConstVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, i, gcvFALSE);
            gcmASSERT(_VSC_CPF_DF_GetLattice(srcFlow, i) == VSC_CPF_CONSTANT);
            gcmASSERT(srcConstVal != gcvNULL);
            _VSC_CPF_SetConst(pCPF, dstFlow, dstBBId, i, gcvTRUE, srcConstVal->value, srcConstVal->type);
            changed = gcvTRUE;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }

    return changed;
}

static gctBOOL
_VSC_CPF_EqualFlowLattice(
    VSC_CPF_DF_VECTOR *src1Flow,
    VSC_CPF_DF_VECTOR *src2Flow)
{
    if (!vscBV_Equal(&src1Flow->hi, &src2Flow->hi))
    {
        return gcvFALSE;
    }

    if (!vscBV_Equal(&src1Flow->low, &src2Flow->low))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctUINT
_VSC_CPF_GetVRegNo(
    VIR_Instruction            *pInst,
    VIR_Operand                *pOpnd
    )
{
    VIR_OperandInfo opndInfo;
    gctUINT         orgTempCount = VIR_Shader_GetOrgRegCount(VIR_Inst_GetShader(pInst));

    VIR_Operand_GetOperandInfo(pInst, pOpnd, &opndInfo);

    if (VIR_Operand_IsPerPatch(pOpnd) ||
        VIR_Operand_IsArrayedPerVertex(pOpnd))
    {
        return VIR_INVALID_ID;
    }

    if (!VIR_OpndInfo_Is_Virtual_Reg(&opndInfo))
    {
        return VIR_INVALID_ID;
    }
    else if (opndInfo.u1.virRegInfo.virReg >= (256 + orgTempCount))
    {
        return (opndInfo.u1.virRegInfo.virReg - 256);
    }
    else
    {
        return (opndInfo.u1.virRegInfo.virReg);
    }
}

static gctBOOL
_VSC_CPF_isScalarConst(
    VSC_CPF             *pCPF,
    gctUINT             srcBBId,
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    gctUINT8            opndChannel,
    VSC_CPF_DF_VECTOR   *tmpFlow,
    VSC_CPF_Const       *constVal,
    VSC_CPF_LATTICE     *srcLattic
    )
{
    VIR_PrimitiveTypeId type = VIR_TYPE_VOID;
    VSC_CPF_LATTICE lattic = VSC_CPF_UNDEFINE;
    VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(pOpnd);
    gctUINT8 symChannel = VIR_Swizzle_GetChannel(swizzle, opndChannel);

    if (!_VSC_CPF_typeToChannelType(VIR_Operand_GetType(pOpnd), &type))
    {
        return gcvFALSE;
    }

    if(VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_IMMEDIATE)
    {
        /* immediate case */
        constVal->value = pOpnd->u1.uConst;
        constVal->type = type;
        lattic = VSC_CPF_CONSTANT;
    }
    else if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_CONST)
    {
        /* constant vec case */
        VIR_Shader* pShader = VIR_Inst_GetShader(pInst);
        VIR_Const* cur_const = VIR_Shader_GetConstFromId(pShader, pOpnd->u1.constId);
        constVal->value = cur_const->value.vecVal.u32Value[symChannel];
        constVal->type = type;
        lattic = VSC_CPF_CONSTANT;
    }
    else if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)))
    {
        VIR_Shader* pShader = VIR_Inst_GetShader(pInst);
        VIR_Symbol* uniformSym = VIR_Operand_GetSymbol(pOpnd);
        if(isSymUniformCompiletimeInitialized(uniformSym))
        {
            VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniformSym);
            VIR_ConstId constID = VIR_Uniform_GetInitializer(uniform);
            VIR_Const* cur_const = VIR_Shader_GetConstFromId(pShader, constID);
            constVal->value = cur_const->value.vecVal.u32Value[symChannel];
            constVal->type = type;
            lattic = VSC_CPF_CONSTANT;
        }
        else
        {
            lattic = VSC_CPF_NOT_CONSTANT;
        }
    }
    else
    {
        /* vreg */
        VIR_OperandInfo opndInfo;
        VIR_Operand_GetOperandInfo(pInst, pOpnd, &opndInfo);

        if (VIR_OpndInfo_Is_Virtual_Reg(&opndInfo))
        {
            gctUINT regNo = _VSC_CPF_GetVRegNo(pInst, pOpnd);
            if (regNo != VIR_INVALID_ID)
            {
                if (_VSC_CPF_DF_GetLattice(tmpFlow, regNo * 4 + symChannel) == VSC_CPF_CONSTANT)
                {
                    VSC_CPF_Const   *srcConstVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, regNo * 4 + symChannel, gcvFALSE);

                    gcmASSERT(srcConstVal != gcvNULL);

                    constVal->value = srcConstVal->value;
                    constVal->type = srcConstVal->type;
                    lattic = VSC_CPF_CONSTANT;
                }
                else
                {
                    lattic = VSC_CPF_NOT_CONSTANT;
                }
            }
        }
        else
        {
            lattic = VSC_CPF_NOT_CONSTANT;
        }
    }

    if(srcLattic)
    {
        *srcLattic = lattic;
    }

    return lattic == VSC_CPF_CONSTANT;
}

/* only called when all defined channels are constant */
void
_VSC_CPF_SetDestConst(
    VSC_CPF                 *pCPF,
    gctUINT                 srcBBId,
    VIR_Instruction         *pInst,
    gctUINT8                channel,
    VSC_CPF_DF_VECTOR       *tmpFlow,
    VSC_CPF_Const           *constVal
    )
{
    VIR_Operand         *dstOpnd = VIR_Inst_GetDest(pInst);
    VIR_PrimitiveTypeId type = VIR_TYPE_VOID;
    gctUINT             regNo = VIR_INVALID_ID;

    gcmASSERT(dstOpnd != gcvNULL);
    regNo = _VSC_CPF_GetVRegNo(pInst, dstOpnd);

    if (regNo != VIR_INVALID_ID)
    {
        _VSC_CPF_typeToChannelType(VIR_Operand_GetType(dstOpnd), &type);

        if (type != constVal->type)
        {
            gcmASSERT(gcvFALSE);
        }

        /* we change the const value key for non isIN one */
        _VSC_CPF_SetConst(pCPF, tmpFlow, srcBBId, regNo * 4 + channel, gcvFALSE,
                          constVal->value, constVal->type);
    }
}

void
_VSC_CPF_SetDestNotConst(
    VSC_CPF                 *pCPF,
    gctUINT                 srcBBId,
    VIR_Instruction         *pInst,
    gctUINT8                channel,
    VSC_CPF_DF_VECTOR       *tmpFlow
    )
{
    VIR_Operand         *dstOpnd = VIR_Inst_GetDest(pInst);
    gctUINT             regNo = VIR_INVALID_ID;

    gcmASSERT(dstOpnd != gcvNULL);
    regNo = _VSC_CPF_GetVRegNo(pInst, dstOpnd);

    if (regNo != VIR_INVALID_ID)
    {
        VIR_OperandInfo opndInfo;
        gctUINT i;
        VIR_Operand_GetOperandInfo(pInst, dstOpnd, &opndInfo);

        for (i = 0; i < opndInfo.u1.virRegInfo.virRegCount; i++)
        {
            /* we change the const value key for non isIN one */
            _VSC_CPF_SetNotConst(pCPF, tmpFlow, srcBBId, regNo * 4 + channel, gcvFALSE);
            regNo ++;
        }
    }
}


/* when all the channels in the inst's dest are constant, change
   this instruction to a mov instruction, return gcvTRUE if changed */
static gctBOOL
_VSC_CPF_FoldConst(
    VSC_CPF                 *pCPF,
    gctUINT                 srcBBId,
    VIR_Instruction         *pInst
    )
{
    VSC_OPTN_CPFOptions *pOptions  = VSC_CPF_GetOptions(pCPF);
    VIR_Operand         *dstOpnd;
    VIR_PrimitiveTypeId dstType;
    VIR_Enable          dstEnable;
    gctUINT             dstEnableChannelCount;
    gctUINT             regNo = VIR_INVALID_ID;
    VIR_Operand         *srcOpnd;
    gctUINT8            channel;
    gctBOOL             folded = gcvFALSE;

    dstOpnd = VIR_Inst_GetDest(pInst);
    gcmASSERT(dstOpnd != gcvNULL);
    dstType = VIR_Operand_GetType(dstOpnd);
    dstEnable = VIR_Operand_GetEnable(dstOpnd);
    dstEnableChannelCount = VIR_Enable_Channel_Count(dstEnable);

    regNo = _VSC_CPF_GetVRegNo(pInst, dstOpnd);

    if (regNo == VIR_INVALID_ID)
    {
        return gcvFALSE; /* no change */
    }

    srcOpnd = VIR_Inst_GetSource(pInst, 0);
    gcmASSERT(srcOpnd != gcvNULL);

    /* no need to propagte if the mov'src is already const */
    if ((VIR_Inst_GetOpcode(pInst) == VIR_OP_MOV) &&
        (VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_IMMEDIATE ||
         VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_CONST))
    {
        return gcvFALSE; /* no change */
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "[CPF] Fold Const\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    /* immediate case */
    if (dstEnableChannelCount == 1)
    {
        gctUINT channelIndex = 0;
        VSC_CPF_Const   *dstConstVal;
        while(dstEnable != 1 << channelIndex)
        {
            channelIndex++;
        }
        gcmASSERT(channelIndex <= 3);

        dstConstVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, regNo * 4 + channelIndex, gcvFALSE);
        gcmASSERT(dstConstVal != gcvNULL);
        switch(dstConstVal->type)
        {
            case VIR_TYPE_FLOAT32:
                srcOpnd->u1.fConst = gcoMATH_UIntAsFloat(dstConstVal->value);;
                break;
            case VIR_TYPE_INT32:
            case VIR_TYPE_INT16:
            case VIR_TYPE_INT8:
            case VIR_TYPE_UINT32:
            case VIR_TYPE_UINT16:
            case VIR_TYPE_UINT8:
            case VIR_TYPE_BOOLEAN:
                srcOpnd->u1.uConst = dstConstVal->value;
                break;
            default:
                gcmASSERT(gcvFALSE);
        }
        gcmASSERT(dstType < VIR_TYPE_LAST_PRIMITIVETYPE);

        VIR_Operand_SetOpKind(srcOpnd, VIR_OPND_IMMEDIATE);
        VIR_Operand_SetType(srcOpnd, VIR_GetTypeComponentType(dstType));
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetConditionOp(pInst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(pInst, 1);
        folded = gcvTRUE;
    }
    else if (ENABLE_FULL_NEW_LINKER)
    {
        /* const vec case */
        VIR_ConstVal    new_const_val;
        VIR_ConstId     new_const_id;
        VIR_Const       *new_const;
        VIR_Swizzle     new_swizzle;
        gctUINT8        constChannel = 0;
        gctUINT8        lastEnableChannel = 0;

        memset(&new_const_val, 0, sizeof(VIR_ConstVal));

        for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
        {
            if (dstEnable & (1 << channel))
            {
                VSC_CPF_Const *dstConstVal = _VSC_CPF_GetConstVal(pCPF, srcBBId, regNo * 4 + channel, gcvFALSE);
                gcmASSERT(dstConstVal != gcvNULL);

                switch(dstConstVal->type)
                {
                    case VIR_TYPE_FLOAT32:
                        new_const_val.vecVal.f32Value[constChannel] = gcoMATH_UIntAsFloat(dstConstVal->value);
                        break;
                    case VIR_TYPE_INT32:
                    case VIR_TYPE_INT16:
                    case VIR_TYPE_INT8:
                    case VIR_TYPE_UINT32:
                    case VIR_TYPE_UINT16:
                    case VIR_TYPE_UINT8:
                    case VIR_TYPE_BOOLEAN:
                        new_const_val.vecVal.u32Value[constChannel] = dstConstVal->value;
                        break;
                    default:
                        gcmASSERT(gcvFALSE);
                }
                lastEnableChannel = channel;
            }
            else
            {
                new_const_val.vecVal.u32Value[constChannel] = 0;
            }
            constChannel++;
        }

        /* Update the source type. */
        dstType = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(dstType),
                                                  lastEnableChannel + 1,
                                                  1);

        if (lastEnableChannel == 0)
        {
            new_swizzle = VIR_SWIZZLE_XXXX;
        }
        else if (lastEnableChannel == 1)
        {
            new_swizzle = VIR_SWIZZLE_XYYY;
        }
        else if (lastEnableChannel == 1)
        {
            new_swizzle = VIR_SWIZZLE_XYZZ;
        }
        else
        {
            new_swizzle = VIR_SWIZZLE_XYZW;
        }

        VIR_Shader_AddConstant(VSC_CPF_GetShader(pCPF),
                               dstType, &new_const_val, &new_const_id);
        new_const = VIR_Shader_GetConstFromId(VSC_CPF_GetShader(pCPF), new_const_id);
        new_const->type = dstType;
        VIR_Operand_SetConstId(srcOpnd, new_const_id);
        VIR_Operand_SetOpKind(srcOpnd, VIR_OPND_CONST);
        VIR_Operand_SetType(srcOpnd, dstType);
        VIR_Operand_SetSwizzle(srcOpnd, new_swizzle);
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetConditionOp(pInst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(pInst, 1);
        folded = gcvTRUE;
    }

    if(folded && VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
                                VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "[CPF] to instruction\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
        VIR_LOG(pDumper, "\n");
        VIR_LOG_FLUSH(pDumper);
    }

    return gcvTRUE;
}

/* when all the channels in the inst's src are constant,
   we propagate the const to its src */
static void
_VSC_CPF_PropagateConst(
    VSC_CPF                     *pCPF,
    VIR_Instruction             *pInst,
    VIR_Enable                  dstEnable,
    VIR_Operand                 *pOpnd,
    VSC_CPF_Const               *pConst
    )
{
    VSC_OPTN_CPFOptions *pOptions  = VSC_CPF_GetOptions(pCPF);
    VIR_PrimitiveTypeId srcType = VIR_TYPE_VOID;
    VIR_Swizzle opndSwizzle = VIR_Operand_GetSwizzle(pOpnd);
    VIR_OpCode opCode;
    gctUINT8 channel;
    gctUINT8 dstEnableCount, opndSwizzleCount;

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "[CPF] Propagate const\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
    }

    _VSC_CPF_typeToChannelType(VIR_Operand_GetType(pOpnd), &srcType);
    dstEnableCount = VIR_Enable_Channel_Count(dstEnable);
    opndSwizzleCount = VIR_Swizzle_Channel_Count(opndSwizzle);

    /* immediate case */
    if (dstEnableCount == 1 || opndSwizzleCount == 1)
    {
        gctFLOAT floatValue;
        gctUINT  intValue;

        for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
        {
            if (dstEnable & (1 << channel))
            {
                break;
            }
        }

        if (_VSC_CPF_isTypeFloat(pConst[channel].type))
        {
            floatValue = gcoMATH_UIntAsFloat(pConst[channel].value);
            intValue = (gctUINT)floatValue;
        }
        else
        {
            intValue = pConst[channel].value;
            floatValue = (gctFLOAT) intValue;
        }

        if (_VSC_CPF_isTypeFloat(srcType))
        {
            pOpnd->u1.fConst = floatValue;
        }
        else if (_VSC_CPF_isTypeINT(srcType))
        {
            pOpnd->u1.iConst = intValue;
        }
        else if (_VSC_CPF_isTypeUINT(srcType))
        {
            pOpnd->u1.uConst = intValue;
        }
        else if (_VSC_CPF_isTypeBOOL(srcType))
        {
            pOpnd->u1.uConst = intValue;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        gcmASSERT(srcType < VIR_TYPE_LAST_PRIMITIVETYPE);

        VIR_Operand_SetType(pOpnd, VIR_GetTypeComponentType(srcType));
        VIR_Operand_SetOpKind(pOpnd, VIR_OPND_IMMEDIATE);
        /* change the immediate to EvisModifier for EVIS inst if it is modifier operand */
        opCode = VIR_Inst_GetOpcode(pInst);
        if (VIR_OPCODE_isVXOnly(opCode))
        {
            int evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(opCode);
            VIR_Operand *evisOpnd = VIR_Inst_GetSource(pInst, evisSrcNo);

            if (evisOpnd == pOpnd)
            {
                /* set newSrc to EVISModifier operand */
                VIR_Operand_SetOpKind(pOpnd, VIR_OPND_EVIS_MODIFIER);
            }
        }
    }
    else
    {
        /* const vec case */
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "[CPF] to instruction\n");
        VIR_Inst_Dump(pDumper, pInst);
        VIR_LOG_FLUSH(pDumper);
        VIR_LOG(pDumper, "\n");
        VIR_LOG_FLUSH(pDumper);
    }
}

static gctBOOL
_VSC_CPF_GetConditionResult(
    VIR_ConditionOp condOp,
    VSC_CPF_Const* constVal0,
    VSC_CPF_Const* constVal1,
    gctBOOL* result
    )
{
    gcmASSERT(constVal0 && result);

    switch(constVal0->type)
    {
        case VIR_TYPE_FLOAT32:
        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT8:
        case VIR_TYPE_BOOLEAN:
            break;
        default:
            gcmASSERT(0);
            return gcvFALSE;
    }

    switch(condOp)
    {
        case VIR_COP_ALWAYS:
            *result = gcvTRUE;
            return gcvTRUE;
        case VIR_COP_GREATER:
        {
            gcmASSERT(constVal1);
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal1->value);
                    *result = f0 > f1;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value > (gctINT32)constVal1->value;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value > constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_LESS:
        {
            gcmASSERT(constVal1);
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal1->value);
                    *result = f0 < f1;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value < (gctINT32)constVal1->value;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value < constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_GREATER_OR_EQUAL:
        {
            gcmASSERT(constVal1);
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal1->value);
                    *result = f0 >= f1;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value >= (gctINT32)constVal1->value;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value >= constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_LESS_OR_EQUAL:
        {
            gcmASSERT(constVal1);
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal1->value);
                    *result = f0 <= f1;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value <= (gctINT32)constVal1->value;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value <= constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_EQUAL:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value == constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_NOT_EQUAL:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value != constVal1->value;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_AND:
        {
            gcmASSERT(constVal1);
            gcmASSERT(constVal0->type != VIR_TYPE_FLOAT32);
            *result = constVal0->value & constVal1->value;
            return gcvTRUE;
        }
        case VIR_COP_OR:
        {
            gcmASSERT(constVal1);
            gcmASSERT(constVal0->type != VIR_TYPE_FLOAT32);
            *result = constVal0->value | constVal1->value;
            return gcvTRUE;
        }
        case VIR_COP_XOR:
        {
            gcmASSERT(constVal1);
            gcmASSERT(constVal0->type != VIR_TYPE_FLOAT32);
            *result = constVal0->value ^ constVal1->value;
            return gcvTRUE;
        }
        case VIR_COP_NOT:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value == 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_NOT_ZERO:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value != 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_GREATER_OR_EQUAL_ZERO:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);

                    *result = f0 >= 0.0;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value >= 0;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = (gctINT32)constVal0->value >= 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_GREATER_ZERO:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);

                    *result = f0 > 0.0;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value > 0;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value > 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_LESS_OREQUAL_ZERO:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);

                    *result = f0 <= 0.0;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value <= 0;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = constVal0->value <= 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_LESS_ZERO:
        {
            switch(constVal0->type)
            {
                case VIR_TYPE_FLOAT32:
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal0->value);

                    *result = f0 < 0.0;
                    return gcvTRUE;
                }
                case VIR_TYPE_INT32:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT8:
                    *result = (gctINT32)constVal0->value < 0;
                    return gcvTRUE;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT8:
                case VIR_TYPE_BOOLEAN:
                    *result = (gctINT32)constVal0->value < 0;
                    return gcvTRUE;
                default:
                    gcmASSERT(0);
                    return gcvFALSE;
            }
        }
        case VIR_COP_FINITE:
        case VIR_COP_INFINITE:
        case VIR_COP_NAN:
        case VIR_COP_NORMAL:
        {
            gcmASSERT(constVal0->type == VIR_TYPE_FLOAT32);
            gcmASSERT(0);
            return gcvFALSE;
        }
        case VIR_COP_ANYMSB:
        case VIR_COP_ALLMSB:
        {
            gcmASSERT(0);
            return gcvFALSE;
        }
        case VIR_COP_SELMSB:
        {
            *result = constVal0->value & 0x8000;
            return gcvTRUE;
        }
        case VIR_COP_UCARRY:
        case VIR_COP_HELPER:
        case VIR_COP_NOTHELPER:
        default:
            gcmASSERT(0);
            return gcvFALSE;
    }
}

static void
_VSC_CPF_EvaluateConst(
    VIR_Instruction*    pInst,
    VSC_CPF_Const       *constVal,
    VSC_CPF_Const       *resultVal,
    VSC_CPF_LATTICE     *srcLattice
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);
    VIR_PrimitiveTypeId dstType = VIR_TYPE_VOID;
    _VSC_CPF_typeToChannelType(VIR_Operand_GetType(VIR_Inst_GetDest(pInst)), &dstType);

    switch (opcode)
    {
        case VIR_OP_MOV:
        {
            gcmASSERT(srcLattice[0] == VSC_CPF_CONSTANT);       /* otherwise we cannot evaluate */
            resultVal->value = constVal[0].value;
            break;
        }
        case VIR_OP_ABS:
        {
            gcmASSERT(srcLattice[0] == VSC_CPF_CONSTANT);       /* otherwise we cannot evaluate */
            if (_VSC_CPF_isTypeFloat(dstType))
            {
                gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);
                resultVal->value = gcoMATH_FloatAsUInt(f0 >= 0.0 ? f0 : -f0);
            }
            else
            {
                resultVal->value = ((gctINT)constVal[0].value) >= 0 ? constVal[0].value : (gctUINT)(-(gctINT)constVal[0].value);
            }
            break;
        }
        case VIR_OP_ADD:
        {
            if (_VSC_CPF_isTypeFloat(dstType))
            {
                gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);
                gctFLOAT f1 = *(gctFLOAT*) &(constVal[1].value);

                resultVal->value = gcoMATH_FloatAsUInt(f0 + f1);
            }
            else
            {
                resultVal->value = constVal[0].value + constVal[1].value;
            }
            break;
        }
        case VIR_OP_AND_BITWISE:
        {
            resultVal->value = constVal[0].value & constVal[1].value;
            break;
        }
        case VIR_OP_OR_BITWISE:
        {
            resultVal->value = constVal[0].value | constVal[1].value;
            break;
        }
        case VIR_OP_XOR_BITWISE:
        {
            resultVal->value = constVal[0].value ^ constVal[1].value;
            break;
        }
        case VIR_OP_SUB:
        {
            if(srcLattice[0] == VSC_CPF_CONSTANT)
            {
                gcmASSERT(srcLattice[1] == VSC_CPF_CONSTANT);   /* otherwise we cannot evaluate */
                if(_VSC_CPF_isTypeFloat(dstType))
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal[1].value);

                    resultVal->value = gcoMATH_FloatAsUInt(f0 - f1);
                }
                else
                {
                    resultVal->value = constVal[0].value - constVal[1].value;
                }
            }
            else
            {
                gcmASSERT(srcLattice[1] != VSC_CPF_CONSTANT);   /* must be a = b - b */
                resultVal->value = 0;
            }
            break;
        }
        case VIR_OP_MUL:
        {
            if(srcLattice[0] != VSC_CPF_CONSTANT || srcLattice[1] != VSC_CPF_CONSTANT)
            {
                resultVal->value = 0;
            }
            else
            {
                if(_VSC_CPF_isTypeFloat(dstType))
                {
                    gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);
                    gctFLOAT f1 = *(gctFLOAT*) &(constVal[1].value);

                    resultVal->value = gcoMATH_FloatAsUInt(f0 * f1);
                }
                else
                {
                    resultVal->value = constVal[0].value * constVal[1].value;
                }
            }
            break;
        }
        case VIR_OP_MULHI:
        {
            if(srcLattice[0] != VSC_CPF_CONSTANT || srcLattice[1] != VSC_CPF_CONSTANT)
            {
                resultVal->value = 0;
            }
            else
            {
                switch(dstType)
                {
                    case VIR_TYPE_INT8:
                    case VIR_TYPE_INT16:
                    case VIR_TYPE_INT32:
                    {
                        long long ll0 = (gctINT)constVal[0].value;
                        long long ll1 = (gctINT)constVal[1].value;
                        long long res = ll0 * ll1;
                        resultVal->value = (gctUINT)(res >> 32);
                        break;
                    }
                    case VIR_TYPE_UINT8:
                    case VIR_TYPE_UINT16:
                    case VIR_TYPE_UINT32:
                    case VIR_TYPE_BOOLEAN:
                    {
                        unsigned long long ll0 = constVal[0].value;
                        unsigned long long ll1 = constVal[1].value;
                        unsigned long long res = ll0 * ll1;
                        resultVal->value = (gctUINT)(res >> 32);
                        break;
                    }
                    default:
                        gcmASSERT(0);
                }
            }
            break;
        }
        case VIR_OP_NEG:
        {
            gcmASSERT(srcLattice[0] == VSC_CPF_CONSTANT);       /* otherwise we cannot evaluate */
            if(_VSC_CPF_isTypeFloat(dstType))
            {
                gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);

                resultVal->value = gcoMATH_FloatAsUInt(-f0);
            }
            else
            {
                resultVal->value = (gctUINT)(-(gctINT)constVal[0].value);
            }
            break;
        }
        case VIR_OP_CMP:
        {
            gctBOOL cmpResult = gcvFALSE;

            if(!_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal[0], &constVal[1], &cmpResult))
            {
                gcmASSERT(0);
            }
            if(_VSC_CPF_isTypeFloat(dstType))
            {
                resultVal->value = gcoMATH_FloatAsUInt(cmpResult ? 1.0f : 0.0f);
            }
            else if (dstType == VIR_TYPE_INT8 || dstType == VIR_TYPE_UINT8)
            {
                resultVal->value = cmpResult ? 0xff : 0;
            }
            else if (dstType == VIR_TYPE_INT16 || dstType == VIR_TYPE_UINT16)
            {
                resultVal->value = cmpResult ? 0xffff : 0;
            }
            else
            {
                resultVal->value = cmpResult ? 0xffffffff : 0;
            }
            break;
        }
        case VIR_OP_SELECT:
        case VIR_OP_AQ_SELECT:
        {
            gctBOOL cmpResult = gcvFALSE;
            if(!_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal[0], &constVal[1], &cmpResult))
            {
                gcmASSERT(0);
            }
            if(cmpResult)
            {
                resultVal->value = constVal[1].value;
            }
            else
            {
                resultVal->value = constVal[2].value;
            }
            break;
        }
        case VIR_OP_CONV:
        {
            if(!_VSC_CPF_isTypeFloat(dstType))
            {
                switch(constVal[0].type)
                {
                    case VIR_TYPE_FLOAT32:
                        resultVal->value = (gctUINT)(gctINT)*(gctFLOAT*)&constVal[0].value;
                        break;
                    default:
                        resultVal->value = constVal[0].value;
                        break;
                }
            }
            else
            {
                switch(constVal[0].type)
                {
                    case VIR_TYPE_INT32:
                    case VIR_TYPE_INT16:
                    case VIR_TYPE_INT8:
                        resultVal->value = gcoMATH_FloatAsUInt((gctFLOAT)(gctINT32)constVal[0].value);
                        break;
                    case VIR_TYPE_UINT32:
                    case VIR_TYPE_UINT16:
                    case VIR_TYPE_UINT8:
                    case VIR_TYPE_BOOLEAN:
                        resultVal->value = gcoMATH_FloatAsUInt((gctFLOAT)constVal[0].value);
                        break;
                    case VIR_TYPE_FLOAT32:
                        resultVal->value = gcoMATH_FloatAsUInt((gctFLOAT)(gctINT32)*(gctFLOAT*)&constVal[0].value);
                        break;
                    default:
                        gcmASSERT(0);
                }
            }
            break;
        }
        case VIR_OP_LSHIFT:
        {
            resultVal->value = constVal[0].value << constVal[1].value;
            break;
        }
        case VIR_OP_RSHIFT:
        {
            VIR_PrimitiveTypeId src0Type = VIR_TYPE_VOID;
            _VSC_CPF_typeToChannelType(VIR_Operand_GetType(VIR_Inst_GetSource(pInst, 0)), &src0Type);

            switch(src0Type)
            {
                case VIR_TYPE_INT8:
                case VIR_TYPE_INT16:
                case VIR_TYPE_INT32:
                {
                    resultVal->value = ((gctINT)constVal[0].value) >> constVal[1].value;
                    break;
                }
                case VIR_TYPE_UINT8:
                case VIR_TYPE_UINT16:
                case VIR_TYPE_UINT32:
                case VIR_TYPE_BOOLEAN:
                {
                    resultVal->value = constVal[0].value >> constVal[1].value;
                    break;
                }
                default:
                    gcmASSERT(0);
            }

            break;
        }
        case VIR_OP_RCP:
        {
            gctFLOAT f0 = *(gctFLOAT*) &(constVal[0].value);

            resultVal->value = gcoMATH_FloatAsUInt(1.0f / f0);
            break;
        }
        default:
            gcmASSERT(gcvFALSE);
            break;
    }
    resultVal->type = dstType;
}

static gctBOOL
_VSC_CPF_isAssignmentOpcode(
    VIR_OpCode      opcode
    )
{
    if (opcode == VIR_OP_MOV ||
        opcode == VIR_OP_ABS ||
        opcode == VIR_OP_ADD ||
        opcode == VIR_OP_AND_BITWISE ||
        opcode == VIR_OP_OR_BITWISE ||
        opcode == VIR_OP_XOR_BITWISE ||
        opcode == VIR_OP_SUB ||
        opcode == VIR_OP_MUL ||
        opcode == VIR_OP_MULHI ||
        opcode == VIR_OP_NEG ||
        opcode == VIR_OP_CMP ||
        opcode == VIR_OP_SELECT ||
        opcode == VIR_OP_AQ_SELECT ||
        opcode == VIR_OP_CONV ||
        opcode == VIR_OP_RCP ||
        opcode == VIR_OP_LSHIFT ||
        opcode == VIR_OP_RSHIFT)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}


static gctBOOL
_VSC_CPF_Propagatable(
    VIR_OpCode      opcode
    )
{
    if (opcode == VIR_OP_LDARR)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_VSC_CPF_SourceChannelCouldBeNonConst(
    VSC_CPF             *pCPF,
    gctUINT             srcBBId,
    VIR_Instruction     *pInst,
    gctUINT             srcID,
    gctUINT8            channel,
    VSC_CPF_DF_VECTOR   *tmpFlow
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);
    VSC_CPF_Const constVal0, constVal1;

    switch(opcode)
    {
        case VIR_OP_ADD:
            break;
        case VIR_OP_SUB:
            break;
        case VIR_OP_MUL:
        case VIR_OP_MULHI:
        {
            gctBOOL isConst;
            switch(srcID)
            {
                case 0:
                    isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 1), channel, tmpFlow, &constVal1, gcvNULL);
                    return isConst && constVal1.value == 0;
                case 1:
                    isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 0), channel, tmpFlow, &constVal0, gcvNULL);
                    return isConst && constVal0.value == 0;
            }
            break;
        }
        case VIR_OP_SELECT:
        {
            gcmASSERT(VIR_ConditionOp_SingleOperand(VIR_Inst_GetConditionOp(pInst)));
            switch(srcID)
            {
                case 0:
                    return gcvFALSE;
                case 1:
                case 2:
                {
                    gctBOOL isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 0), channel, tmpFlow, &constVal0, gcvNULL);
                    gctBOOL cmpResult = gcvFALSE;
                    if(!isConst)
                    {
                        gcmASSERT(0);
                    }
                    if(_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal0, gcvNULL, &cmpResult))
                    {
                        return (srcID == 1) ^ cmpResult;
                    }
                    else
                    {
                        return gcvFALSE;
                    }
                }
            }
            break;
        }
        case VIR_OP_AQ_SELECT:
        {
            switch(srcID)
            {
                case 0:
                    return gcvFALSE;
                case 1:
                {
                    if(VIR_ConditionOp_SingleOperand(VIR_Inst_GetConditionOp(pInst)))
                    {
                        gctBOOL isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 0), channel, tmpFlow, &constVal0, gcvNULL);
                        gctBOOL cmpResult = gcvFALSE;
                        if(!isConst)
                        {
                            gcmASSERT(0);
                        }
                        if(_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal0, gcvNULL, &cmpResult))
                        {
                            return !cmpResult;
                        }
                        else
                        {
                            return gcvFALSE;
                        }
                    }
                    else if(VIR_ConditionOp_DoubleOperand(VIR_Inst_GetConditionOp(pInst)))
                    {
                        return gcvFALSE;
                    }
                }
                case 2:
                {
                    if(VIR_ConditionOp_SingleOperand(VIR_Inst_GetConditionOp(pInst)))
                    {
                        gctBOOL isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 0), channel, tmpFlow, &constVal0, gcvNULL);
                        gctBOOL cmpResult = gcvFALSE;
                        if(!isConst)
                        {
                            gcmASSERT(0);
                        }
                        if(_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal0, gcvNULL, &cmpResult))
                        {
                            return cmpResult;
                        }
                        else
                        {
                            return gcvFALSE;
                        }
                    }
                    else if(VIR_ConditionOp_DoubleOperand(VIR_Inst_GetConditionOp(pInst)))
                    {
                        gctBOOL isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 0), channel, tmpFlow, &constVal0, gcvNULL);
                        gctBOOL cmpResult = gcvFALSE;
                        if(!isConst)
                        {
                            gcmASSERT(0);
                        }
                        isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, VIR_Inst_GetSource(pInst, 1), channel, tmpFlow, &constVal1, gcvNULL);
                        if(!isConst)
                        {
                            gcmASSERT(0);
                        }
                        if(_VSC_CPF_GetConditionResult(VIR_Inst_GetConditionOp(pInst), &constVal0, &constVal1, &cmpResult))
                        {
                            return cmpResult;
                        }
                        else
                        {
                            return gcvFALSE;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return gcvFALSE;
}

/* depending on the flag, analysisOnly, it performans constant data flow
   analysis for an instruction or do the propagate/folding at the same time.

   algorithm:
    if pInst is an assignment (A = B op C)
       A = VSC_CPF_NOT_CONSTANT if B == VSC_CPF_NOT_CONSTANT or C == VSC_CPF_NOT_CONSTANT
       A = VSC_CPF_UNDEFINE if B == VSC_CPF_UNDEFINE or C == VSC_CPF_UNDEFINE
       A = VSC_CPF_CONSTANT (A = Cb op Cc) if B == VSC_CPF_CONSTANT and C == VSC_CPF_CONSTANT
    if pInst is not assignment (e.g., jmpc L, B, C)
       if B or C is constant then propagate the constant to B or C

*/
static VSC_ErrCode
_VSC_CPF_PerformOnInst(
    VSC_CPF             *pCPF,
    gctUINT             srcBBId,
    VIR_Instruction     *pInst,
    VSC_CPF_DF_VECTOR   *tmpFlow,
    gctBOOL             analysisOnly
    )
{
    VSC_ErrCode errCode    = VSC_ERR_NONE;
    VIR_OpCode  opcode = VIR_Inst_GetOpcode(pInst);

    if (opcode == VIR_OP_CALL)
    {
        /* since it is not the inter function, the return temp should be not-const */
        VIR_Function *pCalleeFunc = VIR_Inst_GetCallee(pInst);
        gctINT  i = 0, channel;

        gcmASSERT(pCalleeFunc != gcvNULL);

        for (i = pCalleeFunc->tempIndexStart; i < pCalleeFunc->tempIndexCount; i++)
        {
            for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
            {
                _VSC_CPF_SetNotConst(pCPF, tmpFlow, srcBBId, i * 4 + channel, gcvFALSE);
            }
        }

    }
    if (_VSC_CPF_isAssignmentOpcode(opcode))
    {
        gctUINT i;
        VIR_Operand *srcOpnd = gcvNULL;
        VIR_Operand *dstOpnd = VIR_Inst_GetDest(pInst);
        VIR_Enable  dstEnable = VIR_Operand_GetEnable(dstOpnd);

        gctUINT8    channel;
        gctBOOL     allChannelConst = gcvTRUE;

        if (_VSC_CPF_GetVRegNo(pInst, dstOpnd) != VIR_INVALID_ID)
        {
            gctBOOL evaluable[VIR_CHANNEL_NUM] = {gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};
            VSC_CPF_Const resultVal[VIR_CHANNEL_NUM];

            /* for each channel, check whether all src is constant.
               if yes, then set the dst's corresponding channel to be constant */
            for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
            {
                VSC_CPF_Const vecVal[VIR_MAX_SRC_NUM];
                VSC_CPF_LATTICE srcLattic[VIR_MAX_SRC_NUM] =
                    {VSC_CPF_UNDEFINE, VSC_CPF_UNDEFINE, VSC_CPF_UNDEFINE, VSC_CPF_UNDEFINE};

                if (!(dstEnable & (1 << channel)))
                {
                    continue;
                }

                for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
                {
                    srcOpnd = VIR_Inst_GetSource(pInst, i);

                    if (_VSC_CPF_GetVRegNo(pInst, srcOpnd) != VIR_INVALID_ID ||
                        (VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_isUniform(VIR_Operand_GetSymbol(srcOpnd)) && isSymUniformCompiletimeInitialized(VIR_Operand_GetSymbol(srcOpnd))) ||
                        VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_IMMEDIATE ||
                        VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_CONST)
                    {
                        gctBOOL isConst;
                        isConst = _VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, srcOpnd, channel, tmpFlow, &vecVal[i], &srcLattic[i]);
                        if(!isConst)
                        {
                            gcmASSERT(_VSC_CPF_GetVRegNo(pInst, srcOpnd) != VIR_INVALID_ID);
                            if(!_VSC_CPF_SourceChannelCouldBeNonConst(pCPF, srcBBId, pInst, i, channel, tmpFlow))
                            {
                                evaluable[channel] = gcvFALSE;
                                break;
                            }
                        }
                    }
                    else
                    {
                        evaluable[channel] = gcvFALSE;
                        break;
                    }
                }

                if (evaluable[channel])
                {
                    _VSC_CPF_EvaluateConst(pInst, &vecVal[0], &resultVal[channel], &srcLattic[0]);
                }
                else
                {
                    allChannelConst = gcvFALSE;
                }
            }

            /* DEST and SRC may use the same register, so set DEST as CONST/NON-CONST after evaluate all channels. */
            for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
            {
                if (!(dstEnable & (1 << channel)))
                {
                    continue;
                }
                if (evaluable[channel])
                {
                    _VSC_CPF_SetDestConst(pCPF, srcBBId, pInst, channel, tmpFlow, &resultVal[channel]);
                }
                else
                {
                    _VSC_CPF_SetDestNotConst(pCPF, srcBBId, pInst, channel, tmpFlow);
                }
            }

            if (allChannelConst && !analysisOnly)
            {
                /* constant folding */
                _VSC_CPF_FoldConst(pCPF, srcBBId, pInst);
            }
        }
    }
    else
    {
        /* not assignment case or assignment that we don't current handle.
           we need to
           1) set dst to be VSC_CPF_NOT_CONSTANT
           2) propagate the const src to this src if we are doing the transformation
        */
        gctUINT i,j;
        VIR_Operand *srcOpnd = gcvNULL;
        VIR_Swizzle srcSwizzle = VIR_SWIZZLE_X;
        VIR_Enable  dstEnable = VIR_ENABLE_NONE;
        gctUINT8    channel;

        VIR_Operand *dstOpnd = VIR_Inst_GetDest(pInst);

        if (dstOpnd != gcvNULL &&
            _VSC_CPF_GetVRegNo(pInst, dstOpnd) != VIR_INVALID_ID)
        {
            dstEnable = VIR_Operand_GetEnable(dstOpnd);
            for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
            {
                if (dstEnable & (1 << channel))
                {
                    _VSC_CPF_SetDestNotConst(pCPF, srcBBId, pInst, channel, tmpFlow);
                }
            }
        }

        if (!analysisOnly &&
            _VSC_CPF_Propagatable(opcode))
        {
            /* For each src, check wether all its channels are constant.
               If yes, we need to propagate the const to its src */

            /* store instruction's first src is to dest */
            gctUINT startIndex = 0;
            if (VIR_OPCODE_isMemSt(opcode))
            {
                startIndex = 1;
            }
            for (i = startIndex; i < VIR_Inst_GetSrcNum(pInst); i++)
            {
                gctBOOL     allChannelConst = gcvTRUE;
                VSC_CPF_Const vecVal[VIR_CHANNEL_NUM];
                VSC_CPF_LATTICE srcLattic = VSC_CPF_UNDEFINE;

                srcOpnd = VIR_Inst_GetSource(pInst, i);

                /* Check the texld modifier operand. */
                if (VIR_OPCODE_isTexLd(opcode) &&
                    VIR_Operand_GetOpKind(srcOpnd) == VIR_OPND_TEXLDPARM &&
                    i == 2)
                {
                    VIR_Operand_TexldModifier * texldModifier = (VIR_Operand_TexldModifier *)srcOpnd;
                    VIR_Operand * texldParm;

                    for (j = 0; j < VIR_TEXLDMODIFIER_COUNT; j++)
                    {
                        texldParm = texldModifier->tmodifier[j];

                        if (texldParm == gcvNULL) continue;

                        srcSwizzle = VIR_Operand_GetSwizzle(texldParm);

                        if (dstOpnd == gcvNULL ||
                            _VSC_CPF_GetVRegNo(pInst, dstOpnd) == VIR_INVALID_ID ||
                            !VIR_OPCODE_isComponentwise(opcode))
                        {
                            dstEnable = VIR_Swizzle_2_Enable(srcSwizzle);
                        }

                        gcmASSERT(dstEnable != VIR_ENABLE_NONE);

                        if (_VSC_CPF_GetVRegNo(pInst, texldParm) != VIR_INVALID_ID)
                        {
                            for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                            {
                                if (dstEnable & (1 << channel))
                                {
                                    if (!_VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, texldParm, channel, tmpFlow,
                                            &vecVal[channel], &srcLattic))
                                    {
                                        allChannelConst = gcvFALSE;
                                        break;
                                    }
                                }
                            }

                            /* we need to propagate the const to this src */
                            if (allChannelConst)
                            {
                                _VSC_CPF_PropagateConst(pCPF, pInst, dstEnable, texldParm, &vecVal[0]);
                            }
                        }
                    }
                }

                if (_VSC_CPF_GetVRegNo(pInst, srcOpnd) == VIR_INVALID_ID)
                {
                    continue;
                }

                srcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);

                if (dstOpnd == gcvNULL ||
                    _VSC_CPF_GetVRegNo(pInst, dstOpnd) == VIR_INVALID_ID ||
                    !VIR_OPCODE_isComponentwise(opcode))
                {
                    dstEnable = VIR_Swizzle_2_Enable(srcSwizzle);
                }

                gcmASSERT(dstEnable != VIR_ENABLE_NONE);

                if (_VSC_CPF_GetVRegNo(pInst, srcOpnd) != VIR_INVALID_ID)
                {
                    for (channel = 0; channel < VIR_CHANNEL_NUM; channel++ )
                    {
                        if (dstEnable & (1 << channel))
                        {
                            if (!_VSC_CPF_isScalarConst(pCPF, srcBBId, pInst, srcOpnd, channel, tmpFlow,
                                    &vecVal[channel], &srcLattic))
                            {
                                allChannelConst = gcvFALSE;
                                break;
                            }
                        }
                    }

                    /* we need to propagate the const to this src */
                    if (allChannelConst)
                    {
                        _VSC_CPF_PropagateConst(pCPF, pInst, dstEnable, srcOpnd, &vecVal[0]);
                    }
                }
            }
        }
    }

    return errCode;
}

static VSC_ErrCode
_VSC_CPF_AnalysisOnBlock(
    VSC_CPF         *pCPF,
    VIR_BASIC_BLOCK *pBB
    )
{
    VSC_ErrCode errCode    = VSC_ERR_NONE;
    VSC_OPTN_CPFOptions *pOptions  = VSC_CPF_GetOptions(pCPF);

    VSC_CPF_DF_VECTOR  tmpFlow;
    VSC_CPF_DF_VECTOR  *inFlow = gcvNULL;
    VSC_CPF_DF_VECTOR  *outFlow = gcvNULL;

    VSC_CPF_BLOCK_FLOW  *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
            vscSRARR_GetElement(VSC_CPF_GetBlkFlowArray(pCPF),
                                pBB->dgNode.id);

    inFlow = &pBlkFlow->inFlow;
    outFlow = &pBlkFlow->outFlow;

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "before merge predecesors\n");
        _VSC_CPF_BB_DataFlow_Dump(pCPF, pBB);
        VIR_LOG_FLUSH(pDumper);
    }

    /* IN = ^OUT(preds) */
    {
        VIR_BASIC_BLOCK             *pPredBasicBlk;
        VSC_ADJACENT_LIST_ITERATOR  predEdgeIter;
        VIR_CFG_EDGE                *pPredEdge;
        gctBOOL                     firstPred = gcvTRUE;
        gctBOOL                     inChanged = gcvFALSE;

        vscCPF_DV_Initialize(&tmpFlow, VSC_CPF_GetMM(pCPF), vscCPF_DV_BitCount(inFlow));

        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pBB->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL;
            pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            VSC_CPF_BLOCK_FLOW  *predBlkFlow = gcvNULL;

            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);

            predBlkFlow = (VSC_CPF_BLOCK_FLOW*)
                vscSRARR_GetElement(VSC_CPF_GetBlkFlowArray(pCPF),
                                    pPredBasicBlk->dgNode.id);

            if (pPredBasicBlk->flowType == VIR_FLOW_TYPE_ENTRY ||
                BB_GET_END_INST(pPredBasicBlk) == gcvNULL)
            {
                continue;
            }

            if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
                VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
            {
                VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
                VIR_LOG(pDumper, "merge bb\n");
                _VSC_CPF_BB_DataFlow_Dump(pCPF, pPredBasicBlk);
                VIR_LOG_FLUSH(pDumper);
            }

            if (firstPred)
            {
                /* tmpFlow = pred->out */
                _VSC_CPF_CopyFlow(pCPF, pBB->dgNode.id, &tmpFlow, pPredBasicBlk->dgNode.id, &predBlkFlow->outFlow);
                firstPred = gcvFALSE;
            }
            else
            {
                /* tmpFlow = tmpFlow ^ pred->out */
                _VSC_CPF_AndFlow(pCPF, pBB->dgNode.id, &tmpFlow, pPredBasicBlk->dgNode.id, &predBlkFlow->outFlow);
            }

            inChanged = gcvTRUE;
        }

        if (inChanged)
        {
            _VSC_CPF_CopyFlowLattice(inFlow, &tmpFlow);
        }
        else
        {
            _VSC_CPF_CopyFlowLattice(&tmpFlow, inFlow);
        }
    }

    _VSC_CPF_CopyConstKey(pCPF, pBB->dgNode.id);

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "before walk through bb\n");
        _VSC_CPF_BB_DataFlow_Dump(pCPF, pBB);
        VIR_LOG_FLUSH(pDumper);
    }

    /* go through all the instructions to do analysis only */
    {
        VIR_Instruction *inst = BB_GET_START_INST(pBB);
        while(inst)
        {
            _VSC_CPF_PerformOnInst(pCPF, pBB->dgNode.id, inst, &tmpFlow, gcvTRUE /*analysis only*/);

            if (inst == BB_GET_END_INST(pBB))
            {
                break;
            }
            inst = VIR_Inst_GetNext(inst);
        }
    }

    /* according to the block type, add its successors to worklist */
    {
        gctBOOL constCondition = gcvFALSE;
        gctBOOL checkingResult = gcvFALSE;

        if (pBB->flowType == VIR_FLOW_TYPE_ENTRY ||
            !_VSC_CPF_EqualFlowLattice(outFlow, &tmpFlow))
        {

            VIR_BASIC_BLOCK             *psuccBasicBlk;
            VSC_ADJACENT_LIST_ITERATOR  succEdgeIter;
            VIR_CFG_EDGE                *pSuccEdge;

            /* copy the tmpFlow (i.e., new outFlow) to outFlow */
            _VSC_CPF_CopyFlowLattice(outFlow, &tmpFlow);

            if (pBB->flowType == VIR_FLOW_TYPE_JMPC)
            {
                VIR_Instruction *inst = BB_GET_END_INST(pBB);
                gcmASSERT(VIR_Inst_GetOpcode(inst) == VIR_OP_JMPC ||
                          VIR_Inst_GetOpcode(inst) == VIR_OP_JMP_ANY);

                if (VIR_Evaluate_JMPC_Condition(VSC_CPF_GetShader(pCPF),
                        inst, &checkingResult))
                {
                    constCondition = gcvTRUE;
                }
            }

            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pBB->dgNode.succList);
            pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
            for (; pSuccEdge != gcvNULL;
                pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
            {
                psuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);

                /* exit block or empty block */
                if (psuccBasicBlk->flowType != VIR_FLOW_TYPE_EXIT &&
                    BB_GET_END_INST(psuccBasicBlk) != gcvNULL)
                {
                    if (!constCondition ||
                        (checkingResult && pSuccEdge->type == VIR_CFG_EDGE_TYPE_TRUE) ||
                        (!checkingResult && pSuccEdge->type == VIR_CFG_EDGE_TYPE_FALSE)
                       )
                    {
                        if (!_VSC_CPF_InWorkList(pCPF, psuccBasicBlk))
                        {
                            _VSC_CPF_WorkListQueue(pCPF, psuccBasicBlk);
                        }
                    }
                }
            }
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);
        VIR_LOG(pDumper, "after walk through bb\n");
        _VSC_CPF_BB_DataFlow_Dump(pCPF, pBB);
        VIR_LOG_FLUSH(pDumper);
    }

    vscCPF_DV_Finalize(&tmpFlow);

    return errCode;
}

static VSC_ErrCode
_VSC_CPF_TransformOnBlock(
    VSC_CPF         *pCPF,
    VIR_BASIC_BLOCK *pBB
    )
{
    VSC_ErrCode errCode    = VSC_ERR_NONE;

    VSC_CPF_DF_VECTOR  tmpFlow;
    VSC_CPF_DF_VECTOR  *inFlow = gcvNULL;

    VIR_Instruction *inst = gcvNULL;

    VSC_CPF_BLOCK_FLOW  *pBlkFlow = (VSC_CPF_BLOCK_FLOW*)
            vscSRARR_GetElement(VSC_CPF_GetBlkFlowArray(pCPF),
                                pBB->dgNode.id);

    inFlow = &pBlkFlow->inFlow;

    vscCPF_DV_Initialize(&tmpFlow, VSC_CPF_GetMM(pCPF), vscCPF_DV_BitCount(inFlow));
    _VSC_CPF_CopyFlowLattice(&tmpFlow, inFlow);

    _VSC_CPF_CopyConstKey(pCPF, pBB->dgNode.id);

    /* go through all the instructions to
       propagate the constant. call simplification to do folding. */
    inst = BB_GET_START_INST(pBB);
    while(inst)
    {
        _VSC_CPF_PerformOnInst(pCPF, pBB->dgNode.id, inst, &tmpFlow, gcvFALSE);

        if (inst == BB_GET_END_INST(pBB))
        {
            break;
        }
        inst = VIR_Inst_GetNext(inst);
    }

    vscCPF_DV_Finalize(&tmpFlow);

    return errCode;
}

static VSC_ErrCode
_VSC_CPF_PerformOnFunction(
    VSC_CPF         *pCPF,
    VIR_Function    *pFunc
    )
{
    VSC_ErrCode         errCode    = VSC_ERR_NONE;
    VSC_OPTN_CPFOptions *pOptions  = VSC_CPF_GetOptions(pCPF);

    VIR_BASIC_BLOCK     *pBB        = gcvNULL;
    CFG_ITERATOR        cfgIter;

    errCode = _VSC_CPF_InitFunction(pCPF, pFunc);

    /* Two phases algorithm:
       first do global constant data flow analysis;
       second propagate the constant and call simplication to do
       constant folding.

       Because of back edge, we have to seperate the analysis with
       the transformation phases.
       */

    /* constant data flow analysis:
       only change the data flow state */
    while(!QUEUE_CHECK_EMPTY(VSC_CPF_GetWorkList(pCPF)))
    {
        _VSC_CPF_WorkListDequeue(pCPF, &pBB);

        if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
            VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
        {
            VIR_Dumper *pDumper = VSC_CPF_GetDumper(pCPF);

            VIR_LOG(pDumper, "Analyze BB[%d]", pBB->dgNode.id);
            VIR_LOG_FLUSH(pDumper);
        }

        _VSC_CPF_AnalysisOnBlock(pCPF, pBB);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_ALGORITHM))
    {
        _VSC_CPF_CFG_DataFlow_Dump(pCPF, &pFunc->pFuncBlock->cfg);
    }

    /* constant propagate and fold */
    CFG_ITERATOR_INIT(&cfgIter, &pFunc->pFuncBlock->cfg);
    for (pBB = CFG_ITERATOR_FIRST(&cfgIter); pBB != gcvNULL;
         pBB = CFG_ITERATOR_NEXT(&cfgIter))
    {
        _VSC_CPF_TransformOnBlock(pCPF, pBB);
    }

    _VSC_CPF_FinalizeFunction(pCPF, pFunc);

    return errCode;
}

static VSC_ErrCode
_VSC_CPF_PerformOnShader(
    IN OUT VSC_CPF *pCPF
    )
{
    VSC_ErrCode         errcode    = VSC_ERR_NONE;
    VIR_Shader          *pShader     = VSC_CPF_GetShader(pCPF);
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node  = gcvNULL;
    VSC_OPTN_CPFOptions *pOptions    = VSC_CPF_GetOptions(pCPF);

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Constant Propagation and Folding", pShader, gcvTRUE);
    }

    /* to-do: go through the call chain for the inter-procedure */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;
        _VSC_CPF_PerformOnFunction(pCPF, func);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CPFOptions_GetTrace(pOptions),
        VSC_OPTN_CPFOptions_TRACE_OUTPUT) ||
        gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after Constant Propagation and Folding", pShader, gcvTRUE);
    }

    return errcode;
}

/* Interface function to be called by drvi */
VSC_ErrCode
VSC_CPF_PerformOnShader(
    IN VIR_Shader           *shader,
    IN VSC_OPTN_CPFOptions  *options,
    IN VIR_Dumper           *dumper
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_CPF cpf;

    if (!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_CPFOptions_GetBeforeShader(options),
                          VSC_OPTN_CPFOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_CPFOptions_GetTrace(options))
        {
            VIR_Dumper* pDumper = shader->dumper;
            VIR_LOG(pDumper, "Constant Propagation and Folding skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(pDumper);
        }
        return errCode;
    }

    /* too large shader, don't do CPF */
    if (VIR_Shader_GetVirRegCount(shader) > VSC_CPF_MAX_TEMP)
    {
        if(VSC_OPTN_CPFOptions_GetTrace(options))
        {
            VIR_Dumper* pDumper = shader->dumper;
            VIR_LOG(pDumper, "Constant Propagation and Folding skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(pDumper);
        }
        return errCode;
    }

    _VSC_CPF_Init(&cpf, shader, options, dumper);
    errCode = _VSC_CPF_PerformOnShader(&cpf);
    _VSC_CPF_Final(&cpf);

    return errCode;
}

