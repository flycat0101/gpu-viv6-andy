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


#include "vir/transform/gc_vsc_vir_cse.h"

typedef struct VSC_LCSE_KEY
{
    VSC_UNI_LIST_NODE   node;
    VIR_Instruction*    inst;
    VIR_Instruction*    commonEval;
    VIR_Instruction*    firstEval;
} VSC_LCSE_Key;

#define VSC_LCSE_Key_GetInst(k)             ((k)->inst)
#define VSC_LCSE_Key_SetInst(k, i)          ((k)->inst = (i))
#define VSC_LCSE_Key_GetCommonEval(k)       ((k)->commonEval)
#define VSC_LCSE_Key_SetCommonEval(k, c)    ((k)->commonEval = (c))
#define VSC_LCSE_Key_GetFirstEval(k)        ((k)->firstEval)
#define VSC_LCSE_Key_SetFirstEval(k, f)     ((k)->firstEval = (f))

static void _VSC_LCSE_Key_Init(
    IN OUT VSC_LCSE_Key* key,
    IN VIR_Instruction* inst,
    IN VIR_Instruction* commonEval,
    IN VIR_Instruction* firstEval
    )
{
    VSC_LCSE_Key_SetInst(key, inst);
    VSC_LCSE_Key_SetCommonEval(key, commonEval);
    VSC_LCSE_Key_SetFirstEval(key, firstEval);
}

/*Expression mapping related functions. */
typedef struct VSC_LCSE_EXPMAP VSC_LCSE_ExpMap;

typedef struct VSC_LCSE
{
    VIR_Shader*             shader;
    VIR_Function*           currFunc;
    VSC_LCSE_ExpMap*        pFunctionExpMap;
    VIR_BASIC_BLOCK*        currBB;
    VSC_OPTN_LCSEOptions*   options;
    VIR_Dumper*             dumper;
    VSC_MM*                 pMM;
    VIR_DEF_USAGE_INFO      *duInfo;
} VSC_LCSE;

#define VSC_LCSE_GetShader(l)           ((l)->shader)
#define VSC_LCSE_SetShader(l, s)        ((l)->shader = (s))
#define VSC_LCSE_GetCurrFunc(l)         ((l)->currFunc)
#define VSC_LCSE_SetCurrFunc(l, f)      ((l)->currFunc = (f))
#define VSC_LCSE_GetFuncExpMap(l)       ((l)->pFunctionExpMap)
#define VSC_LCSE_SetFuncExpMap(l, b)    ((l)->pFunctionExpMap = (b))
#define VSC_LCSE_GetCurrBB(l)           ((l)->currBB)
#define VSC_LCSE_SetCurrBB(l, b)        ((l)->currBB = (b))
#define VSC_LCSE_GetOptions(l)          ((l)->options)
#define VSC_LCSE_SetOptions(l, o)       ((l)->options = (o))
#define VSC_LCSE_GetDumper(l)           ((l)->dumper)
#define VSC_LCSE_SetDumper(l, d)        ((l)->dumper = (d))
#define VSC_LCSE_GetDuInfo(l)           ((l)->duInfo)
#define VSC_LCSE_SetDuInfo(l, d)        ((l)->duInfo = (d))

static void _VSC_LCSE_Init(
    IN OUT VSC_LCSE*            lcse,
    IN VIR_Shader*              shader,
    IN VIR_Function*            currFunc,
    IN VIR_BASIC_BLOCK*         currBB,
    IN VSC_OPTN_LCSEOptions*    options,
    IN VIR_Dumper*              dumper,
    IN VSC_MM*                  pMM,
    IN VIR_DEF_USAGE_INFO       *duInfo
    )
{
    VSC_LCSE_SetShader(lcse, shader);
    VSC_LCSE_SetCurrFunc(lcse, currFunc);
    VSC_LCSE_SetCurrBB(lcse, currBB);
    VSC_LCSE_SetOptions(lcse, options);
    VSC_LCSE_SetDumper(lcse, dumper);
    VSC_LCSE_SetDuInfo(lcse, duInfo);
    lcse->pMM = pMM;
}

struct VSC_LCSE_EXPMAP
{
    VSC_LCSE*               lcse;
    VSC_UNI_LIST            keyList;
    VSC_HASH_TABLE*         pWorkingInstSet;
    VSC_MM*                 pMM;
    gctBOOL                 bVectorizeAttrLd;
};

#define VSC_LCSE_ExpMap_GetLCSE(em)                 ((em)->lcse)
#define VSC_LCSE_ExpMap_SetLCSE(em, l)              ((em)->lcse = (l))
#define VSC_LCSE_ExpMap_GetShader(em)               ((em)->lcse->shader)
#define VSC_LCSE_ExpMap_GetOptions(em)              ((em)->lcse->options)
#define VSC_LCSE_ExpMap_GetDumper(em)               ((em)->lcse->dumper)
#define VSC_LCSE_ExpMap_GetKeyList(em)              (&(em)->keyList)
#define VSC_LCSE_ExpMap_GetWorkingInstSet(em)       ((em)->pWorkingInstSet)
#define VSC_LCSE_ExpMap_SetWorkingInstSet(em, l)    ((em)->pWorkingInstSet = (l))
#define VSC_LCSE_ExpMap_GetMM(em)                   ((em)->lcse->pMM)
#define VSC_LCSE_ExpMap_GetVectorizeAttrLd(em)      ((em)->bVectorizeAttrLd)
#define VSC_LCSE_ExpMap_SetVectorizeAttrLd(em, l)   ((em)->bVectorizeAttrLd = (l))

static VSC_ErrCode _VSC_LCSE_ExpMap_Init(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VSC_LCSE*                lcse,
    IN gctBOOL                  bCreateWorkingInstSet
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VSC_HASH_TABLE*             pHT = gcvNULL;

    VSC_LCSE_ExpMap_SetLCSE(expMap, lcse);
    vscUNILST_Initialize(VSC_LCSE_ExpMap_GetKeyList(expMap), gcvFALSE);
    expMap->pMM = lcse->pMM;

    VSC_LCSE_ExpMap_SetVectorizeAttrLd(expMap, gcvFALSE);

    if (bCreateWorkingInstSet)
    {
        pHT = vscHTBL_Create(lcse->pMM, vscHFUNC_Default, vscHKCMP_Default, 8);
        if(pHT == gcvNULL)
            return VSC_ERR_OUT_OF_MEMORY;
    }

    VSC_LCSE_ExpMap_SetWorkingInstSet(expMap, pHT);

    return errCode;
}

static void _VSC_LCSE_ExpMap_Final(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN gctBOOL                  bResetHashTableOnly
    )
{
    VSC_HASH_TABLE*             pHT = VSC_LCSE_ExpMap_GetWorkingInstSet(expMap);

    vscUNILST_Finalize(VSC_LCSE_ExpMap_GetKeyList(expMap));
    VSC_LCSE_ExpMap_SetVectorizeAttrLd(expMap, gcvFALSE);

    if (pHT != gcvNULL)
    {
        if (bResetHashTableOnly)
        {
            vscHTBL_Reset(pHT);
        }
        else
        {
            vscHTBL_Destroy(pHT);
            VSC_LCSE_ExpMap_SetWorkingInstSet(expMap, gcvNULL);
        }
    }
}

static VSC_LCSE_Key* _VSC_LCSE_ExpMap_NewKey(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*         inst,
    IN VIR_Instruction*         commonEval,
    IN VIR_Instruction*         firstEval
    )
{
    VSC_LCSE_Key* newKey = (VSC_LCSE_Key*)vscMM_Alloc(VSC_LCSE_ExpMap_GetMM(expMap), sizeof(VSC_LCSE_Key));
    if(!newKey)
        return newKey;
    _VSC_LCSE_Key_Init(newKey, inst, commonEval, firstEval);

    return newKey;
}

