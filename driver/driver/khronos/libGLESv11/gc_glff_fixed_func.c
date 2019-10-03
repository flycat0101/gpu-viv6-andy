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


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE gcdZONE_ES11_TRACE

/******************************************************************************
** util fuction: the same as gcUNIFORM_SetValue_Ex/gcUNIFORM_SetValueF_Ex,
** which will be use by VG/VX etc,
** figure out a individual function which only used by es11
********************************************************************************/
gceSTATUS
glfUtilUniformSetValue(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN gcsHINT_PTR Hints,
    IN const gctPOINTER Value
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows, physicalAddress;
    gctBOOL useFullNewLinker = gcUseFullNewLinker(gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_HALTI2));

    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Value);

    /* Verify the argiments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Value != gcvNULL);

    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);
    rows *= gcmMIN((gctINT) Count,
                   useFullNewLinker ? Uniform->usedArraySize : Uniform->arraySize);

    physicalAddress = Uniform->address;
    if (useFullNewLinker)
    {
        gcSHADER_ComputeUniformPhysicalAddress(Hints->hwConstRegBases,
                                               Uniform,
                                               &physicalAddress);
    }

    status = gcoSHADER_BindUniform(gcvNULL,
                                   physicalAddress, GetUniformPhysical(Uniform),
                                   columns, rows,
                                   1, gcvFALSE,
                                   columns * 4,
                                   0,
                                   (gctPOINTER) Value,
                                   gcvUNIFORMCVT_NONE,
                                   Uniform->shaderKind);

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

/*******************************************************************************
** Shader generation helpers.
*/

gceSTATUS glfSetUniformFromFractions(
    IN gcUNIFORM Uniform,
    IN gcsHINT_PTR Hints,
    IN GLfloat X,
    IN GLfloat Y,
    IN GLfloat Z,
    IN GLfloat W
    )
{
    GLfloat vector[4];
    gceSTATUS status;
    gcmHEADER_ARG("Uniform=0x%x X=%f Y=%f Z=%f W=%f", Uniform, X, Y, Z, W);

    vector[0] = X;
    vector[1] = Y;
    vector[2] = Z;
    vector[3] = W;

    status = glfUtilUniformSetValue(Uniform, 1, Hints, (gctPOINTER)vector);

    gcmFOOTER();

    return status;
}

gceSTATUS glfSetUniformFromFloats(
    IN gcUNIFORM Uniform,
    IN gcsHINT_PTR Hints,
    IN GLfloat* FloatX,
    IN GLfloat* FloatY,
    IN GLfloat* FloatZ,
    IN GLfloat* FloatW,
    IN GLfloat* ValueArray,
    IN gctUINT Count
    )
{
    gctUINT i;
    GLfloat* vector = ValueArray;
    gceSTATUS status;

    gcmHEADER_ARG("Uniform=0x%x MutantX=0x%x MutantY=0x%x MutantZ=0x%x MutantW=0x%x "
                    "ValueArray=0x%x Count=%d",
                    Uniform, FloatX, FloatY, FloatZ, FloatW, ValueArray, Count);

    for (i = 0; i < Count; i++)
    {
        if (FloatX != gcvNULL)
        {
            *vector = FloatX[i];
            vector++;
        }

        if (FloatY != gcvNULL)
        {
            *vector = FloatY[i];
            vector++;
        }

        if (FloatZ != gcvNULL)
        {
            *vector = FloatZ[i];
            vector++;
        }

        if (FloatW != gcvNULL)
        {
            *vector = FloatW[i];
            vector++;
        }
    }

    status = glfUtilUniformSetValue(Uniform, Count, Hints, (gctPOINTER)ValueArray);

    gcmFOOTER();

    return status;
}

gceSTATUS glfSetUniformFromVectors(
    IN gcUNIFORM Uniform,
    IN gcsHINT_PTR Hints,
    IN glsVECTOR_PTR Vector,
    IN GLfloat* ValueArray,
    IN gctUINT Count
    )
{
    gctUINT i;
    GLfloat* vector = ValueArray;
    gceSTATUS status;
    gcmHEADER_ARG("Uniform=0x%x Vector=0x%x ValueArray=0x%x Count=%d",
                    Uniform, Vector, ValueArray, Count);

    for (i = 0; i < Count; i++)
    {
        glfGetFloatFromVector4(
            &Vector[i],
            vector
            );

        vector += 4;
    }

    status = glfUtilUniformSetValue(Uniform, Count, Hints, (gctPOINTER)ValueArray);

    gcmFOOTER();

    return status;
}

