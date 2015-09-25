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


#include "gc_vsc.h"

static VSC_ErrCode _DoPointCoordYDirectionPatch(VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList*       pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLsts);
    VIR_Symbol*                pAttrSym = gcvNULL;
    gctUINT                    attrIdx, attrSymId = VIR_INVALID_ID;
    gctBOOL                    bHasPointCoordAttr = gcvFALSE;
    VIR_Instruction*           pNewInsertedInst;

    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
        pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);

        if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
        {
            if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_POINT_COORD)
            {
                bHasPointCoordAttr = gcvTRUE;
                break;
            }
        }
    }

    if (!bHasPointCoordAttr)
    {
        return errCode;
    }

    /* Add following inst at the begin of main routine:

       sub pointCoord.y, 1, pointCoord.y
    */

    errCode = VIR_Function_PrependInstruction(pShader->mainFunction,
                                              VIR_OP_SUB,
                                              VIR_TYPE_FLOAT32,
                                              &pNewInsertedInst);
    ON_ERROR(errCode, "Prepend instruction");

    /* dst */
    VIR_Operand_SetSymbol(pNewInsertedInst->dest, pShader->mainFunction, attrSymId);
    VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_Y);
    VIR_Operand_SetPrecision(pNewInsertedInst->dest, VIR_Symbol_GetPrecision(pAttrSym));

    /* src0 */
    VIR_Operand_SetImmediateFloat(pNewInsertedInst->src[VIR_Operand_Src0], 1.0);

    /* src1 */
    VIR_Operand_SetSymbol(pNewInsertedInst->src[VIR_Operand_Src1], pShader->mainFunction, attrSymId);
    VIR_Operand_SetSwizzle(pNewInsertedInst->src[VIR_Operand_Src1], VIR_SWIZZLE_YYYY);
    VIR_Operand_SetPrecision(pNewInsertedInst->src[VIR_Operand_Src1], VIR_Symbol_GetPrecision(pAttrSym));

OnError:
    return errCode;
}

