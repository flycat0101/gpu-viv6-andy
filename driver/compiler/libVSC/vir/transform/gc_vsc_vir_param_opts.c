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


#include "vir/transform/gc_vsc_vir_param_opts.h"

void VSC_PARAM_optimization_Init(
    IN OUT VSC_PARAM_optimization *po,
    IN VIR_Shader *shader,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *candidateFuncs,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments,
    IN VIR_Operand *argMmPtr,
    IN VIR_Dumper *dumper,
    IN VSC_OPTN_ParamOptOptions *options,
    IN VIR_DEF_USAGE_INFO *duInfo
    )
{
    VSC_PARAM_optimization_SetShader(po, shader);
    VSC_PARAM_optimization_SetDumper(po, dumper);
    VSC_PARAM_optimization_SetCandidateFuncs(po, candidateFuncs);
    VSC_PARAM_optimization_SetArgumentPtr(po, argMmPtr);
    VSC_PARAM_optimization_SetlongSizeArguments(po, longSizeArguments);
    VSC_PARAM_optimization_SetThreshold(po, options->longArrayThreshold);
    VSC_PARAM_optimization_SetOptions(po, options);
    VSC_PARAM_optimization_SetDuInfo(po, duInfo);
    VSC_PARAM_optimization_SetCfgChanged(po, gcvFALSE);
}

void VSC_PARAM_optimization_Final(
    IN OUT VSC_PARAM_optimization* po
    )
{
    VSC_PARAM_optimization_SetShader(po, gcvNULL);
    VSC_PARAM_optimization_SetDumper(po, gcvNULL);
    VSC_PARAM_optimization_SetCandidateFuncs(po, gcvNULL);
    VSC_PARAM_optimization_SetArgumentPtr(po, gcvNULL);
    VSC_PARAM_optimization_SetlongSizeArguments(po, gcvNULL);
    VSC_PARAM_optimization_SetThreshold(po, 0);
    VSC_PARAM_optimization_SetDuInfo(po, gcvNULL);
}

/*******************************************************************************
                            getInstDestVregIndex
********************************************************************************

    Get the destination register index of one instruction.

    INPUT:
        VIR_Instruction *inst
            Pointer to a VIR_Instruction.

    INPUT & OUTPUT:
        gctUINT *destVregIndex
            Destination oprand register index.

*********************************************************************************/
VSC_ErrCode getInstDestVregIndex(
    IN VIR_Instruction *inst,
    IN OUT gctUINT *destVregIndex
    )
{
    VIR_Operand* Operand = gcvNULL;
    VIR_Symbol *sym = gcvNULL;

    if(VIR_Inst_GetDest(inst) != gcvNULL)
    {
        Operand = VIR_Inst_GetDest(inst);
        gcmASSERT(VIR_Operand_isLvalue(Operand));
        if (VIR_Operand_GetOpKind(Operand) == VIR_OPND_SYMBOL)
        {
            sym = Operand->u.n.u1.sym;
            *destVregIndex = VIR_Symbol_GetVregIndex(sym);
            return VSC_ERR_NONE;
        }
        return VSC_ERR_INVALID_TYPE;
    }
    return VSC_ERR_INVALID_ARGUMENT;
}

/*******************************************************************************
                            getInstSrcVregIndex
********************************************************************************

    Get the source register index of one instruction.

    INPUT:
        VIR_Instruction *inst
            Pointer to a VIR_Instruction.
        gctUINT8 srcIndex
            Source index of instruction. (0/1/2).

    INPUT & OUTPUT:
        gctUINT *srcVregIndex
            Source oprand register index.
*********************************************************************************/
VSC_ErrCode getInstSrcVregIndex(
    IN VIR_Instruction *inst,
    IN gctUINT8 srcIndex,
    IN OUT gctUINT *srcVregIndex
    )
{
    VIR_Operand* Operand = gcvNULL;
    VIR_Symbol *sym = gcvNULL;

    if(VIR_Inst_GetDest(inst) != gcvNULL)
    {
        Operand = VIR_Inst_GetSource(inst, srcIndex);
        if (VIR_Operand_GetOpKind(Operand) == VIR_OPND_SYMBOL)
        {
            sym = Operand->u.n.u1.sym;
            *srcVregIndex = VIR_Symbol_GetVregIndex(sym);
            return VSC_ERR_NONE;
        }
        return VSC_ERR_INVALID_TYPE;
    }
    return VSC_ERR_INVALID_ARGUMENT;
}

/*******************************************************************************
                            getParamByIdx
********************************************************************************

    Get the parameter from optimizable parameter list according to current vreg
    index.

    INPUT:
        VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams
            Pointer to a optimizable parameter list.
        gctUINT paramCurrentIndex
            current virtual register index.

    Return:
        corresponding parameter pointer
*********************************************************************************/
LONG_SIZE_PARAMETER *getParamByIdx(
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams,
    IN gctUINT paramCurrentIndex
    )
{
    gctSIZE_T i;
    for (i = 0; i < vscSRARR_GetElementCount(longSizeParams); i++)
    {
        LONG_SIZE_PARAMETER *parameter = (LONG_SIZE_PARAMETER*)vscSRARR_GetElement(longSizeParams, i);
        if (parameter->regStartIndex <= paramCurrentIndex &&
            (parameter->paramArraySize + parameter->regStartIndex) > paramCurrentIndex)
        {
            return parameter;
        }
    }
    return gcvNULL;
}

/*******************************************************************************
                            getArgByIdx
********************************************************************************

    Get the argument from optimizable argument list according to current vreg
    index.

    INPUT:
        VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArgs
            Pointer to a optimizable argument list.
        gctUINT argCurrentIndex
            current virtual register index.

    Return:
        corresponding argument pointer
*********************************************************************************/
LONG_SIZE_ARGUMENT *getArgByIdx(
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArgs,
    IN gctUINT argCurrentIndex
    )
{
    gctSIZE_T i;
    for (i = 0; i < vscSRARR_GetElementCount(longSizeArgs); i++)
    {
        LONG_SIZE_ARGUMENT *argument = (LONG_SIZE_ARGUMENT*)vscSRARR_GetElement(longSizeArgs, i);
        if (argument->regStartIndex <= argCurrentIndex &&
            (argument->regStartIndex + argument->argArraySize) > argCurrentIndex)
        {
            return argument;
        }
    }
    return gcvNULL;
}