static VSC_LCSE_Key* _VSC_LCSE_ExpMap_FindSameExpKey(
    IN VSC_LCSE_ExpMap*         expMap,
    IN VIR_Instruction*         inst
    )
{
    VSC_UNI_LIST*               keyList = VSC_LCSE_ExpMap_GetKeyList(expMap);
    gctBOOL                     bVectorizeAttrLd = VSC_LCSE_ExpMap_GetVectorizeAttrLd(expMap);
    gctBOOL                     bFound = gcvFALSE;
    VSC_UL_ITERATOR             iter;
    VSC_LCSE_Key*               key;
    VSC_LCSE_Key*               pMatchKey = gcvNULL;
    VIR_Operand*                pInstSrc0 = VIR_Inst_GetSource(inst, 0);
    VIR_Swizzle                 origInstSrc0Swizzle = VIR_Operand_GetSwizzle(pInstSrc0);
    VIR_Swizzle                 defaultSwizzle = VIR_SWIZZLE_XYZW;

    /* If we are vectorizing ATTR_LD, then we don't need to care about the swizzle of source0. */
    if (bVectorizeAttrLd)
    {
        VIR_Operand_SetSwizzle(pInstSrc0, defaultSwizzle);
    }

    vscULIterator_Init(&iter, keyList);
    for (key = (VSC_LCSE_Key*)vscULIterator_First(&iter);
         key != gcvNULL;
         key = (VSC_LCSE_Key*)vscULIterator_Next(&iter))
    {
        VIR_Instruction*        keyInst = VSC_LCSE_Key_GetInst(key);
        VIR_OpCode              keyOpcode = VIR_Inst_GetOpcode(keyInst);
        VIR_Operand*            pKeyInstSrc0 = VIR_Inst_GetSource(keyInst, 0);
        VIR_Swizzle             origKeyInstSrc0Swizzle = VIR_Operand_GetSwizzle(pKeyInstSrc0);

        if (bVectorizeAttrLd)
        {
            VIR_Operand_SetSwizzle(pKeyInstSrc0, defaultSwizzle);
        }

        if (VIR_Inst_IdenticalExpression(keyInst, inst, VSC_LCSE_ExpMap_GetShader(expMap), gcvTRUE, gcvTRUE, gcvFALSE))
        {
            /* load r1.xy, base, offset
               load r1.yz, base, offset
               the two loads are loading the same data to xy and yz */
            if (VIR_OPCODE_isMemLd(keyOpcode))
            {
                if ((VIR_Enable_Channel_Count(VIR_Inst_GetEnable(keyInst)) == 1 &&
                     VIR_Enable_Channel_Count(VIR_Inst_GetEnable(inst))    == 1)
                    ||
                    VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst))
                {
                    pMatchKey = key;
                    bFound = gcvTRUE;
                }
            }
            /* load_attr r1.x, base, regmap_index, attr_index
               load_attr r1.y, base, regmap_index, attr_index
               the two load_attrs are loading x and y components */
            else if (VIR_OPCODE_isAttrLd(VIR_Inst_GetOpcode(keyInst)))
            {
                if (VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst)
                    ||
                    /* We don't need to check enable here. */
                    bVectorizeAttrLd)
                {
                    pMatchKey = key;
                    bFound = gcvTRUE;
                }
            }
            else
            {
                if (VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst))
                {
                    pMatchKey = key;
                    bFound = gcvTRUE;
                }
            }
        }

        /* Reset the swizzle. */
        if (bVectorizeAttrLd)
        {
            VIR_Operand_SetSwizzle(pKeyInstSrc0, origKeyInstSrc0Swizzle);
        }

        /* Find a matched key. */
        if (bFound)
        {
            break;
        }
    }

    /* Reset the swizzle. */
    if (bVectorizeAttrLd)
    {
        VIR_Operand_SetSwizzle(pInstSrc0, origInstSrc0Swizzle);
    }

    if (bFound)
    {
        gcmASSERT(pMatchKey != gcvNULL);

        return pMatchKey;
    }

    return gcvNULL;
}

static void _VSC_LCSE_ExpMap_InsertKey(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VSC_LCSE_Key*            key
    )
{
    vscUNILST_Append(VSC_LCSE_ExpMap_GetKeyList(expMap), (VSC_UNI_LIST_NODE*)key);
}

static void _VSC_LCSE_ExpMap_RemoveKey(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VSC_LCSE_Key*            key
    )
{
    vscUNILST_Remove(VSC_LCSE_ExpMap_GetKeyList(expMap), (VSC_UNI_LIST_NODE*)key);
}

static gctBOOL _VSC_LCSE_ExpMap_FindExp(
    IN VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*     inst,
    OUT VIR_Instruction**   commonEval,
    OUT VIR_Instruction**   firstEval
    )
{
    VSC_LCSE_Key* key = _VSC_LCSE_ExpMap_FindSameExpKey(expMap, inst);

    gcmASSERT(commonEval && firstEval);
    if(key)
    {
        *commonEval = VSC_LCSE_Key_GetCommonEval(key);
        *firstEval = VSC_LCSE_Key_GetFirstEval(key);
        return gcvTRUE;
    }
    return gcvFALSE;
}

static VSC_ErrCode _VSC_LCSE_ExpMap_Gen(
    IN VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*     inst
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VSC_OPTN_LCSEOptions*   options = VSC_LCSE_ExpMap_GetOptions(expMap);
    VIR_Dumper*             dumper = VSC_LCSE_ExpMap_GetDumper(expMap);
    gctBOOL                 bVectorizeAttrLd = VSC_LCSE_ExpMap_GetVectorizeAttrLd(expMap);
    VSC_HASH_TABLE*         pWorkingInstSet = VSC_LCSE_ExpMap_GetWorkingInstSet(expMap);
    VSC_LCSE_Key*           key;

    /* generated new expression */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_GENERATING))
    {
        VIR_LOG(dumper, "Generated new expression:\n");
        VIR_Inst_Dump(dumper, inst);
    }

    key = _VSC_LCSE_ExpMap_NewKey(expMap, inst, gcvNULL, inst);
    if(key == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    _VSC_LCSE_ExpMap_InsertKey(expMap, key);

    if (bVectorizeAttrLd)
    {
        gcmASSERT(pWorkingInstSet != gcvNULL);

        /* Insert a NULL key value here. */
        CHECK_ERROR0(vscHTBL_DirectSet(pWorkingInstSet, (void *)inst, (void *)gcvNULL));
    }
    return errCode;
}

static void _VSC_LCSE_ExpMap_Update(
    IN VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*     inst,
    IN VIR_Instruction*     commonEval
    )
{
    VSC_LCSE_Key* key = _VSC_LCSE_ExpMap_FindSameExpKey(expMap, inst);
    VSC_LCSE_Key_SetInst(key, commonEval);
    VSC_LCSE_Key_SetCommonEval(key, commonEval);
}

