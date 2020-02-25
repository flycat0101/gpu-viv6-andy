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


#include "vir/transform/gc_vsc_vir_inline.h"

extern gctUINT vscHFUNC_Label(const char *);
extern gctBOOL vcsHKCMP_Label(const char *, const char *);

/* ===========================================================================
   _VSC_IL_UpdateMaxCallDepth:
   Update the func block's max call depth
   ===========================================================================
*/
static void _VSC_IL_UpdateMaxCallDepth(
    VIR_Inliner       *pInliner,
    VIR_FUNC_BLOCK    *pFuncBlk
    )
{
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CG_EDGE*                 pEdge;

    pFuncBlk->maxCallDepth = 0;
    /* go through all its callers */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
    pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
        if (callerBlk->maxCallDepth + 1 > pFuncBlk->maxCallDepth)
        {
            pFuncBlk->maxCallDepth = callerBlk->maxCallDepth + 1;
        }
    }
}

static VSC_ErrCode
VSC_IL_ReplaceSymInOperand(
    IN  VIR_Shader      *pShader,
    IN  VIR_Function    *pFunc,
    IN  VIR_Operand     *pOperand,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Symbol          *pOpndSym = gcvNULL;
    VIR_Symbol          *pNewVarSym = gcvNULL;
    gctUINT             i = 0;

    if (VIR_Operand_isParameters(pOperand))
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(pOperand);
        for (i = 0; i < parm->argNum; i++)
        {
            if (parm->args[i])
            {
                errCode = VSC_IL_ReplaceSymInOperand(pShader, pFunc, parm->args[i], pTempSet);
                ON_ERROR(errCode, "replace symbol in operand");
            }
        }
    }
    else if (VIR_Operand_isTexldParm(pOperand))
    {
        VIR_Operand *texldOperand = (VIR_Operand*)pOperand;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            if (VIR_Operand_GetTexldModifier(texldOperand,i))
            {
                errCode = VSC_IL_ReplaceSymInOperand(pShader, pFunc, VIR_Operand_GetTexldModifier(texldOperand,i), pTempSet);
                ON_ERROR(errCode, "replace symbol in operand");
            }
        }
    }
    else
    {
        if (VIR_Operand_isSymbol(pOperand))
        {
            pOpndSym = VIR_Operand_GetSymbol(pOperand);

            /* If this symbol is a variable and belong to the origFunc, udpate it.*/
            if (VIR_Symbol_isVariable(pOpndSym))
            {
                if (vscHTBL_DirectTestAndGet(pTempSet, (void*)pOpndSym, (void **)&pNewVarSym))
                {
                    VIR_Operand_SetSym(pOperand, pNewVarSym);
                }
            }
            /* If this symbol is a vreg and belong to the origFunc, udpate it. */
            else if (VIR_Symbol_isVreg(pOpndSym))
            {
                gcmASSERT(!VIR_Id_isFunctionScope(VIR_Symbol_GetIndex(pOpndSym)));
                if (vscHTBL_DirectTestAndGet(pTempSet, (void*)pOpndSym, (void **)&pNewVarSym))
                {
                    VIR_Operand_SetSym(pOperand, pNewVarSym);
                }
            }

            /*
            ** When the operand symbol is a combined sampler, we need to check its separateSampler and separateImage,
            ** if they are the function parameter, we need to update it.
            */
            if (isSymCombinedSampler(pOpndSym))
            {
                if (VIR_Symbol_GetSeparateImageFuncId(pOpndSym) != VIR_INVALID_ID)
                {
                    VIR_Symbol*     pSeparateImageSym = VIR_Symbol_GetSeparateImage(pShader, pOpndSym);

                    if (vscHTBL_DirectTestAndGet(pTempSet, (void*)pSeparateImageSym, (void **)&pNewVarSym))
                    {
                        VIR_Symbol_SetSeparateImageId(pOpndSym,
                                                      VIR_Symbol_GetIndex(pNewVarSym),
                                                      (isSymLocal(pNewVarSym) ? VIR_Function_GetSymId(VIR_Symbol_GetHostFunction(pNewVarSym)) : VIR_INVALID_ID));
                    }
                }
                if (VIR_Symbol_GetSeparateSamplerFuncId(pOpndSym) != VIR_INVALID_ID)
                {
                    VIR_Symbol*     pSeparateSamplerSym = VIR_Symbol_GetSeparateSampler(pShader, pOpndSym);

                    if (vscHTBL_DirectTestAndGet(pTempSet, (void*)pSeparateSamplerSym, (void **)&pNewVarSym))
                    {
                        VIR_Symbol_SetSeparateSamplerId(pOpndSym,
                                                        VIR_Symbol_GetIndex(pNewVarSym),
                                                        (isSymLocal(pNewVarSym) ? VIR_Function_GetSymId(VIR_Symbol_GetHostFunction(pNewVarSym)) : VIR_INVALID_ID));
                    }
                }
            }
        }

        if (VIR_Operand_GetRelAddrMode(pOperand) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *indexRegSym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(pOperand));

            if (vscHTBL_DirectTestAndGet(pTempSet, (void*)indexRegSym, (void **)&pNewVarSym))
            {
                VIR_Operand_SetRelIndexing(pOperand, VIR_Symbol_GetIndex(pNewVarSym));
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
VSC_IL_DupOperand(
    IN  VIR_Shader      *pShader,
    IN  VIR_Function    *pFunc,
    IN  VIR_Operand     *pOrigOperand,
    IN  VIR_Operand     **ppNewOperand,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Operand         *pNewOperand;

    errCode = VIR_Function_DupFullOperand(pFunc, pOrigOperand, ppNewOperand);
    ON_ERROR(errCode, "duplicate operand");

    /* Replace the variable symbol if needed. */
    pNewOperand = *ppNewOperand;
    errCode = VSC_IL_ReplaceSymInOperand(pShader, pFunc, pNewOperand, pTempSet);
    ON_ERROR(errCode, "replace symbol in operand");

OnError:
    return errCode;
}

/* Duplicate a new instruction in Function based on OrigInst */
VSC_ErrCode
VSC_IL_DupInstruction(
    IN  VIR_Inliner     *pInliner,
    IN  VIR_Function    *OrigFunction,
    IN  VIR_Function    *Function,
    IN  VIR_Instruction *OrigInst,
    IN  gctUINT         callerIdx,
    OUT VIR_Instruction **Inst,
    OUT VSC_HASH_TABLE  *pLabelSet,
    OUT VSC_HASH_TABLE  *pJmpSet,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Shader      *pShader = VSC_IL_GetShader(pInliner);

    VIR_Instruction *inst = (VIR_Instruction *)vscMM_Alloc(
                                               &Function->hostShader->pmp.mmWrapper,
                                               sizeof(VIR_Instruction));
    gctUINT srcNum = VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(OrigInst));
    gcmASSERT(srcNum <= VIR_MAX_SRC_NUM);

    *Inst = inst;
    if (inst == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        VIR_Operand *dest = gcvNULL, *src = gcvNULL, *origDest, *origSrc;
        gctUINT i;
        VIR_OpCode opcode;

        /* don't need to add symbol to the caller's symbol table, since function symbol table is not used yet */
        memset(inst, 0, sizeof(VIR_Instruction));
        VIR_Inst_SetOpcode(inst, VIR_Inst_GetOpcode(OrigInst));
        VIR_Inst_SetSrcNum(inst, srcNum);
        VIR_Inst_SetInstType(inst, VIR_Inst_GetInstType(OrigInst));
        VIR_Inst_SetConditionOp(inst, VIR_Inst_GetConditionOp(OrigInst));
        VIR_Inst_SetResOpType(inst, VIR_Inst_GetResOpType(OrigInst));
        VIR_Inst_SetFunction(inst, Function);
        VIR_Inst_SetId(inst, VIR_Function_GetAndIncressLastInstId(Function));

        VIR_Inst_CopySrcLoc(OrigInst->sourceLoc, inst->sourceLoc);

        /* allocate dest operand */
        if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(OrigInst)))
        {
            origDest = VIR_Inst_GetDest(OrigInst);
            errCode = VSC_IL_DupOperand(pShader, Function, origDest, &dest, pTempSet);
            VIR_Inst_SetDest(inst, dest);
        }

        /* allocate source operand */
        for (i = 0; i < srcNum; i++)
        {
            origSrc = VIR_Inst_GetSource(OrigInst, i);
            errCode = VSC_IL_DupOperand(pShader, Function, origSrc, &src, pTempSet);
            VIR_Inst_SetSource(inst, i, src);
        }

        opcode = VIR_Inst_GetOpcode(OrigInst);

        /* special handling for some special instructions */
        if (opcode == VIR_OP_LABEL)
        {
            VIR_Label       *label = gcvNULL;
            VIR_Label       *newLabel = gcvNULL;
            gctUINT offset = 0;
            gctCHAR labelName[__MAX_SYM_NAME_LENGTH__];
            VIR_LabelId labelId;

            label = VIR_Operand_GetLabel(VIR_Inst_GetDest(OrigInst));
            gcmASSERT(label != gcvNULL);
            gcoOS_PrintStrSafe(labelName,
                               gcmSIZEOF(labelName),
                               &offset,
                               "%s_%u_%u_%u",
                               VIR_Shader_GetSymNameString(Function->hostShader,
                               VIR_Function_GetSymbol(OrigFunction)),
                               VSC_IL_GetPassData(pInliner)->passIndex,
                               callerIdx,
                               VIR_Label_GetId(label));

            errCode = VIR_Function_AddLabel(Function,
                                            labelName,
                                            &labelId);
            newLabel = VIR_Function_GetLabelFromId(Function, labelId);
            newLabel->defined = inst;
            VIR_Operand_SetLabel(VIR_Inst_GetDest(inst), newLabel);
            vscHTBL_DirectSet(pLabelSet, (void*) label, (void*) newLabel);
        }
        else if (VIR_OPCODE_isBranch(opcode))
        {
            VIR_Label       *label = VIR_Operand_GetLabel(VIR_Inst_GetDest(OrigInst));
            VIR_Label       *pNewLabel = gcvNULL;
            VIR_Link        *pNewLink     = gcvNULL;

            if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) label, (void **)&pNewLabel))
            {
                VIR_Operand_SetLabel(VIR_Inst_GetDest(inst), pNewLabel);
                VIR_Function_NewLink(Function, &pNewLink);
                VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)inst);
                VIR_Link_AddLink(&(pNewLabel->referenced), pNewLink);
            }
            else
            {
                /* we need to save the unchanged jmp into a list, its label willl be changed
                   at the end */
                vscHTBL_DirectSet(pJmpSet, (void*) inst, gcvNULL);
            }
        }
    }

    return errCode;
}