VSC_ErrCode _DoOutSampleMaskPatch(VIR_Shader* pShader)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_Operand       *dst;
    VIR_Symbol        *dstSym, *varSym;
    VIR_TypeId        dstTypeId;
    VIR_SymId         newDstSymId;
    gctUINT           newDstRegNo;
    VIR_Instruction*  newInst;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            dst = VIR_Inst_GetDest(inst);
            if (dst == gcvNULL ||
                VIR_Operand_GetOpKind(dst) == VIR_OPND_LABEL)
            {
                continue;
            }

            dstSym = VIR_Operand_GetSymbol(dst);
            if (VIR_Symbol_GetKind(dstSym) == VIR_SYM_VIRREG)
            {
                varSym = VIR_Symbol_GetVregVariable(dstSym);
                if (varSym && VIR_Symbol_GetName(varSym) == VIR_NAME_SAMPLE_MASK)
                {
                    dstTypeId = VIR_Operand_GetType(dst);

                    /* Since MSAA count we supported is less than 32, and bits we can write for sample-mask
                       has only be 0~3 for a specific channel, so we temp change starr to mov. For general
                       solution, we should do loop optimization to remove all indexed access */
                    if (VIR_Inst_GetOpcode(inst) == VIR_OP_STARR)
                    {
                        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                        inst->src[VIR_Operand_Src0] = inst->src[VIR_Operand_Src1];
                        VIR_Inst_SetSrcNum(inst, 1);
                    }

                    /* Add a new-temp-reg number */
                    newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
                    errCode = VIR_Shader_AddSymbol(pShader,
                                                   VIR_SYM_VIRREG,
                                                   newDstRegNo,
                                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &newDstSymId);
                    ON_ERROR(errCode, "Add symbol");

                    /* Set dst to this new temp reg number */
                    VIR_Operand_SetTempRegister(dst, func, newDstSymId, VIR_TYPE_UINT32);

                    /* sample-mask-out = sample-mask-out & 0xFFFFFFF0 */

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_AND_BITWISE, VIR_TYPE_UINT32, inst, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(newInst->dest, func, dstSym->index, dstTypeId);
                    VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_X);
                    VIR_Operand_SetPrecision(newInst->dest, VIR_Symbol_GetPrecision(dstSym));

                    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0], func, dstSym->index, dstTypeId);
                    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);
                    VIR_Operand_SetPrecision(newInst->src[VIR_Operand_Src0], VIR_Symbol_GetPrecision(dstSym));

                    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1], 0xFFFFFFF0);

                    /* new-temp-reg = new-temp-reg & 0x0000000F */

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_AND_BITWISE, VIR_TYPE_UINT32, newInst, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(newInst->dest, func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_X);

                    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0], func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);

                    VIR_Operand_SetImmediateUint(newInst->src[VIR_Operand_Src1], 0x0000000F);

                    /* sample-mask-out = sample-mask-out | new-temp-reg */

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_OR_BITWISE, VIR_TYPE_UINT32, newInst, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(newInst->dest, func, dstSym->index, dstTypeId);
                    VIR_Operand_SetEnable(newInst->dest, VIR_ENABLE_X);
                    VIR_Operand_SetPrecision(newInst->dest, VIR_Symbol_GetPrecision(dstSym));

                    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src0], func, dstSym->index, dstTypeId);
                    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src0], VIR_SWIZZLE_X);
                    VIR_Operand_SetPrecision(newInst->src[VIR_Operand_Src0], VIR_Symbol_GetPrecision(dstSym));

                    VIR_Operand_SetTempRegister(newInst->src[VIR_Operand_Src1], func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(newInst->src[VIR_Operand_Src1], VIR_SWIZZLE_X);

                    /* Move 3 new inst steps */
                    inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);
                    inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);
                    inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);
                }
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoRtLayerPatch(VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList*       pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
    VIR_OutputIdList*          pOutputIdLsts = VIR_Shader_GetOutputs(pShader);
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLsts);
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLsts);
    VIR_Symbol*                pAttrSym = gcvNULL;
    VIR_Symbol*                pOutputSym;
    VIR_Symbol*                pOutLayerSym;
    gctUINT                    attrIdx, outputIdx, nextOutputLlSlot = 0, thisOutputRegCount;
    gctUINT                    attrSymId = VIR_INVALID_ID, outputSymId = VIR_INVALID_ID;
    gctBOOL                    bHasLayerAttr = gcvFALSE;
    VIR_Instruction*           pNewInsertedInst;

    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
        pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);

        if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
        {
            if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_LAYER)
            {
                bHasLayerAttr = gcvTRUE;
                break;
            }
        }
    }

    if (!bHasLayerAttr)
    {
        return errCode;
    }

    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        outputSymId = VIR_IdList_GetId(pOutputIdLsts, outputIdx);
        pOutputSym = VIR_Shader_GetSymFromId(pShader, outputSymId);

        if (!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym))
        {
            gcmASSERT(VIR_Symbol_GetFirstSlot(pOutputSym) != NOT_ASSIGNED);

            thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pOutputSym);
            if (nextOutputLlSlot < (VIR_Symbol_GetFirstSlot(pOutputSym) + thisOutputRegCount))
            {
                nextOutputLlSlot = VIR_Symbol_GetFirstSlot(pOutputSym) + thisOutputRegCount;
            }
        }
    }

    /* Add layer output and set its llSlot */
    pOutLayerSym = VIR_Shader_AddBuiltinOutput(pShader, VIR_TYPE_UINT32, gcvFALSE, VIR_NAME_PS_OUT_LAYER);
    VIR_Symbol_SetFirstSlot(pOutLayerSym, nextOutputLlSlot);
    /* per HW requirement, output layer is highp */
    VIR_Symbol_SetPrecision(pOutLayerSym, VIR_PRECISION_HIGH);

    /* Add following inst at the begin of main routine:

       mov output_layer, input_layer
    */

    errCode = VIR_Function_PrependInstruction(pShader->mainFunction,
                                              VIR_OP_MOV,
                                              VIR_TYPE_UINT32,
                                              &pNewInsertedInst);
    ON_ERROR(errCode, "Prepend instruction");

    /* dst */
    VIR_Operand_SetSymbol(pNewInsertedInst->dest, pShader->mainFunction, pOutLayerSym->index);
    VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(pNewInsertedInst->dest, VIR_Symbol_GetPrecision(pOutLayerSym));

    /* src */
    VIR_Operand_SetSymbol(pNewInsertedInst->src[VIR_Operand_Src0], pShader->mainFunction, attrSymId);
    VIR_Operand_SetSwizzle(pNewInsertedInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Symbol_GetPrecision(pAttrSym));

OnError:
    return errCode;
}

/* gl_postion will be put into r0, while HW requires gl_depth is put into r0.z.
   it is possible that these two liveranges overlap with each other. Thus when
   both appear in the shader, we add an extra mov at the end of shader for gl_depth.
   (we are not adding mov for position since depth is a float).
   TODO: only add move when gl_postion and gl_depth overlaps. */