static gctBOOL _VIR_LCSE_isDefOutput(
    IN VIR_Instruction*     pInst)
{
    VIR_Symbol *sym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetDest(pInst));

    if (sym &&
        (VIR_Symbol_isOutput(sym) || VIR_Symbol_isPerPatchOutput(sym)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static void _VSC_LCSE_ExpMap_MultiKill(
    IN VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*     killingInst
    )
{
    VIR_Shader*             pShader = VSC_LCSE_ExpMap_GetShader(expMap);
    VSC_OPTN_LCSEOptions*   options = VSC_LCSE_ExpMap_GetOptions(expMap);
    VIR_Dumper*             dumper = VSC_LCSE_ExpMap_GetDumper(expMap);
    VSC_UNI_LIST*           keyList = VSC_LCSE_ExpMap_GetKeyList(expMap);
    VSC_UL_ITERATOR         iter;
    VSC_LCSE_Key*           key;
    VIR_OpCode              killOpcode = VIR_Inst_GetOpcode(killingInst);

    gcmASSERT(VIR_OPCODE_hasDest(killOpcode) ||
              VIR_OPCODE_isMemSt(killOpcode) ||
              VIR_OPCODE_isAttrSt(killOpcode) ||
              killOpcode == VIR_OP_EMIT0 ||
              killOpcode == VIR_OP_EMIT);

    vscULIterator_Init(&iter, keyList);
    for(key = (VSC_LCSE_Key*)vscULIterator_First(&iter);
        key != gcvNULL; key = (VSC_LCSE_Key*)vscULIterator_Next(&iter))
    {
        VIR_Instruction*    keyInst = VSC_LCSE_Key_GetInst(key);
        VIR_OpCode          keyOpcode = VIR_Inst_GetOpcode(keyInst);
        gctUINT             i;
        gctBOOL             kills = gcvFALSE;

        if (killOpcode == VIR_OP_EMIT0 ||
            killOpcode == VIR_OP_EMIT)
        {
            /* output destination is killed */
            if (VIR_Inst_GetOpcode(keyInst) == VIR_OP_ATTR_LD ||
                _VIR_LCSE_isDefOutput(keyInst))
            {
                kills = gcvTRUE;
            }
        }
        else
        {
            if (VIR_OPCODE_isMemSt(killOpcode) && VIR_OPCODE_isMemLd(keyOpcode))
            {
                if((VIR_Operand_isSymbol(VIR_Inst_GetSource(killingInst, 0)) && !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(killingInst, 0)))) ||
                   (VIR_Operand_isSymbol(VIR_Inst_GetSource(keyInst, 0)) && !VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(keyInst, 0)))))
                {
                    kills = gcvTRUE;
                }
                else if(!VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 0), VIR_Inst_GetSource(keyInst, 0), VSC_LCSE_ExpMap_GetShader(expMap), gcvFALSE))
                {
                    /* different base means no overlap */
                    kills = gcvFALSE;
                }
                else if(VIR_Operand_isSymbol(VIR_Inst_GetSource(killingInst, 1)) ||
                        VIR_Operand_isSymbol(VIR_Inst_GetSource(keyInst, 1)))
                {
                    /* same base, symbol offset. we have to assume they overlaps */
                    kills = gcvTRUE;
                }
                else
                {
                    /* same base, constant offset. see whether the constant values are the same */
                    kills = VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 1), VIR_Inst_GetSource(keyInst, 1), VSC_LCSE_ExpMap_GetShader(expMap), gcvFALSE);
                }
            }
            /* A ATTR_ST only affects a ATTR_LD instruction. */
            else if (VIR_OPCODE_isAttrSt(killOpcode) && VIR_OPCODE_isAttrLd(keyOpcode))
            {
                VIR_Operand*    pStOutputOpnd = VIR_Inst_GetDest(killingInst);
                VIR_Operand*    pStInvocationIndexOpnd = VIR_Inst_GetSource(killingInst, 0);
                VIR_Operand*    pStOffsetOpnd = VIR_Inst_GetSource(killingInst, 1);
                VIR_Operand*    pLdOutputOpnd = VIR_Inst_GetSource(keyInst, 0);
                VIR_Operand*    pLdInvocationIndexOpnd = VIR_Inst_GetSource(keyInst, 1);
                VIR_Operand*    pLdOffsetOpnd = VIR_Inst_GetSource(keyInst, 2);

                /* Do not kill this key only if we can guarantee that these ATTR_ST and ATTR_LD are processing two different memory. */
                /* Different output variables, no need to kill. */
                if (!VIR_Operand_Defines(pStOutputOpnd, pLdOutputOpnd))
                {
                    continue;
                }

                if ((VIR_Operand_isImm(pStInvocationIndexOpnd) || VIR_Operand_isConst(pStInvocationIndexOpnd))
                    &&
                    (VIR_Operand_isImm(pStOffsetOpnd) || VIR_Operand_isConst(pStOffsetOpnd))
                    &&
                    (VIR_Operand_isImm(pLdInvocationIndexOpnd) || VIR_Operand_isConst(pLdInvocationIndexOpnd))
                    &&
                    (VIR_Operand_isImm(pLdOffsetOpnd) || VIR_Operand_isConst(pLdOffsetOpnd))
                    &&
                    VIR_Operand_Identical(pStInvocationIndexOpnd, pLdInvocationIndexOpnd, pShader, gcvFALSE)
                    &&
                    VIR_Operand_Identical(pStOffsetOpnd, pLdOffsetOpnd, pShader, gcvFALSE))
                {
                    continue;
                }

                kills = gcvTRUE;
            }
            else
            {
                for(i = 0; i < VIR_Inst_GetSrcNum(keyInst); i++)
                {
                    if(VIR_Operand_Defines(VIR_Inst_GetDest(killingInst), VIR_Inst_GetSource(keyInst, i)))
                    {
                        kills = gcvTRUE;
                        break;;
                    }
                }

                /* since we are not generating the new instruction,
                   we need to check redef of keyInst's def */
                if (keyInst != killingInst &&
                    VIR_Operand_Defines(VIR_Inst_GetDest(killingInst), VIR_Inst_GetDest(keyInst)))
                {
                    kills = gcvTRUE;
                }
            }
        }

        if(kills)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_KILLING))
            {
                VIR_LOG(dumper, "Expression:\n");
                VIR_Inst_Dump(dumper, keyInst);
                VIR_LOG(dumper, "is killed by:\n");
                VIR_Inst_Dump(dumper, killingInst);
            }
            _VSC_LCSE_ExpMap_RemoveKey(expMap, key);
        }
    }
}

/* Replace the usage instruction. */
static VSC_ErrCode _VSC_LCSE_ReplaceUses(
    IN OUT VSC_LCSE             *lcse,
    IN VIR_Instruction          *replacedInst,
    IN VIR_Instruction          *commonInst
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_LCSEOptions* options = VSC_LCSE_GetOptions(lcse);
    VIR_Dumper* dumper = VSC_LCSE_GetDumper(lcse);
    VIR_BASIC_BLOCK* bb = VSC_LCSE_GetCurrBB(lcse);
    VIR_Instruction *instIter;
    gctBOOL    findInst = gcvFALSE;

    gcmASSERT(BB_GET_LENGTH(bb) != 0);


    if (VIR_Operand_GetModifier(VIR_Inst_GetDest(replacedInst)))
    {
        return errCode;
    }

    for(instIter = BB_GET_START_INST(bb); instIter != BB_GET_END_INST(bb); instIter = VIR_Inst_GetNext(instIter))
    {
        if (!findInst)
        {
            if (instIter != replacedInst)
            {
                continue;
            }
            else
            {
                findInst = gcvTRUE;
            }
        }
        else
        {
            gctUINT    i, channel;
            VIR_Operand *newDstOpnd, *srcOpnd;
            VIR_OperandInfo srcInfo;

            VIR_Function_DupOperand(VSC_LCSE_GetCurrFunc(lcse), VIR_Inst_GetDest(replacedInst), &newDstOpnd);
            VIR_Operand_Change2Src(newDstOpnd);

            /* use of dest of replacedInst*/
            for(i = 0; i < VIR_Inst_GetSrcNum(instIter); i++)
            {
                srcOpnd = VIR_Inst_GetSource(instIter, i);

                if (VIR_Operand_GetLShift(srcOpnd) > 0 ||
                    VIR_Operand_GetModifier(srcOpnd) != VIR_MOD_NONE)
                {
                    continue;
                }

                VIR_Operand_GetOperandInfo(instIter, srcOpnd, &srcInfo);

                if (VIR_Operand_Identical(srcOpnd, newDstOpnd, VSC_LCSE_GetShader(lcse), gcvFALSE))
                {
                    VIR_TypeId srcOpndTypeId = VIR_Operand_GetTypeId(srcOpnd);

                    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
                    {
                        VIR_LOG(dumper, "change the use instruction:\n");
                        VIR_Inst_Dump(dumper, instIter);
                    }

                    if (srcInfo.isVreg)
                    {
                        vscVIR_DeleteUsage(VSC_LCSE_GetDuInfo(lcse),
                                           VIR_ANY_DEF_INST,
                                           instIter,
                                           srcOpnd,
                                           gcvFALSE,
                                           srcInfo.u1.virRegInfo.virReg,
                                           1,
                                           VIR_Operand_GetEnable(VIR_Inst_GetDest(replacedInst)),
                                           VIR_HALF_CHANNEL_MASK_FULL,
                                           gcvNULL);
                    }

                    VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(replacedInst, 0));
                    VIR_Operand_SetTypeId(srcOpnd, srcOpndTypeId);

                    VIR_Operand_GetOperandInfo(instIter, srcOpnd, &srcInfo);

                    if (srcInfo.isVreg)
                    {
                        for (channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
                        {
                            errCode = vscVIR_AddNewUsageToDef(VSC_LCSE_GetDuInfo(lcse),
                                                              commonInst,
                                                              instIter,
                                                              srcOpnd,
                                                              gcvFALSE,
                                                              srcInfo.u1.virRegInfo.virReg,
                                                              1,
                                                              (1 << channel),
                                                              VIR_HALF_CHANNEL_MASK_FULL,
                                                              gcvNULL);
                            ON_ERROR(errCode, "Failed to add new usage to def.");
                        }
                    }

                    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
                    {
                        VIR_LOG(dumper, "to:\n");
                        VIR_Inst_Dump(dumper, instIter);
                        VIR_LOG(dumper, "\n");
                    }
                }
            }

            VIR_Function_FreeOperand(VSC_LCSE_GetCurrFunc(lcse), newDstOpnd);

            /* redefine of any part of dest of replacedInst */
            if (VIR_Operand_SameLocation(instIter, VIR_Inst_GetDest(instIter), replacedInst, VIR_Inst_GetDest(replacedInst)) ||
                VIR_Operand_SameLocation(instIter, VIR_Inst_GetDest(instIter), replacedInst, VIR_Inst_GetSource(replacedInst, 0)))
            {
                break;
            }
        }
    }
OnError:
    return errCode;
}