gceSTATUS glfSetUniformFromMatrix(
    IN gcUNIFORM Uniform,
    IN gcsHINT_PTR Hints,
    IN glsMATRIX_PTR Matrix,
    IN GLfloat* ValueArray,
    IN gctUINT MatrixCount,
    IN gctUINT ColumnCount,
    IN gctUINT RowCount
    )
{
    gctUINT i, x, y;
    GLfloat matrix[4 * 4];
    GLfloat* value = ValueArray;
    gceSTATUS status;

    gcmHEADER_ARG("Uniform=0x%x Matrix=0x%x ValueArray=0x%x MatrixCount=%d "
                    "ColumnCount=%d RowCount=%d",
                    Uniform, Matrix, ValueArray, MatrixCount, ColumnCount, RowCount);

    for (i = 0; i < MatrixCount; i++)
    {
        glfGetFloatFromMatrix(
            &Matrix[i],
            matrix
            );

        for (y = 0; y < RowCount; y++)
        {
            for (x = 0; x < ColumnCount; x++)
            {
                *value++ = matrix[y + x * 4];
            }
        }
    }

    status = glfUtilUniformSetValue(Uniform, MatrixCount * RowCount, Hints, (gctPOINTER)ValueArray);

    gcmFOOTER();

    return status;
}