/* if getVariableSym is true, return variable sym if found
 * otherwise  return virRegSymbol */
static VIR_Symbol*
_VSC_IL_GetActualParm(
    IN  VIR_Function    *pCallerFunc,
    IN  VIR_Function    *pCalleeFunc,
    VIR_Instruction     *pCallSiteInst,
    VIR_Symbol          *parmVregSym,
    gctBOOL             isInput
)
{
    VIR_Symbol* realParm = gcvNULL;
    VIR_Instruction *newInst = gcvNULL;
    if (isInput)
    {
        newInst = VIR_Shader_FindParmInst(pCalleeFunc, pCallSiteInst, gcvTRUE, parmVregSym, gcvNULL);
        /* now only deal with assignment to parament is MOV case */
        if (newInst && VIR_Inst_GetOpcode(newInst) == VIR_OP_MOV)
        {
            VIR_Operand *src0 = VIR_Inst_GetSource(newInst, 0);
            gcmASSERT(VIR_Operand_isSymbol(src0));
            realParm = VIR_Operand_GetSymbol(src0);
            VIR_Function_ChangeInstToNop(pCallerFunc, newInst);
        }
    }
    else
    {
        newInst = VIR_Shader_FindParmInst(pCalleeFunc, pCallSiteInst, gcvFALSE, parmVregSym, gcvNULL);
        if (newInst && VIR_Inst_GetOpcode(newInst) == VIR_OP_MOV)
        {
            VIR_Operand *dest = VIR_Inst_GetDest(newInst);
            gcmASSERT(VIR_Operand_isSymbol(dest));
            realParm = VIR_Operand_GetSymbol(dest);
            VIR_Function_ChangeInstToNop(pCallerFunc, newInst);
        }
    }

    return realParm;
}

