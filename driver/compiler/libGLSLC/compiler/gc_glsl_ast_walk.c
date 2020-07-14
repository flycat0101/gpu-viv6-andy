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


#include "gc_glsl_ast_walk.h"
#include "gc_glsl_emit_code.h"

static gceSTATUS
_CountFuncResources(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN slsNAME *FuncName
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x FuncName=0x%x",
                  Compiler, ObjectCounter, FuncName);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    gcmASSERT(FuncName);

    if(FuncName->context.isCounted)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    ObjectCounter->functionCount++;
    FuncName->context.isCounted = gcvTRUE;

    gcmFOOTER();
    return status;
}

gceSTATUS
sloIR_ITERATION_CountForCode(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_ITERATION Iteration,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    slsGEN_CODE_PARAMETERS  initParameters, restParameters, bodyParameters;
    slsGEN_CODE_PARAMETERS  condParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Iteration=0x%x",
                  Compiler, ObjectCounter, Iteration);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Iteration, slvIR_ITERATION);
    gcmASSERT(Iteration->type == slvFOR);

    /* The init part */
    if (Iteration->forInitStatement != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&initParameters, gcvFALSE, gcvFALSE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     Iteration->forInitStatement,
                                     &ObjectCounter->visitor,
                                     &initParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&initParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    /* The rest part */
    if (Iteration->forRestExpr != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&restParameters, gcvFALSE, gcvFALSE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     &Iteration->forRestExpr->base,
                                     &ObjectCounter->visitor,
                                     &restParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&restParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    /* The condition part */
    if (Iteration->condExpr != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     &Iteration->condExpr->base,
                                     &ObjectCounter->visitor,
                                     &condParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    /* The body part */
    if (Iteration->loopBody != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     Iteration->loopBody,
                                     &ObjectCounter->visitor,
                                     &bodyParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
}
    }
    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_ITERATION_CountWhileCode(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_ITERATION Iteration,
    IN OUT slsGEN_CODE_PARAMETERS * Parameters
    )
{
    gceSTATUS               status;
    slsGEN_CODE_PARAMETERS  bodyParameters, condParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Iteration=0x%x",
                  Compiler, ObjectCounter, Iteration);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Iteration, slvIR_ITERATION);
    gcmASSERT(Iteration->type == slvWHILE);

    /* The condition part */
    if (Iteration->condExpr != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     &Iteration->condExpr->base,
                                     &ObjectCounter->visitor,
                                     &condParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    /* The loop body */
    if (Iteration->loopBody != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     Iteration->loopBody,
                                     &ObjectCounter->visitor,
                                     &bodyParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_ITERATION_CountDoWhileCode(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_ITERATION Iteration,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS  status;
    slsGEN_CODE_PARAMETERS  bodyParameters, condParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Iteration=0x%x",
                  Compiler, ObjectCounter, Iteration);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Iteration, slvIR_ITERATION);
    gcmASSERT(Iteration->type == slvDO_WHILE);

    if (Iteration->loopBody != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);
        status = sloIR_OBJECT_Accept(Compiler,
                                     Iteration->loopBody,
                                     &ObjectCounter->visitor,
                                     &bodyParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    /* The condition part */
    if (Iteration->condExpr != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);
        status = sloIR_OBJECT_Accept(Compiler,
                                     &Iteration->condExpr->base,
                                     &ObjectCounter->visitor,
                                     &condParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_ITERATION_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_ITERATION Iteration,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Iteration=0x%x",
                  Compiler, ObjectCounter, Iteration);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Iteration, slvIR_ITERATION);

    do
    {
        sloIR_BINARY_EXPR binaryExpr;
        slsITERATION_CONTEXT iterationContext;
        slsNAME * loopIndexName;
        slsGEN_CODE_PARAMETERS initParameters, restParameters, bodyParameters;
        slsGEN_CODE_PARAMETERS condParameters;
        gctUINT beginAtomicOpCount;

        if (Iteration->type != slvFOR) break;

        if (Iteration->forInitStatement == gcvNULL ||
            sloIR_OBJECT_GetType(Iteration->forInitStatement) != slvIR_BINARY_EXPR)
        {
            break;
        }

        binaryExpr = (sloIR_BINARY_EXPR)Iteration->forInitStatement;

        if (binaryExpr->type != slvBINARY_ASSIGN ||
            sloIR_OBJECT_GetType(&binaryExpr->leftOperand->base) != slvIR_VARIABLE)
        {
            break;
        }

        loopIndexName = ((sloIR_VARIABLE)binaryExpr->leftOperand)->name;

        /* The init part */
        if (Iteration->forInitStatement != gcvNULL)
        {
            slsGEN_CODE_PARAMETERS_Initialize(&initParameters, gcvFALSE, gcvFALSE);
            status = sloIR_OBJECT_Accept(Compiler,
                                         Iteration->forInitStatement,
                                         &ObjectCounter->visitor,
                                         &initParameters);
            slsGEN_CODE_PARAMETERS_Finalize(&initParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        /* The rest part */
        if (Iteration->forRestExpr != gcvNULL)
        {
            slsGEN_CODE_PARAMETERS_Initialize(&restParameters, gcvFALSE, gcvFALSE);
            status = sloIR_OBJECT_Accept(Compiler,
                                         &Iteration->forRestExpr->base,
                                         &ObjectCounter->visitor,
                                         &restParameters);
            slsGEN_CODE_PARAMETERS_Finalize(&restParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        /* The condition part */
        if (Iteration->condExpr != gcvNULL)
        {
            slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvTRUE);
            status = sloIR_OBJECT_Accept(Compiler,
                                         &Iteration->condExpr->base,
                                         &ObjectCounter->visitor,
                                         &condParameters);
            slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        status = slGenDefineUnrolledIterationBegin(Compiler,
                                                   ObjectCounter->codeGenerator,
                                                   loopIndexName,
                                                   gcvTRUE,
                                                   0,
                                                   &iterationContext);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        /* The body part */
        if (Iteration->loopBody != gcvNULL)
        {
            beginAtomicOpCount = ObjectCounter->opcodeCount[slvOPCODE_ATOMADD];
            slsGEN_CODE_PARAMETERS_Initialize(&bodyParameters, gcvFALSE, gcvFALSE);
            status = sloIR_OBJECT_Accept(Compiler,
                                         Iteration->loopBody,
                                         &ObjectCounter->visitor,
                                         &bodyParameters);
            slsGEN_CODE_PARAMETERS_Finalize(&bodyParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            Iteration->atomicOpCount =
                ObjectCounter->opcodeCount[slvOPCODE_ATOMADD] - beginAtomicOpCount;
        }

        status = slGenDefineUnrolledIterationEnd(Compiler,
                                                 ObjectCounter->codeGenerator,
                                                 gcvTRUE);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    switch (Iteration->type)
    {
    case slvFOR:
        status = sloIR_ITERATION_CountForCode(Compiler,
                                              ObjectCounter,
                                              Iteration,
                                              Parameters);
        break;

    case slvWHILE:
        status = sloIR_ITERATION_CountWhileCode(Compiler,
                                                 ObjectCounter,
                                                 Iteration,
                                                 Parameters);
        break;

    case slvDO_WHILE:
        status = sloIR_ITERATION_CountDoWhileCode(Compiler,
                                                  ObjectCounter,
                                                  Iteration,
                                                  Parameters);
        break;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER();
       return status;
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_JUMP_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_JUMP Jump,
    IN OUT slsGEN_CODE_PARAMETERS * Parameters
    )
{
    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Jump=0x%x",
                  Compiler, ObjectCounter, Jump);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Jump, slvIR_JUMP);
    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

static gceSTATUS
_CountVariableOrArray(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN slsNAME *Name,
    IN slsDATA_TYPE *DataType
    )
{
    sltSTORAGE_QUALIFIER    qualifier;
    gctUINT         logicalRegCount;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Name=0x%x "
                  "DataType=0x%x",
                  Compiler, ObjectCounter, Name,
                  DataType);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    gcmVERIFY_ARGUMENT(Name);
    gcmASSERT(DataType);
    gcmASSERT(!slsDATA_TYPE_IsStruct(DataType));

    qualifier = Name->dataType->qualifiers.storage;

    logicalRegCount = slsDATA_TYPE_GetLogicalCountForAnArray(DataType);

    switch (qualifier)
    {
    case slvSTORAGE_QUALIFIER_NONE:
    case slvSTORAGE_QUALIFIER_CONST_IN:
    case slvSTORAGE_QUALIFIER_IN:
    case slvSTORAGE_QUALIFIER_OUT:
    case slvSTORAGE_QUALIFIER_INOUT:
    case slvSTORAGE_QUALIFIER_INSTANCE_ID:
    case slvSTORAGE_QUALIFIER_VERTEX_ID:
    case slvSTORAGE_QUALIFIER_SHARED:
        if (Name->type == slvFUNC_NAME || Name->type == slvPARAMETER_NAME)
        {
            /** SKIP THE COUNT FOR FUNCTION ARGUMENTS - DELAY TILL CODE GENERATION */
            ;
        }
        else
        {
            ObjectCounter->variableCount++;
            Name->context.isCounted = gcvTRUE;
        }
        break;

    case slvSTORAGE_QUALIFIER_UNIFORM:
    case slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER:
        ObjectCounter->uniformCount++;
        Name->context.isCounted = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_ATTRIBUTE:
    case slvSTORAGE_QUALIFIER_VARYING_IN:
    case slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER:
        ObjectCounter->attributeCount++;
        Name->context.isCounted = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_VARYING_OUT:
    case slvSTORAGE_QUALIFIER_FRAGMENT_OUT:
    case slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER:
    case slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER:
        ObjectCounter->outputCount += logicalRegCount;
        Name->context.isCounted = gcvTRUE;

        if (qualifier == slvSTORAGE_QUALIFIER_VARYING_OUT)
        {
            ObjectCounter->variableCount++;
        }
        break;

    case slvSTORAGE_QUALIFIER_CONST:
        /* skip counting constant variable */
        break;

    case slvSTORAGE_QUALIFIER_BUFFER:
        break;

    case slvSTORAGE_QUALIFIER_IN_IO_BLOCK:
        logicalRegCount = slsDATA_TYPE_GetLogicalOperandCount(Name->dataType, gcvFALSE);
        ObjectCounter->attributeCount += logicalRegCount;
        break;

    case slvSTORAGE_QUALIFIER_OUT_IO_BLOCK:
        logicalRegCount = slsDATA_TYPE_GetLogicalOperandCount(Name->dataType, gcvFALSE);
        ObjectCounter->outputCount += logicalRegCount;
        break;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CountVariable(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN slsNAME *Name,
    IN slsDATA_TYPE *DataType
    )
{
    gceSTATUS    status;
    gctUINT    count, i;
    slsNAME    *fieldName;

    gcmHEADER_ARG("Compiler=0x%x objectCounter=0x%x Name=0x%x "
                  "DataType=0x%x",
                  Compiler, ObjectCounter, Name, DataType);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    gcmVERIFY_ARGUMENT(Name);
    gcmASSERT(DataType);

    if (DataType->elementType == slvTYPE_STRUCT)
    {
        count = slsDATA_TYPE_GetLogicalCountForAnArray(DataType);

        gcmASSERT(Name->dataType->fieldSpace);

        for (i = 0; i < count; i++)
        {
            FOR_EACH_DLINK_NODE(&DataType->fieldSpace->names, slsNAME, fieldName)
            {
                gcmASSERT(fieldName->dataType);

                status = _CountVariable(Compiler,
                                        ObjectCounter,
                                        Name,
                                        fieldName->dataType);

                if (gcmIS_ERROR(status))
                {
                    gcmFOOTER();
                    return status;
                }
            }
        }
    }
    else
    {
        status = _CountVariableOrArray(Compiler,
                                       ObjectCounter,
                                       Name,
                                       DataType);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
slsNAME_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN slsNAME *Name
    )
{
    gceSTATUS  status;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Name=0x%x",
                  Compiler, ObjectCounter, Name);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    gcmVERIFY_ARGUMENT(Name);
    gcmASSERT(Name->dataType);
    gcmASSERT(Name->type == slvVARIABLE_NAME
                || Name->type == slvPARAMETER_NAME
                || Name->type == slvFUNC_NAME);

    if (Name->context.isCounted)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (Name->type == slvPARAMETER_NAME && Name->u.parameterInfo.aliasName != gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    do {
        status = _CountVariable(Compiler,
                                ObjectCounter,
                                Name,
                                Name->dataType);

        if (gcmIS_ERROR(status))
            break;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloIR_VARIABLE_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_VARIABLE Variable,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS  status;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Variable=0x%x",
                  Compiler, ObjectCounter, Variable);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Variable, slvIR_VARIABLE);
    gcmASSERT(Parameters);

    gcmASSERT(Variable->name);

    if (ObjectCounter &&
        ObjectCounter->codeGenerator->currentIterationContext &&
        ObjectCounter->codeGenerator->currentIterationContext->u.unrolledInfo.loopIndexName == Variable->name &&
        Parameters &&
        Parameters->needLOperand)
    {
        Variable->name->u.variableInfo.isCanUsedAsUnRollLoopIndex = gcvFALSE;
    }

    /* Allocate all logical registers */
    status = slsNAME_Count(Compiler,
                           ObjectCounter,
                           Variable->name);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_SET_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_SET Set,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    sloIR_BASE              member;
    slsGEN_CODE_PARAMETERS  memberParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Set=0x%x",
                  Compiler, ObjectCounter, Set);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(Set, slvIR_SET);

    switch (Set->type)
    {
    case slvDECL_SET:
        FOR_EACH_DLINK_NODE(&Set->members, struct _sloIR_BASE, member)
        {
            slsGEN_CODE_PARAMETERS_Initialize(&memberParameters, gcvFALSE, gcvFALSE);
            /* Count through members in the set */
            status = sloIR_OBJECT_Accept(Compiler,
                                         member,
                                         &ObjectCounter->visitor,
                                         &memberParameters);
            slsGEN_CODE_PARAMETERS_Finalize(&memberParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }
        break;

    case slvSTATEMENT_SET:
        if (Set->funcName != gcvNULL)
        {
            status = _CountFuncResources(Compiler,
                                         ObjectCounter,
                                         Set->funcName);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }

        FOR_EACH_DLINK_NODE(&Set->members, struct _sloIR_BASE, member)
        {
            slsGEN_CODE_PARAMETERS_Initialize(&memberParameters, gcvFALSE, gcvFALSE);

            status = sloIR_OBJECT_Accept(Compiler,
                                         member,
                                         &ObjectCounter->visitor,
                                         &memberParameters);

            slsGEN_CODE_PARAMETERS_Finalize(&memberParameters);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }
        }
        break;

    case slvEXPR_SET:
        gcmASSERT(0);
        break;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_CONSTANT_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_CONSTANT Constant,
    IN OUT slsGEN_CODE_PARAMETERS * Parameters
    )
{
    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Constant=0x%x",
                  Compiler, ObjectCounter, Constant);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Constant, slvIR_CONSTANT);

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_LABEL_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_LABEL Label,
    IN OUT slsGEN_CODE_PARAMETERS * Parameters
    )
{
    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Label=0x%x",
                  Compiler, ObjectCounter, Label);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(Label, slvIR_LABEL);

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_UNARY_EXPR_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_UNARY_EXPR UnaryExpr,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    slsGEN_CODE_PARAMETERS  operandParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x UnaryExpr=0x%x",
                  Compiler, ObjectCounter, UnaryExpr);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(UnaryExpr, slvIR_UNARY_EXPR);

    /* Generate the code of the operand */
    gcmASSERT(UnaryExpr->operand);

    switch (UnaryExpr->type)
    {
    case slvUNARY_FIELD_SELECTION:
    case slvUNARY_COMPONENT_SELECTION:
        slsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                                          Parameters->needLOperand,
                                          Parameters->needROperand);
        break;

    case slvUNARY_POST_INC:
    case slvUNARY_POST_DEC:
    case slvUNARY_PRE_INC:
    case slvUNARY_PRE_DEC:
        slsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                                          gcvTRUE,
                                          gcvTRUE);
        break;

    case slvUNARY_NEG:
    case slvUNARY_NOT:
    case slvUNARY_NOT_BITWISE:
        slsGEN_CODE_PARAMETERS_Initialize(&operandParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        break;

    default:
        break;
    }

    status = sloIR_OBJECT_Accept(Compiler,
                                 &UnaryExpr->operand->base,
                                 &ObjectCounter->visitor,
                                 &operandParameters);

    slsGEN_CODE_PARAMETERS_Finalize(&operandParameters);

    gcmFOOTER();
    return status;
}

gceSTATUS
sloIR_BINARY_EXPR_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_BINARY_EXPR BinaryExpr,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    slsGEN_CODE_PARAMETERS leftParameters, rightParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x BinaryExpr=0x%x",
                  Compiler, ObjectCounter, BinaryExpr);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(BinaryExpr, slvIR_BINARY_EXPR);
    gcmASSERT(Parameters);

    /* Count through the left operand */
    gcmASSERT(BinaryExpr->leftOperand);

    switch (BinaryExpr->type)
    {
    case slvBINARY_SUBSCRIPT:
        slsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                                          Parameters->needLOperand,
                                          Parameters->needROperand);
        slsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                                          gcvFALSE,
                                          Parameters->needLOperand || Parameters->needROperand);
        break;

    case slvBINARY_ADD:
    case slvBINARY_SUB:
    case slvBINARY_MUL:
    case slvBINARY_DIV:
    case slvBINARY_MOD:

    case slvBINARY_GREATER_THAN:
    case slvBINARY_LESS_THAN:
    case slvBINARY_GREATER_THAN_EQUAL:
    case slvBINARY_LESS_THAN_EQUAL:

    case slvBINARY_EQUAL:
    case slvBINARY_NOT_EQUAL:
    case slvBINARY_XOR:

    case slvBINARY_AND:

    case slvBINARY_OR:

    case slvBINARY_AND_BITWISE:
    case slvBINARY_OR_BITWISE:
    case slvBINARY_XOR_BITWISE:

    case slvBINARY_LSHIFT:
    case slvBINARY_RSHIFT:
        slsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        slsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        break;

    case slvBINARY_SEQUENCE:
        slsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                                          gcvFALSE,
                                          gcvFALSE);
        slsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        break;

    case slvBINARY_ASSIGN:
        slsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                                          gcvTRUE,
                                          Parameters->needROperand);
        slsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                                          gcvFALSE,
                                          gcvTRUE);
        break;

    case slvBINARY_LEFT_ASSIGN:
    case slvBINARY_RIGHT_ASSIGN:

    case slvBINARY_AND_ASSIGN:
    case slvBINARY_XOR_ASSIGN:
    case slvBINARY_OR_ASSIGN:

    case slvBINARY_MUL_ASSIGN:
    case slvBINARY_DIV_ASSIGN:
    case slvBINARY_ADD_ASSIGN:
    case slvBINARY_SUB_ASSIGN:
    case slvBINARY_MOD_ASSIGN:
        slsGEN_CODE_PARAMETERS_Initialize(&leftParameters,
                                          gcvTRUE,
                                          gcvTRUE);
        slsGEN_CODE_PARAMETERS_Initialize(&rightParameters,
                                          gcvFALSE,
                                          gcvTRUE);
        break;

    default:
        break;
    }

    status = sloIR_OBJECT_Accept(Compiler,
                                 &BinaryExpr->leftOperand->base,
                                 &ObjectCounter->visitor,
                                 &leftParameters);
    if (gcmIS_ERROR(status)) {
       gcmFOOTER();
       return status;
    }

    /* Count through the right operand */
    status = sloIR_OBJECT_Accept(Compiler,
                                 &BinaryExpr->rightOperand->base,
                                 &ObjectCounter->visitor,
                                 &rightParameters);

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER();
       return status;
    }

    slsGEN_CODE_PARAMETERS_Finalize(&leftParameters);
    slsGEN_CODE_PARAMETERS_Finalize(&rightParameters);

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_SELECTION_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_SELECTION Selection,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    gctBOOL                 emptySelection;
    slsGEN_CODE_PARAMETERS  condParameters, trueParameters, falseParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Selection=0x%x",
                  Compiler, ObjectCounter, Selection);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(Selection, slvIR_SELECTION);
    gcmASSERT(Selection->condExpr);
    gcmASSERT(Parameters);

    emptySelection = (Selection->trueOperand == gcvNULL && Selection->falseOperand == gcvNULL);

    if (emptySelection)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvFALSE);

        status = sloIR_OBJECT_Accept(Compiler,
                                     &Selection->condExpr->base,
                                     &ObjectCounter->visitor,
                                     &condParameters);

        slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

        gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
        return gcvSTATUS_OK;
    }

    slsGEN_CODE_PARAMETERS_Initialize(&condParameters, gcvFALSE, gcvFALSE);
    status = sloIR_OBJECT_Accept(Compiler,
                                 &Selection->condExpr->base,
                                 &ObjectCounter->visitor,
                                 &condParameters);
    slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if (Selection->trueOperand != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&trueParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        status = sloIR_OBJECT_Accept(Compiler,
                                     Selection->trueOperand,
                                     &ObjectCounter->visitor,
                                     &trueParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&trueParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    /* Generate the code of the false operand */
    if (Selection->falseOperand != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&falseParameters,
                                          gcvFALSE,
                                          Parameters->needROperand);
        status = sloIR_OBJECT_Accept(Compiler,
                                     Selection->falseOperand,
                                     &ObjectCounter->visitor,
                                     &falseParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&falseParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_SWITCH_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_SWITCH Switch,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS               status;
    slsGEN_CODE_PARAMETERS  condParameters, switchBodyParameters;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x Switch=0x%x",
                  Compiler, ObjectCounter, Switch);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(Switch, slvIR_SWITCH);
    gcmASSERT(Switch->condExpr);
    gcmASSERT(Parameters);

    slsGEN_CODE_PARAMETERS_Initialize(&condParameters,
                                      gcvFALSE,
                                      gcvTRUE);
    status = sloIR_OBJECT_Accept(Compiler,
                                 &Switch->condExpr->base,
                                 &ObjectCounter->visitor,
                                 &condParameters);
    slsGEN_CODE_PARAMETERS_Finalize(&condParameters);

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER();
       return status;
    }

    if (Switch->switchBody != gcvNULL)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&switchBodyParameters, gcvFALSE, gcvFALSE);
        status = sloIR_OBJECT_Accept(Compiler,
                                     Switch->switchBody,
                                     &ObjectCounter->visitor,
                                     &switchBodyParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&switchBodyParameters);

        if (gcmIS_ERROR(status))
        {
           gcmFOOTER();
           return status;
        }
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_POLYNARY_EXPR_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    slsGEN_CODE_PARAMETERS operandsParameters;
    sloIR_EXPR operand;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x PolynaryExpr=0x%x",
                  Compiler, ObjectCounter, PolynaryExpr);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(Parameters);

    if (PolynaryExpr->type == slvPOLYNARY_FUNC_CALL)
    {
        /* Check if it is a atomic memory function. */
        if (slsFUNC_HAS_FLAG(&(PolynaryExpr->funcName->u.funcInfo), slvFUNC_ATOMIC))
        {
            /* Currently always save atomic-related opcode into ATOMADD. */
            ObjectCounter->opcodeCount[slvOPCODE_ATOMADD]++;
        }

        /* Allocate the function resources */
        if (!PolynaryExpr->funcName->isBuiltIn)
        {
           status = _CountFuncResources(Compiler,
                                        ObjectCounter,
                                        PolynaryExpr->funcName);

           if (gcmIS_ERROR(status))
           {
               gcmFOOTER();
               return status;
           }
        }
    }

    if (PolynaryExpr->operands == gcvNULL)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_EXPR, operand)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&operandsParameters,
                                          gcvFALSE,
                                          gcvTRUE);
        status = sloIR_OBJECT_Accept(Compiler,
                                        &operand->base,
                                        &ObjectCounter->visitor,
                                        &operandsParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&operandsParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloIR_VIV_Asm_Count(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter,
    IN sloIR_VIV_ASM     VivAsm,
    IN OUT slsGEN_CODE_PARAMETERS *Parameters
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    slsGEN_CODE_PARAMETERS operandsParameters;
    sloIR_EXPR operand;

    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x VivAsm=0x%x",
                  Compiler, ObjectCounter, VivAsm);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);
    slmVERIFY_IR_OBJECT(VivAsm, slvIR_VIV_ASM);
    gcmASSERT(Parameters);

    ObjectCounter->vivAsmCount++;

    if (VivAsm->operands == gcvNULL)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    FOR_EACH_DLINK_NODE(&VivAsm->operands->members, struct _sloIR_EXPR, operand)
    {
        slsGEN_CODE_PARAMETERS_Initialize(&operandsParameters,
                                          gcvFALSE,
                                          gcvTRUE);
        status = sloIR_OBJECT_Accept(Compiler,
                                        &operand->base,
                                        &ObjectCounter->visitor,
                                        &operandsParameters);
        slsGEN_CODE_PARAMETERS_Finalize(&operandsParameters);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }
    gcmFOOTER_ARG("*Parameters=0x%x", *Parameters);
    return gcvSTATUS_OK;
}