static VSC_ErrCode _DoDepthPatch(VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList*       pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLsts);
    VIR_Symbol*                pAttrSym = gcvNULL;
    gctUINT                    attrIdx, attrSymId = VIR_INVALID_ID;
    VIR_OutputIdList           *pOutputIdLsts = VIR_Shader_GetOutputs(pShader);
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLsts);
    VIR_Symbol*                pOutputSym = gcvNULL, *pNewSym = gcvNULL;
    gctUINT                    outputIdx, outputSymId = VIR_INVALID_ID, newDstSymId = VIR_INVALID_ID;
    gctBOOL                    needPatch = gcvFALSE;
    VIR_Instruction*           pNewInsertedInst = gcvNULL;
    gctUINT                    newDstRegNo;

    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_TypeId        depthTypeId;

    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
        pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);

        if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
        {
            if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_POSITION)
            {
                needPatch = gcvTRUE;
                break;
            }
        }
    }

    if (!needPatch)
    {
        return errCode;
    }

    needPatch = gcvFALSE;

    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        outputSymId = VIR_IdList_GetId(pOutputIdLsts, outputIdx);
        pOutputSym = VIR_Shader_GetSymFromId(pShader, outputSymId);

        if (!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym))
        {
            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_DEPTH)
            {
                needPatch = gcvTRUE;
                break;
            }
        }
    }

    if (!needPatch)
    {
        return errCode;
    }

    /* Add following inst at the end of main routine:
       mov depth, temp
    */
    errCode = VIR_Function_AddInstruction(pShader->mainFunction,
                                              VIR_OP_MOV,
                                              VIR_TYPE_FLOAT32,
                                              &pNewInsertedInst);
    ON_ERROR(errCode, "append instruction");

    newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                    VIR_SYM_VIRREG,
                                    newDstRegNo,
                                    VIR_Symbol_GetType(pOutputSym),
                                    VIR_STORAGE_UNKNOWN,
                                    &newDstSymId);
    ON_ERROR(errCode, "Add symbol");
    pNewSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

    /* src0 */
    depthTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pOutputSym));
    VIR_Operand_SetTempRegister(pNewInsertedInst->src[VIR_Operand_Src0], pShader->mainFunction, newDstSymId, depthTypeId);
    VIR_Operand_SetSwizzle(pNewInsertedInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Symbol_GetPrecision(pOutputSym));
    VIR_Symbol_SetPrecision(pNewSym, VIR_Symbol_GetPrecision(pOutputSym));

    /* dest */
    VIR_Operand_SetSymbol(pNewInsertedInst->dest, pShader->mainFunction, outputSymId);
    VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(pNewInsertedInst->dest, VIR_Symbol_GetPrecision(pOutputSym));

    /* change the def of depth to temp */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_Operand *dstOpnd = VIR_Inst_GetDest(inst);
            VIR_Symbol  *dstSym = gcvNULL;

            if (inst == pNewInsertedInst ||
                !VIR_Inst_GetDest(inst))
            {
                continue;
            }

            dstSym = VIR_Operand_GetSymbol(dstOpnd);

            if (VIR_Symbol_GetKind(dstSym) == VIR_SYM_VIRREG)
            {
                dstSym = VIR_Symbol_GetVregVariable(dstSym);
                if (dstSym && VIR_Symbol_GetName(dstSym) == VIR_NAME_DEPTH)
                {
                    gcmASSERT(VIR_Operand_GetEnable(dstOpnd) == VIR_ENABLE_X);
                    VIR_Operand_SetTempRegister(dstOpnd, func, newDstSymId, depthTypeId);
                }
            }
        }
    }
OnError:
    return errCode;
}

VSC_ErrCode vscVIR_PerformSpecialHwPatches(VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    if (pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        /* For point coord, HW generate Y direction is bottom-up, while spec require it to
           be up-bottom, so we need revert it */
        errCode = _DoPointCoordYDirectionPatch(pShader);
        ON_ERROR(errCode, "Do point coord Y direction patch");

        /* Hw's output-sample-mask is only located in LSB4, and other bits has use for other
           purpose, so we need patch to insure only LSB4 is touched when output-sample-mask
           is written */
        errCode = _DoOutSampleMaskPatch(pShader);
        ON_ERROR(errCode, "Do output-sample-mask patch");

        /* Hw's PE does not get gl_Layer from RA, so we need add explicit mov to make this
           work */
        errCode = _DoRtLayerPatch(pShader);
        ON_ERROR(errCode, "Do rt-layer patch");

        errCode = _DoDepthPatch(pShader);
        ON_ERROR(errCode, "Do Depth patch");
    }

OnError:
    return errCode;
}