static VSC_ErrCode
VSC_IL_DupSingleVariable(
    IN  VIR_Inliner     *pInliner,
    IN  VIR_Function    *pCallerFunc,
    IN  VIR_Function    *pCalleeFunc,
    IN  VIR_Symbol      *pOldSym,
    IN  gctUINT         callerIdx,
    IN  VIR_Instruction *pCallSiteInst,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VIR_TypeId          oldTypeId, oldBaseTypeId;
    VIR_SymId           oldRegId = VIR_INVALID_ID;
    gctSTRING           oldName = gcvNULL;
    VIR_SymId           newVarSymId = VIR_INVALID_ID;
    VIR_Symbol          *pNewVarSym = gcvNULL;
    VIR_Symbol          *pVirRegSym = gcvNULL;
    gctINT              indexRange = 0;
    gctUINT             i, regCount;
    gctUINT             newVregId;
    gctBOOL             isCallerMainFunc = VIR_Function_HasFlag(pCallerFunc, VIR_FUNCFLAG_MAIN);
    gctBOOL             isParam = VIR_Symbol_isParamVariable(pOldSym);
    /* we can't handle param is InOut at the same time */
    gctBOOL             isInputOrOutParam = (VIR_Symbol_isInParam(pOldSym) ^ VIR_Symbol_isOutParam(pOldSym));

    oldRegId = VIR_Symbol_GetVariableVregIndex(pOldSym);
    /* for parameter, need to check if the corresponding global variable is array, if so
     * use the global variable for the copying so it duplicated variable has the correct
     * register count and can be indexed, otherwise duplicated variable just a scalar which
     * cannot be indexed */
    if (isParam)
    {
        VIR_Symbol   *pVarSym;
        VIR_TypeId    typeId;
        VIR_Type     *type;
        pVirRegSym = VIR_Shader_FindSymbolByTempIndex(pShader, oldRegId);
        if (pVirRegSym)
        {
            gcmASSERT(VIR_Symbol_isVreg(pVirRegSym));
            pVarSym = VIR_Symbol_GetVregVariable(pVirRegSym);
            gcmASSERT(pVarSym);
            typeId = VIR_Symbol_GetTypeId(pVarSym);
            type = VIR_Shader_GetTypeFromId(pShader, typeId);
            if (VIR_Type_isArray(type))
            {
                /* the global var is array */
                if (oldRegId != VIR_Symbol_GetVregIndex(pVarSym))
                {
                    /* not first virreg, skip it */
                    return errCode;
                }
                /* use global varialbe to dup the symbol */
                pOldSym = pVarSym;
            }
        }
    }
    oldTypeId = VIR_Symbol_GetTypeId(pOldSym);
    oldBaseTypeId = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(pShader, oldTypeId));
    oldName = VIR_Shader_GetSymNameString(pShader, pOldSym);
    indexRange = VIR_Symbol_GetIndexRange(pOldSym) - VIR_Symbol_GetVregIndex(pOldSym);

    regCount = VIR_Type_GetRegOrOpaqueCount(pShader,
                                            VIR_Shader_GetTypeFromId(pShader, oldTypeId),
                                            VIR_TypeId_isSampler(oldBaseTypeId),
                                            VIR_TypeId_isImage(oldBaseTypeId),
                                            VIR_TypeId_isAtomicCounters(oldBaseTypeId),
                                            gcvFALSE);

    if (!vscHTBL_DirectTestAndGet(pTempSet, (void*)pOldSym, (void **)&pNewVarSym))
    {
        /* if parameter is pass by reference, find the real parameter to replace it
         * now NOT deal with parameter used as input and output at the same time case.
         * otherwise create a new temp variable to copy value
         */
        if (isParam && isInputOrOutParam && isSymPassByReference(pOldSym))
        {
            gcmASSERT(pCallSiteInst != gcvNULL);
            for (i = 0; i < regCount; i++)
            {
                VIR_Symbol      *pTempVirReg = gcvNULL;
                VIR_Symbol      *pNewVirReg = gcvNULL;
                VIR_VirRegId    tmpVregId = oldRegId + i;
                pTempVirReg = VIR_Shader_FindSymbolByTempIndex(pShader, tmpVregId);
                if (pTempVirReg == gcvNULL || vscHTBL_DirectTestAndGet(pTempSet, (void*)pTempVirReg, (void **)&pNewVirReg))
                {
                    continue;
                }
                pNewVirReg = _VSC_IL_GetActualParm(pCallerFunc, pCalleeFunc, pCallSiteInst, pTempVirReg, VIR_Symbol_isInParam(pOldSym));
                /* save the new vreg to the table */
                if (pNewVirReg)
                {
                    vscHTBL_DirectSet(pTempSet, (void*)pTempVirReg, (void*)pNewVirReg);
                }
            }
        }
        else
        {
            gctCHAR     newVarName[__MAX_SYM_NAME_LENGTH__];
            gctCHAR     temp[16];
            gctUINT     offset = 0;

            gcoOS_PrintStrSafe(temp, 16, &offset, "%d-%d-", VIR_Function_GetSymId(pCallerFunc), VIR_Function_GetSymId(pCalleeFunc));
            gcoOS_StrCopySafe(newVarName, __MAX_SYM_NAME_LENGTH__, temp);
            gcoOS_StrCatSafe(newVarName, __MAX_SYM_NAME_LENGTH__, oldName);

            /* Pass index. */
            offset = 0;
            gcoOS_PrintStrSafe(temp, 16, &offset, "-%d", VSC_IL_GetPassData(pInliner)->passIndex);
            gcoOS_StrCatSafe(newVarName, 128, temp);

            /* Caller index. */
            offset = 0;
            gcoOS_PrintStrSafe(temp, 16, &offset, "-%d", callerIdx);
            gcoOS_StrCatSafe(newVarName, __MAX_SYM_NAME_LENGTH__, temp);

            /* 1. put local variables to global scope if the caller is main
             * 2. put parameter variables to global scope */
            if (isCallerMainFunc || isParam)
            {
                errCode = VIR_Shader_AddSymbolWithName(pShader,
                                                   VIR_SYM_VARIABLE,
                                                   newVarName,
                                                   VIR_Shader_GetTypeFromId(pShader, oldTypeId),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &newVarSymId);
                ON_ERROR(errCode, "duplicate variable list");
            }
            else
            {
                /* make this variable as a local variable. */
                errCode = VIR_Function_AddLocalVar(pCallerFunc,
                                                   newVarName,
                                                   oldTypeId,
                                                   &newVarSymId);
                ON_ERROR(errCode, "duplicate variable list");
            }

            pNewVarSym = VIR_Function_GetSymFromId(pCallerFunc, newVarSymId);

            /* Copy some informations. */
            VIR_Symbol_SetPrecision(pNewVarSym, VIR_Symbol_GetPrecision(pOldSym));
            if (isCallerMainFunc || isParam)
            {
                /* Don't inherit the flag LOCAL. */
                VIR_Symbol_SetFlag(pNewVarSym, (VIR_Symbol_GetFlags(pOldSym) & ~VIR_SYMFLAG_LOCAL));
            }
            else
            {
                /* Don't inherit the flag LOCAL. */
                VIR_Symbol_SetFlag(pNewVarSym, VIR_Symbol_GetFlags(pOldSym));
            }
            /* Create new vreg symbol. */
            if (regCount > 0)
            {
                /* create temp registers in Shader */
                newVregId = VIR_Shader_NewVirRegId(pShader, regCount);
                VIR_Symbol_SetVariableVregIndex(pNewVarSym, newVregId);

                /* Update the reg. */
                for (i = 0; i < regCount; i++)
                {
                    VIR_VirRegId    tmpVregId;
                    VIR_Symbol      *pTempVirReg = gcvNULL;
                    VIR_Symbol      *pNewVirReg = gcvNULL;
                    VIR_SymId       newVirRegId;

                    tmpVregId = oldRegId + i;
                    pTempVirReg = VIR_Shader_FindSymbolByTempIndex(pShader, tmpVregId);

                    if (pTempVirReg == gcvNULL)
                    {
                        continue;
                    }

                    if (!vscHTBL_DirectTestAndGet(pTempSet, (void*)pTempVirReg, (void **)&pNewVirReg))
                    {
                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_VIRREG,
                                                       newVregId + i,
                                                       VIR_Symbol_GetType(pTempVirReg),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &newVirRegId);
                        ON_ERROR(errCode, "add vreg symbol");

                        pNewVirReg = VIR_Shader_GetSymFromId(pShader, newVirRegId);
                        VIR_Symbol_SetVregVariable(pNewVirReg, pNewVarSym);

                        /* Copy some informations. */
                        VIR_Symbol_SetPrecision(pNewVirReg, VIR_Symbol_GetPrecision(pTempVirReg));

                        /* save the new vreg to the table */
                        vscHTBL_DirectSet(pTempSet, (void*)pTempVirReg, (void*)pNewVirReg);

                        VIR_Symbol_SetIndexRange(pNewVirReg, VIR_Symbol_GetVregIndex(pNewVirReg) + indexRange - i);
                    }
                }

                VIR_Symbol_SetIndexRange(pNewVarSym, VIR_Symbol_GetVregIndex(pNewVarSym) + indexRange);
            }
        }

        /* save the new variable to the table */
        vscHTBL_DirectSet(pTempSet, (void*)pOldSym, (void*)pNewVarSym);
    }

