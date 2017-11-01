/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_chip_shader.h"
#include "gc_chip_hash.h"


#define _GC_OBJ_ZONE __GLES3_ZONE_TRACE

extern gceSTATUS getHashedProgram( __GLchipContext  *chipCtx, glsPROGRAMINFO_PTR* Program);
extern gceSTATUS gcLoadShaders( IN gcoHAL Hal, IN gcsPROGRAM_STATE ProgramState);
/*******************************************************************************
** Shader generation helpers.
*/

gceSTATUS glfUsingUniform(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN glfUNIFORMSET UniformSet,
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
            gctUINT32 index = 0;
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
                (gctUINT32)Length,
                gcSHADER_PRECISION_LOW,
                &wrap.uniform
                ));

            /* Set the callback function. */
            wrap.set = UniformSet;

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
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEINFO_PTR AttributeInfo,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gctINT Binding,
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
            gctUINT32 index = 0;
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
                (gctUINT32)Length,
                IsTexture,
                ShadingMode,
                gcSHADER_PRECISION_LOW,
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
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
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
        ShadingMode
        );
    gcmFOOTER();

    return status;
}

static gceSTATUS set_uTexGenObjectPlane(
    IN __GLcontext * gc,
    IN gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0;
    GLuint numOfTex;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;
    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxCmptTextureImageUnits,8);
    while((i < numOfTex) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.f.w;
        }
        stageEnableMask >>= 1;
        i++;

    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, 4 * i, chipCtx->currProgram->programState.hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uTexGenEyePlane(
    IN __GLcontext * gc,
    IN gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0;
    GLuint numOfTex;
    gltFRACTYPE valueArray[4 * 4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;
    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxCmptTextureImageUnits,8);
    while((i < numOfTex) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.f.w;

            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.f.x;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.f.y;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.f.z;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.f.w;
        }
        stageEnableMask >>= 1;
        i++;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, 4 * i, chipCtx->currProgram->programState.hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  uTexCoord constant is used in both VS and FS, define its functions here.
*/

static gceSTATUS set_uTexCoord(
    IN __GLcontext * gc,
    IN gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    GLuint i = 0;
    GLuint numOfTex;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;
    gcsGLSLCaps *shaderCaps = gcvNULL;

    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;
    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxCmptTextureImageUnits,8);
    while((i < numOfTex) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            /* Compute constant texture coordinnate. */
            __GLcoord * vector = &gc->state.current.texture[i];
            __GLcoord resVector;

            if ((vector->fTex.q != 0.0f) && (vector->fTex.q != 1.0f))
            {
                vector->fTex.s = gcoMATH_Divide(vector->fTex.s, vector->fTex.q);
                vector->fTex.t = gcoMATH_Divide(vector->fTex.t, vector->fTex.q);
                vector->fTex.r = gcoMATH_Divide(vector->fTex.r, vector->fTex.q);
                vector->fTex.q = 1.0f;
            }

            /* Transform texture coordinate. */
            __glTransformCoord(&resVector, vector, &gc->transform.texture[i]->matrix);

            *value++ = resVector.fTex.s;
            *value++ = resVector.fTex.t;
            *value++ = resVector.fTex.r;
            *value++ = resVector.fTex.q;
        } else {
            value += 4;
        }

        stageEnableMask >>= 1;
        i++;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->programState.hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS glfUsing_uTexCoord(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("ShaderControl=0x%x UniformWrap=0x%x", ShaderControl, UniformWrap);
    status = glfUsingUniform(
        ShaderControl,
        "uTexCoord",
        gcSHADER_FRAC_X4,
        glvMAX_TEXTURES,
        set_uTexCoord,
        UniformWrap
        );
    gcmFOOTER();
    return status;
}

gceSTATUS glfUsing_uTexGenObjectPlane(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("ShaderControl=0x%x UniformWrap=0x%x", ShaderControl, UniformWrap);
    status = glfUsingUniform(
        ShaderControl,
        "uTexGenObjectPlane",
        gcSHADER_FRAC_X4,
        4 * glvMAX_TEXTURES,
        set_uTexGenObjectPlane,
        UniformWrap
        );
    gcmFOOTER();
    return status;
}

gceSTATUS glfUsing_uTexGenEyePlane(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("ShaderControl=0x%x UniformWrap=0x%x", ShaderControl, UniformWrap);
    status = glfUsingUniform(
        ShaderControl,
        "uTexGenEyePlane",
        gcSHADER_FRAC_X4,
        4 * glvMAX_TEXTURES,
        set_uTexGenEyePlane,
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

gceSTATUS loadUniforms(
    IN __GLcontext * gc,
    IN glsSHADERCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gctUINT32 i = 0, uniformCount = 0;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    /* Query the number of attributes. */
    gcmONERROR(gcSHADER_GetUniformCount(
        ShaderControl->shader,
        &uniformCount
        ));

    /* Iterate though the attributes. */
    for (i = 0; i < uniformCount; i++)
    {
        /* Get current uniform. */
        glsUNIFORMWRAP_PTR wrap = &ShaderControl->uniforms[i];

        /* Set the uniform. */
        if (wrap->set != gcvNULL && !isUniformInactive(wrap->uniform))
        {
            gcmONERROR((*wrap->set) (gc, wrap->uniform));
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
    IN __GLcontext * gc
    )
{


}
#endif


/*******************************************************************************
**
**  loadFixFunctionShader
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

gceSTATUS gcChipLoadFixFunctionShader(
    IN __GLcontext * gc
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    glsPROGRAMINFO_PTR program = gcvNULL;
    gctUINT enableBits = 0;
    gctUINT i, j, count = 0, attr = 0;
    gcmHEADER_ARG("gc=0x%x ", gc);

    do
    {
        /* Update everything that may affect the hash key. */

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
        /* Debug printing of various states. */
        glfPrintStates(gc);
#endif

        /* Locate the program entry. */
        gcmONERROR(getHashedProgram(chipCtx, &program));

        if (program != chipCtx->currProgram)
        {
            chipCtx->currProgram = program;

            /* Create new program if necessary. */
            if (chipCtx->currProgram->programState.stateBufferSize == 0)
            {
                /* Generate shaders. */
                gcmONERROR(glfGenerateVSFixedFunction(gc));
                gcmONERROR(glfGenerateFSFixedFunction(gc));

                /* Link the program. */
                gcmONERROR(gcLinkShaders(
                    chipCtx->currProgram->vs.shader,
                    chipCtx->currProgram->fs.shader,
                    (gceSHADER_FLAGS)
                    ( 0
                    | gcvSHADER_DEAD_CODE
                    | gcvSHADER_RESOURCE_USAGE
                    | gcvSHADER_OPTIMIZER
                    ),
                    &chipCtx->currProgram->programState
                    ));
            }

            /* Send states to hardware. */
            gcmONERROR(gcLoadShaders(
                chipCtx->hal,
                chipCtx->currProgram->programState
                ));
        } else {
            /* Shader switched from GLSH to fix function */
            if (chipCtx->programDirty) {
                /* Send states to hardware. */
                gcmONERROR(gcLoadShaders(
                    chipCtx->hal,
                    chipCtx->currProgram->programState
                    ));
                chipCtx->programDirty = GL_FALSE;
            }
        }

        /* Load uniforms. */
        gcmONERROR(loadUniforms(gc, &chipCtx->currProgram->vs));
        gcmONERROR(loadUniforms(gc, &chipCtx->currProgram->fs));
    }
    while (gcvFALSE);

    /* Get number of attributes for the vertex shader. */
    gcmONERROR(gcSHADER_GetAttributeCount(chipCtx->currProgram->vs.shader, &count));

    /* Walk all vertex shader attributes. */
    for (i = 0, j = 0; i < count; ++i)
    {
        gctBOOL attributeEnabled;


        /* Get the attribute linkage. */
        attr = chipCtx->currProgram->vs.attributes[i].binding;

        gcmERR_BREAK(gcATTRIBUTE_IsEnabled(
            chipCtx->currProgram->vs.attributes[i].attribute,
            &attributeEnabled
            ));

        if (attributeEnabled)
        {
            /* Link the attribute to the vertex shader input. */
            chipCtx->attributeArray[attr].linkage = j++;

            /* Enable the attribute. */
            enableBits |= 1 << attr;
        }
    }
    chipCtx ->attribMask = enableBits;

    /* Return status. */
    gcmFOOTER();
    return status;

OnError:
    /* Reset currProgram, as it may have been partially created due to an error. */
    if (chipCtx->currProgram != gcvNULL)
    {
        if (chipCtx->currProgram->vs.shader)
        {
            gcmVERIFY_OK(gcSHADER_Destroy(chipCtx->currProgram->vs.shader));
            chipCtx->currProgram->vs.shader = gcvNULL;
        }

        if (chipCtx->currProgram->fs.shader)
        {
            gcmVERIFY_OK(gcSHADER_Destroy(chipCtx->currProgram->fs.shader));
            chipCtx->currProgram->fs.shader = gcvNULL;
        }

        chipCtx->currProgram = gcvNULL;
    }
    gcmFOOTER();
    /* Return status. */
    return status;
}