static VSC_ErrCode _VSC_LCSE_ReplaceInst(
    IN OUT VSC_LCSE*            lcse,
    IN VSC_LCSE_ExpMap*         expMap,
    IN VIR_Instruction*         commonEval,
    IN VIR_Instruction*         toBeReplaced
    )
{
    VSC_ErrCode                 errCode;
    VIR_Shader*                 pShader = VSC_LCSE_GetShader(lcse);
    VIR_DEF_USAGE_INFO*         pDuInfo = VSC_LCSE_GetDuInfo(lcse);
    VIR_Instruction*            pReplacedInst = commonEval;
    gctUINT                     srcIdx, channel;
    VIR_Operand *               srcOpnd;
    VIR_OperandInfo             srcInfo, dstInfo;
    VIR_Swizzle                 srcSwizzle;
    gctBOOL                     bVectorizeAttrLd = VSC_LCSE_ExpMap_GetVectorizeAttrLd(expMap);
    VSC_HASH_TABLE*             pWorkingInstSet = VSC_LCSE_ExpMap_GetWorkingInstSet(expMap);
    VIR_Instruction*            pReplacedInstInHt = gcvNULL;

    if (bVectorizeAttrLd)
    {
        gcmASSERT(pWorkingInstSet);

        if (vscHTBL_DirectTestAndGet(pWorkingInstSet, (void *)commonEval, (void **)&pReplacedInstInHt))
        {
            /* First time to process this instruction, insert a new ATTR_LD first. */
            if (pReplacedInstInHt == gcvNULL)
            {
                VIR_Symbol*     pBaseSym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetSource(commonEval, 0));
                VIR_SymId       pReplacedSymId = VIR_INVALID_ID;
                VIR_Type*       pSymType;
                VIR_VirRegId    virRegId = VIR_Shader_NewVirRegId(pShader, 1);
                VIR_Function*   pWorkingFunc = VIR_Inst_GetFunction(commonEval);
                VIR_Instruction*pNewAttrLdInst = gcvNULL;
                VIR_Operand*    pNewOpnd = gcvNULL;
                VIR_Enable      newEnable;

                /* Get the base type. */
                pSymType = VIR_Symbol_GetType(pBaseSym);
                while (VIR_Type_isArray(pSymType))
                {
                    pSymType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetIndex(pSymType));
                }

                /* Add a new temp register to save the full components of this variable. */
                errCode = VIR_Shader_AddSymbol(pShader,
                                               VIR_SYM_VIRREG,
                                               virRegId,
                                               pSymType,
                                               VIR_STORAGE_UNKNOWN,
                                               &pReplacedSymId);
                ON_ERROR(errCode, "Fail to add a new vir reg symbol.");

                /* Insert a new ATTR_LD. */
                errCode = VIR_Function_AddCopiedInstructionBefore(pWorkingFunc,
                                                                  commonEval,
                                                                  commonEval,
                                                                  gcvTRUE,
                                                                  &pNewAttrLdInst);
                ON_ERROR(errCode, "Fail to insert a copied instruction.");

                /* Set the DEST. */
                pNewOpnd = VIR_Inst_GetDest(pNewAttrLdInst);
                VIR_Operand_SetSymbol(pNewOpnd, pWorkingFunc, pReplacedSymId);
                newEnable = VIR_Type_Conv2Enable(pSymType);
                VIR_Operand_SetEnable(pNewOpnd, newEnable);

                /* Update the swizzle of SRC0. */
                pNewOpnd = VIR_Inst_GetSource(pNewAttrLdInst, 0);
                VIR_Operand_SetSwizzle(pNewOpnd, VIR_Enable_2_Swizzle_WShift(newEnable));

                /* Update the DU. */
                errCode = vscVIR_AddNewDef(pDuInfo,
                                           pNewAttrLdInst,
                                           virRegId,
                                           1,
                                           newEnable,
                                           VIR_HALF_CHANNEL_MASK_FULL,
                                           gcvNULL,
                                           gcvNULL);
                ON_ERROR(errCode, "Failed to add new def.");

                for (srcIdx = 0; srcIdx < VIR_Inst_GetSrcNum(pNewAttrLdInst); srcIdx++)
                {
                    srcOpnd = VIR_Inst_GetSource(pNewAttrLdInst, srcIdx);
                    srcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);
                    VIR_Operand_GetOperandInfo(pNewAttrLdInst, srcOpnd, &srcInfo);

                    if (!srcInfo.isVreg)
                    {
                        continue;
                    }

                    errCode = vscVIR_AddNewUsageToDef(pDuInfo,
                                                      VIR_ANY_DEF_INST,
                                                      pNewAttrLdInst,
                                                      srcOpnd,
                                                      gcvFALSE,
                                                      srcInfo.u1.virRegInfo.virReg,
                                                      1,
                                                      VIR_Swizzle_2_Enable(srcSwizzle),
                                                      VIR_HALF_CHANNEL_MASK_FULL,
                                                      gcvNULL);
                    ON_ERROR(errCode, "Failed to add new usage to def.");
                }

                /* Set the hash key. */
                CHECK_ERROR0(vscHTBL_DirectSet(pWorkingInstSet, (void *)commonEval, (void *)pNewAttrLdInst));
                pReplacedInstInHt = pNewAttrLdInst;

                /* Replace the commonEval first. */
                _VSC_LCSE_ReplaceInst(lcse, expMap, pReplacedInstInHt, commonEval);
            }

            pReplacedInst = pReplacedInstInHt;
        }
    }

    do
    {
        VSC_OPTN_LCSEOptions*   options = VSC_LCSE_GetOptions(lcse);
        VIR_Dumper*             dumper = VSC_LCSE_GetDumper(lcse);
        VIR_Operand*            commonInstDest = VIR_Inst_GetDest(pReplacedInst);
        VIR_Operand*            toBeReplacedOperand = VIR_Inst_GetSource(toBeReplaced, 0);
        VIR_Swizzle             origSrc0Swizzle = VIR_Operand_GetSwizzle(toBeReplacedOperand);

        VIR_Operand_GetOperandInfo(pReplacedInst, commonInstDest, &dstInfo);

        /* to be replaced instruction */
        if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
        {
            VIR_LOG(dumper, "To be replaced instruction:\n");
            VIR_Inst_Dump(dumper, toBeReplaced);
        }

        /* du update */
        for (srcIdx = 0; srcIdx < VIR_Inst_GetSrcNum(toBeReplaced); srcIdx++)
        {
            srcOpnd = VIR_Inst_GetSource(toBeReplaced, srcIdx);
            srcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);
            VIR_Operand_GetOperandInfo(toBeReplaced, srcOpnd, &srcInfo);

            if (!srcInfo.isVreg)
            {
                continue;
            }

            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               toBeReplaced,
                               srcOpnd,
                               gcvFALSE,
                               srcInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(srcSwizzle),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);
        }

        VIR_Operand_Copy(toBeReplacedOperand, commonInstDest);
        VIR_Operand_Change2Src(toBeReplacedOperand);
        /* clear destination's modifier */
        VIR_Operand_SetModifier(toBeReplacedOperand, VIR_MOD_NONE);
        if (bVectorizeAttrLd)
        {
            VIR_Operand_SetSwizzle(toBeReplacedOperand, origSrc0Swizzle);
        }
        else
        {
            VIR_Operand_SetSwizzle(toBeReplacedOperand, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(commonInstDest)));
        }


        VIR_Inst_SetOpcode(toBeReplaced, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(toBeReplaced, 1);

        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel++)
        {
            if (VIR_Operand_GetEnable(commonInstDest) & (1 << channel))
            {
                errCode = vscVIR_AddNewUsageToDef(pDuInfo,
                                                  pReplacedInst,
                                                  toBeReplaced,
                                                  toBeReplacedOperand,
                                                  gcvFALSE,
                                                  dstInfo.u1.virRegInfo.virReg,
                                                  1,
                                                  (1 << channel),
                                                  VIR_HALF_CHANNEL_MASK_FULL,
                                                  gcvNULL);
                ON_ERROR(errCode, "Failed to add new usage to def.");
            }
        }

        /* replaced instruction */
        if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
        {
            VIR_LOG(dumper, "Replaced instruction:\n");
            VIR_Inst_Dump(dumper, toBeReplaced);
        }

        /* we replace the uses of the def of toBeReplaced to expose more LCSE opportunities */
        errCode = _VSC_LCSE_ReplaceUses(lcse, toBeReplaced, pReplacedInst);
        ON_ERROR(errCode, "Fail to replace the uses.");

    } while(gcvFALSE);