/*******************************************************************************
                            _VSC_SIMP_OptimizeParamInCallee
********************************************************************************

    Perform optimization of one parameter on one candidate callee function.

    INPUT:
        VSC_PARAM_optimization *paramOptimizer
            Pointer to a struct holding needed information of optimization.
        LONG_SIZE_PARAMETER *parameter
            pointer to the parameter being optimized.
        VIR_Shader *shader
            pointer to the shader holding this function.

    INPUT & OUTPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            pointer to the passworker which control this pass.
        CANDIDATE_FUNCTION *candidateFunc
            pointer to the callee function being optimized.
        VIR_DEF_USAGE_INFO *duInfo
            pointer to the duInfo structure.
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_OptimizeParamInCallee(
    IN OUT VSC_SH_PASS_WORKER *pPassWorker,
    IN OUT CANDIDATE_FUNCTION *candidateFunc,
    IN VSC_PARAM_optimization *paramOptimizer,
    IN LONG_SIZE_PARAMETER *parameter,
    IN VIR_Shader *shader,
    IN OUT VIR_DEF_USAGE_INFO *duInfo
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctSIZE_T i;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;
    VIR_Instruction *currentinst = gcvNULL;
    VIR_Instruction *mulInst = gcvNULL;

    VIR_Instruction *loadInst = gcvNULL;
    VIR_Function *currentFunc = candidateFunc->functionBlock->pVIRFunc;
    VIR_SymTable *SymTable = &currentFunc->symTable;
    gctUINT regStartIndex = parameter->regStartIndex;
    gctUINT regEndIndex = regStartIndex + parameter->paramArraySize;
    gctUINT src0VregIndex = 0;
    gctUINT sizeofParamArrayItemVal = 0;
    gctUINT offsetVal = 0;
    VIR_SymId newTempSymId = VIR_INVALID_ID;
    VIR_Operand *paramPtr = parameter->paramPtr;
    VIR_Operand* inputParam = gcvNULL;
    VIR_Operand *src0 = gcvNULL;
    VIR_Operand* dest = gcvNULL;
    VIR_Operand* loadOffset = gcvNULL;
    VIR_Operand* ldarrOffset = gcvNULL;
    VIR_Operand* sizeofParamArrayItem = gcvNULL;
    VIR_Operand* movaDest = gcvNULL;
    VIR_Operand* mulSrc0 = gcvNULL;
    VIR_VirRegId tempRegId = VIR_INVALID_ID;
    VIR_Symbol *sym = gcvNULL;

    VIR_Symbol *paramPtrSym = VIR_Operand_GetSymbol(paramPtr);
    VIR_Symbol *inputParamSym = gcvNULL;
    VIR_Symbol *loadOffsetSym = gcvNULL;
    VIR_Symbol *ldarrOffsetSym = gcvNULL;
    VIR_Symbol *movaDestSym = gcvNULL;


    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctSTRING symName = "spillMemAddress";

    /*Remove unused input parameters.*/
    VIR_VariableIdList *paramIdList = &currentFunc->paramters;
    for(i = 0; i < VIR_IdList_Count(paramIdList); i++)
    {
        VIR_Id id = VIR_IdList_GetId(paramIdList, i);
        VIR_Symbol *sym = VIR_GetSymFromId(SymTable, id);
        VIR_SymbolKind symKind = VIR_Symbol_GetKind(sym);

        if (symKind == VIR_SYM_VARIABLE &&
            sym->u2.tempIndex >= regStartIndex && sym->u2.tempIndex < regEndIndex)
        {
            VIR_IdList_DeleteByIndex(paramIdList, i);
            i--;
        }
    }
    /*Replace input parameters with memory address pointer.*/
    {
        /*Create a new operand to hold input pointer.*/
        errCode =VIR_Function_NewOperand(currentFunc, &inputParam);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }

        VIR_Function_AddSymbolWithName(
            currentFunc,
            VIR_SYM_VARIABLE,
            symName,
            VIR_Shader_GetTypeFromId(shader, VIR_TYPE_UINT32),
            VIR_STORAGE_INPARM,
            &newTempSymId);
        CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
        inputParamSym = VIR_GetFuncSymFromId(currentFunc, newTempSymId);
        VIR_Symbol_SetVariableVregIndex(inputParamSym, VIR_Symbol_GetVregIndex(paramPtrSym));
        VIR_Operand_SetOpKind(inputParam, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(inputParam, inputParamSym);
        VIR_Operand_SetPrecision(inputParam, VIR_PRECISION_HIGH);
        VIR_Operand_SetTypeId(inputParam, VIR_TYPE_UINT32);

        VIR_IdList_Add(paramIdList, inputParamSym->index);
    }
    /*Process all instructions that use optimizable long parameter.*/
    VIR_InstIterator_Init(&instIter, &currentFunc->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

        switch(opcode)
        {
        case VIR_OP_MOV:
            if(getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex))
            {
                continue;
            }
            src0 = VIR_Inst_GetSource(inst, VIR_Operand_Src0);
            sym = VIR_Operand_GetSymbol(src0);
            /*Check if this mov inst uses the input parameter as sorurce.*/
            for (i = regStartIndex; i < regEndIndex; i++)
            {
                if (src0VregIndex == i)
                {
                    /*Add one load inst to replace original inst.*/
                    errCode = VIR_Function_AddInstructionAfter(currentFunc, VIR_OP_LOAD_S, VIR_TYPE_FLOAT32,
                        inst, gcvTRUE, &loadInst);
                    if(errCode)
                    {
                        return errCode;
                    }
                    dest = VIR_Inst_GetDest(inst);

                    /*Get offset to base memory address.*/
                    sizeofParamArrayItemVal = VIR_Type_GetTypeByteSize(shader, VIR_Symbol_GetType(sym));
                    offsetVal = (src0VregIndex - regStartIndex) * sizeofParamArrayItemVal;
                    errCode = VIR_Function_NewOperand(currentFunc, &loadOffset);
                    if (errCode != VSC_ERR_NONE)
                    {
                        return errCode;
                    }
                    VIR_Operand_SetImmediateUint(loadOffset, offsetVal);

                    VIR_Inst_CopyDest(loadInst, dest, gcvFALSE);
                    VIR_Inst_CopySource(loadInst, VIR_Operand_Src0, paramPtr, gcvFALSE);
                    VIR_Inst_CopySource(loadInst, VIR_Operand_Src1, loadOffset, gcvFALSE);
                    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(loadInst, VIR_Operand_Src0),
                                            VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(paramPtr)));
                    /*dump*/
                    if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                            VSC_OPTN_ParamOptOptions_TRACE))
                    {
                        VIR_LOG(paramOptimizer->dumper, "\n[PAOPT]Added one LOAD_S inst:\n");
                        VIR_Inst_Dump(paramOptimizer->dumper, loadInst);
                        VIR_LOG_FLUSH(paramOptimizer->dumper);
                    }

                    VIR_Pass_RemoveInstruction(currentFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                    inst = loadInst;
                }
            }
            break;
        case VIR_OP_LDARR:
            errCode= getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex);
            if(errCode)
            {
                return errCode;
            }
            src0 = VIR_Inst_GetSource(inst, VIR_Operand_Src0);
            sym = VIR_Operand_GetSymbol(src0);
            /*Check if this ldarr inst uses the input parameter as sorurce.*/
            if (src0VregIndex == regStartIndex)
            {
                /*Add one MUL inst to compute offset for load inst*/
                errCode = VIR_Function_AddInstructionAfter(currentFunc, VIR_OP_MUL, VIR_TYPE_FLOAT32,
                    inst, gcvTRUE, &mulInst);
                if(errCode)
                {
                    return errCode;
                }
                ldarrOffset = VIR_Inst_GetSource(inst, VIR_Operand_Src1);
                ldarrOffsetSym = VIR_Operand_GetSymbol(ldarrOffset);
                /*Search backward to find ldarroffset in MOVA inst.*/
                currentinst = inst;
                while (inst != gcvNULL)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Prev(&instIter);
                    if (inst == gcvNULL || VIR_Inst_GetOpcode(inst) != VIR_OP_MOVA)
                    {
                        continue;
                    }
                    movaDest = VIR_Inst_GetDest(inst);
                    movaDestSym = VIR_Operand_GetSymbol(movaDest);
                    if (VIR_Symbol_GetVregIndex(movaDestSym) == VIR_Symbol_GetVregIndex(ldarrOffsetSym))
                    {
                        mulSrc0 = VIR_Inst_GetSource(inst, VIR_Operand_Src0);

                        /*remove MOVA inst.*/
                        VIR_Pass_RemoveInstruction(currentFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                        break;
                    }
                }
                /*Move inst iterator back to current inst.*/
                while (inst != currentinst)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                }

                /*Allocate a new temp register to hold load inst's offset.*/
                tempRegId = VIR_Shader_NewVirRegId(shader, 1);
                errCode = VIR_Function_AddSymbol(currentFunc,
                    VIR_SYM_VIRREG,
                    tempRegId,
                    VIR_Shader_GetTypeFromId(shader, VIR_TYPE_FLOAT32),
                    VIR_STORAGE_REGISTER,
                    &newTempSymId);
                CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
                loadOffsetSym = VIR_GetFuncSymFromId(currentFunc, newTempSymId);
                /*Creat a new operand to hold offset for load inst.*/
                errCode =VIR_Function_NewOperand(currentFunc, &loadOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetOpKind(loadOffset, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(loadOffset, VIR_Operand_GetTypeId(mulSrc0));
                VIR_Operand_SetSym(loadOffset, loadOffsetSym);
                VIR_Operand_SetLvalue(loadOffset, 1);
                VIR_Operand_SetEnable(loadOffset, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(loadOffset)));
                VIR_Operand_SetPrecision(loadOffset, VIR_PRECISION_HIGH);
                /*Creat a new operand to hold byte size of parameter for mul inst.*/
                errCode =VIR_Function_NewOperand(currentFunc, &sizeofParamArrayItem);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                sizeofParamArrayItemVal = VIR_Type_GetTypeByteSize(shader, VIR_Symbol_GetType(sym));
                VIR_Operand_SetImmediateUint(sizeofParamArrayItem, sizeofParamArrayItemVal);

                VIR_Inst_CopyDest(mulInst, loadOffset, gcvFALSE);
                VIR_Inst_CopySource(mulInst, VIR_Operand_Src0, sizeofParamArrayItem, gcvFALSE);
                VIR_Inst_CopySource(mulInst, VIR_Operand_Src1, mulSrc0, gcvFALSE);
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Added one MUL inst:\n");
                    VIR_Inst_Dump(paramOptimizer->dumper, mulInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }


                /*Add one load inst to replace orignal LDARR inst.*/
                errCode = VIR_Function_AddInstructionAfter(currentFunc, VIR_OP_LOAD_S, VIR_TYPE_FLOAT32,
                    mulInst, gcvTRUE, &loadInst);
                if(errCode)
                {
                    return errCode;
                }
                dest = VIR_Inst_GetDest(inst);

                VIR_Inst_CopyDest(loadInst, dest, gcvFALSE);
                VIR_Inst_CopySource(loadInst, VIR_Operand_Src0, paramPtr, gcvFALSE);
                VIR_Inst_CopySource(loadInst, VIR_Operand_Src1, loadOffset, gcvFALSE);
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(loadInst, VIR_Operand_Src0),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(paramPtr)));
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(loadInst, VIR_Operand_Src1),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(loadOffset)));
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Added one LOAD_S inst: \n");
                    VIR_Inst_Dump(paramOptimizer->dumper, loadInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }

                VIR_Pass_RemoveInstruction(currentFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                inst = loadInst;
            }
            break;
        default:
            break;
        }
    }
    return errCode;
}

