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


#include "gc_vsc.h"
#include "vir/analysis/gc_vsc_vir_ssa.h"

VSC_ErrCode vscVIR_Transform2SSA(VIR_Shader* pShader)
{
    VIR_Shader_SetFlag(pShader, VIR_SHFLAG_BY_SSA_FORM);

    return VSC_ERR_NONE;
}

static VSC_ErrCode
_ReplaceOperandSymbolWithAlias(
    IN  VIR_Shader              *pShader,
    IN  VIR_Operand             *Operand
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    gctUINT                      i;

    if (Operand == gcvNULL)
    {
        return errCode;
    }

    if (VIR_Operand_isParameters(Operand))
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(Operand);
        for (i = 0; i < parm->argNum; i++)
        {
            _ReplaceOperandSymbolWithAlias(pShader, parm->args[i]);
        }
    }
    else if (VIR_Operand_isTexldParm(Operand))
    {
        VIR_Operand *texldOperand = Operand;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            _ReplaceOperandSymbolWithAlias(pShader, VIR_Operand_GetTexldModifier(texldOperand, i));
        }
    }

    /* Ignore non-symbol operands. */
    if (!VIR_Operand_isSymbol(Operand))
    {
        return errCode;
    }

    {
        VIR_Symbol* opndSym = VIR_Operand_GetSymbol(Operand);
        VIR_SymAliasTable* symAliasTable = VIR_Shader_GetSymAliasTable(pShader);

        if(!VIR_SymAliasTable_IsEmpty(symAliasTable))
        {
            VIR_Symbol* opndSymAlias = VIR_SymAliasTable_GetAlias(symAliasTable, opndSym);

            if(opndSymAlias)
            {
                VIR_Operand_SetSym(Operand, opndSymAlias);
            }
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_TransformFromSSA(VIR_Shader* pShader)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;

    if (!VIR_SymAliasTable_IsEmpty(VIR_Shader_GetSymAliasTable(pShader)))
    {
        VIR_FuncIterator             func_iter;
        VIR_FunctionNode            *func_node;
        gctUINT                      i;

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

        for (func_node = VIR_FuncIterator_First(&func_iter);
             func_node != gcvNULL;
             func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function        *func = func_node->function;
            VIR_Instruction     *inst = func->instList.pHead;

            for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
            {
                if(VIR_Inst_GetOpcode(inst) == VIR_OP_PHI)
                {
                    VIR_Function_RemoveInstruction(func, inst);
                    continue;
                }

                errCode = _ReplaceOperandSymbolWithAlias(pShader, VIR_Inst_GetDest(inst));
                CHECK_ERROR(errCode, "_ReplaceOperandSymbolWithAlias failed.");

                for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                {
                    errCode = _ReplaceOperandSymbolWithAlias(pShader, VIR_Inst_GetSource(inst, i));
                    CHECK_ERROR(errCode, "_ReplaceOperandSymbolWithAlias failed.");
                }
            }
        }

        VIR_Shader_DestroySymAliasTable(pShader);
    }

    VIR_Shader_ClrFlag(pShader, VIR_SHFLAG_BY_SSA_FORM);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Transform from SSA.", pShader, gcvTRUE);
    }

    return VSC_ERR_NONE;
}