OnError:
    return errCode;
}

static VSC_ErrCode
VSC_IL_DupVariableList(
    IN  VIR_Inliner     *pInliner,
    IN  VIR_Function    *pCallerFunc,
    IN  VIR_Function    *pCalleeFunc,
    IN  VIR_VariableIdList *pVarList,
    IN  gctUINT         callerIdx,
    IN  VIR_Instruction *pCallSiteInst,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_SymId           oldSymId = VIR_INVALID_ID;
    VIR_Symbol          *pOldSym = gcvNULL;
    gctUINT             i;

    for (i = 0; i < VIR_IdList_Count(pVarList); i++)
    {
        oldSymId = VIR_IdList_GetId(pVarList, i);
        pOldSym = VIR_Function_GetSymFromId(pCalleeFunc, oldSymId);

        errCode = VSC_IL_DupSingleVariable(pInliner,
                                           pCallerFunc,
                                           pCalleeFunc,
                                           pOldSym,
                                           callerIdx,
                                           pCallSiteInst,
                                           pTempSet);
        ON_ERROR(errCode, "duplicate the single variable.");
    }

OnError:
    return errCode;
}

/* if parameter has flag passbyRef, find its actual argument in caller function
 * and store <oldVirTempSym, actualVirTempSym> to pTempSet
 * otherwise, create a new temp variable for parameter
 */
VSC_ErrCode
VSC_IL_DupParamsAndLocalVars(
    IN  VIR_Inliner     *pInliner,
    IN  VIR_Function    *pCallerFunc,
    IN  VIR_Function    *pCalleeFunc,
    IN  gctUINT         callerIdx,
    IN  VIR_Instruction *pCallSiteInst,
    OUT VSC_HASH_TABLE  *pTempSet
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;

    /* Duplicate all paramters. */
    errCode = VSC_IL_DupVariableList(pInliner,
                                     pCallerFunc,
                                     pCalleeFunc,
                                     &pCalleeFunc->paramters,
                                     callerIdx,
                                     pCallSiteInst,
                                     pTempSet);
    ON_ERROR(errCode, "dupliate parameters");

    /* Duplicate all local variables. */
    errCode = VSC_IL_DupVariableList(pInliner,
                                     pCallerFunc,
                                     pCalleeFunc,
                                     &pCalleeFunc->localVariables,
                                     callerIdx,
                                     gcvNULL,
                                     pTempSet);
    ON_ERROR(errCode, "dupliate local variables");

OnError:
    return errCode;
}