/*******************************************************************************
                            _VSC_SIMP_OptimizeCaller
********************************************************************************

    Perform optimization of one parameter on one candidate callee function.

    INPUT:
        VSC_PARAM_optimization *paramOptimizer
            Pointer to a struct holding needed information of optimization.
        VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams
            pointer to the array holding parameters being optimized.
        VIR_Shader *shader
            pointer to the shader holding this function.

    INPUT & OUTPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            pointer to the passworker which control this pass.
        VIR_Function *callerFunc
            pointer to the caller of candidate function being optimized.
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_OptimizeCaller(
    IN OUT VSC_SH_PASS_WORKER *pPassWorker,
    IN OUT VIR_Function *callerFunc,
    IN VSC_PARAM_optimization *paramOptimizer,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams,
    IN VIR_Shader *shader
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;
    gctUINT destVregIndex = 0;
    gctUINT src0VregIndex = 0;
    gctUINT storeOffsetVal = 0;
    LONG_SIZE_ARGUMENT *currentArgument = gcvNULL;
    LONG_SIZE_PARAMETER *currentParameter = gcvNULL;
    VIR_Operand* sizeofArgArrayItem = gcvNULL;
    VIR_Operand* argStartAddressOffset = gcvNULL;
    VIR_Operand* loadOffset = gcvNULL;
    VIR_Operand* ldarrOffset = gcvNULL;
    VIR_Operand* storeOffset = gcvNULL;
    VIR_Operand* starrOffset = gcvNULL;
    VIR_Operand* madSrc1 = gcvNULL;
    VIR_Operand* movaDest = gcvNULL;
    VIR_Operand* dest = gcvNULL;
    VIR_Operand* buffer = gcvNULL;
    VIR_Operand* mmAddr = gcvNULL;
    VIR_Operand* val = gcvNULL;
    VIR_Operand* argBaseAddr = gcvNULL;
    VIR_Instruction *nextInst = gcvNULL;
    VIR_Instruction *storeInst = gcvNULL;
    VIR_Instruction *loadInst = gcvNULL;
    VIR_Instruction *madInst = gcvNULL;
    VIR_Instruction *addInst = gcvNULL;
    VIR_Instruction *movInst = gcvNULL;

    VIR_Instruction *currentinst = gcvNULL;
    VIR_VirRegId tempRegId = VIR_INVALID_ID;
    VIR_SymId newTempSymId = VIR_INVALID_ID;
    VIR_Symbol *loadOffsetSym = gcvNULL;
    VIR_Symbol *storeOffsetSym = gcvNULL;
    VIR_Symbol *ldarrOffsetSym = gcvNULL;
    VIR_Symbol *starrOffsetSym = gcvNULL;

    VIR_Symbol *movaDestSym = gcvNULL;



    VIR_Symbol *bufferSym = gcvNULL;
    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;

    {
        /*Allocate a new temp register to hold STORE_S inst's buffer.*/
        tempRegId = VIR_Shader_NewVirRegId(shader, 1);
        errCode = VIR_Function_AddSymbol(callerFunc,
            VIR_SYM_VIRREG,
            tempRegId,
            VIR_Shader_GetTypeFromId(shader, VIR_TYPE_FLOAT32),
            VIR_STORAGE_REGISTER,
            &newTempSymId);
        CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
        bufferSym = VIR_GetFuncSymFromId(callerFunc, newTempSymId);
        errCode = VIR_Function_NewOperand(callerFunc, &buffer);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
        VIR_Operand_SetOpKind(buffer, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(buffer, bufferSym);
        VIR_Operand_SetPrecision(buffer, VIR_PRECISION_HIGH);
    }
    /*Process all instructions that use optimizable long parameter.*/
    VIR_InstIterator_Init(&instIter, &callerFunc->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
         switch(opcode)
        {
         case VIR_OP_STARR:
             getInstDestVregIndex(inst, &destVregIndex);
             getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex);
             currentArgument = getArgByIdx(paramOptimizer->longSizeArguments, destVregIndex);
             /*Check if this inst defines one of the optimizable arguments*/
             if (currentArgument != gcvNULL && currentArgument->holderFunction == callerFunc)
             {
                 /*Add one MAD inst to compute offset for STORE_S inst*/
                errCode = VIR_Function_AddInstructionAfter(callerFunc, VIR_OP_MAD, VIR_TYPE_FLOAT32,
                    inst, gcvTRUE, &madInst);
                if(errCode)
                {
                    return errCode;
                }
                /*Search backward to find ldarr offset in MOVA inst.*/
                starrOffset = VIR_Inst_GetSource(inst, VIR_Operand_Src0);
                starrOffsetSym = VIR_Operand_GetSymbol(starrOffset);
                currentinst = inst;
                while (inst != gcvNULL)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Prev(&instIter);
                    if (inst == gcvNULL || VIR_Inst_GetOpcode(inst) != VIR_OP_MOVA)
                    {
                        continue;
                    }
                    movaDest = VIR_Inst_GetDest(inst);
                    movaDestSym = VIR_Operand_GetSymbol(movaDest);
                    if (VIR_Symbol_GetVregIndex(movaDestSym) == VIR_Symbol_GetVregIndex(starrOffsetSym))
                    {
                        madSrc1 = VIR_Inst_GetSource(inst, VIR_Operand_Src0);

                        /*remove MOVA inst.*/
                        VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                        break;
                    }
                }
                /*Move inst iterator back to current inst.*/
                while (inst != currentinst)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                }


                /*Allocate a new temp register to hold STORE_S inst's offset.*/
                tempRegId = VIR_Shader_NewVirRegId(shader, 1);
                errCode = VIR_Function_AddSymbol(callerFunc,
                    VIR_SYM_VIRREG,
                    tempRegId,
                    VIR_Shader_GetTypeFromId(shader, VIR_TYPE_FLOAT32),
                    VIR_STORAGE_REGISTER,
                    &newTempSymId);
                CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
                storeOffsetSym = VIR_GetFuncSymFromId(callerFunc, newTempSymId);
                errCode = VIR_Function_NewOperand(callerFunc, &storeOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetOpKind(storeOffset, VIR_OPND_SYMBOL);
                VIR_Operand_SetSym(storeOffset, storeOffsetSym);
                VIR_Operand_SetPrecision(storeOffset, VIR_PRECISION_HIGH);
                VIR_Operand_SetTypeId(storeOffset, VIR_Operand_GetTypeId(madSrc1));
                VIR_Operand_SetLvalue(storeOffset, 1);
                VIR_Operand_SetEnable(storeOffset, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(storeOffset)));

                errCode = VIR_Function_NewOperand(callerFunc, &sizeofArgArrayItem);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetImmediateUint(sizeofArgArrayItem, currentArgument->argTypeByteSize);

                errCode = VIR_Function_NewOperand(callerFunc, &argStartAddressOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetImmediateUint(argStartAddressOffset, currentArgument->offset);
                /*storeOffset = sizeofArgArrayItem * starrOffset + startOfArg*/
                VIR_Inst_CopyDest(madInst, storeOffset, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src0, sizeofArgArrayItem, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src1, madSrc1, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src2, argStartAddressOffset, gcvFALSE);
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Added one MAD inst: dest:\n");
                    VIR_Inst_Dump(paramOptimizer->dumper, madInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }


                 /*Add one store inst to replace orignal STARR inst.*/
                errCode = VIR_Function_AddInstructionAfter(callerFunc, VIR_OP_STORE_S, VIR_TYPE_FLOAT32,
                    madInst, gcvTRUE, &storeInst);
                if(errCode)
                {
                    return errCode;
                }

                mmAddr = paramOptimizer->argMmPtr;
                val = VIR_Inst_GetSource(inst, VIR_Operand_Src1);

                VIR_Inst_CopyDest(storeInst, buffer, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src0, mmAddr, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src1, storeOffset, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src2, val, gcvFALSE);

                VIR_Operand_SetTypeId(VIR_Inst_GetDest(storeInst), VIR_Operand_GetTypeId(val));

                VIR_Operand_SetEnable(VIR_Inst_GetDest(storeInst), VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(val)));

                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src0),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(mmAddr)));
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src1),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(storeOffset)));

                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Added one STORE_S inst:\n");
                    VIR_Inst_Dump(paramOptimizer->dumper, storeInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }

                VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                inst = storeInst;
                break;
             }
             else
             {
                 break;
             }
         case VIR_OP_LDARR:
             getInstDestVregIndex(inst, &destVregIndex);
             getInstSrcVregIndex(inst, 0, &src0VregIndex);
             currentArgument = getArgByIdx(paramOptimizer->longSizeArguments, src0VregIndex);
             /*Check if this inst uses one of the optimizable arguments*/
             if (currentArgument != gcvNULL && currentArgument->holderFunction == callerFunc)
             {
                 /*Add one MAD inst to compute offset for load*/
                errCode = VIR_Function_AddInstructionAfter(callerFunc, VIR_OP_MAD, VIR_TYPE_FLOAT32,
                    inst, gcvTRUE, &madInst);
                if(errCode)
                {
                    return errCode;
                }
                ldarrOffset = VIR_Inst_GetSource(inst, VIR_Operand_Src1);
                ldarrOffsetSym = VIR_Operand_GetSymbol(ldarrOffset);
                /*Search backward to find ldarroffset in MOVA inst.*/
                currentinst = inst;
                while (inst != gcvNULL)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Prev(&instIter);
                    if (inst == gcvNULL || VIR_Inst_GetOpcode(inst) != VIR_OP_MOVA)
                    {
                        continue;
                    }
                    movaDest = VIR_Inst_GetDest(inst);
                    movaDestSym = VIR_Operand_GetSymbol(movaDest);
                    if (VIR_Symbol_GetVregIndex(movaDestSym) == VIR_Symbol_GetVregIndex(ldarrOffsetSym))
                    {
                        madSrc1 = VIR_Inst_GetSource(inst, VIR_Operand_Src0);

                        /*remove MOVA inst.*/
                        VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                        break;
                    }
                }
                /*Move inst iterator back to current inst.*/
                while (inst != currentinst)
                {
                    inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                }
                /*Allocate a new temp register to hold load inst's offset.*/
                tempRegId = VIR_Shader_NewVirRegId(shader, 1);
                errCode = VIR_Function_AddSymbol(callerFunc,
                    VIR_SYM_VIRREG,
                    tempRegId,
                    VIR_Shader_GetTypeFromId(shader, VIR_TYPE_FLOAT32),
                    VIR_STORAGE_REGISTER,
                    &newTempSymId);
                CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
                loadOffsetSym = VIR_GetFuncSymFromId(callerFunc, newTempSymId);

                errCode = VIR_Function_NewOperand(callerFunc, &loadOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetOpKind(loadOffset, VIR_OPND_SYMBOL);
                VIR_Operand_SetTypeId(loadOffset, VIR_Operand_GetTypeId(madSrc1));
                VIR_Operand_SetSym(loadOffset, loadOffsetSym);
                VIR_Operand_SetLvalue(loadOffset, 1);
                VIR_Operand_SetEnable(loadOffset, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(loadOffset)));
                VIR_Operand_SetPrecision(loadOffset, VIR_PRECISION_HIGH);


                errCode = VIR_Function_NewOperand(callerFunc, &sizeofArgArrayItem);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetImmediateUint(sizeofArgArrayItem, currentArgument->argTypeByteSize);

                errCode = VIR_Function_NewOperand(callerFunc, &argStartAddressOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                VIR_Operand_SetImmediateUint(argStartAddressOffset, currentArgument->offset);
                /*loadOffset = sizeofArgArrayItem * ldarrOffset + startOfArg*/
                VIR_Inst_CopyDest(madInst, loadOffset, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src0, sizeofArgArrayItem, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src1, madSrc1, gcvFALSE);
                VIR_Inst_CopySource(madInst, VIR_Operand_Src2, argStartAddressOffset, gcvFALSE);
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Added one MAD inst:\n");
                    VIR_Inst_Dump(paramOptimizer->dumper, madInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }


                 /*Add one load inst to replace original ldarr inst.*/
                errCode = VIR_Function_AddInstructionAfter(callerFunc, VIR_OP_LOAD_S, VIR_TYPE_FLOAT32,
                    madInst, gcvTRUE, &loadInst);
                if(errCode)
                {
                    return errCode;
                }
                dest = VIR_Inst_GetDest(inst);
                mmAddr = paramOptimizer->argMmPtr;

                VIR_Inst_CopyDest(loadInst, dest, gcvFALSE);
                VIR_Inst_CopySource(loadInst, VIR_Operand_Src0, mmAddr, gcvFALSE);
                VIR_Inst_CopySource(loadInst, VIR_Operand_Src1, loadOffset, gcvFALSE);
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(loadInst, VIR_Operand_Src0),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(mmAddr)));
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(loadInst, VIR_Operand_Src1),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(loadOffset)));
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                    "\nA[PAOPT]dded one LOAD_S inst:\n");
                    VIR_Inst_Dump(paramOptimizer->dumper, loadInst);
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }

                VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                inst = loadInst;
                break;
             }
             else
             {
                 break;
             }
         default:
             getInstDestVregIndex(inst, &destVregIndex);
             currentArgument = getArgByIdx(paramOptimizer->longSizeArguments, destVregIndex);
             /*Check if this inst defines one of the optimizable arguments*/
             if (currentArgument != gcvNULL && currentArgument->holderFunction == callerFunc)
             {
                 /*Add one store inst to replace orignal inst.*/
                errCode = VIR_Function_AddInstructionAfter(callerFunc, VIR_OP_STORE_S, VIR_TYPE_FLOAT32,
                    inst, gcvTRUE, &storeInst);
                if(errCode)
                {
                    return errCode;
                }

                mmAddr = paramOptimizer->argMmPtr;
                val = VIR_Inst_GetDest(inst);
                /*Add a new operand to hold offset of store inst.*/
                errCode = VIR_Function_NewOperand(callerFunc, &storeOffset);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                /*Offset to base address = byte size of argument * index of vreg within argument + offset to start vreg index of argument*/
                storeOffsetVal = currentArgument->argTypeByteSize * (destVregIndex - currentArgument->regStartIndex) + currentArgument->offset;
                VIR_Operand_SetImmediateUint(storeOffset, storeOffsetVal);

                VIR_Inst_CopyDest(storeInst, buffer, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src0, mmAddr, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src1, storeOffset, gcvFALSE);
                VIR_Inst_CopySource(storeInst, VIR_Operand_Src2, val, gcvFALSE);

                VIR_Operand_SetTypeId(VIR_Inst_GetDest(storeInst), VIR_Operand_GetTypeId(val));

                VIR_Operand_SetEnable(VIR_Inst_GetDest(storeInst), VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(val)));

                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src0),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(mmAddr)));
                VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src2),
                                        VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(val)));


                inst = storeInst;
                break;
             }
             else
             {
                 break;
             }

         }
    }
    /*Go back to first inst to handle MOV insts.*/
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    argBaseAddr = paramOptimizer->argMmPtr;
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
        VIR_Operand *paramPtr = gcvNULL;
        gctUINT spillMemVRegIndex = 0;

        if (opcode == VIR_OP_MOV)
        {
            /*make sure this inst use optimizable parameter as its dest and optimizable argument as its source0.*/
            getInstDestVregIndex(inst, &destVregIndex);
            if(getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex))
            {
                continue;
            }
            currentParameter = getParamByIdx(longSizeParams, destVregIndex);
            currentArgument = getArgByIdx(paramOptimizer->longSizeArguments, src0VregIndex);
            if (currentParameter == gcvNULL)
            {
                continue;
            }
            paramPtr = currentParameter->paramPtr;
            VIR_Operand_SetLvalue(paramPtr, 1);
            VIR_Operand_SetEnable(paramPtr, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(paramPtr)));
            /*Insert ADD inst that copy argument pointer to parameter pointer.*/
            errCode = VIR_Function_AddInstructionBefore(callerFunc, VIR_OP_ADD, VIR_TYPE_FLOAT32,
                inst, gcvTRUE, &addInst);
            if(errCode)
            {
                return errCode;
            }
             /*Compute argument's memory address according to base address and offset.*/
            errCode = VIR_Function_NewOperand(callerFunc, &argStartAddressOffset);
            if (errCode != VSC_ERR_NONE)
            {
                return errCode;
            }
            VIR_Operand_SetImmediateUint(argStartAddressOffset, currentArgument->offset);
            VIR_Inst_CopyDest(addInst, paramPtr, gcvFALSE);
            VIR_Inst_CopySource(addInst, VIR_Operand_Src0, argBaseAddr, gcvFALSE);
            VIR_Inst_CopySource(addInst, VIR_Operand_Src1, argStartAddressOffset, gcvFALSE);

            /*Delete MOV insts that copy argument to parameter.*/
            while (inst != gcvNULL)
            {
                nextInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                if (VIR_Inst_GetOpcode(inst) != VIR_OP_MOV)
                {
                    inst = nextInst;
                    continue;
                }
                getInstDestVregIndex(inst, &destVregIndex);
                if(getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex))
                {
                    inst = nextInst;
                    continue;
                }
                if (destVregIndex >= currentParameter->regStartIndex
                    && destVregIndex < (currentParameter->regStartIndex + currentParameter->paramArraySize)
                    && src0VregIndex >= currentArgument->regStartIndex
                    && src0VregIndex < (currentArgument->regStartIndex + currentArgument->argArraySize))
                {
                    /*Need to store register content to memory if current argument is not defined in current function.*/
                    if (currentArgument->isDefinedInCaller == gcvFALSE)
                    {
                        /*Add one store inst to replace orignal inst.*/
                        errCode = VIR_Function_AddInstructionBefore(callerFunc, VIR_OP_STORE_S, VIR_TYPE_FLOAT32,
                            inst, gcvTRUE, &storeInst);
                        if(errCode)
                        {
                            return errCode;
                        }
                        mmAddr = paramOptimizer->argMmPtr;
                        val = VIR_Inst_GetSource(inst, VIR_Operand_Src0);

                        errCode = VIR_Function_NewOperand(callerFunc, &storeOffset);
                        if (errCode != VSC_ERR_NONE)
                        {
                            return errCode;
                        }
                        storeOffsetVal = currentArgument->argTypeByteSize * (src0VregIndex - currentArgument->regStartIndex) + currentArgument->offset;
                        VIR_Operand_SetImmediateUint(storeOffset, storeOffsetVal);

                        VIR_Inst_CopyDest(storeInst, buffer, gcvFALSE);
                        VIR_Inst_CopySource(storeInst, VIR_Operand_Src0, mmAddr, gcvFALSE);
                        VIR_Inst_CopySource(storeInst, VIR_Operand_Src1, storeOffset, gcvFALSE);
                        VIR_Inst_CopySource(storeInst, VIR_Operand_Src2, val, gcvFALSE);

                        VIR_Operand_SetTypeId(VIR_Inst_GetDest(storeInst), VIR_Operand_GetTypeId(val));

                        VIR_Operand_SetEnable(VIR_Inst_GetDest(storeInst), VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(val)));

                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src0),
                                                VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(mmAddr)));
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(storeInst, VIR_Operand_Src2),
                                                VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(val)));
                    }
                    VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                    inst = nextInst;
                }
                else
                {
                    inst = nextInst;
                }
            }
            inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
        }
        /*Handle the case that use input memory pointer as parameter of function call.*/
        else if (opcode == VIR_OP_LOAD_S)
        {
            getInstDestVregIndex(inst, &destVregIndex);
            if(getInstSrcVregIndex(inst, VIR_Operand_Src0, &src0VregIndex))
            {
                continue;
            }
            currentParameter = getParamByIdx(longSizeParams, destVregIndex);
            if (currentParameter == gcvNULL)
            {
                continue;
            }
            paramPtr = currentParameter->paramPtr;
            VIR_Operand_SetLvalue(paramPtr, 1);
            VIR_Operand_SetEnable(paramPtr, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(paramPtr)));
            /*Insert MOV inst that pass caller func's input memory address to callee.*/
            errCode = VIR_Function_AddInstructionBefore(callerFunc, VIR_OP_MOV, VIR_TYPE_FLOAT32,
                inst, gcvTRUE, &movInst);
            if(errCode)
            {
                return errCode;
            }
            VIR_Inst_CopyDest(movInst, paramPtr, gcvFALSE);
            VIR_Inst_CopySource(movInst, VIR_Operand_Src0, VIR_Inst_GetSource(inst, VIR_Operand_Src0), gcvFALSE);

            /*Delete LOAD_S insts that load argument to parameter.*/
            while (inst != gcvNULL)
            {
                nextInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                if (VIR_Inst_GetOpcode(inst) != VIR_OP_LOAD_S)
                {
                    inst = nextInst;
                    continue;
                }
                getInstDestVregIndex(inst, &destVregIndex);
                if(getInstSrcVregIndex(inst, VIR_Operand_Src0, &spillMemVRegIndex))
                {
                    continue;
                }
                if (destVregIndex >= currentParameter->regStartIndex
                    && destVregIndex < (currentParameter->regStartIndex + currentParameter->paramArraySize)
                    && src0VregIndex == spillMemVRegIndex)
                {
                    VIR_Pass_RemoveInstruction(callerFunc, inst, &VSC_PARAM_optimization_GetCfgChanged(paramOptimizer));
                    inst = nextInst;
                }
                else
                {
                    inst = nextInst;
                }
            }
            inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
        }

    }
    return errCode;
}