OnError:
    return errCode;
}

static VSC_ErrCode _VSC_LCSE_PerformOnBBForAttrLd(
    IN VSC_LCSE*                pLcse
    )
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VIR_Shader*                 pShader = VSC_LCSE_GetShader(pLcse);
    VIR_BASIC_BLOCK*            pBB = VSC_LCSE_GetCurrBB(pLcse);
    VSC_LCSE_ExpMap*            pFuncExpMap = VSC_LCSE_GetFuncExpMap(pLcse);
    VIR_Instruction*            pInstIter;
    gctBOOL                     bCheckInputOnly = gcvTRUE;
    VSC_HASH_TABLE*             pWorkingInstSet = VSC_LCSE_ExpMap_GetWorkingInstSet(pFuncExpMap);
    gctINT                      maxInstCount = 4;

    VSC_LCSE_ExpMap_SetVectorizeAttrLd(pFuncExpMap, gcvTRUE);

    /*
        Do the following optimization:

            001: ATTR_LD            mp temp(623).mp.x, hp  gl_in.gl_Position.hp.x, int 0, int 0
            002: ATTR_LD            mp temp(627).mp.x, hp  gl_in.gl_Position.hp.y, int 0, int 0
            003: ATTR_LD            mp temp(631).mp.x, hp  gl_in.gl_Position.hp.z, int 0, int 0
            004: ATTR_LD            hp temp(635).hp.x, hp  gl_in.gl_Position.hp.w, int 0, int 0
        -->
            005: ATTR_LD            hp temp(600).hp, hp  gl_in.gl_Position.hp, int 0, int 0
            001: MOV                mp temp(623).mp.x, hp temp(600).hp.x
            002: MOV                mp temp(627).mp.x, hp temp(600).hp.y
            003: MOV                mp temp(631).mp.x, hp temp(600).hp.z
            004: MOV                mp temp(635).mp.x, hp temp(600).hp.w

        There are some rules for this optimization:
            1) So far check ATTR_LD for the attributes only, we can handle the outputs in further by checking the DU.
            2) So far the working range is function scope so we won't use too many temp registers.
            3) Only check those variables with more than one component.
    */

    /* Go through all instructions. */
    for (pInstIter = BB_GET_START_INST(pBB); pInstIter != BB_GET_END_INST(pBB); pInstIter = VIR_Inst_GetNext(pInstIter))
    {
        VIR_Instruction*        pCommonEval = gcvNULL;
        VIR_Instruction*        pFirstEval = gcvNULL;
        VIR_OpCode              instOpcode = VIR_Inst_GetOpcode(pInstIter);
        VIR_Symbol*             pBaseSym = gcvNULL;
        VIR_Type*               pSymType = gcvNULL;

        /* work on ATTR_LD */
        while (instOpcode == VIR_OP_ATTR_LD)
        {
            /* Get the base symbol. */
            pBaseSym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetSource(pInstIter, 0));

            /* Check inputs only. */
            if (bCheckInputOnly
                &&
                !(pBaseSym && (VIR_Symbol_isInput(pBaseSym) || VIR_Symbol_isPerPatchInput(pBaseSym))))
            {
                break;
            }

            /* Get the base type. */
            pSymType = VIR_Symbol_GetType(pBaseSym);
            while (VIR_Type_isArray(pSymType))
            {
                pSymType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetIndex(pSymType));
            }

            /* Check those variables with more than one component. */
            if (VIR_GetTypeComponents(VIR_Type_GetIndex(pSymType)) == 1)
            {
                break;
            }

            if (_VSC_LCSE_ExpMap_FindExp(pFuncExpMap, pInstIter, &pCommonEval, &pFirstEval))
            {
                if (pCommonEval)
                {
                    _VSC_LCSE_ReplaceInst(pLcse, pFuncExpMap, pCommonEval, pInstIter);
                }
                else
                {
                    _VSC_LCSE_ExpMap_Update(pFuncExpMap, pFirstEval, pFirstEval);
                    _VSC_LCSE_ReplaceInst(pLcse, pFuncExpMap, pFirstEval, pInstIter);
                }
            }
            else
            {
                if (vscHTBL_CountItems(pWorkingInstSet) <= maxInstCount)
                {
                    errCode = _VSC_LCSE_ExpMap_Gen(pFuncExpMap, pInstIter);
                    ON_ERROR(errCode, "Fail to gen ExpMap.");
                }
            }

            break;
        };

        /* Kill redefined expression, we don't need to do this if we only check inputs here. */
        if (!bCheckInputOnly
            &&
            (VIR_OPCODE_isAttrSt(instOpcode) ||
             instOpcode == VIR_OP_EMIT0      ||
             instOpcode == VIR_OP_EMIT))
        {
            _VSC_LCSE_ExpMap_MultiKill(pFuncExpMap, pInstIter);
        }
    }

    VSC_LCSE_ExpMap_SetVectorizeAttrLd(pFuncExpMap, gcvFALSE);

    OnError:
    return errCode;
}