VSC_ErrCode vscVIR_TransformFromSpvSSA(VIR_Shader* pShader)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_FuncIterator             func_iter;
    VIR_FunctionNode            *func_node;
    VSC_HASH_TABLE*              labelEndMap;
    VSC_HASH_TABLE*              movToDupMap;
    VSC_HASH_TABLE*              movFromDupMap;

    labelEndMap = vscHTBL_Create(&pShader->pmp.mmWrapper, vscHFUNC_Default, vscHKCMP_Default, 512);
    movToDupMap = vscHTBL_Create(&pShader->pmp.mmWrapper, vscHFUNC_Default, vscHKCMP_Default, 512);
    movFromDupMap = vscHTBL_Create(&pShader->pmp.mmWrapper, vscHFUNC_Default, vscHKCMP_Default, 512);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;
        VIR_Instruction* inst = func->instList.pHead;
        VIR_Label* label = gcvNULL;
        VIR_Label* lastLabel = gcvNULL;

        for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
        {
            if(VIR_Inst_GetOpcode(inst) == VIR_OP_LABEL)
            {
                label = VIR_Operand_GetLabel(VIR_Inst_GetDest(inst));
                if(lastLabel)
                {
                    vscHTBL_DirectSet(labelEndMap, (void*)lastLabel, VIR_Inst_GetPrev(inst));
                }
                lastLabel = label;
            }
            if(VIR_Inst_GetNext(inst) == gcvNULL && label)
            {
                vscHTBL_DirectSet(labelEndMap, (void*)label, inst);
            }
        }

        if(label)
        {
            for(inst = func->instList.pHead; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
            {
                if(VIR_Inst_GetOpcode(inst) == VIR_OP_SPV_PHI)
                {
                    VIR_Operand* dest = VIR_Inst_GetDest(inst);
                    VIR_Symbol* destSym = VIR_Operand_GetSymbol(dest);
                    VIR_Symbol* dupDestSym = gcvNULL;
                    VIR_SymId dupDestSymId;
                    VIR_Operand* source = VIR_Inst_GetSource(inst, 0);
                    VIR_PhiOperandArray* phiOperandArray = VIR_Operand_GetPhiOperands(source);
                    gctUINT i;

                    /* Duplicate the variable and update the type as the dest operand type id.*/
                    VIR_Shader_DuplicateVariableFromSymbol(pShader, destSym, &dupDestSymId);
                    dupDestSym = VIR_Shader_GetSymFromId(pShader, dupDestSymId);
                    VIR_Symbol_SetTypeId(dupDestSym, VIR_Operand_GetTypeId(dest));

                    for(i = 0; i < VIR_PhiOperandArray_GetCount(phiOperandArray); i++)
                    {
                        VIR_Instruction* prevMovToDupInst = gcvNULL;
                        VIR_Instruction* movToDupInst = gcvNULL;
                        VIR_Instruction* prevMovFromDupInst = gcvNULL;
                        VIR_Instruction* movFromDupInst = gcvNULL;
                        VIR_PhiOperand* phiOperand = VIR_PhiOperandArray_GetNthOperand(phiOperandArray, i);

                        /* insert mov to dup inst */
                        if(!vscHTBL_DirectTestAndGet(movToDupMap, (void*)VIR_PhiOperand_GetLabel(phiOperand), (void**)&prevMovToDupInst))
                        {
                            VIR_Instruction* labelEnd = (VIR_Instruction*)vscHTBL_DirectGet(labelEndMap, (void*)VIR_PhiOperand_GetLabel(phiOperand));

                            while(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(labelEnd)))
                            {
                                labelEnd = VIR_Inst_GetPrev(labelEnd);
                            }

                            prevMovToDupInst = labelEnd;
                            /* here we need to exclude the branch condition setting instruction. need to be modified */
                            if(VIR_Inst_GetOpcode(VIR_Inst_GetNext(labelEnd)) == VIR_OP_JMPC)
                            {
                                VIR_Instruction* jmpc = VIR_Inst_GetNext(labelEnd);

                                if(VIR_Inst_GetOpcode(labelEnd) == VIR_OP_SET &&
                                   (VIR_Operand_Identical(VIR_Inst_GetDest(inst), VIR_Inst_GetSource(labelEnd, 0), pShader) ||
                                    VIR_Operand_Identical(VIR_Inst_GetDest(inst), VIR_Inst_GetSource(labelEnd, 1), pShader)) &&
                                   (VIR_Operand_Identical(VIR_Inst_GetDest(labelEnd), VIR_Inst_GetSource(jmpc, 0), pShader) ||
                                    VIR_Operand_Identical(VIR_Inst_GetDest(labelEnd), VIR_Inst_GetSource(jmpc, 1), pShader))
                                  )
                                {
                                    prevMovToDupInst = VIR_Inst_GetPrev(labelEnd);
                                }
                                else if(VIR_Inst_GetOpcode(labelEnd) == VIR_OP_CSELECT &&
                                   (VIR_Operand_Identical(VIR_Inst_GetDest(labelEnd), VIR_Inst_GetSource(jmpc, 0), pShader) ||
                                    VIR_Operand_Identical(VIR_Inst_GetDest(labelEnd), VIR_Inst_GetSource(jmpc, 1), pShader))
                                  )
                                {
                                    VIR_Instruction* preLabelEnd = VIR_Inst_GetPrev(labelEnd);

                                    if(VIR_Inst_GetOpcode(preLabelEnd) == VIR_OP_COMPARE &&
                                       (VIR_Operand_Identical(VIR_Inst_GetDest(inst), VIR_Inst_GetSource(preLabelEnd, 0), pShader) ||
                                        VIR_Operand_Identical(VIR_Inst_GetDest(inst), VIR_Inst_GetSource(preLabelEnd, 1), pShader)) &&
                                        VIR_Operand_Identical(VIR_Inst_GetDest(preLabelEnd), VIR_Inst_GetSource(labelEnd, 0), pShader))
                                    {
                                        prevMovToDupInst = VIR_Inst_GetPrev(preLabelEnd);
                                    }
                                }
                            }
                        }

                        VIR_Function_AddInstructionAfter(func, VIR_OP_MOV, VIR_Operand_GetTypeId(dest), prevMovToDupInst, gcvTRUE, &movToDupInst);
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(movToDupInst), func, VIR_Symbol_GetIndex(dupDestSym));
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(movToDupInst), VIR_Operand_GetEnable(dest));

                        VIR_Operand_Copy(VIR_Inst_GetSource(movToDupInst, 0), VIR_PhiOperand_GetValue(phiOperand));

                        vscHTBL_DirectSet(movToDupMap, (void*)VIR_PhiOperand_GetLabel(phiOperand), (void*)movToDupInst);

                        /* insert mov from dup inst */
                        if(!vscHTBL_DirectTestAndGet(movFromDupMap, (void*)VIR_PhiOperand_GetLabel(phiOperand), (void**)&prevMovFromDupInst))
                        {
                            prevMovFromDupInst = movToDupInst;
                        }

                        VIR_Function_AddInstructionAfter(func, VIR_OP_MOV, VIR_Operand_GetTypeId(dest), prevMovFromDupInst, gcvTRUE, &movFromDupInst);
                        VIR_Operand_Copy(VIR_Inst_GetDest(movFromDupInst), dest);

                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(movFromDupInst, 0), func, VIR_Symbol_GetIndex(dupDestSym));
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(movFromDupInst, 0), VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dest)));

                        vscHTBL_DirectSet(movFromDupMap, (void*)VIR_PhiOperand_GetLabel(phiOperand), (void*)movFromDupInst);
                    }

                    VIR_Function_RemoveInstruction(func, inst);
                }
            }

            vscHTBL_Reset(labelEndMap);
            vscHTBL_Reset(movToDupMap);
            vscHTBL_Reset(movFromDupMap);
        }
    }

    vscHTBL_Destroy(labelEndMap);
    vscHTBL_Destroy(movToDupMap);
    vscHTBL_Destroy(movFromDupMap);
    VIR_Shader_ClrFlag(pShader, VIR_SHFLAG_BY_SPV_SSA_FORM);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Transform from Spirv SSA.", pShader, gcvTRUE);
    }

    return errCode;
}