gceSTATUS glfUsingUniform(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctUINT Length,
    IN glfUNIFORMSET UniformSet,
    IN gctBOOL_PTR DirtyBit,
    IN glsUNIFORMWRAP_PTR* UniformWrap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("ShaderControl=0x%x Name=0x%x Type=0x%04x Length=%u "
                    "UniformSet=0x%x UniformWrap=0x%x",
                    ShaderControl, Name, Type, Length, UniformSet, UniformWrap);

    do
    {
        /* Allocate if not yet allocated. */
        if (*UniformWrap == gcvNULL)
        {
            gctUINT index;
            glsUNIFORMWRAP wrap;

            /* Query the number of uniforms in the shader. */
            gcmERR_BREAK(gcSHADER_GetUniformCount(
                ShaderControl->shader,
                &index
                ));

            /* Allocate a new uniform. */
            gcmERR_BREAK(gcSHADER_AddUniform(
                ShaderControl->shader,
                Name,
                Type,
                Length,
                gcSHADER_PRECISION_HIGH,    /* not sure, but no clue */
                &wrap.uniform
                ));

            /* Set the callback function. */
            wrap.set = UniformSet;

            /* Set dirty bit address. */
            wrap.dirty = DirtyBit;

            /* Add to the allocated uniform array. */
            ShaderControl->uniforms[index] = wrap;

            /* Set uniform quick access pointer. */
            *UniformWrap = &ShaderControl->uniforms[index];
        }

        /* Return success otherwise. */
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

gceSTATUS glfUsingAttribute(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctUINT Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEINFO_PTR AttributeInfo,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gctINT Binding,
    IN gctBOOL ExplicitilySetHighP,
    IN gcSHADER_SHADERMODE ShadingMode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("ShaderControl=0x%x Name=0x%x Type=0x%04x Length=%u "
                    "IsTexture=%d AttributeInfo=0x%x AttributeWrap=0x%x",
                    ShaderControl, Name, Type, Length, IsTexture, AttributeInfo, AttributeWrap);

    do
    {
        /* Allocate if not yet allocated. */
        if (*AttributeWrap == gcvNULL)
        {
            gctUINT index;
            glsATTRIBUTEWRAP wrap;

            /* Query the number of attributes in the shader. */
            gcmERR_BREAK(gcSHADER_GetAttributeCount(
                ShaderControl->shader,
                &index
                ));

            /* Allocate a new attribute. */
            gcmERR_BREAK(gcSHADER_AddAttribute(
                ShaderControl->shader,
                Name,
                Type,
                Length,
                IsTexture,
                ShadingMode,
                ExplicitilySetHighP ? gcSHADER_PRECISION_HIGH : gcSHADER_PRECISION_MEDIUM,
                &wrap.attribute
                ));

            /* Set stream information. */
            wrap.info = AttributeInfo;
            wrap.binding = Binding;

            /* Add to the allocated uniform array. */
            ShaderControl->attributes[index] = wrap;

            /* Set attribute quick access pointer. */
            *AttributeWrap = &ShaderControl->attributes[index];

        }

        /* Return success otherwise. */
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

gceSTATUS glfUsingVarying(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctUINT Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gctBOOL ExplicitilySetHighP,
    IN gcSHADER_SHADERMODE ShadingMode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("ShaderControl=0x%x Name=0x%x Type=0x%04x Length=%u "
                    "IsTexture=%d AttributeWrap=0x%x",
                    ShaderControl, Name, Type, Length, IsTexture, AttributeWrap);

    status = glfUsingAttribute(
        ShaderControl,
        Name,
        Type,
        Length,
        IsTexture,
        gcvFALSE,
        AttributeWrap,
        -1,
        ExplicitilySetHighP,
        ShadingMode
        );
    gcmFOOTER();

    return status;
}


/*******************************************************************************
**  uTexCoord constant is used in both VS and FS, define its functions here.
*/

static gceSTATUS _Set_uTexCoord(
    glsCONTEXT_PTR Context,
    gcUNIFORM Uniform,
    GLubyte* pointer
    )
{
    GLint i;
    GLfloat valueArray[4 * glvMAX_TEXTURES];
    GLfloat* vector = valueArray;
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", Context, Uniform);

    for (i = 0; i < Context->texture.pixelSamplers; i++)
    {
        glsTEXTURESAMPLER_PTR sampler = &Context->texture.sampler[i];

        /* Only do the enabled stages. */
        if (sampler->stageEnabled)
        {
            /* Compute constant texture coordinnate. */
            if (sampler->recomputeCoord)
            {
                /* Get a shortcut to the matrix stack. */
                glsMATRIXSTACK_PTR stack =
                    &Context->matrixStackArray[glvTEXTURE_MATRIX_0 + i];

                /* Transform texture coordinate. */
                glfMultiplyVector4ByMatrix4x4(
                    &sampler->homogeneousCoord,
                    stack->topMatrix,
                    &sampler->aTexCoordInfo.currValue
                    );

                /* Reset the flag. */
                sampler->recomputeCoord = GL_FALSE;
            }

            glfGetFloatFromVector4(
                &sampler->aTexCoordInfo.currValue,
                vector
                );
        }

        vector += 4;
    }

    status = glfUtilUniformSetValue(Uniform,
                                    glvMAX_TEXTURES,
                                    Context->currProgram->programState.hints,
                                    (gctPOINTER)valueArray);

    gcmFOOTER();
    return status;
}

gceSTATUS glfUsing_uTexCoord(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctBOOL_PTR DirtyBit,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("ShaderControl=0x%x UniformWrap=0x%x", ShaderControl, UniformWrap);
    status = glfUsingUniform(
        ShaderControl,
        "uTexCoord",
        gcSHADER_FLOAT_X4,
        glvMAX_TEXTURES,
        _Set_uTexCoord,
        DirtyBit,
        UniformWrap
        );
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  _LoadUniforms
**
**  Set the values for program uniforms.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the shader control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS _LoadUniforms(
    IN glsCONTEXT_PTR Context,
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctBOOL FlushALL
    )
{
    gceSTATUS status;
    gctUINT i, uniformCount, uboCount;
    GLubyte*  ubPtr = gcvNULL;
    gctUINT32 physical = 0;
    gcUNIFORM ubUniform = gcvNULL;

    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", Context, ShaderControl);

    /* Query the number of attributes. */
    gcmONERROR(gcSHADER_GetUniformCount(
        ShaderControl->shader,
        &uniformCount
        ));

    gcmONERROR(gcSHADER_GetUniformBlockCount(
        ShaderControl->shader,
        &uboCount
        ));

    if (ShaderControl->logicalAddress && !Context->programDirty)
    {
        ubPtr = ShaderControl->logicalAddress;
    }
    else if (uboCount == 1 && Context->programDirty)
    {
        gcsUNIFORM_BLOCK uniformBlock;

        gcmONERROR(gcSHADER_GetUniformBlock(ShaderControl->shader, 0, &uniformBlock));

        if (!uniformBlock || GetUBBlockIndex(uniformBlock) == -1)
        {
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        gcSHADER_GetUniform(ShaderControl->shader, GetUBIndex(uniformBlock), &ubUniform);

        /* Skip inactive uniform blocks */
        gcmASSERT(!isUniformInactive(ubUniform));

        if (GetUBBlockSize(uniformBlock) != ShaderControl->dataSize)
        {
            /* reallocated bufObj */
            if (ShaderControl->halBufObj != gcvNULL)
            {
                gcmVERIFY_OK(gcoBUFOBJ_Destroy(ShaderControl->halBufObj));
                ShaderControl->halBufObj = gcvNULL;
            }

            gcmONERROR(gcoBUFOBJ_Construct(gcvNULL, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &ShaderControl->halBufObj));

            gcmONERROR(gcoBUFOBJ_Upload(ShaderControl->halBufObj, gcvNULL, 0, GetUBBlockSize(uniformBlock), gcvBUFOBJ_USAGE_STATIC_READ));

            ShaderControl->dataSize = GetUBBlockSize(uniformBlock);

        }

        gcoBUFOBJ_FastLock(ShaderControl->halBufObj, &physical, (gctPOINTER *)&ubPtr);

        /* this is DUBO uniform */
        glfUtilUniformSetValue(ubUniform, 1, Context->currProgram->programState.hints, (gctPOINTER)&physical);

        ShaderControl->logicalAddress = (gctPOINTER)ubPtr;
    }

    /* Mark defaultUBO will be used by this draw */
    if (ubPtr)
    {
#if gcdSYNC
        if (ShaderControl->halBufObj)
        {
            gcoBUFOBJ_WaitFence(ShaderControl->halBufObj, gcvFENCE_TYPE_WRITE);
        }
#endif
    }

    /* Iterate though the attributes. */
    for (i = 0; i < uniformCount; i++)
    {
        /* Get current uniform. */
        glsUNIFORMWRAP_PTR wrap = &ShaderControl->uniforms[i];

        /* Set the uniform.
        ** And if a uniform is inactive, don't need to update the value.
        */
        if (wrap->set != gcvNULL &&
            (FlushALL || *(wrap->dirty)) &&
            !isUniformInactive(wrap->uniform))
        {
            gcmONERROR((*wrap->set) (Context, wrap->uniform, ubPtr));

            *(wrap->dirty) = gcvFALSE;
        }
    }

OnError:
    gcmFOOTER();
    /* Return status. */
    return status;
}


#if gcmIS_DEBUG(gcdDEBUG_TRACE)
/*******************************************************************************
**
**  glfPrintStates
**
**  Debug printing of various states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

void glfPrintStates(
    IN glsCONTEXT_PTR Context
    )
{


}
#endif

gceSTATUS glfIsProgramSwitched(
    IN glsCONTEXT_PTR Context
    )
{
    gceSTATUS status;

    status = gco3D_IsProgramSwitched(Context->hw);

    return status;
}


/*******************************************************************************
**
**  glfLoadShader
**
**  Generate and load vertex and pixel shaders fixed functions.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfLoadShader(
    IN glsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    glsPROGRAMINFO_PTR program = gcvNULL;
    gcmHEADER_ARG("Context=0x%x ", Context);

    do
    {
        /* Update everything that may affect the hash key. */
        glfUpdateMatrixStates(Context);

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
        /* Debug printing of various states. */
        glfPrintStates(Context);
#endif

        /* Locate the program entry. */
        gcmONERROR(glfGetHashedProgram(Context, &program));

        /* Reset force flush PS uniforms. */
        Context->forceFlushPSUniforms = gcvFALSE;

        Context->programDirty = gcvFALSE;

        if (program != Context->currProgram)
        {
            gctINT prevPsConstStart = -1;

            if (Context->currProgram != gcvNULL &&
                Context->currProgram->programState.hints != gcvNULL &&
                Context->currProgram->programState.hints->unifiedStatus.constantUnifiedMode == gcvUNIFORM_ALLOC_GPIPE_TOP_PS_BOTTOM_FLOAT_BASE_OFFSET)
            {
                prevPsConstStart = Context->currProgram->programState.hints->unifiedStatus.constPSStart;
            }

            Context->currProgram = program;
            Context->programDirty = gcvTRUE;
            /* Create new program if necessary. */
            if (Context->currProgram->programState.stateBufferSize == 0
                || Context->currProgram->vs.shader == gcvNULL
                || Context->currProgram->fs.shader == gcvNULL)
            {
                /* Generate shaders. */
                gcmONERROR(glfGenerateVSFixedFunction(Context));
                gcmONERROR(glfGenerateFSFixedFunction(Context));

                /* disable varying packing for ES11, since it caused MM06
                   fillrate more than 15% degradation */
                /*gcmOPT_SetVaryingPacking(gcvOPTIMIZATION_VARYINGPACKING_NONE);*/
                /* Link the program. */
                gcmONERROR(gcLinkShaders(
                    Context->currProgram->vs.shader,
                    Context->currProgram->fs.shader,
                    (gceSHADER_FLAGS)
                    ( 0
                    | gcvSHADER_DEAD_CODE
                    | gcvSHADER_RESOURCE_USAGE
                    | gcvSHADER_OPTIMIZER
                    | gcvSHADER_USE_GL_POSITION
                    | gcvSHADER_FLUSH_DENORM_TO_ZERO
                    ),
                    &Context->currProgram->programState
                    ));
            }

             if (Context->currProgram->programState.hints &&
                 Context->pointStates.pointPrimitive &&
                 Context->pointStates.smooth &&
                 !Context->pointStates.spriteEnable)
            {
                Context->currProgram->programState.hints->hasKill = 1;
            }

            if (prevPsConstStart != -1 &&
                Context->currProgram->programState.hints != gcvNULL &&
                prevPsConstStart != Context->currProgram->programState.hints->unifiedStatus.constPSStart)
            {
                Context->forceFlushPSUniforms = gcvTRUE;
            }

            /* Send states to hardware. */
            gcmONERROR(gcLoadShaders(
                Context->hal,
                &Context->currProgram->programState
                ));

            /* Load uniforms. */
            gcmONERROR(_LoadUniforms(Context, &Context->currProgram->vs, gcvTRUE));
            gcmONERROR(_LoadUniforms(Context, &Context->currProgram->fs, gcvTRUE));
        }
        else
        {
#if gcdENABLE_APPCTXT_BLITDRAW
            if (glfIsProgramSwitched(Context) == gcvTRUE)
            {
                Context->programDirty = gcvTRUE;
                /* Load uniforms. */
                gcmERR_BREAK(_LoadUniforms(Context, &Context->currProgram->vs, gcvTRUE));

                gcmERR_BREAK(_LoadUniforms(Context, &Context->currProgram->fs, gcvTRUE));
            }
            else
#endif
            {
                Context->programDirty = gcvFALSE;
                /* Load uniforms. */
                gcmERR_BREAK(_LoadUniforms(Context, &Context->currProgram->vs, gcvFALSE));

                /* For fragment shader, if clearing using shader, the clear color needs to be force flushed. */
                gcmERR_BREAK(_LoadUniforms(Context, &Context->currProgram->fs, Context->drawClearRectEnabled));
            }
        }
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;

OnError:
    /* Reset currProgram, as it may have been partially created due to an error. */
    if (Context->currProgram != gcvNULL)
    {
        if (Context->currProgram->vs.shader)
        {
            gcmVERIFY_OK(gcSHADER_Destroy(Context->currProgram->vs.shader));
            Context->currProgram->vs.shader = gcvNULL;
        }

        if (Context->currProgram->fs.shader)
        {
            gcmVERIFY_OK(gcSHADER_Destroy(Context->currProgram->fs.shader));
            Context->currProgram->fs.shader = gcvNULL;
        }

        Context->currProgram = gcvNULL;
    }
    gcmFOOTER();
    /* Return status. */
    return status;
}