static VSC_ErrCode _VSC_LCSE_PerformOnBB(
    IN VSC_LCSE*                lcse
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK* bb = VSC_LCSE_GetCurrBB(lcse);
    VSC_OPTN_LCSEOptions* options = VSC_LCSE_GetOptions(lcse);
    VIR_Dumper* dumper = VSC_LCSE_GetDumper(lcse);
    VSC_LCSE_ExpMap expMap;
    VIR_Instruction* instIter;

    /* dump input bb */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_INPUT_BB))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination starts for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, BB_GET_ID(bb), VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvTRUE);
    }

    _VSC_LCSE_ExpMap_Init(&expMap, lcse, gcvFALSE);
    for(instIter = BB_GET_START_INST(bb); instIter != BB_GET_END_INST(bb); instIter = VIR_Inst_GetNext(instIter))
    {
        VIR_Instruction* commonEval = gcvNULL;
        VIR_Instruction* firstEval = gcvNULL;
        VIR_OpCode       instOpcode = VIR_Inst_GetOpcode(instIter);

        /* work on the expression */
        do
        {
            gctBOOL     handledOp = gcvFALSE;
            switch (instOpcode)
            {
            case VIR_OP_LOAD_L:
            case VIR_OP_LOAD_S:
            case VIR_OP_LOAD:
            case VIR_OP_IMG_LOAD:
            case VIR_OP_IMG_LOAD_3D:
                if (VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetOpts(options), VSC_OPTN_LCSEOptions_OPT_LOAD))
                {
                    handledOp = gcvTRUE;
                }
                break;
            case VIR_OP_ATTR_LD:
                if (VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetOpts(options), VSC_OPTN_LCSEOptions_OPT_ATTR_LD))
                {
                    handledOp = gcvTRUE;
                }
                break;
            case VIR_OP_MOVA:
            case VIR_OP_MUL:
            case VIR_OP_MUL_Z:
            case VIR_OP_NORM_MUL:
            case VIR_OP_FLOOR:
            case VIR_OP_FIX:
            case VIR_OP_CEIL:
            case VIR_OP_FRAC:
            case VIR_OP_RCP:
            case VIR_OP_RSQ:
            case VIR_OP_SQRT:
            case VIR_OP_NORM:
            case VIR_OP_SIN:
            case VIR_OP_COS:
            case VIR_OP_TAN:
            case VIR_OP_ADD:
            case VIR_OP_SUB:
            case VIR_OP_DIV:
            case VIR_OP_MOD:
            case VIR_OP_REM:
            case VIR_OP_MAX:
            case VIR_OP_MIN:
            case VIR_OP_POW:
            case VIR_OP_DP2:
            case VIR_OP_DP3:
            case VIR_OP_DP4:
            case VIR_OP_MAD:
            case VIR_OP_FMA:
            case VIR_OP_LSHIFT:
            case VIR_OP_RSHIFT:
                if (VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetOpts(options), VSC_OPTN_LCSEOptions_OPT_OTHERS))
                {
                    handledOp = gcvTRUE;
                }
                break;
            default:
                break;
            }

            if (!handledOp)
            {
                break;
            }

            if(_VSC_LCSE_ExpMap_FindExp(&expMap, instIter, &commonEval, &firstEval))
            {
                if(commonEval)
                {
                    _VSC_LCSE_ReplaceInst(lcse, &expMap, commonEval, instIter);
                }
                else
                {
                    /* inserted common evaluation instruction */
                    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_INSERTING))
                    {
                        VIR_LOG(dumper, "common expression instruction:\n");
                        VIR_Inst_Dump(dumper, firstEval);
                    }

                    _VSC_LCSE_ExpMap_Update(&expMap, firstEval, firstEval);
                    _VSC_LCSE_ReplaceInst(lcse, &expMap, firstEval, instIter);
                }
            }
            else
            {
                errCode = _VSC_LCSE_ExpMap_Gen(&expMap, instIter);
                ON_ERROR(errCode, "Fail to gen ExpMap.");
            }
        } while(gcvFALSE);

        /* kill redefined expression */
        if(VIR_OPCODE_hasDest(instOpcode) ||
           VIR_OPCODE_isMemSt(instOpcode) ||
           VIR_OPCODE_isAttrSt(instOpcode) ||
           instOpcode == VIR_OP_EMIT0 ||
           instOpcode == VIR_OP_EMIT)
        {
            _VSC_LCSE_ExpMap_MultiKill(&expMap, instIter);
        }
    }
    _VSC_LCSE_ExpMap_Final(&expMap, gcvFALSE);

    /* dump output bb */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_OUTPUT_BB))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination ends for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, BB_GET_ID(bb), VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvTRUE);
    }

    OnError:
    return errCode;
}

static VSC_ErrCode _VSC_LCSE_PerformOnFunction(
    IN VSC_LCSE*                lcse
    )
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VIR_Shader*                 pShader = VSC_LCSE_GetShader(lcse);
    VIR_Function*               func = VSC_LCSE_GetCurrFunc(lcse);
    VSC_OPTN_LCSEOptions*       options = VSC_LCSE_GetOptions(lcse);
    VIR_Dumper*                 dumper = VSC_LCSE_GetDumper(lcse);
    VIR_CONTROL_FLOW_GRAPH*     cfg;
    CFG_ITERATOR                cfg_iter;
    VIR_BASIC_BLOCK*            bb;
    VSC_LCSE_ExpMap             funcExpMap;

    if(VIR_Function_GetInstCount(func) == 0)
    {
        return errCode;
    }

    if(VSC_OPTN_LCSEOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination starts for function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    cfg = VIR_Function_GetCFG(func);

    /* dump input cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_INPUT_CFG))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination: input cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    /* Initialize a expression mapping for the entire function. */
    _VSC_LCSE_ExpMap_Init(&funcExpMap, lcse, gcvTRUE);
    VSC_LCSE_SetFuncExpMap(lcse, &funcExpMap);

    CFG_ITERATOR_INIT(&cfg_iter, cfg);
    for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
    {
        VSC_LCSE_SetCurrBB(lcse, bb);

        if (BB_GET_LENGTH(bb) == 0)
        {
            continue;
        }

        /* Vectorize ATTR_LD first. */
        if (VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetOpts(options), VSC_OPTN_LCSEOptions_OPT_VECTORIZE_ATTR_LD)
            &&
            (VIR_Shader_IsTCS(pShader) || VIR_Shader_IsTES(pShader)))
        {
            errCode = _VSC_LCSE_PerformOnBBForAttrLd(lcse);
            if (errCode)
            {
                return errCode;
            }

            _VSC_LCSE_ExpMap_Final(&funcExpMap, gcvTRUE);
        }

        errCode = _VSC_LCSE_PerformOnBB(lcse);
        if(errCode)
        {
            return errCode;
        }
    }

    _VSC_LCSE_ExpMap_Final(&funcExpMap, gcvFALSE);

    /* dump output cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_OUTPUT_CFG))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination: output cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    if(VSC_OPTN_LCSEOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination ends for function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_LCSE_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_LCSE;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_LCSE_PerformOnShader)
{
    return gcvTRUE;
}

/* local common expression elimination entry function */
VSC_ErrCode VSC_LCSE_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader           *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_OPTN_LCSEOptions  *options = (VSC_OPTN_LCSEOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper           *dumper = pPassWorker->basePassWorker.pDumper;

    if (!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_LCSEOptions_GetBeforeShader(options),
                          VSC_OPTN_LCSEOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_LCSEOptions_GetTrace(options))
        {
            VIR_Dumper* pDumper = shader->dumper;
            VIR_LOG(pDumper, "Local Common Subexpression Elimination skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(pDumper);
        }
        return errCode;
    }

    if(VSC_OPTN_LCSEOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "Local Common Subexpression Elimination starts for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }

    {
        VSC_LCSE lcse;
        VIR_FuncIterator funcIter;
        VIR_FunctionNode* funcNode;
        gctUINT funcIndex;

        _VSC_LCSE_Init(&lcse, shader, gcvNULL, gcvNULL, options, dumper, pPassWorker->basePassWorker.pMM, pPassWorker->pDuInfo);
        VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(shader));
        for(funcNode = VIR_FuncIterator_First(&funcIter), funcIndex = 0;
            funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter), ++funcIndex)
        {
            if(!VSC_OPTN_InRange(funcIndex, VSC_OPTN_LCSEOptions_GetBeforeFunc(options), VSC_OPTN_LCSEOptions_GetAfterFunc(options)))
            {
                if(VSC_OPTN_LCSEOptions_GetTrace(options))
                {
                    VIR_LOG(dumper, "Local Common Subexpression Elimination skips function(%d)\n", funcIndex);
                    VIR_LOG_FLUSH(dumper);
                }
                continue;
            }
            else
            {
                VIR_Function* func = funcNode->function;

                VSC_LCSE_SetCurrFunc(&lcse, func);
                errCode = _VSC_LCSE_PerformOnFunction(&lcse);
                if(errCode)
                {
                    break;
                }
            }
        }
    }

    if(VSC_OPTN_LCSEOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "Local Common Subexpression Elimination ends for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }

    if(VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "LCSE End", shader, gcvTRUE);
    }

    return errCode;
}

/* begin define of CIE */
typedef struct VSC_CIE
{
    VIR_Shader*             shader;
    VIR_Function*           currFunc;
    gctUINT                 candThres; /* if the candlist >= threadhold, run the promotion */
    VIR_Dumper*             dumper;
    gctBOOL                 dumpTrace;
    VSC_MM*                 pMM;
    gctBOOL                 codeChanged;
} VSC_CIE;

