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
    VIR_Operand_SetSymbol(VIR_Inst_GetDest(pNewInsertedInst), pShader->mainFunction, attrSymId);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(pNewInsertedInst), VIR_ENABLE_Y);
    VIR_Operand_SetPrecision(VIR_Inst_GetDest(pNewInsertedInst), VIR_Symbol_GetPrecision(pAttrSym));

    /* src0 */
    VIR_Operand_SetImmediateFloat(VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0), 1.0);

    /* src1 */
    VIR_Operand_SetSymbol(VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1), pShader->mainFunction, attrSymId);
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1), VIR_SWIZZLE_YYYY);
    VIR_Operand_SetPrecision(VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1), VIR_Symbol_GetPrecision(pAttrSym));

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
                    VIR_Operand *opnd;
                    dstTypeId = VIR_Operand_GetType(dst);

                    /* Since MSAA count we supported is less than 32, and bits we can write for sample-mask
                       has only be 0~3 for a specific channel, so we temp change starr to mov. For general
                       solution, we should do loop optimization to remove all indexed access */
                    if (VIR_Inst_GetOpcode(inst) == VIR_OP_STARR)
                    {
                        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                        VIR_Inst_ChangeSource(inst, VIR_Operand_Src0, VIR_Inst_GetSource(inst, VIR_Operand_Src1));
                        VIR_Inst_SetSource(inst, VIR_Operand_Src1, gcvNULL);
                        VIR_Inst_ChangeSrcNum(inst, 1);
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

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_AND_BITWISE, VIR_TYPE_UINT32, inst, gcvTRUE, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(newInst), func, dstSym->index, dstTypeId);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), VIR_ENABLE_X);
                    VIR_Operand_SetPrecision(VIR_Inst_GetDest(newInst), VIR_Symbol_GetPrecision(dstSym));

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
                    VIR_Operand_SetTempRegister(opnd, func, dstSym->index, dstTypeId);
                    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_X);
                    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(dstSym));

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
                    VIR_Operand_SetImmediateUint(opnd, 0xFFFFFFF0);

                    /* new-temp-reg = new-temp-reg & 0x0000000F */

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_AND_BITWISE, VIR_TYPE_UINT32, newInst, gcvTRUE, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(newInst), func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), VIR_ENABLE_X);

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
                    VIR_Operand_SetTempRegister(opnd, func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_X);

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
                    VIR_Operand_SetImmediateUint(opnd, 0x0000000F);

                    /* sample-mask-out = sample-mask-out | new-temp-reg */

                    errCode = VIR_Function_AddInstructionAfter(func, VIR_OP_OR_BITWISE, VIR_TYPE_UINT32, newInst, gcvTRUE, &newInst);
                    ON_ERROR(errCode, "Add instruction after");

                    VIR_Operand_SetTempRegister(VIR_Inst_GetDest(newInst), func, dstSym->index, dstTypeId);
                    VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), VIR_ENABLE_X);
                    VIR_Operand_SetPrecision(VIR_Inst_GetDest(newInst), VIR_Symbol_GetPrecision(dstSym));

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
                    VIR_Operand_SetTempRegister(opnd, func, dstSym->index, dstTypeId);
                    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_X);
                    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(dstSym));

                    opnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
                    VIR_Operand_SetTempRegister(opnd, func, newDstSymId, VIR_TYPE_UINT32);
                    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_X);

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
    VIR_Operand *              opnd;
    VIR_SymId                  outTmpId, outSymId;

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
    outTmpId = VIR_Shader_NewVirRegId(pShader, 1);
    VIR_Shader_AddSymbol(
        pShader,
        VIR_SYM_VIRREG,
        outTmpId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
        VIR_STORAGE_UNKNOWN,
        &outSymId);
    VIR_Symbol_SetVariableVregIndex(pOutLayerSym, outTmpId);
    VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(pShader, outSymId), pOutLayerSym);
    VIR_Symbol_SetFirstSlot(pOutLayerSym, nextOutputLlSlot);

    /* Per HW requirement, output layer is highp */
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
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetTempRegister(opnd,
                                pShader->mainFunction,
                                outSymId,
                                VIR_TYPE_UINT32);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pOutLayerSym));

    /* src */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, attrSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pAttrSym));

OnError:
    return errCode;
}