/* ===========================================================================
   VSC_IL_InlineSingleFunction:
   Inline a functon to its caller
   ===========================================================================
*/
VSC_ErrCode VSC_IL_InlineSingleFunction(
    VIR_Inliner       *pInliner,
    VIR_Function      *pCallerFunc,
    VIR_Function      *pCalleeFunc)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);
    VIR_FUNC_BLOCK      *pCallerBLK = VIR_Function_GetFuncBlock(pCallerFunc);
    VIR_FUNC_BLOCK      *pCalleeBLK = VIR_Function_GetFuncBlock(pCalleeFunc);

    /* save the mapping between the old label id with the new one*/
    VSC_HASH_TABLE       *pLabelSet;

    /* save the jmp instruction whose label is not set the new
       one during DupInstruction, we need to set its label at the end */
    VSC_HASH_TABLE       *pJmpSet;

    /* save temp registers. */
    VSC_HASH_TABLE       *pTempSet;

    VSC_IL_INST_LIST     calleeInsts;

    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;
    gctUINT                      callerIdx;

    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;

    INST_LIST_INITIALIZE(&calleeInsts);

    /* add the callee instructions into a list */
    VIR_InstIterator_Init(&instIter, &pCalleeFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; pInst != gcvNULL;
        pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VSC_IL_INST_LIST_NODE   *node =
            (VSC_IL_INST_LIST_NODE*)vscMM_Alloc(VSC_IL_GetMM(pInliner),
            sizeof(VSC_IL_INST_LIST_NODE));
        node->inst = pInst;
        INST_LIST_ADD_NODE(&calleeInsts, node);
    }


    pLabelSet = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                (PFN_VSC_HASH_FUNC)vscHFUNC_Label, (PFN_VSC_KEY_CMP)vcsHKCMP_Label, 512);

    pJmpSet = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                vscHFUNC_Default, vscHKCMP_Default, 512);

    pTempSet = (VSC_HASH_TABLE*)vscHTBL_Create(VSC_IL_GetMM(pInliner),
                vscHFUNC_Default, vscHKCMP_Default, 512);

    /* go through all the caller to find the right one */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pCallerBLK->dgNode.succList);
    pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
    for (; pCallerEdge != gcvNULL;
        pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
    {
        if (CG_EDGE_GET_TO_FB(pCallerEdge) == pCalleeBLK)
        {
            /* for all the call sites */
            for (callerIdx = 0;
                 callerIdx < vscSRARR_GetElementCount(&pCallerEdge->callSiteArray);
                 callerIdx ++)
            {
                VIR_Instruction *pCallSiteInst =
                    *(VIR_Instruction**) vscSRARR_GetElement(&pCallerEdge->callSiteArray, callerIdx);
                VIR_Instruction *pCallSitePrevInst = VIR_Inst_GetPrev(pCallSiteInst);

                VSC_IL_INST_LIST_ITERATOR calleeInstsIter;
                VSC_IL_INST_LIST_NODE     *calleeInstsNode;

                VSC_HASH_ITERATOR jmpSetIter;
                VSC_DIRECT_HNODE_PAIR jmpSetPair;
                VIR_Label       *callLabel = gcvNULL;

                vscHTBL_Reset(pLabelSet);
                vscHTBL_Reset(pJmpSet);
                vscHTBL_Reset(pTempSet);

                /*
                ** Generate different arguments and local variables for every single call site to make
                ** DU info simple, so we can do more optimizations in further process.
                */

                /* Copy arguments and local variables to the caller function. */
                retValue = VSC_IL_DupParamsAndLocalVars(pInliner,
                                                        pCallerFunc,
                                                        pCalleeFunc,
                                                        callerIdx,
                                                        pCallSiteInst,
                                                        pTempSet);

                /* change the call instruction to a LABEL instruction */
                if (VIR_Inst_GetOpcode(pCallSiteInst) == VIR_OP_CALL)
                {
                    gctUINT offset = 0;
                    gctCHAR labelName[__MAX_SYM_NAME_LENGTH__];
                    VIR_LabelId labelId;

                    VIR_Inst_SetOpcode(pCallSiteInst, VIR_OP_LABEL);

                    gcoOS_PrintStrSafe(labelName,
                                       gcmSIZEOF(labelName),
                                       &offset,
                                       "%s_%s_%u_%u",
                                       VIR_Shader_GetSymNameString(pCallerFunc->hostShader,
                                       VIR_Function_GetSymbol(pCallerFunc)),
                                       VIR_Shader_GetSymNameString(pCallerFunc->hostShader,
                                       VIR_Function_GetSymbol(pCalleeFunc)),
                                       VSC_IL_GetPassData(pInliner)->passIndex,
                                       callerIdx);

                    retValue = VIR_Function_AddLabel(pCallerFunc,
                                                    labelName,
                                                    &labelId);
                    callLabel = VIR_Function_GetLabelFromId(pCallerFunc, labelId);
                    callLabel->defined = pCallSiteInst;
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(pCallSiteInst), callLabel);
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

                /* go through all the instructions in the callee,
                   except the last instruction, which should be RET */
                VSC_IL_INST_LIST_ITERATOR_INIT(&calleeInstsIter, &calleeInsts);
                calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_LAST(&calleeInstsIter);
                gcmASSERT (VIR_Inst_GetOpcode(calleeInstsNode->inst) == VIR_OP_RET);

                for(calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_FIRST(&calleeInstsIter);
                    calleeInstsNode != VSC_IL_INST_LIST_ITERATOR_LAST(&calleeInstsIter);
                    calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_NEXT(&calleeInstsIter))
                {
                    VIR_Instruction *pNewInst = gcvNULL;
                    VIR_Link        *pNewLink = gcvNULL;

                    pInst = calleeInstsNode->inst;

                    /* change the RET instruction to a JMP to LABEL instruction
                       that is coming from the call instruction */
                    if(VIR_Inst_GetOpcode(calleeInstsNode->inst) == VIR_OP_RET)
                    {
                        retValue = VIR_Function_NewInstruction(pCallerFunc,
                            VIR_OP_JMP, VIR_TYPE_FLOAT32, &pNewInst);
                        gcmASSERT(callLabel != gcvNULL);
                        VIR_Operand_SetLabel(VIR_Inst_GetDest(pNewInst), callLabel);
                        VIR_Function_NewLink(pCallerFunc, &pNewLink);
                        VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)pNewInst);
                        VIR_Link_AddLink(&(callLabel->referenced), pNewLink);
                    }
                    else
                    {
                        retValue = VSC_IL_DupInstruction(pInliner, pCalleeFunc, pCallerFunc,
                            pInst, callerIdx, &pNewInst, pLabelSet, pJmpSet, pTempSet);
                    }

                    vscBILST_InsertBefore((VSC_BI_LIST *)&pCallerFunc->instList,
                             (VSC_BI_LIST_NODE *) pCallSiteInst,
                             (VSC_BI_LIST_NODE *) pNewInst);
                }

                /* set the label for the jmp instruction */
                vscHTBLIterator_Init(&jmpSetIter, pJmpSet);
                for(jmpSetPair = vscHTBLIterator_DirectFirst(&jmpSetIter);
                    IS_VALID_DIRECT_HNODE_PAIR(&jmpSetPair); jmpSetPair = vscHTBLIterator_DirectNext(&jmpSetIter))
                {
                    VIR_Instruction* jmpInst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&jmpSetPair);
                    VIR_Label       *label = VIR_Operand_GetLabel(VIR_Inst_GetDest(jmpInst));
                    VIR_Label       *newLabel = gcvNULL;
                    VIR_Link        *pNewLink = gcvNULL;

                    if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) label, (void **)&newLabel) != gcvTRUE)
                    {
                        gcmASSERT(gcvFALSE);
                    }
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(jmpInst), newLabel);
                    VIR_Function_NewLink(pCallerFunc, &pNewLink);
                    VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)jmpInst);
                    VIR_Link_AddLink(&(newLabel->referenced), pNewLink);
                }

                /* Update call parameter assignments. */
                retValue = VIR_Shader_UpdateCallParmAssignment(pShader,
                                                               pCalleeFunc,
                                                               pCalleeFunc,
                                                               pCallerFunc,
                                                               pCallSitePrevInst ? VIR_Inst_GetNext(pCallSitePrevInst) : gcvNULL,
                                                               pCallSiteInst,
                                                               gcvTRUE,
                                                               pTempSet);
            }
            /* move the kernel info to main if the callee is the currentKernel function
             * and change the currentKernel function to main */
            if (pCallerFunc == CG_GET_MAIN_FUNC(pCG) &&
                pCalleeFunc == VIR_Shader_GetCurrentKernelFunction(pShader))
            {
                pCallerFunc->flags |= VIR_FUNCFLAG_KERNEL_MERGED_MAIN;
                pCallerFunc->kernelInfo = pCalleeFunc->kernelInfo;
                pCallerFunc->kernelInfo->isMain = gcvTRUE;
                VIR_Shader_SetCurrentKernelFunction(pShader, pCallerFunc);
                pCalleeFunc->kernelInfo = gcvNULL;
            }
        }
    }

    /* update the call graph accordingly */
    vscDG_RemoveEdge((VSC_DIRECTED_GRAPH*)pCG,
        (VSC_DG_NODE*) VIR_Function_GetFuncBlock(pCallerFunc),
        (VSC_DG_NODE*) VIR_Function_GetFuncBlock(pCalleeFunc));

    INST_LIST_FINALIZE(&calleeInsts);
    vscHTBL_Destroy(pLabelSet);
    vscHTBL_Destroy(pJmpSet);
    vscHTBL_Destroy(pTempSet);

    /* dump */
    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE))
    {
        VIR_LOG(pDumper, "Caller [%s] after inlining callee [%s]\n\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pCallerFunc)),
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pCalleeFunc)));
        VIR_Function_Dump(pDumper, pCallerFunc);
        VIR_LOG_FLUSH(pDumper);
    }

    return retValue;
}