/*******************************************************************************
                            _VSC_SIMP_CheckParamModification
********************************************************************************

    Check whether one input parameter is modified in function.

    INPUT:
        VIR_Shader *Shader
            Pointer to a VIR Shader.
        VIR_SymTable *SymTable
            Pointer to a Symbol table.
        gctUINT regStartIndex
            Start register index of the input parameter.
        gctUINT regEndIndex
            End register index of the input parameter.
        VIR_Function *function
            Pointer to the function that accept the input parameter

    INPUT & OUTPUT:
        gctUINT8 *isVarModified
            Bool that specify whether parameter is modified in function.

*********************************************************************************/
VSC_ErrCode _VSC_SIMP_CheckParamModification(
    IN VIR_Shader *Shader,
    IN VIR_SymTable *SymTable,
    IN gctUINT regStartIndex,
    IN gctUINT regEndIndex,
    IN VIR_Function *function,
    IN OUT gctUINT8 *isVarModified
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctSIZE_T i = 0;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;

    VIR_InstIterator_Init(&instIter, &function->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    /*Iterate through all instructions to see if parameter is used as dest.*/
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
        gctUINT destVregIndex = 0;

        switch(opcode)
        {
        case VIR_OP_STARR:
            errCode = getInstDestVregIndex(inst, &destVregIndex);
            if(errCode)
            {
                return errCode;
            }
            if (destVregIndex == regStartIndex)
            {
                *isVarModified = gcvTRUE;
                return VSC_ERR_NONE;
            }
            break;
        default:
            if(getInstDestVregIndex(inst, &destVregIndex))
            {
                break;
            }
            for (i = regStartIndex; i < regEndIndex; i++)
            {
                if (destVregIndex == i)
                {
                    *isVarModified = gcvTRUE;
                    return VSC_ERR_NONE;
                }
            }
            break;
        }
    }
    return errCode;
}

