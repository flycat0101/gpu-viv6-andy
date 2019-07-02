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


#include "gc_spirv_cvter.h"
#include "gc_spirv.h"
#include "gc_spirv_to_vir.h"

static gctBOOL
__SpvFoldingTwoSrcArithmeticOpPerComponent(
    gcSPV           spv,
    gctUINT         componentTypeId,
    gctUINT         value0,
    gctUINT         value1,
    gctUINT*        pRetValue
    )
{
    SpvOp           opCode = spv->opCode;

    if (SPV_ID_TYPE_IS_SIGNEDINTEGER(componentTypeId))
    {
        gctINT      leftOperand = *((gctINT *)&value0);
        gctINT      rightOperand =*((gctINT *)&value1);

        switch (opCode)
        {
        case SpvOpIAdd:
            *((gctINT *)pRetValue) = leftOperand + rightOperand;
            break;

        case SpvOpIMul:
            *((gctINT *)pRetValue) = leftOperand * rightOperand;
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }

    }
    else if (SPV_ID_TYPE_IS_FLOAT(componentTypeId))
    {
        gctFLOAT    leftOperand = *((gctFLOAT *)&value0);
        gctFLOAT    rightOperand = *((gctFLOAT *)&value1);

        switch (opCode)
        {
        case SpvOpFAdd:
            *((gctFLOAT *)pRetValue) = leftOperand + rightOperand;
            break;

        case SpvOpFMul:
            *((gctFLOAT *)pRetValue) = leftOperand * rightOperand;
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else
    {
        gctUINT     leftOperand = value0;
        gctUINT     rightOperand = value1;

        switch (opCode)
        {
        case SpvOpIAdd:
            *pRetValue = leftOperand + rightOperand;
            break;

        case SpvOpIMul:
            *pRetValue = leftOperand * rightOperand;
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    return gcvTRUE;
}

static gctBOOL
__SpvFoldingTwoSrcArithmeticOp(
    gcSPV           spv,
    VIR_Shader*     pVirShader
    )
{
    gctUINT         operandSize;
    gctUINT         i;
    gctUINT         u32Value[VIR_CONST_MAX_CHANNEL_COUNT];

    if (!(SPV_ID_TYPE(spv->operands[0]) == SPV_ID_TYPE_CONST && SPV_ID_TYPE(spv->operands[1]) == SPV_ID_TYPE_CONST))
    {
        __SpvEmitInstructions(spv, pVirShader);
        return gcvFALSE;
    }

    if (SPV_ID_TYPE_IS_VECTOR(spv->resultTypeId))
    {
        gctUINT     componentCount = SPV_ID_TYPE_VEC_COMP_NUM(spv->resultTypeId);
        gctUINT     componentTypeId = SPV_ID_TYPE_VEC_COMP_TYPE(spv->resultTypeId);

        for (i = 0; i < componentCount; i++)
        {
            __SpvFoldingTwoSrcArithmeticOpPerComponent(spv,
                                                       componentTypeId,
                                                       SPV_ID_VIR_CONST(spv->operands[0]).vecVal.u32Value[i],
                                                       SPV_ID_VIR_CONST(spv->operands[1]).vecVal.u32Value[i],
                                                       &u32Value[i]);
        }

        operandSize = componentCount;
    }
    else
    {
        __SpvFoldingTwoSrcArithmeticOpPerComponent(spv,
                                                   spv->resultTypeId,
                                                   SPV_ID_VIR_CONST(spv->operands[0]).scalarVal.uValue,
                                                   SPV_ID_VIR_CONST(spv->operands[1]).scalarVal.uValue,
                                                   &u32Value[0]);

        operandSize = 1;
    }

    /* Success, change it to OpConstant. */
    spv->opCode = SpvOpConstant;
    spv->operandSize = operandSize;
    for (i = 0; i < operandSize; i++)
    {
        spv->operands[i] = u32Value[i];
    }

    return gcvTRUE;
}

static gctBOOL
__SpvFoldingCompositeVectorType(
    gcSPV           spv,
    gctUINT         objectId,
    gctUINT         componentIndex
    )
{
    gctUINT         objectTypeId = SPV_ID_CST_SPV_TYPE(objectId);

    if (componentIndex >= SPV_ID_TYPE_VEC_COMP_NUM(objectTypeId))
    {
        gcmASSERT(gcvFALSE);
        componentIndex = 0;
    }

    /* Success, change it to OpConstant. */
    spv->opCode = SpvOpConstant;
    spv->operandSize = 1;
    spv->operands[0] = SPV_ID_VIR_CONST(objectId).vecVal.u32Value[componentIndex];

    return gcvTRUE;
}

static gctBOOL
__SpvFoldingCompositeExtractOp(
    gcSPV           spv,
    VIR_Shader*     pVirShader
    )
{
    gctBOOL         bSuccess = gcvTRUE;
    gctUINT         objectId = spv->operands[0];
    gctUINT         objectTypeId = spv->resultTypeId;

    /* So far we only support constant vector. */
    if (!(SPV_ID_TYPE(objectId) == SPV_ID_TYPE_CONST))
    {
        bSuccess = gcvFALSE;
    }
    else
    {
        objectTypeId = SPV_ID_CST_SPV_TYPE(objectId);
        if (!(SPV_ID_TYPE_IS_VECTOR(objectTypeId)))
        {
            bSuccess = gcvFALSE;
        }
    }

    if (!bSuccess)
    {
        __SpvEmitCompositeExtract(spv, pVirShader);
        return bSuccess;
    }

    if (SPV_ID_TYPE_IS_VECTOR(objectTypeId))
    {
        __SpvFoldingCompositeVectorType(spv, spv->operands[0], spv->operands[1]);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return bSuccess;
}

VSC_ErrCode
__SpvFoldingSpecConstantOp(
    gcSPV           spv,
    VIR_Shader*     pVirShader
    )
{
    VSC_ErrCode     virErrCode = VSC_ERR_NONE;
    SpvOp           opCode = spv->opCode;
    gctBOOL         bSuccess = gcvFALSE;

    switch (opCode)
    {
    case SpvOpIAdd:
    case SpvOpIMul:
        bSuccess = __SpvFoldingTwoSrcArithmeticOp(spv, pVirShader);
        break;

    case SpvOpCompositeExtract:
        bSuccess = __SpvFoldingCompositeExtractOp(spv, pVirShader);
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (bSuccess)
    {
        __SpvEmitConstant(spv, pVirShader);
    }

    return virErrCode;
}