/* ===========================================================================
   VSC_IL_SelectInlineFunctions:
   Select Inline functon candidates
   ===========================================================================
*/
VSC_ErrCode VSC_IL_SelectInlineFunctions(
    VIR_Inliner       *pInliner,
    VIR_Function      *pFunc,
    gctBOOL            AlwayInline
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VSC_HASH_TABLE      *pCandidates = VSC_IL_GetCandidates(pInliner);
    VIR_FUNC_BLOCK      *pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
    gctINT              instCount = VIR_Function_GetInstCount(pFunc);
    gctINT              leftBudget;
    gctINT              callSites = 0;
    gctBOOL             smallfuncBody = gcvFALSE;
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CG_EDGE*                 pEdge;

    if (pFunc == CG_GET_MAIN_FUNC(pCG))
    {
        /* main func */
        VSC_IL_SetInlineBudget(pInliner,
                VSC_IL_GetInlineBudget(pInliner) - instCount);
    }
    else
    {
        /* go through all its callers */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
        pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
        for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            /* callsite only stores at the succ edge */
            pEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pEdge);
            callSites += vscSRARR_GetElementCount(&pEdge->callSiteArray);
        }
        /* real uesful instruction counts of current function need to substract MOV instruction used for paraments and extra 2 (nop and ret) */
        smallfuncBody = (gctINT)(instCount - 2 - pFunc->paramters.count) < (gctINT)(2 + pFunc->paramters.count);
        instCount  = (instCount - 1) * (callSites - 1);
        leftBudget = VSC_IL_GetInlineBudget(pInliner) - instCount + pFunc->paramters.count ;

        if (AlwayInline || callSites == 1)
        {
            vscHTBL_DirectSet(pCandidates, (void*) pFunc, gcvNULL);
            VSC_IL_SetInlineBudget(pInliner, leftBudget);
        }
        else if (smallfuncBody)
        {
            vscHTBL_DirectSet(pCandidates, (void*) pFunc, gcvNULL);
            VSC_IL_SetInlineBudget(pInliner, leftBudget);
        }
        /* only use the code size as the heuristic for now */
        else if (leftBudget > 0)
        {
            vscHTBL_DirectSet(pCandidates, (void*) pFunc, gcvNULL);
            VSC_IL_SetInlineBudget(pInliner, leftBudget);
        }
    }

    return retValue;
}

/* ===========================================================================
   VSC_IL_OptimizeCallStackDepth:
   Inline functon that are exceed the max call stack depth.
   ===========================================================================
*/
VSC_ErrCode VSC_IL_OptimizeCallStackDepth(
    VIR_Inliner       *pInliner,
    gctBOOL           *changed)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);

    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;
    VIR_FUNC_BLOCK      *pFuncBlk;

    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);
    gctBOOL             codeChanged = gcvFALSE;

    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(VSC_IL_GetMM(pInliner),
        sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);

    /* bottom up traverse of the call graph */
    vscDG_PstOrderTraversal(&pCG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        VSC_ADJACENT_LIST_ITERATOR   edgeIter;
        VIR_CG_EDGE*                 pEdge;
        gctUINT                      origCallDepth;

        pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
        pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
        origCallDepth = pFuncBlk->maxCallDepth;

        while (pFuncBlk->maxCallDepth > VSC_MAX_CALL_STACK_DEPTH)
        {
            /* dump */
            if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                VSC_OPTN_ILOptions_TRACE))
            {
                VIR_LOG(pDumper, "\nOptimize Call Stack Depth for Function:\t[%s] \n",
                    VIR_Shader_GetSymNameString(pShader,
                    VIR_Function_GetSymbol(pFunc)));
                VIR_LOG_FLUSH(pDumper);
            }

            /* go through all its callers */
            VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
            pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
            for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
            {
                VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
                if (callerBlk->maxCallDepth == pFuncBlk->maxCallDepth - 1)
                {
                    retValue = VSC_IL_InlineSingleFunction(pInliner, callerBlk->pVIRFunc, pFunc);
                    codeChanged = gcvTRUE;
                }
            }

            _VSC_IL_UpdateMaxCallDepth(pInliner, pFuncBlk);

            if (pFuncBlk->maxCallDepth == 0 &&
                (origCallDepth != 0 || VSC_IL_GetRemoveUnusedFunctions(pInliner)))
            {
                /* remove this function block from the call graph */
                vscVIR_RemoveFuncBlockFromCallGraph(pCG, pFuncBlk, gcvTRUE);
            }
        }
    }
    if (changed)
    {
        *changed = codeChanged;
    }
    vscMM_Free(VSC_IL_GetMM(pInliner), ppFuncBlkRPO);

    return retValue;
}