/*******************************************************************************
                            _VSC_SIMP_GetParamArraySize
********************************************************************************

    Get the register end index of one parameter.

    INPUT:
        VIR_Shader *Shader
            Pointer to a VIR Shader.
        VIR_SymTable *SymTable
            Pointer to a Symbol table.
        gctUINT paramRegCurrentIndex
            current register index of the input parameter.
        VIR_Function *function
            Pointer to the function that uses this parameter as input.

    INPUT & OUTPUT:
        gctUINT *paramRegStartIndex
            Register start index of parameter.
        gctUINT *paramArraySize
            Size of parameter.
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_GetParamArraySize(
    IN VIR_Shader *Shader,
    IN VIR_SymTable *SymTable,
    IN gctUINT paramRegCurrentIndex,
    IN OUT gctUINT *paramRegStartIndex,
    IN VIR_Function *function,
    IN OUT gctUINT *paramArraySize
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T i = 0;
    gctUINT paramVregIndex = 0;
    gctUINT argRegCurrentIndex = 0;
    gctUINT argRegStartIndex = 0;
    gctUINT argRegCount = 0;
    VIR_VariableIdList* list = &Shader->variables;
    VIR_Function *callerFunc = gcvNULL;
    VSC_ADJACENT_LIST_ITERATOR edgeIter;
    VIR_CG_EDGE* pEdge;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;
    VIR_FUNC_BLOCK *callerBlk = gcvNULL;
    VIR_Type *type = gcvNULL;

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &function->pFuncBlock->dgNode.predList);
    pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    callerBlk = CG_EDGE_GET_TO_FB(pEdge);
    /*Get the first caller to search for argument size.*/
    callerFunc = callerBlk->pVIRFunc;

    VIR_InstIterator_Init(&instIter, &callerFunc->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV)
        {
            getInstDestVregIndex(inst, &paramVregIndex);
            if (paramVregIndex != paramRegCurrentIndex)
            {
                continue;
            }
            else
            {
                /*Get the corresponding argument array start index.*/
                errCode = getInstSrcVregIndex(inst, VIR_Operand_Src0, &argRegCurrentIndex);
                if(errCode)
                {
                    *paramArraySize = 1;
                    return VSC_ERR_NONE;
                }
                /*Search shader's variable list to find argment array size.*/
                for(i = 0; i < VIR_IdList_Count(list); i++)
                {
                    VIR_Id id = VIR_IdList_GetId(list, i);
                    VIR_Symbol* sym  = VIR_GetSymFromId(SymTable, id);
                    if(sym == gcvNULL)
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                    type = VIR_Symbol_GetType(sym);
                    if(type == gcvNULL)
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                    argRegStartIndex = sym->u2.tempIndex;
                    argRegCount = VIR_Type_GetVirRegCount(Shader, type, -1);
                    if (argRegStartIndex <= argRegCurrentIndex
                        && (argRegStartIndex + argRegCount) > argRegCurrentIndex)
                    {
                        *paramArraySize = argRegCount;
                        *paramRegStartIndex = paramRegCurrentIndex - (argRegCurrentIndex - argRegStartIndex);
                        return VSC_ERR_NONE;
                    }
                }
                /*If the argument is not in shader's variable list*/
                return VSC_ERR_NOT_FOUND;
            }
        }
    }
    return VSC_ERR_INVALID_ARGUMENT;
}