gceSTATUS
sloOBJECT_COUNTER_Construct(
    IN sloCOMPILER Compiler,
    OUT sloOBJECT_COUNTER *ObjectCounter
    )
{
    gceSTATUS           status;
    sloOBJECT_COUNTER   objectCounter;
    gctPOINTER          pointer;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    gcmASSERT(ObjectCounter);

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(struct _sloOBJECT_COUNTER),
                                  &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(struct _sloOBJECT_COUNTER));

    objectCounter = (sloOBJECT_COUNTER)pointer;

    /* Initialize the visitor */
    objectCounter->visitor.object.type          = slvOBJ_OBJECT_COUNTER;

    objectCounter->visitor.visitSet             =
                    (sltVISIT_SET_FUNC_PTR)sloIR_SET_Count;

    objectCounter->visitor.visitIteration       =
                    (sltVISIT_ITERATION_FUNC_PTR)sloIR_ITERATION_Count;

    objectCounter->visitor.visitJump            =
                    (sltVISIT_JUMP_FUNC_PTR)sloIR_JUMP_Count;

    objectCounter->visitor.visitVariable        =
                    (sltVISIT_VARIABLE_FUNC_PTR)sloIR_VARIABLE_Count;

    objectCounter->visitor.visitConstant        =
                    (sltVISIT_CONSTANT_FUNC_PTR)sloIR_CONSTANT_Count;

    objectCounter->visitor.visitLabel        =
                    (sltVISIT_LABEL_FUNC_PTR)sloIR_LABEL_Count;

    objectCounter->visitor.visitUnaryExpr       =
                    (sltVISIT_UNARY_EXPR_FUNC_PTR)sloIR_UNARY_EXPR_Count;

    objectCounter->visitor.visitBinaryExpr      =
                    (sltVISIT_BINARY_EXPR_FUNC_PTR)sloIR_BINARY_EXPR_Count;

    objectCounter->visitor.visitSelection       =
                    (sltVISIT_SELECTION_FUNC_PTR)sloIR_SELECTION_Count;

    objectCounter->visitor.visitSwitch       =
                    (sltVISIT_SWITCH_FUNC_PTR)sloIR_SWITCH_Count;

    objectCounter->visitor.visitPolynaryExpr    =
                    (sltVISIT_POLYNARY_EXPR_FUNC_PTR)sloIR_POLYNARY_EXPR_Count;

    objectCounter->visitor.visitVivAsm          =
                    (sltVISIT_VIV_ASM_FUNC_PTR)sloIR_VIV_Asm_Count;

    *ObjectCounter = objectCounter;
    gcmFOOTER_ARG("*ObjectCounter=0x%x", *ObjectCounter);
    return gcvSTATUS_OK;
}

gceSTATUS
sloOBJECT_COUNTER_Destroy(
    IN sloCOMPILER Compiler,
    IN sloOBJECT_COUNTER ObjectCounter
    )
{
    gcmHEADER_ARG("Compiler=0x%x ObjectCounter=0x%x",
                  Compiler, ObjectCounter);
    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_OBJECT(ObjectCounter, slvOBJ_OBJECT_COUNTER);;

    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, ObjectCounter));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