/* ===========================================================================
   VSC_IL_TopDownInline:
   top down traverse the call graph to select the inline candidates and
   perform the inline
   ===========================================================================
*/
VSC_ErrCode VSC_IL_TopDownInline(
    VIR_Inliner       *pInliner)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);
    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);

    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;
    gctBOOL             inlineAlwaysInlineFuncOnly = VSC_IL_GetCheckAlwaysInlineOnly(pInliner);

    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(VSC_IL_GetMM(pInliner),
        sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);

    vscDG_PstOrderTraversal(&pCG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    /* Seperate the hueristic with the transformation */

    if (gcmOPT_DisableOPTforDebugger())
    {
        /* 1. select the recompiler functions into the worklist in debug mode. */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;

            /* Check recompiler functions. */
            if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_RECOMPILER) ||
                VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_RECOMPILER_STUB))
            {
                if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                    VSC_OPTN_ILOptions_TRACE))
                {
                    VIR_LOG(pDumper, "\nSelect Inline Candidate for Function:\t[%s]\n",
                        VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
                    VIR_LOG_FLUSH(pDumper);
                }
                VSC_IL_SelectInlineFunctions(pInliner, pFunc, gcvTRUE);
            }
        }
    }
    else
    {
        /* 1. select the ALWAYSINLINE/recompile-stub function into the worklist first. */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;

            /* Check ALWAYSINLINE functions. */
            if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_ALWAYSINLINE) ||
                VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_RECOMPILER_STUB))
            {
                if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                    VSC_OPTN_ILOptions_TRACE))
                {
                    VIR_LOG(pDumper, "\nSelect Inline Candidate for Function:\t[%s]\n",
                        VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
                    VIR_LOG_FLUSH(pDumper);
                }
                VSC_IL_SelectInlineFunctions(pInliner, pFunc, gcvTRUE);
            }
        }

        /* 2. select the inline candidates into the worklist */
        if (!inlineAlwaysInlineFuncOnly)
        {
            for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
            {
                pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;

                /* Skip ALWAYSINLINE and NOINLIEN, recompile stub functions. */
                if (!VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_ALWAYSINLINE) &&
                    ! VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_RECOMPILER_STUB) &&
                    !VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_NOINLINE))
                {
                    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                        VSC_OPTN_ILOptions_TRACE))
                    {
                        VIR_LOG(pDumper, "\nSelect Inline Candidate for Function:\t[%s]\n",
                            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
                        VIR_LOG_FLUSH(pDumper);
                    }
                    VSC_IL_SelectInlineFunctions(pInliner, pFunc, gcvFALSE);
                }
            }
        }
    }

    /* 3. do the inline transformation */
    {
        VSC_HASH_TABLE        *pCandidates = VSC_IL_GetCandidates(pInliner);
        VSC_ADJACENT_LIST_ITERATOR   edgeIter;
        VIR_CG_EDGE*                 pEdge;

        /* from bottom up to perform inline */
        vscDG_PstOrderTraversal(&pCG->dgGraph,
                                VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                                gcvTRUE,
                                gcvTRUE,
                                (VSC_DG_NODE**)ppFuncBlkRPO);

        /* 1. select the inline candidates into the worklist */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            VIR_Function *pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
            VIR_FUNC_BLOCK  *pFuncBlk = gcvNULL;
            gctUINT origCallDepth;

            if (vscHTBL_DirectTestAndGet(pCandidates, (void*) pFunc, gcvNULL))
            {
                pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
                origCallDepth = pFuncBlk->maxCallDepth;

                if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                    VSC_OPTN_ILOptions_TRACE))
                {
                    VIR_LOG(pDumper, "\nPerform Inline for Function:\t[%s]\n",
                        VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
                    VIR_LOG_FLUSH(pDumper);
                }

                /* go through all its callers */
                VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
                pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
                for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
                {
                    VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
                    retValue = VSC_IL_InlineSingleFunction(pInliner, callerBlk->pVIRFunc, pFunc);
                }

                _VSC_IL_UpdateMaxCallDepth(pInliner, pFuncBlk);

                if (pFuncBlk->maxCallDepth == 0 &&
                    (origCallDepth != 0 || VSC_IL_GetRemoveUnusedFunctions(pInliner)))
                {
                    /* remove this function block from the call graph */
                    vscVIR_RemoveFuncBlockFromCallGraph(pCG, pFuncBlk, gcvTRUE);
                }
            }
        }
    }

    vscMM_Free(VSC_IL_GetMM(pInliner), ppFuncBlkRPO);

    return retValue;
}

static VSC_ErrCode VSC_IL_CleanupLables(
    VIR_Inliner       *pInliner)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = pInliner->pShader;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function        *func = func_node->function;
        VIR_InstIterator    inst_iter;
        VIR_Instruction     *inst, *next_inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
        while (inst != gcvNULL)
        {
            next_inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);

            if (VIR_Inst_GetOpcode(inst) == VIR_OP_LABEL &&
                VIR_Inst_GetJmpLabel(inst)->referenced == gcvNULL)
            {
                VIR_Pass_DeleteInstruction(func, inst, gcvNULL);
            }

            inst = next_inst;
        }
    }

    /* Renumber instruction ID. */
    VIR_Shader_RenumberInstId(pShader);

    return errCode;
}