/*******************************************************************************
                            _VSC_SIMP_SelectCandidateFunction
********************************************************************************

    Optimize input parameters of one function.

    INPUT & OUTPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            Pointer to passWorker.
        VIR_Shader *Shader
            Pointer to a VIR shader.
        VIR_SymTable *SymTable
            Pointer to a Symbol table.
        VIR_FUNC_BLOCK *functionBlockn
            Pointer to a VIR function block.
        VSC_PARAM_optimization *paramOptimizer
            Pointer to paramOptimizer struct.

    ALGORITHM:
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_SelectCandidateFunction(
    IN OUT VSC_SH_PASS_WORKER *pPassWorker,
    IN OUT VIR_Shader *Shader,
    IN OUT VIR_SymTable *SymTable,
    IN OUT VIR_FUNC_BLOCK *functionBlock,
    IN OUT VSC_PARAM_optimization *paramOptimizer
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctSIZE_T i = 0;
    VIR_Function *function = functionBlock->pVIRFunc;
    gctUINT8 isOptimizable = gcvFALSE;
    gctUINT regStartIndex = 0;
    gctUINT regCurrentIndex = 0;
    gctUINT regEndIndex = 0;
    gctUINT paramArraySize = 0;
    gctUINT paramTypeArraySize = 0;
    CANDIDATE_FUNCTION candidateFunc;
    VIR_VirRegId tempRegId = VIR_INVALID_ID;
    VIR_SymId newTempSymId = VIR_INVALID_ID;
    VIR_VariableIdList *paramIdList = gcvNULL;
    VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParameters = gcvNULL;
    VIR_SymTable *FunSymTable = &function->symTable;
    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;

    gcmASSERT(Shader != gcvNULL &&
        SymTable != gcvNULL &&
        function != gcvNULL);

    paramIdList = &function->paramters;
    longSizeParameters = vscSRARR_Create(pPassWorker->basePassWorker.pMM, 0, sizeof(LONG_SIZE_PARAMETER), gcvNULL);
    candidateFunc.functionBlock = functionBlock;
    candidateFunc.longSizeParams = longSizeParameters;
    /*Iterate through function's parameter list to get parameter size.*/
    for(i = 0; i < VIR_IdList_Count(paramIdList); i++)
    {
        VIR_Id id = VIR_IdList_GetId(paramIdList, i);
        VIR_Symbol *paramSym = VIR_GetSymFromId(FunSymTable, id);
        VIR_Symbol *paramPtrSym = gcvNULL;
        VIR_SymbolKind symKind = VIR_Symbol_GetKind(paramSym);
        LONG_SIZE_PARAMETER currentParam;
        VIR_Operand *Operand;
        gctUINT8 isVarModified = gcvFALSE;

        if (symKind == VIR_SYM_VARIABLE)
        {
            regCurrentIndex = paramSym->u2.tempIndex;

            errCode = _VSC_SIMP_GetParamArraySize(Shader, SymTable, regCurrentIndex, &regStartIndex, function, &paramArraySize);
            /*If the parameter is not in shader's variable list*/
            if(errCode == VSC_ERR_NOT_FOUND)
            {
                if (VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(paramSym), -1) > 1)
                {
                    /*If the parameter is an array, we can get its size directly*/
                    paramArraySize = VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(paramSym), -1);
                    regStartIndex = regCurrentIndex;
                }
                else
                {
                    continue;
                }
            }
            else if (errCode != VSC_ERR_NONE)
            {
                continue;
            }

            if (paramArraySize < paramOptimizer->paramLengthThreshold)
            {
                continue;
            }

            regEndIndex = regStartIndex + paramArraySize;
            /*Check if this parameter is modified in function.*/
            errCode = _VSC_SIMP_CheckParamModification(Shader, SymTable, regStartIndex, regEndIndex, function, &isVarModified);
            if(errCode)
            {
                return errCode;
            }

            if (isVarModified == gcvFALSE)
            {
                errCode =VIR_Function_NewOperand(function, &Operand);
                if (errCode != VSC_ERR_NONE)
                {
                    return errCode;
                }
                /*Allocate a new temp register for each parameter to hold its memory pointer.*/
                tempRegId = VIR_Shader_NewVirRegId(Shader, 1);
                VIR_Function_AddSymbol(
                    function,
                    VIR_SYM_VIRREG,
                    tempRegId,
                    VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UINT32),
                    VIR_STORAGE_INPARM,
                    &newTempSymId);
                CHECK_ERROR(errCode, "VIR_Function_AddSymbol failed.");
                paramPtrSym = VIR_GetFuncSymFromId(function, newTempSymId);
                VIR_Operand_SetOpKind(Operand, VIR_OPND_SYMBOL);
                VIR_Operand_SetSym(Operand, paramPtrSym);
                VIR_Operand_SetPrecision(Operand, VIR_PRECISION_HIGH);
                VIR_Operand_SetTypeId(Operand, VIR_TYPE_UINT32);

                isOptimizable = gcvTRUE;
                currentParam.ownerFunc = function;
                currentParam.paramArraySize = paramArraySize;
                currentParam.regStartIndex = regStartIndex;
                paramTypeArraySize = VIR_Type_GetVirRegCount(Shader, VIR_Symbol_GetType(paramSym), -1);

                currentParam.paramTypeByteSize = VIR_Type_GetTypeByteSize(Shader, VIR_Symbol_GetType(paramSym)) / paramTypeArraySize;
                /*Get pointer to parameter.*/
                currentParam.paramPtr = Operand;
                vscSRARR_AddElement(candidateFunc.longSizeParams, &currentParam);
                /*dump*/
                if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                        VSC_OPTN_ParamOptOptions_TRACE))
                {
                    VIR_LOG(paramOptimizer->dumper,
                        "\n[PAOPT]Added one long size parameter with reg index: [%u - %u] to function: [%s] \n",
                        regStartIndex, regStartIndex + paramArraySize - 1,
                        VIR_Function_GetNameString(function));
                    VIR_LOG_FLUSH(paramOptimizer->dumper);
                }
            }
            /*Skip input param within same array.*/
            while (i < VIR_IdList_Count(paramIdList) - 1)
            {
                id = VIR_IdList_GetId(paramIdList, i + 1);
                paramSym = VIR_GetSymFromId(FunSymTable, id);
                if (paramSym->u2.tempIndex >= regStartIndex && paramSym->u2.tempIndex < (regStartIndex + paramArraySize))
                {
                    i++;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

    }
    if (isOptimizable)
    {
        vscSRARR_AddElement(paramOptimizer->candidateFuncs, &candidateFunc);
        /*dump*/
        if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                VSC_OPTN_ParamOptOptions_TRACE))
        {
            VIR_LOG(paramOptimizer->dumper, "\n[PAOPT]Added one candidate function:\t[%s] \n",
                VIR_Function_GetNameString(function));
            VIR_LOG_FLUSH(paramOptimizer->dumper);
        }

    }

    return VSC_ERR_NONE;
}

