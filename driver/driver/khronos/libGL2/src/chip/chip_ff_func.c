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


#include "gc_gl_context.h"
#include "chip_context.h"

#define _GC_OBJ_ZONE glvZONE_TRACE

extern gceSTATUS getHashedProgram( glsCHIPCONTEXT_PTR chipCtx, glsPROGRAMINFO_PTR* Program);
extern gceSTATUS gcLoadShaders( IN gcoHAL Hal, IN gctSIZE_T StateBufferSize, IN gctPOINTER StateBuffer, IN gcsHINT_PTR Hints);
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
            gctSIZE_T index;
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
            gctSIZE_T index;
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
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLint i = 0;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;

    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;

    while((i < gc->constants.numberOfTextureUnits) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].s.objectPlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].t.objectPlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].r.objectPlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].q.objectPlaneEquation.w;
        }
        stageEnableMask >>= 1;
        i++;

    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, 4 * i, chipCtx->currProgram->hints, valueArray);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS set_uTexGenEyePlane(
    IN __GLcontext * gc,
    IN gcUNIFORM Uniform
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLint i = 0;
    gltFRACTYPE valueArray[4 * 4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;

    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;

    while((i < gc->constants.numberOfTextureUnits) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].s.eyePlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].t.eyePlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].r.eyePlaneEquation.w;

            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.x;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.y;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.z;
            *value++ = gc->state.texture.texUnits[i].q.eyePlaneEquation.w;
        }
        stageEnableMask >>= 1;
        i++;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, 4 * i, chipCtx->currProgram->hints, valueArray);
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
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLint i = 0;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status = 0;
    GLuint stageEnableMask;

    gcmHEADER_ARG("Context=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(value, sizeof(valueArray));
    stageEnableMask = chipCtx->texture.stageEnabledMask;

    while((i < gc->constants.numberOfTextureUnits) && stageEnableMask)
    {
        /* Only do the enabled stages. */
        if (stageEnableMask & 1)
        {
            /* Compute constant texture coordinnate. */
            __GLcoord * vector = &gc->state.current.texture[i];
            __GLcoord resVector;

            if ((vector->q != 0.0f) && (vector->q != 1.0f))
            {
                vector->s = gcoMATH_Divide(vector->s, vector->q);
                vector->t = gcoMATH_Divide(vector->t, vector->q);
                vector->r = gcoMATH_Divide(vector->r, vector->q);
                vector->q = 1.0f;
            }

            /* Transform texture coordinate. */
            __glTransformCoord(&resVector, vector, &gc->transform.texture[i]->matrix);

            *value++ = resVector.s;
            *value++ = resVector.t;
            *value++ = resVector.r;
            *value++ = resVector.q;
        } else {
            value += 4;
        }

        stageEnableMask >>= 1;
        i++;
    }

    if (i > 0) {
        status = gcUNIFORM_SetValueF_Ex(Uniform, i, chipCtx->currProgram->hints, valueArray);
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

static gceSTATUS loadUniforms(
    IN __GLcontext * gc,
    IN glsSHADERCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gctSIZE_T i, uniformCount;
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

gceSTATUS loadFixFunctionShader(
    IN __GLcontext * gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    glsPROGRAMINFO_PTR program = gcvNULL;
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
            if (chipCtx->currProgram->programSize == 0)
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
                    &chipCtx->currProgram->programSize,
                    &chipCtx->currProgram->programBuffer,
                    &chipCtx->currProgram->hints
                    ));
            }

            /* Send states to hardware. */
            gcmONERROR(gcLoadShaders(
                chipCtx->hal,
                chipCtx->currProgram->programSize,
                chipCtx->currProgram->programBuffer,
                chipCtx->currProgram->hints
                ));
        } else {
            /* Shader switched from GLSH to fix function */
            if (chipCtx->programDirty) {
                /* Send states to hardware. */
                gcmONERROR(gcLoadShaders(
                    chipCtx->hal,
                    chipCtx->currProgram->programSize,
                    chipCtx->currProgram->programBuffer,
                    chipCtx->currProgram->hints
                    ));
                chipCtx->programDirty = GL_FALSE;
            }
        }

        /* Load uniforms. */
        gcmONERROR(loadUniforms(gc, &chipCtx->currProgram->vs));
        gcmONERROR(loadUniforms(gc, &chipCtx->currProgram->fs));
    }
    while (gcvFALSE);

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