/*
Constant U32 Y_MULTIPLIER = WORKGROUP_XSIZE % NUM_OF_REGION;
Constant U32 Z_MULTIPLIER = (WORKGROUP_XSIZE * WORKGROUP_YSIZE) %
NUM_OF_REGION;

Acquire_localMem()
{
  // two iMAD instructions for 3D WorkGroupIDs
  // one iMAD instruction for 2D WorkGroupIDs
  // zero iMAD instruction for 1D WorkGroupID
  U32 WorkGroupID1D = WorkGroupID.X + Y_MULTIPLIER * WorkGroupID.Y +
Z_MULTIPLIER*WorkGroupID.Z;
  // one iMOD instruction
  U16 AddressMultiplier = WorkGroupID1D % NUM_OF_REGION;
  // one iMUL instruction
  U32 base = AddressMultiplier * REGION_SIZE;
}
*/
static VSC_ErrCode _DoLocalMemAccessPatch(VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_NameId                 nameId;
    VIR_Symbol*                sym = gcvNULL;
    VIR_SymId                  regionNumId, y_mulSymId, z_mulSymId, workGroupSymId, origBaseAddrSymId;
    VIR_SymId                  workGroup1DIndexSymId, addressMultiplierSymId, baseAddressSymId;
    VIR_VirRegId               regId1, regId2, regId3;
    VIR_ScalarConstVal         regionSize;
    VIR_Instruction*           pNewInsertedInst;
    VIR_Instruction*           pPrevInst;
    VIR_Precision              precision = VIR_PRECISION_HIGH;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          func_node;
    VIR_Operand *              opnd;

    /* Find uniform "#num_of_region, if not found, create them. */
    sym = VIR_Shader_FindSymbolByName(pShader,
                                      VIR_SYM_UNIFORM,
                                      "#num_of_region");

    if (sym == gcvNULL)
    {
        errCode = VIR_Shader_AddString(pShader,
                                       "#num_of_region",
                                       &nameId);
        if (errCode != VSC_ERR_NONE) return errCode;

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                       VIR_STORAGE_UNKNOWN,
                                       &regionNumId);
        if (errCode != VSC_ERR_NONE) return errCode;
        sym = VIR_Shader_GetSymFromId(pShader, regionNumId);
        VIR_Symbol_SetLocation(sym, -1);
    }
    else
    {
        regionNumId = VIR_Symbol_GetIndex(sym);
    }

    /* Find uniforms "#y_multiplier" and "#z_multiplier", if not found, create them. */
    sym = VIR_Shader_FindSymbolByName(pShader,
                                      VIR_SYM_UNIFORM,
                                      "#y_multiplier");

    if (sym == gcvNULL)
    {
        errCode = VIR_Shader_AddString(pShader,
                                       "#y_multiplier",
                                       &nameId);
        if (errCode != VSC_ERR_NONE) return errCode;

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                       VIR_STORAGE_UNKNOWN,
                                       &y_mulSymId);
        if (errCode != VSC_ERR_NONE) return errCode;
        sym = VIR_Shader_GetSymFromId(pShader, y_mulSymId);
        VIR_Symbol_SetLocation(sym, -1);
    }
    else
    {
        y_mulSymId = VIR_Symbol_GetIndex(sym);
    }

    sym = VIR_Shader_FindSymbolByName(pShader,
                                      VIR_SYM_UNIFORM,
                                      "#z_multiplier");

    if (sym == gcvNULL)
    {
        errCode = VIR_Shader_AddString(pShader,
                                       "#z_multiplier",
                                       &nameId);
        if (errCode != VSC_ERR_NONE) return errCode;

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                       VIR_STORAGE_UNKNOWN,
                                       &z_mulSymId);
        if (errCode != VSC_ERR_NONE) return errCode;
        sym = VIR_Shader_GetSymFromId(pShader, z_mulSymId);
        VIR_Symbol_SetLocation(sym, -1);
    }
    else
    {
        z_mulSymId = VIR_Symbol_GetIndex(sym);
    }

    /* Find workGroupID, if not found, create it. */
    sym = VIR_Shader_FindSymbolById(pShader,
                                    VIR_SYM_VARIABLE,
                                    VIR_NAME_WORK_GROUP_ID);
    if (errCode != VSC_ERR_NONE) return errCode;

    if (sym == gcvNULL)
    {
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_WORK_GROUP_ID,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                       VIR_STORAGE_INPUT,
                                       &workGroupSymId);
        if (errCode != VSC_ERR_NONE) return errCode;
    }
    else
    {
        workGroupSymId = VIR_Symbol_GetIndex(sym);
    }

    /* Set region size. */
    regionSize.uValue = VIR_Shader_GetLocalMemorySize(pShader);

    /*--------------begin to insert new instructions--------------*/
    /* workGroup1DIndex = WorkGroupID.X + Y_MULTIPLIER * WorkGroupID.Y */
    /* Create reg for workGroup1DIndex. */
    regId1 = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId1,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &workGroup1DIndexSymId);
    if (errCode != VSC_ERR_NONE) return errCode;
    errCode = VIR_Function_PrependInstruction(pShader->mainFunction,
                                              VIR_OP_MAD,
                                              VIR_TYPE_UINT32,
                                              &pNewInsertedInst);
    if (errCode != VSC_ERR_NONE) return errCode;
    pPrevInst = pNewInsertedInst;

    /* dst */
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroup1DIndexSymId);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src0 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, y_mulSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src1 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroupSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_YYYY);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src2 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src2);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroupSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* workGroup1DIndex = workGroup1DIndex + Z_MULTIPLIER*WorkGroupID.Z */
    errCode = VIR_Function_AddInstructionAfter(pShader->mainFunction,
                                               VIR_OP_MAD,
                                               VIR_TYPE_UINT32,
                                               pPrevInst,
                                               gcvTRUE,
                                               &pNewInsertedInst);
    if (errCode != VSC_ERR_NONE) return errCode;
    pPrevInst = pNewInsertedInst;

    /* dst */
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroup1DIndexSymId);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src0 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, z_mulSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src1 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroupSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src2 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src2);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroup1DIndexSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* addressMultiplier = workGroup1DIndex % NUM_OF_REGION */
    /* Create reg for addressMultiplier. */
    regId2 = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId2,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                   VIR_STORAGE_UNKNOWN,
                                   &addressMultiplierSymId);
    if (errCode != VSC_ERR_NONE) return errCode;
    errCode = VIR_Function_AddInstructionAfter(pShader->mainFunction,
                                               VIR_OP_MOD,
                                               VIR_TYPE_UINT16,
                                               pPrevInst,
                                               gcvTRUE,
                                               &pNewInsertedInst);
    if (errCode != VSC_ERR_NONE) return errCode;
    pPrevInst = pNewInsertedInst;

    /* dst */
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, addressMultiplierSymId);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src0 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, workGroup1DIndexSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src1 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, regionNumId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* base = AddressMultiplier * REGION_SIZE */
    /* Create reg for base. */
    regId3 = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId3,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &baseAddressSymId);
    if (errCode != VSC_ERR_NONE) return errCode;
    errCode = VIR_Function_AddInstructionAfter(pShader->mainFunction,
                                               VIR_OP_MUL,
                                               VIR_TYPE_UINT32,
                                               pPrevInst,
                                               gcvTRUE,
                                               &pNewInsertedInst);
    if (errCode != VSC_ERR_NONE) return errCode;
    pPrevInst = pNewInsertedInst;

    /* dst */
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, baseAddressSymId);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src0 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, addressMultiplierSymId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, precision);

    /* src1 */
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src1);
    VIR_Operand_SetImmediate(opnd, VIR_TYPE_UINT32, regionSize);

    /* Use baseAddressSymId to replace #sh_sharedVar. */
    /* Find #sh_sharedVar. */
    sym = VIR_Shader_FindSymbolByName(pShader,
                                      VIR_SYM_UNIFORM,
                                      "#sh_sharedVar");
    gcmASSERT(sym);
    origBaseAddrSymId = VIR_Symbol_GetIndex(sym);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*       func = func_node->function;
        VIR_Instruction*    inst = func->instList.pHead;
        gctUINT             i, srcNum;
        VIR_Operand*        srcOperand;

        while (inst != gcvNULL)
        {
            srcNum = VIR_Inst_GetSrcNum(inst);
            for (i = 0; i < srcNum; i++)
            {
                srcOperand = VIR_Inst_GetSource(inst, i);
                if (VIR_Operand_GetOpKind(srcOperand) == VIR_OPND_SYMBOL &&
                    VIR_Operand_GetSymbolId_(srcOperand) == origBaseAddrSymId &&
                    VIR_Operand_GetSwizzle(srcOperand) == VIR_SWIZZLE_XXXX)
                {
                    VIR_Operand_SetSymbol(srcOperand, pShader->mainFunction, baseAddressSymId);
                }
            }
            inst = VIR_Inst_GetNext(inst);
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PerformSpecialHwPatches)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
}

VSC_ErrCode vscVIR_PerformSpecialHwPatches(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

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
    }

    if (pShader->shaderKind == VIR_SHADER_COMPUTE)
    {
        /* We need a variable to check if Acquire_localMem is needed. */
        if (0)
        {
            /* For some HWs, they can not allocate local-mem for each thread-group, so we need
               patch machine code to get it done inside of shader. */
            errCode = _DoLocalMemAccessPatch(pShader);
            ON_ERROR(errCode, "Do local-mem access patch");
        }
    }

OnError:
    return errCode;
}


