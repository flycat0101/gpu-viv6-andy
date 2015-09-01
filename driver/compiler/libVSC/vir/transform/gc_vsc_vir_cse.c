/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
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

static void _VSC_LCSE_Init(
    IN OUT VSC_LCSE*            lcse,
    IN VIR_Shader*              shader,
    IN VIR_Function*            currFunc,
    IN VIR_BASIC_BLOCK*         currBB,
    IN VSC_OPTN_LCSEOptions*    options,
    IN VIR_Dumper*              dumper
    )
{
    VSC_LCSE_SetShader(lcse, shader);
    VSC_LCSE_SetCurrFunc(lcse, currFunc);
    VSC_LCSE_SetCurrBB(lcse, currBB);
    VSC_LCSE_SetOptions(lcse, options);
    VSC_LCSE_SetDumper(lcse, dumper);
}

static VSC_ErrCode _VSC_LCSE_ReplaceInst(
    IN OUT VSC_LCSE* lcse,
    IN VIR_Instruction*         commonEval,
    IN VIR_Instruction*         toBeReplaced
    )
{
    VSC_ErrCode errCode;
    do
    {
        VSC_OPTN_LCSEOptions* options = VSC_LCSE_GetOptions(lcse);
        VIR_Dumper* dumper = VSC_LCSE_GetDumper(lcse);
        VIR_Operand* commonInstDest = VIR_Inst_GetDest(commonEval);
        VIR_Operand* newSrc;
        VIR_OpCode toBeReplacedOp = VIR_Inst_GetOpcode(toBeReplaced);

        /* to be replaced instruction */
        if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
        {
            VIR_LOG(dumper, "To be replaced instruction:\n");
            VIR_Inst_Dump(dumper, toBeReplaced);
        }

        errCode = VIR_Function_DupOperand(VSC_LCSE_GetCurrFunc(lcse), commonInstDest, &newSrc);
        if(errCode)
        {
            break;
        }
        VIR_Operand_Change2Src(newSrc);
        VIR_Inst_SetOpcode(toBeReplaced, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(toBeReplaced, 1);
        VIR_Inst_SetSource(toBeReplaced, 0, newSrc);
        if(!VIR_OPCODE_isComponentwise(toBeReplacedOp))
        {
            switch(VIR_Enable_Channel_Count(VIR_Operand_GetEnable(VIR_Inst_GetDest(toBeReplaced))))
            {
                case 1:
                    VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XXXX);
                    break;
                case 2:
                    VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XYYY);
                    break;
                case 3:
                    VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XYZZ);
                    break;
                case 4:
                    VIR_Operand_SetSwizzle(newSrc, VIR_SWIZZLE_XYZW);
                    break;
                default:
                    gcmASSERT(0);
            }
        }

        /* replaced instruction */
        if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_REPLACING))
        {
            VIR_LOG(dumper, "Replaced instruction:\n");
            VIR_Inst_Dump(dumper, toBeReplaced);
        }
    } while(gcvFALSE);

    return errCode;
}

typedef struct VSC_LCSE_EXPMAP
{
    VSC_LCSE*               lcse;
    VSC_UNI_LIST            keyList;
    VSC_PRIMARY_MEM_POOL    pmp;
} VSC_LCSE_ExpMap;

#define VSC_LCSE_ExpMap_GetLCSE(em)         ((em)->lcse)
#define VSC_LCSE_ExpMap_SetLCSE(em, l)      ((em)->lcse = (l))
#define VSC_LCSE_ExpMap_GetShader(em)       ((em)->lcse->shader)
#define VSC_LCSE_ExpMap_GetOptions(em)      ((em)->lcse->options)
#define VSC_LCSE_ExpMap_GetDumper(em)       ((em)->lcse->dumper)
#define VSC_LCSE_ExpMap_GetKeyList(em)      (&(em)->keyList)
#define VSC_LCSE_ExpMap_GetPmp(em)          (&(em)->pmp)
#define VSC_LCSE_ExpMap_GetMM(em)           (&((em)->pmp.mmWrapper))