static void _VSC_IL_Init(
    VIR_Inliner         *pInliner,
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_OPTN_ILOptions  *pOptions,
    VIR_Dumper          *pDumper,
    VIR_CALL_GRAPH      *pCG,
    VSC_MM*             pMM,
    VSC_IL_PASS_DATA    *pILPassData)
{
    gctUINT             maxInstCount = 0;

    VSC_IL_SetShader(pInliner, pShader);
    VSC_IL_SetHwCfg(pInliner, pHwCfg);
    VSC_IL_SetDumper(pInliner, pDumper);
    VSC_IL_SetOptions(pInliner, pOptions);
    VSC_IL_SetCallGraph(pInliner, pCG);
    VSC_IL_SetPassData(pInliner, pILPassData);

    /* initialize the memory pool */
    pInliner->pMM = pMM;

    pInliner->pCandidates = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                vscHFUNC_Default, vscHKCMP_Default, 512);

    if (pHwCfg->hwFeatureFlags.instBufferUnified)
    {
        maxInstCount = pHwCfg->maxTotalInstCount;
    }
    else
    {
        switch (VIR_Shader_GetKind(pShader))
        {
        case VIR_SHADER_VERTEX:
        case VIR_SHADER_TESSELLATION_CONTROL:
        case VIR_SHADER_TESSELLATION_EVALUATION:
        case VIR_SHADER_GEOMETRY:
            maxInstCount = pHwCfg->maxVSInstCount;
            break;

        case VIR_SHADER_FRAGMENT:
            maxInstCount = pHwCfg->maxPSInstCount;
            break;
        case VIR_SHADER_COMPUTE:
            maxInstCount = pHwCfg->hwFeatureFlags.hasThreadWalkerInPS
                ? pHwCfg->maxPSInstCount : pHwCfg->maxVSInstCount;
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    /* adjust the inline budget based on inline level */
    switch (VSC_OPTN_ILOptions_GetInlineLevel(pOptions))
    {
    case VSC_OPTN_ILOptions_LEVEL0:
        maxInstCount = 0;
        break;
    case VSC_OPTN_ILOptions_LEVEL1:
        if (pHwCfg->hwFeatureFlags.hasInstCache)
        {
            maxInstCount = 0x7fffffff;
        }
        break;
    case VSC_OPTN_ILOptions_LEVEL2:
        break;
    case VSC_OPTN_ILOptions_LEVEL3:
        /* If HW has iCache, maybe we can assume more budget. */
        if (pHwCfg->hwFeatureFlags.hasInstCache)
        {
            maxInstCount = 2 * maxInstCount;
        }
        break;
    case VSC_OPTN_ILOptions_LEVEL4:
        /* If HW has iCache, maybe we can assume more budget. */
        if (pHwCfg->hwFeatureFlags.hasInstCache)
        {
            maxInstCount = 0x7fffffff;
        }
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VSC_IL_SetInlineBudget(pInliner, maxInstCount);
    VSC_IL_SetCheckAlwaysInlineOnly(pInliner, pILPassData->bCheckAlwaysInlineOnly);
    VSC_IL_SetRemoveUnusedFunctions(pInliner, VIR_Shader_CanRemoveUnusedFunctions(pShader));

    if (VSC_OPTN_ILOptions_GetInlineLevel(pOptions) == VSC_OPTN_ILOptions_LEVEL1)
    {
        VSC_IL_SetCheckAlwaysInlineOnly(pInliner, gcvTRUE);
    }
}

static void _VSC_IL_Final(
    VIR_Inliner           *pInliner)
{
    VSC_IL_SetShader(pInliner, gcvNULL);
    VSC_IL_SetOptions(pInliner, gcvNULL);
    VSC_IL_SetDumper(pInliner, gcvNULL);
    VSC_IL_SetCallGraph(pInliner, gcvNULL);
}

DEF_QUERY_PASS_PROP(VSC_IL_PerformOnShader)
{
    pPassProp->supportedLevels = (VSC_PASS_LEVEL)(VSC_PASS_LEVEL_ML | VSC_PASS_LEVEL_LL);
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_INLINER;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCg = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_IL_PerformOnShader)
{
    return gcvTRUE;
}

/* ===========================================================================
   VSC_IL_PerformOnShader:
   inliner on shader
   ===========================================================================
*/
VSC_ErrCode VSC_IL_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Inliner         inliner;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_CALL_GRAPH      *pCG = pPassWorker->pCallGraph;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VSC_OPTN_ILOptions  *pOption = (VSC_OPTN_ILOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);
    VSC_IL_PASS_DATA    ILPassData = { 0, gcvFALSE };
    gctBOOL             codeChanged = gcvFALSE;
    if (pPassWorker->basePassWorker.pPassSpecificData != gcvNULL)
    {
        ILPassData = *(VSC_IL_PASS_DATA *)pPassWorker->basePassWorker.pPassSpecificData;
    }

    _VSC_IL_Init(&inliner, pShader, &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                 pOption, pDumper, pCG, pPassWorker->basePassWorker.pMM, &ILPassData);

    /* dump */
    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Inliner", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    if (countOfFuncBlk != 0)
    {
        gctBOOL     changed = gcvFALSE;
        /* inline functons that are exceed the max call stack depth*/
        if (!VSC_IL_GetCheckAlwaysInlineOnly(&inliner) &&
            VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetHeuristics(pOption),
            VSC_OPTN_ILOptions_CALL_DEPTH))
        {
            retValue = VSC_IL_OptimizeCallStackDepth(&inliner, &changed);
            codeChanged |= changed;
        }

        /* top down traverse the call graph */
        if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetHeuristics(pOption),
            VSC_OPTN_ILOptions_TOP_DOWN))
        {
            retValue = VSC_IL_TopDownInline(&inliner);
            codeChanged |= (HTBL_GET_ITEM_COUNT(VSC_IL_GetCandidates(&inliner)) > 0);
        }
    }

    /* clean up the unused labels to avoid unnecessary basic blocks */
    retValue = VSC_IL_CleanupLables(&inliner);

    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        /* Inliner invalids CFG, thus dumping IR with CFG may have issue */
        gctBOOL oldCFGFlag = pShader->dumper->invalidCFG;
        pShader->dumper->invalidCFG = gcvTRUE;
        VIR_Shader_Dump(gcvNULL, "Shader after Inliner", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
        pShader->dumper->invalidCFG = oldCFGFlag;
    }
    if (codeChanged)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCg = gcvTRUE;
    }
    _VSC_IL_Final(&inliner);

    return retValue;
}

