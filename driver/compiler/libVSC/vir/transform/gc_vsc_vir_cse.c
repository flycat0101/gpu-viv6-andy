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

typedef struct VSC_LCSE
{
    VIR_Shader*             shader;
    VIR_Function*           currFunc;
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

                if (VIR_Operand_Identical(srcOpnd, newDstOpnd, VSC_LCSE_GetShader(lcse)))
                {
                    VIR_TypeId srcOpndTypeId = VIR_Operand_GetTypeId(srcOpnd);

                    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
                    {
                        VIR_LOG(dumper, "change the use instruction:\n");
                        VIR_Inst_Dump(dumper, instIter);
                    }

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

                    VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(replacedInst, 0));
                    VIR_Operand_SetTypeId(srcOpnd, srcOpndTypeId);

                    VIR_Operand_GetOperandInfo(instIter, srcOpnd, &srcInfo);

                    for (channel = 0; channel < VIR_CHANNEL_COUNT; channel++)
                    {
                        vscVIR_AddNewUsageToDef(VSC_LCSE_GetDuInfo(lcse),
                            commonInst,
                            instIter,
                            srcOpnd,
                            gcvFALSE,
                            srcInfo.u1.virRegInfo.virReg,
                            1,
                            (1 << channel),
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);
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

    return errCode;
}

static VSC_ErrCode _VSC_LCSE_ReplaceInst(
    IN OUT VSC_LCSE* lcse,
    IN VIR_Instruction*         commonEval,
    IN VIR_Instruction*         toBeReplaced
    )
{
    VSC_ErrCode errCode;
    gctUINT srcIdx, channel;
    VIR_Operand     *srcOpnd;
    VIR_OperandInfo srcInfo, dstInfo;
    VIR_Swizzle     srcSwizzle;

    do
    {
        VSC_OPTN_LCSEOptions* options = VSC_LCSE_GetOptions(lcse);
        VIR_Dumper* dumper = VSC_LCSE_GetDumper(lcse);
        VIR_Operand* commonInstDest = VIR_Inst_GetDest(commonEval);
        VIR_Operand* toBeReplacedOperand = VIR_Inst_GetSource(toBeReplaced, 0);
        VIR_Operand_GetOperandInfo(commonEval, commonInstDest, &dstInfo);

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

            vscVIR_DeleteUsage(VSC_LCSE_GetDuInfo(lcse),
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
        VIR_Operand_SetSwizzle(toBeReplacedOperand, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(commonInstDest)));

        VIR_Inst_SetOpcode(toBeReplaced, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(toBeReplaced, 1);

        for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel++)
        {
            if (VIR_Operand_GetEnable(commonInstDest) & (1 << channel))
            {
                vscVIR_AddNewUsageToDef(VSC_LCSE_GetDuInfo(lcse),
                    commonEval,
                    toBeReplaced,
                    toBeReplacedOperand,
                    gcvFALSE,
                    dstInfo.u1.virRegInfo.virReg,
                    1,
                    (1 << channel),
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvNULL);
            }
        }

        /* replaced instruction */
        if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
        {
            VIR_LOG(dumper, "Replaced instruction:\n");
            VIR_Inst_Dump(dumper, toBeReplaced);
        }

        /* we replace the uses of the def of toBeReplaced to expose more LCSE opportunities */
        errCode = _VSC_LCSE_ReplaceUses(lcse, toBeReplaced, commonEval);

    } while(gcvFALSE);

    return errCode;
}

typedef struct VSC_LCSE_EXPMAP
{
    VSC_LCSE*               lcse;
    VSC_UNI_LIST            keyList;
    VSC_MM*                 pMM;
} VSC_LCSE_ExpMap;

#define VSC_LCSE_ExpMap_GetLCSE(em)         ((em)->lcse)
#define VSC_LCSE_ExpMap_SetLCSE(em, l)      ((em)->lcse = (l))
#define VSC_LCSE_ExpMap_GetShader(em)       ((em)->lcse->shader)
#define VSC_LCSE_ExpMap_GetOptions(em)      ((em)->lcse->options)
#define VSC_LCSE_ExpMap_GetDumper(em)       ((em)->lcse->dumper)
#define VSC_LCSE_ExpMap_GetKeyList(em)      (&(em)->keyList)
#define VSC_LCSE_ExpMap_GetMM(em)           ((em)->lcse->pMM)

static void _VSC_LCSE_ExpMap_Init(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VSC_LCSE*                lcse
    )
{
    VSC_LCSE_ExpMap_SetLCSE(expMap, lcse);
    vscUNILST_Initialize(VSC_LCSE_ExpMap_GetKeyList(expMap), gcvFALSE);
    expMap->pMM = lcse->pMM;
}

static VSC_LCSE_Key* _VSC_LCSE_ExpMap_NewKey(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*         inst,
    IN VIR_Instruction*         commonEval,
    IN VIR_Instruction*         firstEval
    )
{
    VSC_LCSE_Key* newKey = (VSC_LCSE_Key*)vscMM_Alloc(VSC_LCSE_ExpMap_GetMM(expMap), sizeof(VSC_LCSE_Key));
    _VSC_LCSE_Key_Init(newKey, inst, commonEval, firstEval);

    return newKey;
}

static VSC_LCSE_Key* _VSC_LCSE_ExpMap_FindSameExpKey(
    IN VSC_LCSE_ExpMap*         expMap,
    IN VIR_Instruction*         inst
    )
{
    VSC_UNI_LIST* keyList = VSC_LCSE_ExpMap_GetKeyList(expMap);
    VSC_UL_ITERATOR iter;
    VSC_LCSE_Key* key;

    vscULIterator_Init(&iter, keyList);
    for(key = (VSC_LCSE_Key*)vscULIterator_First(&iter);
        key != gcvNULL; key = (VSC_LCSE_Key*)vscULIterator_Next(&iter))
    {
        VIR_Instruction* keyInst = VSC_LCSE_Key_GetInst(key);
        VIR_OpCode      keyOpcode = VIR_Inst_GetOpcode(keyInst);

        if(VIR_Inst_IdenticalExpression(keyInst, inst, VSC_LCSE_ExpMap_GetShader(expMap), gcvTRUE, gcvTRUE))
        {
            /* load r1.xy, base, offset
               load r1.yz, base, offset
               the two loads are loading the same data to xy and yz */
            if (VIR_OPCODE_isMemLd(keyOpcode))
            {
                if ((VIR_Enable_Channel_Count(VIR_Inst_GetEnable(keyInst)) == 1 &&
                     VIR_Enable_Channel_Count(VIR_Inst_GetEnable(inst))    == 1) ||
                    VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst))
                {
                    return key;
                }
            }
            /* load_attr r1.x, base, regmap_index, attr_index
               load_attr r1.y, base, regmap_index, attr_index
               the two load_attrs are loading x and y components */
            else if (VIR_OPCODE_isAttrLd(VIR_Inst_GetOpcode(keyInst)))
            {
                if (VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst))
                {
                    return key;
                }
            }
            else
            {
                if (VIR_Inst_GetEnable(keyInst) == VIR_Inst_GetEnable(inst))
                {
                    return key;
                }
            }
        }
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

static void _VSC_LCSE_ExpMap_Final(
    IN OUT VSC_LCSE_ExpMap*    expMap
    )
{
    vscUNILST_Finalize(VSC_LCSE_ExpMap_GetKeyList(expMap));
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

static void _VSC_LCSE_ExpMap_Gen(
    IN VSC_LCSE_ExpMap*     expMap,
    IN VIR_Instruction*     inst
    )
{
    VSC_OPTN_LCSEOptions* options = VSC_LCSE_ExpMap_GetOptions(expMap);
    VIR_Dumper* dumper = VSC_LCSE_ExpMap_GetDumper(expMap);

    /* generated new expression */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_GENERATING))
    {
        VIR_LOG(dumper, "Generated new expression:\n");
        VIR_Inst_Dump(dumper, inst);
    }

    _VSC_LCSE_ExpMap_InsertKey(expMap, _VSC_LCSE_ExpMap_NewKey(expMap, inst, gcvNULL, inst));
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

    if (sym && VIR_Symbol_isOutput(sym))
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
    VSC_OPTN_LCSEOptions* options = VSC_LCSE_ExpMap_GetOptions(expMap);
    VIR_Dumper* dumper = VSC_LCSE_ExpMap_GetDumper(expMap);
    VSC_UNI_LIST* keyList = VSC_LCSE_ExpMap_GetKeyList(expMap);
    VSC_UL_ITERATOR iter;
    VSC_LCSE_Key* key;
    VIR_OpCode  killOpcode = VIR_Inst_GetOpcode(killingInst);

    gcmASSERT(VIR_OPCODE_hasDest(killOpcode) ||
              VIR_OPCODE_isMemSt(killOpcode) ||
              VIR_OPCODE_isAttrSt(killOpcode) ||
              killOpcode == VIR_OP_EMIT0 ||
              killOpcode == VIR_OP_EMIT);

    vscULIterator_Init(&iter, keyList);
    for(key = (VSC_LCSE_Key*)vscULIterator_First(&iter);
        key != gcvNULL; key = (VSC_LCSE_Key*)vscULIterator_Next(&iter))
    {
        VIR_Instruction* keyInst = VSC_LCSE_Key_GetInst(key);
        VIR_OpCode       keyOpcode = VIR_Inst_GetOpcode(keyInst);
        gctUINT i;
        gctBOOL kills = gcvFALSE;

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
                else if(!VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 0), VIR_Inst_GetSource(keyInst, 0), VSC_LCSE_ExpMap_GetShader(expMap)))
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
                    kills = VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 1), VIR_Inst_GetSource(keyInst, 1), VSC_LCSE_ExpMap_GetShader(expMap));
                }
            }
            else if (VIR_OPCODE_isAttrSt(killOpcode) && VIR_OPCODE_isAttrLd(keyOpcode))
            {
                if(!VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 0), VIR_Inst_GetSource(keyInst, 0), VSC_LCSE_ExpMap_GetShader(expMap)))
                {
                    /* different base means no overlap */
                    kills = gcvFALSE;
                }
                else if(VIR_Operand_isSymbol(VIR_Inst_GetSource(killingInst, 1)) ||
                        VIR_Operand_isSymbol(VIR_Inst_GetSource(keyInst, 1)))
                {
                    /* same base and invocation id is symbol. we have to assume they overlaps */
                    kills = gcvTRUE;
                }
                else
                {
                    /* same base, constant offset. see whether the constant values are the same */
                    if (VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 1), VIR_Inst_GetSource(keyInst, 1), VSC_LCSE_ExpMap_GetShader(expMap)))
                    {
                        if(VIR_Operand_isSymbol(VIR_Inst_GetSource(killingInst, 2)) ||
                           VIR_Operand_isSymbol(VIR_Inst_GetSource(keyInst, 2)))
                        {
                            /* same base, same invocation id and offset is symbol. we have to assume they overlaps */
                            kills = gcvTRUE;
                        }
                        else
                        {
                            kills = VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 2), VIR_Inst_GetSource(keyInst, 2), VSC_LCSE_ExpMap_GetShader(expMap));
                        }
                    }
                    else
                    {
                        kills = gcvFALSE;
                    }
                }
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

    if(BB_GET_LENGTH(bb) == 0)
    {
        return errCode;
    }

    /* dump input bb */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_INPUT_BB))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination starts for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, BB_GET_ID(bb), VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvTRUE);
    }

    _VSC_LCSE_ExpMap_Init(&expMap, lcse);
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
                    _VSC_LCSE_ReplaceInst(lcse, commonEval, instIter);
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
                    _VSC_LCSE_ReplaceInst(lcse, firstEval, instIter);
                }
            }
            else
            {
                _VSC_LCSE_ExpMap_Gen(&expMap, instIter);
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
    _VSC_LCSE_ExpMap_Final(&expMap);

    /* dump output bb */
    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_OUTPUT_BB))
    {
        VIR_LOG(dumper, "%s\nLocal Common Subexpression Elimination ends for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, BB_GET_ID(bb), VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode _VSC_LCSE_PerformOnFunction(
    IN VSC_LCSE*                lcse
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Function* func = VSC_LCSE_GetCurrFunc(lcse);
    VSC_OPTN_LCSEOptions* options = VSC_LCSE_GetOptions(lcse);
    VIR_Dumper* dumper = VSC_LCSE_GetDumper(lcse);
    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfg_iter;
    VIR_BASIC_BLOCK* bb;

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


    CFG_ITERATOR_INIT(&cfg_iter, cfg);
    for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
    {
        VSC_LCSE_SetCurrBB(lcse, bb);
        errCode = _VSC_LCSE_PerformOnBB(lcse);
        if(errCode)
        {
            return errCode;
        }
    }

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