#define VSC_CIE_GetShader(l)           ((l)->shader)
#define VSC_CIE_SetShader(l, s)        ((l)->shader = (s))
#define VSC_CIE_GetCurrFunc(l)         ((l)->currFunc)
#define VSC_CIE_SetCurrFunc(l, f)      ((l)->currFunc = (f))
#define VSC_CIE_GetOptions(l)          ((l)->options)
#define VSC_CIE_SetOptions(l, o)       ((l)->options = (o))
#define VSC_CIE_GetDumper(l)           ((l)->dumper)
#define VSC_CIE_SetDumper(l, d)        ((l)->dumper = (d))
#define VSC_CIE_DumpTrace(l)           ((l)->dumpTrace)
#define VSC_CIE_SetDumpTrace(l, b)     ((l)->dumpTrace = (b))
#define VSC_CIE_GetCodeChanged(l)      ((l)->codeChanged)
#define VSC_CIE_SetCodeChanged(l, b)   ((l)->codeChanged = (b))
#define VSC_CIE_SetMm(l, m)            ((l)->pMM = (m))
#define VSC_CIE_GetMm(l)               ((l)->pMM)
#define VSC_CIE_GetThresHold(l)        ((l)->candThres)
#define VSC_CIE_SetThresHold(l, n)     ((l)->candThres = (n))

static void _VSC_CIE_Init(
    IN OUT VSC_CIE*             cie,
    IN VIR_Shader*              shader,
    IN VIR_Function*            currFunc,
    IN VIR_Dumper*              dumper,
    IN VSC_MM*                  pMM,
    IN gctBOOL                  dumpTrace,
    IN gctUINT                  threshold
    )
{
    VSC_CIE_SetShader(cie, shader);
    VSC_CIE_SetDumper(cie, dumper);
    VSC_CIE_SetMm(cie, pMM);
    VSC_CIE_SetCodeChanged(cie, gcvFALSE);
    VSC_CIE_SetDumpTrace(cie, dumpTrace);
    /*4 is the magic value for dEQP-VK.binding_model.shader_access.primary_cmd_buf.uniform_texel_buffer.geometry* cases */
    VSC_CIE_SetThresHold(cie, threshold);
}

static gctBOOL _VSC_CIE_CheckIntrinsicWithConstant(
    IN VIR_Instruction *pInst)
{
    gctUINT n = VIR_Inst_GetSrcNum(pInst);
    VIR_Operand *paramOperand = VIR_Inst_GetSource(pInst, 1);
    VIR_ParmPassing *opndParm = VIR_Operand_GetParameters(paramOperand);
    gctUINT argNum = opndParm->argNum;
    gctUINT i;
    for (i = 0; i < argNum; i++)
    {
        VIR_Operand* pOperand = opndParm->args[i];
        if (!pOperand ||
            !(VIR_Operand_isImm(pOperand) || VIR_Operand_isConst(pOperand) ||
              (VIR_Operand_isSymbol(pOperand) && VIR_Symbol_UseUniform(VIR_Operand_GetSymbol(pOperand)))))
        {
            break;
        }
    }
    return i == n;
}

/*if return -1, not find */
static gctINT _VSC_CIE_FindIntrinsicKey(
    IN OUT VSC_CIE* cie,
    VSC_SIMPLE_RESIZABLE_ARRAY *intrinsicKeys,
    VIR_Instruction* pInst)
{
    gctUINT keyCount = vscSRARR_GetElementCount(intrinsicKeys);
    gctUINT i;
    for (i = 0; i < keyCount; i++)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY *key = (VSC_SIMPLE_RESIZABLE_ARRAY *)vscSRARR_GetElement(intrinsicKeys, i);
        VIR_Instruction *pKeyInst = *(VIR_Instruction**)vscSRARR_GetElement(key, 0);
        if (VIR_Inst_IdenticalExpression(pKeyInst, pInst, VSC_CIE_GetShader(cie), gcvTRUE, gcvFALSE, gcvTRUE))
        {
            return i;
        }
    }
    return -1;
}

static void _VSC_CIE_DumpIntrinsicKey(
    IN VSC_CIE* cie,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *intrinsicKeys)
{
    gctUINT keyCount = vscSRARR_GetElementCount(intrinsicKeys);
    gctUINT i;
    for (i = 0; i < keyCount; i++)
    {
        gctUINT j;

        VSC_SIMPLE_RESIZABLE_ARRAY* candList = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscSRARR_GetElement(intrinsicKeys, i);
        VIR_LOG(VSC_CIE_GetDumper(cie), "common intrinsic in same group %d\n", i);
        for (j = 0; j < vscSRARR_GetElementCount(candList); j++)
        {
            VIR_Instruction *pInst = *(VIR_Instruction**)vscSRARR_GetElement(candList, j);
            VIR_Inst_Dump(cie->dumper, pInst);
        }
        VIR_LOG(VSC_CIE_GetDumper(cie), "===\n");
    }
}

static gctBOOL _VSC_CIE_RemovalIntrinsic(
    VIR_Instruction* pInst)
{
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_INTRINSIC)
    {
        VIR_IntrinsicsKind intrinsicKind = VIR_Operand_GetIntrinsicKind(VIR_Inst_GetSource(pInst, 0));
        /*TODO:: add more intrinsic */
        if (intrinsicKind == VIR_IK_image_fetch_for_sampler ||
            intrinsicKind == VIR_IK_image_fetch)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/*go through instruction list and collect intrinsic with only uniform/const/immediate parameters */
VSC_ErrCode _VSC_CIE_CollectCands(
    IN VSC_CIE* cie,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *intrinsicKeys)
{
    VSC_ErrCode          errCode = VSC_ERR_NONE;
    VSC_SIMPLE_RESIZABLE_ARRAY* key;
    VIR_Instruction     *pInst = VSC_CIE_GetCurrFunc(cie)->instList.pHead;
    while(pInst != gcvNULL)
    {
        if (_VSC_CIE_RemovalIntrinsic(pInst) &&
            _VSC_CIE_CheckIntrinsicWithConstant(pInst))
        {
            gctINT keyIndex = _VSC_CIE_FindIntrinsicKey(cie, intrinsicKeys, pInst);
            if (keyIndex != -1)
            {
                /*found the key */
                key = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscSRARR_GetElement(intrinsicKeys, keyIndex);
                vscSRARR_AddElement(key, &pInst);
            }
            else
            {
                /*create a new key and add to intrinsicKeys*/
                gctUINT i = vscSRARR_GetElementCount(intrinsicKeys);
                key = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscSRARR_GetNextEmpty(intrinsicKeys, &i);

                CHECK_ERROR0(vscSRARR_Initialize(key,
                                                 VSC_CIE_GetMm(cie),
                                                 4,
                                                 sizeof(VIR_Instruction*),
                                                 gcvNULL));
                vscSRARR_AddElement(key, &pInst);
            }
        }
        pInst = VIR_Inst_GetNext(pInst);
    }
    /* dump the intrinsicKeys */
    if (VSC_CIE_DumpTrace(cie))
    {
        _VSC_CIE_DumpIntrinsicKey(cie, intrinsicKeys);
    }

    return errCode;
}

static void _VSC_CIE_FINALIZE(
    IN VSC_SIMPLE_RESIZABLE_ARRAY *intrinsicKeys)
{
    gctUINT i;
    for (i = 0; i < vscSRARR_GetElementCount(intrinsicKeys); i++)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY* pInstArray = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscSRARR_GetElement(intrinsicKeys, i);
        vscSRARR_Finalize(pInstArray);
    }

    vscSRARR_Finalize(intrinsicKeys);
}

static VIR_BASIC_BLOCK* _VSC_CIE_GetCommonDomBB(
    VIR_BASIC_BLOCK* bb1,
    VIR_BASIC_BLOCK* bb2)
{
    if (BB_IS_DOM(bb1, bb2))
    {
        return bb1;
    }
    else if (BB_IS_DOM(bb2, bb1))
    {
        return bb2;
    }
    else
    {
        VIR_BASIC_BLOCK* domBB1 = BB_GET_IDOM(bb1);
        while (domBB1)
        {
            if (BB_IS_DOM(domBB1, bb2))
            {
                return domBB1;
            }
            domBB1 = BB_GET_IDOM(domBB1);
        }
    }
    gcmASSERT(gcvFALSE); /* should not come here */
    return gcvNULL;
}