static void _VSC_LCSE_ExpMap_Init(
    IN OUT VSC_LCSE_ExpMap*     expMap,
    IN VSC_LCSE*                lcse
    )
{
    VSC_LCSE_ExpMap_SetLCSE(expMap, lcse);
    vscUNILST_Initialize(VSC_LCSE_ExpMap_GetKeyList(expMap), gcvFALSE);
    vscPMP_Intialize(VSC_LCSE_ExpMap_GetPmp(expMap), gcvNULL, 1024,
                     sizeof(void*), gcvTRUE);
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
        if(VIR_Inst_IdenticalExpression(keyInst, inst, VSC_LCSE_ExpMap_GetShader(expMap), gcvTRUE))
        {
            return key;
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
    vscPMP_Finalize(VSC_LCSE_ExpMap_GetPmp(expMap));
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

    gcmASSERT(VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(killingInst)) ||
              VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(killingInst)));

    vscULIterator_Init(&iter, keyList);
    for(key = (VSC_LCSE_Key*)vscULIterator_First(&iter);
        key != gcvNULL; key = (VSC_LCSE_Key*)vscULIterator_Next(&iter))
    {
        VIR_Instruction* keyInst = VSC_LCSE_Key_GetInst(key);
        gctUINT i;
        gctBOOL kills = gcvFALSE;

        if(VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(killingInst)) &&
           VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(keyInst)))
        {
            if(!VIR_Operand_Identical(VIR_Inst_GetSource(killingInst, 0), VIR_Inst_GetSource(keyInst, 0), VSC_LCSE_ExpMap_GetShader(expMap)))
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

        /* work on the expression */
        do
        {
            if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetHeuristics(options), VSC_OPTN_LCSEOptions_HEUR_LOAD_ONLY))
            {
                if(VIR_Inst_GetOpcode(instIter) != VIR_OP_LOAD)
                {
                    break;
                }
            }
            else
            {
                if(!VIR_OPCODE_IsExpr(VIR_Inst_GetOpcode(instIter)))
                {
                    break;
                }
                if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetHeuristics(options), VSC_OPTN_LCSEOptions_HEUR_EXCLUDE_MUL))
                {
                    if(VIR_Inst_GetOpcode(instIter) == VIR_OP_MUL)
                    {
                        break;
                    }
                }
            }

            if(_VSC_LCSE_ExpMap_FindExp(&expMap, instIter, &commonEval, &firstEval))
            {
                if(commonEval)
                {
                    _VSC_LCSE_ReplaceInst(lcse, commonEval, instIter);
                }
                else
                {
                    gctUINT i;
                    VIR_Instruction* newEval;
                    VIR_VirRegId newDstRegid;
                    VIR_SymId newDstSymid;
                    VIR_TypeId newDstTypeID;

                    gcmASSERT(firstEval);
                    VIR_Function_AddInstructionBefore(VSC_LCSE_GetCurrFunc(lcse), VIR_Inst_GetOpcode(instIter), VIR_Operand_GetType(VIR_Inst_GetDest(instIter)), firstEval, &newEval);

                    newDstRegid = VIR_Shader_NewVirRegId(VSC_LCSE_GetShader(lcse), 1);
                    newDstTypeID = VIR_Inst_GetExpressionTypeID(firstEval, VSC_LCSE_GetShader(lcse));
                    errCode = VIR_Shader_AddSymbol(VSC_LCSE_GetShader(lcse),
                                                   VIR_SYM_VIRREG,
                                                   newDstRegid,
                                                   VIR_Shader_GetTypeFromId(VSC_LCSE_GetShader(lcse), newDstTypeID),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &newDstSymid);
                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(newEval), VSC_LCSE_GetCurrFunc(lcse), newDstSymid, newDstTypeID);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(newEval), VIR_ENABLE_XYZW);
                    VIR_Operand_SetPrecision(VIR_Inst_GetDest(newEval), VIR_Operand_GetPrecision(VIR_Inst_GetDest(firstEval)));
                    VIR_Symbol_SetPrecision(VIR_Operand_GetSymbol(VIR_Inst_GetDest(newEval)), VIR_Operand_GetPrecision(VIR_Inst_GetDest(firstEval)));
                    VIR_Inst_SetConditionOp(newEval, VIR_Inst_GetConditionOp(firstEval));

                    for(i = 0; i < VIR_Inst_GetSrcNum(firstEval); i++)
                    {
                        VIR_Inst_SetSource(newEval, i, VIR_Inst_GetSource(firstEval, i));
                    }

                    /* inserted common evaluation instruction */
                    if(VSC_UTILS_MASK(VSC_OPTN_LCSEOptions_GetTrace(options), VSC_OPTN_LCSEOptions_TRACE_INSERTING))
                    {
                        VIR_LOG(dumper, "Inserted common evaluation instruction:\n");
                        VIR_Inst_Dump(dumper, newEval);
                    }

                    _VSC_LCSE_ExpMap_Update(&expMap, firstEval, newEval);
                    _VSC_LCSE_ReplaceInst(lcse, newEval, firstEval);
                    _VSC_LCSE_ReplaceInst(lcse, newEval, instIter);
                }
            }
            else
            {
                _VSC_LCSE_ExpMap_Gen(&expMap, instIter);
            }
        } while(gcvFALSE);

        /* kill redefined expression */
        if(VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(instIter)) ||
           VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(instIter)))
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

/* local common expression elimination entry function */
VSC_ErrCode VSC_LCSE_PerformOnShader(
    IN VIR_Shader*              shader,
    IN VSC_OPTN_LCSEOptions*    options,
    IN VIR_Dumper*              dumper
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

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

        _VSC_LCSE_Init(&lcse, shader, gcvNULL, gcvNULL, options, dumper);
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

    if(gcSHADER_DumpCodeGenVerbose(shader))
    {
        VIR_Shader_Dump(gcvNULL, "LCSE End", shader, gcvTRUE);
    }

    return errCode;
}


