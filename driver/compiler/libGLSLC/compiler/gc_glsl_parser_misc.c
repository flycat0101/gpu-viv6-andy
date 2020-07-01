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


#include "gc_glsl_parser.h"

extern int
yyparse(
    void *
    );

extern void
yyrestart(
    gctPOINTER
    );

extern int
yylex(
    YYSTYPE * pyylval,
    sloCOMPILER Compiler
    )
{
    int tokenType;

    gcmHEADER();

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);

    tokenType = sloCOMPILER_Scan(Compiler, &(pyylval->token));

    switch (tokenType)
    {
    case T_BOOL:
    case T_FLOAT:
    case T_DOUBLE:
    case T_INT:
    case T_UINT:
    case T_BVEC2:
    case T_BVEC3:
    case T_BVEC4:
    case T_IVEC2:
    case T_IVEC3:
    case T_IVEC4:
    case T_UVEC2:
    case T_UVEC3:
    case T_UVEC4:
    case T_VEC2:
    case T_VEC3:
    case T_VEC4:
    case T_MAT2:
    case T_MAT2X3:
    case T_MAT2X4:
    case T_MAT3:
    case T_MAT3X2:
    case T_MAT3X4:
    case T_MAT4:
    case T_MAT4X2:
    case T_MAT4X3:
    case T_DVEC2:
    case T_DVEC3:
    case T_DVEC4:
    case T_DMAT2:
    case T_DMAT2X3:
    case T_DMAT2X4:
    case T_DMAT3:
    case T_DMAT3X2:
    case T_DMAT3X4:
    case T_DMAT4:
    case T_DMAT4X2:
    case T_DMAT4X3:
    case T_SAMPLER2D:
    case T_SAMPLERCUBE:
    case T_SAMPLERCUBEARRAY:

    case T_SAMPLER3D:
    case T_SAMPLER1DARRAY:
    case T_SAMPLER2DARRAY:
    case T_SAMPLER1DARRAYSHADOW:
    case T_SAMPLER2DARRAYSHADOW:
    case T_SAMPLER2DSHADOW:
    case T_SAMPLERCUBESHADOW:
    case T_SAMPLERCUBEARRAYSHADOW:

    case T_ISAMPLER2D:
    case T_ISAMPLERCUBE:
    case T_ISAMPLERCUBEARRAY:
    case T_ISAMPLER3D:
    case T_ISAMPLER2DARRAY:

    case T_USAMPLER2D:
    case T_USAMPLERCUBE:
    case T_USAMPLERCUBEARRAY:
    case T_USAMPLER3D:
    case T_USAMPLER2DARRAY:
    case T_SAMPLEREXTERNALOES:
    case T_SAMPLER2DMS:
    case T_ISAMPLER2DMS:
    case T_USAMPLER2DMS:
    case T_SAMPLER2DMSARRAY:
    case T_ISAMPLER2DMSARRAY:
    case T_USAMPLER2DMSARRAY:

    case T_SAMPLERBUFFER:
    case T_ISAMPLERBUFFER:
    case T_USAMPLERBUFFER:

    case T_SAMPLER1D:
    case T_ISAMPLER1D:
    case T_USAMPLER1D:
    case T_SAMPLER1DSHADOW:
    case T_SAMPLER2DRECT:
    case T_ISAMPLER2DRECT:
    case T_USAMPLER2DRECT:
    case T_SAMPLER2DRECTSHADOW:
    case T_ISAMPLER1DARRAY:
    case T_USAMPLER1DARRAY:

    case T_IMAGE2D:
    case T_IIMAGE2D:
    case T_UIMAGE2D:
    case T_IMAGE2DARRAY:
    case T_IIMAGE2DARRAY:
    case T_UIMAGE2DARRAY:
    case T_IMAGE3D:
    case T_IIMAGE3D:
    case T_UIMAGE3D:
    case T_IMAGECUBE:
    case T_IMAGECUBEARRAY:
    case T_IIMAGECUBE:
    case T_IIMAGECUBEARRAY:
    case T_UIMAGECUBE:
    case T_UIMAGECUBEARRAY:

    case T_IMAGEBUFFER:
    case T_IIMAGEBUFFER:
    case T_UIMAGEBUFFER:

    case T_VOID:
    case T_TYPE_NAME:
        gcmVERIFY_OK(sloCOMPILER_SetScannerState(Compiler, slvSCANNER_AFTER_TYPE));
        break;

    case T_LAYOUT:
        if (sloCOMPILER_IsHaltiVersion(Compiler) &&
            sloCOMPILER_GetScannerState(Compiler) == slvSCANNER_NORMAL) {
           gcmVERIFY_OK(sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NO_KEYWORD));
        }
        break;

    default:
        if (sloCOMPILER_IsHaltiVersion(Compiler) &&
            sloCOMPILER_GetScannerState(Compiler) == slvSCANNER_NO_KEYWORD) {
            break;
        }
        gcmVERIFY_OK(sloCOMPILER_SetScannerState(Compiler, slvSCANNER_NORMAL));
        break;
    }

    gcmFOOTER_ARG("%d", tokenType);
    return tokenType;
}

gceSTATUS
sloCOMPILER_Parse(
    IN sloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[]
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
#ifdef SL_SCAN_ONLY
    slsLexToken     token;
#endif

    gcmHEADER_ARG("Compiler=0x%x StringCount=%u Strings=0x%x",
                  Compiler, StringCount, Strings);

#ifdef SL_SCAN_ONLY
    gcoOS_ZeroMemory(&token, gcmSIZEOF(token));
#endif

    status = sloCOMPILER_MakeCurrent(Compiler, StringCount, Strings);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    /* Need to initialize default precision in global name space from its parent which
       is the builtin name space */
    {
        slsNAME_SPACE *globalSpace, *parentSpace;

        globalSpace = sloCOMPILER_GetGlobalSpace(Compiler);
        parentSpace = globalSpace->parent;
        if(parentSpace)
        {
            /* Propogate default precision value from parent */

            gcoOS_MemCopy(globalSpace->defaultPrecision,
                          parentSpace->defaultPrecision,
                          (gctSIZE_T)sizeof(sltPRECISION_QUALIFIER) * slvTYPE_TOTAL_COUNT);
        }
    }

#ifdef SL_SCAN_ONLY
    while (sloCOMPILER_Scan(Compiler, &token) != T_EOF);
#else
    yyInitScanner();
    yyrestart(gcvNULL);

    if (yyparse(Compiler) != 0) status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    slScanDeleteBuffer(Compiler);
#endif

    gcmFOOTER();
    return status;
}

static gceSTATUS
_ReportErrorForDismatchedType(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                    sloCOMPILER_GetCurrentLineNo(Compiler),
                                    sloCOMPILER_GetCurrentStringNo(Compiler),
                                    slvREPORT_ERROR,
                                    "require a matching typed expression"));
    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;

    return status;
}

static gceSTATUS
_CommonCheckForVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN sloIR_EXPR Initializer
    )
{
    if ((slsDATA_TYPE_IsSampler(DataType) || slsDATA_TYPE_IsImage(DataType)) &&
        (DataType->qualifiers.storage != slvSTORAGE_QUALIFIER_UNIFORM))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "Sampler/Image can't be declared without uniform qualifier for global declaration"));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    /* With initializer. */
    if (Initializer != gcvNULL)
    {
        gcePATCH_ID patchId = sloCOMPILER_GetPatchID(Compiler);

        if (!sloCOMPILER_IsOGLVersion(Compiler) &&
            sloCOMPILER_GetCurrentSpace(Compiler) == sloCOMPILER_GetGlobalSpace(Compiler) &&
            (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_NONE || DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST) &&
            sloIR_OBJECT_GetType(&Initializer->base) != slvIR_CONSTANT &&
            /* WAR for some APPs. */
            (patchId != gcvPATCH_YOUILABS_SHADERTEST && !gcdPROC_IS_WEBGL(patchId)))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            sloCOMPILER_GetCurrentLineNo(Compiler),
                                            sloCOMPILER_GetCurrentStringNo(Compiler),
                                            slvREPORT_ERROR,
                                            "In declarations of global variables with no storage "
                                            "qualifier or with a const qualifier, "
                                            "any initializer must be a constant expression"));
            return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForArraysOfArrays(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Identifier,
    IN slsDATA_TYPE *DataType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sleSHADER_TYPE shaderType;

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x DataType=0x%x",
                  Compiler, Identifier, DataType);

    shaderType = Compiler->shaderType;

    if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN &&
        shaderType != slvSHADER_TYPE_TCS &&
        shaderType != slvSHADER_TYPE_TES &&
        shaderType != slvSHADER_TYPE_GS)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                      Identifier->lineNo,
                                      Identifier->stringNo,
                                      slvREPORT_ERROR,
                                      "Shader input '%s' cannot be arrays of arrays",
                                      Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if ((DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_OUT ||
         DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT) &&
        shaderType != slvSHADER_TYPE_TCS)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                      Identifier->lineNo,
                                      Identifier->stringNo,
                                      slvREPORT_ERROR,
                                      "Shader output '%s' cannot be arrays of arrays",
                                      Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_CheckErrorAsConstantExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr,
    OUT sloIR_CONSTANT * Constant
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x Constant=0x%x",
                  Compiler, Expr, Constant);

    gcmASSERT(Expr);
    gcmASSERT(Constant);

    if (sloIR_OBJECT_GetType(&Expr->base) != slvIR_CONSTANT)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Expr->base.lineNo,
                                        Expr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a constant expression"));

        *Constant = gcvNULL;

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    *Constant = (sloIR_CONSTANT)Expr;

    gcmFOOTER_ARG("*Constant=0x%x", *Constant);
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsScalarIntConstantExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr,
    OUT sloIR_CONSTANT * Constant
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x Constant=0x%x",
                  Compiler, Expr, Constant);

    gcmASSERT(Expr);
    gcmASSERT(Constant);

    if (sloIR_OBJECT_GetType(&Expr->base) != slvIR_CONSTANT)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Expr->base.lineNo,
                                        Expr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a constant expression"));

        *Constant = gcvNULL;

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
    else if(!slsDATA_TYPE_IsInt(Expr->dataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Expr->base.lineNo,
                                        Expr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar integer constant expression"));

        *Constant = gcvNULL;

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    *Constant = (sloIR_CONSTANT)Expr;

    gcmFOOTER_ARG("*Constant=0x%x", *Constant);
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsLValueExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr
    )
{
    sloIR_UNARY_EXPR    unaryExpr;
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x",
                  Compiler, Expr);

    gcmASSERT(Expr);
    gcmASSERT(Expr->dataType);

    switch (Expr->dataType->qualifiers.storage)
    {
    case slvSTORAGE_QUALIFIER_CONST:
    case slvSTORAGE_QUALIFIER_UNIFORM:
    case slvSTORAGE_QUALIFIER_ATTRIBUTE:
    case slvSTORAGE_QUALIFIER_VARYING_IN:
    case slvSTORAGE_QUALIFIER_CONST_IN:
    case slvSTORAGE_QUALIFIER_INSTANCE_ID:
    case slvSTORAGE_QUALIFIER_VERTEX_ID:
    case slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER:
        {
            gctBOOL bInvalidCase = gcvTRUE;

            if (Expr->base.vptr->type == slvIR_VARIABLE)
            {
                sloIR_VARIABLE variable = (sloIR_VARIABLE) &Expr->base;

                /* Some uniforms can be declared with a initializer. */
                if (variable->name->u.variableInfo.declareUniformWithInit || variable->name->u.variableInfo.treatConstArrayAsUniform)
                {
                    gcmASSERT(Expr->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM);
                    bInvalidCase = gcvFALSE;
                }
            }

            if (bInvalidCase)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                Expr->base.lineNo,
                                                Expr->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require a l-value expression"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
    }

    if (sloIR_OBJECT_GetType(&Expr->base) == slvIR_UNARY_EXPR)
    {
        unaryExpr = (sloIR_UNARY_EXPR)Expr;

        if (unaryExpr->type == slvUNARY_COMPONENT_SELECTION
            && slIsRepeatedComponentSelection(&unaryExpr->u.componentSelection))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Expr->base.lineNo,
                                            Expr->base.stringNo,
                                            slvREPORT_ERROR,
                                            "The l-value expression select repeated"
                                            " components or swizzles"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_EvaluateExprToArrayLength(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr,
    IN gctBOOL Delete,
    IN gctBOOL CheckLengthValue,
    OUT gctINT * ArrayLength
    )
{
    gceSTATUS       status;
    sloIR_CONSTANT  constant;

    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x ArrayLength=0x%x",
                  Compiler, Expr, ArrayLength);

    if (ArrayLength)
    {
        *ArrayLength = 0;
    }

    status = _CheckErrorAsConstantExpr(
                                    Compiler,
                                    Expr,
                                    &constant);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if (constant->exprBase.dataType == gcvNULL
        || !slsDATA_TYPE_IsInt(constant->exprBase.dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Expr->base.lineNo,
                                        Expr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integral constant expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (constant->valueCount > 1
        || constant->values == gcvNULL
        || (constant->values[0].intValue <= 0 && CheckLengthValue))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Expr->base.lineNo,
                                        Expr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "the array length must be greater than zero"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (ArrayLength)
    {
        *ArrayLength = (gctUINT)constant->values[0].intValue;
    }

    if (Delete)
    {
        gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &constant->exprBase.base));
    }

    gcmFOOTER_ARG("*ArrayLength=%d", *ArrayLength);
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForArraysOfArraysLengthValue(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * LengthList,
    IN gctBOOL isSSBOMember
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloIR_EXPR operand;
    gctINT arrayLength = 0;
    gctBOOL isUnsizedAppear = gcvFALSE;
    gctBOOL allowUnsized = gcvFALSE;

    FOR_EACH_DLINK_NODE_REVERSELY(LengthList, struct _sloIR_EXPR, operand)
    {
        /* Right now, we only allow first dimension array is unsized. */
        allowUnsized = (isSSBOMember &&
                       ((gctPOINTER)LengthList->prev == (gctPOINTER)operand));

        status = _EvaluateExprToArrayLength(Compiler,
                                   operand,
                                   gcvFALSE,
                                   !allowUnsized,
                                   &arrayLength);
        if (gcmIS_ERROR(status))
        {
            break;
        }

        if (arrayLength == -1)
        {
            if (isUnsizedAppear)
            {
                sloCOMPILER_Report(Compiler,
                                   operand->base.lineNo,
                                   operand->base.stringNo,
                                   slvREPORT_ERROR,
                                   "There are two unsized dimensions within an arrays of arrays.");

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }
            else
            {
                isUnsizedAppear = gcvTRUE;
            }
        }
    }
    return status;
}

static gceSTATUS
_ParseArraysOfArraysDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsDLINK_LIST * LengthList,
    IN gctBOOL CheckLengthValue,
    OUT slsDATA_TYPE **ArrayDataType
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    sloIR_EXPR operand;
    gctINT arrayLengthCount = 0, i = 0;
    gctINT * arrayLengthList = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    slsDLINK_LIST * list;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x LengthList=0x%x ArrayDataType=0x%x",
                  Compiler, DataType, LengthList, ArrayDataType);

    gcmASSERT(DataType);
    gcmASSERT(ArrayDataType);

    *ArrayDataType = gcvNULL;

    slsDLINK_NODE_COUNT(LengthList, arrayLengthCount);

    gcmASSERT(arrayLengthCount > 1);

    gcmONERROR(sloCOMPILER_Allocate(
                                Compiler,
                                (gctSIZE_T)(arrayLengthCount * gcmSIZEOF(gctINT)),
                                &pointer));

    gcoOS_ZeroMemory(pointer, arrayLengthCount * gcmSIZEOF(gctINT));
    arrayLengthList = pointer;

    FOR_EACH_DLINK_NODE_REVERSELY(LengthList, struct _sloIR_EXPR, operand)
    {
        _EvaluateExprToArrayLength(Compiler,
                                   operand,
                                   gcvFALSE,
                                   CheckLengthValue,
                                   &arrayLengthList[i]);
        i++;
    }

    gcmASSERT(i == arrayLengthCount);

    gcmONERROR(sloCOMPILER_CreateArraysOfArraysDataType(Compiler,
                                                        DataType,
                                                        arrayLengthCount,
                                                        arrayLengthList,
                                                        gcvFALSE,
                                                        ArrayDataType));

OnError:
    if (arrayLengthList != gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, arrayLengthList));
    }

    list = LengthList->next;
    for (i = 0; i < arrayLengthCount; i++)
    {
        operand = (sloIR_EXPR)list;
        list = list->next;
        gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &operand->base));
    }

    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, LengthList));

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_SetUniformBlockMemberActive(
    IN slsNAME *Block,
    IN gctINT ArrayIndex,
    IN slsNAME *BlockMember
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsINTERFACE_BLOCK_MEMBER *blockMemberInfo;

    gcmHEADER_ARG("Block=0x%x ArrayIndex=%d BlockMember=0x%x",
                  Block, ArrayIndex, BlockMember);

    FOR_EACH_DLINK_NODE(&Block->u.interfaceBlockContent.members, slsINTERFACE_BLOCK_MEMBER, blockMemberInfo) {
        if(BlockMember == blockMemberInfo->name) {
            blockMemberInfo->isActive = gcvTRUE;
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
    gcmASSERT(0);

    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    gcmFOOTER();
    return status;
}

sloIR_EXPR
slParseVariableIdentifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsNAME *       name;
    sloIR_CONSTANT  constant;
    sloIR_VARIABLE  variable;
    sloIR_EXPR      expr;

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x",
                  Compiler, Identifier);

    gcmASSERT(Identifier);

    status = sloCOMPILER_SearchName(Compiler,
                                    Identifier->u.identifier,
                                    gcvTRUE,
                                    &name);

    if (status != gcvSTATUS_OK)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "undefined identifier: '%s'",
                                        Identifier->u.identifier));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if(name->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER)
    {
        status = _SetUniformBlockMemberActive(name->u.variableInfo.interfaceBlock, 0, name);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    switch (name->type)
    {
    case slvVARIABLE_NAME:
        name->u.variableInfo.isReferenced = gcvTRUE;
        break;

    case slvPARAMETER_NAME:
        break;

    case slvFUNC_NAME:
    case slvSTRUCT_NAME:
    case slvFIELD_NAME:
    case slvINTERFACE_BLOCK_NAME:
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "'%s' isn't a variable",
                                        name->symbol));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (name->type == slvVARIABLE_NAME && name->isBuiltIn &&
        name->dataType->qualifiers.storage != slvSTORAGE_QUALIFIER_CONST)
    {
        gctCONST_STRING symbol;
        sltSTORAGE_QUALIFIER qualifier;

        status = slGetBuiltInVariableImplSymbol(Compiler,
                                                name->symbol,
                                                &symbol,
                                                &qualifier);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if(qualifier == slvSTORAGE_QUALIFIER_CONST && name->u.variableInfo.constant == gcvNULL)
        {
            sloIR_CONSTANT      constant;
            sluCONSTANT_VALUE   *values;

            if (gcmIS_SUCCESS(gcoOS_StrCmp(symbol, "#WorkGroupSize")))
            {
                slsLAYOUT_QUALIFIER layout[1];
                slsDATA_TYPE *dataType;
                gctPOINTER pointer;

                status = sloCOMPILER_GetDefaultLayout(Compiler,
                                                      layout,
                                                      slvSTORAGE_QUALIFIER_IN);
                if (gcmIS_ERROR(status))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    name->lineNo,
                                                    name->stringNo,
                                                    slvREPORT_ERROR,
                                                    "error in parsing special variable '%s'", name->symbol));

                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                if(!(layout->id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    name->lineNo,
                                                    name->stringNo,
                                                    slvREPORT_ERROR,
                                                    "input layout qualifiers local_size_x, local_size_y, local_size_z "
                                                    "have not been specified for special variable '%s'", name->symbol));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                gcmASSERT(slmDATA_TYPE_vectorSize_GET(name->dataType) == 3);
                status = sloCOMPILER_CloneDataType(Compiler,
                                                   slvSTORAGE_QUALIFIER_CONST,
                                                   name->dataType->qualifiers.precision,
                                                   name->dataType,
                                                   &dataType);
                if (gcmIS_ERROR(status))
                {
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                /* Create the constant */
                status = sloIR_CONSTANT_Construct(Compiler,
                                                  name->lineNo,
                                                  name->stringNo,
                                                  dataType,
                                                  &constant);
                if (gcmIS_ERROR(status))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    name->lineNo,
                                                    name->stringNo,
                                                    slvREPORT_ERROR,
                                                    "error in parsing special variable '%s'", name->symbol));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                status = sloCOMPILER_Allocate(Compiler,
                                              (gctSIZE_T)sizeof(sluCONSTANT_VALUE) * 3,
                                              &pointer);
                if (gcmIS_ERROR(status))
                {
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }
                values = pointer;

                /* Add the constant value */
                values[0].intValue = layout->workGroupSize[0];
                values[1].intValue = layout->workGroupSize[1];
                values[2].intValue = layout->workGroupSize[2];

                status = sloIR_CONSTANT_SetValues(Compiler,
                                                  constant,
                                                  3,
                                                  values);
                if (gcmIS_ERROR(status))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    name->lineNo,
                                                    name->stringNo,
                                                    slvREPORT_ERROR,
                                                    "error in parsing special variable '%s'", name->symbol));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                name->u.variableInfo.constant = constant;
                name->u.variableInfo.constant->variable = name;
            }
        }
    }

    if (name->type == slvVARIABLE_NAME && name->u.variableInfo.constant != gcvNULL)
    {
        status = sloIR_CONSTANT_Clone(Compiler,
                                      Identifier->lineNo,
                                      Identifier->stringNo,
                                      name->u.variableInfo.constant,
                                      &constant);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        expr = &constant->exprBase;
    }
    else
    {
        status = sloIR_VARIABLE_Construct(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        name,
                                        &variable);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        expr = &variable->exprBase;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<VARIABLE_IDENTIFIER line=\"%d\" string=\"%d\" name=\"%s\" />",
                                Identifier->lineNo,
                                Identifier->stringNo,
                                Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=0x%x", expr);
    return expr;
}

sloIR_EXPR
slParseIntConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * IntConstant
    )
{
    gceSTATUS           status;
    slsDATA_TYPE *      dataType;
    sloIR_CONSTANT      constant;
    sluCONSTANT_VALUE   value;

    gcmHEADER_ARG("Compiler=0x%x IntConstant=0x%x",
                  Compiler, IntConstant);

    gcmASSERT(IntConstant);

    /* Create the data type */
    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_INT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    /* Create the constant */
    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    IntConstant->lineNo,
                                    IntConstant->stringNo,
                                    dataType,
                                    &constant);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Add the constant value */
    value.intValue = IntConstant->u.constant.intValue;

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    constant,
                                    1,
                                    &value);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<INT_CONSTANT line=\"%d\" string=\"%d\" value=\"%d\" />",
                                IntConstant->lineNo,
                                IntConstant->stringNo,
                                IntConstant->u.constant.intValue));

    gcmFOOTER_ARG("<return>=0x%x", &constant->exprBase);
    return &constant->exprBase;
}

sloIR_EXPR
slParseUintConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * UintConstant
    )
{
    gceSTATUS           status;
    slsDATA_TYPE *      dataType;
    sloIR_CONSTANT      constant;
    sluCONSTANT_VALUE   value;

    gcmHEADER_ARG("Compiler=0x%x UintConstant=0x%x",
                  Compiler, UintConstant);

    gcmASSERT(UintConstant);

    /* Create the data type */
    status = sloCOMPILER_CreateDataType(Compiler,
                                        T_UINT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    /* Create the constant */
    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    UintConstant->lineNo,
                                    UintConstant->stringNo,
                                    dataType,
                                    &constant);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Add the constant value */
    value.uintValue = UintConstant->u.constant.uintValue;

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      constant,
                                      1,
                                      &value);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<UINT_CONSTANT line=\"%d\" string=\"%d\" value=\"%u\" />",
                                  UintConstant->lineNo,
                                  UintConstant->stringNo,
                                  UintConstant->u.constant.uintValue));

    gcmFOOTER_ARG("<return>=0x%x", &constant->exprBase);
    return &constant->exprBase;
}

sloIR_EXPR
slParseFloatConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * FloatConstant
    )
{
    gceSTATUS           status;
    slsDATA_TYPE *      dataType;
    sloIR_CONSTANT      constant;
    sluCONSTANT_VALUE   value;

    gcmHEADER_ARG("Compiler=0x%x FloatConstant=0x%x",
                  Compiler, FloatConstant);

    gcmASSERT(FloatConstant);

    /* Create the data type */
    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_FLOAT,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    /* Create the constant */
    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    FloatConstant->lineNo,
                                    FloatConstant->stringNo,
                                    dataType,
                                    &constant);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Add the constant value */
    value.floatValue = FloatConstant->u.constant.floatValue;

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    constant,
                                    1,
                                    &value);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FLOAT_CONSTANT line=\"%d\" string=\"%d\" value=\"%f\" />",
                                FloatConstant->lineNo,
                                FloatConstant->stringNo,
                                FloatConstant->u.constant.floatValue));

    gcmFOOTER_ARG("<return>=0x%x", &constant->exprBase);
    return &constant->exprBase;
}

sloIR_EXPR
slParseBoolConstant(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BoolConstant
    )
{
    gceSTATUS           status;
    slsDATA_TYPE *      dataType;
    sloIR_CONSTANT      constant;
    sluCONSTANT_VALUE   value;

    gcmHEADER_ARG("Compiler=0x%x BoolConstant=0x%x",
                  Compiler, BoolConstant);

    gcmASSERT(BoolConstant);

    /* Create the data type */
    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_BOOL,
                                        gcvNULL,
                                        &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    /* Create the constant */
    status = sloIR_CONSTANT_Construct(
                                    Compiler,
                                    BoolConstant->lineNo,
                                    BoolConstant->stringNo,
                                    dataType,
                                    &constant);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Add the constant value */
    value.boolValue = BoolConstant->u.constant.boolValue;

    status = sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    constant,
                                    1,
                                    &value);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<BOOL_CONSTANT line=\"%d\" string=\"%d\" value=\"%s\" />",
                                BoolConstant->lineNo,
                                BoolConstant->stringNo,
                                (BoolConstant->u.constant.boolValue)? "true" : "false"));

    gcmFOOTER_ARG("<return>=0x%x", &constant->exprBase);
    return &constant->exprBase;
}

gceSTATUS
_CheckErrorForSubscriptExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gctINT32        index;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the left operand */
    if (!slsDATA_TYPE_IsBVecOrIVecOrVec(LeftOperand->dataType)
        && !slsDATA_TYPE_IsMat(LeftOperand->dataType)
        && !slsDATA_TYPE_IsArray(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an array or matrix or vector typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* Check error for multiple subscript depth. */
    if (sloIR_OBJECT_GetType(&LeftOperand->base) ==  slvIR_BINARY_EXPR)
    {
        sloIR_EXPR expr = (sloIR_EXPR)(&LeftOperand->base);
        gctINT subScriptLevel = 1;
        gctINT arrayLevel = 0;

        if (((sloIR_BINARY_EXPR)expr)->type == slvBINARY_SUBSCRIPT)
        {
            while (sloIR_OBJECT_GetType(&expr->base) != slvIR_VARIABLE &&
                   sloIR_OBJECT_GetType(&expr->base) != slvIR_CONSTANT &&
                   sloIR_OBJECT_GetType(&expr->base) != slvIR_UNARY_EXPR)
            {
                sleBINARY_EXPR_TYPE type;
                gcmASSERT(sloIR_OBJECT_GetType(&expr->base) == slvIR_BINARY_EXPR);

                type = ((sloIR_BINARY_EXPR)expr)->type;

                if (type >= slvBINARY_ASSIGN)
                    break;

                expr = (sloIR_EXPR)(&((sloIR_BINARY_EXPR)expr)->leftOperand->base);
                subScriptLevel++;
            }
        }

        if (slsDATA_TYPE_IsArray(expr->dataType))
        {
            arrayLevel = expr->dataType->arrayLengthCount;
        }

        /*
        ** If this is a matrix, it have two more depth level.
        ** If this is a vector, it have one more depth level.
        */
        if (expr->dataType->matrixSize.columnCount != 0)
        {
            arrayLevel += 2;
        }
        else if (slmDATA_TYPE_vectorSize_GET(expr->dataType) != 0)
        {
            arrayLevel += 1;
        }

        if (subScriptLevel > arrayLevel)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            LeftOperand->base.lineNo,
                                            LeftOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "array dimension level exceed declared value"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if (sloIR_OBJECT_GetType(&expr->base) == slvIR_VARIABLE &&
            expr->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT &&
            sloIR_OBJECT_GetType(&RightOperand->base) != slvIR_CONSTANT
            )
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "Fragment shader outputs declared as arrays may only be indexed by a constant integral expression."));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    /* Check the right operand */
    if (!slsDATA_TYPE_IsInt(RightOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar integer expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* Check constant index range */
    if (sloIR_OBJECT_GetType(&RightOperand->base) == slvIR_CONSTANT)
    {
        gcmASSERT(((sloIR_CONSTANT)RightOperand)->valueCount == 1);

        index = ((sloIR_CONSTANT)RightOperand)->values[0].intValue;

        if (index < 0)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require a nonnegative index"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if (slsDATA_TYPE_IsBVecOrIVecOrVec(LeftOperand->dataType))
        {
            if ((gctUINT8) index >= slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->dataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "the index exceeds the vector type size"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (slsDATA_TYPE_IsMat(LeftOperand->dataType))
        {
            if (index >= slmDATA_TYPE_matrixColumnCount_GET(LeftOperand->dataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "the index exceeds the matrix type size"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else
        {
            gcmASSERT(slsDATA_TYPE_IsArray(LeftOperand->dataType));

            if (!slsDATA_TYPE_IsImplicitlySizedArray(LeftOperand->dataType) &&
                LeftOperand->dataType->arrayLength > 0 &&
                index >= LeftOperand->dataType->arrayLength)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "the index exceeds the array type size"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }

            if (slsDATA_TYPE_IsImplicitlySizedArray(LeftOperand->dataType) &&
                sloCOMPILER_IsOGLVersion(Compiler))
            {
                if (sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_VARIABLE)
                {
                    sloIR_VARIABLE variableExpr = (sloIR_VARIABLE)(&LeftOperand->base);
                    slsNAME *name = variableExpr->name;
                    if (name && name->dataType->arrayLength < index + 1)
                    {
                        name->dataType->arrayLength = index + 1;
                        name->dataType->arrayLengthList[0] = index + 1;
                        variableExpr->exprBase.dataType->arrayLength = index + 1;
                        variableExpr->exprBase.dataType->arrayLengthList[0] = index + 1;
                    }
                }
                else if (sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_UNARY_EXPR)
                {
                    sloIR_UNARY_EXPR unaryExpr = (sloIR_UNARY_EXPR)(&LeftOperand->base);

                    if (unaryExpr->type == slvUNARY_FIELD_SELECTION)
                    {
                        slsNAME *name = unaryExpr->u.fieldName;
                        if (name && name->dataType->arrayLength < index + 1)
                        {
                            name->dataType->arrayLength = index + 1;
                            name->dataType->arrayLengthList[0] = index + 1;
                            unaryExpr->exprBase.dataType->arrayLength = index + 1;
                            unaryExpr->exprBase.dataType->arrayLengthList[0] = index + 1;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
            sloEXTENSION extension = {0};
            extension.extension1 = slvEXTENSION1_GPU_SHADER5;
            if(slsDATA_TYPE_IsUnderlyingInterfaceBlock(LeftOperand->dataType) &&
               !slsDATA_TYPE_IsUnderlyingIOBlock(LeftOperand->dataType) &&
               !sloCOMPILER_ExtensionEnabled(Compiler, &extension))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "interface block array may only be indexed by a constant integral expression"));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }

            if(LeftOperand->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "Fragment shader outputs declared as arrays may only be indexed by a constant integral expression."));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        if (slsDATA_TYPE_IsSampler(LeftOperand->dataType) ||
            slsDATA_TYPE_IsImage(LeftOperand->dataType))
        {
            sloEXTENSION extension = {0};
            extension.extension1 = slvEXTENSION1_GPU_SHADER5;
            if (sloIR_OBJECT_GetType(&RightOperand->base) != slvIR_CONSTANT &&
                !sloCOMPILER_ExtensionEnabled(Compiler, &extension) )
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                " Samplers/Images can only be indexed with a constant integral expression."));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
    }


    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_EXPR
slParseSubscriptExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS           status;
    sloIR_CONSTANT      resultConstant;
    sloIR_BINARY_EXPR   binaryExpr;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    if (LeftOperand == gcvNULL || RightOperand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Check the UnSizeArray. */
    if (sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_VARIABLE && sloIR_OBJECT_GetType(&RightOperand->base) == slvIR_CONSTANT)
    {
        sloIR_VARIABLE variable = (sloIR_VARIABLE)(LeftOperand);
        gctINT arraySize = ((sloIR_CONSTANT)RightOperand)->values[0].intValue + 1;

        if (variable->name->symbol && gcmIS_SUCCESS(gcoOS_StrCmp(variable->name->symbol, "gl_ClipDistance")))
        {
            if (!variable->name->isUnsizeArraySet)
            {
                if (arraySize > variable->name->dataType->arrayLength)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    LeftOperand->base.lineNo,
                                                    LeftOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "The array index is larger than the array size."));

                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }

                variable->name->dataType->arrayLength = arraySize;
                variable->name->dataType->arrayLengthList[0] = arraySize;
                variable->name->isUnsizeArraySet = gcvTRUE;
            }
            else
            {
                if (arraySize > variable->name->dataType->arrayLength)
                {
                    variable->name->dataType->arrayLength = arraySize;
                    variable->name->dataType->arrayLengthList[0] = arraySize;
                }
            }
        }
    }

    /* Check error */
    status = _CheckErrorForSubscriptExpr(
                                        Compiler,
                                        LeftOperand,
                                        RightOperand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Constant calculation */
    if (sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_CONSTANT
        && sloIR_OBJECT_GetType(&RightOperand->base) == slvIR_CONSTANT)
    {
        status = sloIR_BINARY_EXPR_Evaluate(
                                            Compiler,
                                            slvBINARY_SUBSCRIPT,
                                            (sloIR_CONSTANT)LeftOperand,
                                            (sloIR_CONSTANT)RightOperand,
                                            &resultConstant);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        gcmFOOTER_ARG("<return>=0x%x", &resultConstant->exprBase);
        return &resultConstant->exprBase;
    }

    /* Create the binary expression */
    status = sloIR_BINARY_EXPR_Construct(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        LeftOperand->base.lineNo,
                                        slvBINARY_SUBSCRIPT,
                                        LeftOperand,
                                        RightOperand,
                                        &binaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<SUBSCRIPT_EXPR line=\"%d\" string=\"%d\" />",
                                LeftOperand->base.lineNo,
                                LeftOperand->base.stringNo));

    gcmFOOTER_ARG("<return>=0x%x", &binaryExpr->exprBase);
    return &binaryExpr->exprBase;
}

static gceSTATUS
_CheckErrorAsScalarConstructor(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr
    )
{
    gctUINT     operandCount;
    sloIR_EXPR  operand;

    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(
                                            Compiler,
                                            PolynaryExpr->operands,
                                            &operandCount));
    }

    if (operandCount != 1)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "require one expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(operand);
    gcmASSERT(operand->dataType);

    if (!slsDATA_TYPE_IsBoolOrBVec(operand->dataType)
        && !slsDATA_TYPE_IsIntOrIVec(operand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(operand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        operand->base.lineNo,
                                        operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an boolean or integer or floating-point"
                                        " typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsArrayConstructor(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT    operandCount;
    sloIR_EXPR operand;
    slsDATA_TYPE refDataType[1];
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);

    gcmASSERT(slsDATA_TYPE_IsArray(PolynaryExpr->exprBase.dataType));

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(Compiler,
                                              PolynaryExpr->operands,
                                              &operandCount));
    }

    if(PolynaryExpr->exprBase.dataType->arrayLength > 0 &&
       (gctINT)operandCount != PolynaryExpr->exprBase.dataType->arrayLength)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "expected number of elements for array not matching"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (PolynaryExpr->operands == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "null pointer not allow"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
    gcmASSERT(operand);
    gcmASSERT(operand->dataType);

    refDataType[0] = *(PolynaryExpr->exprBase.dataType);
    refDataType->arrayLength =
    refDataType->arrayLengthList[0] = 0;

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_EXPR, operand)
    {
        gcmASSERT(operand);
        gcmASSERT(operand->dataType);

        if (!sloCOMPILER_IsHaltiVersion(Compiler) && !slmDATA_TYPE_IsArithmeticType(operand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            operand->base.lineNo,
                                            operand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require any boolean or integer or floating-point"
                                            " typed expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
        {
            status = slMakeImplicitConversionForOperand(Compiler,
                                              operand,
                                              refDataType);
            if (gcmIS_ERROR(status)) {
                return status;
            }
        }
        else
        {
            sloIR_EXPR_SetToBeTheSameDataType(operand);
        }

        if (!slsDATA_TYPE_IsInitializableTo(refDataType, operand->toBeDataType))
        {
            status = _ReportErrorForDismatchedType(Compiler);
            gcmFOOTER();
            return status;
        }
    }

    PolynaryExpr->exprBase.dataType->arrayLength =
    PolynaryExpr->exprBase.dataType->arrayLengthList[0] = operandCount;
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsArraysOfArraysConstructor(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_POLYNARY_EXPR PolynaryExpr
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT    operandCount;
    sloIR_EXPR operand;
    slsDATA_TYPE * elementDataType = gcvNULL;
    gctINT i, length = -1;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);

    gcmASSERT(slsDATA_TYPE_IsArraysOfArrays(PolynaryExpr->exprBase.dataType));

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(Compiler,
                                              PolynaryExpr->operands,
                                              &operandCount));
    }

    PolynaryExpr->exprBase.dataType->arrayLengthList[0] = (gctINT)operandCount;
    PolynaryExpr->exprBase.dataType->arrayLength = (gctINT)operandCount;

    if (PolynaryExpr->operands == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "null pointer not allow"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_EXPR, operand)
    {
        gcmASSERT(operand);
        gcmASSERT(operand->dataType);

        if (length == -1)
        {
            elementDataType = operand->dataType;
            length = operand->dataType->arrayLength;
            if (length == 0)
            {
                length = 1;
            }
        }
        else
        {
            if (length != operand->dataType->arrayLength)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                slvREPORT_ERROR,
                                                " Array constructors have different array size for an array of arrays"));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }

        }

        if (!sloCOMPILER_IsHaltiVersion(Compiler) && !slmDATA_TYPE_IsArithmeticType(operand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            operand->base.lineNo,
                                            operand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require any boolean or integer or floating-point"
                                            " typed expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if (!slsDATA_TYPE_IsInitializableTo(elementDataType, operand->dataType)) {
            status = _ReportErrorForDismatchedType(Compiler);
            gcmFOOTER();
            return status;
        }
    }

    PolynaryExpr->exprBase.dataType->arrayLengthList[1] = length;

    for (i = 2; i < PolynaryExpr->exprBase.dataType->arrayLengthCount; i++)
    {
        if (PolynaryExpr->exprBase.dataType->arrayLengthList[i] == -1)
        {
            PolynaryExpr->exprBase.dataType->arrayLengthList[i] = elementDataType->arrayLengthList[i - 1];
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsVectorOrMatrixConstructor(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctBOOL IsVectorConstructor
    )
{
    gctUINT     operandCount;
    sloIR_EXPR  operand;
    gctUINT     operandDataSizes = 0;
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x IsVectorConstructor=%d",
                  Compiler, PolynaryExpr, IsVectorConstructor);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(
                                            Compiler,
                                            PolynaryExpr->operands,
                                            &operandCount));
    }

    if (operandCount == 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "require at least one expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _sloIR_EXPR, operand)
    {
        gcmASSERT(operand);
        gcmASSERT(operand->dataType);

        if (!slsDATA_TYPE_IsBoolOrBVec(operand->dataType)
            && !slsDATA_TYPE_IsIntOrIVec(operand->dataType)
            && !slsDATA_TYPE_IsFloatOrVecOrMat(operand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            operand->base.lineNo,
                                            operand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require any boolean or integer or floating-point"
                                            " typed expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
    }

        if (operandDataSizes >= slsDATA_TYPE_GetSize(PolynaryExpr->exprBase.dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "too many expressions in the constructor"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        operandDataSizes += slsDATA_TYPE_GetSize(operand->dataType);
    }

    if (operandCount == 1)
    {
        operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
        gcmASSERT(operand);

        if (IsVectorConstructor)
        {
            if (slsDATA_TYPE_IsScalar(operand->dataType))
            {
                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
        }
        else
        {
            if (slsDATA_TYPE_IsScalar(operand->dataType) || slsDATA_TYPE_IsMat(operand->dataType))
            {
                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
        }
    }

    if (operandDataSizes < slsDATA_TYPE_GetSize(PolynaryExpr->exprBase.dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "require more expressions"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsStructConstructor(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr
    )
{
    gctUINT         operandCount;
    sloIR_EXPR      operand;
    slsDATA_TYPE *  structDataType;
    slsNAME *       fieldName;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(
                                            Compiler,
                                            PolynaryExpr->operands,
                                            &operandCount));
    }

    if (operandCount == 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        slvREPORT_ERROR,
                                        "require at least one expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    structDataType = PolynaryExpr->exprBase.dataType;
    gcmASSERT(structDataType);
    gcmASSERT(slsDATA_TYPE_IsStruct(structDataType));

    for (fieldName = slsDLINK_LIST_First(&structDataType->fieldSpace->names, slsNAME),
            operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
        (slsDLINK_NODE *)fieldName != &structDataType->fieldSpace->names;
        fieldName = slsDLINK_NODE_Next(&fieldName->node, slsNAME),
            operand = slsDLINK_NODE_Next(&operand->base.node, struct _sloIR_EXPR))
    {
        if ((slsDLINK_NODE *)operand == &PolynaryExpr->operands->members)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "require more expressions"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if (!slsDATA_TYPE_IsEqual(fieldName->dataType, operand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            operand->base.lineNo,
                                            operand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require the same typed expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    if ((slsDLINK_NODE *)operand != &PolynaryExpr->operands->members)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        operand->base.lineNo,
                                        operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "too many expressions"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorAsFuncCall(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr
    )
{
    gceSTATUS       status;
    gctUINT         operandCount, paramCount;
    sloIR_EXPR      operand;
    slsNAME *       paramName;

    gcmHEADER_ARG("Compiler=0x%x PolynaryExpr=0x%x",
                  Compiler, PolynaryExpr);

    /* Verify the arguments. */
    slmVERIFY_OBJECT(Compiler, slvOBJ_COMPILER);
    slmVERIFY_IR_OBJECT(PolynaryExpr, slvIR_POLYNARY_EXPR);
    gcmASSERT(PolynaryExpr->funcName);
    gcmASSERT(PolynaryExpr->funcName->u.funcInfo.localSpace);

    if (PolynaryExpr->funcName->isBuiltIn &&
        slsFUNC_HAS_FLAG(&(PolynaryExpr->funcName->u.funcInfo), slvFUNC_HAS_VAR_ARG))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (Compiler->shaderType == slvSHADER_TYPE_TCS &&
        gcoOS_StrNCmp(PolynaryExpr->funcName->symbol, "barrier", 7) == gcvSTATUS_OK)
    {
        if (!slsSLINK_LIST_IsEmpty(&Compiler->context.switchScope))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "in tessellation control shaders, barrier cannot be put in control flow."));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
        else
        {
            slsNAME_SPACE* nameSpace = Compiler->context.currentSpace;

            while (nameSpace)
            {
                if (nameSpace->nameSpaceType == slvNAME_SPACE_TYPE_SELECT_SET ||
                    nameSpace->nameSpaceType == slvNAME_SPACE_TYPE_LOOP_SET)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "in tessellation control shaders, barrier cannot be put in control flow."));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }

                if (nameSpace->nameSpaceType == slvNAME_SPACE_TYPE_FUNCTION)
                {
                    if (nameSpace->nameSpaceFlags & sleNAME_SPACE_FLAGS_RETURN_INSERTED)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "in tessellation control shaders, barrier cannot be put after return."));

                        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                        gcmFOOTER();
                        return status;
                    }
                    if (gcoOS_StrNCmp(nameSpace->spaceName, "main", 4) != gcvSTATUS_OK)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "in tessellation control shaders, barrier cannot be put in functions other than main."));

                        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                        gcmFOOTER();
                        return status;
                    }
                    break;
                }

                nameSpace = nameSpace->parent;
            }
        }
    }

    if (PolynaryExpr->operands == gcvNULL)
    {
        operandCount = 0;
    }
    else
    {
        gcmVERIFY_OK(sloIR_SET_GetMemberCount(
                                            Compiler,
                                            PolynaryExpr->operands,
                                            &operandCount));
    }

    if (operandCount == 0)
    {
        gcmVERIFY_OK(sloNAME_GetParamCount(
                                            Compiler,
                                            PolynaryExpr->funcName,
                                            &paramCount));

        if (paramCount != 0)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "require %d argument(s)",
                                            paramCount));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
        else
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

    for (paramName = slsDLINK_LIST_First(
                            &PolynaryExpr->funcName->u.funcInfo.localSpace->names, slsNAME),
            operand = slsDLINK_LIST_First(&PolynaryExpr->operands->members, struct _sloIR_EXPR);
        (slsDLINK_NODE *)paramName != &PolynaryExpr->funcName->u.funcInfo.localSpace->names;
        paramName = slsDLINK_NODE_Next(&paramName->node, slsNAME),
            operand = slsDLINK_NODE_Next(&operand->base.node, struct _sloIR_EXPR))
    {
        if (paramName->type != slvPARAMETER_NAME) break;

        if ((slsDLINK_NODE *)operand == &PolynaryExpr->operands->members)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            slvREPORT_ERROR,
                                            "require more arguments"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        if (!slsDATA_TYPE_IsEqual(paramName->dataType, operand->toBeDataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            operand->base.lineNo,
                                            operand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require the same typed argument"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        switch (paramName->dataType->qualifiers.storage)
        {
        case slvSTORAGE_QUALIFIER_OUT:
        case slvSTORAGE_QUALIFIER_INOUT:
            status = _CheckErrorAsLValueExpr(Compiler, operand);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            break;
        }
    }

    if ((slsDLINK_NODE *)operand != &PolynaryExpr->operands->members)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        operand->base.lineNo,
                                        operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "too many arguments"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CreateUnnamedConstantExpr(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType,
    IN sloIR_CONSTANT Constant,
    OUT sloIR_EXPR *ConstantExpr
)
{
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE *dataType   = gcvNULL;
    slsNAME *unnamedConstant = gcvNULL;
    slsNAME_SPACE *nameSpace = gcvNULL;
    sloIR_CONSTANT constant;
    gctUINT lineNo = Constant->exprBase.base.lineNo;
    gctUINT stringNo = Constant->exprBase.base.stringNo;

    gcmASSERT (ConstantExpr);
    if (!slsDATA_TYPE_IsVectorType(DataType) ||
        Constant->variable || Constant->allValuesEqual ||
        !(Compiler->context.optimizationOptions & slvOPTIMIZATION_SHARE_VEC_CONSTANTS))
    {
       *ConstantExpr = &Constant->exprBase;
       return gcvSTATUS_OK;
    }

    status = sloCOMPILER_GetVecConstant(Compiler,
                                        Constant,
                                        &unnamedConstant);
    if (status == gcvSTATUS_NOT_FOUND)
    { /* not found */
       sloEXTENSION extension = {0};
       status = sloCOMPILER_PushUnnamedSpace(Compiler, &nameSpace);
       if (gcmIS_ERROR(status)) return status;

       gcmONERROR(sloCOMPILER_CloneDataType(Compiler,
                                            slvSTORAGE_QUALIFIER_CONST,
                                            DataType->qualifiers.precision,
                                            DataType,
                                            &dataType));

       extension.extension1 = slvEXTENSION1_NONE;
       gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                         Constant->exprBase.base.lineNo,
                                         Constant->exprBase.base.stringNo,
                                         slvVARIABLE_NAME,
                                         dataType,
                                         "",
                                         extension,
                                         gcvFALSE,
                                         &unnamedConstant));

       slsDLINK_NODE_Detach(&unnamedConstant->node);
       status = sloCOMPILER_SetVecConstant(Compiler,
                                           unnamedConstant);
       Constant->variable = unnamedConstant;
       unnamedConstant->u.variableInfo.constant = Constant;
    }
    else
    {
        gcmVERIFY_OK(sloIR_CONSTANT_Destroy(Compiler, &Constant->exprBase.base));
    }


    gcmONERROR(sloIR_CONSTANT_Clone(Compiler,
                                    lineNo,
                                    stringNo,
                                    unnamedConstant->u.variableInfo.constant,
                                    &constant));

    if (!slsDATA_TYPE_IsEqual(constant->exprBase.dataType, DataType))
    {
        gcmONERROR(sloCOMPILER_CloneDataType(Compiler,
                                             DataType->qualifiers.storage,
                                             DataType->qualifiers.precision,
                                             DataType,
                                             &constant->exprBase.dataType));
    }

    *ConstantExpr = &constant->exprBase;

OnError:
    if (nameSpace)
    {
       sloCOMPILER_PopCurrentNameSpace(Compiler, &nameSpace);
    }
    return status;
}

sloIR_EXPR
_ParseFuncCallExprAsExpr(
                         IN sloCOMPILER Compiler,
                         IN sloIR_EXPR Expr
                         );

static sloIR_EXPR
_MakeImplicitConversionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Arg
    )
{
    slsLexToken  funcName;
    sloIR_POLYNARY_EXPR funcCall;

    (void)gcoOS_ZeroMemory((gctPOINTER)&funcName, sizeof(slsLexToken));
    funcName.lineNo = Arg->base.lineNo;
    funcName.stringNo = Arg->base.stringNo;
    funcName.type = T_BASIC_TYPE;
    funcName.u.basicType = Arg->toBeDataType;
    funcCall = slParseFuncCallHeaderExpr(Compiler,
                                         &funcName);

    funcCall = slParseFuncCallArgument(Compiler,
                                       funcCall,
                                       Arg);

    return  slParseFuncCallExprAsExpr(Compiler,
                                      funcCall);
}

sloIR_EXPR
slParseFuncCallExprAsExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR FuncCall
    )
{
    gceSTATUS       status;
    sloIR_CONSTANT  constant;
    sloIR_EXPR      firstOperand;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x FuncCall=0x%x",
                  Compiler, FuncCall);

    if (FuncCall == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switch (FuncCall->type)
    {
    case slvPOLYNARY_CONSTRUCT_FLOAT:
    case slvPOLYNARY_CONSTRUCT_INT:
    case slvPOLYNARY_CONSTRUCT_UINT:
    case slvPOLYNARY_CONSTRUCT_BOOL:
    case slvPOLYNARY_CONSTRUCT_DOUBLE:
        status = _CheckErrorAsScalarConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvPOLYNARY_CONSTRUCT_ARRAY:
        status = _CheckErrorAsArrayConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvPOLYNARY_CONSTRUCT_ARRAYS_OF_ARRAYS:
        status = _CheckErrorAsArraysOfArraysConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvPOLYNARY_CONSTRUCT_VEC2:
    case slvPOLYNARY_CONSTRUCT_VEC3:
    case slvPOLYNARY_CONSTRUCT_VEC4:
    case slvPOLYNARY_CONSTRUCT_BVEC2:
    case slvPOLYNARY_CONSTRUCT_BVEC3:
    case slvPOLYNARY_CONSTRUCT_BVEC4:
    case slvPOLYNARY_CONSTRUCT_IVEC2:
    case slvPOLYNARY_CONSTRUCT_IVEC3:
    case slvPOLYNARY_CONSTRUCT_IVEC4:
    case slvPOLYNARY_CONSTRUCT_UVEC2:
    case slvPOLYNARY_CONSTRUCT_UVEC3:
    case slvPOLYNARY_CONSTRUCT_UVEC4:
    case slvPOLYNARY_CONSTRUCT_DVEC2:
    case slvPOLYNARY_CONSTRUCT_DVEC3:
    case slvPOLYNARY_CONSTRUCT_DVEC4:
        status = _CheckErrorAsVectorOrMatrixConstructor(Compiler, FuncCall, gcvTRUE);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvPOLYNARY_CONSTRUCT_MAT2:
    case slvPOLYNARY_CONSTRUCT_MAT2X3:
    case slvPOLYNARY_CONSTRUCT_MAT2X4:
    case slvPOLYNARY_CONSTRUCT_MAT3:
    case slvPOLYNARY_CONSTRUCT_MAT3X2:
    case slvPOLYNARY_CONSTRUCT_MAT3X4:
    case slvPOLYNARY_CONSTRUCT_MAT4:
    case slvPOLYNARY_CONSTRUCT_MAT4X2:
    case slvPOLYNARY_CONSTRUCT_MAT4X3:
    case slvPOLYNARY_CONSTRUCT_DMAT2:
    case slvPOLYNARY_CONSTRUCT_DMAT2X3:
    case slvPOLYNARY_CONSTRUCT_DMAT2X4:
    case slvPOLYNARY_CONSTRUCT_DMAT3:
    case slvPOLYNARY_CONSTRUCT_DMAT3X2:
    case slvPOLYNARY_CONSTRUCT_DMAT3X4:
    case slvPOLYNARY_CONSTRUCT_DMAT4:
    case slvPOLYNARY_CONSTRUCT_DMAT4X2:
    case slvPOLYNARY_CONSTRUCT_DMAT4X3:
        status = _CheckErrorAsVectorOrMatrixConstructor(Compiler, FuncCall, gcvFALSE);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvPOLYNARY_CONSTRUCT_STRUCT:
        status = _CheckErrorAsStructConstructor(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case slvPOLYNARY_FUNC_CALL:
        status = sloCOMPILER_BindFuncCall(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        status = _CheckErrorAsFuncCall(Compiler, FuncCall);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;
    default:
        gcmASSERT(0);
    }

    switch (FuncCall->type)
    {
    case slvPOLYNARY_CONSTRUCT_FLOAT:
    case slvPOLYNARY_CONSTRUCT_DOUBLE:
    case slvPOLYNARY_CONSTRUCT_INT:
    case slvPOLYNARY_CONSTRUCT_UINT:
    case slvPOLYNARY_CONSTRUCT_BOOL:
    case slvPOLYNARY_CONSTRUCT_ARRAY:
    case slvPOLYNARY_CONSTRUCT_ARRAYS_OF_ARRAYS:
    case slvPOLYNARY_CONSTRUCT_VEC2:
    case slvPOLYNARY_CONSTRUCT_VEC3:
    case slvPOLYNARY_CONSTRUCT_VEC4:
    case slvPOLYNARY_CONSTRUCT_BVEC2:
    case slvPOLYNARY_CONSTRUCT_BVEC3:
    case slvPOLYNARY_CONSTRUCT_BVEC4:
    case slvPOLYNARY_CONSTRUCT_IVEC2:
    case slvPOLYNARY_CONSTRUCT_IVEC3:
    case slvPOLYNARY_CONSTRUCT_IVEC4:
    case slvPOLYNARY_CONSTRUCT_UVEC2:
    case slvPOLYNARY_CONSTRUCT_UVEC3:
    case slvPOLYNARY_CONSTRUCT_UVEC4:
    case slvPOLYNARY_CONSTRUCT_MAT2:
    case slvPOLYNARY_CONSTRUCT_MAT2X3:
    case slvPOLYNARY_CONSTRUCT_MAT2X4:
    case slvPOLYNARY_CONSTRUCT_MAT3:
    case slvPOLYNARY_CONSTRUCT_MAT3X2:
    case slvPOLYNARY_CONSTRUCT_MAT3X4:
    case slvPOLYNARY_CONSTRUCT_MAT4:
    case slvPOLYNARY_CONSTRUCT_MAT4X2:
    case slvPOLYNARY_CONSTRUCT_MAT4X3:
    case slvPOLYNARY_CONSTRUCT_DVEC2:
    case slvPOLYNARY_CONSTRUCT_DVEC3:
    case slvPOLYNARY_CONSTRUCT_DVEC4:
    case slvPOLYNARY_CONSTRUCT_DMAT2:
    case slvPOLYNARY_CONSTRUCT_DMAT2X3:
    case slvPOLYNARY_CONSTRUCT_DMAT2X4:
    case slvPOLYNARY_CONSTRUCT_DMAT3:
    case slvPOLYNARY_CONSTRUCT_DMAT3X2:
    case slvPOLYNARY_CONSTRUCT_DMAT3X4:
    case slvPOLYNARY_CONSTRUCT_DMAT4:
    case slvPOLYNARY_CONSTRUCT_DMAT4X2:
    case slvPOLYNARY_CONSTRUCT_DMAT4X3:
        firstOperand = slsDLINK_LIST_First(&FuncCall->operands->members, struct _sloIR_EXPR);
        FuncCall->exprBase.dataType->qualifiers.precision = firstOperand->dataType->qualifiers.precision;
        break;
    case slvPOLYNARY_FUNC_CALL:
    case slvPOLYNARY_CONSTRUCT_STRUCT:
        break;
    default:
        gcmASSERT(0);
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if (FuncCall->operands != gcvNULL &&
        FuncCall->type == slvPOLYNARY_FUNC_CALL &&
        sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        sloIR_EXPR operand;
        FOR_EACH_DLINK_NODE(&FuncCall->operands->members, struct _sloIR_EXPR, operand)
        {
            if (sloIR_EXPR_ImplicitConversionDone(operand))
            {
                slsDLINK_NODE node = operand->base.node;
                sloIR_EXPR newOperand = _MakeImplicitConversionExpr(Compiler, operand);
                slsDLINK_LIST_Replace(newOperand, &node);
                operand = newOperand;
            }
        }
    }

    /* Try to evaluate it */
    status = sloIR_POLYNARY_EXPR_Evaluate(
                                        Compiler,
                                        FuncCall,
                                        &constant);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (constant)
    {
       sloIR_EXPR constantExpr = gcvNULL;

       status = _CreateUnnamedConstantExpr(Compiler,
                                           constant->exprBase.dataType,
                                           constant,
                                           &constantExpr);
       if (gcmIS_ERROR(status))
       {
          gcmFOOTER_ARG("<return>=%s", "<nil>");
          return gcvNULL;
       }

       gcmFOOTER_ARG("<return>=0x%x", constantExpr);
       return constantExpr;
    }

    if (constant != gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", &constant->exprBase);
        return &constant->exprBase;
    }
    else if(FuncCall->type == slvPOLYNARY_CONSTRUCT_ARRAY               ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_ARRAYS_OF_ARRAYS    ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT3                ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT2                ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT2X3              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT2X4              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT3X2              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT3X4              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT4                ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT4X2              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_MAT4X3              ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT3               ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT2               ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT2X3             ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT2X4             ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT3X2             ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT3X4             ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT4               ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT4X2             ||
            FuncCall->type == slvPOLYNARY_CONSTRUCT_DMAT4X3)
    {
        sloIR_EXPR retIR_EXP = _ParseFuncCallExprAsExpr(Compiler, &FuncCall->exprBase);
        gcmFOOTER_ARG("<return>=0x%x", retIR_EXP);
        return retIR_EXP;
    }
    else
    {
        gcmFOOTER_ARG("<return>=0x%x", &FuncCall->exprBase);
        return &FuncCall->exprBase;
    }
}

gceSTATUS
_ParseComponentSelection(
    IN sloCOMPILER Compiler,
    IN gctUINT8 VectorSize,
    IN slsLexToken * FieldSelection,
    OUT slsCOMPONENT_SELECTION * ComponentSelection
    )
{
    gctUINT8        i;
    gctUINT8        nameSets[4];
    sleCOMPONENT    components[4];
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x VectorSize=%u FieldSelection=0x%x ComponentSelection=0x%x",
                  Compiler, VectorSize, FieldSelection, ComponentSelection);


    gcmASSERT(VectorSize <= 4);
    gcmASSERT(FieldSelection);
    gcmASSERT(FieldSelection->u.fieldSelection);
    gcmASSERT(ComponentSelection);

    for (i = 0; FieldSelection->u.fieldSelection[i] != '\0'; i++)
    {
        if(i >= 4)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            FieldSelection->lineNo,
                                            FieldSelection->stringNo,
                                            slvREPORT_ERROR,
                                            "more than 4 components are selected : \"%s\"",
                                            FieldSelection->u.fieldSelection));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        switch (FieldSelection->u.fieldSelection[i])
        {
        case 'x': nameSets[i] = 0; components[i] = slvCOMPONENT_X; break;
        case 'y': nameSets[i] = 0; components[i] = slvCOMPONENT_Y; break;
        case 'z': nameSets[i] = 0; components[i] = slvCOMPONENT_Z; break;
        case 'w': nameSets[i] = 0; components[i] = slvCOMPONENT_W; break;

        case 'r': nameSets[i] = 1; components[i] = slvCOMPONENT_X; break;
        case 'g': nameSets[i] = 1; components[i] = slvCOMPONENT_Y; break;
        case 'b': nameSets[i] = 1; components[i] = slvCOMPONENT_Z; break;
        case 'a': nameSets[i] = 1; components[i] = slvCOMPONENT_W; break;

        case 's': nameSets[i] = 2; components[i] = slvCOMPONENT_X; break;
        case 't': nameSets[i] = 2; components[i] = slvCOMPONENT_Y; break;
        case 'p': nameSets[i] = 2; components[i] = slvCOMPONENT_Z; break;
        case 'q': nameSets[i] = 2; components[i] = slvCOMPONENT_W; break;

        default:
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            FieldSelection->lineNo,
                                            FieldSelection->stringNo,
                                            slvREPORT_ERROR,
                                            "invalid component name: '%c'",
                                            FieldSelection->u.fieldSelection[i]));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    ComponentSelection->components = i;

    for (i = 1; i < ComponentSelection->components; i++)
    {
        if (nameSets[i] != nameSets[0])
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            FieldSelection->lineNo,
                                            FieldSelection->stringNo,
                                            slvREPORT_ERROR,
                                            "the component name: '%c'"
                                            " do not come from the same set",
                                            FieldSelection->u.fieldSelection[i]));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    for (i = 0; i < ComponentSelection->components; i++)
    {
        if ((gctUINT8)components[i] >= VectorSize)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            FieldSelection->lineNo,
                                            FieldSelection->stringNo,
                                            slvREPORT_ERROR,
                                            "the component: '%c' beyond the specified vector type",
                                            FieldSelection->u.fieldSelection[i]));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        switch (i)
        {
        case 0: ComponentSelection->x = components[0]; break;
        case 1: ComponentSelection->y = components[1]; break;
        case 2: ComponentSelection->z = components[2]; break;
        case 3: ComponentSelection->w = components[3]; break;

        default:
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static slePOLYNARY_EXPR_TYPE
_ConvToPolynaryExprType(
    IN slsDATA_TYPE  *DataType
    )
{

    if(!DataType) {
       return slvPOLYNARY_CONSTRUCT_NONE;
    }

    if (DataType->arrayLengthCount > 0)
    {
        if (DataType->arrayLengthCount > 1)
        {
            return slvPOLYNARY_CONSTRUCT_ARRAYS_OF_ARRAYS;
        }
        else
        {
            return slvPOLYNARY_CONSTRUCT_ARRAY;
        }
    }

    switch (DataType->type)
    {
    case T_FLOAT:   return slvPOLYNARY_CONSTRUCT_FLOAT;
    case T_DOUBLE:  return slvPOLYNARY_CONSTRUCT_DOUBLE;
    case T_INT:     return slvPOLYNARY_CONSTRUCT_INT;
    case T_UINT:    return slvPOLYNARY_CONSTRUCT_UINT;
    case T_BOOL:    return slvPOLYNARY_CONSTRUCT_BOOL;
    case T_VEC2:    return slvPOLYNARY_CONSTRUCT_VEC2;
    case T_VEC3:    return slvPOLYNARY_CONSTRUCT_VEC3;
    case T_VEC4:    return slvPOLYNARY_CONSTRUCT_VEC4;
    case T_BVEC2:   return slvPOLYNARY_CONSTRUCT_BVEC2;
    case T_BVEC3:   return slvPOLYNARY_CONSTRUCT_BVEC3;
    case T_BVEC4:   return slvPOLYNARY_CONSTRUCT_BVEC4;
    case T_IVEC2:   return slvPOLYNARY_CONSTRUCT_IVEC2;
    case T_IVEC3:   return slvPOLYNARY_CONSTRUCT_IVEC3;
    case T_IVEC4:   return slvPOLYNARY_CONSTRUCT_IVEC4;
    case T_UVEC2:   return slvPOLYNARY_CONSTRUCT_UVEC2;
    case T_UVEC3:   return slvPOLYNARY_CONSTRUCT_UVEC3;
    case T_UVEC4:   return slvPOLYNARY_CONSTRUCT_UVEC4;
    case T_MAT2:    return slvPOLYNARY_CONSTRUCT_MAT2;
    case T_MAT2X3:  return slvPOLYNARY_CONSTRUCT_MAT2X3;
    case T_MAT2X4:  return slvPOLYNARY_CONSTRUCT_MAT2X4;
    case T_MAT3:    return slvPOLYNARY_CONSTRUCT_MAT3;
    case T_MAT3X2:  return slvPOLYNARY_CONSTRUCT_MAT3X2;
    case T_MAT3X4:  return slvPOLYNARY_CONSTRUCT_MAT3X4;
    case T_MAT4:    return slvPOLYNARY_CONSTRUCT_MAT4;
    case T_MAT4X2:  return slvPOLYNARY_CONSTRUCT_MAT4X2;
    case T_MAT4X3:  return slvPOLYNARY_CONSTRUCT_MAT4X3;
    case T_DVEC2:   return slvPOLYNARY_CONSTRUCT_DVEC2;
    case T_DVEC3:   return slvPOLYNARY_CONSTRUCT_DVEC3;
    case T_DVEC4:   return slvPOLYNARY_CONSTRUCT_DVEC4;
    case T_DMAT2:   return slvPOLYNARY_CONSTRUCT_DMAT2;
    case T_DMAT2X3: return slvPOLYNARY_CONSTRUCT_DMAT2X3;
    case T_DMAT2X4: return slvPOLYNARY_CONSTRUCT_DMAT2X4;
    case T_DMAT3:   return slvPOLYNARY_CONSTRUCT_DMAT3;
    case T_DMAT3X2: return slvPOLYNARY_CONSTRUCT_DMAT3X2;
    case T_DMAT3X4: return slvPOLYNARY_CONSTRUCT_DMAT3X4;
    case T_DMAT4:   return slvPOLYNARY_CONSTRUCT_DMAT4;
    case T_DMAT4X2: return slvPOLYNARY_CONSTRUCT_DMAT4X2;
    case T_DMAT4X3: return slvPOLYNARY_CONSTRUCT_DMAT4X3;

    case T_STRUCT:  return slvPOLYNARY_CONSTRUCT_STRUCT;

    default:
        return slvPOLYNARY_CONSTRUCT_NONE;
    }
}

static gctCONST_STRING
_GetTypeName(
    IN gctINT TokenType
    )
{
    switch (TokenType)
    {
    case T_VOID:        return "void";
    case T_FLOAT:       return "float";
    case T_DOUBLE:      return "double";
    case T_INT:         return "int";
    case T_UINT:        return "unsigned int";
    case T_BOOL:        return "bool";
    case T_VEC2:        return "vec2";
    case T_VEC3:        return "vec3";
    case T_VEC4:        return "vec4";
    case T_BVEC2:       return "bvec2";
    case T_BVEC3:       return "bvec3";
    case T_BVEC4:       return "bvec4";
    case T_IVEC2:       return "ivec2";
    case T_IVEC3:       return "ivec3";
    case T_IVEC4:       return "ivec4";
    case T_UVEC2:       return "uvec2";
    case T_UVEC3:       return "uvec3";
    case T_UVEC4:       return "uvec4";
    case T_MAT2:        return "mat2";
    case T_MAT2X3:      return "mat2x3";
    case T_MAT2X4:      return "mat2x4";
    case T_MAT3:        return "mat3";
    case T_MAT3X2:      return "mat3x2";
    case T_MAT3X4:      return "mat3x4";
    case T_MAT4:        return "mat4";
    case T_MAT4X2:      return "mat4x2";
    case T_MAT4X3:      return "mat4x3";
    case T_DVEC2:       return "dvec2";
    case T_DVEC3:       return "dvec3";
    case T_DVEC4:       return "dvec4";
    case T_DMAT2:       return "dmat2";
    case T_DMAT2X3:     return "dmat2x3";
    case T_DMAT2X4:     return "dmat2x4";
    case T_DMAT3:       return "dmat3";
    case T_DMAT3X2:     return "dmat3x2";
    case T_DMAT3X4:     return "dmat3x4";
    case T_DMAT4:       return "dmat4";
    case T_DMAT4X2:     return "dmat4x2";
    case T_DMAT4X3:     return "dmat4x3";
    case T_SAMPLER2D:   return "sampler2D";
    case T_SAMPLERCUBE: return "samplerCube";

    case T_SAMPLER3D:              return "sampler3D";
    case T_SAMPLER1DARRAY:         return "sampler1DArray";
    case T_SAMPLER2DARRAY:         return "sampler2DArray";
    case T_SAMPLER1DARRAYSHADOW:   return "sampler1DArrayShadow";
    case T_SAMPLER2DARRAYSHADOW:   return "sampler2DArrayShadow";
    case T_SAMPLER2DSHADOW:        return "sampler2DShadow";
    case T_SAMPLERCUBESHADOW:      return "samplerCubeShadow";
    case T_SAMPLERCUBEARRAY:       return "samplerCubeArray";
    case T_SAMPLERCUBEARRAYSHADOW: return "samplerCubeArrayShadow";

    case T_ISAMPLER2D:             return "isampler2D";
    case T_ISAMPLERCUBE:           return "isamplerCube";
    case T_ISAMPLERCUBEARRAY:      return "isamplerCubeArray";
    case T_ISAMPLER3D:             return "isampler3D";
    case T_ISAMPLER2DARRAY:        return "isampler2DArray";

    case T_USAMPLER2D:             return "usampler2D";
    case T_USAMPLERCUBE:           return "usamplerCube";
    case T_USAMPLERCUBEARRAY:      return "usamplerCubeArray";
    case T_USAMPLER3D:             return "usampler3D";
    case T_USAMPLER2DARRAY:        return "usampler2DArray";
    case T_SAMPLEREXTERNALOES:     return "samplerExternalOES";

    case T_SAMPLER2DMS:            return "sampler2DMS";
    case T_ISAMPLER2DMS:           return "isampler2DMS";
    case T_USAMPLER2DMS:           return "usampler2DMS";
    case T_SAMPLER2DMSARRAY:       return "sampler2DMSArray";
    case T_ISAMPLER2DMSARRAY:      return "isampler2DMSArray";
    case T_USAMPLER2DMSARRAY:      return "usampler2DMSArray";

    case T_SAMPLER1D:              return "sampler1D";
    case T_ISAMPLER1D:             return "isampler1D";
    case T_USAMPLER1D:             return "usampler1D";
    case T_SAMPLER1DSHADOW:        return "sampler1DShadow";
    case T_SAMPLER2DRECT:          return "sampler2DRect";
    case T_ISAMPLER2DRECT:         return "isampler2DRect";
    case T_USAMPLER2DRECT:         return "usampler2DRect";
    case T_SAMPLER2DRECTSHADOW:    return "sampler2DRectShadow";
    case T_ISAMPLER1DARRAY:        return "isampler1DArray";
    case T_USAMPLER1DARRAY:        return "usampler1DArray";

    case T_IMAGE2D:                return "image2D";
    case T_IIMAGE2D:               return "iimage2D";
    case T_UIMAGE2D:               return "uimage2D";
    case T_IMAGE2DARRAY:           return "image2DArray";
    case T_IIMAGE2DARRAY:          return "iimage2DArray";
    case T_UIMAGE2DARRAY:          return "uimage2DArray";
    case T_IMAGE3D:                return "image3D";
    case T_IIMAGE3D:               return "iimage3D";
    case T_UIMAGE3D:               return "uimage3D";
    case T_IMAGECUBE:              return "imageCube";
    case T_IMAGECUBEARRAY:         return "imageCubeArray";
    case T_IIMAGECUBE:             return "iimageCube";
    case T_IIMAGECUBEARRAY:        return "iimageCubeArray";
    case T_UIMAGECUBE:             return "uimageCube";
    case T_UIMAGECUBEARRAY:        return "uimageCubeArray";

    case T_STRUCT:                 return "struct";
    case T_ATOMIC_UINT:            return "atomic_uint";

    case T_SAMPLERBUFFER:          return "samplerBuffer";
    case T_ISAMPLERBUFFER:         return "isamplerBuffer";
    case T_USAMPLERBUFFER:         return "usamplerBuffer";
    case T_IMAGEBUFFER:            return "imageBuffer";
    case T_IIMAGEBUFFER:           return "iimageBuffer";
    case T_UIMAGEBUFFER:           return "uimageBuffer";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

sloIR_POLYNARY_EXPR
slParseFuncCallHeaderExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * FuncIdentifier
    )
{
    gceSTATUS               status;
    slePOLYNARY_EXPR_TYPE   exprType;
    slsDATA_TYPE *          dataType    = gcvNULL;
    sltPOOL_STRING          funcSymbol  = gcvNULL;
    sloIR_POLYNARY_EXPR     polynaryExpr;
    slsNAME *               typeName = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x FuncIdentifier=0x%x",
                  Compiler, FuncIdentifier);

    gcmASSERT(FuncIdentifier);

    switch (FuncIdentifier->type)
    {
    case T_BASIC_TYPE:
        exprType  = _ConvToPolynaryExprType(FuncIdentifier->u.basicType);

        if(exprType == slvPOLYNARY_CONSTRUCT_NONE) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            FuncIdentifier->lineNo,
                                            FuncIdentifier->stringNo,
                                            slvREPORT_ERROR,
                                            "invalid constructor type: '%s'",
                                            FuncIdentifier->u.basicType ?
                                            _GetTypeName(FuncIdentifier->u.basicType->type) : ""));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;

        }
        status = sloCOMPILER_CloneDataType(Compiler,
                                           slvSTORAGE_QUALIFIER_CONST,
                                           FuncIdentifier->u.basicType->qualifiers.precision,
                                           FuncIdentifier->u.basicType,
                                           &dataType);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case T_IDENTIFIER:
        exprType    = slvPOLYNARY_FUNC_CALL;

        funcSymbol  = FuncIdentifier->u.identifier;

        /*
        ** Check if there are variables that redeclared with this name in the visible name space,
        ** if so, we need to report a compile error.
        */
        status = sloCOMPILER_SearchName(Compiler,
                                        funcSymbol,
                                        gcvTRUE,
                                        &typeName);

        if (typeName != gcvNULL &&
            typeName->type != slvFUNC_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            FuncIdentifier->lineNo,
                                            FuncIdentifier->stringNo,
                                            slvREPORT_ERROR,
                                            "invalid to calling hidden function: '%s'",
                                            funcSymbol));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        /* check if this is builtin function and if the corresponding extension is enabled */
        status = sloCOMPILER_BuiltinFuncEnabled(Compiler, funcSymbol);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                FuncIdentifier->lineNo,
                FuncIdentifier->stringNo,
                slvREPORT_ERROR,
                "invalid call function '%s', require enabling the extension",
                funcSymbol));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    default:
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create polynary expression */
    status = sloIR_POLYNARY_EXPR_Construct(Compiler,
                                           FuncIdentifier->lineNo,
                                           FuncIdentifier->stringNo,
                                           exprType,
                                           dataType,
                                           funcSymbol,
                                           &polynaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FUNC_CALL_HEADER type=\"%s\" line=\"%d\" string=\"%d\" />",
                                slGetIRPolynaryExprTypeName(exprType),
                                FuncIdentifier->lineNo,
                                FuncIdentifier->stringNo));

    gcmFOOTER_ARG("<return>=0x%x", polynaryExpr);
    return polynaryExpr;
}

sloIR_POLYNARY_EXPR
slParseFuncCallArgument(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR FuncCall,
    IN sloIR_EXPR Argument
    )
{
    gceSTATUS   status;

    gcmHEADER_ARG("Compiler=0x%x FuncCall=0x%x Argument=0x%x",
                  Compiler, FuncCall, Argument);

    if (FuncCall == gcvNULL || Argument == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (FuncCall->operands == gcvNULL)
    {
        status = sloIR_SET_Construct(
                                    Compiler,
                                    Argument->base.lineNo,
                                    Argument->base.stringNo,
                                    slvEXPR_SET,
                                    &FuncCall->operands);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    if (FuncCall->funcSymbol &&
        gcoOS_StrNCmp(FuncCall->funcSymbol, "imageStore", 10) == gcvSTATUS_OK &&
        slsDATA_TYPE_IsImage(Argument->dataType) &&
        Argument->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_READONLY)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Argument->base.lineNo,
                                        Argument->base.stringNo,
                                        slvREPORT_ERROR,
                                        "readonly image cannot be used in imageStore"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (FuncCall->funcSymbol &&
        gcoOS_StrNCmp(FuncCall->funcSymbol, "imageLoad", 9) == gcvSTATUS_OK &&
        slsDATA_TYPE_IsImage(Argument->dataType) &&
        Argument->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Argument->base.lineNo,
                                        Argument->base.stringNo,
                                        slvREPORT_ERROR,
                                        "writeonly image cannot be used in imageLoad"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (FuncCall->funcSymbol &&
        gcoOS_StrNCmp(FuncCall->funcSymbol, "imageAtomic", 11) == gcvSTATUS_OK &&
        slsDATA_TYPE_IsImage(Argument->dataType))
    {
        if(Argument->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_READONLY ||
           Argument->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Argument->base.lineNo,
                                            Argument->base.stringNo,
                                            slvREPORT_ERROR,
                                            "readonly/writeonly image cannot be used in imageAtomicXXX"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        if(slsDATA_TYPE_IsFloatImage(Argument->dataType) && Argument->dataType->qualifiers.layout.imageFormat != slvLAYOUT_IMAGE_FORMAT_R32F)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Argument->base.lineNo,
                                            Argument->base.stringNo,
                                            slvREPORT_ERROR,
                                            "float image used in imageAtomicXXX must be in r32f format"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        if(slsDATA_TYPE_IsIntImage(Argument->dataType) && Argument->dataType->qualifiers.layout.imageFormat != slvLAYOUT_IMAGE_FORMAT_R32I)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Argument->base.lineNo,
                                            Argument->base.stringNo,
                                            slvREPORT_ERROR,
                                            "integer image used in imageAtomicXXX must be in r32i format"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        if(slsDATA_TYPE_IsUintImage(Argument->dataType) && Argument->dataType->qualifiers.layout.imageFormat != slvLAYOUT_IMAGE_FORMAT_R32UI)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Argument->base.lineNo,
                                            Argument->base.stringNo,
                                            slvREPORT_ERROR,
                                            "unsigned integer image used in imageAtomicXXX must be in r32ui format"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    gcmASSERT(FuncCall->operands);
    gcmVERIFY_OK(sloIR_SET_AddMember(
                                    Compiler,
                                    FuncCall->operands,
                                    &Argument->base));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FUNC_CALL_ARGUMENT />"));

    gcmFOOTER_ARG("<return>=0x%x", FuncCall);
    return FuncCall;
}


sloIR_VIV_ASM
slParseAsmOpcode(
    IN sloCOMPILER         Compiler,
    IN slsASM_OPCODE       *AsmOpcode
    )
{
    sloIR_VIV_ASM vivAsm = gcvNULL;

    gceSTATUS     status;

    gcmHEADER_ARG("AsmOpcode=0x%x", AsmOpcode);
    gcmASSERT(AsmOpcode);

    /* Create sloIR_VIV_ASM */
    status = sloIR_VIV_ASM_Construct(Compiler,
                                           AsmOpcode->lineNo,
                                           AsmOpcode->stringNo,
                                           AsmOpcode,
                                           &vivAsm);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<slParseAsmOpcode line=\"%d\" string=\"%d\" />",
                                AsmOpcode->lineNo,
                                AsmOpcode->stringNo));

    gcmFOOTER_ARG("<return>=0x%x", vivAsm);
    return vivAsm;
}

sloIR_VIV_ASM
slParseAsmOperand(
    IN sloCOMPILER         Compiler,
    IN sloIR_VIV_ASM       VivAsm,
    IN sloIR_EXPR          Operand
    )
{
    gceSTATUS   status;
    gctBOOL     firstOperand = gcvFALSE;

    gcmHEADER_ARG("Compiler=0x%x FuncCall=0x%x Argument=0x%x",
                  Compiler, VivAsm, Operand);

    if (VivAsm == gcvNULL || Operand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (VivAsm->operands == gcvNULL)
    {
        status = sloIR_SET_Construct(
                                    Compiler,
                                    Operand->base.lineNo,
                                    Operand->base.stringNo,
                                    slvEXPR_SET,
                                    &VivAsm->operands);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        firstOperand = gcvTRUE;
    }

    gcmASSERT(VivAsm->operands);

    if (firstOperand)
    {
        status = _CheckErrorAsLValueExpr(Compiler, Operand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    gcmVERIFY_OK(sloIR_SET_AddMember(
                                    Compiler,
                                    VivAsm->operands,
                                    &Operand->base));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FUNC_CALL_ARGUMENT />"));

    gcmFOOTER_ARG("<return>=0x%x", VivAsm);
    return VivAsm;
}

sloIR_EXPR
slParseAsmAppendOperandModifiers(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR  Expr,
    IN slsASM_MODIFIERS *Modifiers
    )
{
    gcmHEADER_ARG("Modifiers=0x%x", Modifiers);
    gcmASSERT(Modifiers);

    if (Modifiers)
    {
        gctPOINTER pointer = gcvNULL;
        slsASM_MODIFIERS *modifierCopy;

        sloCOMPILER_Allocate(
                        Compiler,
                        (gctSIZE_T)sizeof(slsASM_MODIFIERS),
                        &pointer);

        modifierCopy = (slsASM_MODIFIERS *) pointer;

        *modifierCopy = *Modifiers;

        Modifiers = modifierCopy;
    }

    if (Expr)
    {
        Expr->asmMods = Modifiers;
    }

    gcmFOOTER_ARG("<return>=0x%x", Expr);
    return  Expr;
}

slsASM_MODIFIERS
slParseAsmAppendModifier(
    IN sloCOMPILER Compiler,
    IN slsASM_MODIFIERS *Modifiers,
    IN slsASM_MODIFIER  *Modifier
    )
{
    slsASM_MODIFIERS modifiers;

    gcmHEADER_ARG("Modifier=0x%x", Modifier);
    gcmASSERT(Modifier);

    if (!Modifiers)
    {
        gcoOS_MemFill((gctPOINTER)&modifiers, (gctUINT8)-1, sizeof(slsASM_MODIFIERS));

        Modifiers = (slsASM_MODIFIERS *) &modifiers;
    }

    Modifiers->modifiers[Modifier->type] = *Modifier;

    gcmFOOTER_ARG("<return>=0x%x", Modifiers);
    return  *Modifiers;
}

slsASM_MODIFIER
slParseAsmModifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Type,
    IN slsLexToken * Value
    )
{
    slsASM_MODIFIER asmMod;

    gcmHEADER_ARG("Opcode=0x%x, Opcode=0x%x", Type, Value);
    gcmASSERT(Type && Value);

    gcoOS_ZeroMemory((gctPOINTER)&asmMod, sizeof(slsASM_MODIFIER));

    /* DATATYPE */
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "f")))
    {
#define STRING_MATCH_FORMAT(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPND_FORMAT;   \
    asmMod.value = gcSL_##STR;   \
    }

        STRING_MATCH_FORMAT(FLOAT)
        else STRING_MATCH_FORMAT(INTEGER)
        else STRING_MATCH_FORMAT(INT32)
        else STRING_MATCH_FORMAT(BOOLEAN)
        else STRING_MATCH_FORMAT(UINT32)
        else STRING_MATCH_FORMAT(INT8)
        else STRING_MATCH_FORMAT(UINT8)
        else STRING_MATCH_FORMAT(INT16)
        else STRING_MATCH_FORMAT(UINT16)
        else STRING_MATCH_FORMAT(INT64)
        else STRING_MATCH_FORMAT(UINT64)
        else STRING_MATCH_FORMAT(SNORM8)
        else STRING_MATCH_FORMAT(UNORM8)
        else STRING_MATCH_FORMAT(FLOAT16)
        else STRING_MATCH_FORMAT(FLOAT64)
        else STRING_MATCH_FORMAT(SNORM16)
        else STRING_MATCH_FORMAT(UNORM16)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

    }
    /* PRECISION */
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "p")))
    {
#define STRING_MATCH_PRECISION(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type  = sleASM_MODIFIER_OPND_PRECISION;   \
    asmMod.value = gcSHADER_PRECISION_##STR;   \
    }

        STRING_MATCH_PRECISION(DEFAULT)
        else STRING_MATCH_PRECISION(HIGH)
        else STRING_MATCH_PRECISION(MEDIUM)
        else STRING_MATCH_PRECISION(LOW)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

#undef STRING_MATCH_PRECISION
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "c")))
    {
#define STRING_MATCH_CONDITION(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPCODE_CONDITION;   \
    asmMod.value = gcSL_##STR;   \
    }
        /*
        Regular expression:
        Find:    ^[ ]+gcSL_([a-zA-Z0-9_]+),.*
        Replace: else STRING_MATCH_CONDITION(\1)
        */
        STRING_MATCH_CONDITION(ALWAYS)
        else STRING_MATCH_CONDITION(NOT_EQUAL)
        else STRING_MATCH_CONDITION(LESS_OR_EQUAL)
        else STRING_MATCH_CONDITION(LESS)
        else STRING_MATCH_CONDITION(EQUAL)
        else STRING_MATCH_CONDITION(GREATER)
        else STRING_MATCH_CONDITION(GREATER_OR_EQUAL)
        else STRING_MATCH_CONDITION(AND)
        else STRING_MATCH_CONDITION(OR)
        else STRING_MATCH_CONDITION(XOR)
        else STRING_MATCH_CONDITION(NOT_ZERO)
        else STRING_MATCH_CONDITION(ZERO)
        else STRING_MATCH_CONDITION(GREATER_OR_EQUAL_ZERO)
        else STRING_MATCH_CONDITION(GREATER_ZERO)
        else STRING_MATCH_CONDITION(LESS_OREQUAL_ZERO)
        else STRING_MATCH_CONDITION(LESS_ZERO)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_CONDITION
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "t")))
    {
#define STRING_MATCH_THREAD(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_THREAD_OPCODE_MODE;   \
    asmMod.value = gcSL_##STR;   \
    }


#undef STRING_MATCH_THREAD
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "rnd")))
    {
#define STRING_MATCH_ROUND(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPCODE_ROUND;   \
    asmMod.value = gcSL_ROUND_##STR;   \
    }
        STRING_MATCH_ROUND(DEFAULT)
        else STRING_MATCH_ROUND(RTZ)
        else STRING_MATCH_ROUND(RTNE)
        else STRING_MATCH_ROUND(RTP)
        else STRING_MATCH_ROUND(RTN)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }

#undef STRING_MATCH_ROUND
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "sat")))
    {
#define STRING_MATCH_SATURATE(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPCODE_SAT;   \
    asmMod.value = gcSL_##STR;   \
    }
        STRING_MATCH_SATURATE(NO_SATURATE)
        else STRING_MATCH_SATURATE(SATURATE)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_SATURATE
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "abs")))
    {
#define STRING_MATCH_ABS(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPND_ABS;   \
    asmMod.value = 1;   \
    }
        STRING_MATCH_ABS(1)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_ABS
    }
    else if(gcmIS_SUCCESS(gcoOS_StrCmp(Type->u.identifier, "neg")))
    {
#define STRING_MATCH_NEG(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Value->u.identifier, #STR))) { \
    asmMod.type = sleASM_MODIFIER_OPND_NEG;   \
    asmMod.value = 1;   \
    }
        STRING_MATCH_NEG(1)
        else {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                Value->lineNo,
                Value->stringNo,
                slvREPORT_ERROR,
                "unknown token: '%s'",
                Value->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return asmMod;
        }
#undef STRING_MATCH_NEG
    }
    else {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
            Type->lineNo,
            Type->stringNo,
            slvREPORT_ERROR,
            "unknown type: '%s'",
            Type->u.identifier));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return asmMod;
    }

    gcmFOOTER_ARG("<return>=0x%x", &asmMod);
    return  asmMod;
}

slsASM_OPCODE
slParseAsmCreateOpcode(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Opcode,
    IN slsASM_MODIFIERS * Modifiers
    )
{
    slsASM_OPCODE asmOpcode;

    gcmHEADER_ARG("Opcode=0x%x", Opcode);
    gcmASSERT(Opcode);

    gcoOS_MemFill((gctPOINTER)&asmOpcode, (gctUINT8)-1, sizeof(slsASM_OPCODE));

#define STRING_MATCH_OPCODE(STR) \
    if(gcmIS_SUCCESS(gcoOS_StrCmp(Opcode->u.identifier, #STR))) { \
    asmOpcode.opcode = gcSL_##STR; \
    }
    /*
    Regular expression:
    Find:    ^[ ]+gcSL_([a-zA-Z0-9_]+),.*
    Replace: else STRING_MATCH_OPCODE(\1)
    */
    STRING_MATCH_OPCODE(NOP)
    else STRING_MATCH_OPCODE(MOV)
    else STRING_MATCH_OPCODE(SAT)
    else STRING_MATCH_OPCODE(DP3)
    else STRING_MATCH_OPCODE(DP4)
    else STRING_MATCH_OPCODE(ABS)
    else STRING_MATCH_OPCODE(JMP)
    else STRING_MATCH_OPCODE(ADD)
    else STRING_MATCH_OPCODE(MUL)
    else STRING_MATCH_OPCODE(RCP)
    else STRING_MATCH_OPCODE(SUB)
    else STRING_MATCH_OPCODE(KILL)
    else STRING_MATCH_OPCODE(TEXLD)
    else STRING_MATCH_OPCODE(TEXLD_U)
    else STRING_MATCH_OPCODE(CALL)
    else STRING_MATCH_OPCODE(RET)
    else STRING_MATCH_OPCODE(NORM)
    else STRING_MATCH_OPCODE(MAX)
    else STRING_MATCH_OPCODE(MIN)
    else STRING_MATCH_OPCODE(POW)
    else STRING_MATCH_OPCODE(RSQ)
    else STRING_MATCH_OPCODE(LOG)
    else STRING_MATCH_OPCODE(FRAC)
    else STRING_MATCH_OPCODE(FLOOR)
    else STRING_MATCH_OPCODE(CEIL)
    else STRING_MATCH_OPCODE(CROSS)
    else STRING_MATCH_OPCODE(TEXLDPROJ)
    else STRING_MATCH_OPCODE(TEXBIAS)
    else STRING_MATCH_OPCODE(TEXGRAD)
    else STRING_MATCH_OPCODE(TEXLOD)
    else STRING_MATCH_OPCODE(SIN)
    else STRING_MATCH_OPCODE(COS)
    else STRING_MATCH_OPCODE(TAN)
    else STRING_MATCH_OPCODE(EXP)
    else STRING_MATCH_OPCODE(SIGN)
    else STRING_MATCH_OPCODE(STEP)
    else STRING_MATCH_OPCODE(SQRT)
    else STRING_MATCH_OPCODE(ACOS)
    else STRING_MATCH_OPCODE(ASIN)
    else STRING_MATCH_OPCODE(ATAN)
    else STRING_MATCH_OPCODE(SET)
    else STRING_MATCH_OPCODE(DSX)
    else STRING_MATCH_OPCODE(DSY)
    else STRING_MATCH_OPCODE(FWIDTH)
    else STRING_MATCH_OPCODE(DIV)
    else STRING_MATCH_OPCODE(MOD)
    else STRING_MATCH_OPCODE(AND_BITWISE)
    else STRING_MATCH_OPCODE(OR_BITWISE)
    else STRING_MATCH_OPCODE(XOR_BITWISE)
    else STRING_MATCH_OPCODE(NOT_BITWISE)
    else STRING_MATCH_OPCODE(LSHIFT)
    else STRING_MATCH_OPCODE(RSHIFT)
    else STRING_MATCH_OPCODE(ROTATE)
    else STRING_MATCH_OPCODE(BITSEL)
    else STRING_MATCH_OPCODE(LEADZERO)
    else STRING_MATCH_OPCODE(LOAD)
    else STRING_MATCH_OPCODE(STORE)
    else STRING_MATCH_OPCODE(BARRIER)
    else STRING_MATCH_OPCODE(STORE1)
    else STRING_MATCH_OPCODE(ATOMADD)
    else STRING_MATCH_OPCODE(ATOMSUB)
    else STRING_MATCH_OPCODE(ATOMXCHG)
    else STRING_MATCH_OPCODE(ATOMCMPXCHG)
    else STRING_MATCH_OPCODE(ATOMMIN)
    else STRING_MATCH_OPCODE(ATOMMAX)
    else STRING_MATCH_OPCODE(ATOMOR)
    else STRING_MATCH_OPCODE(ATOMAND)
    else STRING_MATCH_OPCODE(ATOMXOR)
    else STRING_MATCH_OPCODE(TEXLDPCF)
    else STRING_MATCH_OPCODE(TEXLDPCFPROJ)
    else STRING_MATCH_OPCODE(TEXLODQ)
    else STRING_MATCH_OPCODE(FLUSH)
    else STRING_MATCH_OPCODE(JMP_ANY)
    else STRING_MATCH_OPCODE(BITRANGE)
    else STRING_MATCH_OPCODE(BITRANGE1)
    else STRING_MATCH_OPCODE(BITEXTRACT)
    else STRING_MATCH_OPCODE(BITINSERT)
    else STRING_MATCH_OPCODE(FINDLSB)
    else STRING_MATCH_OPCODE(FINDMSB)
    else STRING_MATCH_OPCODE(IMAGE_OFFSET)
    else STRING_MATCH_OPCODE(IMAGE_ADDR)
    else STRING_MATCH_OPCODE(IMAGE_ADDR_3D)
    else STRING_MATCH_OPCODE(SINPI)
    else STRING_MATCH_OPCODE(COSPI)
    else STRING_MATCH_OPCODE(TANPI)
    else STRING_MATCH_OPCODE(ADDLO)
    else STRING_MATCH_OPCODE(MULLO)
    else STRING_MATCH_OPCODE(CONV)
    else STRING_MATCH_OPCODE(GETEXP)
    else STRING_MATCH_OPCODE(GETMANT)
    else STRING_MATCH_OPCODE(MULHI)
    else STRING_MATCH_OPCODE(CMP)
    else STRING_MATCH_OPCODE(I2F)
    else STRING_MATCH_OPCODE(F2I)
    else STRING_MATCH_OPCODE(ADDSAT)
    else STRING_MATCH_OPCODE(SUBSAT)
    else STRING_MATCH_OPCODE(MULSAT)
    else STRING_MATCH_OPCODE(DP2)
    else STRING_MATCH_OPCODE(UNPACK)
    else STRING_MATCH_OPCODE(IMAGE_WR)
    else STRING_MATCH_OPCODE(MOVA)
    else STRING_MATCH_OPCODE(IMAGE_RD)
    else STRING_MATCH_OPCODE(IMAGE_SAMPLER)
    else STRING_MATCH_OPCODE(NORM_MUL)
    else STRING_MATCH_OPCODE(NORM_DP2)
    else STRING_MATCH_OPCODE(NORM_DP3)
    else STRING_MATCH_OPCODE(NORM_DP4)
    else STRING_MATCH_OPCODE(PRE_DIV)
    else STRING_MATCH_OPCODE(PRE_LOG2)
    else STRING_MATCH_OPCODE(TEXGATHER)
    else STRING_MATCH_OPCODE(TEXFETCH_MS)
    else STRING_MATCH_OPCODE(POPCOUNT)
    else STRING_MATCH_OPCODE(BIT_REVERSAL)
    else STRING_MATCH_OPCODE(BYTE_REVERSAL)
    else STRING_MATCH_OPCODE(TEXPCF)
    else STRING_MATCH_OPCODE(UCARRY)
    else STRING_MATCH_OPCODE(TEXU)
    else STRING_MATCH_OPCODE(TEXU_LOD)
    else STRING_MATCH_OPCODE(MEM_BARRIER)
    else STRING_MATCH_OPCODE(SAMPLER_ASSIGN)
    else STRING_MATCH_OPCODE(GET_SAMPLER_IDX)
    else STRING_MATCH_OPCODE(GET_SAMPLER_LMM)
    else STRING_MATCH_OPCODE(GET_SAMPLER_LBS)
    else STRING_MATCH_OPCODE(IMAGE_RD_3D)
    else STRING_MATCH_OPCODE(IMAGE_WR_3D)
    else STRING_MATCH_OPCODE(FMA_MUL)
    else STRING_MATCH_OPCODE(FMA_ADD)
    else STRING_MATCH_OPCODE(ARCTRIG0)
    else STRING_MATCH_OPCODE(ARCTRIG1)
    else STRING_MATCH_OPCODE(MUL_Z)
    else STRING_MATCH_OPCODE(PARAM_CHAIN)
    else STRING_MATCH_OPCODE(INTRINSIC)
    else STRING_MATCH_OPCODE(INTRINSIC_ST)
    else {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
            Opcode->lineNo,
            Opcode->stringNo,
            slvREPORT_ERROR,
            "unknown opcode: '%s'",
            Opcode->u.identifier));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return asmOpcode;
    }

#undef STRING_MATCH_OPCODE

    asmOpcode.lineNo   = Opcode->lineNo;
    asmOpcode.stringNo = Opcode->stringNo;

    if (Modifiers)
    {
        asmOpcode.condition  = Modifiers->modifiers[sleASM_MODIFIER_OPCODE_CONDITION].value;
        asmOpcode.threadMode = Modifiers->modifiers[sleASM_MODIFIER_OPCODE_THREAD_MODE].value;
        asmOpcode.satuate    = Modifiers->modifiers[sleASM_MODIFIER_OPCODE_SAT].value;
        asmOpcode.round      = Modifiers->modifiers[sleASM_MODIFIER_OPCODE_ROUND].value;
    }

    gcmFOOTER_ARG("<return>=0x%x", asmOpcode);
    return asmOpcode;
}

static gceSTATUS
_GetUniformBlockInfo(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR UniformBlockExpr,
    OUT slsNAME **BlockName,
    OUT gctINT *ArrayIndex
    )
{
    gceSTATUS status;
    slsNAME *blockName = gcvNULL;
    gctINT arrayIndex = 0;
    sloIR_VARIABLE variable;
    sloIR_BINARY_EXPR binaryExpr;
    sloIR_CONSTANT constant;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("UniformBlockExpr=0x%x BlockName=0x%x ArrayIndex=0x%x",
                  UniformBlockExpr, BlockName, ArrayIndex);

    gcmASSERT(slsDATA_TYPE_IsUniformBlock(UniformBlockExpr->dataType));
    gcmASSERT(BlockName && ArrayIndex);

    switch (sloIR_OBJECT_GetType(&UniformBlockExpr->base))
    {
    case slvIR_VARIABLE:
        variable = (sloIR_VARIABLE) &UniformBlockExpr->base;
        blockName = variable->name->u.variableInfo.interfaceBlock;
        break;

    case slvIR_BINARY_EXPR:
        binaryExpr = (sloIR_BINARY_EXPR) &UniformBlockExpr->base;
        gcmASSERT(binaryExpr->type == slvBINARY_SUBSCRIPT);
        if(binaryExpr->type != slvBINARY_SUBSCRIPT) {
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        extension.extension1 = slvEXTENSION1_GPU_SHADER5;
        variable = (sloIR_VARIABLE) &binaryExpr->leftOperand->base;
        blockName = variable->name->u.variableInfo.interfaceBlock;
        gcmASSERT(sloIR_OBJECT_GetType(&binaryExpr->rightOperand->base) == slvIR_CONSTANT ||
                  sloCOMPILER_ExtensionEnabled(Compiler, &extension));
        if(!sloCOMPILER_ExtensionEnabled(Compiler, &extension))
        {
            constant = (sloIR_CONSTANT)&binaryExpr->rightOperand->base;
            gcmASSERT(constant->valueCount == 1);
            arrayIndex = constant->values[0].intValue;
        }
        else
        {
            arrayIndex = -1;
        }
        break;

    default:
        gcmASSERT(0);
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    *BlockName = blockName;
    *ArrayIndex = arrayIndex;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_EXPR
slParseFieldSelectionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand,
    IN slsLexToken * FieldSelection
    )
{
    gceSTATUS               status;
    sleUNARY_EXPR_TYPE      exprType;
    slsNAME *               fieldName = gcvNULL;
    slsCOMPONENT_SELECTION  componentSelection;
    sloIR_CONSTANT          resultConstant;
    sloIR_UNARY_EXPR        unaryExpr;

    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x FieldSelection=0x%x",
                  Compiler, Operand, FieldSelection);

    gcmASSERT(FieldSelection);

    if (Operand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (slsDATA_TYPE_IsStruct(Operand->dataType) ||
        slsDATA_TYPE_IsInterfaceBlock(Operand->dataType))
    {
        gcmASSERT(Operand->dataType->fieldSpace);

        exprType = slvUNARY_FIELD_SELECTION;

        status = slsNAME_SPACE_Search(Compiler,
                                      Operand->dataType->fieldSpace,
                                      FieldSelection->u.fieldSelection,
                                      gcvNULL,
                                      gcvNULL,
                                      gcvFALSE,
                                      gcvFALSE,
                                      &fieldName);

        if (status != gcvSTATUS_OK)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            FieldSelection->lineNo,
                                            FieldSelection->stringNo,
                                            slvREPORT_ERROR,
                                            "unknown field: '%s'",
                                            FieldSelection->u.fieldSelection));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        fieldName->u.variableInfo.isReferenced = gcvTRUE;

        gcmASSERT(fieldName->type == slvFIELD_NAME);
        if(slsDATA_TYPE_IsUniformBlock(Operand->dataType))
        {
            slsNAME *blockName;
            gctINT arrayIndex;
            sloEXTENSION extension = {0};

            status = _GetUniformBlockInfo(Compiler, Operand, &blockName, &arrayIndex);
            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            extension.extension1 = slvEXTENSION1_GPU_SHADER5;
            gcmASSERT(fieldName->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER);
            if(!(arrayIndex >= 0 || (arrayIndex == -1 && sloCOMPILER_ExtensionEnabled(Compiler, &extension))))
            {
                gcmASSERT(gcvFALSE);
            }
            status = _SetUniformBlockMemberActive(blockName, arrayIndex, fieldName);
            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
    }
    else if (slsDATA_TYPE_IsBVecOrIVecOrVec(Operand->dataType))
    {
        exprType = slvUNARY_COMPONENT_SELECTION;

        status = _ParseComponentSelection(Compiler,
                                          slmDATA_TYPE_vectorSize_NOCHECK_GET(Operand->dataType),
                                          FieldSelection,
                                          &componentSelection);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }
    else
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a struct or vector typed expression"));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Constant calculation */
    if (sloIR_OBJECT_GetType(&Operand->base) == slvIR_CONSTANT)
    {
        status = sloIR_UNARY_EXPR_Evaluate(Compiler,
                                           exprType,
                                           (sloIR_CONSTANT)Operand,
                                           fieldName,
                                           &componentSelection,
                                           &resultConstant);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        gcmFOOTER_ARG("<return>=0x%x", &resultConstant->exprBase);
        return &resultConstant->exprBase;
    }

    /* Create unary expression */
    status = sloIR_UNARY_EXPR_Construct(Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        exprType,
                                        Operand,
                                        fieldName,
                                        &componentSelection,
                                        &unaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<UNARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\""
                                  " fieldSelection=\"%s\" />",
                                  slGetIRUnaryExprTypeName(exprType),
                                  Operand->base.lineNo,
                                  Operand->base.stringNo,
                                  FieldSelection->u.fieldSelection));

    gcmFOOTER_ARG("<return>=0x%x", &unaryExpr->exprBase);
    return &unaryExpr->exprBase;
}

static sloIR_CONSTANT
_ParseCreateConstant(
IN sloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctINT Type,
sluCONSTANT_VALUE *Value
)
{
  gceSTATUS status;
  slsDATA_TYPE *dataType;
  sloIR_CONSTANT constant;

/* Create the data type */
  status = sloCOMPILER_CreateDataType(Compiler,
                                      Type,
                                      gcvNULL,
                                      &dataType);
  if (gcmIS_ERROR(status)) return gcvNULL;

/* Create the constant */
  dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;
  status = sloIR_CONSTANT_Construct(Compiler,
                                    LineNo,
                                    StringNo,
                                    dataType,
                                    &constant);
  if (gcmIS_ERROR(status)) return gcvNULL;

/* Add the constant value */
  status = sloIR_CONSTANT_AddValues(Compiler,
                                    constant,
                                    1,
                                    Value);
  if (gcmIS_ERROR(status)) return gcvNULL;
  return constant;
}

static sloIR_EXPR
_MakeFuncCallExpr(
    IN sloCOMPILER Compiler,
    IN gctSTRING NameString,
    IN sloIR_EXPR Arg
    )
{
    slsLexToken  funcName;
    sloIR_POLYNARY_EXPR funcCall;

    (void)gcoOS_ZeroMemory((gctPOINTER)&funcName, sizeof(slsLexToken));
    funcName.lineNo = Arg->base.lineNo;
    funcName.stringNo = Arg->base.stringNo;
    funcName.type = T_IDENTIFIER;
    sloCOMPILER_AllocatePoolString(Compiler,
                                   NameString,
                                   &funcName.u.identifier);

    funcCall = slParseFuncCallHeaderExpr(Compiler,
                                         &funcName);

    funcCall = slParseFuncCallArgument(Compiler,
                                       funcCall,
                                       Arg);

    return  slParseFuncCallExprAsExpr(Compiler,
                                      funcCall);
}

extern gctINT
_GetInputArraySizeByPrimitiveType(
    IN slvGS_PRIMITIVE Primitive
    );

sloIR_EXPR
slParseLengthMethodExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    )
{
    sloIR_CONSTANT resultConstant = gcvNULL;
    sleSHADER_TYPE shaderType;

    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x",
                  Compiler, Operand);

    shaderType = Compiler->shaderType;

    if (Operand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }
    else
    {
        if (slsDATA_TYPE_IsArray(Operand->dataType))
        {
            if (slsDATA_TYPE_IsArrayLengthImplicit(Operand->dataType))
            {
                if (shaderType == slvSHADER_TYPE_GS)
                {
                    slsLAYOUT_QUALIFIER layout;
                    sluCONSTANT_VALUE value[1];

                    if (sloCOMPILER_IsOGLVersion(Compiler) &&
                        Operand->dataType->qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_IN &&
                        Operand->dataType->qualifiers.storage != slvSTORAGE_QUALIFIER_IN_IO_BLOCK)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Operand->base.lineNo,
                                                        Operand->base.stringNo,
                                                        slvREPORT_ERROR,
                                                        "The array must be declared with a size before using the method length."));

                        gcmFOOTER_ARG("<return>=%s", "<nil>");
                        return gcvNULL;
                    }

                    sloCOMPILER_GetDefaultLayout(Compiler,
                                                 &layout,
                                                 slvSTORAGE_QUALIFIER_IN);

                    if (layout.gsPrimitive == slvGS_PRIMITIVE_NONE)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Operand->base.lineNo,
                                                        Operand->base.stringNo,
                                                        slvREPORT_ERROR,
                                                        "For inputs declared without an array size,"
                                                        "a layout must be declared before any use of "
                                                        "the method length or other any array use that requires the array size to be known."));

                        gcmFOOTER_ARG("<return>=%s", "<nil>");
                        return gcvNULL;
                    }
                    value->intValue = _GetInputArraySizeByPrimitiveType(layout.gsPrimitive);
                    resultConstant = _ParseCreateConstant(Compiler,
                                                          Operand->base.lineNo,
                                                          Operand->base.stringNo,
                                                          T_INT,
                                                          value);
                }
                else
                {
                    sloIR_EXPR expr;
                    expr = _MakeFuncCallExpr(Compiler,
                                             "_viv_arrayLengthMethod",
                                             Operand);
                    sloCOMPILER_GetDefaultPrecision(Compiler,
                                                    expr->dataType->elementType,
                                                    &expr->dataType->qualifiers.precision);
                    gcmFOOTER_ARG("<return>=0x%x", expr);
                    return expr;
                }
            }
            else
            {
                sluCONSTANT_VALUE value[1];
                value->intValue = Operand->dataType->arrayLength;
                resultConstant = _ParseCreateConstant(Compiler,
                                                      Operand->base.lineNo,
                                                      Operand->base.stringNo,
                                                      T_INT,
                                                      value);
            }
        }
        else
        {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           Operand->base.lineNo,
                                           Operand->base.stringNo,
                                           slvREPORT_ERROR,
                                           "require an array expression"));

           gcmFOOTER_ARG("<return>=%s", "<nil>");
           return gcvNULL;
        }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<ARRAY_LENGTH_METHOD line=\"%d\" string=\"%d\""
                                " array=\"0x%x\" length()=\"%d\" />",
                                Operand->base.lineNo,
                                Operand->base.stringNo,
                                Operand,
                                Operand->dataType->arrayLength));

    gcmFOOTER_ARG("<return>=0x%x", &resultConstant->exprBase);
    return &resultConstant->exprBase;
}

slsLexToken
slParseCheckStorage(
    IN sloCOMPILER Compiler,
    IN slsLexToken Storage,
    IN slsLexToken InOrOut
    )
{
    sleSHADER_TYPE  shaderType;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    shaderType = Compiler->shaderType;

    if (InOrOut.type == T_IN)
    {
        if (Storage.type == T_PATCH)
        {
            if (shaderType != slvSHADER_TYPE_TES)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                InOrOut.lineNo,
                                                InOrOut.stringNo,
                                                slvREPORT_ERROR,
                                                "patch in can only be used in tessellation evaluation shader."));

            }
            else
            {
                slsQUALIFIERS_SET_FLAG(&(InOrOut.u.qualifiers), slvQUALIFIERS_FLAG_PATCH);
            }
        }
    }
    else
    {
        gcmASSERT(InOrOut.type == T_OUT);

        if (Storage.type == T_PATCH)
        {
            if (shaderType != slvSHADER_TYPE_TCS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                InOrOut.lineNo,
                                                InOrOut.stringNo,
                                                slvREPORT_ERROR,
                                                "patch in can only be used in tessellation control shader."));

            }
            else
            {
                slsQUALIFIERS_SET_FLAG(&(InOrOut.u.qualifiers), slvQUALIFIERS_FLAG_PATCH);
            }
        }
    }

    gcmFOOTER_NO();
    return InOrOut;
}

void
slParseFuncDefinitionBegin(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    slsNAME_SPACE       *nameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x", Compiler, FuncName);

    if (FuncName == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    /* For ES20, A function body has a scope nested inside the function’s definition. */
    if (!sloCOMPILER_IsHaltiVersion(Compiler))
    {
        status = sloCOMPILER_CreateNameSpace(Compiler,
                                             FuncName->symbol,
                                             slvNAME_SPACE_TYPE_FUNCTION,
                                             &nameSpace);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return;
        }
    }
    /* For ES30 and above, A function's parameter declarations and body together form a single scope. */
    else
    {
        nameSpace = sloCOMPILER_GetCurrentSpace(Compiler);
    }
    FuncName->u.funcInfo.functionBodySpace = nameSpace;

    gcmFOOTER_NO();
    return;
}

void
slParseFuncDefinitionEnd(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    )
{
    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x", Compiler, FuncName);

    if (FuncName == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    /* For ES20, pop the function body name space. */
    if (!sloCOMPILER_IsHaltiVersion(Compiler))
    {
        sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);
    }

    gcmFOOTER_NO();
    return;
}

gceSTATUS
_CheckErrorForIncOrDecExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x",
                  Compiler, Operand);

    gcmASSERT(Operand);
    gcmASSERT(Operand->dataType);

    /* Check the operand */
    status = _CheckErrorAsLValueExpr(Compiler, Operand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsIntOrIVec(Operand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(Operand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_EXPR
slParseIncOrDecExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sleUNARY_EXPR_TYPE ExprType,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS           status;
    gctUINT             lineNo;
    gctUINT             stringNo;
    sloIR_UNARY_EXPR    unaryExpr;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x ExprType=%d Operand=0x%x",
                  Compiler, StartToken, ExprType, Operand);

    if (Operand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (StartToken != gcvNULL)
    {
        lineNo      = StartToken->lineNo;
        stringNo    = StartToken->stringNo;
    }
    else
    {
        lineNo      = Operand->base.lineNo;
        stringNo    = Operand->base.stringNo;
    }

    /* Check Error */
    status = _CheckErrorForIncOrDecExpr(
                                        Compiler,
                                        Operand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create unary expression */
    status = sloIR_UNARY_EXPR_Construct(
                                        Compiler,
                                        lineNo,
                                        stringNo,
                                        ExprType,
                                        Operand,
                                        gcvNULL,
                                        gcvNULL,
                                        &unaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<UNARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                                slGetIRUnaryExprTypeName(ExprType),
                                lineNo,
                                stringNo));

    gcmFOOTER_ARG("<return>=0x%x", &unaryExpr->exprBase);
    return &unaryExpr->exprBase;
}

static gceSTATUS
_CheckErrorForPosOrNegExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x",
                  Compiler, Operand);

    gcmASSERT(Operand);
    gcmASSERT(Operand->dataType);

    /* Check the operand */
    if (!slsDATA_TYPE_IsIntOrIVec(Operand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(Operand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForNotExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x",
                  Compiler, Operand);

    gcmASSERT(Operand);
    gcmASSERT(Operand->dataType);

    /* Check the operand */
    if (!slsDATA_TYPE_IsBool(Operand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForBitwiseNotExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x Operand=0x%x",
                  Compiler, Operand);

    gcmASSERT(Operand);
    gcmASSERT(Operand->dataType);

    /* Check the operand */
    if (!(slsDATA_TYPE_IsUInt(Operand->dataType) || slsDATA_TYPE_IsIntOrIVec(Operand->dataType)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_EXPR
slParseNormalUnaryExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Operator,
    IN sloIR_EXPR Operand
    )
{
    gceSTATUS           status;
    sleUNARY_EXPR_TYPE  exprType = slvUNARY_NEG;
    sloIR_CONSTANT      resultConstant;
    sloIR_UNARY_EXPR    unaryExpr;

    gcmHEADER_ARG("Compiler=0x%x Operator=0x%x Operand=0x%x",
                  Compiler, Operator, Operand);

    gcmASSERT(Operator);

    if (Operand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if ((Operand->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY) != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Operand->base.lineNo,
                                        Operand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot access to writeonly data"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }
    switch (Operator->u.operator)
    {
    case '-':
        exprType = slvUNARY_NEG;
        /* Fall through. */

    case '+':
        status = _CheckErrorForPosOrNegExpr(
                                            Compiler,
                                            Operand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case '!':
        exprType = slvUNARY_NOT;

        status = _CheckErrorForNotExpr(
                                    Compiler,
                                    Operand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case '~':
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
            exprType = slvUNARY_NOT_BITWISE;

            /* The operand of operator (~) must be of type signed or unsigned integer or integer vector. */
            status = _CheckErrorForBitwiseNotExpr(
                                        Compiler,
                                        Operand);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            break;
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            slvREPORT_ERROR,
                                            "reserved unary operator '~'"));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (Operator->u.operator == '+')
    {
        /* No action needed for '+' operator. */
        gcmFOOTER_ARG("<return>=0x%x", Operand);
        return Operand;
    }

    /* Constant calculation */
    if (sloIR_OBJECT_GetType(&Operand->base) == slvIR_CONSTANT)
    {
        status = sloIR_UNARY_EXPR_Evaluate(
                                            Compiler,
                                            exprType,
                                            (sloIR_CONSTANT)Operand,
                                            gcvNULL,
                                            gcvNULL,
                                            &resultConstant);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        gcmFOOTER_ARG("<return>=0x%x", &resultConstant->exprBase);
        return &resultConstant->exprBase;
    }

    /* Create unary expression */
    status = sloIR_UNARY_EXPR_Construct(
                                        Compiler,
                                        Operator->lineNo,
                                        Operator->stringNo,
                                        exprType,
                                        Operand,
                                        gcvNULL,
                                        gcvNULL,
                                        &unaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<UNARY_EXPR type=\"%c\" line=\"%d\" string=\"%d\" />",
                                Operator->u.operator,
                                Operator->lineNo,
                                Operator->stringNo));

    gcmFOOTER_ARG("<return>=0x%x", &unaryExpr->exprBase);
    return &unaryExpr->exprBase;
}

static gctCONST_STRING
_GetBinaryOperatorName(
    IN gctINT TokenType
    )
{
    switch (TokenType)
    {
    case '*':               return "*";
    case '/':               return "/";
    case '%':               return "%";

    case '+':               return "+";
    case '-':               return "-";

    case T_LEFT_OP:         return "<<";
    case T_RIGHT_OP:        return ">>";

    case '<':               return "<";
    case '>':               return ">";

    case T_LE_OP:           return "<=";
    case T_GE_OP:           return ">=";

    case T_EQ_OP:           return "==";
    case T_NE_OP:           return "!=";

    case '&':               return "&";
    case '^':               return "^";
    case '|':               return "|";

    case T_AND_OP:          return "&&";
    case T_XOR_OP:          return "^^";
    case T_OR_OP:           return "||";

    case ',':               return ",";

    case '=':               return "=";

    case T_MUL_ASSIGN:      return "*=";
    case T_DIV_ASSIGN:      return "/=";
    case T_MOD_ASSIGN:      return "%=";
    case T_ADD_ASSIGN:      return "+=";
    case T_SUB_ASSIGN:      return "-=";
    case T_LEFT_ASSIGN:     return "<<=";
    case T_RIGHT_ASSIGN:    return ">>=";
    case T_AND_ASSIGN:      return "&=";
    case T_XOR_ASSIGN:      return "^=";
    case T_OR_ASSIGN:       return "|=";

    default:
        gcmASSERT(0);
        return "invalid";
    }
}

static gceSTATUS
_CheckErrorForArithmeticExpr(
    IN sloCOMPILER Compiler,
    IN gctBOOL IsMul,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x IsMul=%d LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, IsMul, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the operands */
    if (!slsDATA_TYPE_IsIntOrIVec(LeftOperand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsIntOrIVec(RightOperand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(RightOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        status = slMakeImplicitConversionForOperandPair(Compiler,
                                                      LeftOperand,
                                                      RightOperand,
                                                      gcvFALSE);
        if (gcmIS_ERROR(status))
        {
            return status;
        }
    }
    else
    {
        sloIR_EXPR_SetToBeTheSameDataType(LeftOperand);
        sloIR_EXPR_SetToBeTheSameDataType(RightOperand);
    }

    if (!slsDATA_TYPE_IsEqual(LeftOperand->toBeDataType, RightOperand->toBeDataType))
    {
        if (slsDATA_TYPE_IsInt(LeftOperand->toBeDataType))
        {
            if (!slsDATA_TYPE_IsIVec(RightOperand->toBeDataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require an integer typed expression"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (slsDATA_TYPE_IsIVec(LeftOperand->toBeDataType))
        {
            if (!slsDATA_TYPE_IsInt(RightOperand->toBeDataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require an int or ivec%d expression",
                                                slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (slsDATA_TYPE_IsFloat(LeftOperand->toBeDataType))
        {
            if (!slsDATA_TYPE_IsVecOrMat(RightOperand->toBeDataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require a float or vec or mat expression"));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (slsDATA_TYPE_IsVec(LeftOperand->toBeDataType))
        {
            if (!IsMul)
            {
                if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(
                                                    Compiler,
                                                    RightOperand->base.lineNo,
                                                    RightOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "require a float or vec%d expression",
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }
            }
            else
            {
                if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                    && !(slsDATA_TYPE_IsMat(RightOperand->toBeDataType)
                        && slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)
                        == slmDATA_TYPE_matrixRowCount_GET(RightOperand->toBeDataType)))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(
                                                    Compiler,
                                                    RightOperand->base.lineNo,
                                                    RightOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "require a float or vec%d or mat%d expression",
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType),
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }
            }
        }
        else if (slsDATA_TYPE_IsMat(LeftOperand->toBeDataType))
        {
            if (!IsMul)
            {
                if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                    && !(slsDATA_TYPE_IsMat(RightOperand->toBeDataType)
                        && slmDATA_TYPE_matrixSize_GET(LeftOperand->toBeDataType)
                        == slmDATA_TYPE_matrixSize_GET(RightOperand->toBeDataType)))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(
                                                    Compiler,
                                                    RightOperand->base.lineNo,
                                                    RightOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "require a float or mat%d expression",
                                                    slmDATA_TYPE_matrixSize_GET(LeftOperand->toBeDataType)));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }
            }
            else
            {
                if (!sloCOMPILER_IsHaltiVersion(Compiler))
                {
                    if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                        && !(slsDATA_TYPE_IsVec(RightOperand->toBeDataType)
                            && slmDATA_TYPE_matrixColumnCount_GET(LeftOperand->toBeDataType)
                            == slmDATA_TYPE_vectorSize_NOCHECK_GET(RightOperand->toBeDataType)))
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(
                                                        Compiler,
                                                        RightOperand->base.lineNo,
                                                        RightOperand->base.stringNo,
                                                        slvREPORT_ERROR,
                                                        "require a float or vec%d or mat%d expression",
                                                        LeftOperand->toBeDataType->matrixSize,
                                                        LeftOperand->toBeDataType->matrixSize));

                        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                        gcmFOOTER();
                        return status;
                    }
                }
                else
                {
                    if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                        && !((slsDATA_TYPE_IsVec(RightOperand->toBeDataType) || slsDATA_TYPE_IsMat(RightOperand->toBeDataType))
                            && slmDATA_TYPE_matrixColumnCount_GET(LeftOperand->toBeDataType)
                            == slmDATA_TYPE_vectorSize_NOCHECK_GET(RightOperand->toBeDataType)))
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(
                                                        Compiler,
                                                        RightOperand->base.lineNo,
                                                        RightOperand->base.stringNo,
                                                        slvREPORT_ERROR,
                                                        "require a float or vec%d or mat%d expression",
                                                        LeftOperand->toBeDataType->matrixSize,
                                                        LeftOperand->toBeDataType->matrixSize));

                        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                        gcmFOOTER();
                        return status;
                    }
                }
            }
        }
        else
        {
            gcmASSERT(0);
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForRelationalExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloEXTENSION extension = {0};
    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the operands */
    if (!slsDATA_TYPE_IsInt(LeftOperand->dataType)
        && !slsDATA_TYPE_IsFloat(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar integer or scalar floating-point expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        status = slMakeImplicitConversionForOperandPair(Compiler,
                                                      LeftOperand,
                                                      RightOperand,
                                                      gcvFALSE);
        if (gcmIS_ERROR(status)) {
            return status;
        }
    }
    else
    {
        sloIR_EXPR_SetToBeTheSameDataType(LeftOperand);
        sloIR_EXPR_SetToBeTheSameDataType(RightOperand);
    }

    if (slsDATA_TYPE_IsInt(LeftOperand->toBeDataType)
        && !slsDATA_TYPE_IsInt(RightOperand->toBeDataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar integer expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
    else if (slsDATA_TYPE_IsFloat(LeftOperand->toBeDataType)
        && !slsDATA_TYPE_IsFloat(RightOperand->toBeDataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar floating-point expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForEqualityExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the operands */
    if (!slsDATA_TYPE_IsAssignableAndComparable(Compiler, LeftOperand->dataType))
    {
        if (sloCOMPILER_IsHaltiVersion(Compiler)) {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           LeftOperand->base.lineNo,
                                           LeftOperand->base.stringNo,
                                           slvREPORT_ERROR,
                                           "require any typed expression except sampler types",
                                           " and structures containing sampler types"));
        }
        else {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           LeftOperand->base.lineNo,
                                           LeftOperand->base.stringNo,
                                           slvREPORT_ERROR,
                                           "require any typed expression except arrays, structures"
                                           " containing arrays, sampler types, and structures"
                                           " containing sampler types"));
        }

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        status = slMakeImplicitConversionForOperandPair(Compiler,
                                                      LeftOperand,
                                                      RightOperand,
                                                      gcvFALSE);
        if (gcmIS_ERROR(status)) {
            return status;
        }
    }
    else
    {
        sloIR_EXPR_SetToBeTheSameDataType(LeftOperand);
        sloIR_EXPR_SetToBeTheSameDataType(RightOperand);
    }

    if (!slsDATA_TYPE_IsEqual(LeftOperand->toBeDataType, RightOperand->toBeDataType))
    {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForLogicalExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the operands */
    if (!slsDATA_TYPE_IsBool(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsBool(RightOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckBitwiseShiftExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
        Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    if(!slmDATA_TYPE_IsIntegerType(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if(!slmDATA_TYPE_IsIntegerType(RightOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (slsDATA_TYPE_IsScalar(LeftOperand->dataType))
    {
        if(!slsDATA_TYPE_IsScalar(RightOperand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "Right operand of shift operator has to be scalar to match with the left operand"));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }
    else if(slsDATA_TYPE_IsVectorType(LeftOperand->dataType))
    {
        if(slsDATA_TYPE_IsVectorType(RightOperand->dataType))
        {
            if(slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->dataType) !=
                slmDATA_TYPE_vectorSize_NOCHECK_GET(RightOperand->dataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                LeftOperand->base.lineNo,
                                                LeftOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require the right and left operands of matching vector size"));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (!slsDATA_TYPE_IsScalar(RightOperand->dataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require the right operand of shift operator to be scalar to match with the left operand"));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckAssignImplicitOperability(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE *leftDataType;
    slsDATA_TYPE *rightDataType;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                   Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand->dataType);

    leftDataType = LeftOperand->dataType;
    rightDataType = RightOperand->dataType;

    /* Check the operands */
    if(!slmDATA_TYPE_IsArithmeticType(leftDataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "arithmetic operand required"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
    if(!slmDATA_TYPE_IsArithmeticType(rightDataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "arithmetic operand required"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if(slsDATA_TYPE_IsVectorType(leftDataType)) {
        if(!slsDATA_TYPE_IsScalar(rightDataType) &&
            !slmDATA_TYPE_IsSameVectorType(leftDataType, rightDataType)) {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require a scalar or a matching vector typed expression"));
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForSequenceExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckImplicitOperability(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
)
{
  gceSTATUS status = gcvSTATUS_OK;
  slsDATA_TYPE *leftDataType;
  slsDATA_TYPE *rightDataType;

  gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

  gcmASSERT(LeftOperand->dataType);
  gcmASSERT(RightOperand->dataType);

  leftDataType = LeftOperand->dataType;
  rightDataType = RightOperand->dataType;

  /* Check the operands */
  if(!slmDATA_TYPE_IsArithmeticType(leftDataType)) {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                    LeftOperand->base.lineNo,
                                    LeftOperand->base.stringNo,
                                    slvREPORT_ERROR,
                                    "arithmetic operand required"));
    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    gcmFOOTER();
    return status;
  }
  if(!slmDATA_TYPE_IsArithmeticType(rightDataType)) {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                    RightOperand->base.lineNo,
                                    RightOperand->base.stringNo,
                                    slvREPORT_ERROR,
                                    "arithmetic operand required"));
    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    gcmFOOTER();
    return status;
  }

  if(slsDATA_TYPE_IsScalar(leftDataType)) {
    if(slsDATA_TYPE_IsVectorType(rightDataType)) {
       if(leftDataType->elementType != rightDataType->elementType) {
         gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                         LeftOperand->base.lineNo,
                                         LeftOperand->base.stringNo,
                         slvREPORT_ERROR,
                                         "require matching elemnet type of operands in expression"));
         status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
         gcmFOOTER();
         return status;
       }
    }
    else if(!slsDATA_TYPE_IsScalar(rightDataType)) {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }
    else if(leftDataType->elementType != rightDataType->elementType) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require matching elemnet type of operands in expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
  }
  else if(slsDATA_TYPE_IsScalar(rightDataType)) {
    if(slsDATA_TYPE_IsVectorType(leftDataType)) {
       if(rightDataType->elementType != leftDataType->elementType) {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           RightOperand->base.lineNo,
                                           RightOperand->base.stringNo,
                           slvREPORT_ERROR,
                                           "require matching elemnet type of operands in expression"));
           status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
           gcmFOOTER();
           return status;
       }
    }
    else if(!slsDATA_TYPE_IsScalar(leftDataType)) {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }
    else if(leftDataType->elementType != rightDataType->elementType) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require matching elemnet type of operands in expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }
  }
  else if(!slmDATA_TYPE_IsSameVectorType(leftDataType, rightDataType)) {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    slvREPORT_ERROR,
                    "require a matching vector typed expression"));
    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    gcmFOOTER();
    return status;
  }

  gcmFOOTER_NO();
  return gcvSTATUS_OK;
}

static gceSTATUS
_CheckBitwiseLogicalExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
)
{
  gceSTATUS status = gcvSTATUS_OK;

  gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->dataType);

/* Check the operands */
  if (!slmDATA_TYPE_IsIntegerType(LeftOperand->dataType)) {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    LeftOperand->base.lineNo,
                    LeftOperand->base.stringNo,
                    slvREPORT_ERROR,
                    "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
  }

  if (!slmDATA_TYPE_IsIntegerType(RightOperand->dataType)) {
    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    slvREPORT_ERROR,
                    "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
  }

  status = _CheckImplicitOperability(Compiler, LeftOperand, RightOperand);
  gcmFOOTER();
  return status;
}

sloIR_EXPR
slParseNormalBinaryExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN slsLexToken * Operator,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS           status;
    sleBINARY_EXPR_TYPE exprType = (sleBINARY_EXPR_TYPE) 0;
    sloIR_CONSTANT      resultConstant;
    sloIR_BINARY_EXPR   binaryExpr;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x Operator=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, Operator, RightOperand);

    gcmASSERT(Operator);

    if (LeftOperand == gcvNULL || RightOperand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* check if the left operand is writeonly */
    if ((LeftOperand->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY) != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot access to writeonly data"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if ((RightOperand->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY) != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot access to writeonly data"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switch (Operator->u.operator)
    {
    case '%':
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
            status = _CheckErrorForArithmeticExpr(Compiler,
                                                  gcvFALSE,
                                                  LeftOperand,
                                                  RightOperand);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            exprType = slvBINARY_MOD;
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            slvREPORT_ERROR,
                                            "reserved binary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case '&':
    case '^':
    case '|':
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
             status = _CheckBitwiseLogicalExpr(Compiler,
                                               LeftOperand,
                                               RightOperand);

             if (gcmIS_ERROR(status))
             {
                  gcmFOOTER_ARG("<return>=%s", "<nil>");
                  return gcvNULL;
             }

             switch (Operator->u.operator)
             {
             case '&':
                  exprType = slvBINARY_AND_BITWISE;
                  break;

             case '|':
                  exprType = slvBINARY_OR_BITWISE;
                  break;

             case '^':
                  exprType = slvBINARY_XOR_BITWISE;
                  break;
             }
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            slvREPORT_ERROR,
                                            "reserved bit-wise binary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case T_LEFT_OP:
    case T_RIGHT_OP:
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
             status = _CheckBitwiseShiftExpr(Compiler,
                                             LeftOperand,
                                             RightOperand);
             if (gcmIS_ERROR(status))
             {
                  gcmFOOTER_ARG("<return>=%s", "<nil>");
                  return gcvNULL;
             }

             switch (Operator->u.operator)
             {
             case T_LEFT_OP:
                  exprType = slvBINARY_LSHIFT;
                  break;

             case T_RIGHT_OP:
                  exprType = slvBINARY_RSHIFT;
                  break;
             }
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            slvREPORT_ERROR,
                                            "reserved shift binary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case '*':
    case '/':
    case '+':
    case '-':
        switch (Operator->u.operator)
        {
        case '*': exprType = slvBINARY_MUL; break;
        case '/': exprType = slvBINARY_DIV; break;
        case '+': exprType = slvBINARY_ADD; break;
        case '-': exprType = slvBINARY_SUB; break;
        }

        status = _CheckErrorForArithmeticExpr(Compiler,
                                              (Operator->u.operator == '*'),
                                              LeftOperand,
                                              RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case '<':
    case '>':
    case T_LE_OP:
    case T_GE_OP:
        switch (Operator->u.operator)
        {
        case '<':       exprType = slvBINARY_LESS_THAN;             break;
        case '>':       exprType = slvBINARY_GREATER_THAN;          break;
        case T_LE_OP:   exprType = slvBINARY_LESS_THAN_EQUAL;       break;
        case T_GE_OP:   exprType = slvBINARY_GREATER_THAN_EQUAL;    break;
        }

        status = _CheckErrorForRelationalExpr(
                                            Compiler,
                                            LeftOperand,
                                            RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case T_EQ_OP:
    case T_NE_OP:
        switch (Operator->u.operator)
        {
        case T_EQ_OP: exprType = slvBINARY_EQUAL;       break;
        case T_NE_OP: exprType = slvBINARY_NOT_EQUAL;   break;
        }

        status = _CheckErrorForEqualityExpr(
                                        Compiler,
                                        LeftOperand,
                                        RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case T_AND_OP:
    case T_XOR_OP:
    case T_OR_OP:
        switch (Operator->u.operator)
        {
        case T_AND_OP:  exprType = slvBINARY_AND;   break;
        case T_XOR_OP:  exprType = slvBINARY_XOR;   break;
        case T_OR_OP:   exprType = slvBINARY_OR;    break;
        }

        status = _CheckErrorForLogicalExpr(
                                        Compiler,
                                        LeftOperand,
                                        RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case ',':
        exprType = slvBINARY_SEQUENCE;

        status = _CheckErrorForSequenceExpr(
                                            Compiler,
                                            LeftOperand,
                                            RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if (!sloCOMPILER_IsHaltiVersion(Compiler) &&
            sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_CONSTANT)
        {
            gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &LeftOperand->base));
            gcmFOOTER_ARG("<return>=0x%x", RightOperand);
            return RightOperand;
        }

        break;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (sloIR_EXPR_ImplicitConversionDone(LeftOperand))
    {
        LeftOperand = _MakeImplicitConversionExpr(Compiler, LeftOperand);
    }

    if (sloIR_EXPR_ImplicitConversionDone(RightOperand))
    {
        RightOperand = _MakeImplicitConversionExpr(Compiler, RightOperand);
    }

    /* Constant calculation */
    if ((exprType != slvBINARY_SEQUENCE || !sloCOMPILER_IsHaltiVersion(Compiler)) &&
        sloIR_OBJECT_GetType(&LeftOperand->base) == slvIR_CONSTANT &&
        sloIR_OBJECT_GetType(&RightOperand->base) == slvIR_CONSTANT)
    {
        status = sloIR_BINARY_EXPR_Evaluate(
                                            Compiler,
                                            exprType,
                                            (sloIR_CONSTANT)LeftOperand,
                                            (sloIR_CONSTANT)RightOperand,
                                            &resultConstant);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        gcmFOOTER_ARG("<return>=0x%x", &resultConstant->exprBase);
        return &resultConstant->exprBase;
    }

    /* Create binary expression */
    status = sloIR_BINARY_EXPR_Construct(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        LeftOperand->base.lineNo,
                                        exprType,
                                        LeftOperand,
                                        RightOperand,
                                        &binaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<BINARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                                _GetBinaryOperatorName(Operator->u.operator),
                                LeftOperand->base.lineNo,
                                LeftOperand->base.stringNo));

    gcmFOOTER_ARG("<return>=0x%x", &binaryExpr->exprBase);
    return &binaryExpr->exprBase;
}

static gceSTATUS
_CheckErrorForSelectionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr,
    IN sloIR_EXPR TrueOperand,
    IN sloIR_EXPR FalseOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x CondExpr=0x%x TrueOperand=0x%x FalseOperand=0x%x",
                  Compiler, CondExpr, TrueOperand, FalseOperand);

    gcmASSERT(CondExpr);
    gcmASSERT(CondExpr->dataType);
    gcmASSERT(TrueOperand);
    gcmASSERT(TrueOperand->dataType);
    gcmASSERT(FalseOperand);
    gcmASSERT(FalseOperand->dataType);

    /* Check the operands */
    if (!slsDATA_TYPE_IsBool(CondExpr->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        CondExpr->base.lineNo,
                                        CondExpr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (slsDATA_TYPE_IsArray(TrueOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        TrueOperand->base.lineNo,
                                        TrueOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a non-array expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsEqual(TrueOperand->dataType, FalseOperand->dataType))
    {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_EXPR
slParseSelectionExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr,
    IN sloIR_EXPR TrueOperand,
    IN sloIR_EXPR FalseOperand
    )
{
    gceSTATUS       status;
    sloIR_SELECTION selection;
    sloIR_CONSTANT  condConstant;
    gctBOOL         condValue;
    slsDATA_TYPE *  dataType;
    sltPRECISION_QUALIFIER    precision;

    gcmHEADER_ARG("Compiler=0x%x CondExpr=0x%x TrueOperand=0x%x FalseOperand=0x%x",
                  Compiler, CondExpr, TrueOperand, FalseOperand);

    if (CondExpr == gcvNULL || TrueOperand == gcvNULL || FalseOperand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Check error */
    status = _CheckErrorForSelectionExpr(
                                        Compiler,
                                        CondExpr,
                                        TrueOperand,
                                        FalseOperand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Constant calculation */
    if (sloIR_OBJECT_GetType(&CondExpr->base) == slvIR_CONSTANT)
    {
        condConstant = (sloIR_CONSTANT)CondExpr;
        gcmASSERT(condConstant->valueCount == 1);
        gcmASSERT(condConstant->values);

        condValue = condConstant->values[0].boolValue;

        gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &CondExpr->base));

        if (condValue)
        {
            gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &FalseOperand->base));
            gcmFOOTER_ARG("<return>=0x%x", TrueOperand);
            return TrueOperand;
        }
        else
        {
            gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &TrueOperand->base));
            gcmFOOTER_ARG("<return>=0x%x", FalseOperand);
            return FalseOperand;
        }
    }

    if(slmDATA_TYPE_IsHigherPrecision(FalseOperand->dataType,
                                      TrueOperand->dataType))
    {
        precision = FalseOperand->dataType->qualifiers.precision;
    }
    else
    {
        precision = TrueOperand->dataType->qualifiers.precision;
    }

    /* Create the selection */
    status = sloCOMPILER_CloneDataType(Compiler,
                                       slvSTORAGE_QUALIFIER_CONST,
                                       precision,
                                       TrueOperand->dataType,
                                       &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloIR_SELECTION_Construct(
                                    Compiler,
                                    CondExpr->base.lineNo,
                                    CondExpr->base.stringNo,
                                    dataType,
                                    CondExpr,
                                    &TrueOperand->base,
                                    &FalseOperand->base,
                                    &selection);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<SELECTION_EXPR line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                                " TrueOperand=\"0x%x\" FalseOperand=\"0x%x\" />",
                                CondExpr->base.lineNo,
                                CondExpr->base.stringNo,
                                CondExpr,
                                TrueOperand,
                                FalseOperand));

    gcmFOOTER_ARG("<return>=0x%x", &selection->exprBase);
    return &selection->exprBase;
}

static gceSTATUS
_CheckErrorForAssignmentExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand->dataType);

    /* Check the left operand */
    status = _CheckErrorAsLValueExpr(Compiler, LeftOperand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if ((LeftOperand->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_READONLY) != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot assign to readonly data"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (sloCOMPILER_IsOGLVersion(Compiler) &&
        ( LeftOperand->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER
       || LeftOperand->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot change the value of a shader input or uniform"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if ((RightOperand->dataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY) != 0)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "cannot access to writeonly data"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* Check the operands */
    if (!slsDATA_TYPE_IsAssignableAndComparable(Compiler, LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require any typed expression except arrays, structures"
                                        " containing arrays, sampler types, and structures"
                                        " containing sampler types"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        status = slMakeImplicitConversionForOperandPair(Compiler,
                                                      LeftOperand,
                                                      RightOperand,
                                                      gcvTRUE);
        if (gcmIS_ERROR(status)) {
            return status;
        }
    }
    else
    {
        sloIR_EXPR_SetToBeTheSameDataType(LeftOperand);
        sloIR_EXPR_SetToBeTheSameDataType(RightOperand);
    }

    if (!slsDATA_TYPE_IsEqual(LeftOperand->toBeDataType, RightOperand->toBeDataType))
    {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckErrorForArithmeticAssignmentExpr(
    IN sloCOMPILER Compiler,
    IN gctBOOL IsMul,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS status;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x IsMul=%d LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, IsMul, LeftOperand, RightOperand);

    gcmASSERT(LeftOperand);
    gcmASSERT(LeftOperand->dataType);
    gcmASSERT(RightOperand);
    gcmASSERT(RightOperand->dataType);

    /* Check the left operand */
    status = _CheckErrorAsLValueExpr(Compiler, LeftOperand);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    /* Check the operands */
    if (!slsDATA_TYPE_IsIntOrIVec(LeftOperand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(LeftOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        LeftOperand->base.lineNo,
                                        LeftOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsIntOrIVec(RightOperand->dataType)
        && !slsDATA_TYPE_IsFloatOrVecOrMat(RightOperand->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        RightOperand->base.lineNo,
                                        RightOperand->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require an integer or floating-point typed expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
    if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
    {
        status = slMakeImplicitConversionForOperandPair(Compiler,
                                                        LeftOperand,
                                                        RightOperand,
                                                        gcvTRUE);
        if (gcmIS_ERROR(status))
        {
            return status;
        }
    }
    else
    {
        sloIR_EXPR_SetToBeTheSameDataType(LeftOperand);
        sloIR_EXPR_SetToBeTheSameDataType(RightOperand);
    }

    if (!slsDATA_TYPE_IsEqual(LeftOperand->toBeDataType, RightOperand->toBeDataType))
    {
        if (slsDATA_TYPE_IsInt(LeftOperand->toBeDataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require a scalar integer expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
        else if (slsDATA_TYPE_IsIVec(LeftOperand->toBeDataType))
        {
            if (!slsDATA_TYPE_IsInt(RightOperand->toBeDataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require an int or ivec%d expression",
                                                slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
        }
        else if (slsDATA_TYPE_IsFloat(LeftOperand->toBeDataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            RightOperand->base.lineNo,
                                            RightOperand->base.stringNo,
                                            slvREPORT_ERROR,
                                            "require a scalar float expression"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
        else if (slsDATA_TYPE_IsVec(LeftOperand->toBeDataType))
        {
            if (!IsMul)
            {
                if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(
                                                    Compiler,
                                                    RightOperand->base.lineNo,
                                                    RightOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "require a float or vec%d expression",
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }
            }
            else
            {
                if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                    && !(slsDATA_TYPE_IsMat(RightOperand->toBeDataType)
                        && slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)
                        == slmDATA_TYPE_matrixRowCount_GET(RightOperand->toBeDataType)))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(
                                                    Compiler,
                                                    RightOperand->base.lineNo,
                                                    RightOperand->base.stringNo,
                                                    slvREPORT_ERROR,
                                                    "require a float or vec%d or mat%d expression",
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType),
                                                    slmDATA_TYPE_vectorSize_NOCHECK_GET(LeftOperand->toBeDataType)));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmFOOTER();
                    return status;
                }
            }
        }
        else if (slsDATA_TYPE_IsMat(LeftOperand->toBeDataType))
        {
            if (!slsDATA_TYPE_IsFloat(RightOperand->toBeDataType)
                && !(slsDATA_TYPE_IsMat(RightOperand->toBeDataType)
                    && slmDATA_TYPE_matrixSize_GET(LeftOperand->toBeDataType)
                    == slmDATA_TYPE_matrixSize_GET(RightOperand->toBeDataType)))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                                                Compiler,
                                                RightOperand->base.lineNo,
                                                RightOperand->base.stringNo,
                                                slvREPORT_ERROR,
                                                "require a float or mat%d expression",
                                                slmDATA_TYPE_matrixSize_GET(LeftOperand->toBeDataType)));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
            }
         }
         else {
                gcmASSERT(0);
                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmFOOTER();
                return status;
         }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckLogicalAssignmentExpr(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Operator,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand
)
{
  gceSTATUS status;

  gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, RightOperand);

  gcmASSERT(LeftOperand);
  gcmASSERT(LeftOperand->dataType);
  gcmASSERT(RightOperand);
  gcmASSERT(RightOperand->dataType);

  /* Check the left operand */
  status = _CheckErrorAsLValueExpr(Compiler, LeftOperand);

  if (gcmIS_ERROR(status))
  {
      gcmFOOTER();
      return status;
  }

  if(Operator->u.operator == T_LEFT_ASSIGN ||
     Operator->u.operator == T_RIGHT_ASSIGN)
  {
     status = _CheckBitwiseShiftExpr(Compiler,
                                     LeftOperand,
                                     RightOperand);
     if (gcmIS_ERROR(status))
     {
         gcmFOOTER();
         return status;
     }
  }
  else
  {
     if (!slsDATA_TYPE_IsIntOrIVec(LeftOperand->dataType))
     {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    LeftOperand->base.lineNo,
                    LeftOperand->base.stringNo,
                    slvREPORT_ERROR,
                    "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
     }

     if (!slsDATA_TYPE_IsIntOrIVec(RightOperand->dataType))
     {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                    RightOperand->base.lineNo,
                    RightOperand->base.stringNo,
                    slvREPORT_ERROR,
                    "require an integer expression"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
     }

     status =  _CheckAssignImplicitOperability(Compiler,
                                               RightOperand,
                                               LeftOperand);
     gcmFOOTER();
     return status;
  }

  gcmFOOTER_NO();
  return gcvSTATUS_OK;
}

sloIR_EXPR
slParseAssignmentExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR LeftOperand,
    IN slsLexToken * Operator,
    IN sloIR_EXPR RightOperand
    )
{
    gceSTATUS           status;
    sleBINARY_EXPR_TYPE exprType = (sleBINARY_EXPR_TYPE) 0;
    sloIR_BINARY_EXPR   binaryExpr;

    gcmHEADER_ARG("Compiler=0x%x LeftOperand=0x%x Operator=0x%x RightOperand=0x%x",
                  Compiler, LeftOperand, Operator, RightOperand);

    gcmASSERT(Operator);

    if (LeftOperand == gcvNULL || RightOperand == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switch (Operator->u.operator)
    {
    case T_MOD_ASSIGN:
    case T_LEFT_ASSIGN:
    case T_RIGHT_ASSIGN:
    case T_AND_ASSIGN:
    case T_XOR_ASSIGN:
    case T_OR_ASSIGN:
        if (sloCOMPILER_IsHaltiVersion(Compiler))
        {
            if (Operator->u.operator == T_MOD_ASSIGN)
            {
                 exprType = slvBINARY_MOD_ASSIGN;
                 status = _CheckErrorForArithmeticAssignmentExpr(Compiler,
                                                                 gcvFALSE,
                                                                 LeftOperand,
                                                                 RightOperand);
            }
            else
            {
                 switch(Operator->u.operator)
                 {
                 case T_LEFT_ASSIGN:
                      exprType = slvBINARY_LEFT_ASSIGN;
                      break;
                 case T_RIGHT_ASSIGN:
                      exprType = slvBINARY_RIGHT_ASSIGN;
                      break;
                 case T_AND_ASSIGN:
                      exprType = slvBINARY_AND_ASSIGN;
                      break;
                 case T_XOR_ASSIGN:
                      exprType = slvBINARY_XOR_ASSIGN;
                      break;
                 case T_OR_ASSIGN:
                      exprType = slvBINARY_OR_ASSIGN;
                      break;
                 }

                 status = _CheckLogicalAssignmentExpr(Compiler,
                                                      Operator,
                                                      LeftOperand,
                                                      RightOperand);
            }

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Operator->lineNo,
                                            Operator->stringNo,
                                            slvREPORT_ERROR,
                                            "reserved binary operator '%s'",
                                            _GetBinaryOperatorName(Operator->u.operator)));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case '=':
        exprType = slvBINARY_ASSIGN;

        status = _CheckErrorForAssignmentExpr(Compiler,
                                              LeftOperand,
                                              RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    case T_MUL_ASSIGN:
    case T_DIV_ASSIGN:
    case T_ADD_ASSIGN:
    case T_SUB_ASSIGN:
        switch (Operator->u.operator)
        {
        case T_MUL_ASSIGN:  exprType = slvBINARY_MUL_ASSIGN;    break;
        case T_DIV_ASSIGN:  exprType = slvBINARY_DIV_ASSIGN;    break;
        case T_ADD_ASSIGN:  exprType = slvBINARY_ADD_ASSIGN;    break;
        case T_SUB_ASSIGN:  exprType = slvBINARY_SUB_ASSIGN;    break;
        }

        status = _CheckErrorForArithmeticAssignmentExpr(Compiler,
                                                        (Operator->u.operator == T_MUL_ASSIGN),
                                                        LeftOperand,
                                                        RightOperand);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        break;

    default:
        gcmASSERT(0);
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (sloIR_EXPR_ImplicitConversionDone(RightOperand))
    {
        RightOperand = _MakeImplicitConversionExpr(Compiler, RightOperand);
    }

    /* Create binary expression */
    status = sloIR_BINARY_EXPR_Construct(Compiler,
                                         LeftOperand->base.lineNo,
                                         LeftOperand->base.stringNo,
                                         LeftOperand->base.lineNo,
                                         exprType,
                                         LeftOperand,
                                         RightOperand,
                                         &binaryExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<BINARY_EXPR type=\"%s\" line=\"%d\" string=\"%d\" />",
                                _GetBinaryOperatorName(Operator->u.operator),
                                LeftOperand->base.lineNo,
                                LeftOperand->base.stringNo));

    gcmFOOTER_ARG("<return>=0x%x", &binaryExpr->exprBase);
    return &binaryExpr->exprBase;
}

slsDATA_TYPE *
slParseArrayDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    slsDATA_TYPE *dataType = gcvNULL;
    gctINT       arrayLength;
    gceSTATUS    status;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DataType, ArrayLengthExpr);

    if(!ArrayLengthExpr) {
       arrayLength = -1;
    }
    else {
       status = _EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &arrayLength);

       if (gcmIS_ERROR(status))
       {
           gcmFOOTER_ARG("<return>=0x%x", dataType);
           return dataType;
       }
    }

    status = sloCOMPILER_CreateArrayDataType(Compiler,
                                             DataType,
                                             arrayLength,
                                             &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", dataType);
        return dataType;
    }

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

slsDATA_TYPE *
slParseArrayListDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsDLINK_LIST * LengthList
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE * dataType = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x LengthList=0x%x",
                  Compiler, DataType, LengthList);

    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (LengthList == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable declaration"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    /* Don't check array length here, because it may has implicit size. */
    gcmONERROR(_ParseArraysOfArraysDataType(Compiler, DataType, LengthList, gcvFALSE, &dataType));

OnError:
    gcmFOOTER();
    return dataType;
}

static gceSTATUS
_ParseUpdateLayoutOffset(
    IN     sloCOMPILER   Compiler,
    IN     slsLexToken  *Identifier,
    IN OUT slsDATA_TYPE *DataType
    )
{
    gceSTATUS         status       = gcvSTATUS_OK;
    slsLAYOUT_OFFSET *layoutOffset = gcvNULL;
    gctUINT lineNo   = sloCOMPILER_GetCurrentLineNo(Compiler);
    gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x DataType=0x%x",
                  Compiler, Identifier, DataType);

    if(DataType == gcvNULL)
    {
        gcmFOOTER();
        return status;
    }

    if (!slsDATA_TYPE_IsAtomic(DataType))
    {
        gcmFOOTER();
        return status;
    }

    sloCOMPILER_SearchLayoutOffset(Compiler, DataType->qualifiers.layout.binding, &layoutOffset);

    if (layoutOffset == gcvNULL)
    {
        sloCOMPILER_ConstructLayoutOffset(Compiler, DataType->qualifiers.layout.binding, &layoutOffset);
    }

    if (DataType->qualifiers.layout.id & slvLAYOUT_OFFSET)
    {
        gctBOOL overlaps = gcvFALSE;

        /* The offset of atomic counter must be multiple of 4. */
        if((DataType->qualifiers.layout.offset % 4) != 0)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            lineNo,
                                            stringNo,
                                            slvREPORT_ERROR,
                                            "invalid offset."));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }

        sloCOMPILER_FindLayoutOffsetInBinding(Compiler,
            layoutOffset,
            DataType->qualifiers.layout.offset,
            slsDATA_TYPE_GetSize(DataType),
            Identifier != gcvNULL,
            &overlaps);

        if(overlaps)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            lineNo,
                                            stringNo,
                                            slvREPORT_ERROR,
                                            "overlaps offset."));
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }
    else
    {
        if(Identifier)
        {
            gctBOOL overlaps = gcvTRUE;
            while(overlaps)
            {
                DataType->qualifiers.layout.offset = layoutOffset->cur_offset;
                sloCOMPILER_FindLayoutOffsetInBinding(Compiler,
                    layoutOffset,
                    DataType->qualifiers.layout.offset,
                    slsDATA_TYPE_GetSize(DataType),
                    gcvTRUE,
                    &overlaps);
                layoutOffset->cur_offset += slsDATA_TYPE_GetSize(DataType) * 4;
            }
        }
    }

    DataType->qualifiers.layout.id |= slvLAYOUT_OFFSET;
    DataType->qualifiers.layout.id |= slvLAYOUT_BINDING;

    layoutOffset->cur_offset = DataType->qualifiers.layout.offset;

    if(Identifier)
    {
        layoutOffset->cur_offset += slsDATA_TYPE_GetSize(DataType) * 4;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_IsSimpleStruct(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN gctBOOL IsVariableArray,
    IN slsDATA_TYPE * DataType
    )
{
    slsNAME * field;

    if (DataType->arrayLength > 0 || IsVariableArray)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "shader can't contain an array of structures"
                                        "output '%s'.",
                                        Identifier->u.identifier));
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    FOR_EACH_DLINK_NODE(&(DataType->fieldSpace->names), slsNAME, field)
    {
        if (field->dataType->arrayLength > 0)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "shader can't contain a structure"
                                            "that contain an array for output '%s'.",
                                            Identifier->u.identifier));
            return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }

        if (field->dataType->elementType == slvTYPE_STRUCT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "shader can't contain a structure"
                                            "that contain a structure for output '%s'.",
                                            Identifier->u.identifier));
            return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }
    }

    return gcvSTATUS_OK;
}

static gctBOOL
_ContainDoubleOrIntegerTypedField(
    IN slsDATA_TYPE * DataType
)
{
    slsNAME *fieldName = gcvNULL;
    FOR_EACH_DLINK_NODE(&DataType->fieldSpace->names, slsNAME, fieldName)
    {
#ifndef __clang__
        fieldName = fieldName;
#endif
        if (slmIsElementTypeDouble(fieldName->dataType->elementType) ||
            slmIsElementTypeInteger(fieldName->dataType->elementType))
        {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

static gceSTATUS
_ParseUpdateHaltiQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN gctBOOL IsVariableArray,
    IN OUT slsDATA_TYPE * DataType
)
{
    gceSTATUS    status = gcvSTATUS_OK;
    sleSTORAGE_QUALIFIER storage = DataType->qualifiers.storage;
    sleSHADER_TYPE shaderType;

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x DataType=0x%x",
                  Compiler, Identifier, DataType);

    gcmASSERT(DataType);

    shaderType = Compiler->shaderType;

    if (DataType->qualifiers.layout.id  != slvLAYOUT_NONE &&
        storage== slvSTORAGE_QUALIFIER_UNIFORM &&
        sloCOMPILER_IsES30Version(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "uniform declared outside uniform block can't have layouts"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    /* a vertex shader */
    if (shaderType == slvSHADER_TYPE_VERTEX)
    {
        if (storage == slvSTORAGE_QUALIFIER_IN)
        {
            /* VS input can't be a boolean/opaque/array/struct/double. */
            if (slsDATA_TYPE_IsBool(DataType) ||
                slsDATA_TYPE_IsOpaque(DataType) ||
                (slsDATA_TYPE_IsArray(DataType) && !sloCOMPILER_IsOGLVersion(Compiler))||
                slsDATA_TYPE_IsStruct(DataType) ||
                slmIsElementTypeDouble(DataType->elementType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vs input '%s' has wrong data type.",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }
            storage = slvSTORAGE_QUALIFIER_ATTRIBUTE;
        }
        else if (storage == slvSTORAGE_QUALIFIER_OUT)
        {
            /* VS output can't be a boolean/opaque. */
            if (slsDATA_TYPE_IsBool(DataType) ||
                slsDATA_TYPE_IsOpaque(DataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vs output '%s' has wrong data type.",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }
            /* For es20, glsl1.3 and glsl1.4, vertex output can't be a struct,
            ** for es30/31, vertex output can only be a simple struct.
            */
            if (slsDATA_TYPE_IsStruct(DataType))
            {
                if (sloCOMPILER_IsOGL13Version(Compiler) ||
                    sloCOMPILER_IsOGL14Version(Compiler))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Identifier->lineNo,
                                                    Identifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "vertex varying '%s' cannot be a struct",
                                                    Identifier->u.identifier));
                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmONERROR(status);
                }
                else if (sloCOMPILER_IsHaltiVersion(Compiler))
                {
                    gcmONERROR(_IsSimpleStruct(Compiler,
                                               Identifier,
                                               IsVariableArray,
                                               DataType));
                }
                else
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Identifier->lineNo,
                                                    Identifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "vertex varying '%s' cannot be a struct",
                                                    Identifier->u.identifier));
                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmONERROR(status);
                }
            }
            storage = slvSTORAGE_QUALIFIER_VARYING_OUT;
        }
    }
    else
    {
        if (storage == slvSTORAGE_QUALIFIER_IN)
        {
            /* For es20, input can't be a struct,
            ** for es30/31, input can only be a simple struct.
            ** for ogl, double or integer input has to be flat. And for GLSL1.3 and 1.4, input can't be a struct.
            */
            if (slsDATA_TYPE_IsStruct(DataType))
            {
                if (sloCOMPILER_IsOGLVersion(Compiler))
                {
                    if (sloCOMPILER_IsOGL40Version(Compiler) ||
                        sloCOMPILER_IsOGL33Version(Compiler) ||
                        sloCOMPILER_IsOGL15Version(Compiler))
                    {
                        if (shaderType == slvSHADER_TYPE_FRAGMENT &&
                            DataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
                            _ContainDoubleOrIntegerTypedField(DataType))
                        {
                            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                            Identifier->lineNo,
                                                            Identifier->stringNo,
                                                            slvREPORT_ERROR,
                                                            "double-precision floating-point or integer typed input '%s' has to have flat interpolation qualifier",
                                                            Identifier->u.identifier));
                            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                            gcmONERROR(status);
                        }
                    }
                    else
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Identifier->lineNo,
                                                        Identifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "input '%s' cannot be a struct",
                                                        Identifier->u.identifier));
                        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                        gcmONERROR(status);
                    }
                }
                else if (sloCOMPILER_IsHaltiVersion(Compiler))
                {
                    if (shaderType == slvSHADER_TYPE_FRAGMENT ||
                        slsQUALIFIERS_HAS_FLAG(&DataType->qualifiers, slvQUALIFIERS_FLAG_PATCH))
                    {
                        gcmONERROR(_IsSimpleStruct(Compiler,
                                                   Identifier,
                                                   IsVariableArray,
                                                   DataType));
                    }
                }
                else
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Identifier->lineNo,
                                                    Identifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "input '%s' cannot be a struct",
                                                    Identifier->u.identifier));
                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmONERROR(status);
                }
            }

            /* integer input must have "flat" qualifier. */
            if (slmDATA_TYPE_IsIntegerType(DataType) &&
                DataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
                (shaderType == slvSHADER_TYPE_FRAGMENT || shaderType == slvSHADER_TYPE_COMPUTE))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "integer typed input '%s' has to have flat interpolation qualifier",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }

            /* double input must have "flat" qualifier. */
            if (slmIsElementTypeDouble(DataType->elementType) &&
                DataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
                shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "double-precision floating-point typed input '%s' has to have flat interpolation qualifier",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }

            /* FS input can't be a boolean/opaque. */
            if ((slsDATA_TYPE_IsBool(DataType) ||
                 slsDATA_TYPE_IsOpaque(DataType)) &&
                shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "fragment input '%s' has wrong data type.",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }

            storage = slvSTORAGE_QUALIFIER_VARYING_IN;
        }
        else if (storage == slvSTORAGE_QUALIFIER_OUT)
        {
            /* PS ouput can't be a struct.
            ** TS/GS output can only be a simple struct.
            */
            if (slsDATA_TYPE_IsStruct(DataType))
            {
                if (shaderType == slvSHADER_TYPE_FRAGMENT)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Identifier->lineNo,
                                                    Identifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "fragment output '%s' cannot be structures",
                                                    Identifier->u.identifier));

                    status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                    gcmONERROR(status);
                }
                else if (slsQUALIFIERS_HAS_FLAG(&DataType->qualifiers, slvQUALIFIERS_FLAG_PATCH))
                {
                    gcmONERROR(_IsSimpleStruct(Compiler,
                                               Identifier,
                                               IsVariableArray,
                                               DataType));
                }
            }
            /* FS output can't be a boolean/opaque/matrix/double. */
            if ((slsDATA_TYPE_IsBool(DataType) ||
                 slsDATA_TYPE_IsOpaque(DataType) ||
                 slsDATA_TYPE_IsMat(DataType) ||
                 slmIsElementTypeDouble(DataType->elementType)) &&
                shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "fragment output '%s' has wrong data type.",
                                                Identifier->u.identifier));

                status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                gcmONERROR(status);
            }

            if (shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                storage = slvSTORAGE_QUALIFIER_FRAGMENT_OUT;
            }
            else
            {
                storage = slvSTORAGE_QUALIFIER_VARYING_OUT;
            }
        }

        if (storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT &&
            sloCOMPILER_IsES31VersionOrAbove(Compiler))
        {
            slsLAYOUT_QUALIFIER defaultLayout[1];

            gcmONERROR(sloCOMPILER_GetDefaultLayout(Compiler,
                                                    defaultLayout,
                                                    slvSTORAGE_QUALIFIER_OUT));
            slmDATA_TYPE_layoutId_SET(DataType, defaultLayout->id);
       }
    }

    DataType->qualifiers.storage = storage;

OnError:
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_CheckDataTypePrecision(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS    status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);

    if(sloCOMPILER_IsOGLVersion(Compiler))
    {
        /* do not check precision qualifers here */
        gcmFOOTER();
        return status;
    }

    if(slmIsElementTypeFloatingOrDouble(DataType->elementType) &&
       DataType->qualifiers.precision == slvPRECISION_QUALIFIER_DEFAULT)
    {
       sleSHADER_TYPE shaderType;

        shaderType = Compiler->shaderType;
        if(shaderType == slvSHADER_TYPE_FRAGMENT)
        {
           if (sloCOMPILER_IsHaltiVersion(Compiler))
           {
               if(gcmGetOptimizerOption()->fragmentFPPrecision != gcSHADER_PRECISION_DEFAULT)
               {
                   gctSTRING precisionStr;

                   switch (gcmGetOptimizerOption()->fragmentFPPrecision)
                   {
                   case gcSHADER_PRECISION_HIGH:
                       DataType->qualifiers.precision = slvPRECISION_QUALIFIER_HIGH;
                       precisionStr = "highp";
                       break;

                   case gcSHADER_PRECISION_MEDIUM:
                       DataType->qualifiers.precision = slvPRECISION_QUALIFIER_MEDIUM;
                       precisionStr = "mediump";
                       break;

                   case gcSHADER_PRECISION_LOW:
                       DataType->qualifiers.precision = slvPRECISION_QUALIFIER_LOW;
                       precisionStr = "lowp";
                       break;

                   default:
                       gcmASSERT(0);
                       gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                       Identifier->lineNo,
                                                       Identifier->stringNo,
                                                       slvREPORT_ERROR,
                                                       "missing precision for floating point type for variable: '%s'",
                                                       Identifier->u.identifier));
                       gcmFOOTER_NO();
                       return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
                   }

                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   Identifier->lineNo,
                                                   Identifier->stringNo,
                                                   slvREPORT_WARN,
                                                   "missing precision for floating point type for variable: '%s' -\n"
                                                   " set to %s as given by VC_OPTION -FRAGMENT_FP_PRECISION",
                                                   Identifier->u.identifier, precisionStr));
               }
               else
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   Identifier->lineNo,
                                                   Identifier->stringNo,
                                                   slvREPORT_ERROR,
                                                   "missing precision for floating point type for variable: '%s'",
                                                   Identifier->u.identifier));

                   status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
               }
           }
           else
           {
               gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                               Identifier->lineNo,
                                               Identifier->stringNo,
                                               slvREPORT_WARN,
                                               "missing precision for floating point type for variable: '%s'",
                                               Identifier->u.identifier));

               DataType->qualifiers.precision = slvPRECISION_QUALIFIER_HIGH;
           }
       }
    }
    else if ((slsDATA_TYPE_IsSampler(DataType) || slsDATA_TYPE_IsImage(DataType)) &&
             DataType->qualifiers.precision == slvPRECISION_QUALIFIER_DEFAULT)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "missing precision for sampler type for variable: '%s'",
                                        Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_CheckQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Qualifiers
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    sleSHADER_TYPE shaderType;
    gctUINT lineNo = sloCOMPILER_GetCurrentLineNo(Compiler);
    gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);

    gcmHEADER_ARG("Compiler=0x%x Qualifiers=0x%x",
                  Compiler, Qualifiers);

    gcmASSERT(Qualifiers);

    shaderType = Compiler->shaderType;

    if(Qualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_SHARED &&
       shaderType != slvSHADER_TYPE_COMPUTE)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Qualifiers->lineNo,
                                        Qualifiers->stringNo,
                                        slvREPORT_ERROR,
                                        "\'shared\' storage qualifer allowed for compute shader only."));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if(slsQUALIFIERS_GET_ORDERS(&Qualifiers->u.qualifiers))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        lineNo,
                                        stringNo,
                                        slvREPORT_ERROR,
                                        "qualifier order is not met"));

        gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if (slsQUALIFIERS_HAS_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT))
    {
        if (Qualifiers->u.qualifiers.storage != slvSTORAGE_QUALIFIER_OUT &&
            Qualifiers->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_OUT &&
            (!sloCOMPILER_IsES20Version(Compiler) || Qualifiers->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_IN)
           )
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifiers->lineNo,
                                            Qualifiers->stringNo,
                                            slvREPORT_ERROR,
                                            "Only variables output from "
                                            "a shader and es20 fragment shader input "
                                            "can be candidates for invariance"));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }
    }

    if (Qualifiers->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_GS_PRIMITIVE ||
        Qualifiers->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_MAX_VERTICES)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Qualifiers->lineNo,
                                        Qualifiers->stringNo,
                                        slvREPORT_ERROR,
                                        "The primitive type and vertex count identifiers are allowed only on the "
                                        "interface qualifier out, not on an output block, block member, or "
                                        "variable declaration."));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_CheckImageFormat(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT lineNo = sloCOMPILER_GetCurrentLineNo(Compiler);
    gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x",
                  Compiler, DataType);

    if (sloCOMPILER_IsES31VersionOrAbove(Compiler) && slsDATA_TYPE_IsImage(DataType))
    {
        sleIMAGE_FORMAT format = DataType->qualifiers.layout.imageFormat;
        if (format == slvLAYOUT_IMAGE_FORMAT_DEFAULT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            lineNo,
                                            stringNo,
                                            slvREPORT_ERROR,
                                            "Any image variable must specify a format layout qualifier"));

            gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
            return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }

        if (format != slvLAYOUT_IMAGE_FORMAT_R32F &&
            format != slvLAYOUT_IMAGE_FORMAT_R32I &&
            format != slvLAYOUT_IMAGE_FORMAT_R32UI)
        {
            if ((DataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_READONLY) == 0 &&
                (DataType->qualifiers.memoryAccess & slvMEMORY_ACCESS_QUALIFIER_WRITEONLY) == 0)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                slvREPORT_ERROR,
                                                "Except for image variables qualified with the format "
                                                "qualifiers r32f, r32i, and r32ui, image variables must "
                                                "specify either memory qualifier readonly or the memory "
                                                "qualifier writeonly."));

                gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
                return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }
        }

        if (slsDATA_TYPE_IsFloatImage(DataType))
        {
            if (format != slvLAYOUT_IMAGE_FORMAT_RGBA32F &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA16F &&
                format != slvLAYOUT_IMAGE_FORMAT_R32F &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA8 &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA8_SNORM)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                slvREPORT_ERROR,
                                                "Image type mismatch image format."));

                gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
                return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }
        }

        if (slsDATA_TYPE_IsIntImage(DataType))
        {
            if (format != slvLAYOUT_IMAGE_FORMAT_RGBA32I &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA16I &&
                format != slvLAYOUT_IMAGE_FORMAT_R32I &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA8I)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                slvREPORT_ERROR,
                                                "Image type mismatch image format."));

                gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
                return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }
        }

        if (slsDATA_TYPE_IsUintImage(DataType))
        {
            if (format != slvLAYOUT_IMAGE_FORMAT_RGBA32UI &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA16UI &&
                format != slvLAYOUT_IMAGE_FORMAT_R32UI &&
                format != slvLAYOUT_IMAGE_FORMAT_RGBA8UI)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                lineNo,
                                                stringNo,
                                                slvREPORT_ERROR,
                                                "Image type mismatch image format."));

                gcmFOOTER_ARG("<return>=0x%x", gcvSTATUS_COMPILER_FE_PARSER_ERROR);
                return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            }
        }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_CreateArrayLengthExpr(
    IN sloCOMPILER Compiler,
    IN gctINT ArrayLength,
    IN sloIR_EXPR * ArrayLengthExpr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE * dataType;
    sloIR_CONSTANT constant;
    sluCONSTANT_VALUE value;

    /* Create the data type */
    gcmONERROR(sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_INT,
                                        gcvNULL,
                                        &dataType));

    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

    /* Create the constant */
    gcmONERROR(sloIR_CONSTANT_Construct(
                                    Compiler,
                                    sloCOMPILER_GetCurrentLineNo(Compiler),
                                    sloCOMPILER_GetCurrentStringNo(Compiler),
                                    dataType,
                                    &constant));

    /* Add the constant value */
    value.intValue = ArrayLength;

    gcmONERROR(sloIR_CONSTANT_AddValues(
                                    Compiler,
                                    constant,
                                    1,
                                    &value));

    *ArrayLengthExpr = (sloIR_EXPR)(&constant->exprBase.base);

OnError:
    return status;
}

static gceSTATUS
_ParseVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS    status;
    slsNAME *variableName;
    slsDATA_TYPE * dataType;
    sloEXTENSION  extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);

    gcmASSERT(DataType);
    gcmASSERT(Identifier);

    if(DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "require the initializer for the 'const' variable: '%s'",
                                        Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    /* only opaque type, uniform and storage block can take binding qualifier*/
    if(DataType->qualifiers.layout.id & slvLAYOUT_BINDING &&
       !(slsDATA_TYPE_IsOpaque(DataType) ||
         slsDATA_TYPE_IsUnderlyingInterfaceBlock(DataType)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "binding qualifier should not be used by '%s'",
                                        Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    status = slsDATA_TYPE_Clone(Compiler,
                                DataType->qualifiers.storage,
                                DataType->qualifiers.precision,
                                DataType,
                                &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvVARIABLE_NAME,
                                    dataType,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &variableName);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if(DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_SHARED)
    {
        status = sloCOMPILER_AddSharedVariable(Compiler,
                                               variableName);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    status = _ParseUpdateLayoutOffset(Compiler, Identifier, dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    variableName->u.variableInfo.isLocal = slNameIsLocal(Compiler, variableName);

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                                  Identifier->lineNo,
                                  Identifier->stringNo,
                                  Identifier->u.identifier));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseArrayVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS  status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);

    gcmASSERT(Identifier);

    if (DataType == gcvNULL) {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE)
    {
        sleSHADER_TYPE shaderType;

        shaderType = Compiler->shaderType;

        if(shaderType == slvSHADER_TYPE_VERTEX &&
           !sloCOMPILER_IsOGL40Version(Compiler) &&
           !sloCOMPILER_IsOGL33Version(Compiler) &&
           !sloCOMPILER_IsOGL15Version(Compiler))
        {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           Identifier->lineNo,
                                           Identifier->stringNo,
                                           slvREPORT_ERROR,
                                           "vertex shader input '%s' cannot be arrays",
                                            Identifier->u.identifier));

           status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
           gcmFOOTER();
           return status;
        }
    }

    status = _ParseVariableDecl(Compiler,
                                DataType,
                                Identifier);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer,
    IN gctBOOL TreatConstArrayAsUniform,
    OUT sloIR_EXPR * InitExpr
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    slsNAME *           name;
    sloIR_VARIABLE      variable;
    sloIR_BINARY_EXPR   binaryExpr;
    sloEXTENSION        extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x Initializer=0x%x InitExpr=0x%x",
                  Compiler, DataType, Identifier, Initializer, InitExpr);

    gcmASSERT(DataType);
    gcmASSERT(Identifier);
    gcmASSERT(InitExpr);

    if (Initializer == gcvNULL)
    {
       gcmFOOTER();
       return gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    /* Create the name */
    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvVARIABLE_NAME,
                                    DataType,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &name);

    if (gcmIS_ERROR(status)) {
       gcmFOOTER();
       return status;
    }

    if(DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_SHARED) {
        status = sloCOMPILER_AddSharedVariable(Compiler,
                                               name);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    name->u.variableInfo.treatConstArrayAsUniform = TreatConstArrayAsUniform;
    if (!name->u.variableInfo.treatConstArrayAsUniform)
    {
        name->u.variableInfo.isLocal = slNameIsLocal(Compiler, name);
    }

    if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST)
    {
        status = _CheckErrorAsConstantExpr(Compiler, Initializer, &name->u.variableInfo.constant);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        name->u.variableInfo.constant->variable = name;
        *InitExpr = Initializer;
    }
    else
    {
        /* Create the variable */
        status = sloIR_VARIABLE_Construct(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        name,
                                        &variable);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        status = _CheckErrorForAssignmentExpr(
                                            Compiler,
                                            &variable->exprBase,
                                            Initializer);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }

        if (sloIR_EXPR_ImplicitConversionDone(Initializer))
        {
            Initializer = _MakeImplicitConversionExpr(Compiler, Initializer);
        }

        if (name->u.variableInfo.declareUniformWithInit || name->u.variableInfo.treatConstArrayAsUniform)
        {
            /* The initializer expr must be const. */
            gcmASSERT(Initializer->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST);

            /* uniform initializer */
            name->u.variableInfo.initializer = Initializer;
            InitExpr = gcvNULL;
        }
        else
        {
            /* Create the assigement expression */
            status = sloIR_BINARY_EXPR_Construct(
                                                Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                Identifier->lineNo,
                                                slvBINARY_ASSIGN,
                                                &variable->exprBase,
                                                Initializer,
                                                &binaryExpr);

            if (gcmIS_ERROR(status))
            {
                gcmFOOTER();
                return status;
            }

            *InitExpr = &binaryExpr->exprBase;
        }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<VARIABLE_DECL_WITH_INITIALIZER line=\"%d\" string=\"%d\""
                                " dataType=\"0x%x\" identifier=\"%s\" initializer=\"0x%x\" />",
                                Identifier->lineNo,
                                Identifier->stringNo,
                                DataType,
                                Identifier->u.identifier,
                                Initializer));

    gcmFOOTER_ARG("*InitExpr=0x%x", *InitExpr);
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseArrayVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer,
    OUT sloIR_EXPR * InitExpr
    )
{
    gceSTATUS  status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x "
                  "Initializer=0x%x InitExpr=0x%x",
                  Compiler, DataType, Identifier, Initializer, InitExpr);

    gcmASSERT(Identifier);

    if (DataType == gcvNULL) {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE)
    {
        sleSHADER_TYPE shaderType;

        shaderType = Compiler->shaderType;

        if(shaderType == slvSHADER_TYPE_VERTEX &&
           !sloCOMPILER_IsOGL40Version(Compiler) &&
           !sloCOMPILER_IsOGL33Version(Compiler) &&
           !sloCOMPILER_IsOGL15Version(Compiler))
        {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           Identifier->lineNo,
                                           Identifier->stringNo,
                                           slvREPORT_ERROR,
                                           "vertex shader input '%s' cannot be arrays",
                                            Identifier->u.identifier));

           status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
           gcmFOOTER();
           return status;
       }
    }

    status = _ParseVariableDeclWithInitializer(Compiler,
                                               DataType,
                                               Identifier,
                                               Initializer,
                                               gcvFALSE,
                                               InitExpr);

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER();
       return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseArrayLengthDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN sloIR_EXPR ArrayLengthExpr,
    IN sloIR_EXPR Initializer,
    IN gctINT ArrayLength,
    IN gctBOOL CheckArrayLength,
    OUT slsDATA_TYPE **ArrayDataType
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    gctINT     arrayLength;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x ArrayLengthExpr=0x%x ArrayDataType=0x%x",
                  Compiler, DataType, ArrayLengthExpr, ArrayDataType);


    gcmASSERT(DataType);
    gcmASSERT(ArrayDataType);

    *ArrayDataType = gcvNULL;
    if (ArrayLengthExpr)
    {
       status = _EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           CheckArrayLength,
                                           &arrayLength);

       if (gcmIS_ERROR(status))
       {
          gcmFOOTER();
          return status;
       }
    }
    else
    {
        if (Initializer != gcvNULL && Initializer->dataType != gcvNULL)
        {
            arrayLength = Initializer->dataType->arrayLength;
        }
        else
        {
            arrayLength = ArrayLength;
        }
    }
    status = sloCOMPILER_CreateArrayDataType(Compiler,
                                             DataType,
                                             arrayLength,
                                             ArrayDataType);
    if (gcmIS_ERROR(status))
    {
       gcmFOOTER();
       return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ParseArrayLengthVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    slsDATA_TYPE *  arrayDataType;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DataType, Identifier, ArrayLengthExpr);


    gcmASSERT(DataType);
    gcmASSERT(Identifier);

    status = _ParseArrayLengthDataType(Compiler,
                                       DataType,
                                       ArrayLengthExpr,
                                       gcvNULL,
                                       -1,
                                       gcvTRUE,
                                       &arrayDataType);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    status = _ParseVariableDecl(Compiler,
                                arrayDataType,
                                Identifier);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER();
        return status;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                                  Identifier->lineNo,
                                  Identifier->stringNo,
                                  Identifier->u.identifier));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

slsDeclOrDeclList
slParseNonArrayVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);


    gcmASSERT(Identifier);

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (declOrDeclList.dataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, gcvNULL));

    status = _CheckDataTypePrecision(Compiler,
                                     DataType,
                                     Identifier);
    if(gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CheckImageFormat(Compiler, DataType);

    if(gcmIS_ERROR(status))
    {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, DataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        status = _ParseUpdateHaltiQualifiers(Compiler,
                                             Identifier,
                                             gcvFALSE,
                                             declOrDeclList.dataType);
        if(gcmIS_ERROR(status))
        {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }

        if (slsDATA_TYPE_IsArray(declOrDeclList.dataType))
        {
            if (!slsDATA_TYPE_IsArraysOfArrays(declOrDeclList.dataType))
            {
                status = _ParseArrayVariableDecl(Compiler,
                                                declOrDeclList.dataType,
                                                Identifier);
                gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                return declOrDeclList;
            }
            else
            {
                status = _CheckErrorForArraysOfArrays(Compiler, Identifier, DataType);
                if (gcmIS_ERROR(status))
                {
                   gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                   return declOrDeclList;
                }
            }
        }
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        slsLAYOUT_QUALIFIER layout[1];

        switch (DataType->qualifiers.storage)
        {
            case slvSTORAGE_QUALIFIER_UNIFORM:
            case slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER:
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                             layout,
                                             slvSTORAGE_QUALIFIER_UNIFORM);

                sloCOMPILER_MergeInterFaceLayoutId(Compiler,
                                          layout,
                                          slsDATA_TYPE_IsAtomic(DataType),
                                          DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER,
                                          &DataType->qualifiers.layout);
                break;
            }
            case slvSTORAGE_QUALIFIER_BUFFER:
            case slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER:
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                             layout,
                                             slvSTORAGE_QUALIFIER_BUFFER);

                sloCOMPILER_MergeInterFaceLayoutId(Compiler,
                                          layout,
                                          slsDATA_TYPE_IsAtomic(DataType),
                                          DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER,
                                          &DataType->qualifiers.layout);
                break;
            }
            default:
                break;
        }
    }

    status = _ParseVariableDecl(Compiler,
                                DataType,
                                Identifier);

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

static gceSTATUS
_CheckErrorForArray(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Identifier,
    IN slsDATA_TYPE *DataType,
    IN sloIR_EXPR ArrayLengthExpr
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x DataType=0x%x",
                  Compiler, Identifier, DataType);

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
       if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN)
       {
           sleSHADER_TYPE shaderType;

           shaderType = Compiler->shaderType;

           if(shaderType == slvSHADER_TYPE_VERTEX &&
              !sloCOMPILER_IsOGL40Version(Compiler) &&
              !sloCOMPILER_IsOGL33Version(Compiler) &&
              !sloCOMPILER_IsOGL15Version(Compiler))
           {
              gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                              Identifier->lineNo,
                                              Identifier->stringNo,
                                              slvREPORT_ERROR,
                                              "vertex shader input '%s' cannot be arrays",
                                              Identifier->u.identifier));

              status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
              gcmFOOTER();
              return status;
           }
       }
       if(slsDATA_TYPE_IsArray(DataType) && !slmIsLanguageVersion3_1(Compiler))
       {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           Identifier->lineNo,
                                           Identifier->stringNo,
                                           slvREPORT_ERROR,
                                           "cannot declare array of array: '%s'",
                                           Identifier->u.identifier));

           status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
           gcmFOOTER();
           return status;
       }
    }
    else
    {
       if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST ||
           DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE)
       {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           Identifier->lineNo,
                                           Identifier->stringNo,
                                           slvREPORT_ERROR,
                                           "cannot declare the array: '%s' with the '%s' qualifier",
                                           Identifier->u.identifier,
                                           slGetStorageQualifierName(Compiler, DataType->qualifiers.storage)));

           status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
           gcmFOOTER();
           return status;
       }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_UpdateDataTypeForArraysOfArraysInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType,
    IN slsDATA_TYPE *RelDataType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x RelDataType=0x%x",
                  Compiler, DataType, RelDataType);

    if (DataType->arrayLengthCount != RelDataType->arrayLengthCount)
    {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }

    for (i = 0; i < DataType->arrayLengthCount; i++)
    {
        if ((DataType->arrayLengthList[i] != -1 && DataType->arrayLengthList[i] != RelDataType->arrayLengthList[i]) ||
            (RelDataType->arrayLengthList[i] == -1))
        {
            status = _ReportErrorForDismatchedType(Compiler);
            gcmFOOTER();
            return status;
        }

        if (DataType->arrayLengthList[i] == -1)
        {
            DataType->arrayLengthList[i] = RelDataType->arrayLengthList[i];
        }
    }

    DataType->arrayLength = DataType->arrayLengthList[0];

    if (!slsDATA_TYPE_IsInitializableTo(DataType, RelDataType))
    {
        status = _ReportErrorForDismatchedType(Compiler);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

slsDeclOrDeclList
slParseArrayVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;
    slsDATA_TYPE *      arrayDataType;
    sleSHADER_TYPE      shaderType;
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DataType, Identifier, ArrayLengthExpr);

    gcmASSERT(Identifier);

    shaderType = Compiler->shaderType;

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (declOrDeclList.dataType == gcvNULL || ArrayLengthExpr == gcvNULL)
    {
        if (!ArrayLengthExpr)
        {
            if (!sloCOMPILER_IsOGLVersion(Compiler) &&
                !(declOrDeclList.dataType != gcvNULL &&
                (((shaderType == slvSHADER_TYPE_TCS || shaderType == slvSHADER_TYPE_TES) &&
                (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN ||
                  DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_OUT))
                ||
                ((shaderType == slvSHADER_TYPE_GS) &&
                (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN))
                )))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Identifier->lineNo,
                                                Identifier->stringNo,
                                                slvREPORT_ERROR,
                                                "unspecified array size in variable declaration"));
                gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                return declOrDeclList;
            }
        }
        else
        {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, gcvNULL));

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, DataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CheckImageFormat(Compiler, DataType);

    if (gcmIS_ERROR(status))
    {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
    }

    status = _CheckDataTypePrecision(Compiler,
                                     DataType,
                                     Identifier);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CheckErrorForArray(Compiler,
                                 Identifier,
                                 DataType,
                                 ArrayLengthExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        status = _ParseUpdateHaltiQualifiers(Compiler,
                                             Identifier,
                                             gcvTRUE,
                                             DataType);
        if (gcmIS_ERROR(status))
        {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }
    }

    /* Check an arrays of array. */
    if (slsDATA_TYPE_IsArray(DataType))
    {
        gctINT arrayLength = -1;

        /* Only GLSL31 can support arrays of arrays. */
        if (!slmIsLanguageVersion3_1(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            " This GLSL version can't support arrays of arrays."));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        gcmONERROR(_CheckDataTypePrecision(Compiler,
                                         DataType,
                                         Identifier));

        /* Vertex/Fragment input and output cann't be an array of arrays. */
        gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                     Identifier,
                                     DataType));

        if (ArrayLengthExpr)
        {
            gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &arrayLength));
        }

        gcmONERROR(sloCOMPILER_InsertArrayForDataType(Compiler, DataType, arrayLength, &arrayDataType));
    }
    else
    {
        slsLAYOUT_QUALIFIER layout;
        gctINT arrayLength = -1;

        /* Check a unsized array. */
        if ((shaderType == slvSHADER_TYPE_TCS ||
             shaderType == slvSHADER_TYPE_TES) &&
            ArrayLengthExpr == gcvNULL)
        {
            sloCOMPILER_GetDefaultLayout(Compiler,
                                         &layout,
                                         slvSTORAGE_QUALIFIER_OUT);
            if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_OUT)
            {
                if (shaderType == slvSHADER_TYPE_TES)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Identifier->lineNo,
                                                    Identifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "\"%s\" can't be unsized array.",
                                                    Identifier->u.identifier));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return declOrDeclList;
                }
            }
            else if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_IN)
            {
                arrayLength = layout.maxVerticesNumber;
            }
        }

        status = _ParseArrayLengthDataType(Compiler,
                                           DataType,
                                           ArrayLengthExpr,
                                           gcvNULL,
                                           arrayLength,
                                           gcvTRUE,
                                           &arrayDataType);
        if (gcmIS_ERROR(status))
        {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }
    }

    if (arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM &&
        arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctSIZE_T length;

        length = (gctSIZE_T)slsDATA_TYPE_GetLogicalOperandCount(arrayDataType, gcvFALSE);

        status = sloCOMPILER_SetUniformLocationInUse(Compiler,
                                                     arrayDataType->qualifiers.layout.location,
                                                     length);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of uniforms beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    if (arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE &&
        arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctINT i, length = 1;
        for (i = 0; i < arrayDataType->arrayLengthCount; i++)
        {
            length *= arrayDataType->arrayLengthList[i];
        }

        status = sloCOMPILER_SetInputLocationInUse(Compiler,
                                                   arrayDataType->qualifiers.layout.location,
                                                   length);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of attribute beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    if (arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT &&
        arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctINT i, length = 1;
        for (i = 0; i < arrayDataType->arrayLengthCount; i++)
        {
            length *= arrayDataType->arrayLengthList[i];
        }

        status = sloCOMPILER_SetOutputLocationInUse(Compiler,
                                                    arrayDataType->qualifiers.layout.location,
                                                    length);
        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of fragment shader outputs beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    status = _ParseVariableDecl(Compiler,
                                arrayDataType,
                                Identifier);

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseArrayVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr,
    IN sloIR_EXPR Initializer
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList = {0};
    slsDATA_TYPE *  arrayDataType = gcvNULL;
    sloIR_EXPR initExpr           = gcvNULL;
    sloIR_CONSTANT tempConst      = gcvNULL;
    sluCONSTANT_VALUE   *value    = gcvNULL;
    slsDATA_TYPE *      dataType  = gcvNULL;
    gctPOINTER pointer            = gcvNULL;
    gctBOOL treatConstArrayAsUniform = gcvFALSE;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x "
                  "ArrayLengthExpr=0x%x Initializer=0x%x",
                  Compiler, DataType, Identifier, ArrayLengthExpr, Initializer);

    gcmASSERT(Identifier);

    if(Initializer == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    /* Check if we need to treat a constant array as a uniform. */
    if (Compiler->context.optimizationOptions & slvOPTIMIZATION_TREAT_CONST_ARRAY_AS_UNIFORM)
    {
        if (ArrayLengthExpr->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST &&
            Initializer->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST     &&
            ((sloIR_CONSTANT)Initializer)->valueCount >= 32                             &&
            DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST                  &&
            DataType->elementType != slvTYPE_STRUCT)
        {
            DataType->qualifiers.storage = slvSTORAGE_QUALIFIER_UNIFORM;
            treatConstArrayAsUniform = gcvTRUE;
        }
    }

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (declOrDeclList.dataType == gcvNULL) {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, Initializer));

    /* on es30, an array type can also be formed without specifying a size if the definition includes an initializer. */
    if (sloCOMPILER_IsHaltiVersion(Compiler) && ArrayLengthExpr == gcvNULL)
    {
        status = sloCOMPILER_Allocate(Compiler,
                                      (gctSIZE_T)sizeof(sluCONSTANT_VALUE),
                                      &pointer);
        value = pointer;

        if(gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        status = sloCOMPILER_CreateDataType(
                                            Compiler,
                                            T_INT,
                                            gcvNULL,
                                            &dataType);
        dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;

        if(gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        value->intValue = Initializer->dataType->arrayLength;
        status = sloIR_CONSTANT_Construct(Compiler, Identifier->lineNo, Identifier->stringNo, dataType, &tempConst);

        if(gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        status = sloIR_CONSTANT_SetValues(Compiler,
                                          tempConst,
                                          1,
                                          value);

        if(gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        ArrayLengthExpr = &tempConst->exprBase;
    }

    status = _CheckDataTypePrecision(Compiler,
                                     DataType,
                                     Identifier);
    if(gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "Array initializer not allowed"));
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CheckErrorForArray(Compiler,
                                 Identifier,
                                 declOrDeclList.dataType,
                                 ArrayLengthExpr);

    if(gcmIS_ERROR(status)) {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    status = _ParseUpdateHaltiQualifiers(Compiler,
                                         Identifier,
                                         gcvTRUE,
                                         declOrDeclList.dataType);
    if(gcmIS_ERROR(status)) {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArray(DataType))
    {
        gctINT arrayLength = -1;

        /* Only GLSL31 can support arrays of arrays. */
        if (!slmIsLanguageVersion3_1(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            " This GLSL version can't support arrays of arrays."));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        gcmONERROR(_CheckDataTypePrecision(Compiler,
                                         DataType,
                                         Identifier));

        /* Vertex/Fragment input and vertex output cann't be an array of arrays. */
        gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                     Identifier,
                                     DataType));

        if (ArrayLengthExpr)
        {
            gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &arrayLength));
        }

        gcmONERROR(sloCOMPILER_InsertArrayForDataType(Compiler, DataType, arrayLength, &arrayDataType));

        gcmONERROR(_UpdateDataTypeForArraysOfArraysInitializer(Compiler, arrayDataType, Initializer->dataType));
    }
    else
    {
        status = _ParseArrayLengthDataType(Compiler,
                                           DataType,
                                           ArrayLengthExpr,
                                           Initializer,
                                           -1,
                                           gcvTRUE,
                                           &arrayDataType);
        if (gcmIS_ERROR(status)) {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }
    }

    status = _ParseVariableDeclWithInitializer(Compiler,
                                               arrayDataType,
                                               Identifier,
                                               Initializer,
                                               treatConstArrayAsUniform,
                                               &initExpr);

    if (gcmIS_ERROR(status)) {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    declOrDeclList.initStatement = &initExpr->base;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseArrayVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr,
    IN sloIR_EXPR Initializer
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;
    slsDATA_TYPE *savedDataType = gcvNULL;
    slsDATA_TYPE *arrayDataType = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DeclOrDeclList, Identifier, ArrayLengthExpr);

    gcmASSERT(Identifier);

    if (!sloCOMPILER_IsHaltiVersion(Compiler)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "Array initializer not allowed"));
        gcmFOOTER_ARG("<return>=0x%x", DeclOrDeclList);
        return DeclOrDeclList;
    }
    declOrDeclList = DeclOrDeclList;

    if (declOrDeclList.dataType == gcvNULL) {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, declOrDeclList.dataType, Initializer));

    status = _CheckErrorForArray(Compiler,
                                 Identifier,
                                 declOrDeclList.dataType,
                                 ArrayLengthExpr);

    if(gcmIS_ERROR(status)) {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArray(declOrDeclList.dataType))
    {
        gctINT arrayLength = -1;
        slsDATA_TYPE * arrayDataType = gcvNULL;
        sloIR_EXPR initExpr = gcvNULL;
        sloIR_BASE initStatement = gcvNULL;

        /* Only GLSL31 can support arrays of arrays. */
        if (!slmIsLanguageVersion3_1(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            " This GLSL version can't support arrays of arrays."));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        if (ArrayLengthExpr)
        {
            gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &arrayLength));
        }

        gcmONERROR(sloCOMPILER_InsertArrayForDataType(Compiler, declOrDeclList.dataType, arrayLength, &arrayDataType));

        gcmONERROR(_UpdateDataTypeForArraysOfArraysInitializer(Compiler, arrayDataType, Initializer->dataType));

        gcmONERROR(_ParseVariableDeclWithInitializer(Compiler,
                                                     arrayDataType,
                                                     Identifier,
                                                     Initializer,
                                                     gcvFALSE,
                                                     &initExpr));

        if (declOrDeclList.initStatement != gcvNULL)
        {
            gcmASSERT(declOrDeclList.initStatements == gcvNULL);

            status = sloIR_SET_Construct(Compiler,
                                         declOrDeclList.initStatement->lineNo,
                                         declOrDeclList.initStatement->stringNo,
                                         slvDECL_SET,
                                         &declOrDeclList.initStatements);

            if (gcmIS_ERROR(status)) {
                gcmASSERT(declOrDeclList.initStatements == gcvNULL);
                gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                return declOrDeclList;
            }

            gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                             declOrDeclList.initStatements,
                                             declOrDeclList.initStatement));

            declOrDeclList.initStatement = gcvNULL;
        }

        initStatement = &initExpr->base;

        if (declOrDeclList.initStatements != gcvNULL)
        {
            gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                             declOrDeclList.initStatements,
                                             initStatement));
        }
        else
        {
            gcmASSERT(declOrDeclList.initStatement == gcvNULL);

            declOrDeclList.initStatement = initStatement;
        }
    }
    else
    {
        status = _ParseArrayLengthDataType(Compiler,
                                           declOrDeclList.dataType,
                                           ArrayLengthExpr,
                                           Initializer,
                                           -1,
                                           gcvTRUE,
                                           &arrayDataType);
        if (gcmIS_ERROR(status)) {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }

        savedDataType = declOrDeclList.dataType;
        declOrDeclList.dataType = arrayDataType;

        declOrDeclList = slParseVariableDeclWithInitializer2(Compiler,
                                                             declOrDeclList,
                                                             Identifier,
                                                             Initializer);

        declOrDeclList.dataType = savedDataType;
    }

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseNonArrayVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x",
                  Compiler, DeclOrDeclList, Identifier);

    gcmASSERT(Identifier);

    declOrDeclList = DeclOrDeclList;

    if (declOrDeclList.dataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, declOrDeclList.dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        if(slsDATA_TYPE_IsArray(declOrDeclList.dataType))
        {
            if (!slsDATA_TYPE_IsArraysOfArrays(declOrDeclList.dataType))
            {
                status = _ParseArrayVariableDecl(Compiler,
                                                declOrDeclList.dataType,
                                                Identifier);
                if (gcmIS_ERROR(status))
                {
                   gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                   return declOrDeclList;
                }
                gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                return declOrDeclList;
            }
        }
    }

    status = _CheckImageFormat(Compiler, declOrDeclList.dataType);

    if(gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _ParseVariableDecl(Compiler,
                                declOrDeclList.dataType,
                                Identifier);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseArrayVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DeclOrDeclList, Identifier, ArrayLengthExpr);

    gcmASSERT(Identifier);

    declOrDeclList = DeclOrDeclList;

    if (declOrDeclList.dataType == gcvNULL) {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, declOrDeclList.dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CheckErrorForArray(Compiler,
                                 Identifier,
                                 declOrDeclList.dataType,
                                 ArrayLengthExpr);

    if(gcmIS_ERROR(status)) {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
        if(ArrayLengthExpr == gcvNULL && declOrDeclList.dataType->arrayLength < 0) {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }
    }

    status = _CheckImageFormat(Compiler, declOrDeclList.dataType);

    if(gcmIS_ERROR(status)) {
           gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
           return declOrDeclList;
        }

    if (slsDATA_TYPE_IsArray(declOrDeclList.dataType))
    {
        gctINT arrayLength = -1;
        slsDATA_TYPE * arrayDataType = gcvNULL;

        /* Only GLSL31 can support arrays of arrays. */
        if (!slmIsLanguageVersion3_1(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            " This GLSL version can't support arrays of arrays."));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }

        if (ArrayLengthExpr)
        {
            gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                           ArrayLengthExpr,
                                           gcvTRUE,
                                           gcvTRUE,
                                           &arrayLength));
        }

        gcmONERROR(sloCOMPILER_InsertArrayForDataType(Compiler, declOrDeclList.dataType, arrayLength, &arrayDataType));
        gcmONERROR(_ParseVariableDecl(Compiler,
                                    arrayDataType,
                                    Identifier));
    }
    else
    {
        status = _ParseArrayLengthVariableDecl(Compiler,
                                               declOrDeclList.dataType,
                                               Identifier,
                                               ArrayLengthExpr);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }
    }

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

sloIR_EXPR
_ParseVariableDeclWithInitializerAndAssign(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer,
    IN gctBOOL InternalVariable
    )
{
    gceSTATUS           status;
    sloIR_EXPR          initExpr = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x Initializer=0x%x",
                  Compiler, DataType, Identifier, Initializer);

    gcmASSERT(Identifier);

    /* If the array length is less than 0, it may has a implicit size. */
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        if (DataType->arrayLength < 0)
        {
            DataType->arrayLength = Initializer->dataType->arrayLength;
            if (DataType->arrayLengthList[0] == -1)
            {
                DataType->arrayLengthList[0] = DataType->arrayLength;
            }
        }
    }

    if (DataType == gcvNULL || Initializer == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (!InternalVariable)
    {
        status = _CheckDataTypePrecision(Compiler,
                                         DataType,
                                         Identifier);
        if(gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
        status = _ParseUpdateHaltiQualifiers(Compiler,
                                             Identifier,
                                             gcvFALSE,
                                             DataType);
        if(gcmIS_ERROR(status)) {
           gcmFOOTER_ARG("<return>=%s", "<nil>");
           return gcvNULL;
        }
        if(slsDATA_TYPE_IsArray(DataType)) {
           if(DataType->arrayLength < 0) {
               gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                               Identifier->lineNo,
                                               Identifier->stringNo,
                                               slvREPORT_ERROR,
                                               "unspecified array size in variable type declaration"));

               gcmFOOTER_ARG("<return>=%s", "<nil>");
               return gcvNULL;
           }
           status = _ParseArrayVariableDeclWithInitializer(Compiler,
                                                           DataType,
                                                           Identifier,
                                                           Initializer,
                                                           &initExpr);
           if (gcmIS_ERROR(status))
           {
               gcmFOOTER_ARG("<return>=%s", "<nil>");
               return gcvNULL;
           }

           goto done;
        }
    }

    status = _ParseVariableDeclWithInitializer(Compiler,
                                               DataType,
                                               Identifier,
                                               Initializer,
                                               gcvFALSE,
                                               &initExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

done:

    gcmFOOTER_ARG("<return>=%x", initExpr);
    return initExpr;
}

/* This is for fixing a bug about indexed implicit array, such as float[](in0.x, in0.y)[f]. I need
   to construct a temperate identifier to call _ParseVariableDeclWithInitializerAndAssign(). See
   bug 8008 */
gceSTATUS _CreateTempIdentifier(
                                IN sloCOMPILER Compiler,
                                IN OUT sltPOOL_STRING *auxiArraySymbol
                                )
{
    gceSTATUS           status          = gcvSTATUS_OK;
    sltPOOL_STRING      tempSymbol      = gcvNULL;
    gctPOINTER          pointer         = gcvNULL;

    gctUINT             offset          = 0;
    gctUINT64           curTime         = 0;

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    status = gcoOS_Allocate(
        gcvNULL,
        (gctSIZE_T)256,
        &pointer);
    if (gcmIS_ERROR(status)) {    gcmFOOTER(); return status; }

    tempSymbol = pointer;

    gcoOS_GetTime(&curTime);
    gcoOS_PrintStrSafe(tempSymbol, 256, &offset, "%llu_scalarArray", curTime);

    status = sloCOMPILER_AllocatePoolString(
        Compiler,
        tempSymbol,
        auxiArraySymbol);
    gcmOS_SAFE_FREE(gcvNULL, tempSymbol);

    gcmFOOTER();
    return status;
}

sloIR_EXPR
_ParseFuncCallExprAsExpr(
                         IN sloCOMPILER Compiler,
                         IN sloIR_EXPR Expr
                         )
{
    slsLexToken         identifier;
    gceSTATUS           status;
    sloIR_EXPR          initExpr   = gcvNULL;
    slsDATA_TYPE        *dataType  = gcvNULL;
    slsNAME_SPACE       *nameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x",
        Compiler, Expr);

    status = sloCOMPILER_CreateNameSpace(
                                   Compiler,
                                   gcvNULL,
                                   slvNAME_SPACE_TYPE_FUNCTION,
                                   &nameSpace);

    if(gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_CloneDataType(Compiler,
                                slvSTORAGE_QUALIFIER_NONE,
                                Expr->dataType->qualifiers.precision,
                                Expr->dataType,
                                &dataType);
    if(gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    identifier.lineNo = Expr->base.lineNo;
    identifier.stringNo = Expr->base.stringNo;
    identifier.type = slvVARIABLE_NAME;

    status = _CreateTempIdentifier(Compiler, &(identifier.u.identifier));
    if(gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    initExpr = _ParseVariableDeclWithInitializerAndAssign(Compiler, dataType, &identifier, Expr, gcvTRUE);


    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    gcmFOOTER_ARG("<return>=%s", initExpr);
    return initExpr;
}

slsDeclOrDeclList
slParseVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
    )
{
    slsDeclOrDeclList   declOrDeclList;
    sloIR_EXPR          initExpr;
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE * dataType = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x Initializer=0x%x",
                  Compiler, DataType, Identifier, Initializer);

    gcmASSERT(Identifier);

    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;
    declOrDeclList.dataType         = DataType;

    if (declOrDeclList.dataType == gcvNULL || Initializer == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, Initializer));

    gcmONERROR(slsDATA_TYPE_Clone(Compiler, DataType->qualifiers.storage, DataType->qualifiers.precision, DataType, &dataType));

    /* If the array length is less than 0, it may has a implicit size. */
    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        if (DataType->arrayLength < 0 && DataType->arrayLengthCount == 1)
        {
            dataType->arrayLength = Initializer->dataType->arrayLength;
            if (dataType->arrayLengthList[0] == -1)
            {
                dataType->arrayLengthList[0] = dataType->arrayLength;
            }
        }
    }

    if (dataType->arrayLengthCount > 1)
    {
        gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                     Identifier,
                                     dataType));

        gcmONERROR(_ParseUpdateHaltiQualifiers(Compiler,
                                             Identifier,
                                             gcvFALSE,
                                             dataType));

        gcmONERROR(_UpdateDataTypeForArraysOfArraysInitializer(Compiler, dataType, Initializer->dataType));

        gcmONERROR(_ParseVariableDeclWithInitializer(Compiler,
                                                     dataType,
                                                     Identifier,
                                                     Initializer,
                                                     gcvFALSE,
                                                     &initExpr));
    }
    else
    {
        initExpr = _ParseVariableDeclWithInitializerAndAssign(Compiler, dataType, Identifier, Initializer, gcvFALSE);

        if ( gcvNULL == initExpr)
        {
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }
    }

    declOrDeclList.initStatement = &initExpr->base;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
    )
{
    gceSTATUS           status;
    slsDeclOrDeclList   declOrDeclList;
    sloIR_EXPR          initExpr      = gcvNULL;
    sloIR_BASE          initStatement = gcvNULL;
    slsDATA_TYPE *      varDataType = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x Initializer=0x%x",
                  Compiler, DeclOrDeclList, Identifier, Initializer);

    gcmASSERT(Identifier);

    declOrDeclList = DeclOrDeclList;

    if (declOrDeclList.dataType == gcvNULL || Initializer == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = _CommonCheckForVariableDecl(Compiler, declOrDeclList.dataType, Initializer);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    status = slsDATA_TYPE_Clone(Compiler,
                                declOrDeclList.dataType->qualifiers.storage,
                                declOrDeclList.dataType->qualifiers.precision,
                                declOrDeclList.dataType,
                                &varDataType
                                );
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (declOrDeclList.initStatement != gcvNULL)
    {
        gcmASSERT(declOrDeclList.initStatements == gcvNULL);

        status = sloIR_SET_Construct(Compiler,
                                     declOrDeclList.initStatement->lineNo,
                                     declOrDeclList.initStatement->stringNo,
                                     slvDECL_SET,
                                     &declOrDeclList.initStatements);

        if (gcmIS_ERROR(status)) {
            gcmASSERT(declOrDeclList.initStatements == gcvNULL);
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                         declOrDeclList.initStatements,
                                         declOrDeclList.initStatement));

        declOrDeclList.initStatement = gcvNULL;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
        if (varDataType->arrayLength < 0)
        {
            varDataType->arrayLength = Initializer->dataType->arrayLength;

            if (varDataType->arrayLengthList[0] == -1)
            {
                varDataType->arrayLengthList[0] = varDataType->arrayLength;
            }
        }

        if (slsDATA_TYPE_IsArray(varDataType))
        {
            if (varDataType->arrayLengthCount > 1)
            {
                status = _UpdateDataTypeForArraysOfArraysInitializer(Compiler, varDataType, Initializer->dataType);

                if (gcmIS_ERROR(status))
                {
                   gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                   return declOrDeclList;
                }

                status = _ParseVariableDeclWithInitializer(Compiler,
                                                           varDataType,
                                                           Identifier,
                                                           Initializer,
                                                           gcvFALSE,
                                                           &initExpr);

                goto done;
            }
            else
            {
                status = _ParseArrayVariableDeclWithInitializer(Compiler,
                                                                varDataType,
                                                                Identifier,
                                                                Initializer,
                                                                &initExpr);
                if (gcmIS_ERROR(status))
                {
                   gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
                   return declOrDeclList;
                }

                goto done;
            }
        }
    }
    status = _ParseVariableDeclWithInitializer(Compiler,
                                               varDataType,
                                               Identifier,
                                               Initializer,
                                               gcvFALSE,
                                               &initExpr);

    if (gcmIS_ERROR(status))
    {
       gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
       return declOrDeclList;
    }

done:
    initStatement = &initExpr->base;

    if (declOrDeclList.initStatements != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                         declOrDeclList.initStatements,
                                         initStatement));
    }
    else
    {
        gcmASSERT(declOrDeclList.initStatement == gcvNULL);

        declOrDeclList.initStatement = initStatement;
    }

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsNAME *
slParseFuncHeader(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsNAME *       name  = gcvNULL;
    slsNAME *       field = gcvNULL;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);

    gcmASSERT(Identifier);

    if (DataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Function return types do not use storage qualifiers */
    if (DataType->qualifiers.storage != slvSTORAGE_QUALIFIER_NONE)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "'no qualifiers allowed for function return"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /*
    ** For a ES20 shader, the return type can also be a structure
    ** only if the structure does not contain an array.
    */
    if (DataType->fieldSpace != gcvNULL &&
       !sloCOMPILER_IsHaltiVersion(Compiler))
    {
        /* Check if there is an array in struct. */
        FOR_EACH_DLINK_NODE(&(DataType->fieldSpace->names), slsNAME, field)
        {
           if(field->dataType->arrayLength > 0)
           {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   Identifier->lineNo,
                                                   Identifier->stringNo,
                                                   slvREPORT_ERROR,
                                                   "'function' return type can't contain an array"));
                   gcmFOOTER_ARG("<return>=%s", "<nil>");
                   return gcvNULL;
           }
        }
    }

    /* Function return types and structure fields do not use storage qualifiers */
    if (!slsDATA_TYPE_IsVoid(DataType))
    {
        if (DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM ||
            DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE ||
            DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_OUT ||
            DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_IN ||
            DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_CONST)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "Function return types and structure fields do not use storage qualifiers"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvFUNC_NAME,
                                    DataType,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &name);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         Identifier->u.identifier,
                                         slvNAME_SPACE_TYPE_FUNCTION,
                                         &name->u.funcInfo.localSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<FUNCTION line=\"%d\" string=\"%d\" name=\"%s\">",
                                  Identifier->lineNo,
                                  Identifier->stringNo,
                                  Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=0x%x", name);
    return name;
}

sloIR_BASE
slParseFuncDecl(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    )
{
    gceSTATUS   status;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x",
                  Compiler, FuncName);

    if (FuncName == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    slsFUNC_RESET_FLAG(&(FuncName->u.funcInfo), slvFUNC_DEFINED);

    status = sloCOMPILER_CheckNewFuncName(
                                        Compiler,
                                        FuncName,
                                        gcvNULL);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = slsNAME_SPACE_CheckFuncInGlobalNamespace(
                                        Compiler,
                                        FuncName);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</FUNCTION>"));

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

sloIR_BASE
slParseDeclaration(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList
    )
{
    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x",
                  Compiler, DeclOrDeclList);

    if (DeclOrDeclList.initStatement != gcvNULL)
    {
        gcmASSERT(DeclOrDeclList.initStatements == gcvNULL);

        gcmFOOTER_ARG("<return>=0x%x", DeclOrDeclList.initStatement);
        return DeclOrDeclList.initStatement;
    }
    else if (DeclOrDeclList.initStatements != gcvNULL)
    {
        gcmASSERT(DeclOrDeclList.initStatement == gcvNULL);

        gcmFOOTER_ARG("<return>=0x%x", &DeclOrDeclList.initStatements->base);
        return &DeclOrDeclList.initStatements->base;
    }
    else
    {
        if (DeclOrDeclList.dataType != gcvNULL)
        {
            if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, DeclOrDeclList.dataType))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                sloCOMPILER_GetCurrentLineNo(Compiler),
                                                sloCOMPILER_GetCurrentStringNo(Compiler),
                                                slvREPORT_ERROR,
                                                "Empty declaration can't have unspecified array size."));
            }
        }
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }
}

sloIR_BASE
slParseDefaultPrecisionQualifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN slsLexToken * PrecisionQualifier,
    IN slsDATA_TYPE * DataType
    )
{
    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x PrecisionQualifier=0x%x DataType=0x%x",
                  Compiler, StartToken, PrecisionQualifier, DataType);

    gcmASSERT(StartToken);
    gcmASSERT(PrecisionQualifier);

    if(slsQUALIFIERS_KIND_ISNOT(&PrecisionQualifier->u.qualifiers, slvQUALIFIERS_FLAG_PRECISION)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        PrecisionQualifier->lineNo,
                                        PrecisionQualifier->stringNo,
                                        slvREPORT_ERROR,
                                        "type qualifier other than precision qualifier is specified for a default precision declaration"));
    }

    if (DataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switch(DataType->type) {
    case T_INT:
    case T_FLOAT:
    case T_DOUBLE:
    case T_SAMPLER2D:
    case T_SAMPLERCUBE:
    case T_SAMPLERCUBEARRAY:
    case T_SAMPLER3D:
    case T_SAMPLER1DARRAY:
    case T_SAMPLER2DARRAY:
    case T_SAMPLER2DSHADOW:
    case T_SAMPLER1DARRAYSHADOW:
    case T_SAMPLER2DARRAYSHADOW:
    case T_SAMPLERCUBESHADOW:
    case T_SAMPLERCUBEARRAYSHADOW:
    case T_ISAMPLER2D:
    case T_ISAMPLERCUBE:
    case T_ISAMPLERCUBEARRAY:
    case T_ISAMPLER3D:
    case T_ISAMPLER2DARRAY:
    case T_USAMPLER2D:
    case T_USAMPLERCUBE:
    case T_USAMPLERCUBEARRAY:
    case T_USAMPLER3D:
    case T_USAMPLER2DARRAY:
    case T_SAMPLEREXTERNALOES:
    case T_SAMPLER2DMS:
    case T_ISAMPLER2DMS:
    case T_USAMPLER2DMS:
    case T_SAMPLER2DMSARRAY:
    case T_ISAMPLER2DMSARRAY:
    case T_USAMPLER2DMSARRAY:
    case T_SAMPLERBUFFER:
    case T_ISAMPLERBUFFER:
    case T_USAMPLERBUFFER:
    case T_SAMPLER1D:
    case T_ISAMPLER1D:
    case T_USAMPLER1D:
    case T_SAMPLER1DSHADOW:
    case T_SAMPLER2DRECT:
    case T_ISAMPLER2DRECT:
    case T_USAMPLER2DRECT:
    case T_SAMPLER2DRECTSHADOW:
    case T_ISAMPLER1DARRAY:
    case T_USAMPLER1DARRAY:

    case T_IMAGE2D:
    case T_IMAGECUBE:
    case T_IMAGECUBEARRAY:
    case T_IMAGE3D:
    case T_IMAGE2DARRAY:
    case T_IIMAGE2D:
    case T_IIMAGECUBE:
    case T_IIMAGECUBEARRAY:
    case T_IIMAGE3D:
    case T_IIMAGE2DARRAY:
    case T_UIMAGE2D:
    case T_UIMAGECUBE:
    case T_UIMAGECUBEARRAY:
    case T_UIMAGE3D:
    case T_UIMAGE2DARRAY:
    case T_ATOMIC_UINT:

    case T_IMAGEBUFFER:
    case T_IIMAGEBUFFER:
    case T_UIMAGEBUFFER:
        sloCOMPILER_SetDefaultPrecision(Compiler,
                                        DataType->elementType,
                                        PrecisionQualifier->u.qualifiers.precision);
        break;

    default:
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        StartToken->lineNo,
                                        StartToken->stringNo,
                                        slvREPORT_ERROR,
                                        "invalid type field for default precision statement"));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<DEFAULT_PRECISION line=\"%d\" string=\"%d\""
                                " precision=\"%d\" dataType=\"0x%x\" />",
                                StartToken->lineNo,
                                StartToken->stringNo,
                                PrecisionQualifier->u.qualifiers.precision,
                                DataType));

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

void
slParseCompoundStatementBegin(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    status = sloCOMPILER_CreateNameSpace(
                                        Compiler,
                                        gcvNULL,
                                        slvNAME_SPACE_TYPE_STATE_SET,
                                        &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<COMPOUND_STATEMENT>"));
    gcmFOOTER_NO();
}

sloIR_SET
slParseCompoundStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_SET Set
    )
{
    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x Set=0x%x",
                  Compiler, StartToken, Set);

    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    if (Set == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    Set->base.lineNo    = sloCOMPILER_GetCurrentLineNo(Compiler);
    Set->base.stringNo  = sloCOMPILER_GetCurrentStringNo(Compiler);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</COMPOUND_STATEMENT>"));

    gcmFOOTER_ARG("<return>=0x%x", Set);
    return Set;
}

void
slParseSelectStatementBegin(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    status = sloCOMPILER_CreateNameSpace(
                                        Compiler,
                                        gcvNULL,
                                        slvNAME_SPACE_TYPE_SELECT_SET,
                                        &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<SELECT_STATEMENT>"));
    gcmFOOTER_NO();
}

sloIR_SET
slParseSelectStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_SET Set
    )
{
    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x Set=0x%x",
                  Compiler, StartToken, Set);

    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    if (Set == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    Set->base.lineNo    = sloCOMPILER_GetCurrentLineNo(Compiler);
    Set->base.stringNo  = sloCOMPILER_GetCurrentStringNo(Compiler);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</SELECT_STATEMENT>"));

    gcmFOOTER_ARG("<return>=0x%x", Set);
    return Set;
}

void
slParseCompoundStatementNoNewScopeBegin(
    IN sloCOMPILER Compiler
    )
{
    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<COMPOUND_STATEMENT_NO_NEW_SCOPE>"));
    gcmFOOTER_NO();
}

sloIR_SET
slParseCompoundStatementNoNewScopeEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_SET Set
    )
{
    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x Set=0x%x",
                  Compiler, StartToken, Set);

    gcmASSERT(StartToken);

    if (Set == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    Set->base.lineNo    = StartToken->lineNo;
    Set->base.stringNo  = StartToken->stringNo;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</COMPOUND_STATEMENT_NO_NEW_SCOPE>"));

    gcmFOOTER_ARG("<return>=0x%x", Set);
    return Set;
}

void
slParseSwitchBodyBegin(
IN sloCOMPILER Compiler
)
{
   gcmHEADER_ARG("Compiler=0x%x", Compiler);

   (void) sloCOMPILER_PushSwitchScope(Compiler, gcvNULL);

   gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                     slvDUMP_PARSER,
                     "<SWITCH_BODY>"));
    gcmFOOTER_NO();
}

sloIR_BASE
slParseSwitchBodyEnd(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_SET Set
)
{
   gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x Set=0x%x",
                  Compiler, StartToken, Set);

   gcmASSERT(StartToken);

   if (Set == gcvNULL) return gcvNULL;

   Set->base.lineNo    = StartToken->lineNo;
   Set->base.stringNo    = StartToken->stringNo;

   gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                 slvDUMP_PARSER,
                 "</SWITCH_BODY>"));

   gcmFOOTER_ARG("<return>=0x%x", &Set->base);
   return &Set->base;
}

sloIR_SET
slParseStatementList(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE Statement
    )
{
    gceSTATUS status;
    sloIR_SET set;

    gcmHEADER_ARG("Compiler=0x%x Statement=0x%x",
                  Compiler, Statement);

    status = sloIR_SET_Construct(
                                Compiler,
                                0,
                                0,
                                slvSTATEMENT_SET,
                                &set);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (Statement != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_SET_AddMember(
                                        Compiler,
                                        set,
                                        Statement));
    }

    gcmFOOTER_ARG("<return>=0x%x", set);
    return set;
}

sloIR_SET
slParseStatementList2(
    IN sloCOMPILER Compiler,
    IN sloIR_SET Set,
    IN sloIR_BASE Statement
    )
{
    gcmHEADER_ARG("Compiler=0x%x Set=0x%x Statement=0x%x",
                  Compiler, Set, Statement);

    if (Set != gcvNULL && Statement != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_SET_AddMember(
                                        Compiler,
                                        Set,
                                        Statement));
    }

    gcmFOOTER_ARG("<return>=0x%x", Set);
    return Set;
}

sloIR_BASE
slParseAsmAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_VIV_ASM VivAsm
    )
{
    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x",
                  Compiler, VivAsm);

    if (VivAsm == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<EXPRESSION_STATEMENT expr=\"0x%x\" />",
                                VivAsm));

    gcmFOOTER_ARG("<return>=0x%x", &VivAsm->base);
    return &VivAsm->base;
}

sloIR_BASE
slParseExprAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR Expr
    )
{
    gcmHEADER_ARG("Compiler=0x%x Expr=0x%x",
                  Compiler, Expr);

    if (Expr == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<EXPRESSION_STATEMENT expr=\"0x%x\" />",
                                Expr));

    gcmFOOTER_ARG("<return>=0x%x", &Expr->base);
    return &Expr->base;
}

sloIR_BASE
slParseQualifierAsStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Qualifier
    )
{
    sleSHADER_TYPE shaderType;

    gcmHEADER_ARG("Compiler=0x%x Qualifier=0x%x",
                  Compiler, Qualifier);

    if (Qualifier == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Qualifier must be a layout qualifier. */
    if (!slsQUALIFIERS_HAS_FLAG(&Qualifier->u.qualifiers, slvQUALIFIERS_FLAG_LAYOUT) ||
        slsQUALIFIERS_GET_AUXILIARY(&Qualifier->u.qualifiers) ||
        Qualifier->u.qualifiers.interpolation)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Qualifier->lineNo,
                                        Qualifier->stringNo,
                                        slvREPORT_ERROR,
                                        "It must be a layout qualifier."));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    shaderType = Compiler->shaderType;

    /* check layout extension, for TS/GS. */
    if (Qualifier->u.qualifiers.layout.ext_id != slvLAYOUT_EXT_NONE)
    {
        sleSHADER_TYPE shaderType;
        sleLAYOUT_ID_EXT layoutIdExt = Qualifier->u.qualifiers.layout.ext_id;

        shaderType = Compiler->shaderType;

        /* Check TS layout. */
        if (layoutIdExt & slvLAYOUT_EXT_VERTICES)
        {
            slsLAYOUT_QUALIFIER layout;

            if (shaderType != slvSHADER_TYPE_TCS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "layout qualifier \"vertices\" is only allowed for tessellation control shaders."));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_OUT &&
                Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_OUT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "layout qualifier \"vertices\" is only allowed for outputs of tessellation control shaders."));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            else if (Qualifier->u.qualifiers.layout.verticesNumber > (gctINT)GetGLMaxTessPatchVertices())
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "The output vertex count must be less than"
                                                " the implementation-dependent maximum patch size"));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            sloCOMPILER_GetDefaultLayout(Compiler,
                                         &layout,
                                         Qualifier->u.qualifiers.storage);
            if (layoutIdExt & slvLAYOUT_EXT_VERTICES &&
                layout.ext_id & slvLAYOUT_EXT_VERTICES &&
                Qualifier->u.qualifiers.layout.verticesNumber != layout.verticesNumber)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vertices number is different from the previous one."));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }

        if ((layoutIdExt & slvLAYOUT_EXT_TS_PRIMITIVE_MODE) ||
            (layoutIdExt & slvlAYOUT_EXT_VERTEX_SPACING) ||
            (layoutIdExt & slvlAYOUT_EXT_ORERING) ||
            (layoutIdExt & slvLAYOUT_EXT_POINT_MODE))
        {
            slsLAYOUT_QUALIFIER layout;

            if (shaderType != slvSHADER_TYPE_TES)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifiers is only allowed for tessellation evaluation shaders."));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_IN &&
                Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_IN)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifiers are only allowed for inputs of tessellation evaluation shaders."));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            sloCOMPILER_GetDefaultLayout(Compiler,
                                         &layout,
                                         Qualifier->u.qualifiers.storage);
            if (layoutIdExt & slvLAYOUT_EXT_TS_PRIMITIVE_MODE &&
                layout.ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE &&
                Qualifier->u.qualifiers.layout.tesPrimitiveMode != layout.tesPrimitiveMode)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "primitive mode is different from the previous one."));
            }
            if (layoutIdExt & slvlAYOUT_EXT_VERTEX_SPACING &&
                layout.ext_id & slvlAYOUT_EXT_VERTEX_SPACING &&
                Qualifier->u.qualifiers.layout.tesVertexSpacing != layout.tesVertexSpacing)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vertex spacing is different from the previous one."));
            }
            if (layoutIdExt & slvlAYOUT_EXT_ORERING &&
                layout.ext_id & slvlAYOUT_EXT_ORERING &&
                Qualifier->u.qualifiers.layout.tesOrdering != layout.tesOrdering)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vertex ordering is different from the previous one."));
            }
        }

        /* Check GS layout. */
        if ((layoutIdExt & slvLAYOUT_EXT_GS_PRIMITIVE) ||
            (layoutIdExt & slvLAYOUT_EXT_INVOCATIONS) ||
            (layoutIdExt & slvLAYOUT_EXT_MAX_VERTICES))
        {
            slsLAYOUT_QUALIFIER layout;

            if (shaderType != slvSHADER_TYPE_GS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifiers is only allowed for geometry shaders."));

            }
            if (layoutIdExt & slvLAYOUT_EXT_GS_POINTS)
            {
                sleSTORAGE_QUALIFIER qualifier = slvSTORAGE_QUALIFIER_NONE;

                if (Qualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_OUT ||
                    Qualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT)
                {
                    qualifier = slvSTORAGE_QUALIFIER_OUT;
                }
                else if (Qualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_VARYING_IN ||
                    Qualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN)
                {
                    qualifier = slvSTORAGE_QUALIFIER_IN;
                }
                else
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Qualifier->lineNo,
                                                    Qualifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "layout \"points\" is only allowed for input/output of geometry shaders."));
                }
                if (qualifier != slvSTORAGE_QUALIFIER_NONE)
                {
                    sloCOMPILER_GetDefaultLayout(Compiler,
                                                 &layout,
                                                 qualifier);
                    if (layout.gsPrimitive != slvGS_PRIMITIVE_NONE &&
                        layout.gsPrimitive != Qualifier->u.qualifiers.layout.gsPrimitive)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Qualifier->lineNo,
                                                        Qualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "all geometry shader layout declarations in a program must declare the same layout."));
                    }
                }
            }
            /* Check input-only layout. */
            if (layoutIdExt & slvLAYOUT_EXT_GS_IN_PRIMITIVE || layoutIdExt & slvLAYOUT_EXT_INVOCATIONS)
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                                &layout,
                                                slvSTORAGE_QUALIFIER_IN);
                if (Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_IN &&
                    Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_IN)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Qualifier->lineNo,
                                                    Qualifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "some layout qualifiers are only allowed for inputs of geometry shaders."));
                }
                if ((layoutIdExt & slvLAYOUT_EXT_GS_IN_PRIMITIVE) &&
                    layout.gsPrimitive != slvGS_PRIMITIVE_NONE &&
                    layout.gsPrimitive != Qualifier->u.qualifiers.layout.gsPrimitive)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Qualifier->lineNo,
                                                    Qualifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "all geometry shader input layout declarations in a program must declare the same layout."));
                }
                if (layoutIdExt & slvLAYOUT_EXT_INVOCATIONS)
                {
                    if (layout.gsInvocationTime != -1 &&
                        layout.gsInvocationTime != Qualifier->u.qualifiers.layout.gsInvocationTime)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Qualifier->lineNo,
                                                        Qualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "all geometry shader input layout declarations in a program must declare the same layout."));
                    }
                    if (Qualifier->u.qualifiers.layout.gsInvocationTime > (gctINT)GetGLMaxGSInvocationCount())
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Qualifier->lineNo,
                                                        Qualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "Shader specifies an invocation count greater than the implementation-dependent maximum."));
                    }
                }
            }
            /* Check output-only layout. */
            if (layoutIdExt & slvLAYOUT_EXT_GS_OUT_PRIMITIVE || layoutIdExt & slvLAYOUT_EXT_MAX_VERTICES)
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                             &layout,
                                             slvSTORAGE_QUALIFIER_OUT);
                if (Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_VARYING_OUT &&
                    Qualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_OUT)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Qualifier->lineNo,
                                                    Qualifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "some layout qualifiers are only allowed for outputs of geometry shaders."));
                }
                if ((layoutIdExt & slvLAYOUT_EXT_GS_OUT_PRIMITIVE) &&
                    layout.gsPrimitive != slvGS_PRIMITIVE_NONE &&
                    layout.gsPrimitive != Qualifier->u.qualifiers.layout.gsPrimitive)
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    Qualifier->lineNo,
                                                    Qualifier->stringNo,
                                                    slvREPORT_ERROR,
                                                    "all geometry shader output layout declarations in a program must declare the same layout."));
                }
                if (layoutIdExt & slvLAYOUT_EXT_MAX_VERTICES)
                {
                    if (layout.maxGSVerticesNumber != -1 &&
                        Qualifier->u.qualifiers.layout.maxGSVerticesNumber != layout.maxGSVerticesNumber)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Qualifier->lineNo,
                                                        Qualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "all geometry shader output vertex count declarations in a program must declare the same count."));
                    }
                    if (Qualifier->u.qualifiers.layout.maxGSVerticesNumber > (gctINT)GetGLMaxGSOutVertices())
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        Qualifier->lineNo,
                                                        Qualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "Shader specifies an max vertices greater than the implementation-dependent maximum."));
                    }
                }
            }
        }
    }

    /* Check normal layout. */
    switch(Qualifier->u.qualifiers.storage)
    {
    case slvSTORAGE_QUALIFIER_OUT:
        if (slmIsLanguageVersion3_1(Compiler) || sloCOMPILER_IsOGLVersion(Compiler))
        {
            if (Qualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "location id is not applicable to output default layout"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.layout.id & slvLAYOUT_BINDING)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "binding is not applicable to output default layout"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.layout.id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "local size layout qualifier id is applicable to inputs only "));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (shaderType != slvSHADER_TYPE_FRAGMENT &&
                Qualifier->u.qualifiers.layout.id & sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "blend support layout qualifier id is applicable to fragment shader only "));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.layout.id &
                (sldLAYOUT_MEMORY_BIT_FIELDS |sldLAYOUT_MATRIX_BIT_FIELDS))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "memory and/or matrix layout qualifier id is not applicable to outputs"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
        else
        {
            gctUINT32 languageVersion;
            gctCHAR *versionPtr;

            languageVersion = sloCOMPILER_GetLanguageVersion(Compiler);
            versionPtr = (gctCHAR *) &languageVersion;
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "output default layout qualifiers are not allowed for language version %d%d%d",
                                            versionPtr[3], versionPtr[2], versionPtr[1]));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvSTORAGE_QUALIFIER_IN:
        if (slmIsLanguageVersion3_1(Compiler) || sloCOMPILER_IsOGLVersion(Compiler))
        {
            if (shaderType != slvSHADER_TYPE_COMPUTE &&
                Qualifier->u.qualifiers.layout.id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "work group size layout qualifier id is applicable to compute shader only "));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (shaderType == slvSHADER_TYPE_COMPUTE &&
                Qualifier->u.qualifiers.layout.id & ~sldLAYOUT_WORK_GROUP_SIZE_FIELDS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifier ids are not applicable to compute shader inputs"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (shaderType == slvSHADER_TYPE_COMPUTE &&
                !sloCOMPILER_DefaultComputeGroupLayoutMatch(Compiler, &Qualifier->u.qualifiers.layout))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "different layout qualifiers are specified for compute shader inputs"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (shaderType == slvSHADER_TYPE_FRAGMENT &&
                Qualifier->u.qualifiers.layout.id != slvLAYOUT_EARLY_FRAGMENT_TESTS)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifier ids are not applicable to fragment shader inputs"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
        else
        {
            gctUINT32 languageVersion;
            gctCHAR *versionPtr;

            languageVersion = sloCOMPILER_GetLanguageVersion(Compiler);
            versionPtr = (gctCHAR *) &languageVersion;
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "input default layout qualifiers are not allowed for language version %d%d%d",
                                            versionPtr[3], versionPtr[2], versionPtr[1]));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    case slvSTORAGE_QUALIFIER_UNIFORM:
        if (Qualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "location id is not applicable to uniform block default layout"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        if (Qualifier->u.qualifiers.layout.id &
            ~(slvLAYOUT_BINDING | sldLAYOUT_MEMORY_BIT_FIELDS | sldLAYOUT_MATRIX_BIT_FIELDS))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "some layout qualifier id are not applicable to uniform block default layout"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        else if (Qualifier->u.qualifiers.layout.id & slvLAYOUT_STD430)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "memory layout qualifier id std430 is not allowed for uniform block"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;

        }
        break;

    case slvSTORAGE_QUALIFIER_BUFFER:
        if (slmIsLanguageVersion3_1(Compiler))
        {
            if (Qualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "location id is not applicable to storage block default layout"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            if (Qualifier->u.qualifiers.layout.id &
                ~(sldLAYOUT_MEMORY_BIT_FIELDS | sldLAYOUT_MATRIX_BIT_FIELDS | sldLAYOUT_WORK_GROUP_SIZE_FIELDS))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Qualifier->lineNo,
                                                Qualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "some layout qualifier id are not applicable to storage block default layout"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
        else
        {
            gctUINT32 languageVersion;
            gctCHAR *versionPtr;

            languageVersion = sloCOMPILER_GetLanguageVersion(Compiler);
            versionPtr = (gctCHAR *) &languageVersion;
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Qualifier->lineNo,
                                            Qualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "buffer default layout qualifiers are not allowed for language version %d%d%d",
                                            versionPtr[3], versionPtr[2], versionPtr[1]));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        break;

    default:
        gcmASSERT(0);
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Qualifier->lineNo,
                                        Qualifier->stringNo,
                                        slvREPORT_ERROR,
                                        "some layout qualifier id is not applicable to storage qualifier default layout"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;

    }

    /* If there are some variables which have already been created, we need to record it. */
    switch (Qualifier->u.qualifiers.storage)
    {
    case slvSTORAGE_QUALIFIER_OUT:
        Compiler->context.applyOutputLayout.bApplyLayout = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_IN:
        Compiler->context.applyInputLayout.bApplyLayout = gcvTRUE;
        break;

    default:
        break;
    }

    sloCOMPILER_SetDefaultLayout(Compiler,
                                    &Qualifier->u.qualifiers.layout,
                                    Qualifier->u.qualifiers.storage);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<QUALIFIER_STATEMENT qualifier=\"0x%x\" />",
                                Qualifier));

    gcmFOOTER_ARG("<return>=0x%x", gcvNULL);
    return gcvNULL;
}

sloIR_BASE
slParseTypeAsStatement(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType
    )
{
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x",
                  Compiler, DataType);

    if (DataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STRUCT_STATEMENT struct=\"0x%x\" />",
                                DataType));

    gcmFOOTER_ARG("<return>=0x%x", gcvNULL);
    return gcvNULL;
}

sloIR_BASE
slParseCompoundStatementAsStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_SET CompoundStatement
    )
{
    gcmHEADER_ARG("Compiler=0x%x CompoundStatement=0x%x",
                  Compiler, CompoundStatement);

    if (CompoundStatement == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STATEMENT compoundStatement=\"0x%x\" />",
                                CompoundStatement));

    gcmFOOTER_ARG("<return>=0x%x", &CompoundStatement->base);
    return &CompoundStatement->base;
}

sloIR_BASE
slParseCompoundStatementNoNewScopeAsStatementNoNewScope(
    IN sloCOMPILER Compiler,
    IN sloIR_SET CompoundStatementNoNewScope
    )
{
    gcmHEADER_ARG("Compiler=0x%x CompoundStatementNoNewScope=0x%x",
                  Compiler, CompoundStatementNoNewScope);

    if (CompoundStatementNoNewScope == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STATEMENT_NO_NEW_SCOPE compoundStatementNoNewScope=\"0x%x\" />",
                                CompoundStatementNoNewScope));

    gcmFOOTER_ARG("<return>=0x%x", &CompoundStatementNoNewScope->base);
    return &CompoundStatementNoNewScope->base;
}

slsSelectionStatementPair
slParseSelectionRestStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE TrueStatement,
    IN sloIR_BASE FalseStatement
    )
{
    slsSelectionStatementPair pair;
    gcmHEADER_ARG("Compiler=0x%x TrueStatement=0x%x FalseStatement=0x%x",
                   Compiler, TrueStatement, FalseStatement);

    pair.trueStatement  = TrueStatement;
    pair.falseStatement = FalseStatement;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<SELECTION_REST_STATEMENT trueStatement=\"0x%x\""
                                " falseStatement=\"0x%x\" />",
                                TrueStatement,
                                FalseStatement));

    gcmFOOTER_ARG("<return>=0x%x", pair);
    return pair;
}

static gceSTATUS
_CheckErrorForCondExpr(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Compiler=0x%x CondExpr=0x%x",
                  Compiler, CondExpr);

    gcmASSERT(CondExpr);
    gcmASSERT(CondExpr->dataType);

    /* Check the operand */
    if (!slsDATA_TYPE_IsBool(CondExpr->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        CondExpr->base.lineNo,
                                        CondExpr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar boolean expression"));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_BASE
slParseSelectionStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR CondExpr,
    IN slsSelectionStatementPair SelectionStatementPair
    )
{
    gceSTATUS       status;
    sloIR_SELECTION selection;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x CondExpr=0x%x SelectionStatementPair=0x%x",
                  Compiler, StartToken, CondExpr, SelectionStatementPair);

    gcmASSERT(StartToken);

    if (CondExpr == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Check error */
    status = _CheckErrorForCondExpr(
                                    Compiler,
                                    CondExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create the selection */
    status = sloIR_SELECTION_Construct(
                                    Compiler,
                                    StartToken->lineNo,
                                    StartToken->stringNo,
                                    gcvNULL,
                                    CondExpr,
                                    SelectionStatementPair.trueStatement,
                                    SelectionStatementPair.falseStatement,
                                    &selection);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<SELECTION_STATEMENT line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                                " trueStatement=\"0x%x\" falseStatement=\"0x%x\" />",
                                StartToken->lineNo,
                                StartToken->stringNo,
                                CondExpr,
                                SelectionStatementPair.trueStatement,
                                SelectionStatementPair.falseStatement));

    gcmFOOTER_ARG("<return>=0x%x", &selection->exprBase.base);
    return &selection->exprBase.base;
}


sloIR_BASE
slParseSwitchStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_EXPR ControlExpr,
IN sloIR_BASE SwitchBody
)
{
    gceSTATUS    status;
    sloIR_SWITCH switchSelect;
    sloIR_LABEL cases = gcvNULL;
    sloIR_SET switchBody;
    sloIR_BASE member;
    sloIR_LABEL *curLoc;
    sloIR_LABEL curCase;
    gctBOOL hasDefault = gcvFALSE;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x ControlExpr=0x%x SwitchBody=0x%x",
                  Compiler, StartToken, ControlExpr, SwitchBody);

    gcmASSERT(StartToken);

    if (ControlExpr == gcvNULL) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Check error */
    if (!slsDATA_TYPE_IsStrictInt(ControlExpr->dataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        ControlExpr->base.lineNo,
                                        ControlExpr->base.stringNo,
                                        slvREPORT_ERROR,
                                        "require a scalar integer expression"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switchBody = (sloIR_SET)SwitchBody;

    /* Create the selection */
    if(SwitchBody) {
        slsSWITCH_SCOPE *switchScope;

        switchScope = sloCOMPILER_GetSwitchScope(Compiler);
        if(switchScope) {
          cases =  switchScope->cases;
        }

        /* check if the data type of init expression matches */
        if (cases && sloCOMPILER_IsOGLVersion(Compiler))
        {
            sloIR_LABEL caseLabel = cases;
            gceSTATUS status = gcvSTATUS_OK;
            sloEXTENSION extension = {0};
            extension.extension1 = slvEXTENSION1_EXT_SHADER_IMPLICIT_CONVERSIONS;
            while(caseLabel)
            {
                if (caseLabel->type == slvDEFAULT)
                {
                    caseLabel = caseLabel->nextCase;
                    continue;
                }
                if(sloCOMPILER_ExtensionEnabled(Compiler, &extension))
                {
                    status = slMakeImplicitConversionForOperand(Compiler,
                                              &(caseLabel->caseValue->exprBase),
                                              ControlExpr->dataType);
                    if (gcmIS_ERROR(status)) {
                        gcmFOOTER_ARG("<return>=%s", "<nil>");
                        return gcvNULL;
                    }
                }
                else
                {
                    sloIR_EXPR_SetToBeTheSameDataType(&caseLabel->caseValue->exprBase);
                }

                if (!slsDATA_TYPE_IsInitializableTo(ControlExpr->dataType, caseLabel->caseValue->exprBase.toBeDataType))
                {
                    status = _ReportErrorForDismatchedType(Compiler);
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return gcvNULL;
                }
                caseLabel = caseLabel->nextCase;
            }
        }

        member = (sloIR_BASE)switchBody->members.next;
        if (member->vptr->type != slvIR_LABEL)
        {
             gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                             member->lineNo,
                             member->stringNo,
                             slvREPORT_ERROR,
                             "No statements are allowed "
                             "in a switch statement before the first case statement."));
             gcmFOOTER_ARG("<return>=%s", "<nil>");
             return gcvNULL;
        }

        member = (sloIR_BASE)switchBody->members.prev;
        if (member->vptr->type == slvIR_LABEL)
        {
             gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                             member->lineNo,
                             member->stringNo,
                             slvREPORT_ERROR,
                             "There must be some statements  "
                             "in a switch statement after the last case/default statement."));
             gcmFOOTER_ARG("<return>=%s", "<nil>");
             return gcvNULL;
        }
    }

    /* After validity check for statement, if there is not a default label, add one. */
    curLoc = &cases;
    curCase = *curLoc;
    while(curCase)
    {
        if(curCase->type == slvDEFAULT)
        {
            hasDefault = gcvTRUE;
            break;
        }
        curLoc = &curCase->nextCase;
        curCase = *curLoc;
        continue;
    }
    if(!hasDefault && SwitchBody)
    {
        sloIR_BASE defaultStatement;

        defaultStatement = slParseDefaultStatement(Compiler,
                                        StartToken);
        switchBody = slParseStatementList2(Compiler,
                                        switchBody,
                                        defaultStatement);
    }

    if (SwitchBody)
        sloCOMPILER_PopSwitchScope(Compiler);

    status = sloIR_SWITCH_Construct(Compiler,
                                    StartToken->lineNo,
                                    StartToken->stringNo,
                                    ControlExpr,
                                    SwitchBody,
                                    cases,
                                    &switchSelect);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<SWITCH_STATEMENT line=\"%d\" string=\"%d\" condExpr=\"0x%x\""
                                  " switchBody=\"0x%x\" cases=\"0x%x\" />",
                                  StartToken->lineNo,
                                  StartToken->stringNo,
                                  ControlExpr,
                                  SwitchBody,
                                  cases));

    gcmFOOTER_ARG("<return>=0x%x", &switchSelect->exprBase.base);
    return &switchSelect->exprBase.base;
}

static void
_slInsertCases(
IN sloCOMPILER Compiler,
IN sloIR_LABEL NewCase,
IN OUT sloIR_LABEL *CaseHead
)
{
   sloIR_LABEL *curLoc;
   sloIR_LABEL curCase;

   gcmHEADER_ARG("Compiler=0x%x NewCase=0x%x CaseHead=0x%x",
                  Compiler, NewCase, CaseHead);

   gcmASSERT(NewCase->type != slvCASE ||
             slsDATA_TYPE_IsInt(NewCase->caseValue->exprBase.dataType));

   gcmASSERT(CaseHead);
   curLoc = CaseHead;
   curCase = *curLoc;

   while(curCase) {
      if(curCase->type == slvDEFAULT) {
        if(NewCase->type == slvDEFAULT) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                  NewCase->base.lineNo,
                            NewCase->base.stringNo,
                            slvREPORT_ERROR,
                            "default case already specified"));
        }
        break;
      }
      if(NewCase->type == slvDEFAULT ||
         NewCase->caseValue->values[0].intValue < curCase->caseValue->values[0].intValue) {
         curLoc = &curCase->nextCase;
     curCase = *curLoc;
         continue;
      }
      if(NewCase->caseValue->values[0].intValue == curCase->caseValue->values[0].intValue) {
         gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                            NewCase->base.lineNo,
                         NewCase->base.stringNo,
                         slvREPORT_ERROR,
                         "case value \"%d\" already used",
                                         NewCase->caseValue));
      }
      break;
   }
   NewCase->nextCase = curCase;
   *curLoc = NewCase;
   gcmFOOTER_ARG("CaseHead=0x%x", *CaseHead);
}

sloIR_BASE
slParseCaseStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * StartToken,
IN sloIR_EXPR CaseExpr
)
{
    gceSTATUS status;
    sloIR_LABEL caseLabel;
    sloIR_CONSTANT caseConstant;
    slsSWITCH_SCOPE *switchScope;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x CaseExpr=0x%x",
                  Compiler, StartToken, CaseExpr);

    status = _CheckErrorAsScalarIntConstantExpr(Compiler,
                                                CaseExpr,
                                                &caseConstant);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmASSERT(StartToken);

    /* Create the case label statement */
    status = sloIR_LABEL_Construct(Compiler,
                                   StartToken->lineNo,
                                   StartToken->stringNo,
                                   &caseLabel);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    caseLabel->type = slvCASE;
    caseLabel->caseValue = caseConstant;
    switchScope = sloCOMPILER_GetSwitchScope(Compiler);
    gcmASSERT(switchScope);
    _slInsertCases(Compiler, caseLabel, &switchScope->cases);

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<CASE_LABEL line=\"%d\" string=\"%d\" caseExpr=\"0x%x\"",
                                  StartToken->lineNo,
                                  StartToken->stringNo,
                                  CaseExpr));

    gcmFOOTER_ARG("<return>=0x%x", &caseLabel->base);
    return &caseLabel->base;
}

sloIR_BASE
slParseDefaultStatement(
IN sloCOMPILER Compiler,
IN slsLexToken * DefaultToken
)
{
    gceSTATUS status;
    sloIR_LABEL defaultLabel;
    slsSWITCH_SCOPE *switchScope;

    gcmHEADER_ARG("Compiler=0x%x DefaultToken=0x%x",
                  Compiler, DefaultToken);

    gcmASSERT(DefaultToken);

    /* Create the label null statement */
    status = sloIR_LABEL_Construct(Compiler,
                                   DefaultToken->lineNo,
                                   DefaultToken->stringNo,
                                   &defaultLabel);
    if (gcmIS_ERROR(status)) return gcvNULL;

    defaultLabel->type = slvDEFAULT;
    switchScope = sloCOMPILER_GetSwitchScope(Compiler);
    if (!switchScope)
    {
        gcmFOOTER_ARG("<return>=0x%x", gcvNULL);
        return gcvNULL;
    }

    gcmASSERT(switchScope);
    _slInsertCases(Compiler, defaultLabel, &switchScope->cases);

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<DEFAULT_LABEL line=\"%d\" string=\"%d\"",
                                  DefaultToken->lineNo,
                                  DefaultToken->stringNo));


    gcmFOOTER_ARG("<return>=0x%x", &defaultLabel->base);
    return &defaultLabel->base;
}

void
slParseWhileStatementBegin(
    IN sloCOMPILER Compiler
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace;

    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         gcvNULL,
                                         slvNAME_SPACE_TYPE_LOOP_SET,
                                         &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<WHILE_STATEMENT>"));
    gcmFOOTER_NO();
}

sloIR_BASE
slParseWhileStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR CondExpr,
    IN sloIR_BASE LoopBody
    )
{
    gceSTATUS           status;
    sloIR_ITERATION     iteration;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x CondExpr=0x%x LoopBody=0x%x",
                  Compiler, StartToken, CondExpr, LoopBody);

    gcmASSERT(StartToken);

    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    /* Check error */
    if (CondExpr == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        StartToken->lineNo,
                                        StartToken->stringNo,
                                        slvREPORT_ERROR,
                                        "while statement has no condition"));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = _CheckErrorForCondExpr(
                                    Compiler,
                                    CondExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create the iteration */
    status = sloIR_ITERATION_Construct(
                                    Compiler,
                                    StartToken->lineNo,
                                    StartToken->stringNo,
                                    slvWHILE,
                                    CondExpr,
                                    LoopBody,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &iteration);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</WHILE_STATEMENT>"));

    gcmFOOTER_ARG("<return>=0x%x", &iteration->base);
    return &iteration->base;
}

sloIR_BASE
slParseDoWhileStatement(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_BASE LoopBody,
    IN sloIR_EXPR CondExpr
    )
{
    gceSTATUS       status;
    sloIR_ITERATION iteration;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x LoopBody=0x%x CondExpr=0x%x",
                  Compiler, StartToken, LoopBody, CondExpr);

    gcmASSERT(StartToken);

    if (CondExpr == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        StartToken->lineNo,
                                        StartToken->stringNo,
                                        slvREPORT_ERROR,
                                        "do-while statement has no condition"));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = _CheckErrorForCondExpr(
                                    Compiler,
                                    CondExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloIR_ITERATION_Construct(
                                    Compiler,
                                    StartToken->lineNo,
                                    StartToken->stringNo,
                                    slvDO_WHILE,
                                    CondExpr,
                                    LoopBody,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &iteration);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<DO_WHILE_STATEMENT line=\"%d\" string=\"%d\""
                                " condExpr=\"0x%x\" LoopBody=\"0x%x\" />",
                                StartToken->lineNo,
                                StartToken->stringNo,
                                CondExpr,
                                LoopBody));

    gcmFOOTER_ARG("<return>=0x%x", &iteration->base);
    return &iteration->base;
}

void
slParseForStatementBegin(
    IN sloCOMPILER Compiler)
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace;

    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         gcvNULL,
                                         slvNAME_SPACE_TYPE_LOOP_SET,
                                         &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FOR_STATEMENT>"));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<FOR_STATEMENT>"));

    gcmFOOTER_NO();
}

sloIR_BASE
slParseForStatementEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN sloIR_BASE ForInitStatement,
    IN slsForExprPair ForExprPair,
    IN sloIR_BASE LoopBody
    )
{
    gceSTATUS       status;
    sloIR_ITERATION iteration;
    slsNAME_SPACE * forSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x ForInitStatement=0x%x ForExprPair=0x%x LoopBody=0x%x",
                  Compiler, StartToken, ForInitStatement, ForExprPair, LoopBody);

    gcmASSERT(StartToken);

    sloCOMPILER_PopCurrentNameSpace(Compiler, &forSpace);

    if (ForExprPair.condExpr != gcvNULL)
    {
        status = _CheckErrorForCondExpr(
                                        Compiler,
                                        ForExprPair.condExpr);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    status = sloIR_ITERATION_Construct(
                                    Compiler,
                                    StartToken->lineNo,
                                    StartToken->stringNo,
                                    slvFOR,
                                    ForExprPair.condExpr,
                                    LoopBody,
                                    forSpace,
                                    ForInitStatement,
                                    ForExprPair.restExpr,
                                    &iteration);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</FOR_STATEMENT>"));

    gcmFOOTER_ARG("<return>=0x%x", &iteration->base);
    return &iteration->base;
}

sloIR_EXPR
slParseCondition(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR Initializer
)
{
    gceSTATUS       status   = gcvSTATUS_OK;
    sloIR_EXPR      initExpr = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x Initializer=0x%x",
                  Compiler, DataType, Identifier, Initializer);

    gcmASSERT(Identifier);

    if (DataType == gcvNULL || Initializer == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status =
        _ParseVariableDeclWithInitializer(Compiler,
                                          DataType,
                                          Identifier,
                                          Initializer,
                                          gcvFALSE,
                                          &initExpr);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<CONDITION line=\"%d\" string=\"%d\""
                                " dataType=\"0x%x\" identifier=\"%s\" initializer=\"0x%x\" />",
                                Identifier->lineNo,
                                Identifier->stringNo,
                                DataType,
                                Identifier->u.identifier,
                                Initializer));

    gcmFOOTER_ARG("<return>=0x%x", initExpr);
    return initExpr;
}

slsForExprPair
slParseForRestStatement(
    IN sloCOMPILER Compiler,
    IN sloIR_EXPR CondExpr,
    IN sloIR_EXPR RestExpr
    )
{
    slsForExprPair pair;

    gcmHEADER_ARG("Compiler=0x%x CondExpr=0x%x RestExpr=0x%x",
                  Compiler, CondExpr, RestExpr);

    pair.condExpr   = CondExpr;
    pair.restExpr   = RestExpr;

    gcmFOOTER_ARG("<return>=0x%x", pair);
    return pair;
}

gceSTATUS
_CheckErrorForJump(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleJUMP_TYPE Type,
    IN sloIR_EXPR ReturnExpr
    )
{
    sleSHADER_TYPE  shaderType;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x LineNo=%u StringNo=%u Type=%d ReturnExpr=0x%x",
                   Compiler, LineNo, StringNo, Type, ReturnExpr);

    if (Type == slvDISCARD)
    {
        shaderType = Compiler->shaderType;

        if (shaderType != slvSHADER_TYPE_FRAGMENT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            LineNo,
                                            StringNo,
                                            slvREPORT_ERROR,
                                            "'discard' is only allowed "
                                            "within the fragment shaders"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmFOOTER();
            return status;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

sloIR_BASE
slParseJumpStatement(
    IN sloCOMPILER Compiler,
    IN sleJUMP_TYPE Type,
    IN slsLexToken * StartToken,
    IN sloIR_EXPR ReturnExpr
    )
{
    gceSTATUS       status;
    sloIR_JUMP      jump;

    gcmHEADER_ARG("Compiler=0x%x Type=%d StartToken=0x%x ReturnExpr=0x%x",
                  Compiler, Type, StartToken, ReturnExpr);

    gcmASSERT(StartToken);

    status = _CheckErrorForJump(
                                Compiler,
                                StartToken->lineNo,
                                StartToken->stringNo,
                                Type,
                                ReturnExpr);

    if(Type == slvRETURN)
    {
        slsNAME_SPACE* nameSpace = sloCOMPILER_GetCurrentFunctionSpace(Compiler);

        if(nameSpace)
        {
            nameSpace->nameSpaceFlags |= sleNAME_SPACE_FLAGS_RETURN_INSERTED;
        }
    }

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloIR_JUMP_Construct(
                                Compiler,
                                StartToken->lineNo,
                                StartToken->stringNo,
                                Type,
                                ReturnExpr,
                                &jump);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<JUMP line=\"%d\" string=\"%d\""
                                " type=\"%s\" returnExpr=\"0x%x\" />",
                                StartToken->lineNo,
                                StartToken->stringNo,
                                slGetIRJumpTypeName(Type),
                                ReturnExpr));

    gcmFOOTER_ARG("<return>=0x%x", &jump->base);
    return &jump->base;
}

void
slParseExternalDecl(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE Decl
    )
{
    gcmHEADER_ARG("Compiler=0x%x Decl=0x%x",
                  Compiler, Decl);

    if (Decl == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_AddExternalDecl(Compiler, Decl));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<EXTERNAL_DECL decl=\"0x%x\" />",
                                Decl));

    gcmFOOTER_NO();
}

void
slParseFuncDef(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    IN sloIR_SET Statements
    )
{
    gceSTATUS   status;
    slsNAME *   firstFuncName;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x Statements=0x%x",
                  Compiler, FuncName, Statements);

    if (FuncName == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    if (Statements == gcvNULL)
    {
        status = sloIR_SET_Construct(
                                    Compiler,
                                    FuncName->lineNo,
                                    FuncName->stringNo,
                                    slvSTATEMENT_SET,
                                    &Statements);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_NO();
            return;
        }
    }

    sloCOMPILER_PopCurrentNameSpace(Compiler, gcvNULL);

    slsFUNC_SET_FLAG(&(FuncName->u.funcInfo), slvFUNC_DEFINED);

    status = sloCOMPILER_CheckNewFuncName(
                                        Compiler,
                                        FuncName,
                                        &firstFuncName);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    if (firstFuncName == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    if (FuncName != firstFuncName)
    {
        status = slsNAME_BindAliasParamNames(Compiler, FuncName, firstFuncName);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_NO();
            return;
        }
    }

    gcmVERIFY_OK(sloNAME_BindFuncBody(Compiler,
                                      firstFuncName,
                                      Statements));
    slsFUNC_SET_FLAG(&(firstFuncName->u.funcInfo), slvFUNC_DEFINED);

    gcmVERIFY_OK(sloCOMPILER_AddExternalDecl(Compiler, &Statements->base));

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</FUNCTION>"));
    gcmFOOTER_NO();
}

slsNAME *
slParseParameterList(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    IN slsNAME * ParamName
    )
{
    gctUINT paramCount = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL hasVoidParameter;

    gcmHEADER_ARG("Compiler=0x%x FuncName=0x%x ParamName=0x%x",
                  Compiler, FuncName, ParamName);

    if (FuncName == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", FuncName);
        return FuncName;
    }

    hasVoidParameter = slsFUNC_HAS_FLAG(&(FuncName->u.funcInfo), slvFUNC_HAS_VOID_PARAM);

    /* Add a "void" parameter. */
    if (ParamName == gcvNULL)
    {
        sloNAME_GetParamCount(Compiler, FuncName, &paramCount);

        /* A function can only has one void parameter. */
        if (hasVoidParameter || paramCount != 0)
        {
            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        }

        slsFUNC_SET_FLAG(&(FuncName->u.funcInfo), slvFUNC_HAS_VOID_PARAM);
    }
    else if (hasVoidParameter)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
    }

    if (status != gcvSTATUS_OK)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        FuncName->lineNo,
                                        FuncName->stringNo,
                                        slvREPORT_ERROR,
                                        "function \"%s\" has a void parameter and other parameter.",
                                        FuncName->symbol));
    }

    gcmFOOTER_ARG("<return>=0x%x", FuncName);
    return FuncName;
}

slsDeclOrDeclList
slParseTypeDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    )
{
    slsDeclOrDeclList declOrDeclList;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x",
                  Compiler, DataType);

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (DataType &&
        (slsDATA_TYPE_IsSampler(DataType) || slsDATA_TYPE_IsImage(DataType)) &&
        (DataType->qualifiers.storage != slvSTORAGE_QUALIFIER_UNIFORM))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "Sampler/Image can't be declared without uniform qualifier for global declaration"));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return declOrDeclList;
    }

    _ParseUpdateLayoutOffset(Compiler, gcvNULL, DataType);

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseInvariantOrPreciseDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * StartToken,
    IN slsLexToken * Identifier
    )
{
    slsDeclOrDeclList declOrDeclList;
    slsNAME *       Name = gcvNULL;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmHEADER_ARG("Compiler=0x%x StartToken=0x%x Identifier=0x%x",
                  Compiler, StartToken, Identifier);

    gcmASSERT(StartToken);
    gcmASSERT(Identifier);

    if(slsQUALIFIERS_KIND_ISNOT(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT) &&
       slsQUALIFIERS_KIND_ISNOT(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "type qualifier other than variance qualifier and precise qualifier"
                                        " is specified."));
    }

    declOrDeclList.dataType         = gcvNULL;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (slsQUALIFIERS_KIND_IS(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT) &&
        sloCOMPILER_GetGlobalSpace(Compiler) != sloCOMPILER_GetCurrentSpace(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "'invariant '%s' declaration' : only allowed at global scope",
                                        Identifier->u.identifier));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return declOrDeclList;
    }

    status = slsNAME_SPACE_Search(Compiler,
                                  sloCOMPILER_GetGlobalSpace(Compiler),
                                  Identifier->u.identifier,
                                  gcvNULL,
                                  gcvNULL,
                                  gcvFALSE,
                                  gcvFALSE,
                                  &Name);

    if (status != gcvSTATUS_OK)
    {
        status = slsNAME_SPACE_Search(Compiler,
                                      sloCOMPILER_GetBuiltInSpace(Compiler),
                                      Identifier->u.identifier,
                                      gcvNULL,
                                      gcvNULL,
                                      gcvTRUE,
                                      gcvFALSE,
                                      &Name);

        if (status != gcvSTATUS_OK)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "Identifier '%s' not found",
                                            Identifier->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    gcmASSERT(Name);

    if (slsQUALIFIERS_KIND_IS(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT) &&
        sloCOMPILER_IsOGLVersion(Compiler))
    {
        if (Name->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "'invariant '%s' declaration' : cannot be used on uniforms.",
                                            Identifier->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
        if (Name->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_NONE)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "'invariant '%s' declaration' : cannot be used on temps.",
                                            Identifier->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    /* Copy flags. */
    if (slsQUALIFIERS_HAS_FLAG(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT))
    {
        slsQUALIFIERS_SET_FLAG(&Name->dataType->qualifiers, slvQUALIFIERS_FLAG_INVARIANT);
    }
    if (slsQUALIFIERS_HAS_FLAG(&StartToken->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE))
    {
        slsQUALIFIERS_SET_FLAG(&Name->dataType->qualifiers, slvQUALIFIERS_FLAG_PRECISE);
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<INVARIANT_DECL line=\"%d\" string=\"%d\" identifier=\"%s\" />",
                                StartToken->lineNo,
                                StartToken->stringNo,
                                Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsNAME *
slParseNonArrayParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsNAME *       name;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x",
                  Compiler, DataType, Identifier);

    if (DataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler) && slsDATA_TYPE_IsArray(DataType) && Identifier != gcvNULL) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "invalid forming of array type from '%s'",
                                        _GetTypeName(DataType->type)));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }
    if (slsDATA_TYPE_IsVoid(DataType) && Identifier == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if(slsQUALIFIERS_KIND_ISNOT(&DataType->qualifiers, slvQUALIFIERS_FLAG_PRECISION) &&
       slsQUALIFIERS_KIND_ISNOT(&DataType->qualifiers, slvQUALIFIERS_FLAG_NONE))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "type qualifier other than precision qualifier is specified for parameter."));
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    (Identifier != gcvNULL)? Identifier->lineNo : 0,
                                    (Identifier != gcvNULL)? Identifier->stringNo : 0,
                                    slvPARAMETER_NAME,
                                    DataType,
                                    (Identifier != gcvNULL)? Identifier->u.identifier : "",
                                    extension,
                                    gcvTRUE,
                                    &name);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<PARAMETER_DECL dataType=\"0x%x\" name=\"%s\" />",
                                DataType,
                                (Identifier != gcvNULL)? Identifier->u.identifier : ""));

    gcmFOOTER_ARG("<return>=0x%x", name);
    return name;
}

slsNAME *
slParseArrayParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    gceSTATUS      status;
    gctINT         arrayLength;
    slsDATA_TYPE * arrayDataType;
    slsNAME *      name;
    sloEXTENSION   extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DataType, Identifier, ArrayLengthExpr);

    if (DataType == gcvNULL || ArrayLengthExpr == gcvNULL)
    {
        if(ArrayLengthExpr == gcvNULL) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            (Identifier != gcvNULL)? Identifier->lineNo : 0,
                                            (Identifier != gcvNULL)? Identifier->stringNo : 0,
                                            slvREPORT_ERROR,
                                            "unspecified array size in parameter declaration"));
        }

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler) && slsDATA_TYPE_IsArray(DataType) && Identifier != gcvNULL) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "invalid forming of array type from '%s'",
                                        _GetTypeName(DataType->type)));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = _EvaluateExprToArrayLength(Compiler,
                                        ArrayLengthExpr,
                                        gcvTRUE,
                                        gcvTRUE,
                                        &arrayLength);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_CreateArrayDataType(
                                            Compiler,
                                            DataType,
                                            arrayLength,
                                            &arrayDataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    (Identifier != gcvNULL)? Identifier->lineNo : 0,
                                    (Identifier != gcvNULL)? Identifier->stringNo : 0,
                                    slvPARAMETER_NAME,
                                    arrayDataType,
                                    (Identifier != gcvNULL)? Identifier->u.identifier : "",
                                    extension,
                                    gcvTRUE,
                                    &name);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<PARAMETER_DECL dataType=\"0x%x\" name=\"%s\" />",
                                DataType,
                                (Identifier != gcvNULL)? Identifier->u.identifier : ""));

    gcmFOOTER_ARG("<return>=0x%x", name);
    return name;
}

slsLexToken
slParseBasicType(
IN sloCOMPILER Compiler,
IN slsDATA_TYPE *BasicType
)
{
    slsLexToken token = {0};

    gcmHEADER_ARG("Compiler=0x%x BasicType=0x%x",
                  Compiler);

    if (BasicType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return token;
    }

    if(slsQUALIFIERS_KIND_ISNOT(&BasicType->qualifiers, slvQUALIFIERS_FLAG_PRECISION) &&
       slsQUALIFIERS_KIND_ISNOT(&BasicType->qualifiers, slvQUALIFIERS_FLAG_NONE)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        0,
                                        0,
                                        slvREPORT_ERROR,
                                        "type qualifier other than precision qualifier is specified for a function"));
    }
    gcoOS_ZeroMemory(&token, gcmSIZEOF(token));
    token.lineNo        = sloCOMPILER_GetCurrentLineNo(Compiler);
    token.stringNo      = sloCOMPILER_GetCurrentStringNo(Compiler);
    token.type          = T_BASIC_TYPE;
    token.u.basicType   = BasicType;

    gcmFOOTER_ARG("<return>=0x%x", &token);
    return token;
}

slsNAME *
slParseQualifiedParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * ParameterQualifiers,
    IN slsNAME * ParameterDecl
    )
{
    slsLexToken token = {0};

    gcmHEADER_ARG("Compiler=0x%x ParameterQualifier=0x%x ParameterDecl=0x%x",
                  Compiler, ParameterQualifiers, ParameterDecl);

    if (ParameterDecl == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmASSERT(ParameterDecl->dataType);

    if(ParameterQualifiers == gcvNULL)
    {
        ParameterQualifiers = &token;
    }
    else if(ParameterQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_CONST)
    {
        ParameterQualifiers->u.qualifiers.storage = slvSTORAGE_QUALIFIER_CONST_IN;
    }

    ParameterDecl->dataType->qualifiers.storage =  slsQUALIFIERS_HAS_FLAG(&ParameterQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_STORAGE)
                                                ? ParameterQualifiers->u.qualifiers.storage
                                                : slvSTORAGE_QUALIFIER_IN;

    if(ParameterQualifiers->u.qualifiers.precision)
    {
        ParameterDecl->dataType->qualifiers.precision = ParameterQualifiers->u.qualifiers.precision;
    }
    else
    {
        sloCOMPILER_GetDefaultPrecision(Compiler,
                                        ParameterDecl->dataType->elementType,
                                        &ParameterDecl->dataType->qualifiers.precision);
    }

    if(slsQUALIFIERS_HAS_FLAG(&ParameterQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE))
    {
        slsQUALIFIERS_SET_FLAG(&ParameterDecl->dataType->qualifiers, slvQUALIFIERS_FLAG_PRECISE);
    }

    gcmFOOTER_ARG("<return>=0x%x", ParameterDecl);
    return ParameterDecl;
}

slsDATA_TYPE *
slParseFullySpecifiedType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeQualifier,
    IN slsDATA_TYPE * DataType
    )
{
    gceSTATUS status;
    sleSHADER_TYPE shaderType;
    gctBOOL mustAtGlobalNameSpace, atGlobalNameSpace;
    sleCOMPILER_FLAGS flag;
    slsNAME_SPACE *currentNameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x TypeQualifier=0x%x DataType=0x%x",
                  Compiler, TypeQualifier, DataType);

    if(TypeQualifier)
    {
        _CheckQualifiers(Compiler, TypeQualifier);
    }

    if (DataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler) && slsDATA_TYPE_IsArray(DataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "invalid forming of array type from '%s'",
                                        _GetTypeName(DataType->type)));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    currentNameSpace = sloCOMPILER_GetCurrentSpace(Compiler);

    if(sloCOMPILER_IsOGLVersion(Compiler) && TypeQualifier)
    {
        if (slmIsElementTypeBoolean(DataType->elementType) &&
            TypeQualifier->u.qualifiers.precision != slvPRECISION_QUALIFIER_DEFAULT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "Precision qualifiers cannot be applied to boolean variables"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        if (slsDATA_TYPE_IsStruct(DataType) &&
            TypeQualifier->u.qualifiers.precision != slvPRECISION_QUALIFIER_DEFAULT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "Precision qualifiers cannot be applied to structs"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    /* Copy flags. */
    if (TypeQualifier)
    {
        if (slsQUALIFIERS_HAS_FLAG(&TypeQualifier->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT))
        {
            slsQUALIFIERS_SET_FLAG(&DataType->qualifiers, slvQUALIFIERS_FLAG_INVARIANT);
        }
    }

    shaderType = Compiler->shaderType;

    if(TypeQualifier && TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_SHARED &&
       shaderType != slvSHADER_TYPE_COMPUTE)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        TypeQualifier->lineNo,
                                        TypeQualifier->stringNo,
                                        slvREPORT_ERROR,
                                        "\'shared\' storage qualifer allowed for compute shader only."));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler) && TypeQualifier != gcvNULL)
    {
        slsLAYOUT_QUALIFIER layout[1];

        switch (TypeQualifier->u.qualifiers.storage)
        {
            case slvSTORAGE_QUALIFIER_UNIFORM:
            case slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER:
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                             layout,
                                             slvSTORAGE_QUALIFIER_UNIFORM);

                sloCOMPILER_MergeInterFaceLayoutId(Compiler,
                                          layout,
                                          slsDATA_TYPE_IsAtomic(DataType),
                                          TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER,
                                          &TypeQualifier->u.qualifiers.layout);
                break;
            }
            case slvSTORAGE_QUALIFIER_BUFFER:
            case slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER:
            {
                sloCOMPILER_GetDefaultLayout(Compiler,
                                             layout,
                                             slvSTORAGE_QUALIFIER_BUFFER);

                sloCOMPILER_MergeInterFaceLayoutId(Compiler,
                                          layout,
                                          slsDATA_TYPE_IsAtomic(DataType),
                                          TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER,
                                          &TypeQualifier->u.qualifiers.layout);
                break;
            }
            default:
                break;
        }
    }

    if(slsDATA_TYPE_IsAtomic(DataType))
    {
        if(TypeQualifier == gcvNULL ||
          (TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_BINDING) == 0)
        {
            gctUINT lineNo   = sloCOMPILER_GetCurrentLineNo(Compiler);
            gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);

            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier == gcvNULL ? lineNo : TypeQualifier->lineNo,
                                            TypeQualifier == gcvNULL ? stringNo : TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "The binding parameter of atomic counter is not optional."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if (TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
        {
            gctUINT lineNo   = sloCOMPILER_GetCurrentLineNo(Compiler);
            gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);

            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier == gcvNULL ? lineNo : TypeQualifier->lineNo,
                                            TypeQualifier == gcvNULL ? stringNo : TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "atomic counter can't have location layout."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if (slsQUALIFIERS_HAS_FLAG(&TypeQualifier->u.qualifiers, slvQUALIFIERS_FLAG_PRECISION) &&
            TypeQualifier->u.qualifiers.precision != slvPRECISION_QUALIFIER_HIGH)
        {
            gctUINT lineNo   = sloCOMPILER_GetCurrentLineNo(Compiler);
            gctUINT stringNo = sloCOMPILER_GetCurrentStringNo(Compiler);

            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier == gcvNULL ? lineNo : TypeQualifier->lineNo,
                                            TypeQualifier == gcvNULL ? stringNo : TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "atomic counter can't have non-highp precision."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    if(TypeQualifier == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", DataType);
        return DataType;
    }

    if (sloCOMPILER_IsHaltiVersion(Compiler))
    {
       sleSHADER_TYPE shaderType;

        shaderType = Compiler->shaderType;

       if(slsQUALIFIERS_HAS_FLAG(&TypeQualifier->u.qualifiers, slvQUALIFIERS_FLAG_LAYOUT))
       {
          switch(TypeQualifier->u.qualifiers.storage)
          {
          case slvSTORAGE_QUALIFIER_UNIFORM:
             if((TypeQualifier->u.qualifiers.layout.id & ~(slvLAYOUT_LOCATION |
                                                           slvLAYOUT_BINDING |
                                                           slvLAYOUT_OFFSET |
                                                           slvLAYOUT_IMAGE_FORMAT)) &&
                /* default block uniform should be in global name space */
                sloCOMPILER_GetCurrentSpace(Compiler) == sloCOMPILER_GetGlobalSpace(Compiler))
             {

                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                TypeQualifier->lineNo,
                                                TypeQualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "layout id's other than location, binding or offset are not applicable "
                                                " to default block uniform"));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
             }

             if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
             {
                if(!sloCOMPILER_IsES31VersionOrAbove(Compiler))
                {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                TypeQualifier->lineNo,
                                                TypeQualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "location id not applicable for uniform variable"));
                   gcmFOOTER_ARG("<return>=%s", "<nil>");
                   return gcvNULL;
                }
                else
                {
                   gctSIZE_T length;

                   length = (gctSIZE_T)slsDATA_TYPE_GetLogicalOperandCount(DataType, gcvFALSE);
                   status = sloCOMPILER_SetUniformLocationInUse(Compiler,
                                                                TypeQualifier->u.qualifiers.layout.location,
                                                                length);
                   if(gcmIS_ERROR(status))
                   {
                      gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                      TypeQualifier->lineNo,
                                                      TypeQualifier->stringNo,
                                                      slvREPORT_ERROR,
                                                      "# of uniforms beyond limit"));
                      gcmFOOTER_ARG("<return>=%s", "<nil>");
                      return gcvNULL;
                   }
                }
             }
             else
             {
                flag = Compiler->context.compilerFlags;;
                slsCOMPILER_SetUnspecifiedUniformLocation(flag);
                Compiler->context.compilerFlags = flag;
             }

             if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_IMAGE_FORMAT)
             {
                slmDATA_TYPE_imageFormat_SET(DataType, TypeQualifier->u.qualifiers.layout.imageFormat);
             }
             break;

          case slvSTORAGE_QUALIFIER_OUT:
             if(shaderType == slvSHADER_TYPE_VERTEX && !sloCOMPILER_IsES31VersionOrAbove(Compiler))
             {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                TypeQualifier->lineNo,
                                                TypeQualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "vertex shader output cannot have layout qualifiers"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
             }
             else
             { /* fragment shader assumed */
                if(TypeQualifier->u.qualifiers.layout.id)
                {
                    if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
                    {
                       status = sloCOMPILER_SetOutputLocationInUse(Compiler,
                                                             TypeQualifier->u.qualifiers.layout.location,
                                                             DataType->arrayLength > 0 ? DataType->arrayLength : 1);
                       if(gcmIS_ERROR(status))
                       {
                          gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                          TypeQualifier->lineNo,
                                                          TypeQualifier->stringNo,
                                                          slvREPORT_ERROR,
                                                          "# of fragment shader outputs beyond limit"));
                          gcmFOOTER_ARG("<return>=%s", "<nil>");
                          return gcvNULL;
                       }
                    }
                    else
                    {
                       gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                       TypeQualifier->lineNo,
                                                       TypeQualifier->stringNo,
                                                       slvREPORT_ERROR,
                                                       "layout id's other than 'location' are not applicable "
                                                       " to storage qualifier 'in' or 'out'"));
                       gcmFOOTER_ARG("<return>=%s", "<nil>");
                       return gcvNULL;
                   }
                }
                else
                {
                    /* no location specified */
                    status = sloCOMPILER_SetUnspecifiedOutputLocationExist(Compiler);

                    if (gcmIS_ERROR(status) && sloCOMPILER_NeedCheckOutputLocationExist(Compiler))
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        TypeQualifier->lineNo,
                                                        TypeQualifier->stringNo,
                                                        slvREPORT_ERROR,
                                                        "fragment shader's output location has to be explicitly specified"));
                        gcmFOOTER_ARG("<return>=%s", "<nil>");
                        return gcvNULL;
                    }
                }
             }
             break;

          case slvSTORAGE_QUALIFIER_IN:
             if(shaderType == slvSHADER_TYPE_FRAGMENT && !sloCOMPILER_IsES31VersionOrAbove(Compiler))
             {
                 gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                 TypeQualifier->lineNo,
                                                 TypeQualifier->stringNo,
                                                 slvREPORT_ERROR,
                                                 "fragment shader input cannot have layout qualifiers"));
                 gcmFOOTER_ARG("<return>=%s", "<nil>");
                 return gcvNULL;
             }
             else
             { /* vertex shader assumed */
                 if(TypeQualifier->u.qualifiers.layout.id)
                 {
                    if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
                    {
                       status = sloCOMPILER_SetInputLocationInUse(Compiler,
                                                             TypeQualifier->u.qualifiers.layout.location,
                                                             1);
                       if(gcmIS_ERROR(status))
                       {
                          gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                          TypeQualifier->lineNo,
                                                          TypeQualifier->stringNo,
                                                          slvREPORT_ERROR,
                                                          "# of vertex shader inputs beyond limit"));
                          gcmFOOTER_ARG("<return>=%s", "<nil>");
                          return gcvNULL;
                       }
                    }
                    else
                    {
                       gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                 TypeQualifier->lineNo,
                                                 TypeQualifier->stringNo,
                                                 slvREPORT_ERROR,
                                                 "layout id's other than 'location' are not applicable "
                                                 " to storage qualifier 'in' or 'out'"));
                       gcmFOOTER_ARG("<return>=%s", "<nil>");
                       return gcvNULL;
                    }
                  }
              }
              break;
           }

           /* set DataType with layout qualifiers */
           if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
           {
               slmDATA_TYPE_layoutLocation_SET(DataType, TypeQualifier->u.qualifiers.layout.location);
           }

           if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_OFFSET)
           {
               slmDATA_TYPE_layoutOffset_SET(DataType, TypeQualifier->u.qualifiers.layout.offset);
           }

           if(TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_BINDING)
           {
               slmDATA_TYPE_layoutBinding_SET(DataType, TypeQualifier->u.qualifiers.layout.binding);
           }
           slmDATA_TYPE_layoutId_SET(DataType, TypeQualifier->u.qualifiers.layout.id);
       }
       else
       { /* no layout(especially, no location) specified */
           if(TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM &&
              sloCOMPILER_GetCurrentSpace(Compiler) == sloCOMPILER_GetGlobalSpace(Compiler))
           {
               flag = Compiler->context.compilerFlags;
               slsCOMPILER_SetUnspecifiedUniformLocation(flag);
               Compiler->context.compilerFlags = flag;
           }

           if(shaderType == slvSHADER_TYPE_FRAGMENT &&
              TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT)
           {
               status = sloCOMPILER_SetUnspecifiedOutputLocationExist(Compiler);

               if(gcmIS_ERROR(status) && sloCOMPILER_NeedCheckOutputLocationExist(Compiler))
               {
                  gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                  TypeQualifier->lineNo,
                                                  TypeQualifier->stringNo,
                                                  slvREPORT_ERROR,
                                                  "fragment shader's output location has to be explicitly specified"));
                   gcmFOOTER_ARG("<return>=%s", "<nil>");
                   return gcvNULL;
               }
           }
       }

       DataType->qualifiers.interpolation = TypeQualifier->u.qualifiers.interpolation;

       if(shaderType == slvSHADER_TYPE_VERTEX)
       {
          if(TypeQualifier->u.qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_DEFAULT &&
              TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN)
          {
             gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                             TypeQualifier->lineNo,
                                             TypeQualifier->stringNo,
                                             slvREPORT_ERROR,
                                             "vertex shader input cannot have interpolation qualifiers"));
             gcmFOOTER_ARG("<return>=%s", "<nil>");
             return gcvNULL;
          }
       }
       else if(shaderType == slvSHADER_TYPE_FRAGMENT)
       {
          if(TypeQualifier->u.qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_DEFAULT &&
              TypeQualifier->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT)
          {
             gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                             TypeQualifier->lineNo,
                                             TypeQualifier->stringNo,
                                             slvREPORT_ERROR,
                                             "fragment shader output cannot have interpolation qualifiers"));
             gcmFOOTER_ARG("<return>=%s", "<nil>");
             return gcvNULL;
          }
       }
    }

    if(DataType->qualifiers.storage == slvSTORAGE_QUALIFIER_NONE)
    {
        DataType->qualifiers.storage = TypeQualifier->u.qualifiers.storage;
    }

    switch (DataType->qualifiers.storage)
    {
    case slvSTORAGE_QUALIFIER_UNIFORM:
        if (sloCOMPILER_IsHaltiVersion(Compiler) &&
            !sloCOMPILER_IsES31VersionOrAbove(Compiler) &&
            TypeQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
        {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                           TypeQualifier->lineNo,
                                           TypeQualifier->stringNo,
                                           slvREPORT_ERROR,
                                           "location id not applicable for uniform variable"));
           gcmFOOTER_ARG("<return>=%s", "<nil>");
           return gcvNULL;
        }

        mustAtGlobalNameSpace = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_ATTRIBUTE:
        if (!sloCOMPILER_IsOGLVersion(Compiler) && !slsDATA_TYPE_IsFloatOrVecOrMat(DataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "the 'attribute' qualifier can be used only with"
                                            " the data types: 'float', 'vec2', 'vec3',"
                                            " 'vec4', 'mat2', 'mat3', and 'mat4'"));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        mustAtGlobalNameSpace = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_VARYING_OUT:
    case slvSTORAGE_QUALIFIER_VARYING_IN:
        if (DataType->elementType != slvTYPE_FLOAT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "the 'varying' qualifier can be used only with"
                                            " the data types: 'float', 'vec2', 'vec3',"
                                            " 'vec4', 'mat2', 'mat3', and 'mat4', or arrays of these"));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        mustAtGlobalNameSpace = gcvTRUE;
        break;

    case slvSTORAGE_QUALIFIER_OUT:
    case slvSTORAGE_QUALIFIER_IN:
        if (!sloCOMPILER_IsHaltiVersion(Compiler) && !(sloCOMPILER_IsOGLVersion(Compiler)))
        {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                TypeQualifier->lineNo,
                                                TypeQualifier->stringNo,
                                                slvREPORT_ERROR,
                                                "unrecognizable type qualifier \'%s\'",
                                                slGetStorageQualifierName(Compiler, TypeQualifier->u.qualifiers.storage)));

                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
        }

        if (DataType->elementType == slvTYPE_BOOL)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "the 'out' qualifier can't be used with boolean type."));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if(shaderType == slvSHADER_TYPE_COMPUTE)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "compute shader does not have user defined input/output variable."));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if (slsQUALIFIERS_HAS_FLAG(&TypeQualifier->u.qualifiers, slvQUALIFIERS_FLAG_PATCH))
        {
            slsQUALIFIERS_SET_FLAG(&DataType->qualifiers, slvQUALIFIERS_FLAG_PATCH);
        }

        slsQUALIFIERS_SET_AUXILIARY(&DataType->qualifiers, slsQUALIFIERS_GET_AUXILIARY(&TypeQualifier->u.qualifiers));

        mustAtGlobalNameSpace = gcvTRUE;
        break;

    default:
        /* For a IO block member, we need to copy the auxiliary qualifiers. */
        if (currentNameSpace->nameSpaceType == slvNAME_SPACE_TYPE_IO_BLOCK)
        {
            slsQUALIFIERS_SET_AUXILIARY(&DataType->qualifiers, slsQUALIFIERS_GET_AUXILIARY(&TypeQualifier->u.qualifiers));
        }
        mustAtGlobalNameSpace = gcvFALSE;
        break;
    }

    if (mustAtGlobalNameSpace)
    {
        gcmVERIFY_OK(sloCOMPILER_AtGlobalNameSpace(Compiler, &atGlobalNameSpace));

        if (!atGlobalNameSpace)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            TypeQualifier->lineNo,
                                            TypeQualifier->stringNo,
                                            slvREPORT_ERROR,
                                            "the '%s' qualifier can be used to declare"
                                            " global variables only",
                                            slGetStorageQualifierName(Compiler, TypeQualifier->u.qualifiers.storage)));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    DataType->qualifiers.memoryAccess |= TypeQualifier->u.qualifiers.memoryAccess;
    DataType->qualifiers.interpolation |= TypeQualifier->u.qualifiers.interpolation;
    if(slsQUALIFIERS_HAS_FLAG(&TypeQualifier->u.qualifiers, slvQUALIFIERS_FLAG_PRECISION))
    {
        DataType->qualifiers.precision = TypeQualifier->u.qualifiers.precision;
    }

    gcmFOOTER_ARG("<return>=0x%x", DataType);
    return DataType;
}

static gctINT _GetOrderOfQualifier(
    IN sleQUALIFIERS_FLAG QualifierFlag
    )
{
    if(slvQUALIFIERS_FLAG_PRECISION & QualifierFlag)
    {
        return 3;
    }
    else if (slvQUALIFIERS_FLAG_STORAGE & QualifierFlag ||
             slvQUALIFIERS_FLAG_AUXILIARY & QualifierFlag)
    {
        return 2;
    }
    else if (slvQUALIFIERS_FLAG_INTERPOLATION & QualifierFlag)
    {
        return 1;
    }
    else
        return 0;
}

slsLexToken
slMergeTypeQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken *Qualifiers,
    IN slsLexToken *ComingQualifier
    )
{
    slsLexToken token = {0};
    sleQUALIFIERS_FLAG qualifier;

    gcmHEADER_ARG("Compiler=0x%x Qualifiers=0x%x ComingQualifier=0x%x",
                 Compiler, Qualifiers, ComingQualifier);

    gcmASSERT(ComingQualifier);

    token.lineNo = ComingQualifier->lineNo;
    token.stringNo = ComingQualifier->stringNo;
    if (Qualifiers == gcvNULL)
    {
        slsLAYOUT_Initialize(&token.u.qualifiers.layout);
        Qualifiers = &token;
    }

    if (!slsQUALIFIERS_GET_KINDS(&ComingQualifier->u.qualifiers))
    {
        return *Qualifiers;
    }

    /* Order checking for ES20/ES30. */
    if (slsQUALIFIERS_HAS_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_CENTROID_OR_IN_OR_OUT))
    {
        if (slsQUALIFIERS_GET_AUXILIARY(&ComingQualifier->u.qualifiers) != slvAUXILIARY_QUALIFIER_CENTROID &&
            ComingQualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_IN &&
            ComingQualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_OUT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                         Compiler,
                         ComingQualifier->lineNo,
                         ComingQualifier->stringNo,
                         slvREPORT_ERROR,
                         "in ES30 or earlier version, flat/smooth should be followed by centroid or in or out."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return *Qualifiers;
        }
        else
        {
            slsQUALIFIERS_RESET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_CENTROID_OR_IN_OR_OUT);
        }
    }

    if (slsQUALIFIERS_HAS_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_IN_OR_OUT))
    {
        if (ComingQualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_IN &&
            ComingQualifier->u.qualifiers.storage != slvSTORAGE_QUALIFIER_OUT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                         Compiler,
                         ComingQualifier->lineNo,
                         ComingQualifier->stringNo,
                         slvREPORT_ERROR,
                         "in ES30 or earlier version, centroid/sample should be followed by in or out."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return *Qualifiers;
        }
        else
        {
            slsQUALIFIERS_RESET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_IN_OR_OUT);
        }
    }

    qualifier = slsQUALIFIERS_GET_KINDS(&ComingQualifier->u.qualifiers);

    if (!sloCOMPILER_IsES31VersionOrAbove(Compiler) &&
        (_GetOrderOfQualifier(qualifier) <
        _GetOrderOfQualifier(slsQUALIFIERS_GET_KINDS(&(Qualifiers->u.qualifiers)))))
    {
            gcmVERIFY_OK(sloCOMPILER_Report(
                         Compiler,
                         ComingQualifier->lineNo,
                         ComingQualifier->stringNo,
                         slvREPORT_ERROR,
                         "In ES30 or earlier version, when multiple qualifications are present, they must follow a strit order."));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return *Qualifiers;
    }

    /* Merge qualifiers. */
    switch (qualifier)
    {
        case slvQUALIFIERS_FLAG_LAYOUT:
        {
            if (Qualifiers->u.qualifiers.layout.id & ComingQualifier->u.qualifiers.layout.id ||
                Qualifiers->u.qualifiers.layout.ext_id & ComingQualifier->u.qualifiers.layout.ext_id)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "layout qualifiers have duplicate items"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            else
            {
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_LOCATION)
                {
                    Qualifiers->u.qualifiers.layout.location = ComingQualifier->u.qualifiers.layout.location;
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_X)
                {
                    Qualifiers->u.qualifiers.layout.workGroupSize[0] = ComingQualifier->u.qualifiers.layout.workGroupSize[0];
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_Y)
                {
                    Qualifiers->u.qualifiers.layout.workGroupSize[1] = ComingQualifier->u.qualifiers.layout.workGroupSize[1];
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_Z)
                {
                    Qualifiers->u.qualifiers.layout.workGroupSize[2] = ComingQualifier->u.qualifiers.layout.workGroupSize[2];
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_BINDING)
                {
                    Qualifiers->u.qualifiers.layout.binding = ComingQualifier->u.qualifiers.layout.binding;
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_OFFSET)
                {
                    Qualifiers->u.qualifiers.layout.offset = ComingQualifier->u.qualifiers.layout.offset;
                }
                if (ComingQualifier->u.qualifiers.layout.id & slvLAYOUT_IMAGE_FORMAT)
                {
                    Qualifiers->u.qualifiers.layout.imageFormat = ComingQualifier->u.qualifiers.layout.imageFormat;
                }
                Qualifiers->u.qualifiers.layout.id |= ComingQualifier->u.qualifiers.layout.id;
                /* Check ext id. */
                /* TS layout. */
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE)
                {
                    Qualifiers->u.qualifiers.layout.tesPrimitiveMode = ComingQualifier->u.qualifiers.layout.tesPrimitiveMode;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_VERTEX_SPACING)
                {
                    Qualifiers->u.qualifiers.layout.tesVertexSpacing = ComingQualifier->u.qualifiers.layout.tesVertexSpacing;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_ORERING)
                {
                    Qualifiers->u.qualifiers.layout.tesOrdering = ComingQualifier->u.qualifiers.layout.tesOrdering;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_POINT_MODE)
                {
                    Qualifiers->u.qualifiers.layout.tesPointMode = ComingQualifier->u.qualifiers.layout.tesPointMode;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_VERTICES)
                {
                    Qualifiers->u.qualifiers.layout.verticesNumber = ComingQualifier->u.qualifiers.layout.verticesNumber;
                }
                /* GS layout. */
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_GS_PRIMITIVE)
                {
                    Qualifiers->u.qualifiers.layout.gsPrimitive = ComingQualifier->u.qualifiers.layout.gsPrimitive;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_INVOCATIONS)
                {
                    Qualifiers->u.qualifiers.layout.gsInvocationTime = ComingQualifier->u.qualifiers.layout.gsInvocationTime;
                }
                if (ComingQualifier->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_MAX_VERTICES)
                {
                    Qualifiers->u.qualifiers.layout.maxGSVerticesNumber = ComingQualifier->u.qualifiers.layout.maxGSVerticesNumber;
                }
                Qualifiers->u.qualifiers.layout.ext_id |= ComingQualifier->u.qualifiers.layout.ext_id;
            }
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_LAYOUT);
            break;
        }
        case slvQUALIFIERS_FLAG_MEMORY_ACCESS:
        {
            Qualifiers->u.qualifiers.memoryAccess |= ComingQualifier->u.qualifiers.memoryAccess;
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_MEMORY_ACCESS);
            break;
        }
        case slvQUALIFIERS_FLAG_AUXILIARY:
        {
            if (!sloCOMPILER_IsES31VersionOrAbove(Compiler))
            {
                slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_IN_OR_OUT);
            }
            if (slsQUALIFIERS_GET_AUXILIARY(&Qualifiers->u.qualifiers))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "auxiliary qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            slsQUALIFIERS_SET_AUXILIARY(&Qualifiers->u.qualifiers, slsQUALIFIERS_GET_AUXILIARY(&ComingQualifier->u.qualifiers));
            break;
        }
        case slvQUALIFIERS_FLAG_STORAGE:
        {
            if (Qualifiers->u.qualifiers.storage)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "storage qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            Qualifiers->u.qualifiers.storage = ComingQualifier->u.qualifiers.storage;
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_STORAGE);
            if (slsQUALIFIERS_HAS_FLAG(&ComingQualifier->u.qualifiers, slvQUALIFIERS_FLAG_PATCH))
            {
                slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PATCH);
            }
            break;
        }
        case slvQUALIFIERS_FLAG_PRECISION:
        {
            if (Qualifiers->u.qualifiers.precision)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "precision qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            Qualifiers->u.qualifiers.precision = ComingQualifier->u.qualifiers.precision;
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISION);
            break;
        }
        case slvQUALIFIERS_FLAG_INVARIANT:
        {
            if (slsQUALIFIERS_HAS_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "variance qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_INVARIANT);
            break;
        }
        case slvQUALIFIERS_FLAG_PRECISE:
        {
            if (slsQUALIFIERS_HAS_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                    Compiler,
                    ComingQualifier->lineNo,
                    ComingQualifier->stringNo,
                    slvREPORT_ERROR,
                    "precise qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE);
            break;
        }
        case slvQUALIFIERS_FLAG_INTERPOLATION:
        {
            if (Qualifiers->u.qualifiers.interpolation)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(
                             Compiler,
                             ComingQualifier->lineNo,
                             ComingQualifier->stringNo,
                             slvREPORT_ERROR,
                             "interpolation qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *Qualifiers;
            }
            if (!sloCOMPILER_IsES31VersionOrAbove(Compiler))
            {
                slsQUALIFIERS_SET_FLAG(&Qualifiers->u.qualifiers, slvQUALIFIERS_FLAG_EXPECTING_CENTROID_OR_IN_OR_OUT);
            }
            Qualifiers->u.qualifiers.interpolation = ComingQualifier->u.qualifiers.interpolation;
            break;
        }
        default:
        {
            gcmASSERT(0);
        }
    }

   gcmFOOTER_ARG("<return>=0x%x", &token);
   return *Qualifiers;
}

slsLexToken
slMergeParameterQualifiers(
    IN sloCOMPILER Compiler,
    IN slsLexToken *CurrentQualifiers,      /* The left qualifiers*/
    IN slsLexToken *IncomingQualifiers      /* The right qualifiers. */
    )
{
    sleQUALIFIERS_FLAG qualifier;
    gctBOOL needCheckQualifierOrder;

    gcmHEADER_ARG("Compiler=0x%x CurrentQualifiers=0x%x IncomingQualifiers=0x%x",
                 Compiler, CurrentQualifiers, IncomingQualifiers);

    gcmASSERT(CurrentQualifiers && IncomingQualifiers);

    /*
    ** For a ES20/ES30 shader, we need to check the parameter order and the order is:
    ** const-qualifier(const) --> parameter-qualifier(in/out) --> precision-qualifier(higph/lowp)
    */
    needCheckQualifierOrder = !sloCOMPILER_IsES31VersionOrAbove(Compiler);

    if (!slsQUALIFIERS_GET_KINDS(&IncomingQualifiers->u.qualifiers))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return *CurrentQualifiers;
    }

    qualifier = (sleQUALIFIERS_FLAG)slsQUALIFIERS_GET_KINDS(&IncomingQualifiers->u.qualifiers);

    switch (qualifier)
    {
        case slvQUALIFIERS_FLAG_STORAGE:
        {
            if (needCheckQualifierOrder)
            {
                if (IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_CONST &&
                    slsQUALIFIERS_GET_KINDS(&CurrentQualifiers->u.qualifiers))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    IncomingQualifiers->lineNo,
                                                    IncomingQualifiers->stringNo,
                                                    slvREPORT_ERROR,
                                                    "const-qualifier must appear first."));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return *CurrentQualifiers;
                }

                if ((IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN ||
                     IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT ||
                     IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_INOUT)
                    &&
                    (slsQUALIFIERS_GET_KINDS(&CurrentQualifiers->u.qualifiers) & slvQUALIFIERS_FLAG_PRECISION))
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    IncomingQualifiers->lineNo,
                                                    IncomingQualifiers->stringNo,
                                                    slvREPORT_ERROR,
                                                    "parameter-qualifier must appear before precision-qualifier."));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return *CurrentQualifiers;
                }
            }

            if (CurrentQualifiers->u.qualifiers.storage)
            {
                if ((CurrentQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_CONST &&
                     IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN) ||
                    (CurrentQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN &&
                     IncomingQualifiers->u.qualifiers.storage == slvSTORAGE_QUALIFIER_CONST))
                {
                    CurrentQualifiers->u.qualifiers.storage = slvSTORAGE_QUALIFIER_CONST_IN;
                }
                else
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    IncomingQualifiers->lineNo,
                                                    IncomingQualifiers->stringNo,
                                                    slvREPORT_ERROR,
                                                    "storage qualifier is set multiple times"));
                    gcmFOOTER_ARG("<return>=%s", "<nil>");
                    return *CurrentQualifiers;
                }
            }
            else
            {
                CurrentQualifiers->u.qualifiers.storage = IncomingQualifiers->u.qualifiers.storage;
            }

            slsQUALIFIERS_SET_FLAG(&CurrentQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_STORAGE);
            break;
        }
        case slvQUALIFIERS_FLAG_PRECISION:
        {
            if (CurrentQualifiers->u.qualifiers.precision)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                IncomingQualifiers->lineNo,
                                                IncomingQualifiers->stringNo,
                                                slvREPORT_ERROR,
                                                "precision qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *CurrentQualifiers;
            }
            else
            {
                CurrentQualifiers->u.qualifiers.precision = IncomingQualifiers->u.qualifiers.precision;
                slsQUALIFIERS_SET_FLAG(&CurrentQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISION);
            }

            break;
        }
        case slvQUALIFIERS_FLAG_PRECISE:
        {
            if (slsQUALIFIERS_HAS_FLAG(&CurrentQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE))
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                IncomingQualifiers->lineNo,
                                                IncomingQualifiers->stringNo,
                                                slvREPORT_ERROR,
                                                "precise qualifier is set multiple times"));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return *CurrentQualifiers;
            }
            else
            {
                slsQUALIFIERS_SET_FLAG(&CurrentQualifiers->u.qualifiers, slvQUALIFIERS_FLAG_PRECISE);
            }

            break;
        }
        default:
        {
            gcmASSERT(0);
        }
    }

   gcmFOOTER_ARG("<return>=%s", "<nil>");
   return *CurrentQualifiers;
}

static void
_ParseSearchLayoutId(
    sloCOMPILER                         Compiler,
    slsLexToken *                       LayoutId,
    gctINT16    *                       imageFormat,
    slvTES_PRIMITIVE_MODE *             TesPrimitiveMode,
    slvTES_VERTEX_SPACING *             TesVertexSpacing,
    slvTES_ORDERING *                   TesOrdering,
    slvTES_POINT_MODE *                 TesPointMode,
    slvGS_PRIMITIVE *                   GsPrimitive,
    sleLAYOUT_ID *                      LayoutId1,
    sleLAYOUT_ID_EXT *                  LayoutId2
    )
{
    sleLAYOUT_ID        layoutId1 = slvLAYOUT_NONE;
    sleLAYOUT_ID_EXT    layoutId2 = slvLAYOUT_EXT_NONE;
    sleSHADER_TYPE      shaderType;
    sleCOMPILER_FLAGS   flag;
    sloEXTENSION extension = {0};

    shaderType = Compiler->shaderType;
    flag = Compiler->context.compilerFlags;

    /* Match layoutId1. */
    extension.extension2 = slvEXTENSION2_GL_ARB_EXPLICIT_ATTRIB_LOCATION;
    if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "location"))) {
        if ((!sloCOMPILER_IsOGLVersion(Compiler)) ||
            ((sloCOMPILER_GetLanguageVersion(Compiler) > _SHADER_GL15_VERSION ||
            (sloCOMPILER_GetLanguageVersion(Compiler) == _SHADER_GL15_VERSION &&
             sloCOMPILER_ExtensionEnabled(Compiler, &extension)))))
        {
            layoutId1 = slvLAYOUT_LOCATION;
        }
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "shared"))) {
        layoutId1 = slvLAYOUT_SHARED;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "packed"))) {
        layoutId1 = slvLAYOUT_PACKED;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "std140"))) {
        layoutId1 = slvLAYOUT_STD140;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "std430"))) {
        layoutId1 = slvLAYOUT_STD430;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "row_major"))) {
        layoutId1 = slvLAYOUT_ROW_MAJOR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "column_major"))) {
        layoutId1 = slvLAYOUT_COLUMN_MAJOR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "local_size_x"))) {
        layoutId1 = slvLAYOUT_WORK_GROUP_SIZE_X;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "local_size_y"))) {
        layoutId1 = slvLAYOUT_WORK_GROUP_SIZE_Y;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "local_size_z"))) {
        layoutId1 = slvLAYOUT_WORK_GROUP_SIZE_Z;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "binding"))) {
        layoutId1 = slvLAYOUT_BINDING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "early_fragment_tests"))) {
        slsCOMPILER_SetEarlyFragText(flag);
        layoutId1 = slvLAYOUT_EARLY_FRAGMENT_TESTS;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba32f"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA32F;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba16f"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA16F;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "r32f"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_R32F;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba8"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA8;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba8_snorm"))) {
       *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA8_SNORM;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba32i"))) {
       *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA32I;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba16i"))) {
       *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA16I;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba8i"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA8I;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "r32i"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_R32I;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba32ui"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA32UI;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba16ui"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA16UI;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "rgba8ui"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_RGBA8UI;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "r32ui"))) {
        *imageFormat = slvLAYOUT_IMAGE_FORMAT_R32UI;
        layoutId1 = slvLAYOUT_IMAGE_FORMAT;
    }
    else if (sloCOMPILER_IsES31VersionOrAbove(Compiler) &&
        gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "offset"))) {
        layoutId1 = slvLAYOUT_OFFSET;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_multiply"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_MULTIPLY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_screen"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_SCREEN;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_overlay"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_OVERLAY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_darken"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_DARKEN;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_lighten"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_LIGHTEN;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_colordodge"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_COLORDODGE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_colorburn"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_COLORBURN;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_hardlight"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_HARDLIGHT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_softlight"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_SOFTLIGHT;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_difference"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_DIFFERENCE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_exclusion"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_EXCLUSION;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_hsl_hue"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_HSL_HUE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_hsl_saturation"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_HSL_SATURATION;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_hsl_color"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_HSL_COLOR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_hsl_luminosity"))) {
        layoutId1 = slvLAYOUT_BLEND_SUPPORT_HSL_LUMINOSITY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "blend_support_all_equations"))) {
        layoutId1 = sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS;
    }
    /* Match layoutId2. */
    /* Match TS layout. */
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "triangles"))) {
        if (shaderType == slvSHADER_TYPE_GS)
        {
            *GsPrimitive = slvGS_TRIANGLES;
            layoutId2 = slvLAYOUT_EXT_GS_TRIANGLES;
        }
        else
        {
            *TesPrimitiveMode = slvTES_PRIMITIVE_MODE_TRIANGLES;
            layoutId2 = slvLAYOUT_EXT_TS_PRIMITIVE_MODE;
        }
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "quads"))) {
        *TesPrimitiveMode = slvTES_PRIMITIVE_MODE_QUADS;
        layoutId2 = slvLAYOUT_EXT_TS_PRIMITIVE_MODE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "isolines"))) {
        *TesPrimitiveMode = slvTES_PRIMITIVE_MODE_ISOLINES;
        layoutId2 = slvLAYOUT_EXT_TS_PRIMITIVE_MODE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "equal_spacing"))) {
        *TesVertexSpacing = slvTES_VERTEX_SPACING_EQUAL_SPACING;
        layoutId2 = slvlAYOUT_EXT_VERTEX_SPACING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "fractional_even_spacing"))) {
        *TesVertexSpacing = slvTES_VERTEX_SPACING_FRACTIONAL_EVEN_SPACING;
        layoutId2 = slvlAYOUT_EXT_VERTEX_SPACING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "fractional_odd_spacing"))) {
        *TesVertexSpacing = slvTES_VERTEX_SPACING_FRACTIONAL_ODD_SPACING;
        layoutId2 = slvlAYOUT_EXT_VERTEX_SPACING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "cw"))) {
        *TesOrdering = slvTES_ORDERING_CW;
        layoutId2 = slvlAYOUT_EXT_ORERING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "ccw"))) {
        *TesOrdering = slvTES_ORDERING_CCW;
        layoutId2 = slvlAYOUT_EXT_ORERING;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "point_mode"))) {
        *TesPointMode = slvTES_POINT_MODE_POINT_MODE;
        layoutId2 = slvLAYOUT_EXT_POINT_MODE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "vertices"))) {
        layoutId2 = slvLAYOUT_EXT_VERTICES;
    }
    /* Match GS layout. */
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "points"))) {
        *GsPrimitive = slvGS_POINTS;
        layoutId2 = slvLAYOUT_EXT_GS_POINTS;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "lines"))) {
        *GsPrimitive = slvGS_LINES;
        layoutId2 = slvLAYOUT_EXT_GS_LINES;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "lines_adjacency"))) {
        *GsPrimitive = slvGS_LINES_ADJACENCY;
        layoutId2 = slvLAYOUT_EXT_GS_LINES_ADJACENCY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "triangles_adjacency"))) {
        *GsPrimitive = slvGS_TRIANGLES_ADJACENCY;
        layoutId2 = slvLAYOUT_EXT_GS_TRIANGLES_ADJACENCY;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "line_strip"))) {
        *GsPrimitive = slvGS_LINE_STRIP;
        layoutId2 = slvLAYOUT_EXT_GS_LINE_STRIP;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "triangle_strip"))) {
        *GsPrimitive = slvGS_TRIANGLE_STRIP;
        layoutId2 = slvLAYOUT_EXT_GS_TRIANGLE_STRIP;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "invocations"))) {
        layoutId2 = slvLAYOUT_EXT_INVOCATIONS;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(LayoutId->u.identifier, "max_vertices"))) {
        layoutId2 = slvLAYOUT_EXT_MAX_VERTICES;
    }
    if (LayoutId1)
    {
        *LayoutId1 = layoutId1;
    }

    if (LayoutId2)
    {
        *LayoutId2 = layoutId2;
    }

    Compiler->context.compilerFlags = flag;
}

slsLexToken
slParseLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutId,
    IN slsLexToken * Value
    )
{
    slsLexToken                     layoutQualifier;
    sleLAYOUT_ID                    layoutId = slvLAYOUT_NONE;
    sleLAYOUT_ID_EXT                layoutIdExt =slvLAYOUT_EXT_NONE;
    gctINT16                        imageForat = 0;
    slvTES_PRIMITIVE_MODE           tesPrimitiveMode = slvTES_PRIMITIVE_MODE_NONE;
    slvTES_VERTEX_SPACING           tesVertexSpacing = slvTES_VERTEX_SPACING_NONE;
    slvTES_ORDERING                 tesOrdering = slvTES_ORDERING_NONE;
    slvTES_POINT_MODE               tesPointMode = slvTES_POINT_MODE_NONE;
    slvGS_PRIMITIVE                 gsPrimitive = slvGS_PRIMITIVE_NONE;
    sloEXTENSION                    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x Layout Id=0x%x Value=0x%x",
                  Compiler, LayoutId, Value);

    gcmASSERT(LayoutId);
    gcoOS_ZeroMemory(&layoutQualifier, gcmSIZEOF(layoutQualifier));
    layoutQualifier.type          = T_LAYOUT;
    layoutQualifier.lineNo        = LayoutId->lineNo;
    layoutQualifier.stringNo      = LayoutId->stringNo;
    slsLAYOUT_Initialize(&(layoutQualifier.u.qualifiers.layout));

    _ParseSearchLayoutId(Compiler,
                         LayoutId,
                         &imageForat,
                         &tesPrimitiveMode,
                         &tesVertexSpacing,
                         &tesOrdering,
                         &tesPointMode,
                         &gsPrimitive,
                         &layoutId,
                         &layoutIdExt);

    extension.extension1 = slvEXTENSION1_BLEND_EQUATION_ADVANCED;
    do {
        if(layoutId == slvLAYOUT_NONE && layoutIdExt == slvLAYOUT_EXT_NONE) {
          gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                          LayoutId->lineNo,
                                          LayoutId->stringNo,
                                          slvREPORT_ERROR,
                                          "unrecognizable layout id \'%s\'",
                                          LayoutId->u.identifier));
          break;
       }

       if((layoutId & sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS) &&
           !sloCOMPILER_ExtensionEnabled(Compiler, &extension))
       {
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                          LayoutId->lineNo,
                                          LayoutId->stringNo,
                                          slvREPORT_ERROR,
                                          "blend equation advanced is not enabled but used(\'%s\')",
                                          LayoutId->u.identifier));
           break;
       }

       layoutQualifier.u.qualifiers.layout.id = layoutId;
       layoutQualifier.u.qualifiers.layout.ext_id = layoutIdExt;

       if (layoutIdExt != slvLAYOUT_EXT_NONE)
       {
           layoutQualifier.u.qualifiers.layout.tesPrimitiveMode  = tesPrimitiveMode;
           layoutQualifier.u.qualifiers.layout.tesVertexSpacing  = tesVertexSpacing;
           layoutQualifier.u.qualifiers.layout.tesOrdering       = tesOrdering;
           layoutQualifier.u.qualifiers.layout.tesPointMode      = tesPointMode;
           layoutQualifier.u.qualifiers.layout.gsPrimitive       = gsPrimitive;

           if (layoutIdExt & slvLAYOUT_EXT_VERTICES)
           {
               if(Value->u.constant.intValue <= 0)
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "invalid vertex count number",
                                                   LayoutId->u.identifier));
                   break;
               }
               else
               {
                   layoutQualifier.u.qualifiers.layout.verticesNumber = Value->u.constant.intValue;
               }
           }
           else if (layoutIdExt & slvLAYOUT_EXT_INVOCATIONS)
           {
               layoutQualifier.u.qualifiers.layout.gsInvocationTime = Value->u.constant.intValue;
           }
           else if (layoutIdExt & slvLAYOUT_EXT_MAX_VERTICES)
           {
               layoutQualifier.u.qualifiers.layout.maxGSVerticesNumber = Value->u.constant.intValue;
           }
       }

       if (layoutId & slvLAYOUT_IMAGE_FORMAT)
       {
            layoutQualifier.u.qualifiers.layout.imageFormat = imageForat;
       }

       if(layoutId != slvLAYOUT_NONE && Value) {
           switch(layoutId) {
           case slvLAYOUT_LOCATION:
               layoutQualifier.u.qualifiers.layout.location = Value->u.constant.intValue;
               break;

           case slvLAYOUT_WORK_GROUP_SIZE_X:
               if (Value->u.constant.intValue > (gctINT)GetGLMaxComputeWorkGroupSize(0))
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "local_size_x \"%d\" is greater than the maximum value \"%d\".",
                                                   Value->u.constant.intValue,
                                                   GetGLMaxComputeWorkGroupSize(0)));
               }
               layoutQualifier.u.qualifiers.layout.workGroupSize[0] = Value->u.constant.intValue;
               break;

           case slvLAYOUT_WORK_GROUP_SIZE_Y:
               if (Value->u.constant.intValue > (gctINT)GetGLMaxComputeWorkGroupSize(1))
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "local_size_y \"%d\" is greater than the maximum value \"%d\".",
                                                   Value->u.constant.intValue,
                                                   GetGLMaxComputeWorkGroupSize(1)));
               }
               layoutQualifier.u.qualifiers.layout.workGroupSize[1] = Value->u.constant.intValue;
               break;

           case slvLAYOUT_WORK_GROUP_SIZE_Z:
               if (Value->u.constant.intValue > (gctINT)GetGLMaxComputeWorkGroupSize(2))
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "local_size_z \"%d\" is greater than the maximum value \"%d\".",
                                                   Value->u.constant.intValue,
                                                   GetGLMaxComputeWorkGroupSize(2)));
               }
               layoutQualifier.u.qualifiers.layout.workGroupSize[2] = Value->u.constant.intValue;
               break;

           case slvLAYOUT_BINDING:
               layoutQualifier.u.qualifiers.layout.binding = Value->u.constant.intValue;
               break;

           case slvLAYOUT_OFFSET:
               layoutQualifier.u.qualifiers.layout.offset = Value->u.constant.intValue;
               break;

           default:
               gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                               LayoutId->lineNo,
                                               LayoutId->stringNo,
                                               slvREPORT_ERROR,
                                               "value not applicable to layout id \'%s\'",
                                               LayoutId->u.identifier));
               break;
          }
       }
    } while (gcvFALSE);

    gcmFOOTER_ARG("<return>=0x%x", &layoutQualifier);
    return layoutQualifier;
}


static gctCONST_STRING
_ParseGetLayoutIdName(
IN sleLAYOUT_ID Id
)
{
   switch(Id) {
   case slvLAYOUT_SHARED:
        return "shared";

   case slvLAYOUT_PACKED:
        return "packed";

   case slvLAYOUT_STD140:
        return "std140";

   case slvLAYOUT_STD430:
        return "std430";

   case slvLAYOUT_ROW_MAJOR:
        return "row_major";

   case slvLAYOUT_COLUMN_MAJOR:
        return "column_major";

   case slvLAYOUT_LOCATION:
        return "location";

   case slvLAYOUT_WORK_GROUP_SIZE_X:
        return "local_size_x";

   case slvLAYOUT_WORK_GROUP_SIZE_Y:
        return "local_size_y";

   case slvLAYOUT_WORK_GROUP_SIZE_Z:
        return "local_size_z";

   case slvLAYOUT_BINDING:
        return "binding";

   case slvLAYOUT_OFFSET:
       return "offset";

   default:
        return "invalid id";
   }
}

slsLexToken
slParseLayoutQualifier(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutIdList
    )
{
    gcmHEADER_ARG("Compiler=0x%x Layout Id List=0x%x",
                  Compiler, LayoutIdList);

    gcmASSERT(LayoutIdList);

    slsQUALIFIERS_SET_FLAG(&LayoutIdList->u.qualifiers, slvQUALIFIERS_FLAG_LAYOUT);

    gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
    return *LayoutIdList;
}

slsLexToken
slParseAddLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLexToken * LayoutIdList,
    IN slsLexToken * LayoutId
    )
{
    gcmHEADER_ARG("Compiler=0x%x Layout Id List=0x%x LayoutId=0x%x",
                  Compiler, LayoutIdList, LayoutId);

    gcmASSERT(LayoutIdList);
    gcmASSERT(LayoutId);

    do
    {
       if (LayoutId->u.qualifiers.layout.id == slvLAYOUT_NONE &&
           LayoutId->u.qualifiers.layout.ext_id == slvLAYOUT_EXT_NONE)
           break;

       if (LayoutId->u.qualifiers.layout.id != slvLAYOUT_NONE)
       {
           if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_LOCATION) {
              if(LayoutIdList->u.qualifiers.layout.id & slvLAYOUT_LOCATION) {
                 gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                 LayoutId->lineNo,
                                                 LayoutId->stringNo,
                                                 slvREPORT_ERROR,
                                                 "Layout location already specified"));
                 gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
                 return *LayoutIdList;
              }
              else {
                 LayoutIdList->u.qualifiers.layout.location = LayoutId->u.qualifiers.layout.location;
              }
           }
           if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_OFFSET) {
               LayoutIdList->u.qualifiers.layout.offset = LayoutId->u.qualifiers.layout.offset;
           }
           if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_BINDING) {
              LayoutIdList->u.qualifiers.layout.binding = LayoutId->u.qualifiers.layout.binding;
           }
           if(LayoutId->u.qualifiers.layout.id & sldLAYOUT_MEMORY_BIT_FIELDS) {
              LayoutIdList->u.qualifiers.layout.id &= ~sldLAYOUT_MEMORY_BIT_FIELDS;
           }
           if(LayoutId->u.qualifiers.layout.id & sldLAYOUT_MATRIX_BIT_FIELDS) {
              LayoutIdList->u.qualifiers.layout.id &= ~sldLAYOUT_MATRIX_BIT_FIELDS;
           }
           if(LayoutId->u.qualifiers.layout.id & sldLAYOUT_WORK_GROUP_SIZE_FIELDS) {
              if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_X) {
                 LayoutIdList->u.qualifiers.layout.workGroupSize[0] = LayoutId->u.qualifiers.layout.workGroupSize[0];
              }
              if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_Y) {
                 LayoutIdList->u.qualifiers.layout.workGroupSize[1] = LayoutId->u.qualifiers.layout.workGroupSize[1];
              }
              if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_WORK_GROUP_SIZE_Z) {
                 LayoutIdList->u.qualifiers.layout.workGroupSize[2] = LayoutId->u.qualifiers.layout.workGroupSize[2];
              }
           }
           if(LayoutId->u.qualifiers.layout.id & slvLAYOUT_IMAGE_FORMAT) {
              LayoutIdList->u.qualifiers.layout.imageFormat = LayoutId->u.qualifiers.layout.imageFormat;
           }
           LayoutIdList->u.qualifiers.layout.id |= LayoutId->u.qualifiers.layout.id;
       }

       if (LayoutId->u.qualifiers.layout.ext_id != slvLAYOUT_EXT_NONE)
       {
           /* TS layout. */
           if (LayoutId->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE)
           {
               if(LayoutIdList->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_TS_PRIMITIVE_MODE)
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "declared multiple primitive modes in one layout"));
                   gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
                   return *LayoutIdList;
               }
               else
               {
                   LayoutIdList->u.qualifiers.layout.tesPrimitiveMode = LayoutId->u.qualifiers.layout.tesPrimitiveMode;
                   LayoutIdList->u.qualifiers.layout.ext_id |= slvLAYOUT_EXT_TS_PRIMITIVE_MODE;
               }
           }
           if (LayoutId->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_VERTEX_SPACING)
           {
               if(LayoutIdList->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_VERTEX_SPACING)
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "declared multiple vertex spacing in one layout"));
                   gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
                   return *LayoutIdList;
               }
               else
               {
                   LayoutIdList->u.qualifiers.layout.tesVertexSpacing = LayoutId->u.qualifiers.layout.tesVertexSpacing;
                   LayoutIdList->u.qualifiers.layout.ext_id |= slvlAYOUT_EXT_VERTEX_SPACING;
               }
           }
           if (LayoutId->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_ORERING)
           {
               if(LayoutIdList->u.qualifiers.layout.ext_id & slvlAYOUT_EXT_ORERING)
               {
                   gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                   LayoutId->lineNo,
                                                   LayoutId->stringNo,
                                                   slvREPORT_ERROR,
                                                   "declared multiple vertex ordering in one layout"));
                   gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
                   return *LayoutIdList;
               }
               else
               {
                   LayoutIdList->u.qualifiers.layout.tesOrdering = LayoutId->u.qualifiers.layout.tesOrdering;
                   LayoutIdList->u.qualifiers.layout.ext_id |= slvlAYOUT_EXT_ORERING;
               }
           }
           if (LayoutId->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_POINT_MODE)
           {
               LayoutIdList->u.qualifiers.layout.tesPointMode     = LayoutId->u.qualifiers.layout.tesPointMode;
               LayoutIdList->u.qualifiers.layout.ext_id |= slvLAYOUT_EXT_POINT_MODE;
           }
           /* GS layout. */
           if (LayoutId->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_GS_PRIMITIVE)
           {
               if (LayoutIdList->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_GS_PRIMITIVE)
               {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    LayoutId->lineNo,
                                                    LayoutId->stringNo,
                                                    slvREPORT_ERROR,
                                                    "Layout primitive already specified"));
                    gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
                    return *LayoutIdList;
               }
               LayoutIdList->u.qualifiers.layout.gsPrimitive = LayoutId->u.qualifiers.layout.gsPrimitive;
           }
           if (LayoutId->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_MAX_VERTICES)
               LayoutIdList->u.qualifiers.layout.maxGSVerticesNumber = LayoutId->u.qualifiers.layout.maxGSVerticesNumber;
           if (LayoutId->u.qualifiers.layout.ext_id & slvLAYOUT_EXT_INVOCATIONS)
               LayoutIdList->u.qualifiers.layout.gsInvocationTime = LayoutId->u.qualifiers.layout.gsInvocationTime;
           LayoutIdList->u.qualifiers.layout.ext_id |= LayoutId->u.qualifiers.layout.ext_id;
       }
    } while (gcvFALSE);

    gcmFOOTER_ARG("<return>=0x%x", LayoutIdList);
    return *LayoutIdList;
}

slsDATA_TYPE *
slParseNonStructType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Token,
    IN gctINT TokenType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x Token=0x%x TokenType=%d",
                  Compiler, Token, TokenType);

    gcmASSERT(Token);

    status = sloCOMPILER_CreateDataType(
                                    Compiler,
                                    TokenType,
                                    gcvNULL,
                                    &dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<DATA_TYPE line=\"%d\" string=\"%d\" name=\"%s\" />",
                                Token->lineNo,
                                Token->stringNo,
                                _GetTypeName(TokenType)));

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

slsDATA_TYPE *
slParseStructType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * StructType
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x StructType=0x%x",
                  Compiler, StructType);

    if (StructType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_CloneDataType(Compiler,
                                       slvSTORAGE_QUALIFIER_NONE,
                                       StructType->qualifiers.precision,
                                       StructType,
                                       &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

slsDATA_TYPE *
slParseNamedType(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;

    gcmHEADER_ARG("Compiler=0x%x TypeName=0x%x",
                  Compiler, TypeName);

    gcmASSERT(TypeName);
    gcmASSERT(TypeName->u.typeName);

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<DATA_TYPE line=\"%d\" string=\"%d\" name=\"%s\" />",
                                TypeName->lineNo,
                                TypeName->stringNo,
                                TypeName->u.typeName->symbol));


    status = sloCOMPILER_CloneDataType(Compiler,
                                       slvSTORAGE_QUALIFIER_NONE,
                                       TypeName->u.typeName->dataType->qualifiers.precision,
                                       TypeName->u.typeName->dataType,
                                       &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

void
slParseStructDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace;

    gcmHEADER_ARG("Compiler=0x%x",
                  Compiler);

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         Identifier ? Identifier->u.identifier : gcvNULL,
                                         slvNAME_SPACE_TYPE_STRUCT,
                                         &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STRUCT_DECL>"));
    gcmFOOTER_NO();
}

slsDATA_TYPE *
slParseStructDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;
    slsNAME *       field;
    slsNAME_SPACE * prevNameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x", Compiler, Identifier);

    status = sloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /*
    ** check it there is a embedded structure definition
    */
    FOR_EACH_DLINK_NODE(&(prevNameSpace->names), slsNAME, field)
    {
        if (field->type == slvSTRUCT_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "Embedded structure definitions are not supported"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_STRUCT,
                                        prevNameSpace,
                                        &dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (Identifier != gcvNULL)
    {
        sloEXTENSION extension = {0};
        extension.extension1 = slvEXTENSION1_NONE;
        status = sloCOMPILER_CreateName(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvSTRUCT_NAME,
                                        dataType,
                                        Identifier->u.identifier,
                                        extension,
                                        gcvTRUE,
                                        gcvNULL);

        if (gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</STRUCT_DECL>"));

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

void
slParseStructReDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace;

    gcmHEADER_ARG("Compiler=0x%x TypeName=0x%x", Compiler, TypeName);

    if (TypeName->u.typeName->mySpace == sloCOMPILER_GetCurrentSpace(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        " Struct \"%s\" is redeclared.",
                                        TypeName->u.typeName->symbol));
        return;
    }

    status = sloCOMPILER_CreateNameSpace(Compiler,
                                         gcvNULL,
                                         slvNAME_SPACE_TYPE_STRUCT,
                                         &nameSpace);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STRUCT_DECL>"));
    gcmFOOTER_NO();
}

slsDATA_TYPE *
slParseStructReDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeName
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;
    slsNAME *       field;
    slsNAME_SPACE * prevNameSpace = gcvNULL;
    sltPOOL_STRING  typeName = TypeName->u.typeName->symbol;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x TypeName=0x%x", Compiler, TypeName);

    status = sloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /*
    ** check it there is a embedded structure definition
    */
    FOR_EACH_DLINK_NODE(&(prevNameSpace->names), slsNAME, field)
    {
        if (field->type == slvSTRUCT_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "Embedded structure definitions are not supported"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    status = sloCOMPILER_CreateDataType(Compiler,
                                        T_STRUCT,
                                        prevNameSpace,
                                        &dataType);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    sloCOMPILER_GetCurrentLineNo(Compiler),
                                    sloCOMPILER_GetCurrentStringNo(Compiler),
                                    slvSTRUCT_NAME,
                                    dataType,
                                    typeName,
                                    extension,
                                    gcvTRUE,
                                    gcvNULL);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "</STRUCT_DECL>"));

    gcmFOOTER_ARG("<return>=0x%x", dataType);
    return dataType;
}

sloIR_BASE
slParseInterfaceBlockImplicitArrayLength(
    IN sloCOMPILER Compiler,
    IN slsNAME *Block,
    IN slsLexToken *BlockInstance
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT length = 0;
    sloIR_CONSTANT arrayLength;
    sleSHADER_TYPE  shaderType;
    slsDATA_TYPE * dataType;
    sluCONSTANT_VALUE value[4];
    slsLAYOUT_QUALIFIER layout;

    gcmHEADER_ARG("Compiler=0x%x Block=0x%x BlockInstance=0x%x",
                  Compiler, Block, BlockInstance);

    shaderType = Compiler->shaderType;

    if (Block == gcvNULL)
    {
        /* must have parsing error prior to this */
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }
    if (!(((shaderType == slvSHADER_TYPE_TCS || shaderType == slvSHADER_TYPE_TES) &&
          (Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK ||
           Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_OUT_IO_BLOCK))
          ||
          (shaderType == slvSHADER_TYPE_GS &&
          Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockInstance->lineNo,
                                        BlockInstance->stringNo,
                                        slvREPORT_ERROR,
                                        "Only TS/GS can support implicit array size for blocks."));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (!slsDATA_TYPE_IsUnderlyingIOBlock(Block->dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockInstance->lineNo,
                                        BlockInstance->stringNo,
                                        slvREPORT_ERROR,
                                        "Only IO block can support implicit array size."));

        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create the data type. */
    status = sloCOMPILER_CreateDataType(Compiler,
                                        T_INT,
                                        gcvNULL,
                                        &dataType);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Create the constant expr. */
    dataType->qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;
    status = sloIR_CONSTANT_Construct(Compiler,
                                      BlockInstance->lineNo,
                                      BlockInstance->stringNo,
                                      dataType,
                                      &arrayLength);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Set the value. */
    sloCOMPILER_GetDefaultLayout(Compiler,
                                 &layout,
                                 slvSTORAGE_QUALIFIER_OUT);
    if (shaderType == slvSHADER_TYPE_TES)
    {
        if (Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK)
        {
            length = layout.maxVerticesNumber;
        }
        else
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockInstance->lineNo,
                                            BlockInstance->stringNo,
                                            slvREPORT_ERROR,
                                            "\"%s\" can't be unsized array.",
                                            BlockInstance->u.identifier));

            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }
    else if (shaderType == slvSHADER_TYPE_GS && sloCOMPILER_IsOGLVersion(Compiler))
    {
        sloCOMPILER_GetDefaultLayout(Compiler,
                                     &layout,
                                     slvSTORAGE_QUALIFIER_IN);
        if (layout.gsPrimitive != slvGS_PRIMITIVE_NONE)
            length = _GetInputArraySizeByPrimitiveType(layout.gsPrimitive);
        else
            length = -1;
    }
    else
    {
        gcmASSERT(shaderType == slvSHADER_TYPE_GS || shaderType == slvSHADER_TYPE_TCS);
        length = -1;
    }
    value[0].intValue = length;

    status = sloIR_CONSTANT_AddValues(Compiler,
                                      arrayLength,
                                      1,
                                      value);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Creat the block. */
    slParseInterfaceBlock(Compiler,
                          Block,
                          BlockInstance,
                          (sloIR_EXPR)arrayLength,
                          gcvFALSE);

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

sloIR_BASE
slParseInterfaceBlock(
    IN sloCOMPILER Compiler,
    IN slsNAME *Block,
    IN slsLexToken *BlockInstance,
    IN sloIR_EXPR ArrayLengthExpr,
    IN gctBOOL CheckArrayLength
    )
{
    gceSTATUS status;
    slsNAME *field;
    slsNAME *memberName;
    slsNAME *instance;
    slsINTERFACE_BLOCK_MEMBER *blockMember;
    sltSTORAGE_QUALIFIER qualifier;
    gctPOINTER pointer;
    gctBOOL addMember;
    sleSHADER_TYPE shaderType;

    gcmHEADER_ARG("Compiler=0x%x Block=0x%x BlockInstance=0x%x",
                  Compiler, Block, BlockInstance);

    if (!Block)
    {
       gcmFOOTER_ARG("<return>=%s", "<nil>");
       return gcvNULL;
    }

    shaderType = Compiler->shaderType;
    addMember = slsDLINK_LIST_IsEmpty(&Block->u.interfaceBlockContent.members);

    if (slsDATA_TYPE_IsUnderlyingUniformBlock(Block->dataType))
    {
        qualifier = slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER;
    }
    else if (slsDATA_TYPE_IsUnderlyingStorageBlock(Block->dataType))
    {
        qualifier = slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER;
    }
    else if (slsDATA_TYPE_IsUnderlyingIOBlock(Block->dataType))
    {
        if (Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK)
        {
            if (shaderType == slvSHADER_TYPE_VERTEX)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to have an input block \"%s\" in a vertex shader",
                                                Block->symbol));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            qualifier = slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER;
        }
        else
        {
            if (shaderType == slvSHADER_TYPE_FRAGMENT)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to have an output block \"%s\" in a fragment shader",
                                                Block->symbol));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            qualifier = slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER;
        }
    }
    else
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* If this is a redeclaration for a built-in block, check instance name first, then just return NULL. */
    if (Compiler->context.redeclareBuiltInVar)
    {
        sleSHADER_TYPE shaderType = Compiler->shaderType;

        Compiler->context.redeclareBuiltInVar = gcvFALSE;
        if (BlockInstance)
        {
            slsNAME *instanceName = gcvNULL;

            status = slsNAME_SPACE_Search(Compiler,
                                          sloCOMPILER_GetBuiltInSpace(Compiler),
                                          BlockInstance->u.identifier,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvFALSE,
                                          gcvFALSE,
                                          &instanceName);
            if (gcmIS_ERROR(status)) { gcmFOOTER(); return gcvNULL; }

            if (!instanceName)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to change the built-in instance name in the redeclaration."));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            if (instanceName->u.variableInfo.interfaceBlock != Block)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to re-declare \"%s\"",
                                                BlockInstance->u.identifier));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            if (Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK &&
                shaderType == slvSHADER_TYPE_GS &&
                !ArrayLengthExpr)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to redeclare \"%s\" as a non-array.",
                                                BlockInstance->u.identifier));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
        }
        else
        {
            /* report error if redeclaring the gl_PerVertex input without specifying an instance name */
            if (Block->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK &&
                gcoOS_StrNCmp(Block->symbol, "gl_PerVertex", 12) == gcvSTATUS_OK)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                Block->lineNo,
                                                Block->stringNo,
                                                slvREPORT_ERROR,
                                                "It is a compile-time error to not include the built-in instance name in the redeclaration."));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

        }
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (BlockInstance)
    {
        slsDATA_TYPE *  dataType;

        status = slsDATA_TYPE_Clone(Compiler,
                                    Block->dataType->qualifiers.storage,
                                    Block->dataType->qualifiers.precision,
                                    Block->dataType,
                                    &dataType);

        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
        /* create the block instance */
        status = sloCOMPILER_CreateName(Compiler,
                                        BlockInstance->lineNo,
                                        BlockInstance->stringNo,
                                        slvVARIABLE_NAME,
                                        dataType,
                                        BlockInstance->u.identifier,
                                        Block->extension,
                                        gcvTRUE,
                                        &instance);
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        if (ArrayLengthExpr)
        {
            status = _ParseArrayLengthDataType(Compiler,
                                               instance->dataType,
                                               ArrayLengthExpr,
                                               gcvNULL,
                                               -1,
                                               CheckArrayLength,
                                               &instance->dataType);
            if (gcmIS_ERROR(status))
            {
               gcmFOOTER_ARG("<return>=%s", "<nil>");
               return gcvNULL;
            }
        }
        instance->u.variableInfo.interfaceBlock = Block;
    }

    /*
    ** transfer the field names to block's member list
    */
    FOR_EACH_DLINK_NODE(&(Block->dataType->fieldSpace->names), slsNAME, field)
    {
        if (BlockInstance == gcvNULL)
        {
           /* make field global scope */
           status = slsNAME_SPACE_CreateName(Compiler,
                                             Block->mySpace,
                                             field->lineNo,
                                             field->stringNo,
                                             slvVARIABLE_NAME,
                                             field->dataType,
                                             field->symbol,
                                             field->isBuiltIn,
                                             field->extension,
                                             gcvTRUE,
                                             &memberName);
           if (gcmIS_ERROR(status))
           {
              gcmFOOTER_ARG("<return>=%s", "<nil>");
              return gcvNULL;
           }
           memberName->u.variableInfo.interfaceBlock = Block;
        }
        else
        {
           memberName = field;
        }

        memberName->dataType->qualifiers.storage = qualifier;

        /* Update the qualifiers. */
        if (slsQUALIFIERS_HAS_FLAG(&Block->dataType->qualifiers, slvQUALIFIERS_FLAG_PATCH))
        {
            slsQUALIFIERS_SET_FLAG(&(memberName->dataType->qualifiers), slvQUALIFIERS_FLAG_PATCH);
        }

        if (addMember)
        {
            status = sloCOMPILER_Allocate(Compiler,
                                          (gctSIZE_T)sizeof(slsINTERFACE_BLOCK_MEMBER),
                                          (gctPOINTER *) &pointer);
            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }
            blockMember = pointer;
            blockMember->name = memberName;
            blockMember->isActive = gcvFALSE;
            if (qualifier == slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER)
            {
                blockMember->isActive = gcvFALSE;
            }
            else
            {
                blockMember->isActive = gcvTRUE;
            }

            slsDLINK_LIST_InsertLast(&Block->u.interfaceBlockContent.members, &blockMember->node);
        }
    }

    if (BlockInstance == gcvNULL)
    {
        Block->dataType->orgFieldSpace = gcvNULL;
        if (!slsDATA_TYPE_IsUnderlyingIOBlock(Block->dataType))
        {
            Block->dataType->fieldSpace = gcvNULL;
        }
    }

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

slsDATA_TYPE *
slParseInterfaceBlockMember(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE *DataType,
    IN slsFieldDecl *Member
    )
{
    gceSTATUS  status;
    gctINT i;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Member=0x%x",
                  Compiler, DataType, Member);

    if (DataType == gcvNULL || Member == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (slsDATA_TYPE_IsOpaque(DataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Member->field->lineNo,
                                        Member->field->stringNo,
                                        slvREPORT_ERROR,
                                        "opaque types are not allowed on a uniform block"));
    }

    if (slsDATA_TYPE_IsVoid(DataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Member->field->lineNo,
                                        Member->field->stringNo,
                                        slvREPORT_ERROR,
                                        "'%s' can not use the void type",
                                        Member->field->symbol));
    }

    if (slmDATA_TYPE_HasLayoutQualifier(DataType)) {
        if((slmDATA_TYPE_layoutId_GET(DataType) & sldLAYOUT_MEMORY_BIT_FIELDS) != slvLAYOUT_NONE) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Member->field->lineNo,
                                            Member->field->stringNo,
                                            slvREPORT_ERROR,
                                            "memory layout qualifier '%s' cannot be used on interface block member '%s'",
                                            _ParseGetLayoutIdName(slmDATA_TYPE_layoutId_GET(DataType) & sldLAYOUT_MEMORY_BIT_FIELDS),
                                            Member->field->symbol));
        }
        if((slmDATA_TYPE_layoutId_GET(DataType) & slvLAYOUT_BINDING) != slvLAYOUT_NONE) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Member->field->lineNo,
                                            Member->field->stringNo,
                                            slvREPORT_ERROR,
                                            "layout qualifier '%s' cannot be used on interface block member '%s'",
                                            _ParseGetLayoutIdName(slmDATA_TYPE_layoutId_GET(DataType) & slvLAYOUT_BINDING),
                                            Member->field->symbol));
        }
    }

    if (Member->arrayLength == 0)
    {
        Member->field->dataType = DataType;
    }
    else
    {
        if (Member->arrayLengthCount == 1)
        {
            status = sloCOMPILER_CreateArrayDataType(Compiler,
                                                     DataType,
                                                     Member->arrayLength,
                                                     &Member->field->dataType);

            if (gcmIS_ERROR(status))
            {
                Member->field->dataType = DataType;
            }
        }
        else
        {
            gcmASSERT(Member->arrayLengthCount > 1 && Member->arrayLengthList != gcvNULL);

            status = sloCOMPILER_CreateArraysOfArraysDataType(Compiler,
                                                              DataType,
                                                              Member->arrayLengthCount,
                                                              Member->arrayLengthList,
                                                              gcvFALSE,
                                                              &Member->field->dataType);

            if (gcmIS_ERROR(status))
            {
                Member->field->dataType = DataType;
            }
        }
    }

    for (i = 0; i < Member->field->dataType->arrayLengthCount; i++)
    {
        if (Member->field->dataType->arrayLengthList[i] == -1)
        {
            if (sloCOMPILER_IsOGLVersion(Compiler))
                Member->field->dataType->isImplicitlySizedArray = gcvTRUE;
            else
                Member->field->dataType->isInheritFromUnsizedDataType = gcvTRUE;
            break;
        }
    }

    gcmFOOTER_ARG("<return>=0x%x", DataType);
    return DataType;
}

void
slParseInterfaceBlockDeclBegin(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BlockType,
    IN slsLexToken * BlockName
    )
{
    gceSTATUS       status;
    slsNAME_SPACE * nameSpace = gcvNULL;
    sleSHADER_TYPE  shaderType;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x", Compiler);

    if(BlockType)
    {
        status = _CheckQualifiers(Compiler, BlockType);
        if (gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return;
        }
    }

    shaderType = Compiler->shaderType;

    if(BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT &&
       shaderType == slvSHADER_TYPE_FRAGMENT)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "Interface block cannot be used in fragment Shader output."));
        gcmFOOTER_NO();
        return;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "This GLSL version can't support interface block."));
        gcmFOOTER_NO();
        return;
    }

    extension.extension1 = slvEXTENSION1_IO_BLOCKS;
    if (BlockType->u.qualifiers.storage != slvSTORAGE_QUALIFIER_BUFFER &&
        BlockType->u.qualifiers.storage != slvSTORAGE_QUALIFIER_UNIFORM &&
        ((BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN ||
          BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT) &&
          !sloCOMPILER_ExtensionEnabled(Compiler, &extension)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "Unrecognizable interface block qualifier -",
                                        " GL_EXT_shader_io_blocks may need to be enabled."));
    }

    if (BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM)
    {
        BlockType->type = T_UNIFORM_BLOCK;
    }
    else if (BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_BUFFER)
    {
        BlockType->type = T_BUFFER;
    }
    else
    {
        gcmASSERT(BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN ||
                  BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT);

        BlockType->type = T_IO_BLOCK;
    }

    status = sloCOMPILER_CreateAuxGlobalNameSpace(Compiler,
                                                  BlockName->u.identifier,
                                                  slvNAME_SPACE_TYPE_IO_BLOCK,
                                                  &nameSpace);
    if (gcmIS_ERROR(status))  { gcmFOOTER_NO(); return; }

    /* Check if it is the redeclaration of gl_PerVertex. */
    if (gcoOS_StrNCmp(BlockName->u.identifier, "gl_PerVertex", 12) == gcvSTATUS_OK)
    {
        if(shaderType == slvSHADER_TYPE_FRAGMENT)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockType->lineNo,
                                            BlockType->stringNo,
                                            slvREPORT_ERROR,
                                            "The gl_PerVertex interface block cannot be redeclared in fragment Shaders."));
            gcmFOOTER_NO();
            return;
        }
        if(shaderType == slvSHADER_TYPE_VERTEX &&
           BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockType->lineNo,
                                            BlockType->stringNo,
                                            slvREPORT_ERROR,
                                            "The gl_PerVertex input interface block cannot be redeclared in vertex Shaders."));
            gcmFOOTER_NO();
            return;
        }
        Compiler->context.redeclareBuiltInVar = gcvTRUE;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<INTERFACE_BLOCK_DECL>"));
    gcmFOOTER_NO();
    return;
}

slsNAME *
slParseInterfaceBlockDeclEnd(
    IN sloCOMPILER Compiler,
    IN slsLexToken * BlockType,
    IN slsLexToken * BlockName
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;
    slsNAME *field, *lastField;
    slsNAME *name = gcvNULL;
    slsNAME_SPACE * prevNameSpace = gcvNULL;
    gctBOOL hasError = gcvFALSE;
    sltSTORAGE_QUALIFIER storageQualifier;
    slsLAYOUT_QUALIFIER defaultLayout[1];

    gcmHEADER_ARG("Compiler=0x%x BlockType=0x%x BlockName=0x%x",
                  Compiler, BlockType, BlockName);

    status = sloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if(!BlockName || !BlockType) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (BlockName &&
        sloCOMPILER_CheckErrorLog(Compiler, BlockName->lineNo, BlockName->stringNo))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    switch(BlockType->type) {
    case T_UNIFORM_BLOCK:
        storageQualifier = slvSTORAGE_QUALIFIER_UNIFORM;
        break;

    case T_BUFFER:
        storageQualifier = slvSTORAGE_QUALIFIER_BUFFER;
        break;

    case T_IO_BLOCK:
        {
            if (BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_IN)
            {
                storageQualifier = slvSTORAGE_QUALIFIER_IN_IO_BLOCK;
            }
            else
            {
                gcmASSERT(BlockType->u.qualifiers.storage == slvSTORAGE_QUALIFIER_OUT);
                storageQualifier = slvSTORAGE_QUALIFIER_OUT_IO_BLOCK;
            }
        }
        break;

    default:
        gcmASSERT(0);
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "unrecognizable interface block qualifier"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_CreateDataType(Compiler,
                                        BlockType->type,
                                        prevNameSpace,
                                        &dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* Save storage qualifier for IO block. */
    if (BlockType->type == T_IO_BLOCK)
    {
        dataType->qualifiers.storage = storageQualifier;
    }

    if (BlockType->u.qualifiers.memoryAccess != 0)
    {
        dataType->qualifiers.memoryAccess = BlockType->u.qualifiers.memoryAccess;
    }

    /* Only IOBlock can have location layout. */
    if (BlockType->u.qualifiers.layout.id & slvLAYOUT_LOCATION &&
        BlockType->type != T_IO_BLOCK)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "location id not applicable for interface block"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /* IOBlock can only have location layout. */
    if (BlockType->type == T_IO_BLOCK &&
        BlockType->u.qualifiers.layout.id & ~slvLAYOUT_LOCATION)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "IO block can only accept location layout. "));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if(storageQualifier == slvSTORAGE_QUALIFIER_UNIFORM &&
        (BlockType->u.qualifiers.layout.id & slvLAYOUT_STD430)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        BlockType->lineNo,
                                        BlockType->stringNo,
                                        slvREPORT_ERROR,
                                        "memory layout qualifier id std430 is not allowed for uniform block"));
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if(BlockType->u.qualifiers.layout.id & slvLAYOUT_BINDING) {
        if (storageQualifier == slvSTORAGE_QUALIFIER_BUFFER &&
            BlockType->u.qualifiers.layout.binding >= GetGLMaxShaderStorageBufferBindings())
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockType->lineNo,
                                            BlockType->stringNo,
                                            slvREPORT_ERROR,
                                            "buffer binding is too large"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    dataType->qualifiers.layout = BlockType->u.qualifiers.layout;
    dataType->qualifiers.flags = BlockType->u.qualifiers.flags;
    status = sloCOMPILER_GetDefaultLayout(Compiler,
                                            defaultLayout,
                                            storageQualifier);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_MergeLayoutId(Compiler,
                                       defaultLayout,
                                       &dataType->qualifiers.layout);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    if (BlockName != gcvNULL)
    {
        gctBOOL atGlobalNameSpace;
        sloEXTENSION extension = {0};

        gcmVERIFY_OK(sloCOMPILER_AtGlobalNameSpace(Compiler, &atGlobalNameSpace));
        if (!atGlobalNameSpace)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockName->lineNo,
                                            BlockName->stringNo,
                                            slvREPORT_ERROR,
                                            "%s block name '%s' must be defined in global space",
                                            storageQualifier == slvSTORAGE_QUALIFIER_UNIFORM ? "uniform" : "storage",
                                            BlockName->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        /*
        ** It is a compile-time error to use the same block name for more than one block
        ** declaration in the same shader interface within one shader.
        ** But a block name is allowed to have different definitions in different shader interfaces within the same shader.
        */
        status = slsNAME_SPACE_Search(Compiler,
                                      sloCOMPILER_GetCurrentSpace(Compiler),
                                      BlockName->u.identifier,
                                      slsNAME_SPACE_CheckBlockNameForTheSameInterface,
                                      dataType,
                                      gcvFALSE,
                                      gcvFALSE,
                                      &name);
        if (status == gcvSTATUS_OK)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            BlockType->lineNo,
                                            BlockType->stringNo,
                                            slvREPORT_ERROR,
                                            "Redeclare block \"%s\"",
                                            BlockName->u.identifier));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }

        /*
        ** If redeclare built in block, check the block members, it is allowed to delete block member only.
        */
        if (Compiler->context.redeclareBuiltInVar)
        {
            slsNAME *fieldName1, *fieldName2;
            gctBOOL blockMatched = gcvTRUE;

            status = slsNAME_SPACE_Search(Compiler,
                                          sloCOMPILER_GetBuiltInSpace(Compiler),
                                          BlockName->u.identifier,
                                          slsNAME_SPACE_CheckBlockNameForTheSameInterface,
                                          dataType,
                                          gcvFALSE,
                                          gcvFALSE,
                                          &name);
            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            for (fieldName1 = (slsNAME *)(&dataType->fieldSpace->names)->next;
                 (slsDLINK_NODE *)(fieldName1) != (&dataType->fieldSpace->names);
                 fieldName1 = (slsNAME *)(((slsDLINK_NODE *)fieldName1)->next))
            {
                gctBOOL fieldMatched = gcvFALSE;
                slsINTERFACE_BLOCK_MEMBER *blockMemberInfo;

                FOR_EACH_DLINK_NODE(&name->u.interfaceBlockContent.members, slsINTERFACE_BLOCK_MEMBER, blockMemberInfo)
                {
                    fieldName2 = blockMemberInfo->name;
                    if ((fieldName1->symbol == fieldName2->symbol) &&
                        slsDATA_TYPE_IsEqual(fieldName1->dataType, fieldName2->dataType))
                    {
                        fieldMatched = gcvTRUE;
                        break;
                    }
                }

                blockMatched &= fieldMatched;

                if (!blockMatched)
                {
                    break;
                }
            }

            if (!blockMatched)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                BlockType->lineNo,
                                                BlockType->stringNo,
                                                slvREPORT_ERROR,
                                                "only \"%s\" members are allowed in the redeclaration",
                                                BlockName->u.identifier));
                gcmFOOTER_ARG("<return>=%s", "<nil>");
                return gcvNULL;
            }

            gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                          slvDUMP_PARSER,
                                          "</INTERFACE_BLOCK_DECL>"));

            gcmFOOTER_ARG("<return>=0x%x", name);
            return name;
        }

        extension.extension1 = slvEXTENSION1_NONE;
        status = sloCOMPILER_CreateName(Compiler,
                                        BlockName->lineNo,
                                        BlockName->stringNo,
                                        slvINTERFACE_BLOCK_NAME,
                                        dataType,
                                        BlockName->u.identifier,
                                        extension,
                                        gcvFALSE,
                                        &name);

        if (gcmIS_ERROR(status)) {
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return gcvNULL;
        }
    }

    lastField = slsDLINK_LIST_Last(&(prevNameSpace->names), slsNAME);

    /*
    ** check it there is a embedded structure definition
    ** and/or implicitly sized array
    */
    FOR_EACH_DLINK_NODE(&(prevNameSpace->names), slsNAME, field)
    {
        gctBOOL setLocation = gcvFALSE;
        gctINT location = 0;
        sleSHADER_TYPE shaderType = Compiler->shaderType;

        /* double input must have "flat" qualifier. */
        if (slmIsElementTypeDouble(field->dataType->elementType) &&
            field->dataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
            shaderType == slvSHADER_TYPE_FRAGMENT &&
            dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "double-precision floating-point typed input '%s' has to have flat interpolation qualifier",
                                            field->symbol));
            hasError = gcvTRUE;
        }

        /* signed or unsigned interger or integer vectors input must have "flat" qualifier. */
        if ((slmIsElementTypeSigned(field->dataType->elementType) ||
            slmIsElementTypeUnsigned(field->dataType->elementType)) &&
            field->dataType->qualifiers.interpolation != slvINTERPOLATION_QUALIFIER_FLAT &&
            shaderType == slvSHADER_TYPE_FRAGMENT &&
            dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK &&
            sloCOMPILER_IsOGLVersion(Compiler))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "integer typed input '%s' has to have flat interpolation qualifier",
                                            field->symbol));
            hasError = gcvTRUE;
        }

        if (slsDATA_TYPE_IsArray(field->dataType))
        {
            gctINT i;
            gctBOOL implicitArraySize = gcvFALSE;

            for (i = 0; i < field->dataType->arrayLengthCount; i++)
            {
                if (field->dataType->arrayLengthList[i] == -1)
                {
                    implicitArraySize = gcvTRUE;
                    break;
                }
            }

            if (implicitArraySize && !sloCOMPILER_IsOGLVersion(Compiler))
            {
                /* For storage block, last element may be an array that is not sized. */
                if (storageQualifier == slvSTORAGE_QUALIFIER_BUFFER)
                {
                    if (field != lastField)
                    {
                        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                        field->lineNo,
                                                        field->stringNo,
                                                        slvREPORT_ERROR,
                                                        "Implicitly sized array not allowed"));
                        hasError = gcvTRUE;
                    }
                }
                /* For uniform block, implicitly sized array is not allowed. */
                else
                {
                    gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                                    field->lineNo,
                                                    field->stringNo,
                                                    slvREPORT_ERROR,
                                                    "Implicitly sized array not allowed"));
                    hasError = gcvTRUE;
                }
            }
        }

        /* Can't have a embedded structure definition. */
        if (field->type == slvSTRUCT_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "Embedded structure definitions are not supported"));
            hasError = gcvTRUE;
        }
        /* If the field is a struct, we need to duplicate its field space. */
        if (field->dataType->elementType == slvTYPE_STRUCT)
        {
            status = sloCOMPILER_DuplicateFieldSpaceForDataType(Compiler,
                                                                (dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK),
                                                                field->dataType
                                                                );
            if (gcmIS_ERROR(status)) {
               gcmFOOTER_ARG("<return>=%s", "<nil>");
               return gcvNULL;
            }
        }

        if (field->dataType->qualifiers.storage == slvSTORAGE_QUALIFIER_NONE)
        {
           field->dataType->qualifiers.storage = storageQualifier;
        }
        else if (field->dataType->qualifiers.storage != BlockType->u.qualifiers.storage)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "member is not of same storage type \'%s\'",
                                            slGetStorageQualifierName(Compiler, BlockType->u.qualifiers.storage)));
            hasError = gcvTRUE;
        }
        setLocation = field->dataType->qualifiers.layout.id & slvLAYOUT_LOCATION;
        location = field->dataType->qualifiers.layout.location;
        /* inherit layout qualifier from block specification */
        status = sloCOMPILER_MergeLayoutId(Compiler,
                                           &dataType->qualifiers.layout,
                                           &field->dataType->qualifiers.layout);
        if (gcmIS_ERROR(status)) {
           gcmFOOTER_ARG("<return>=%s", "<nil>");
           return gcvNULL;
        }

        if (!setLocation)
        {
            field->dataType->qualifiers.layout.id &= ~slvLAYOUT_LOCATION;
        }
        field->dataType->qualifiers.layout.location = location;

        /* inherit memory access qualifier from block specification */
        field->dataType->qualifiers.memoryAccess |= dataType->qualifiers.memoryAccess;
    }

    if(hasError) {
       gcmFOOTER_ARG("<return>=%s", "<nil>");
       return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "</INTERFACE_BLOCK_DECL>"));

    gcmFOOTER_ARG("<return>=0x%x", name);
    return name;
}

/*klc - to be completed */
sloIR_BASE
slParseStructVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * TypeQualifier,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR  ConstExpr
    )
{
    gceSTATUS       status;
    slsDATA_TYPE *  dataType;
    slsNAME *       field;
    slsNAME_SPACE * prevNameSpace = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x TypeQualifier=0x%x"
                  " DataType=0x%x Identifier=0x%x ConstExpr=0x%x",
                  Compiler, TypeQualifier, DataType, Identifier, ConstExpr);

    status = sloCOMPILER_PopCurrentNameSpace(Compiler, &prevNameSpace);
    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    /*
    ** check it there is a embedded structure definition
    */
    FOR_EACH_DLINK_NODE(&(prevNameSpace->names), slsNAME, field)
    {
        if (field->type == slvSTRUCT_NAME)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            field->lineNo,
                                            field->stringNo,
                                            slvREPORT_ERROR,
                                            "Embedded structure definitions are not supported"));
            return gcvNULL;
        }
    }

    status = sloCOMPILER_CreateDataType(
                                        Compiler,
                                        T_STRUCT,
                                        prevNameSpace,
                                        &dataType);

    if (gcmIS_ERROR(status)) {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_PARSER,
                                "<STRUCT_VARIABLE_DECL line=\"%d\" string=\"%d\" name=\"%s\" />",
                                Identifier->lineNo,
                                Identifier->stringNo,
                                Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

void
slParseTypeSpecifiedFieldDeclList(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsDLINK_LIST * FieldDeclList
    )
{
    gceSTATUS       status;
    slsFieldDecl *  fieldDecl;
    gctINT i;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x FieldDeclList=0x%x",
                  Compiler, DataType, FieldDeclList);

    if (DataType == gcvNULL || FieldDeclList == gcvNULL)
    {
        gcmFOOTER_NO();
        return;
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler) && slsDATA_TYPE_IsArray(DataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "invalid forming of array type from '%s'",
                                        _GetTypeName(DataType->type)));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if(slsQUALIFIERS_KIND_ISNOT(&DataType->qualifiers, slvQUALIFIERS_FLAG_PRECISION) &&
       slsQUALIFIERS_KIND_ISNOT(&DataType->qualifiers, slvQUALIFIERS_FLAG_NONE)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        "type qualifier other than precision qualifier is specified for a field declaration."));
    }

    for (i = 0; i < DataType->arrayLengthCount; i++)
    {
        if (DataType->arrayLengthList[i] == -1)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            sloCOMPILER_GetCurrentLineNo(Compiler),
                                            sloCOMPILER_GetCurrentStringNo(Compiler),
                                            slvREPORT_ERROR,
                                            " Can't declare variable without size"));

            status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
            gcmONERROR(status);
        }
    }

    FOR_EACH_DLINK_NODE(FieldDeclList, slsFieldDecl, fieldDecl)
    {
        if (slsDATA_TYPE_IsVoid(DataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            fieldDecl->field->lineNo,
                                            fieldDecl->field->stringNo,
                                            slvREPORT_ERROR,
                                            "'%s' can not use the void type",
                                            fieldDecl->field->symbol));

            break;
        }

        if (slsDATA_TYPE_IsAtomic(DataType))
        {
            gcmVERIFY_OK(sloCOMPILER_Report(
                                            Compiler,
                                            fieldDecl->field->lineNo,
                                            fieldDecl->field->stringNo,
                                            slvREPORT_ERROR,
                                            "atomic_uint field '%s' can not be contained in struct",
                                            fieldDecl->field->symbol));

            break;
        }

        if (fieldDecl->arrayLength == 0)
        {
            fieldDecl->field->dataType = DataType;
        }
        else
        {
            if (fieldDecl->arrayLengthCount == 1)
            {
                status = sloCOMPILER_CreateArrayDataType(
                                                        Compiler,
                                                        DataType,
                                                        fieldDecl->arrayLength,
                                                        &fieldDecl->field->dataType);

                if (gcmIS_ERROR(status))
                {
                    fieldDecl->field->dataType = DataType;
                    break;
                }
            }
            else
            {
                gcmASSERT(fieldDecl->arrayLengthCount > 1 && fieldDecl->arrayLengthList != gcvNULL);

                status = sloCOMPILER_CreateArraysOfArraysDataType(Compiler,
                                                                  DataType,
                                                                  fieldDecl->arrayLengthCount,
                                                                  fieldDecl->arrayLengthList,
                                                                  gcvFALSE,
                                                                  &fieldDecl->field->dataType);

                if (gcmIS_ERROR(status))
                {
                    fieldDecl->field->dataType = DataType;
                    break;
                }
            }
        }
    }

OnError:
    while (!slsDLINK_LIST_IsEmpty(FieldDeclList))
    {
        slsDLINK_LIST_DetachFirst(FieldDeclList, slsFieldDecl, &fieldDecl);

        if (fieldDecl->arrayLengthCount > 1 && fieldDecl->arrayLengthList != gcvNULL)
        {
            gcmVERIFY_OK(sloCOMPILER_Free(Compiler, fieldDecl->arrayLengthList));
        }

        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, fieldDecl));
    }

    gcmVERIFY_OK(sloCOMPILER_Free(Compiler, FieldDeclList));

    gcmFOOTER_NO();
}

slsDLINK_LIST *
slParseArrayLengthList(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR ArrayLengthExpr1,
    IN sloIR_EXPR ArrayLengthExpr2
    )
{
    gceSTATUS status;
    gctINT arrayLength;
    slsDLINK_LIST * lengthList;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x, LengthList=0x%x, ArrayLengthExpr1=0x%x, ArrayLengthExpr2=0x%x",
                  Compiler, LengthList, ArrayLengthExpr1, ArrayLengthExpr1);

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        sloCOMPILER_GetCurrentLineNo(Compiler),
                                        sloCOMPILER_GetCurrentStringNo(Compiler),
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (ArrayLengthExpr1 == gcvNULL)
    {
        gcmONERROR(_CreateArrayLengthExpr(Compiler, -1, &ArrayLengthExpr1));
    }

    gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                          ArrayLengthExpr1,
                                          gcvFALSE,
                                          gcvFALSE,
                                          &arrayLength));

    if (LengthList == gcvNULL)
    {
        if (ArrayLengthExpr2 == gcvNULL)
        {
            gcmONERROR(_CreateArrayLengthExpr(Compiler, -1, &ArrayLengthExpr2));
        }

        gcmONERROR(_EvaluateExprToArrayLength(Compiler,
                                              ArrayLengthExpr2,
                                              gcvFALSE,
                                              gcvFALSE,
                                              &arrayLength));
    }

    /* If this list is empty, create a list and insert this EXPR to this list. */
    if (LengthList == gcvNULL)
    {
        gcmONERROR(sloCOMPILER_Allocate(
                                    Compiler,
                                    (gctSIZE_T)sizeof(slsDLINK_LIST),
                                    &pointer));

        gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(slsDLINK_LIST));
        lengthList = pointer;
        slsDLINK_LIST_Initialize(lengthList);
    }
    else
    {
        lengthList = LengthList;
    }

    slsDLINK_NODE_InsertNext(lengthList, &(ArrayLengthExpr1->base.node));

    if (ArrayLengthExpr2 != gcvNULL)
    {
        slsDLINK_NODE_InsertNext(lengthList, &(ArrayLengthExpr2->base.node));
    }

    if (LengthList == gcvNULL)
    {
        LengthList = lengthList;
    }

    gcmFOOTER_ARG("<return>=0x%x", LengthList);
    return LengthList;

OnError:
    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

slsDLINK_LIST *
slParseArrayLengthList2(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR ArrayLengthExpr1,
    IN sloIR_EXPR ArrayLengthExpr2
    )
{
    gceSTATUS status;


    gcmHEADER_ARG("Compiler=0x%x, LengthList=0x%x, ArrayLengthExpr1=0x%x, ArrayLengthExpr2=0x%x",
                  Compiler, LengthList, ArrayLengthExpr1, ArrayLengthExpr1);

    if (LengthList == gcvNULL)
    {
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }
    else
    {
        LengthList = slParseArrayLengthList(Compiler,
                                            LengthList,
                                            ArrayLengthExpr1,
                                            ArrayLengthExpr2);
    }

    gcmFOOTER_ARG("<return>=0x%x", LengthList);
    return LengthList;

OnError:
    gcmFOOTER_ARG("<return>=%s", "<nil>");
    return gcvNULL;
}

slsNAME    *
slParseArrayListParameterDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    )
{
    gceSTATUS status;
    slsDATA_TYPE * arrayDataType;
    slsNAME * name = gcvNULL;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DataType, Identifier, LengthList);

    gcmASSERT(Identifier);

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (DataType == gcvNULL || LengthList == gcvNULL)
    {
        if (LengthList == gcvNULL)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "unspecified array size in variable declaration"));
        }
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (!sloCOMPILER_IsHaltiVersion(Compiler) && slsDATA_TYPE_IsArray(DataType)) {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "invalid forming of array type from '%s'",
                                        _GetTypeName(DataType->type)));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }
    if (gcmIS_ERROR(_CheckErrorForArraysOfArraysLengthValue(Compiler, LengthList, gcvFALSE)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " Can't declare array \"%s\" without size",
                                        Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    gcmONERROR(_ParseArraysOfArraysDataType(Compiler,
                                       DataType,
                                       LengthList,
                                       gcvTRUE,
                                       &arrayDataType));

    extension.extension1 = slvEXTENSION1_NONE;
    gcmONERROR(sloCOMPILER_CreateName(Compiler,
                                     (Identifier != gcvNULL)? Identifier->lineNo : 0,
                                     (Identifier != gcvNULL)? Identifier->stringNo : 0,
                                     slvPARAMETER_NAME,
                                     arrayDataType,
                                     (Identifier != gcvNULL)? Identifier->u.identifier : "",
                                     extension,
                                     gcvTRUE,
                                     &name));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                                  slvDUMP_PARSER,
                                  "<PARAMETER_DECL dataType=\"0x%x\" name=\"%s\" />",
                                  DataType,
                                  (Identifier != gcvNULL)? Identifier->u.identifier : ""));

OnError:
    gcmFOOTER_ARG("<return>=0x%x", name);
    return name;
}

slsDeclOrDeclList
slParseArrayListVariableDecl2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    )
{
    gceSTATUS status;
    slsDeclOrDeclList   declOrDeclList;
    slsDATA_TYPE *arrayDataType;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, DeclOrDeclList, Identifier, LengthList);

    gcmASSERT(Identifier);

    declOrDeclList = DeclOrDeclList;

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (declOrDeclList.dataType == gcvNULL || LengthList == gcvNULL)
    {
        if (!LengthList)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "unspecified array size in variable declaration"));
        }
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, declOrDeclList.dataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    /* Vertex/Fragment input and vertex output cann't be an array of arrays. */
    gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                 Identifier,
                                 declOrDeclList.dataType));

    gcmONERROR(_ParseArraysOfArraysDataType(Compiler,
                                       declOrDeclList.dataType,
                                       LengthList,
                                       gcvTRUE,
                                       &arrayDataType));

    gcmONERROR(_ParseVariableDecl(Compiler,
                                arrayDataType,
                                Identifier));

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseArrayListVariableDecl(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList
    )
{
    gceSTATUS status;
    slsDeclOrDeclList   declOrDeclList;
    slsDATA_TYPE *arrayDataType;

    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x LengthList=0x%x",
                  Compiler, DataType, Identifier, LengthList);

    gcmASSERT(Identifier);

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    /* Only ES SL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (declOrDeclList.dataType == gcvNULL || LengthList == gcvNULL)
    {
        if (!LengthList)
        {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "unspecified array size in variable declaration"));
        }
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, gcvNULL));

    if (slsDATA_TYPE_IsArrayHasImplicitLength(Compiler, DataType))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable type declaration"));

        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CheckDataTypePrecision(Compiler,
                                     DataType,
                                     Identifier));

    /* Vertex/Fragment input and vertex output cann't be an array of arrays. */
    gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                 Identifier,
                                 DataType));

    gcmONERROR(_ParseUpdateHaltiQualifiers(Compiler,
                                         Identifier,
                                         gcvTRUE,
                                         DataType));

    gcmONERROR(_ParseArraysOfArraysDataType(Compiler,
                                       DataType,
                                       LengthList,
                                       gcvTRUE,
                                       &arrayDataType));

    if(arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_UNIFORM &&
       arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctSIZE_T length;

        length = (gctSIZE_T)slsDATA_TYPE_GetLogicalOperandCount(arrayDataType, gcvFALSE);

        status = sloCOMPILER_SetUniformLocationInUse(Compiler,
                                                     arrayDataType->qualifiers.layout.location,
                                                     length);
        if(gcmIS_ERROR(status)) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of uniforms beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    if(arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_ATTRIBUTE &&
       arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctINT i, length = 1;
        for(i = 0; i < arrayDataType->arrayLengthCount; i++)
        {
            length *= arrayDataType->arrayLengthList[i];
        }

        status = sloCOMPILER_SetInputLocationInUse(Compiler,
                                                   arrayDataType->qualifiers.layout.location,
                                                   length);
        if(gcmIS_ERROR(status)) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of uniforms beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    if(arrayDataType->qualifiers.storage == slvSTORAGE_QUALIFIER_FRAGMENT_OUT &&
       arrayDataType->qualifiers.layout.id & slvLAYOUT_LOCATION)
    {
        gctINT i, length = 1;
        for(i = 0; i < arrayDataType->arrayLengthCount; i++)
        {
            length *= arrayDataType->arrayLengthList[i];
        }

        status = sloCOMPILER_SetOutputLocationInUse(Compiler,
                                                    arrayDataType->qualifiers.layout.location,
                                                    length);
        if(gcmIS_ERROR(status)) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                            Identifier->lineNo,
                                            Identifier->stringNo,
                                            slvREPORT_ERROR,
                                            "# of fragment shader outputs beyond limit"));
            gcmFOOTER_ARG("<return>=%s", "<nil>");
            return declOrDeclList;
        }
    }

    gcmONERROR(_ParseVariableDecl(Compiler,
                                arrayDataType,
                                Identifier));

    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsFieldDecl *
slParseFieldListDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN gctBOOL IsBlockMember
    )
{
    gceSTATUS       status;
    slsNAME *       field;
    slsFieldDecl *  fieldDecl = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    sloIR_EXPR operand;
    gctINT arrayLengthCount = 0, i = 0;
    gctINT * arrayLengthList = gcvNULL;
    slsDLINK_LIST * list;
    sloEXTENSION extension = {0};

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, Identifier, LengthList);

    gcmASSERT(Identifier);

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (LengthList == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable declaration"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (gcmIS_ERROR(_CheckErrorForArraysOfArraysLengthValue(Compiler, LengthList, IsBlockMember)))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " Can't declare array \"%s\" without size",
                                        Identifier->u.identifier));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvFIELD_NAME,
                                    gcvNULL,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &field);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_Allocate(
                                Compiler,
                                (gctSIZE_T)sizeof(slsFieldDecl),
                                &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(slsFieldDecl));
    fieldDecl = pointer;

    fieldDecl->field    = field;

    slsDLINK_NODE_COUNT(LengthList, arrayLengthCount);

    gcmASSERT(arrayLengthCount > 1);

    gcmONERROR(sloCOMPILER_Allocate(
                                Compiler,
                                (gctSIZE_T)(arrayLengthCount * gcmSIZEOF(gctINT)),
                                &pointer));

    gcoOS_ZeroMemory(pointer, arrayLengthCount * gcmSIZEOF(gctINT));
    fieldDecl->arrayLengthList = pointer;
    fieldDecl->arrayLengthCount = arrayLengthCount;
    fieldDecl->arrayLength = arrayLengthCount;
    arrayLengthList = pointer;

    FOR_EACH_DLINK_NODE_REVERSELY(LengthList, struct _sloIR_EXPR, operand)
    {
        _EvaluateExprToArrayLength(Compiler,
                                   operand,
                                   gcvFALSE,
                                   IsBlockMember ? gcvFALSE : gcvTRUE,
                                   &arrayLengthList[i]);
        i++;
    }

    gcmASSERT(i == arrayLengthCount);

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                               slvDUMP_PARSER,
                               "<FIELD line=\"%d\" string=\"%d\" name=\"%s\" />",
                               Identifier->lineNo,
                               Identifier->stringNo,
                               Identifier->u.identifier));

OnError:

    if (LengthList != gcvNULL)
    {
        list = LengthList->next;
        for (i = 0; i < arrayLengthCount; i++)
        {
            operand = (sloIR_EXPR)list;
            list = list->next;
            gcmVERIFY_OK(sloIR_OBJECT_Destroy(Compiler, &operand->base));
        }

        gcmVERIFY_OK(sloCOMPILER_Free(Compiler, LengthList));
    }

    gcmFOOTER_ARG("<return>=0x%x", fieldDecl);
    return fieldDecl;
}

slsDeclOrDeclList
slParseArrayListVariableDeclWithInitializer(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR Initializer
    )
{
    slsDeclOrDeclList declOrDeclList;
    gceSTATUS status = gcvSTATUS_OK;
    slsDATA_TYPE * arrayDataType = gcvNULL;
    sloIR_EXPR initExpr = gcvNULL;
    gcmHEADER_ARG("Compiler=0x%x DataType=0x%x Identifier=0x%x "
                  "LengthList=0x%x Initializer=0x%x",
                  Compiler, DataType, Identifier, LengthList, Initializer);

    gcmASSERT(Identifier);

    declOrDeclList.dataType         = DataType;
    declOrDeclList.initStatement    = gcvNULL;
    declOrDeclList.initStatements   = gcvNULL;

    if (declOrDeclList.dataType == gcvNULL)
    {
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, DataType, Initializer));

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (Initializer == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " Initializer is illegal."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (LengthList == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable declaration"));
        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    /* Vertex/Fragment input and vertex output cann't be an array of arrays. */
    gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                 Identifier,
                                 DataType));

    gcmONERROR(_ParseUpdateHaltiQualifiers(Compiler,
                                         Identifier,
                                         gcvTRUE,
                                         DataType));

    gcmONERROR(_ParseArraysOfArraysDataType(Compiler,
                                       DataType,
                                       LengthList,
                                       gcvFALSE,
                                       &arrayDataType));

    gcmONERROR(_UpdateDataTypeForArraysOfArraysInitializer(Compiler, arrayDataType, Initializer->dataType));

    gcmONERROR(_ParseVariableDeclWithInitializer(Compiler,
                                                 arrayDataType,
                                                 Identifier,
                                                 Initializer,
                                                 gcvFALSE,
                                                 &initExpr));

    declOrDeclList.initStatement = &initExpr->base;

OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDeclOrDeclList
slParseArrayListVariableDeclWithInitializer2(
    IN sloCOMPILER Compiler,
    IN slsDeclOrDeclList DeclOrDeclList,
    IN slsLexToken * Identifier,
    IN slsDLINK_LIST * LengthList,
    IN sloIR_EXPR Initializer
    )
{
    slsDeclOrDeclList declOrDeclList;
    slsDATA_TYPE * dataType, *arrayDataType;
    gceSTATUS status = gcvSTATUS_OK;
    sloIR_EXPR initExpr = gcvNULL;
    sloIR_BASE initStatement = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x DeclOrDeclList=0x%x Identifier=0x%x "
                  "LengthList=0x%x Initializer=0x%x",
                  Compiler, DeclOrDeclList, Identifier, LengthList, Initializer);

    declOrDeclList = DeclOrDeclList;
    dataType = declOrDeclList.dataType;

    gcmONERROR(_CommonCheckForVariableDecl(Compiler, dataType, Initializer));

    /* Only GLSL31 can support arrays of arrays. */
    if (!slmIsLanguageVersion3_1(Compiler))
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " This GLSL version can't support arrays of arrays."));

        status = gcvSTATUS_COMPILER_FE_PARSER_ERROR;
        gcmONERROR(status);
    }

    if (Initializer == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
                                        Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        " Array size can't be before variable for arrays of arrays."));
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    if (LengthList == gcvNULL)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                        Identifier->lineNo,
                                        Identifier->stringNo,
                                        slvREPORT_ERROR,
                                        "unspecified array size in variable declaration"));
        gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
        return declOrDeclList;
    }

    /* Vertex/Fragment input and vertex output cann't be an array of arrays. */
    gcmONERROR(_CheckErrorForArraysOfArrays(Compiler,
                                 Identifier,
                                 dataType));

    gcmONERROR(_ParseUpdateHaltiQualifiers(Compiler,
                                         Identifier,
                                         gcvTRUE,
                                         dataType));

    gcmONERROR(_ParseArraysOfArraysDataType(Compiler,
                                       dataType,
                                       LengthList,
                                       gcvFALSE,
                                       &arrayDataType));

    gcmONERROR(_UpdateDataTypeForArraysOfArraysInitializer(Compiler, arrayDataType, Initializer->dataType));

    gcmONERROR(_ParseVariableDeclWithInitializer(Compiler,
                                                 arrayDataType,
                                                 Identifier,
                                                 Initializer,
                                                 gcvFALSE,
                                                 &initExpr));

    if (declOrDeclList.initStatement != gcvNULL)
    {
        gcmASSERT(declOrDeclList.initStatements == gcvNULL);

        status = sloIR_SET_Construct(Compiler,
                                     declOrDeclList.initStatement->lineNo,
                                     declOrDeclList.initStatement->stringNo,
                                     slvDECL_SET,
                                     &declOrDeclList.initStatements);

        if (gcmIS_ERROR(status)) {
            gcmASSERT(declOrDeclList.initStatements == gcvNULL);
            gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
            return declOrDeclList;
        }

        gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                         declOrDeclList.initStatements,
                                         declOrDeclList.initStatement));

        declOrDeclList.initStatement = gcvNULL;
    }

    initStatement = &initExpr->base;

    if (declOrDeclList.initStatements != gcvNULL)
    {
        gcmVERIFY_OK(sloIR_SET_AddMember(Compiler,
                                         declOrDeclList.initStatements,
                                         initStatement));
    }
    else
    {
        gcmASSERT(declOrDeclList.initStatement == gcvNULL);

        declOrDeclList.initStatement = initStatement;
    }


OnError:
    gcmFOOTER_ARG("<return>=0x%x", declOrDeclList);
    return declOrDeclList;
}

slsDLINK_LIST *
slParseFieldDeclList(
    IN sloCOMPILER Compiler,
    IN slsFieldDecl * FieldDecl
    )
{
    gceSTATUS       status;
    slsDLINK_LIST * fieldDeclList;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x FieldDecl=0x%x",
                  Compiler, FieldDecl);

    status = sloCOMPILER_Allocate(
                                Compiler,
                                (gctSIZE_T)sizeof(slsDLINK_LIST),
                                &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    fieldDeclList = pointer;

    slsDLINK_LIST_Initialize(fieldDeclList);

    if (FieldDecl != gcvNULL)
    {
        slsDLINK_LIST_InsertLast(fieldDeclList, &FieldDecl->node);
    }

    gcmFOOTER_ARG("<return>=0x%x", fieldDeclList);
    return fieldDeclList;
}

slsDLINK_LIST *
slParseFieldDeclList2(
    IN sloCOMPILER Compiler,
    IN slsDLINK_LIST * FieldDeclList,
    IN slsFieldDecl * FieldDecl
    )
{
    gcmHEADER_ARG("Compiler=0x%x FieldDeclList=0x%x FieldDecl=0x%x",
                  Compiler, FieldDeclList, FieldDecl);

    if (FieldDeclList != gcvNULL && FieldDecl != gcvNULL)
    {
        slsDLINK_LIST_InsertLast(FieldDeclList, &FieldDecl->node);
    }

    gcmFOOTER_ARG("<return>=0x%x", FieldDeclList);
    return FieldDeclList;
}

slsFieldDecl *
slParseFieldDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier,
    IN sloIR_EXPR ArrayLengthExpr
    )
{
    gceSTATUS       status;
    slsNAME *       field;
    slsFieldDecl *  fieldDecl;
    gctPOINTER pointer = gcvNULL;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x ArrayLengthExpr=0x%x",
                  Compiler, Identifier, ArrayLengthExpr);

    gcmASSERT(Identifier);

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvFIELD_NAME,
                                    gcvNULL,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &field);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_Allocate(
                                Compiler,
                                (gctSIZE_T)sizeof(slsFieldDecl),
                                &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(slsFieldDecl));
    fieldDecl = pointer;

    fieldDecl->field    = field;

    if (ArrayLengthExpr == gcvNULL)
    {
        fieldDecl->arrayLength  = 0;
    }
    else
    {
        status = _EvaluateExprToArrayLength(Compiler,
                                            ArrayLengthExpr,
                                            gcvTRUE,
                                            gcvTRUE,
                                            &fieldDecl->arrayLength);

        fieldDecl->arrayLengthCount = 1;

        if (gcmIS_ERROR(status))
        {
            gcmASSERT(fieldDecl->arrayLength == 0);

            gcmFOOTER_ARG("<return>=0x%x", fieldDecl);
            return fieldDecl;
        }
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                               slvDUMP_PARSER,
                               "<FIELD line=\"%d\" string=\"%d\" name=\"%s\" />",
                               Identifier->lineNo,
                               Identifier->stringNo,
                               Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=0x%x", fieldDecl);
    return fieldDecl;
}

slsFieldDecl *
slParseImplicitArraySizeFieldDecl(
    IN sloCOMPILER Compiler,
    IN slsLexToken * Identifier
    )
{
    gceSTATUS       status;
    slsNAME *       field;
    slsFieldDecl *  fieldDecl;
    gctPOINTER      pointer = gcvNULL;
    sloEXTENSION    extension = {0};

    gcmHEADER_ARG("Compiler=0x%x Identifier=0x%x",
                  Compiler, Identifier);

    gcmASSERT(Identifier);

    extension.extension1 = slvEXTENSION1_NONE;
    status = sloCOMPILER_CreateName(Compiler,
                                    Identifier->lineNo,
                                    Identifier->stringNo,
                                    slvFIELD_NAME,
                                    gcvNULL,
                                    Identifier->u.identifier,
                                    extension,
                                    gcvTRUE,
                                    &field);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    status = sloCOMPILER_Allocate(Compiler,
                                  (gctSIZE_T)sizeof(slsFieldDecl),
                                  &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_ARG("<return>=%s", "<nil>");
        return gcvNULL;
    }

    gcoOS_ZeroMemory(pointer, (gctSIZE_T)sizeof(slsFieldDecl));
    fieldDecl = pointer;

    fieldDecl->field    = field;

    fieldDecl->arrayLength  = -1;

    fieldDecl->arrayLengthCount = 1;

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                               slvDUMP_PARSER,
                               "<FIELD line=\"%d\" string=\"%d\" name=\"%s\" />",
                               Identifier->lineNo,
                               Identifier->stringNo,
                               Identifier->u.identifier));

    gcmFOOTER_ARG("<return>=0x%x", fieldDecl);
    return fieldDecl;
}