static VIR_Instruction *_VSC_CIE_CheckAvaiableInsertPos(
    IN VSC_SIMPLE_RESIZABLE_ARRAY *pInstArray,
    VIR_Instruction **copyFromInst)
{
    gctUINT i;
    VIR_BASIC_BLOCK* tbb;
    VIR_Instruction *pInst = *(VIR_Instruction**)vscSRARR_GetElement(pInstArray, 0);
    VIR_BASIC_BLOCK* commonDomBB = VIR_Inst_GetBasicBlock(pInst);
    VIR_Instruction *posInst = pInst;
    *copyFromInst = pInst;

    for (i = 1; i < vscSRARR_GetElementCount(pInstArray); i++)
    {
        VIR_BASIC_BLOCK *newCommBB = gcvNULL;
        pInst = *(VIR_Instruction**)vscSRARR_GetElement(pInstArray, i);
        tbb = VIR_Inst_GetBasicBlock(pInst);
        newCommBB = _VSC_CIE_GetCommonDomBB(commonDomBB, tbb);
        if (newCommBB == tbb)
        {
            *copyFromInst = pInst;
            posInst = pInst;
        }
        else if (newCommBB != commonDomBB && commonDomBB != tbb)
        {
            *copyFromInst = gcvNULL; /* find common dominator, reset reuseIntrinsicInst */
        }
        commonDomBB = newCommBB;
        gcmASSERT(commonDomBB != gcvNULL);
    }

    if (*copyFromInst == gcvNULL)
    {
        *copyFromInst = *(VIR_Instruction**)vscSRARR_GetElement(pInstArray, 0);
        posInst = BB_GET_START_INST(commonDomBB);
    }
    return posInst;
}

VSC_ErrCode _VSC_CIE_Replace(
    IN VSC_CIE* cie,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *pInstArray)
{
    VIR_Instruction *pInst = gcvNULL;
    VIR_Instruction *newCopiedInst = gcvNULL;
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    gctUINT  i;
    VIR_Instruction *posInst = _VSC_CIE_CheckAvaiableInsertPos(pInstArray, &pInst);
    /* if no position instruction is found, skip the elimination */
    if (!posInst)
    {
        return errCode;
    }
    /* copy pInst before posInst and new a dest */
    if (VIR_Inst_GetOpcode(posInst) == VIR_OP_LABEL)
    {
        VIR_Function_AddCopiedInstructionAfter(VSC_CIE_GetCurrFunc(cie),
                                               pInst,
                                               posInst,
                                               gcvTRUE,
                                               &newCopiedInst);
    }
    else
    {
        VIR_Function_AddCopiedInstructionBefore(VSC_CIE_GetCurrFunc(cie),
                                                pInst,
                                                posInst,
                                                gcvTRUE,
                                                &newCopiedInst);
    }

    /* new dest temp variable */
    {
        VIR_Operand* dest = VIR_Inst_GetDest(newCopiedInst);
        VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
        VIR_VirRegId newVirRegId = VIR_Shader_NewVirRegId(VSC_CIE_GetShader(cie), 1);
        VIR_Operand* newDest = gcvNULL;
        VIR_TypeId   newDestTypeID = destSym ? VIR_Symbol_GetTypeId(destSym) : VIR_Operand_GetTypeId(dest);
        VIR_SymId    newDestSymId = VIR_INVALID_ID;
        VIR_Symbol* newDestsym;
        VIR_Precision prec = VIR_Operand_GetPrecision(dest);
        VIR_Swizzle swizzle;
        errCode = VIR_Shader_AddSymbol(VSC_CIE_GetShader(cie),
                                           VIR_SYM_VIRREG,
                                           newVirRegId,
                                           VIR_Shader_GetTypeFromId(VSC_CIE_GetShader(cie), newDestTypeID),
                                           VIR_STORAGE_UNKNOWN,
                                           &newDestSymId);
        newDest = VIR_Inst_GetDest(newCopiedInst);
        VIR_Operand_SetTempRegister(newDest,
                                    VSC_CIE_GetCurrFunc(cie),
                                    newDestSymId,
                                    newDestTypeID);
        newDestsym = VIR_Shader_GetSymFromId(VSC_CIE_GetShader(cie), newDestSymId);
        VIR_Symbol_SetPrecision(newDestsym, prec);

        /* replace newDest to instruction of instArray */
        swizzle = VIR_Enable_2_Swizzle(VIR_Operand_GetEnable(newDest));
        for(i = 0; i < vscSRARR_GetElementCount(pInstArray); i++)
        {
            VIR_Instruction *inst = *(VIR_Instruction**)vscSRARR_GetElement(pInstArray, i);
            VIR_Operand *src0 = VIR_Inst_GetSource(inst, 0);
            VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
            VIR_Inst_SetSrcNum(inst, 1);
            VIR_Operand_SetTempRegister(src0, VSC_CIE_GetCurrFunc(cie), newDestSymId, newDestTypeID);
            VIR_Operand_SetSwizzle(src0, swizzle);
        }
    }

    return errCode;
}

void _VSC_CIE_EliminateCommonIntrinsic(
    IN VSC_CIE* cie,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *intrinsicKeys)
{
    gctUINT i;
    VIR_CONTROL_FLOW_GRAPH* cfg = VIR_Function_GetCFG(VSC_CIE_GetCurrFunc(cie));
    vscVIR_BuildDOMTreePerCFG(cfg);

    for (i = 0; i < vscSRARR_GetElementCount(intrinsicKeys); i++)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY* pInstArray = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscSRARR_GetElement(intrinsicKeys, i);
        gctUINT   candCount = vscSRARR_GetElementCount(pInstArray);
        if (candCount >= VSC_CIE_GetThresHold(cie))
        {
            _VSC_CIE_Replace(cie, pInstArray);
            VSC_CIE_SetCodeChanged(cie,gcvTRUE);
        }
    }
    /* delete dom tree */
    vscVIR_DestroyDOMTreePerCFG(cfg);
}

VSC_ErrCode _VSC_CIE_PerformOnFunction(IN VSC_CIE* cie)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_SIMPLE_RESIZABLE_ARRAY intrinsicKeys;

    CHECK_ERROR0(vscSRARR_Initialize(&intrinsicKeys, VSC_CIE_GetMm(cie), 4, sizeof(VSC_SIMPLE_RESIZABLE_ARRAY), gcvNULL));

    _VSC_CIE_CollectCands(cie, &intrinsicKeys);

    /* replace common expression */
    if (vscSRARR_GetElementCount(&intrinsicKeys) > 0)
    {
        _VSC_CIE_EliminateCommonIntrinsic(cie, &intrinsicKeys);
    }

    /*destroy */
    _VSC_CIE_FINALIZE(&intrinsicKeys);
    return errCode;
}

/* now only focus on INTRINSIC (UNIFORM, CONST) */
DEF_QUERY_PASS_PROP(VSC_CIE_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_CIE;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_CIE_PerformOnShader)
{
    VIR_Shader *Shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    if (VIR_Shader_GetKind(Shader) == VIR_SHADER_FRAGMENT ||
        VIR_Shader_GetKind(Shader) == VIR_SHADER_GEOMETRY)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/* common expression elimination entry function */
VSC_ErrCode VSC_CIE_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader           *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Dumper           *dumper = pPassWorker->basePassWorker.pDumper;
    VSC_OPTN_CIEOptions* options = (VSC_OPTN_CIEOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctBOOL              enableTrace = VSC_OPTN_CIEOptions_GetTrace(options);

    if(enableTrace)
    {
        VIR_Shader_Dump(gcvNULL, "Common INTRINSIC Elimination BEGIN", shader, gcvTRUE);
    }

    {
        VSC_CIE cie;
        VIR_FuncIterator funcIter;
        VIR_FunctionNode* funcNode;
        gctUINT funcIndex;

        _VSC_CIE_Init(&cie, shader, gcvNULL, dumper, pPassWorker->basePassWorker.pMM, enableTrace, VSC_OPTN_CIEOptions_GetThreshold(options));
        VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(shader));
        for(funcNode = VIR_FuncIterator_First(&funcIter), funcIndex = 0;
            funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter), ++funcIndex)
        {
            VIR_Function* func = funcNode->function;
            if (VIR_Function_GetInstCount(func) > 0)
            {
                VSC_CIE_SetCurrFunc(&cie, func);
                errCode = _VSC_CIE_PerformOnFunction(&cie);
                if(errCode)
                {
                    break;
                }
            }
        }
        if (VSC_CIE_GetCodeChanged(&cie))
        {
            pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;
            pPassWorker->pResDestroyReq->s.bInvalidateRdFlow = gcvTRUE;
        }
    }
    if(VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Common INTRINSIC Elimination End", shader, gcvTRUE);
    }

    return errCode;
}