/*******************************************************************************
                            _VSC_SIMP_AddOneArgument
********************************************************************************

    Add one argument to optimizable long argument array.

    INPUT:
        LONG_SIZE_ARGUMENT *longSizeArg
            Pointer to the long size argument.

    INPUT & OUTPUT:
        VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments
            Pointer to argument array.
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_AddOneArgument(
    IN LONG_SIZE_ARGUMENT *longSizeArg,
    IN OUT VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments
    )
{
    gctSIZE_T i;
    gctUINT currentArgumentCount = vscSRARR_GetElementCount(longSizeArguments);
    /*Check array to see if the argument is already in it.*/
    for (i = 0; i < currentArgumentCount; i++)
    {
        LONG_SIZE_ARGUMENT *currentArg = (LONG_SIZE_ARGUMENT*)vscSRARR_GetElement(longSizeArguments, i);
        /*Argument already in the arry, no need to add.*/
        if (currentArg->regStartIndex == longSizeArg->regStartIndex)
        {
            return VSC_ERR_NONE;
        }
    }
    /*Compute offset for current argument*/
    if (currentArgumentCount == 0)
    {
        longSizeArg->offset = 0;
    }
    else
    {
        LONG_SIZE_ARGUMENT *prevArgument = (LONG_SIZE_ARGUMENT*)vscSRARR_GetElement(longSizeArguments, currentArgumentCount - 1);
        longSizeArg->offset = prevArgument->offset + prevArgument->argArraySize * prevArgument->argTypeByteSize;
    }
    vscSRARR_AddElement(longSizeArguments, longSizeArg);
    return VSC_ERR_NONE;
}

/*******************************************************************************
                            _Arg_CheckIsDefinedInCaller
********************************************************************************

    Check if one argument is defined in caller function.

    INPUT:
        VIR_Function *callerFunction
            Pointer to the caller function.

    INPUT & OUTPUT:
        LONG_SIZE_ARGUMENT *argument
            The optimizable argument need to be checked.
*********************************************************************************/
VSC_ErrCode _Arg_CheckIsDefinedInCaller(
    IN VIR_Function *callerFunction,
    IN OUT LONG_SIZE_ARGUMENT *argument
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctSIZE_T i;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;
    gctUINT destVregIndex = 0;
    argument->isDefinedInCaller = gcvFALSE;

    VIR_InstIterator_Init(&instIter, &callerFunction->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    /*Iterate through all insts in caller function to see if the argument is used as dest*/
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        if(getInstDestVregIndex(inst, &destVregIndex))
        {
            continue;
        }
        for (i = argument->regStartIndex; i < argument->regStartIndex + argument->argArraySize; i++)
        {
            if (destVregIndex == i)
            {
                argument->isDefinedInCaller = gcvTRUE;
                return errCode;
            }
        }
    }
    return errCode;
}

/*******************************************************************************
                            _VSC_SIMP_GetLongSizeArguments
********************************************************************************

    Get all optimizabe arguments according to corresponding parameter.

    INPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            Pointer to passWorker struct.
        VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams
            Pointer to long parameters array.

    INPUT & OUTPUT:
        VIR_FUNC_BLOCK *functionBlock
            Pointer to a function block
        VSC_PARAM_optimization *paramOptimizer
            Pointer to paramOptimizer stuct
*********************************************************************************/
VSC_ErrCode _VSC_SIMP_GetLongSizeArguments(
    IN VSC_SH_PASS_WORKER *pPassWorker,
    IN OUT VIR_FUNC_BLOCK *functionBlock,
    IN OUT VSC_PARAM_optimization *paramOptimizer,
    IN VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctSIZE_T i;
    VIR_InstIterator instIter;
    VIR_Instruction *inst = gcvNULL;
    VIR_Instruction *nxtInst = gcvNULL;
    gctUINT destVregIndex;
    gctUINT src0VregIndex ;

    LONG_SIZE_ARGUMENT longSizeArg;
    VIR_Function *currentFunc = functionBlock->pVIRFunc;
    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;
    /*Iterate through all insts and find inst that pass argument to parameter*/
    VIR_InstIterator_Init(&instIter, &currentFunc->instList);
    inst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; inst != gcvNULL; inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

        if (opcode == VIR_OP_MOV)
        {
            for (i = 0; i < vscSRARR_GetElementCount(longSizeParams); i++)
            {
                LONG_SIZE_PARAMETER *longSizeParam = (LONG_SIZE_PARAMETER*)vscSRARR_GetElement(longSizeParams, i);
                getInstDestVregIndex(inst, &destVregIndex);
                getInstSrcVregIndex(inst, 0, &src0VregIndex);
                if (destVregIndex >= longSizeParam->regStartIndex
                    && destVregIndex < (longSizeParam->regStartIndex + longSizeParam->paramArraySize))
                {
                    longSizeArg.holderFunction = currentFunc;
                    longSizeArg.offset = 0;
                    longSizeArg.regStartIndex = src0VregIndex - (destVregIndex - longSizeParam->regStartIndex);
                    longSizeArg.argArraySize = longSizeParam->paramArraySize;
                    longSizeArg.argTypeByteSize = (gctUINT8)longSizeParam->paramTypeByteSize;
                    _Arg_CheckIsDefinedInCaller(currentFunc, &longSizeArg);
                    /*Add argument.*/
                    errCode = _VSC_SIMP_AddOneArgument(&longSizeArg, paramOptimizer->longSizeArguments);
                    if(errCode)
                    {
                        return errCode;
                    }
                    /*dump*/
                    if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                            VSC_OPTN_ParamOptOptions_TRACE))
                    {
                        VIR_LOG(paramOptimizer->dumper,
                        "\n[PAOPT]Added one long size argument with reg index: [%u - %u]\n",
                        longSizeArg.regStartIndex,
                            longSizeArg.regStartIndex + longSizeArg.argArraySize - 1);
                        VIR_LOG_FLUSH(paramOptimizer->dumper);
                    }
                    /*Skip MOV inst that handles the same argument.*/
                    while (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV)
                    {
                        nxtInst = VIR_Inst_GetNext(inst);
                        if (VIR_Inst_GetOpcode(nxtInst) != VIR_OP_MOV)
                        {
                            break;
                        }
                        getInstDestVregIndex(nxtInst, &destVregIndex);
                        if (destVregIndex >= longSizeParam->regStartIndex
                            && destVregIndex < (longSizeParam->regStartIndex + longSizeParam->paramArraySize))
                        {
                            inst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return errCode;
}


/*******************************************************************************
                            VIR_PARAM_Optimization_PerformOnShader
********************************************************************************

    Optimize input parameters of all functions of one VIR shader.

    INPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            Pointer to optimization passworker.

    INPUT & OUTPUT:
        VSC_PARAM_optimization *paramOptimizer
            Pointer to paramOptimizer struct.

*********************************************************************************/
VSC_ErrCode VIR_PARAM_Optimization_PerformOnShader(
    IN VSC_SH_PASS_WORKER *pPassWorker,
    IN OUT VSC_PARAM_optimization *paramOptimizer
    )
{
    gctSIZE_T i, j;
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FUNC_BLOCK **ppFuncBlkRPO;
    gctUINT funcIdx;
    gctUINT longSizeArgCount = 0;
    gctUINT spillMmSize = 0;
    VSC_ADJACENT_LIST_ITERATOR edgeIter;
    VIR_CG_EDGE *pEdge;
    VIR_FUNC_BLOCK  *pFuncBlk = gcvNULL;
    VIR_SymTable *SymTable = &shader->symTable;
    VIR_CALL_GRAPH *pCG = pPassWorker->pCallGraph;
    gctUINT countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);
    VSC_SIMPLE_RESIZABLE_ARRAY *longSizeParams = gcvNULL;
    VIR_Uniform *spillMemUniform = gcvNULL;
    VIR_Symbol *uSymSpillMemAddr = gcvNULL;
    LONG_SIZE_ARGUMENT *lastArgument = gcvNULL;
    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_NameId nameId;
    gctBOOL    needBoundsCheck = (pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK) != 0;

    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(pPassWorker->basePassWorker.pMM,
        sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);

    vscDG_PstOrderTraversal(&pCG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);
    /*Select all optimizable functions.*/
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        pFuncBlk = ppFuncBlkRPO[funcIdx];

        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
        pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);

        if (pEdge != gcvNULL)
        {
            errCode = _VSC_SIMP_SelectCandidateFunction(pPassWorker, shader, SymTable, pFuncBlk, paramOptimizer);
            if(errCode)
            {
                return errCode;
            }
        }

    }
    /*End Phase if there is no candidate function.*/
    if (vscSRARR_GetElementCount(paramOptimizer->candidateFuncs) == 0)
    {
        pPassWorker->pResDestroyReq->s.bDuUnmodified = 1;
        pPassWorker->pResDestroyReq->s.bRdFlowUnmodified = 1;
        return errCode;
    }
    shader->hasRegisterSpill = gcvTRUE;
    /*Get all long size arguments.*/
    for (i = 0; i < vscSRARR_GetElementCount(paramOptimizer->candidateFuncs); i++)
    {
        CANDIDATE_FUNCTION *candidateFunc = (CANDIDATE_FUNCTION*)vscSRARR_GetElement(paramOptimizer->candidateFuncs, i);
        pFuncBlk = candidateFunc->functionBlock;
        longSizeParams = candidateFunc->longSizeParams;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
        pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
        for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
            _VSC_SIMP_GetLongSizeArguments(pPassWorker, callerBlk, paramOptimizer, longSizeParams);
        }
    }

    /*Creat operand of argument start address.*/
    spillMemUniform = VIR_Shader_GetTempRegSpillAddrUniform(shader, needBoundsCheck);
    if (spillMemUniform == gcvNULL)
    {
        return VSC_ERR_OUT_OF_MEMORY;
    }
    uSymSpillMemAddr = VIR_Shader_GetSymFromId(shader, spillMemUniform->sym);
    errCode = VIR_Shader_AddString(shader, "threadSpillAddressPointer", &nameId);
    CHECK_ERROR(errCode, "AddString");
    VIR_Symbol_SetName(uSymSpillMemAddr, nameId);

    VIR_Operand_SetOpKind(paramOptimizer->argMmPtr, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(paramOptimizer->argMmPtr, uSymSpillMemAddr);
    VIR_Operand_SetTypeId(paramOptimizer->argMmPtr, VIR_TYPE_UINT32);
    VIR_Operand_SetLvalue(paramOptimizer->argMmPtr, 0);
    VIR_Operand_SetSwizzle(paramOptimizer->argMmPtr, VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(paramOptimizer->argMmPtr)));

    /*Set spill memory size.*/
    longSizeArgCount = vscSRARR_GetElementCount(paramOptimizer->longSizeArguments);
    lastArgument = (LONG_SIZE_ARGUMENT*)vscSRARR_GetElement(paramOptimizer->longSizeArguments, longSizeArgCount - 1);
    spillMmSize = lastArgument->offset + lastArgument->argArraySize * lastArgument->argTypeByteSize;
    shader->vidmemSizeOfSpill += spillMmSize;

    /*Optimize insts in candidate functions and caller functions.*/
    for (i = 0; i < vscSRARR_GetElementCount(paramOptimizer->candidateFuncs); i++)
    {
        CANDIDATE_FUNCTION *candidateFunc = (CANDIDATE_FUNCTION*)vscSRARR_GetElement(paramOptimizer->candidateFuncs, i);
        pFuncBlk = candidateFunc->functionBlock;
        longSizeParams = candidateFunc->longSizeParams;

        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
        pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
        for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
            /*dump*/
            if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                    VSC_OPTN_ParamOptOptions_TRACE))
            {
                VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Optimize caller function: [%s] of function: [%s] \n",
                    VIR_Function_GetNameString(callerBlk->pVIRFunc),
                    VIR_Function_GetNameString(pFuncBlk->pVIRFunc));
                VIR_LOG_FLUSH(paramOptimizer->dumper);
            }
            _VSC_SIMP_OptimizeCaller(pPassWorker, callerBlk->pVIRFunc, paramOptimizer, longSizeParams, shader);
        }

        /*Optimize candidate functions*/
        for (j = 0; j < vscSRARR_GetElementCount(longSizeParams); j++)
        {
            LONG_SIZE_PARAMETER *currentParameter = (LONG_SIZE_PARAMETER*)vscSRARR_GetElement(longSizeParams, j);
            /*dump*/
            if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions),
                    VSC_OPTN_ParamOptOptions_TRACE))
            {
                VIR_LOG(paramOptimizer->dumper,
                    "\n[PAOPT]Optimize callee function: [%s]\n",
                    VIR_Function_GetNameString(pFuncBlk->pVIRFunc));
                VIR_LOG_FLUSH(paramOptimizer->dumper);
            }
            _VSC_SIMP_OptimizeParamInCallee(pPassWorker, candidateFunc, paramOptimizer, currentParameter, shader, paramOptimizer->duInfo);
        }


    }
    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_PARAM_Optimization_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_PAOPT;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VSC_PARAM_Optimization_PerformOnShader)
{
    VIR_Shader *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    /* if shader has only one shader, skip this pass here */
    if (pShader && VIR_Shader_GetFunctionCount(pShader) == 1)
    {
        return gcvFALSE;
    }
    return gcvTRUE;
}

/*******************************************************************************
                      VSC_PARAM_Optimization_PerformOnShader
********************************************************************************

    Optimize input parameters of all functions of one VIR shader.

    INPUT:
        VSC_SH_PASS_WORKER *pPassWorker
            Pointer to optimization passWorker.

*********************************************************************************/
VSC_ErrCode VSC_PARAM_Optimization_PerformOnShader(
    IN VSC_SH_PASS_WORKER *pPassWorker
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VSC_PARAM_optimization paramOptimizer;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_OPTN_ParamOptOptions* paramOptsOptions = (VSC_OPTN_ParamOptOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper *pDumper = pPassWorker->basePassWorker.pDumper;
    VIR_DEF_USAGE_INFO *duInfo = pPassWorker->pDuInfo;
    VIR_Operand *argMmPtr;

    VSC_SIMPLE_RESIZABLE_ARRAY *candidateFuncs =
        vscSRARR_Create(pPassWorker->basePassWorker.pMM, 0, sizeof(CANDIDATE_FUNCTION), gcvNULL);
    VSC_SIMPLE_RESIZABLE_ARRAY *longSizeArguments =
        vscSRARR_Create(pPassWorker->basePassWorker.pMM, 0, sizeof(LONG_SIZE_ARGUMENT), gcvNULL);

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_ParamOptOptions_GetBeforeShader(paramOptsOptions),
        VSC_OPTN_ParamOptOptions_GetAfterShader(paramOptsOptions)))
    {
        if(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Long Param Optimization skip shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
    }
    else
    {
        if(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Long Param Optimization start for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions), VSC_OPTN_ParamOptOptions_TRACE_INPUT_SHADER))
    {
        VIR_Shader_Dump(gcvNULL, "Before Long Param Optimization.", shader, gcvTRUE);
    }
    errCode =VIR_Function_NewOperand(shader->mainFunction, &argMmPtr);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    VSC_PARAM_optimization_Init(&paramOptimizer, shader, candidateFuncs, longSizeArguments, argMmPtr, pDumper, paramOptsOptions, duInfo);
    errCode = VIR_PARAM_Optimization_PerformOnShader(pPassWorker, &paramOptimizer);
    VSC_PARAM_optimization_Final(&paramOptimizer);

    pPassWorker->pResDestroyReq->s.bInvalidateCfg = VSC_PARAM_optimization_GetCfgChanged(&paramOptimizer);

    if (VSC_UTILS_MASK(VSC_OPTN_ParamOptOptions_GetTrace(paramOptsOptions), VSC_OPTN_ParamOptOptions_TRACE_OUTPUT_SHADER) ||
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Long Param Optimization.", shader, gcvTRUE);
    }

    return errCode;
}

