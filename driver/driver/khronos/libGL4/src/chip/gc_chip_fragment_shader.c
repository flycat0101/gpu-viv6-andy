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


#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_chip_shader.h"
#include "gc_chip_hash.h"
#include "gc_chip_pixel.h"

#define _GC_OBJ_ZONE __GLES3_ZONE_FIXFRAG

/*******************************************************************************
** Strings for debugging.
*/

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
static char* g_functionName[] =
{
    "REPLACE",
    "MODULATE",
    "DECAL",
    "BLEND",
    "ADD",
    "COMBINE"
};

static char* g_combineFunctionName[] =
{
    "REPLACE",
    "MODULATE",
    "ADD",
    "ADD SIGNED",
    "INTERPOLATE",
    "SUBTRACT",
    "DOT3RGB",
    "DOT3RGBA"
};

static char* g_passName[] =
{
    "COLOR",
    "ALPHA"
};

static char* g_sourceName[] =
{
    "TEXTURE",
    "CONSTANT",
    "PRIMARY COLOR",
    "PREVIOUS"
};

static char* g_operandName[] =
{
    "ALPHA",
    "1-ALPHA",
    "COLOR",
    "1-COLOR"
};

#endif


/*******************************************************************************
** Vertex Shader internal parameters.
*/

//typedef struct _glsFSCONTROL * glsFSCONTROL_PTR;
typedef struct _glsFSCONTROL
{
    /* Pointer to the exposed shader interface. */
    glsSHADERCONTROL_PTR i;

    /* Postpone clamping in case if the color is not used anymore.
       Hardware does the clamping of the output by default. */
    gctBOOL clampColor;

    /* Temporary register and label allocation indices. */
    glmDEFINE_TEMP(rColor);
    glmDEFINE_TEMP(rColor2);
    glmDEFINE_TEMP(rLastAllocated);
    glmDEFINE_LABEL(lLastAllocated);

    /* Uniforms. */
    glsUNIFORMWRAP_PTR uniforms[glvUNIFORM_FS_COUNT];

    /* Attributes. */
    glsATTRIBUTEWRAP_PTR attributes[glvATTRIBUTE_FS_COUNT];

    /* Outputs. */
    glmDEFINE_TEMP(oPrevColor);
    glmDEFINE_TEMP(oColor);
}
glsFSCONTROL;

/*******************************************************************************
** Define texture function pointer arrays.
*/
typedef gceSTATUS (* glfDOCOMBINETEXTUREFUNCTION) (
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    );

typedef struct _glsCOMBINEFUNCTION
{
    glfDOCOMBINETEXTUREFUNCTION function;
    GLboolean haveArg[3];
}
glsCOMBINEFUNCTION;

#define glmDECLARECOMBINETEXTUREFUNCTION(Name) \
    static gceSTATUS Name( \
         __GLcontext * gc, \
        glsFSCONTROL_PTR ShaderControl, \
        gctUINT16_PTR Arguments, \
        glsCOMBINEFLOW_PTR CombineFlow \
        )

#define glmCOMBINETEXTUREFUNCTIONENTRY(Name, Arg0, Arg1, Arg2) \
    {Name, {GL_##Arg0, GL_##Arg1, GL_##Arg2}}

glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncReplace);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncModulate);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncAdd);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncAddSigned);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncInterpolate);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncSubtract);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncDot3RGB);
glmDECLARECOMBINETEXTUREFUNCTION(texCombFuncDot3RGBA);

static glsCOMBINEFUNCTION _CombineTextureFunctions[] =
{
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncReplace,     TRUE, FALSE, FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncModulate,    TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncAdd,         TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncAddSigned,   TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncInterpolate, TRUE, TRUE,  TRUE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncSubtract,    TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncDot3RGB,     TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncDot3RGBA,    TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncDot3RGB,     TRUE, TRUE,  FALSE),
    glmCOMBINETEXTUREFUNCTIONENTRY(texCombFuncDot3RGBA,    TRUE, TRUE,  FALSE),
};

#define glmDECLARETEXTUREFUNCTION(Name) \
    static gceSTATUS Name( \
        __GLcontext * gc, \
        glsFSCONTROL_PTR ShaderControl, \
        glsTEXTURESAMPLER_PTR Sampler, \
        gctUINT SamplerNumber \
        )

#define glmTEXTUREFUNCTIONENTRY(Name) \
    Name

glmDECLARETEXTUREFUNCTION(texFuncReplace);
glmDECLARETEXTUREFUNCTION(texFuncModulate);
glmDECLARETEXTUREFUNCTION(texFuncDecal);
glmDECLARETEXTUREFUNCTION(texFuncBlend);
glmDECLARETEXTUREFUNCTION(texFuncAdd);
glmDECLARETEXTUREFUNCTION(texFuncCombine);
glmDECLARETEXTUREFUNCTION(texFuncStipple);

glfDOTEXTUREFUNCTION _TextureFunctions[] =
{
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncReplace)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncModulate)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncDecal)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncBlend)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncAdd)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncCombine)),
    glmTEXTUREFUNCTIONENTRY(((glfDOTEXTUREFUNCTION)texFuncStipple))
};

/*******************************************************************************
** texture environment parameters
*/

/* Possible texture functions. */
GLenum textureFunctionNames[] =
{
    GL_REPLACE,                 /* glvTEXREPLACE */
    GL_MODULATE,                /* glvTEXMODULATE */
    GL_DECAL,                   /* glvTEXDECAL */
    GL_BLEND,                   /* glvTEXBLEND */
    GL_ADD,                     /* glvTEXADD */
    GL_COMBINE,                 /* glvTEXCOMBINE */
    __GL_STIPPLE,               /* Extra flag used for bitmap simulation */
};

GLenum combineColorTextureFunctionNames[] =
{
    GL_REPLACE,                 /* glvCOMBINEREPLACE */
    GL_MODULATE,                /* glvCOMBINEMODULATE */
    GL_ADD,                     /* glvCOMBINEADD */
    GL_ADD_SIGNED,              /* glvCOMBINEADDSIGNED */
    GL_INTERPOLATE,             /* glvCOMBINEINTERPOLATE */
    GL_SUBTRACT,                /* glvCOMBINESUBTRACT */
    GL_DOT3_RGB,                /* glvCOMBINEDOT3RGB */
    GL_DOT3_RGBA,               /* glvCOMBINEDOT3RGBA */
    GL_DOT3_RGB_EXT,            /* glvCOMBINEDOT3RGBEXT */
    GL_DOT3_RGBA_EXT,           /* glvCOMBINEDOT3RGBAEXT */
};

GLenum combineAlphaTextureFunctionNames[] =
{
    GL_REPLACE,                 /* glvCOMBINEREPLACE */
    GL_MODULATE,                /* glvCOMBINEMODULATE */
    GL_ADD,                     /* glvCOMBINEADD */
    GL_ADD_SIGNED,              /* glvCOMBINEADDSIGNED */
    GL_INTERPOLATE,             /* glvCOMBINEINTERPOLATE */
    GL_SUBTRACT,                /* glvCOMBINESUBTRACT */
    GL_MODULATE_ADD_ATI,        /* missing */
    GL_MODULATE_SIGNED_ADD_ATI, /* missing */
    GL_MODULATE_SUBTRACT_ATI,   /* missing */
};

GLboolean setTextureFunction(
    __GLchipContext     *chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    result = glfConvertGLEnum(
        textureFunctionNames,
        gcmCOUNTOF(textureFunctionNames),
        Value, Type,
        &value
        );

    if (result)
    {
        glmSETHASH_3BITS(hashTextureFunction, value, Sampler->index);
        Sampler->doTextureFunction = _TextureFunctions[value];
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}


GLboolean setCombineColorFunction(
    __GLchipContext     *chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineColorTextureFunctionNames,
        gcmCOUNTOF(combineColorTextureFunctionNames),
        Value, Type,
        &value
        );

    if (result)
    {
        glmSETHASH_4BITS(hashTextureCombColorFunction, value, Sampler->index);
        Sampler->combColor.function = (gleTEXCOMBINEFUNCTION)value;

        if (value == glvCOMBINEDOT3RGBA)
        {
            Sampler->colorDataFlow.targetEnable = gcSL_ENABLE_XYZW;
            Sampler->colorDataFlow.tempEnable   = gcSL_ENABLE_XYZW;
            Sampler->colorDataFlow.tempSwizzle  = gcSL_SWIZZLE_XYZW;
            Sampler->colorDataFlow.argSwizzle   = gcSL_SWIZZLE_XYZW;
        }
        else
        {
            Sampler->colorDataFlow.targetEnable = gcSL_ENABLE_XYZ;
            Sampler->colorDataFlow.tempEnable   = gcSL_ENABLE_XYZ;
            Sampler->colorDataFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
            Sampler->colorDataFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLboolean setCombineAlphaFunction(
    __GLchipContext     *chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineAlphaTextureFunctionNames,
        gcmCOUNTOF(combineAlphaTextureFunctionNames),
        Value, Type,
        &value
        );

    if (result)
    {
        glmSETHASH_3BITS(hashTextureCombAlphaFunction, value, Sampler->index);
        Sampler->combAlpha.function = (gleTEXCOMBINEFUNCTION)value;;
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/*******************************************************************************
** Temporary allocation helper.
*/

static gctUINT16 allocateTemp(
    glsFSCONTROL_PTR ShaderControl
    )
{
    gctUINT16 result;
    gcmHEADER_ARG("ShaderControl=0x%x", ShaderControl);
    gcmASSERT(ShaderControl->rLastAllocated < 65535);
    result = ++ShaderControl->rLastAllocated;
    gcmFOOTER_ARG("return=%u", result);
    return result;
}


/*******************************************************************************
** Label allocation helper.
*/

static gctUINT allocateLabel(
    glsFSCONTROL_PTR ShaderControl
    )
{
    gctUINT result;
    gcmHEADER_ARG("ShaderControl=0x%x", ShaderControl);
    result = ++ShaderControl->lLastAllocated;
    gcmFOOTER_ARG("return=%u", result);
    return result;
}


/*******************************************************************************
** Constant setting callbacks.
*/

static gceSTATUS set_uAccum(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);


    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, &chipCtx->accumValue);

    gcmFOOTER();

    return status;
}

/*******************************************************************************
** Constant setting callbacks.
*/

static gceSTATUS set_uYmajor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);


    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, &chipCtx->yMajor);

    gcmFOOTER();

    return status;
}


static gceSTATUS set_uColor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    if (chipCtx->drawClearRectEnabled)
    {
        chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].dirty = GL_TRUE;

        status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, &gc->state.raster.clearColor.clear.r);
        gcmFOOTER();
        return status;
    }
    else
    {
        if (!chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].dirty)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].dirty = GL_FALSE;

        status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, &gc->state.current.color.r);

        gcmFOOTER();
        return status;
    }
}


static gceSTATUS set_uFogFactors(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gltFRACTYPE valueArray[4];
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    gcoOS_ZeroMemory(valueArray, 4 * sizeof(gltFRACTYPE));

    if (gc->state.fog.mode == GL_LINEAR)
    {
        /* Update the factors. */
        GLfloat start, end;

        start = gc->state.fog.start;
        end = gc->state.fog.end;
        /* factor0 = 1 / (s - e) */
        valueArray[0] = 1.0f / (start - end);

        /* factor1 = e / (e - s) */
        valueArray[1] = end / (end - start);
    }
    else if (gc->state.fog.mode == GL_EXP)
    {
        /*
            Hardware only has an exponent function with the base of 2.
            Convert the exponent here to be able to substitute the required
            e-based exponent function with available 2-based exponent
            function:

                EXP mode:
                    1 / log(2) = 1.44269504088896
        */

        static const GLfloat _expFogDensityAdjustment  = 1.44269504088896f;

        /* Get fog density. */
        gltFRACTYPE density = gc->state.fog.density;

        /* Compute adjusted density. */
        valueArray[0] = glmFLOATMULTIPLY(density, _expFogDensityAdjustment);
    }
    else
    {
        /* Update the factors. */
        /*
            Hardware only has an exponent function with the base of 2.
            Convert the exponent here to be able to substitute the required
            e-based exponent function with available 2-based exponent
            function:

                EXP2 mode:
                    sqrt(1 / log(2)) = 1.2011224088
        */

        static const GLfloat _exp2FogDensityAdjustment = 1.2011224088f;

        /* Get fog density. */
        gltFRACTYPE density = gc->state.fog.density;

        /* Compute adjusted density. */
        valueArray[0] = glmFLOATMULTIPLY(density,_exp2FogDensityAdjustment);
    }

    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    /* Return status. */
    return status;
}

static gceSTATUS set_uFogColor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);
    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, &gc->state.fog.color.r);
    gcmFOOTER();
    return status;
}

static gceSTATUS set_uTexColor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    GLuint i,numOfTex;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* value = valueArray;
    gceSTATUS status;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxTextureSamplers,8);
    for (i = 0; i < numOfTex; i++)
    {
        __GLcolor * color = &gc->state.texture.texUnits[i].env.color;
        *value++ = color->r;
        *value++ = color->g;
        *value++ = color->b;
        *value++ = color->a;
    }

    status = gcUNIFORM_SetValueF_Ex(Uniform, numOfTex, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uTexCombScale(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    GLuint i,numOfTex;
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* vector = valueArray;
    gceSTATUS status;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxTextureSamplers,8);
    for (i = 0; i < numOfTex; i++)
    {
        vector[0] =
        vector[1] =
        vector[2] = gc->state.texture.texUnits[i].env.rgbScale;
        vector[3] = gc->state.texture.texUnits[i].env.alphaScale;
        vector += 4;
    }

    status = gcUNIFORM_SetValueF_Ex(Uniform, numOfTex, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uPixelTransferScale(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[4];
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    valueArray[0] = gc->state.pixel.transferMode.r_scale;
    valueArray[1] = gc->state.pixel.transferMode.g_scale;
    valueArray[2] = gc->state.pixel.transferMode.b_scale;
    valueArray[3] = gc->state.pixel.transferMode.a_scale;

    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uPixelTransferBias(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[4];
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    valueArray[0] = gc->state.pixel.transferMode.r_bias;
    valueArray[1] = gc->state.pixel.transferMode.g_bias;
    valueArray[2] = gc->state.pixel.transferMode.b_bias;
    valueArray[3] = gc->state.pixel.transferMode.a_bias;

    status = gcUNIFORM_SetValueF_Ex(Uniform, 1, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    return status;
}

static gceSTATUS set_uTextureBorderColor(
    __GLcontext * gc,
    gcUNIFORM Uniform
    )
{
    gltFRACTYPE valueArray[4 * glvMAX_TEXTURES];
    gltFRACTYPE* vector = valueArray;
    gceSTATUS status;
    GLuint i,numOfTex;
    GLuint enableDim;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x Uniform=0x%x", gc, Uniform);

    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxTextureSamplers,8);
    for (i = 0; i < numOfTex; i++)
    {
        //enableDim = gc->state.texture.texUnits[i].currentEnableDim - 1;
        enableDim = gc->state.texture.texUnits[i].enableDim - 1;
        if (enableDim == 0) continue;
        vector[0] = gc->state.texture.texUnits[i].texObj[enableDim].params.bc.borderColor.r;
        vector[1] = gc->state.texture.texUnits[i].texObj[enableDim].params.bc.borderColor.g;
        vector[2] = gc->state.texture.texUnits[i].texObj[enableDim].params.bc.borderColor.b;
        vector[3] = gc->state.texture.texUnits[i].texObj[enableDim].params.bc.borderColor.a;
        vector += 4;
    }

    status = gcUNIFORM_SetValueF_Ex(Uniform, numOfTex, chipCtx->currProgram->programState.hints, valueArray);

    gcmFOOTER();

    return status;
}

/*******************************************************************************
** Uniform access helpers.
*/

static gceSTATUS using_uAccum(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uAccum",
        gcSHADER_FRAC_X1,
        1,
        set_uAccum,
        &glmUNIFORM_WRAP(FS, uAccum)
        );

    gcmFOOTER();

    return status;
}


/*******************************************************************************
** Uniform access helpers.
*/

static gceSTATUS using_uYmajor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uYmajor",
        gcSHADER_FRAC_X1,
        1,
        set_uYmajor,
        &glmUNIFORM_WRAP(FS, uYmajor)
        );

    gcmFOOTER();

    return status;
}


static gceSTATUS using_uColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uColor",
        gcSHADER_FRAC_X4,
        1,
        set_uColor,
        &glmUNIFORM_WRAP(FS, uColor)
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_uColor2(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uColor2",
        gcSHADER_FRAC_X4,
        1,
        set_uColor,
        &glmUNIFORM_WRAP(FS, uColor2)
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_uFogFactors(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uFogFactors",
        gcSHADER_FRAC_X4,
        1,
        set_uFogFactors,
        &glmUNIFORM_WRAP(FS, uFogFactors)
        );

    gcmFOOTER();
    return status;
}

static gceSTATUS using_uFogColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uFogColor",
        gcSHADER_FRAC_X4,
        1,
        set_uFogColor,
        &glmUNIFORM_WRAP(FS, uFogColor)
        );

    gcmFOOTER();
    return status;
}

static gceSTATUS using_uTexColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    shaderCaps = &gc->constants.shaderCaps;
    status = glfUsingUniform(
        ShaderControl->i,
        "uTexColor",
        gcSHADER_FRAC_X4,
        gcmMIN(shaderCaps->maxTextureSamplers,8),
        set_uTexColor,
        &glmUNIFORM_WRAP(FS, uTexColor)
        );

    gcmFOOTER();
    return status;
}

static gceSTATUS using_uTexCombScale(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    shaderCaps = &gc->constants.shaderCaps;
    status = glfUsingUniform(
        ShaderControl->i,
        "uTexCombScale",
        gcSHADER_FRAC_X4,
        gcmMIN(shaderCaps->maxTextureSamplers,8),
        set_uTexCombScale,
        &glmUNIFORM_WRAP(FS, uTexCombScale)
        );

    gcmFOOTER();
    return status;
}

static gceSTATUS using_uTexSampler(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctINT Sampler
    )
{
    gceSTATUS status;

    static gctCONST_STRING uName[] =
    {
        "uTexSampler0",
        "uTexSampler1",
        "uTexSampler2",
        "uTexSampler3",
        "uTexSampler4",
        "uTexSampler5",
        "uTexSampler6",
        "uTexSampler7"
    };

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=%d", gc, ShaderControl, Sampler);

    do
    {
        gcmERR_BREAK(glfUsingUniform(
            ShaderControl->i,
            uName[Sampler],
            gcSHADER_SAMPLER_2D,
            1,
            gcvNULL,
            &glmUNIFORM_WRAP_INDEXED(FS, uTexSampler0, Sampler)
            ));

        ShaderControl->i->texture[Sampler]
            = glmUNIFORM_WRAP_INDEXED(FS, uTexSampler0, Sampler);
    }
    while (gcvFALSE);

    gcmFOOTER();

    return status;
}

static gceSTATUS using_uTexCoord(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsing_uTexCoord(
        ShaderControl->i,
        &glmUNIFORM_WRAP(FS, uTexCoord)
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_uPixelTransferScale(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uPixelTransferScale",
        gcSHADER_FRAC_X4,
        1,
        set_uPixelTransferScale,
        &glmUNIFORM_WRAP(FS, uPixelTransferScale)
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_uPixelTransferBias(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingUniform(
        ShaderControl->i,
        "uPixelTransferBias",
        gcSHADER_FRAC_X4,
        1,
        set_uPixelTransferBias,
        &glmUNIFORM_WRAP(FS, uPixelTransferBias)
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_uTextureBorderColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    shaderCaps = &gc->constants.shaderCaps;
    status = glfUsingUniform(
        ShaderControl->i,
        "uTextureBorderColor",
        gcSHADER_FRAC_X4,
        gcmMIN(shaderCaps->maxTextureSamplers,8),
        set_uTextureBorderColor,
        &glmUNIFORM_WRAP(FS, uTextureBorderColor)
        );

    gcmFOOTER();

    return status;
}
/*******************************************************************************
** Varying access helpers.
*/

static gceSTATUS using_vPosition(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingVarying(
        ShaderControl->i,
        "#Position",
        gcSHADER_FLOAT_X4,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP(FS, vPosition),
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_vEyePosition(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingVarying(
        ShaderControl->i,
        "FogFragCoordX",
        gcSHADER_FLOAT_X1,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP(FS, vEyePosition),
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_vColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctINT InputIndex
    )
{
    static gctCONST_STRING vName[] =
    {
        "#FrontColor",
        "#BackColor"
    };
    gceSTATUS status;
    gcSHADER_SHADERMODE shadingMode = gcSHADER_SHADER_DEFAULT;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x InputIndex=%d", gc, ShaderControl, InputIndex);

    if (chipCtx->hashKey.hashShadingMode == 1)
        shadingMode = gcSHADER_SHADER_FLAT;

    status = glfUsingVarying(
        ShaderControl->i,
        vName[InputIndex],
        gcSHADER_FLOAT_X4,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP_INDEXED(FS, vColor0, InputIndex),
        shadingMode
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_vColor2(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctINT InputIndex
    )
{
    static gctCONST_STRING vName[] =
    {
        "#FrontSecondaryColor",
        "#BackSecondaryColor"
    };
    gceSTATUS status;
    gcSHADER_SHADERMODE shadingMode = gcSHADER_SHADER_DEFAULT;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x InputIndex=%d", gc, ShaderControl, InputIndex);

    if (chipCtx->hashKey.hashShadingMode == 1)
        shadingMode = gcSHADER_SHADER_FLAT;

    status = glfUsingVarying(
        ShaderControl->i,
        vName[InputIndex],
        gcSHADER_FLOAT_X4,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP_INDEXED(FS, vColor20, InputIndex),
        shadingMode
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_vTexCoord(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctINT Sampler
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    static gctCONST_STRING vName[] =
    {
        "#TexCoord0",
        "#TexCoord1",
        "#TexCoord2",
        "#TexCoord3",
        "#TexCoord4",
        "#TexCoord5",
        "#TexCoord6",
        "#TexCoord7"
    };
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=%d", gc, ShaderControl, Sampler);

    status = glfUsingVarying(
        ShaderControl->i,
        vName[Sampler],
        chipCtx->texture.sampler[Sampler].coordType,
        1,
        gcvTRUE,
        &glmATTRIBUTE_WRAP_INDEXED(FS, vTexCoord0, Sampler),
        gcSHADER_SHADER_DEFAULT
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_vClipPlane(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctINT ClipPlane
    )
{
    static gctCONST_STRING vName[] =
    {
        "#ClipPlane0",
        "#ClipPlane1",
        "#ClipPlane2",
        "#ClipPlane3",
        "#ClipPlane4",
        "#ClipPlane5"
    };
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x ClipPlane=%d", gc, ShaderControl, ClipPlane);

    status = glfUsingVarying(
        ShaderControl->i,
        vName[ClipPlane],
        gcSHADER_FLOAT_X1,
        1,
        gcvTRUE,
        &glmATTRIBUTE_WRAP_INDEXED(FS, vClipPlane0, ClipPlane),
        gcSHADER_SHADER_DEFAULT
        );

    gcmFOOTER();

    return status;
}

static gceSTATUS using_vPointFade(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    status = glfUsingVarying(
        ShaderControl->i,
        "vPointFade",
        gcSHADER_FLOAT_X1,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP(FS, vPointFade),
        gcSHADER_SHADER_DEFAULT
        );

    gcmFOOTER();
    return status;
}

static gceSTATUS using_vPointSmooth(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingVarying(
        ShaderControl->i,
        "vPointSmooth",
        gcSHADER_FLOAT_X3,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP(FS, vPointSmooth),
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}

static gceSTATUS using_vFace(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    status = glfUsingVarying(
        ShaderControl->i,
        "#FrontFacing",
        gcSHADER_BOOLEAN_X1,
        1,
        gcvFALSE,
        &glmATTRIBUTE_WRAP(FS, vFace),
        gcSHADER_SHADER_DEFAULT
        );
    gcmFOOTER();
    return status;
}


/*******************************************************************************
** Output assigning helpers.
*/

static gceSTATUS assign_oColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16 TempRegister
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x TempRegister=%d", gc, ShaderControl, TempRegister);
    status = gcSHADER_AddOutput(
        ShaderControl->i->shader,
        "#Color",
        gcSHADER_FLOAT_X4,
        1,
        TempRegister,
        gcSHADER_PRECISION_ANY
        );
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  _ClampColor
**
**  Clamp color if needed.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS clampColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Clamp color if needed. */
        if (!ShaderControl->clampColor)
        {
            break;
        }

        /* Allocate a temp for previous color value. */
        ShaderControl->oPrevColor = ShaderControl->oColor;

        /* Allocate color output. */
        glmALLOCATE_TEMP(ShaderControl->oColor);

        /* Clamp. */
        glmOPCODE(SAT, ShaderControl->oColor, XYZW);
            glmTEMP(ShaderControl->oPrevColor, XYZW);

        /* Reset the clamping flag. */
        ShaderControl->clampColor = gcvFALSE;
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _ProcessClipPlane
**
**  Kill pixels if the corresponding value (Ax + By + Cz + Dw) computed
**  by Vertex Shader is less then zero.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS processClipPlane(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        GLint i;

        /* Ignore clip planes if DrawTex extension is in use. */
        if (chipCtx->drawTexOESEnabled)
        {
            break;
        }

        /* Igore clip planes if DrawClearRect is in use. */
        if (chipCtx->drawClearRectEnabled)
        {
            break;
        }

        for (i = 0; i < glvMAX_CLIP_PLANES; i++)
        {
            if (gc->state.enables.transform.clipPlanesMask & (1 << i))
            {
                /* Allocate resources. */
                glmUSING_INDEXED_VARYING(vClipPlane, i);

                /* if (vClipPlane[i] < 0) discard */
                glmOPCODE_BRANCH(KILL, LESS, 0);
                    glmVARYING_INDEXED(FS, vClipPlane0, i, XXXX);
                    glmCONST(0);
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetInitialColor
**
**  Determine preliminary color output.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS getInitialColor(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate color registers. */
        glmALLOCATE_TEMP(ShaderControl->rColor);
        glmALLOCATE_TEMP(ShaderControl->oColor);

        /* Two-sided lighting case. */
        if (gc->state.enables.lighting.lighting &&
            chipCtx->lightingStates.doTwoSidedlighting)
        {
            const gctINT constOne = 1, constZero = 0;

            /* Allocate a label. */
            glmALLOCATE_LOCAL_LABEL(lblFrontFace);

            /* Allocate the varyings. */
            glmUSING_INDEXED_VARYING(vColor, 0);
            glmUSING_INDEXED_VARYING(vColor, 1);
            glmUSING_VARYING(vFace);

            /* Assume front face. */
            glmOPCODE(MOV, ShaderControl->rColor, XYZW);
                glmVARYING(FS, vColor0, XYZW);

            if (chipCtx->hashKey.hasColorSum) {
                glmUSING_INDEXED_VARYING(vColor2, 0);
                glmUSING_INDEXED_VARYING(vColor2, 1);
                glmALLOCATE_TEMP(ShaderControl->rColor2);
                /* Assume front face. */
                glmOPCODE(MOV, ShaderControl->rColor2, XYZW);
                    glmVARYING(FS, vColor20, XYZW);
            }
            /* If the front face specified as clockwise (GL_CW), then we pick
               the front color whenever the area is negative. */
            if (gc->state.polygon.frontFace == GL_CCW)
            {
                /* if (face == 1) goto lblFrontFace. */
                glmOPCODE_BRANCH(JMP, EQUAL, lblFrontFace);
                    glmVARYING(FS, vFace, XXXX);
                    glmINT_CONST(&constOne);
            }

            /* If the front face specified as counterclockwise (GL_CCW),
               then we pick the front color whenever the area is positive. */
            else
            {
                /* if (face == 1) goto lblFrontFace. */
                glmOPCODE_BRANCH(JMP, EQUAL, lblFrontFace);
                    glmVARYING(FS, vFace, XXXX);
                    glmINT_CONST(&constZero);
            }

            {
                /* Back face. */
                glmOPCODE(MOV, ShaderControl->rColor, XYZW);
                    glmVARYING(FS, vColor1, XYZW);
                if (chipCtx->hashKey.hasColorSum) {
                    glmOPCODE(MOV, ShaderControl->rColor2, XYZW);
                        glmVARYING(FS, vColor21, XYZW);
                }
            }

            /* Define label. */
            glmLABEL(lblFrontFace);
        }

        /* Single-sided lighting or color stream. */
        else if ((gc->state.enables.lighting.lighting ||
                 chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX].streamEnabled) &&
                 (!chipCtx->drawClearRectEnabled))
        {
            /* Allocate the varying. */
            glmUSING_INDEXED_VARYING(vColor, 0);

            /* Copy the value. */
            glmOPCODE(MOV, ShaderControl->rColor, XYZW);
                glmVARYING(FS, vColor0, XYZW);

            if (chipCtx->hashKey.hasColorSum) {
                glmUSING_INDEXED_VARYING(vColor2, 0);
                glmALLOCATE_TEMP(ShaderControl->rColor2);
                /* Copy the value. */
                glmOPCODE(MOV, ShaderControl->rColor2, XYZW);
                    glmVARYING(FS, vColor20, XYZW);
            }
        }

        /* Color comes from the constant. */
        else
        {
            /* Allocate the uniform. */
            glmUSING_UNIFORM(uColor);

            /* Copy the value. */
            glmOPCODE(MOV, ShaderControl->rColor, XYZW);
                glmUNIFORM(FS, uColor, XYZW);

            if (chipCtx->hashKey.hasColorSum) {
                glmUSING_UNIFORM(uColor2);
                glmALLOCATE_TEMP(ShaderControl->rColor2);
                /* Copy the value. */
                glmOPCODE(MOV, ShaderControl->rColor2, XYZW);
                    glmUNIFORM(FS, uColor2, XYZW);
            }
        }

        /* Copy the value. */
        glmOPCODE(MOV, ShaderControl->oColor, XYZW);
            glmTEMP(ShaderControl->rColor, XYZW);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetArgumentSource
**
**  Retrieve specified argument source a texture operation.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Source
**          Specifies the source of the argument.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      SourceRegister
**          Allocated source.
*/

static gceSTATUS getArgumentSource(
    __GLcontext * gc,
    IN glsFSCONTROL_PTR ShaderControl,
    IN gleCOMBINESOURCE Source,
    IN gctUINT SamplerNumber,
    OUT gctUINT16_PTR SourceRegister
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Source=0x%04x SamplerNumber=%u SourceRegister=%u",
        gc, ShaderControl, Source, SamplerNumber, SourceRegister);

    do
    {
        /* Allocate a temporary for the argument. */
        glmALLOCATE_LOCAL_TEMP(temp);
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_LABEL(label0);
        glmALLOCATE_LOCAL_LABEL(label1);
        glmALLOCATE_LOCAL_LABEL(label2);
        glmALLOCATE_LOCAL_LABEL(label3);
        /* Argument from texture. */
        if ((Source >= glvTEXTURE) && (Source <= glvTEXTURE7))
        {
            /* Get a shortcut to the sampler. */
            glsTEXTURESAMPLER_PTR sampler;
            __GLtextureObject*    tex;

            if (Source != glvTEXTURE) {
                /* get sample number according texture stage which is specified from Source */
                SamplerNumber = Source - glvTEXTURE0;
            }

            /* Get a shortcut to the sampler. */
            sampler = &chipCtx->texture.sampler[SamplerNumber];
            tex     = gc->texture.units[SamplerNumber].currentTexture;
            gcmASSERT(tex);

            /* Allocate sampler. */
            glmUSING_INDEXED_VARYING(uTexSampler, SamplerNumber);

            /* Coordinate from stream? */
            if (chipCtx->drawTexOESEnabled ||
                chipCtx->attributeInfo[SamplerNumber + __GL_INPUT_TEX0_INDEX].streamEnabled ||
                sampler->genEnable ||
                chipCtx->pointStates.spriteActive)
            {
                /* Allocate varying. */
                glmUSING_INDEXED_VARYING(vTexCoord, SamplerNumber);

                /* Flip texture coordinate for point sprite. */
                if (chipCtx->pointStates.spriteActive)
                {
                    glmALLOCATE_LOCAL_TEMP(coord);

                    glmOPCODE(MOV, coord, XYZW);
                        glmVARYINGV_INDEXED(FS, vTexCoord0, SamplerNumber, sampler->coordSwizzle);

                    /* Flip Y coordinate. */
                    glmOPCODE(SUB, coord, Y);
                        glmCONST(1.0f);
                        glmVARYINGV_INDEXED(FS, vTexCoord0, SamplerNumber, sampler->coordSwizzle);

                    if(sampler->coordType == gcSHADER_FLOAT_X4)
                    /* Define a pre-transform texture coordinate temp register. */
                    {
                        glmALLOCATE_LOCAL_TEMP(predivTexCoord);
                        glmALLOCATE_LOCAL_TEMP(texCoordqrcp);
                        glmALLOCATE_LOCAL_TEMP(TexCoord);

                        /* Set the pre-div q texture coordinate. */
                        glmOPCODE(MOV, predivTexCoord, XYZW);
                            glmTEMP(coord, XYZW);

                        /* Get rcp of q (1/q)*/
                        glmOPCODE(RCP, texCoordqrcp, X);
                            glmTEMP(predivTexCoord, WWWW);

                        /* TexCoord.x = predivTexCoord.x * (1/q) */
                        glmOPCODE(MUL, TexCoord, X);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, XXXX);

                        /* TexCoord.y = predivTexCoord.y * (1/q) */
                        glmOPCODE(MUL, TexCoord, Y);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, YYYY);

                        /* TexCoord.z = predivTexCoord.z * (1/q) */
                        glmOPCODE(MUL, TexCoord, Z);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, ZZZZ);

                        /* Set uninitialize register value to 0.0 */
                        glmOPCODE(MOV, TexCoord, W);
                            glmCONST(0.0);

                        /* Load texture. */
                        glmOPCODE(TEXLD, temp, XYZW);
                            glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
                            glmTEMP(TexCoord, XYZW);
                    }
                    else
                    {
                        /* Load texture. */
                        glmOPCODE(TEXLD, temp, XYZW);
                            glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
                            glmTEMP(coord, XYYY);
                    }
                }
                else
                {
                    if(sampler->coordType == gcSHADER_FLOAT_X4)
                    /* Define a pre-transform texture coordinate temp register. */
                    {
                        glmALLOCATE_LOCAL_TEMP(predivTexCoord);
                        glmALLOCATE_LOCAL_TEMP(texCoordqrcp);
                        glmALLOCATE_LOCAL_TEMP(TexCoord);

                        /* Set the pre-div q texture coordinate. */
                        glmOPCODE(MOV, predivTexCoord, XYZW);
                            glmVARYINGV_INDEXED(FS, vTexCoord0, SamplerNumber, sampler->coordSwizzle);

                        /* Get rcp of q (1/q)*/
                        glmOPCODE(RCP, texCoordqrcp, X);
                            glmTEMP(predivTexCoord, WWWW);

                        /* TexCoord.x = predivTexCoord.x * (1/q) */
                        glmOPCODE(MUL, TexCoord, X);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, XXXX);

                        /* TexCoord.y = predivTexCoord.y * (1/q) */
                        glmOPCODE(MUL, TexCoord, Y);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, YYYY);

                        /* TexCoord.z = predivTexCoord.z * (1/q) */
                        glmOPCODE(MUL, TexCoord, Z);
                            glmTEMP(texCoordqrcp, XXXX);
                            glmTEMP(predivTexCoord, ZZZZ);

                        /* Set uninitialize register value to 0.0 */
                        glmOPCODE(MOV, TexCoord, W);
                            glmCONST(0.0);

                        /* Load texture. */
                        glmOPCODE(TEXLD, temp, XYZW);
                            glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
                            glmTEMP(TexCoord, XYZW);

                        /* simulate the wrap mode GL_CLAMP in LINEAR */
                        if ((tex->params.sWrapMode == GL_CLAMP) &&
                            (tex->params.minFilter == GL_LINEAR) &&
                            (tex->params.magFilter == GL_LINEAR))
                        {
                            glmOPCODE_BRANCH(JMP, GREATER_OR_EQUAL, label0);
                                glmTEMP(TexCoord, XXXX);
                                glmCONST(1.0);

                            glmOPCODE_BRANCH(JMP, LESS, label0);
                                glmTEMP(TexCoord, XXXX);
                                glmCONST(0.0);

                            glmOPCODE_BRANCH(JMP, ALWAYS, label1);

                            glmLABEL(label0);
                            {
                                glmUSING_UNIFORM(uTextureBorderColor);

                                glmOPCODE(ADD, temp1, XYZW);
                                    glmTEMP(temp, XYZW);
                                    glmUNIFORM_STATIC(FS, uTextureBorderColor, XYZW, SamplerNumber);

                                glmOPCODE(MUL, temp, XYZW);
                                    glmTEMP(temp1, XYZW);
                                    glmCONST(0.5);
                            }

                            glmLABEL(label1);
                        }

                        if ((tex->params.tWrapMode == GL_CLAMP) &&
                            (tex->params.minFilter == GL_LINEAR) &&
                            (tex->params.magFilter == GL_LINEAR))
                        {
                            glmOPCODE_BRANCH(JMP, LESS, label2);
                                glmTEMP(TexCoord, YYYY);
                                glmCONST(0.0);

                            glmOPCODE_BRANCH(JMP, GREATER_OR_EQUAL, label2);
                                glmTEMP(TexCoord, YYYY);
                                glmCONST(1.0);

                            glmOPCODE_BRANCH(JMP, ALWAYS, label3);

                            glmLABEL(label2);
                            {
                                glmUSING_UNIFORM(uTextureBorderColor);

                                glmOPCODE(ADD, temp1, XYZW);
                                    glmTEMP(temp, XYZW);
                                    glmUNIFORM_STATIC(FS, uTextureBorderColor, XYZW, SamplerNumber);

                                glmOPCODE(MUL, temp, XYZW);
                                    glmTEMP(temp1, XYZW);
                                    glmCONST(0.5);
                            }

                            glmLABEL(label3);
                        }
                    }
                    else
                    {
                        /* Load texture. */
                        glmOPCODE(TEXLD, temp, XYZW);
                            glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
                            glmVARYINGV_INDEXED(FS, vTexCoord0, SamplerNumber, sampler->coordSwizzle);

                    }
                }
            }
            else
            {
                /* Allocate uniform. */
                glmUSING_UNIFORM(uTexCoord);

                /* Load texture. */
                glmOPCODE(TEXLD, temp, XYZW);
                    glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
                    glmUNIFORM_STATIC(FS, uTexCoord, XYZW, SamplerNumber);
            }
        }

        /* Argument from constant color. */
        else if (Source == glvCONSTANT)
        {
            /* Allocate the constant color uniform. */
            glmUSING_UNIFORM(uTexColor);

            /* Move to the argument register. */
            glmOPCODE(MOV, temp, XYZW);
                glmUNIFORM_STATIC(FS, uTexColor, XYZW, SamplerNumber);
        }

        /* Argument from primary fragment color. */
        else if (Source == glvCOLOR)
        {
            /* Copy predetermined value. */
            glmOPCODE(MOV, temp, XYZW);
                glmTEMP(ShaderControl->rColor, XYZW);
        }

        /* Argument from previous stage result. */
        else if (Source == glvPREVIOUS)
        {
            /* Clamp if needed. */
            if (ShaderControl->clampColor)
            {
                /* Move to the argument register with clamping. */
                glmOPCODE(SAT, temp, XYZW);
                    glmTEMP(ShaderControl->oColor, XYZW);
            }
            else
            {
                /* Move to the argument register. */
                glmOPCODE(MOV, temp, XYZW);
                    glmTEMP(ShaderControl->oColor, XYZW);
            }
        }

        /* Assign the result. */
        *SourceRegister = temp;
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetArgumentOperand
**
**  Retrieve specified argument for a texture operation.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Operand
**          Specifies the type of the operand.
**
**      SamplerNumber
**          Sampler number.
**
**      SourceRegister
**          The source register number.
**
**  OUTPUT:
**
**      ArgumentRegister
**          Allocated argument.
*/

static gceSTATUS getArgumentOperand(
    __GLcontext * gc,
    IN glsFSCONTROL_PTR ShaderControl,
    IN gleCOMBINEOPERAND Operand,
    IN gctUINT SamplerNumber,
    IN gctUINT16 SourceRegister,
    OUT gctUINT16_PTR ArgumentRegister
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x Operand=0x%04x SamplerNumber=%u "
                    "SourceRegister=%u ArgumentRegister=%u",
                    gc, ShaderControl, Operand, SamplerNumber, SourceRegister, ArgumentRegister);

    do
    {
        /* Define a temporary for the argument. */
        glmDEFINE_TEMP(temp);

        /* Argument = Csrc. */
        if (Operand == glvSRCCOLOR)
        {
            /* Set the result. */
            temp = SourceRegister;
        }

        /* Argument = 1 - Csrc. */
        else if (Operand == glvSRCCOLORINV)
        {
            /* Allocate the final argument. */
            glmALLOCATE_TEMP(temp);

            /* temp = 1 - source. */
            glmOPCODE(SUB, temp, XYZW);
                glmCONST(1);
                glmTEMP(SourceRegister, XYZW);
        }

        /* Argument = Asrc. */
        else if (Operand == glvSRCALPHA)
        {
            /* Allocate the final argument. */
            glmALLOCATE_TEMP(temp);

            /* temp2 = temp1.w */
            glmOPCODE(MOV, temp, XYZW);
                glmTEMP(SourceRegister, WWWW);
        }

        /* Argument = 1 - Asrc. */
        else
        {
            gcmASSERT(Operand == glvSRCALPHAINV);

            /* Allocate the final argument. */
            glmALLOCATE_TEMP(temp);

            /* temp2 = 1 - temp1.w */
            glmOPCODE(SUB, temp, XYZW);
                glmCONST(1);
                glmTEMP(SourceRegister, WWWW);
        }

        /* Assign the result. */
        *ArgumentRegister = temp;
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetCombineSources
**
**  Retrieve sources required by the selected texture combine function.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      SamplerNumber
**          Sampler number.
**
**      Combine
**          Pointer to the combine function descriptor.
**
**  OUTPUT:
**
**      Sources[4]
**          Array with allocated sources.
*/

static gceSTATUS getCombineSources(
    __GLcontext * gc,
    IN glsFSCONTROL_PTR ShaderControl,
    IN gctUINT SamplerNumber,
    IN glsTEXTURECOMBINE_PTR Combine,
    OUT gctUINT16 Sources[4]
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    /* Make a shortcut to the current combine function. */
    glsCOMBINEFUNCTION_PTR combineFunction = &_CombineTextureFunctions[Combine->function];

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x SamplerNumber=%u Combine=0x%x Sources=0x%x",
        gc, ShaderControl, SamplerNumber, Combine, Sources);


    /* Loop through 3 possible arguments. */
    for (i = 0; i < 3; i++)
    {
        /* Argument needed? */
        if (combineFunction->haveArg[i])
        {
            /* Get current source type. */
            gleCOMBINESOURCE source = Combine->source[i];

            /* Make a shortcut to the current source register. */
            gctUINT16_PTR sourceRegister = &Sources[source];

            /* Not yet allocated? */
            if (*sourceRegister == 0)
            {
                gcmERR_BREAK(getArgumentSource(
                    gc,
                    ShaderControl,
                    source,
                    SamplerNumber,
                    sourceRegister
                    ));
            }
        }
    }

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _GetCombineArguments
**
**  Retrieve arguments required by the selected texture combine function.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      SamplerNumber
**          Sampler number.
**
**      Combine
**          Pointer to the array of combine function descriptors.
**
**      Sources[4]
**          Array with allocated sources.
**
**      ArgumentMap[4][4]
**          Map between allocated sources and arguments.
**
**  OUTPUT:
**
**      Arguments[3]
**          Arguments for the current function.
*/

static gceSTATUS getCombineArguments(
    __GLcontext * gc,
    IN glsFSCONTROL_PTR ShaderControl,
    IN gctUINT SamplerNumber,
    IN glsTEXTURECOMBINE_PTR Combine,
    IN gctUINT16 Sources[4],
    IN gctUINT16 ArgumentMap[4][4],
    OUT gctUINT16 Arguments[3]
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    /* Make a shortcut to the current combine function. */
    glsCOMBINEFUNCTION_PTR combineFunction = &_CombineTextureFunctions[Combine->function];

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x SamplerNumber=%u Combine=0x%x "
                    "Sources=0x%x ArgumentMap=0x%x Arguments=0x%x",
                    gc, ShaderControl, SamplerNumber, Combine, Sources, ArgumentMap, Arguments);

    /* Loop through 3 possible arguments. */
    for (i = 0; i < 3; i++)
    {
        /* Argument needed? */
        if (combineFunction->haveArg[i])
        {
            /* Get current source and operand type. */
            gleCOMBINESOURCE  source  = Combine->source[i];
            gleCOMBINEOPERAND operand = Combine->operand[i];

            /* Make a shortcut to the current argument register. */
            gctUINT16_PTR argumentRegister =
                &ArgumentMap[source][operand];

            /* Not yet allocated? */
            if (*argumentRegister == 0)
            {
                gcmERR_BREAK(getArgumentOperand(
                    gc,
                    ShaderControl,
                    operand,
                    SamplerNumber,
                    Sources[source],
                    argumentRegister
                    ));
            }

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            /* Report source allocation. */
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "           Source %d: from %s(reg=%d),",
                i,
                g_sourceName[source], Sources[source]
                );

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "                      take %s(reg=%d).",
                g_operandName[operand], *argumentRegister
                );
#endif

            /* Set the output. */
            Arguments[i] = *argumentRegister;
        }
    }

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncReplace
**
**  REPLACE texture combine function:
**
**      oColor = Arg0
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncReplace(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        glmOPCODEV(MOV, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncModulate
**
**  MODULATE texture combine function:
**
**      oColor = Arg0 * Arg1
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncModulate(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        glmOPCODEV(MUL, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncAdd
**
**  ADD texture combine function:
**
**      oColor = Arg0 + Arg1
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncAdd(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        glmOPCODEV(ADD, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncAddSigned
**
**  ADD_SIGNED texture combine function:
**
**      oColor = Arg0 + Arg1 - 0.5
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncAddSigned(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        /* Allocate a temp. */
        glmALLOCATE_LOCAL_TEMP(temp);

        glmOPCODEV(ADD, temp, CombineFlow->tempEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);

        glmOPCODEV(SUB, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(temp, CombineFlow->tempSwizzle);
            glmCONST(0.5f);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncInterpolate
**
**  INTERPOLATE texture combine function:
**
**      oColor = Arg0 * Arg2 + Arg1 * (1 ? Arg2)
**             = Arg0 * Arg2 + Arg1 - Arg1 * Arg2
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncInterpolate(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        /* temp1 = Arg0 * Arg2. */
        glmOPCODEV(MUL, temp1, CombineFlow->tempEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[2], CombineFlow->argSwizzle);

        /* temp2 = Arg0 * Arg2 + Arg1. */
        glmOPCODEV(ADD, temp2, CombineFlow->tempEnable);
            glmTEMPV(temp1, CombineFlow->tempSwizzle);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);

        /* temp3 = Arg1 * Arg2 */
        glmOPCODEV(MUL, temp3, CombineFlow->tempEnable);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[2], CombineFlow->argSwizzle);

        /* Finally compute the result. */
        glmOPCODEV(SUB, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(temp2, CombineFlow->tempSwizzle);
            glmTEMPV(temp3, CombineFlow->tempSwizzle);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncSubtract
**
**  SUBTRACT texture combine function:
**
**      oColor = Arg0 - Arg1
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncSubtract(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        glmOPCODEV(SUB, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMPV(Arguments[0], CombineFlow->argSwizzle);
            glmTEMPV(Arguments[1], CombineFlow->argSwizzle);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncDot3RGB
**
**  DOT3_RGB texture combine function:
**
**      oColor.xyz = 4 * ( (Arg0r - 0.5) * (Arg1r - 0.5) +
**                         (Arg0g - 0.5) * (Arg1g - 0.5) +
**                         (Arg0b - 0.5) * (Arg1b - 0.5) )
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncDot3RGB(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        /* temp1 = Arg0 - 0.5 */
        glmOPCODE(SUB, temp1, XYZ);
            glmTEMP(Arguments[0], XYZZ);
            glmCONST(0.5f);

        /* temp2 = Arg1 - 0.5 */
        glmOPCODE(SUB, temp2, XYZ);
            glmTEMP(Arguments[1], XYZZ);
            glmCONST(0.5f);

        /* temp3 = dp3(temp1, temp2) */
        glmOPCODE(DP3, temp3, X);
            glmTEMP(temp1, XYZZ);
            glmTEMP(temp2, XYZZ);

        /* oColor = dp3 * 4 */
        glmOPCODEV(MUL, ShaderControl->oColor, CombineFlow->targetEnable);
            glmTEMP(temp3, XXXX);
            glmCONST(4);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexCombFuncDot3RGBA
**
**  DOT3_RGBA texture combine function:
**
**      oColor.xyzw = 4 * ( (Arg0r - 0.5) * (Arg1r - 0.5) +
**                          (Arg0g - 0.5) * (Arg1g - 0.5) +
**                          (Arg0b - 0.5) * (Arg1b - 0.5) )
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Arguments
**          Pointer to the array of input arguments.
**
**      CombineFlow
**          Data flow control.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texCombFuncDot3RGBA(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    gctUINT16_PTR Arguments,
    glsCOMBINEFLOW_PTR CombineFlow
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Arguments=%u CombineFlow=0x%x",
                    gc, ShaderControl, Arguments, CombineFlow);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        /* temp1 = Arg0 - 0.5 */
        glmOPCODE(SUB, temp1, XYZ);
            glmTEMP(Arguments[0], XYZZ);
            glmCONST(0.5f);

        /* temp2 = Arg1 - 0.5 */
        glmOPCODE(SUB, temp2, XYZ);
            glmTEMP(Arguments[1], XYZZ);
            glmCONST(0.5f);

        /* temp3 = dp3(temp1, temp2) */
        glmOPCODE(DP3, temp3, X);
            glmTEMP(temp1, XYZZ);
            glmTEMP(temp2, XYZZ);

        /* oColor = dp3 * 4 */
        glmOPCODE(MUL, ShaderControl->oColor, XYZW);
            glmTEMP(temp3, XXXX);
            glmCONST(4);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

static gctBOOL checkDoClamp(
    glsFSCONTROL_PTR ShaderControl,
    IN glsTEXTURECOMBINE_PTR Combine,
    IN gctUINT CombineCount
    )
{
    gctBOOL doClamp = gcvFALSE;

    gcmHEADER_ARG("ShaderControl=0x%x Combine=0x%x CombineCount=%u",
                    ShaderControl, Combine, CombineCount);

    switch (Combine[0].function)
    {
    case glvCOMBINEADD       :
    case glvCOMBINEADDSIGNED :
    case glvCOMBINESUBTRACT  :
    case glvCOMBINEDOT3RGB   :
    case glvCOMBINEDOT3RGBA  :
        doClamp = gcvTRUE;
        break;
    default:
        break;
    }

    if (doClamp || CombineCount == 1)
    {
        gcmFOOTER_ARG("%d", doClamp);
        return doClamp;
    }

    switch (Combine[1].function)
    {
    case glvCOMBINEADD       :
    case glvCOMBINEADDSIGNED :
    case glvCOMBINESUBTRACT  :
    case glvCOMBINEDOT3RGB   :
    case glvCOMBINEDOT3RGBA  :
        doClamp = gcvTRUE;
        break;
    default:
        break;
    }

    gcmFOOTER_ARG("%d", doClamp);
    return doClamp;
}

/*******************************************************************************
**
**  _TexFuncCombineComponent
**
**  Perform preconfigured texture combine function.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      SamplerNumber
**          Sampler number.
**
**      TargetEnable
**          Format dependent target enable.
**
**      Combine
**          Pointer to the array of combine function descriptors.
**
**      CombineCount
**          Number of combine function descriptors in the array.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncCombineComponent(
    __GLcontext * gc,
    IN glsFSCONTROL_PTR ShaderControl,
    IN gctUINT SamplerNumber,
    IN gcSL_ENABLE TargetEnable,
    IN glsTEXTURECOMBINE_PTR Combine,
    IN gctUINT CombineCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x SamplerNumber=%u TargetEnable=0x%04x "
                    "Combine=0x%x CombineCount=%u",
                    gc, ShaderControl, SamplerNumber, TargetEnable, Combine, CombineCount);

    do
    {
        gctUINT i;

        /* Determine the scaling enable flag. */
        gctBOOL doScale = (CombineCount == 1)
            ? !Combine[0].scale.one
            : (!Combine[0].scale.one || !Combine[1].scale.one);

        /* There are four different types of sources defined. The algorithm
           will reuse a source instead of allocating a new one if the same
           source is used more then once. Therefore, we only need up to 4
           source registers for both color and alpha functions. */
        gctUINT16 sources[4];

        /* Argument map between argument and source type. */
        gctUINT16 argumentMap[4][4];

        /* Reset sources and argument map. */
        gcoOS_ZeroMemory(sources, sizeof(sources));
        gcoOS_ZeroMemory(argumentMap, sizeof(argumentMap));

        /* Loop through combine descriptors. */
        for (i = 0; i < CombineCount; i++)
        {
            /* Arguments for the current function. */
            gctUINT16 arguments[3];

            /* Make a shortcut to the current combine function descriptor. */
            glsTEXTURECOMBINE_PTR combine = &Combine[i];

            /* Make a shortcut to the current combine function. */
            glsCOMBINEFUNCTION_PTR combineFunction = &_CombineTextureFunctions[combine->function];

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            /* Report texture function. */
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "         Pass=%s,",
                g_passName[i]
                );

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "             function=%s, scale factor=%.5f.",
                g_combineFunctionName[combine->function],
                //glfFloatFromMutant(&combine->scale)
                gcChipUtilFloatFromMutant(&combine->scale)
                );
#endif

            /* Generate function only if it produces a result. */
            if (combine->combineFlow->targetEnable & TargetEnable)
            {
                /* Allocate sources. */
                gcmERR_BREAK(getCombineSources(
                    gc,
                    ShaderControl,
                    SamplerNumber,
                    combine,
                    sources
                    ));

                /* Fetch source arguments. */
                gcmERR_BREAK(getCombineArguments(
                    gc,
                    ShaderControl,
                    SamplerNumber,
                    combine,
                    sources,
                    argumentMap,
                    arguments
                    ));

                /* Save the current color as previous. */
                ShaderControl->oPrevColor = ShaderControl->oColor;

                /* Allocate new color register. */
                glmALLOCATE_TEMP(ShaderControl->oColor);

                /* Generate function. */
                gcmERR_BREAK((*combineFunction->function)(
                    gc,
                    ShaderControl,
                    arguments,
                    combine->combineFlow
                    ));

                /* Copy the leftover part of the color. */
                if (combine->combineFlow->targetEnable == gcSL_ENABLE_XYZ)
                {
                    glmOPCODE(MOV, ShaderControl->oColor, W);
                        glmTEMP(ShaderControl->oPrevColor, WWWW);
                }
                else if (combine->combineFlow->targetEnable == gcSL_ENABLE_W)
                {
                    glmOPCODE(MOV, ShaderControl->oColor, XYZ);
                        glmTEMP(ShaderControl->oPrevColor, XYZZ);
                }
            }
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            else
            {
                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                    "         Skipped."
                    );
            }
#endif

            /* Accordingly to the spec, the alpha result is ignored if RGB
               is set to be computed using DOT3RGBA function. */
            if (combine->function == glvCOMBINEDOT3RGBA)
            {
                break;
            }
        }

        /* Scale the result. */
        if (doScale)
        {
            /* Save the current color as previous. */
            ShaderControl->oPrevColor = ShaderControl->oColor;

            /* Allocate new color register. */
            glmALLOCATE_TEMP(ShaderControl->oColor);

            /* Allocate scale constant. */
            glmUSING_UNIFORM(uTexCombScale);

            glmOPCODE(MUL, ShaderControl->oColor, XYZW);
                glmTEMP(ShaderControl->oPrevColor, XYZW);
                glmUNIFORM_STATIC(FS, uTexCombScale, XYZW, SamplerNumber);
        }

        /* Schedule to clamp the final result. */
        ShaderControl->clampColor = doScale
                                    ? gcvTRUE
                                    : checkDoClamp(ShaderControl,
                                                    Combine,
                                                    CombineCount);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncReplace
**
**  REPLACE texture function:
**
**      oColor.color = Csrc
**      oColor.alpha = Asrc
**
**      We are using REPLACE function to implement the formula:
**
**          oColor = Arg0
**
**              where Arg0 = source texture
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncReplace(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {

        if (Sampler->binding->format == GL_RGBA ||
            Sampler->binding->format == GL_LUMINANCE_ALPHA ||
            Sampler->binding->format == GL_INTENSITY)
        {
            /*C = Cs   A = As */
            static glsTEXTURECOMBINE combine =
            {
                glvCOMBINEREPLACE,

                /* Arg0                  Arg1               Arg2 */
                {glvTEXTURE,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                {glvSRCCOLOR, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                /* Scale. */
                glmFIXEDMUTANTONE,

                /* Combine data flow control structure. */
                gcvNULL
            };

            /* Arg0                  Arg1               Arg2 */
            combine.combineFlow = &Sampler->binding->combineFlow;

            /* Scale. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                Sampler->binding->combineFlow.targetEnable,
                &combine,
                1
                ));
        }
        else if (Sampler->binding->format == GL_RGB ||
                 Sampler->binding->format == GL_LUMINANCE)
        {
            /* C = Cs, A = Af */
            static glsCOMBINEFLOW colorDataFlow =
            {
                gcSL_ENABLE_XYZ,        /* Target enable. */
                gcSL_ENABLE_XYZ,        /* Temp enable. */
                gcSL_SWIZZLE_XYZZ,      /* Temp swizzle. */
                gcSL_SWIZZLE_XYZZ       /* Source argument swizzle. */
            };

            static glsCOMBINEFLOW alphaDataFlow =
            {
                gcSL_ENABLE_W,          /* Target enable. */
                gcSL_ENABLE_X,          /* Temp enable. */
                gcSL_SWIZZLE_XXXX,      /* Temp swizzle. */
                gcSL_SWIZZLE_WWWW       /* Source argument swizzle. */
            };

            static glsTEXTURECOMBINE combine[] =
            {
                {
                    glvCOMBINEREPLACE,

                    /* Arg0                  Arg1               Arg2 */
                    {glvTEXTURE,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                    {glvSRCCOLOR, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &colorDataFlow
                },

                {
                    glvCOMBINEREPLACE,

                    /* Arg0                  Arg1               Arg2 */
                    {glvPREVIOUS,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                    {glvSRCALPHA, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &alphaDataFlow
                }
            };

            /* Combine data flow control structure. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                Sampler->binding->combineFlow.targetEnable,
                combine,
                gcmCOUNTOF(combine)
                ));
        }
        else
        {
            /* C = Cf, A = As */

            static glsCOMBINEFLOW colorDataFlow =
            {
                gcSL_ENABLE_XYZ,        /* Target enable. */
                gcSL_ENABLE_XYZ,        /* Temp enable. */
                gcSL_SWIZZLE_XYZZ,      /* Temp swizzle. */
                gcSL_SWIZZLE_XYZZ       /* Source argument swizzle. */
            };

            static glsCOMBINEFLOW alphaDataFlow =
            {
                gcSL_ENABLE_W,          /* Target enable. */
                gcSL_ENABLE_X,          /* Temp enable. */
                gcSL_SWIZZLE_XXXX,      /* Temp swizzle. */
                gcSL_SWIZZLE_WWWW       /* Source argument swizzle. */
            };

            static glsTEXTURECOMBINE combine[] =
            {
                {
                    glvCOMBINEREPLACE,

                    /* Arg0                  Arg1               Arg2 */
                    {glvPREVIOUS,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                    {glvSRCCOLOR, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &colorDataFlow
                },

                {
                    glvCOMBINEREPLACE,

                    /* Arg0                  Arg1               Arg2 */
                    {glvTEXTURE,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                    {glvSRCALPHA, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &alphaDataFlow
                }
            };

            gcmASSERT(Sampler->binding->format == GL_ALPHA);

            /* Generate function. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                Sampler->binding->combineFlow.targetEnable,
                combine,
                gcmCOUNTOF(combine)
                ));
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncModulate
**
**  MODULATE texture function:
**
**      oColor.color = Cprev * Csrc
**      oColor.alpha = Aprev * Asrc
**
**      We are using MODULATE function to implement the formula:
**
**          oColor = Arg0 * Arg1
**
**              where Arg0 = previous result,
**                    Arg1 = texture value.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncModulate(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        static glsTEXTURECOMBINE combine =
        {
            glvCOMBINEMODULATE,

            /* Arg0          Arg1              Arg2 */
            {glvPREVIOUS, glvTEXTURE,  (gleCOMBINESOURCE)  0},
            {glvSRCCOLOR, glvSRCCOLOR, (gleCOMBINEOPERAND) 0},

            /* Scale. */
            glmFIXEDMUTANTONE,

            /* Combine data flow control structure. */
            gcvNULL
        };

        /* Texture format defines the data flow. */
        combine.combineFlow = &Sampler->binding->combineFlow;

        /* Generate function. */
        gcmERR_BREAK(texFuncCombineComponent(
            gc,
            ShaderControl,
            SamplerNumber,
            Sampler->binding->combineFlow.targetEnable,
            &combine,
            1
            ));
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncDecal
**
**  DECAL texture function:
**
**      RGB format:
**
**          oColor.color = Csrc
**          oColor.alpha = Aprev
**
**          We are using REPLACE function to implement the formula:
**
**              oColor = Arg0
**
**                  where Arg0 = Csrc
**
**      RGBA format:
**
**          oColor.color = Cprev * (1 - Asrc) + Csrc * Asrc
**          oColor.alpha = Aprev
**
**          We are using INTERPOLATE combine function to implement the formula:
**
**              oColor.color = Arg0 * Arg2 + Arg1 * (1 ? Arg2)
**
**                  where Arg0 = Csrc
**                        Arg1 = Cprev
**                        Arg2 = Asrc
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncDecal(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        if (Sampler->binding->format == GL_RGB)
        {
            gcmERR_BREAK(texFuncReplace(
                gc,
                ShaderControl,
                Sampler,
                SamplerNumber
                ));
        }
        else if ((Sampler->binding->format == GL_RGBA) ||
                 (Sampler->binding->format == GL_BGRA_EXT))
        {
            static glsCOMBINEFLOW colorDataFlow =
            {
                gcSL_ENABLE_XYZ,        /* Target enable. */
                gcSL_ENABLE_XYZ,        /* Temp enable. */
                gcSL_SWIZZLE_XYZZ,       /* Temp swizzle. */
                gcSL_SWIZZLE_XYZZ        /* Source argument swizzle. */
            };

            static glsCOMBINEFLOW alphaDataFlow =
            {
                gcSL_ENABLE_W,          /* Target enable. */
                gcSL_ENABLE_X,          /* Temp enable. */
                gcSL_SWIZZLE_XXXX,      /* Temp swizzle. */
                gcSL_SWIZZLE_WWWW       /* Source argument swizzle. */
            };

            static glsTEXTURECOMBINE combine[] =
            {
                {
                    glvCOMBINEINTERPOLATE,

                    /* Arg0       Arg1         Arg2 */
                    {glvTEXTURE,  glvPREVIOUS, glvTEXTURE},
                    {glvSRCCOLOR, glvSRCCOLOR, glvSRCALPHA},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &colorDataFlow
                },

                {
                    glvCOMBINEREPLACE,

                    /* Arg0                  Arg1               Arg2 */
                    {glvPREVIOUS,  (gleCOMBINESOURCE)  0, (gleCOMBINESOURCE)  0},
                    {glvSRCALPHA, (gleCOMBINEOPERAND) 0, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &alphaDataFlow
                }
            };

            /* Generate function. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                gcSL_ENABLE_XYZW,
                combine,
                gcmCOUNTOF(combine)
                ));
        }
        else
        {
            /* For the rest of the formats the result is undefined. */
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncBlend
**
**  BLEND texture function:
**
**      oColor.color = Cprev * (1 - Csrc) + Cconst * Csrc
**
**          We are using INTERPOLATE combine function to implement the formula:
**
**          oColor.color = Arg0 * Arg2 + Arg1 * (1 ? Arg2)
**
**              where Arg0 = Cconst
**                    Arg1 = Cprev
**                    Arg2 = Csrc
**
**      oColor.alpha = Aprev * Asrc
**
**          We are using MODULATE combine function to implement the formula:
**
**          oColor.color = Arg0 * Arg1
**
**              where Arg0 = Aprev
**                    Arg1 = Asrc
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncBlend(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        if (Sampler->binding->format == GL_INTENSITY)
        {
            static glsCOMBINEFLOW dataFlow =
            {
                gcSL_ENABLE_XYZW,        /* Target enable. */
                gcSL_ENABLE_XYZW,        /* Temp enable. */
                gcSL_SWIZZLE_XYZW,      /* Temp swizzle. */
                gcSL_SWIZZLE_XYZW       /* Source argument swizzle. */
            };

            static glsTEXTURECOMBINE combine[] =
            {
                /* RGB function. */
                {
                    glvCOMBINEINTERPOLATE,

                    /* Arg0       Arg1         Arg2 */
                    {glvCONSTANT, glvPREVIOUS, glvTEXTURE},
                    {glvSRCCOLOR, glvSRCCOLOR, glvSRCCOLOR},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &dataFlow
                },
            };

            /* Generate function. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                Sampler->binding->combineFlow.targetEnable,
                combine,
                gcmCOUNTOF(combine)
                ));
        }
        else
        {
            static glsCOMBINEFLOW colorDataFlow =
            {
                gcSL_ENABLE_XYZ,        /* Target enable. */
                gcSL_ENABLE_XYZ,        /* Temp enable. */
                gcSL_SWIZZLE_XYZZ,      /* Temp swizzle. */
                gcSL_SWIZZLE_XYZZ       /* Source argument swizzle. */
            };

            static glsCOMBINEFLOW alphaDataFlow =
            {
                gcSL_ENABLE_W,          /* Target enable. */
                gcSL_ENABLE_X,          /* Temp enable. */
                gcSL_SWIZZLE_XXXX,      /* Temp swizzle. */
                gcSL_SWIZZLE_WWWW       /* Source argument swizzle. */
            };

            static glsTEXTURECOMBINE combine[] =
            {
                /* Arg0       Arg1         Arg2 */
                {
                    glvCOMBINEINTERPOLATE,

                    /* Arg0       Arg1         Arg2 */
                    {glvCONSTANT, glvPREVIOUS, glvTEXTURE},
                    {glvSRCCOLOR, glvSRCCOLOR, glvSRCCOLOR},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &colorDataFlow
                },

                /* Arg0       Arg1         Arg2 */
                {
                    glvCOMBINEMODULATE,

                    /* Arg0       Arg1         Arg2 */
                    {glvPREVIOUS, glvTEXTURE,  (gleCOMBINESOURCE)  0},
                    {glvSRCALPHA, glvSRCALPHA, (gleCOMBINEOPERAND) 0},

                    /* Scale. */
                    glmFIXEDMUTANTONE,

                    /* Combine data flow control structure. */
                    &alphaDataFlow
                }
            };

            /* Generate function. */
            gcmERR_BREAK(texFuncCombineComponent(
                gc,
                ShaderControl,
                SamplerNumber,
                Sampler->binding->combineFlow.targetEnable,
                combine,
                gcmCOUNTOF(combine)
                ));
        }
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncAdd
**
**  ADD texture function:
**
**      oColor.color = Cprev + Csrc
**
**          We are using ADD combine function to implement the formula:
**
**          oColor.color = Arg0 + Arg1
**
**              where Arg0 = Cprev
**                    Arg1 = Csrc
**
**      oColor.alpha = Aprev * Asrc
**
**          We are using MODULATE combine function to implement the formula:
**
**          oColor.color = Arg0 * Arg1
**
**              where Arg0 = Aprev
**                    Arg1 = Asrc
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncAdd(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        static glsCOMBINEFLOW colorDataFlow =
        {
            gcSL_ENABLE_XYZ,        /* Target enable. */
            gcSL_ENABLE_XYZ,        /* Temp enable. */
            gcSL_SWIZZLE_XYZZ,      /* Temp swizzle. */
            gcSL_SWIZZLE_XYZZ       /* Source argument swizzle. */
        };

        static glsCOMBINEFLOW alphaDataFlow =
        {
            gcSL_ENABLE_W,          /* Target enable. */
            gcSL_ENABLE_X,          /* Temp enable. */
            gcSL_SWIZZLE_XXXX,      /* Temp swizzle. */
            gcSL_SWIZZLE_WWWW       /* Source argument swizzle. */
        };

        static glsTEXTURECOMBINE combine[] =
        {
            /* RGB function. */
            {
                glvCOMBINEADD,

                /* Arg0       Arg1         Arg2 */
                {glvPREVIOUS, glvTEXTURE,  (gleCOMBINESOURCE)  0},
                {glvSRCCOLOR, glvSRCCOLOR, (gleCOMBINEOPERAND) 0},

                /* Scale. */
                glmFIXEDMUTANTONE,

                /* Combine data flow control structure. */
                &colorDataFlow
            },

            /* Alpha function. */
            {
                glvCOMBINEMODULATE,

                /* Arg0       Arg1         Arg2 */
                {glvPREVIOUS, glvTEXTURE,  (gleCOMBINESOURCE)  0},
                {glvSRCALPHA, glvSRCALPHA, (gleCOMBINEOPERAND) 0},

                /* Scale. */
                glmFIXEDMUTANTONE,

                /* Combine data flow control structure. */
                &alphaDataFlow
            }
        };

        /* Generate function. */
        gcmERR_BREAK(texFuncCombineComponent(
            gc,
            ShaderControl,
            SamplerNumber,
            Sampler->binding->combineFlow.targetEnable,
            combine,
            gcmCOUNTOF(combine)
            ));
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncCombine
**
**  COMBINE texture function.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncCombine(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        glsTEXTURECOMBINE combine[2];

        /* Set combine functions. */
        combine[0] = Sampler->combColor;
        combine[1] = Sampler->combAlpha;

        /* Generate function. */
        gcmERR_BREAK(texFuncCombineComponent(
            gc,
            ShaderControl,
            SamplerNumber,
            gcSL_ENABLE_XYZW,
            combine,
            gcmCOUNTOF(combine)
            ));
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _TexFuncStipple
**
**  Stipple texture function, used only for bitmap simulation.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**      Sampler
**          Pointer to a sampler descriptor.
**
**      SamplerNumber
**          Sampler number.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS texFuncStipple(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x Sampler=0x%x SamplerNumber=%u",
                    gc, ShaderControl, Sampler, SamplerNumber);

    do
    {
        glmALLOCATE_LOCAL_TEMP(temp);

        /* Allocate sampler. */
        glmUSING_INDEXED_VARYING(uTexSampler, SamplerNumber);
        glmUSING_INDEXED_VARYING(vTexCoord, SamplerNumber);

        /* Load texture. */
        glmOPCODE(TEXLD, temp, XYZW);
            glmUNIFORM_INDEXED(FS, uTexSampler0, SamplerNumber, XYZW);
            glmVARYINGV_INDEXED(FS, vTexCoord0, SamplerNumber, Sampler->coordSwizzle);

        glmOPCODE_BRANCH(KILL, LESS_OR_EQUAL, 0);
            glmTEMP(temp, XXXX);
            glmCONST(0);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}



#if gcmIS_DEBUG(gcdDEBUG_TRACE)
/*******************************************************************************
**
**  _GetTextureFormatName
**
**  Return the string name of the specified texture format.
**
**  INPUT:
**
**      Format
**          Enumerated GL name.
**
**  OUTPUT:
**
**      String name.
*/

static char* _GetTextureFormatName(
    GLenum Format
    )
{
    gcmHEADER_ARG("Format=0x%04x", Format);

    if ((Format >= GL_ALPHA) && (Format <= GL_LUMINANCE_ALPHA))
    {
        static char* _textureFormats[] =
        {
            "GL_ALPHA",
            "GL_RGB",
            "GL_RGBA",
            "GL_LUMINANCE",
            "GL_LUMINANCE_ALPHA"
        };

        gcmFOOTER_ARG("result=0x%x", _textureFormats[Format - GL_ALPHA]);

        return _textureFormats[Format - GL_ALPHA];
    }
    else
    {
        gcmASSERT(Format == GL_BGRA_EXT || Format == GL_INTENSITY);
        if (Format == GL_BGRA_EXT)
        {
            gcmFOOTER_ARG("result=0x%x", "GL_BGRA_EXT");
            return "GL_BGRA_EXT";
        }
        else
        {
            gcmFOOTER_ARG("result=0x%x", "GL_INTENSITY");
            return "GL_INTENSITY";
        }
    }
}
#endif


/*******************************************************************************
**
**  _ProcessTexture
**
**  Generate texture stage functions for enabled samplers.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS processTexture(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcsGLSLCaps *shaderCaps = gcvNULL;
    GLuint i = 0;
    GLuint numOfTex;
    GLuint texEnable;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    if (chipCtx->drawClearRectEnabled)
    {
        gcmFOOTER();
        return status;
    }

    texEnable = chipCtx->texture.stageEnabledMask;
    shaderCaps = &gc->constants.shaderCaps;
    numOfTex = gcmMIN(shaderCaps->maxTextureSamplers,8);


    while ((i < numOfTex) && texEnable)
    {
        glsTEXTURESAMPLER_PTR sampler = &chipCtx->texture.sampler[i];

        if (texEnable & 1)
        {
            gcmASSERT(sampler->binding != gcvNULL);

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            /* Report texture function. */
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "[FS11] Stage=%d, format=%s,",
                i,
                _GetTextureFormatName(sampler->binding->format)
                );

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcvZONE_TEXTURE,
                "       texture function=%s.",
                g_functionName[sampler->function]
                );
#endif

            /* Generate function. */
            gcmERR_BREAK((*sampler->doTextureFunction)(
                gc,
                (GLvoid * )ShaderControl,
                sampler,
                i
                ));
        }
        i++;
        texEnable >>= 1;
    }

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _ProcessFog
**
**  Fog blends a fog color with a rasterized fragment's post-texturing
**  color using a blending factor. The blending factor is computed and passed
**  on to FS from VS. If Cr represents a rasterized fragment’s R, G, or B
**  value, then the corresponding value produced by fog is
**
**      C = f * Cr + (1 - f) * Cf,
**
**  The rasterized fragment’s A value is not changed by fog blending.
**  The R, G, B, and A values of Cf are specified by calling Fog with name
**  equal to FOG_COLOR.
**
**  We compute the color using transformed formula:
**
**      C = f * Cr + (1 - f) * Cf = f * Cr + Cf - f * Cf
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS processFog(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(eyeDistance);
        glmALLOCATE_LOCAL_TEMP(fogFactor);
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        /* Allocate resources. */
        glmUSING_UNIFORM(uFogFactors);
        glmUSING_UNIFORM(uFogColor);
        glmUSING_VARYING(vEyePosition);

        /***********************************************************************
        ** Compute the distance from the eye to the fragment. Per spec there
        ** is a proper way to do it by computing the actual distance from the
        ** fragment to the eye and then there is also an easy way out - to take
        ** Z value of the fragment position for the distance. It seems most
        ** vendors take the easy way, so we'll do the same here.
        */

        /* Take the absolute Z value. */
        glmOPCODE(ABS, eyeDistance, X);
            glmVARYING(FS, vEyePosition, XXXX);

        /***********************************************************************
        ** Compute the fog factor.
        */

        if (gc->state.fog.mode == GL_LINEAR)
        {
            /*******************************************************************
            ** factor = c * [(1 / (s - e))] + [e / (e - s)]
            **
            ** uFogFactors.x = [(1 / (s - e))]
            ** uFogFactors.y = [ e / (e - s) ]
            */

            /* Allocate temporaries. */
            glmALLOCATE_LOCAL_TEMP(_temp1);
            glmALLOCATE_LOCAL_TEMP(_temp2);

            /* _temp1.x = c * [(1 / (s - e))] */
            glmOPCODE(MUL, _temp1, X);
                glmTEMP(eyeDistance, XXXX);
                glmUNIFORM(FS, uFogFactors, XXXX);

            /* _temp2.x = c * [(1 / (s - e))] + [e / (e - s) */
            glmOPCODE(ADD, _temp2, X);
                glmTEMP(_temp1, XXXX);
                glmUNIFORM(FS, uFogFactors, YYYY);

            /* fogFactor = sat(_temp3.x) */
            glmOPCODE(SAT, fogFactor, X);
                glmTEMP(_temp2, XXXX);
        }
        else
        {
            /*******************************************************************
            **            -(density * c)
            ** factor = e
            **
            ** or
            **            -(density * c)^2
            ** factor = e
            **
            ** uFogFactors.x = density
            */

            /* Allocate temporaries. */
            glmALLOCATE_LOCAL_TEMP(_temp1);
            glmDEFINE_TEMP        (_temp2);
            glmALLOCATE_LOCAL_TEMP(_temp3);
            glmALLOCATE_LOCAL_TEMP(_temp4);

            /* Allocate resources. */
            glmUSING_UNIFORM(uFogFactors);

            /* _temp1.x = density * c */
            glmOPCODE(MUL, _temp1, X);
                glmUNIFORM(FS, uFogFactors, XXXX);
                glmTEMP(eyeDistance, XXXX);

            /* Non-squared case? */
                if (gc->state.fog.mode == GL_EXP)
            {
                /* Yes, just assign the register. */
                _temp2 = _temp1;
            }
            else
            {
                /* Allocate another temp. */
                glmALLOCATE_TEMP(_temp2);

                /* _temp2.x = _temp1.x * _temp1.x */
                glmOPCODE(MUL, _temp2, X);
                    glmTEMP(_temp1, XXXX);
                    glmTEMP(_temp1, XXXX);
            }

            /* _temp3.x = -_temp2.x */
            glmOPCODE(SUB, _temp3, X);
                glmCONST(0);
                glmTEMP(_temp2, XXXX);

            /* _temp4.x = exp(_temp3.x) */
            glmOPCODE(EXP, _temp4, X);
                glmTEMP(_temp3, XXXX);

            /* fogFactor = sat(_temp4.x) */
            glmOPCODE(SAT, fogFactor, X);
                glmTEMP(_temp4, XXXX);
        }

        /***********************************************************************
        ** Blend the colors.
        */

        /* Clamp color. */
        gcmERR_BREAK(clampColor(gc, ShaderControl));

        /* Allocate a temp for previous color value. */
        ShaderControl->oPrevColor = ShaderControl->oColor;

        /* Allocate color output. */
        glmALLOCATE_TEMP(ShaderControl->oColor);

        /* temp1 = f * Cr. */
        glmOPCODE(MUL, temp1, XYZ);
            glmTEMP(fogFactor, XXXX);
            glmTEMP(ShaderControl->oPrevColor, XYZZ);

        /* temp2 = f * Cr + Cf. */
        glmOPCODE(ADD, temp2, XYZ);
            glmTEMP(temp1, XYZZ);
            glmUNIFORM(FS, uFogColor, XYZZ);

        /* temp3 = f * Cf */
        glmOPCODE(MUL, temp3, XYZ);
            glmTEMP(fogFactor, XXXX);
            glmUNIFORM(FS, uFogColor, XYZZ);

        /* Finally compute the result. */
        glmOPCODE(SUB, ShaderControl->oColor, XYZ);
            glmTEMP(temp2, XYZZ);
            glmTEMP(temp3, XYZZ);

        /* Copy the alpha component. */
        glmOPCODE(MOV, ShaderControl->oColor, W);
            glmTEMP(ShaderControl->oPrevColor, WWWW);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

static gceSTATUS processColorSum(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        glmALLOCATE_LOCAL_TEMP(temp1);

        /* Clamp color. */
        gcmERR_BREAK(clampColor(gc, ShaderControl));

        /* temp1 = Cprimary + Cspecular */
        glmOPCODE(ADD, temp1, XYZ);
            glmTEMP(ShaderControl->rColor2, XYZZ);
            glmTEMP(ShaderControl->oColor, XYZZ);

        glmOPCODE(MOV, temp1, W);
            glmTEMP(ShaderControl->oColor, WWWW);

        glmOPCODE(MOV, ShaderControl->oColor, XYZW);
            glmTEMP(temp1, XYZW);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


static gceSTATUS processAccumOperation(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    glsCHIPACCUMBUFFER * accumBuffer;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    accumBuffer = (glsCHIPACCUMBUFFER*)(gc->drawablePrivate->accumBuffer.privateData);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp0);
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);

        /* Allocate sampler. */
        glmUSING_INDEXED_VARYING(uTexSampler, 0);
        glmUSING_INDEXED_VARYING(uTexSampler, 1);

        /* Allocate texture coordinate */
        glmUSING_INDEXED_VARYING(vTexCoord, 0);
        glmUSING_INDEXED_VARYING(vTexCoord, 1);

        glmUSING_UNIFORM(uAccum);

        switch (chipCtx->hashKey.accumMode)
        {
            case gccACCUM_RETURN:
            case gccACCUM_MULT:
            case gccACCUM_ADD:
                glmOPCODE(TEXLD, temp0, XYZW);
                    glmUNIFORM_INDEXED(FS, uTexSampler0, 0, XYZW);
                    glmVARYINGV_INDEXED(FS, vTexCoord0, 0, accumBuffer->sampler[0].coordSwizzle);
                break;
            case gccACCUM_LOAD:
                glmOPCODE(TEXLD, temp1, XYZW);
                    glmUNIFORM_INDEXED(FS, uTexSampler0, 1, XYZW);
                    glmVARYINGV_INDEXED(FS, vTexCoord0, 1, accumBuffer->sampler[1].coordSwizzle);
                break;
            case gccACCUM_ACCUM:
                glmOPCODE(TEXLD, temp0, XYZW);
                    glmUNIFORM_INDEXED(FS, uTexSampler0, 0, XYZW);
                    glmVARYINGV_INDEXED(FS, vTexCoord0, 0, accumBuffer->sampler[0].coordSwizzle);
                glmOPCODE(TEXLD, temp1, XYZW);
                    glmUNIFORM_INDEXED(FS, uTexSampler0, 1, XYZW);
                    glmVARYINGV_INDEXED(FS, vTexCoord0, 1, accumBuffer->sampler[1].coordSwizzle);
                break;
        }

        switch (chipCtx->hashKey.accumMode)
        {
            case gccACCUM_ACCUM:
                glmOPCODE(MUL, temp2, XYZW);
                    glmTEMP(temp1, XYZW);
                    glmUNIFORM(FS, uAccum, XXXX);
                glmOPCODE(ADD, temp1, XYZW);
                    glmTEMP(temp0, XYZW);
                    glmTEMP(temp2, XYZW);
                break;
            case gccACCUM_LOAD:
                glmOPCODE(MUL, temp0, XYZW);
                    glmTEMP(temp1, XYZW);
                    glmUNIFORM(FS, uAccum, XXXX);
                glmOPCODE(MOV, temp1, XYZW);
                    glmTEMP(temp0, XYZW);
                break;
            case gccACCUM_RETURN:
                glmOPCODE(MUL, temp2, XYZW);
                    glmTEMP(temp0, XYZW);
                    glmUNIFORM(FS, uAccum, XXXX);
                glmOPCODE(SAT, temp1, XYZW);
                    glmTEMP(temp2, XYZW);
                break;
            case gccACCUM_MULT:
                glmOPCODE(MUL, temp1, XYZW);
                    glmTEMP(temp0, XYZW);
                    glmUNIFORM(FS, uAccum, XXXX);
                break;
            case gccACCUM_ADD:
                glmOPCODE(ADD, temp1, XYZW);
                    glmTEMP(temp0, XYZW);
                    glmUNIFORM(FS, uAccum, XXXX);
                break;
        }

        glmOPCODE(MOV, ShaderControl->oColor, XYZW);
            glmTEMP(temp1, XYZW);
    }
    while (gcvFALSE);
    gcmFOOTER();
    /* Return status. */
    return gcvSTATUS_OK;
}

static gceSTATUS processPolygonStipple(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        gctUINT samplerNumber;

        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        /* Allocate resources. */
        glmUSING_VARYING(vPosition);
        samplerNumber = chipCtx->polygonStippleTextureStage;
        /* Allocate sampler. */
        glmUSING_INDEXED_VARYING(uTexSampler, samplerNumber);

        glmOPCODE(MOV, temp1, XY);
            glmVARYING(FS, vPosition, XYYY);

        glmOPCODE(MUL, temp2, XY);
            glmTEMP(temp1, XYYY);
            glmCONST(0.03125);     /* 0.03125 = 1/ 32 */

        /* Load texture. */
        glmOPCODE(TEXLD, temp1, X);
        glmUNIFORM_INDEXED(FS, uTexSampler0, samplerNumber, XXXX);
            glmTEMP(temp2, XYYY);

        glmOPCODE_BRANCH(KILL, LESS_OR_EQUAL, 0);
            glmTEMP(temp1, XXXX);
            glmCONST(0);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

static gceSTATUS processLineStipple(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gctFLOAT step=0.0625;  /* 0.0625 = 1 / 16 by default */
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    step = 1.0f / (gctFLOAT)(16 * gc->state.line.stippleRepeat);
    do
    {
        gctUINT samplerNumber;

        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_LABEL(label0);
        glmALLOCATE_LOCAL_LABEL(label1);
        glmUSING_UNIFORM(uYmajor);
        /* Allocate resources. */
        glmUSING_VARYING(vPosition);
        samplerNumber = chipCtx->lineStippleTextureStage;
        /* Allocate sampler. */
        glmUSING_INDEXED_VARYING(uTexSampler, samplerNumber);

        glmOPCODE_BRANCH(JMP, GREATER, label0);
        glmUNIFORM(FS, uYmajor, XXXX);
        glmCONST(0.0);

        glmOPCODE(MOV, temp1, X);
        glmVARYING(FS, vPosition, XXXX);
        glmOPCODE_BRANCH(JMP, ALWAYS, label1);

        glmLABEL(label0);
        glmOPCODE(MOV, temp1, X);
        glmVARYING(FS, vPosition, YYYY);


        glmLABEL(label1);

        glmOPCODE(MUL, temp2, X);
            glmTEMP(temp1, XXXX);
            /* 0.0625 = 1 / 16  by default */
            glmCONST(step);

        /* Load texture. */
        glmOPCODE(TEXLD, temp1, X);
        glmUNIFORM_INDEXED(FS, uTexSampler0, samplerNumber, XXXX);
             glmTEMP(temp2, XXXX);

        glmOPCODE_BRANCH(KILL, LESS_OR_EQUAL, 0);
            glmTEMP(temp1, XXXX);
            glmCONST(0);
    }
    while (gcvFALSE);
    gcmFOOTER();

    /* Return status. */
    return status;
}

static gceSTATUS processPixelTransfer(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        glmALLOCATE_LOCAL_TEMP(temp1);

        if (chipCtx->hashKey.hashPixelTransfer & (1 << gccPIXEL_TRANSFER_SCALE))
        {
            glmUSING_UNIFORM(uPixelTransferScale);

            glmOPCODE(MUL, temp1, XYZW);
                glmTEMP(ShaderControl->oColor, XYZW);
                glmUNIFORM(FS, uPixelTransferScale, XYZW);

            glmOPCODE(MOV, ShaderControl->oColor, XYZW);
                glmTEMP(temp1, XYZW);
        }

        if (chipCtx->hashKey.hashPixelTransfer & (1 << gccPIXEL_TRANSFER_BIAS))
        {
            glmUSING_UNIFORM(uPixelTransferBias);

            glmOPCODE(ADD, temp1, XYZW);
                glmTEMP(ShaderControl->oColor, XYZW);
                glmUNIFORM(FS, uPixelTransferBias, XYZW);

            glmOPCODE(MOV, ShaderControl->oColor, XYZW);
                glmTEMP(temp1, XYZW);
        }

    }
    while (gcvFALSE);
    gcmFOOTER();

    /* Return status. */
    return status;
}
/*******************************************************************************
**
**  _ProcessPointSmooth
**
**  Compute the alpha value for antialiased points.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS processPointSmooth(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        GLint i;

        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmDEFINE_LABEL(lblCornerOut);

        /* Allocate resources. */
        glmUSING_VARYING(vPosition);
        glmUSING_VARYING(vPointSmooth);

        /* Compute the distance from the center of the point to the center of
           the fragment that is currently being rendered. */
        glmOPCODE(SUB, temp1, XY);
            glmVARYING(FS, vPosition, XYYY);
            glmVARYING(FS, vPointSmooth, XYYY);

        /* Zero out Z so that we can use DP3 on X and Y. */
        glmOPCODE(MOV, temp1, Z);
            glmCONST(0);

        /* Reset the resulting alpha. */
        glmOPCODE(MOV, temp2, X);
            glmCONST(0);

        /* Allocate resources. */
        glmALLOCATE_LABEL(lblCornerOut);
        /* Iterate through all 4 corners of the current fragment. */
        for (i = 0; i < 4; i++)
        {
            glmALLOCATE_LOCAL_TEMP(temp3);

            /* Top left corner of the current fragment. */
            if (i == 0)
            {
                glmOPCODE(MOV, temp3, XYZW);
                    glmTEMP(temp1, XYZW);
                glmOPCODE(SUB, temp1, XY);
                    glmTEMP(temp3, XYYY);
                    glmCONST(0.5f);
            }

            /* Top right corner of the current fragment. */
            else if (i == 1)
            {
                glmOPCODE(MOV, temp3, XYZW);
                    glmTEMP(temp1, XYZW);
                glmOPCODE(ADD, temp1, X);
                    glmTEMP(temp3, XXXX);
                    glmCONST(1);
            }

            /* Bottom right corner of the current fragment. */
            else if (i == 2)
            {
                glmOPCODE(MOV, temp3, XYZW);
                    glmTEMP(temp1, XYZW);
                glmOPCODE(ADD, temp1, Y);
                    glmTEMP(temp3, YYYY);
                    glmCONST(1);
            }

            /* Bottom left corner of the current fragment. */
            else
            {
                glmOPCODE(MOV, temp3, XYZW);
                    glmTEMP(temp1, XYZW);
                glmOPCODE(SUB, temp1, X);
                    glmTEMP(temp3, XXXX);
                    glmCONST(1);
            }

            /* temp3.x = the squared distance between the center of the point
                         and the current corner of the fragment. */
            glmOPCODE(DP3, temp3, X);
                glmTEMP(temp1, XYZZ);
                glmTEMP(temp1, XYZZ);

            /* If the square of the point's radius (vPointSmooth.z) is less
               or equal then the square of the distance between the center
               of the point and the current corner of the fragment (temp3.x),
               then the corner is outside the circle. */
           /* if outside the circle discard */
            glmOPCODE_BRANCH(KILL, LESS_OR_EQUAL, 0);
                glmVARYING(FS, vPointSmooth, ZZZZ);
                glmTEMP(temp3, XXXX);

            glmOPCODE_BRANCH(JMP, LESS_OR_EQUAL, lblCornerOut);
                glmVARYING(FS, vPointSmooth, ZZZZ);
                glmTEMP(temp3, XXXX);

            {
                /* Add the corner's contribution to the alpha value. */
                glmOPCODE(MOV, temp3, XYZW);
                    glmTEMP(temp2, XYZW);
                glmOPCODE(ADD, temp2, X);
                    glmTEMP(temp3, XXXX);
                    glmCONST(0.25f);
            }
        }

        /* Set the resulting alpha. work?? */

    /* W is alpha channel, alpha channel perhaps is NOT used, it doesn't work even if it is set */
    /* So do something on RGB channel, 2012-02-24-16:21 in shanghai */

       glmOPCODE(MOV, temp1, XYZW);
            glmTEMP(ShaderControl->oColor, XYZW);

       glmOPCODE(MUL, ShaderControl->oColor, XYZ);
            glmTEMP(temp2, XYZW);
            glmTEMP(temp1, XYZW);
        /* Define label. */
        glmLABEL(lblCornerOut);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _ApplyMultisampling
**
**  Apply multisampling fade factor to the final alpha value.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS applyMultisampling(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate resources. */
        glmUSING_VARYING(vPointFade);

        /* Clamp color. */
        gcmERR_BREAK(clampColor(gc, ShaderControl));

        /* Allocate a temp for previous color value. */
        ShaderControl->oPrevColor = ShaderControl->oColor;

        /* Allocate color output. */
        glmALLOCATE_TEMP(ShaderControl->oColor);

        /* Compute the final alpha. */
        glmOPCODE(MUL, ShaderControl->oColor, W);
            glmTEMP(ShaderControl->oPrevColor, WWWW);
            glmVARYING(FS, vPointFade, XXXX);

        /* Copy the color components. */
        glmOPCODE(MOV, ShaderControl->oColor, XYZ);
            glmTEMP(ShaderControl->oPrevColor, XYZZ);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _RoundResult
**
**  Patch for conformance test where it fails in RGB8 mode because of absence
**  of the proper conversion from 32-bit floating point FS output to 8-bit RGB
**  frame buffer. At the time the problem was identified, GC500 was already
**  taped out, so we fix the problem by rouding in the shader:
**
**      oColor.color = ( INT(oColor.color * 255 + 0.5) ) / 256
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS roundResult(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Allocate temporaries. */
        glmALLOCATE_LOCAL_TEMP(temp1);
        glmALLOCATE_LOCAL_TEMP(temp2);
        glmALLOCATE_LOCAL_TEMP(temp3);

        /* Clamp color. */
        gcmERR_BREAK(clampColor(gc, ShaderControl));

        /* Allocate a temp for previous color value. */
        ShaderControl->oPrevColor = ShaderControl->oColor;

        /* Allocate color output. */
        glmALLOCATE_TEMP(ShaderControl->oColor);

        /* Convert to 0..255 range. */
        glmOPCODE(MUL, temp1, XYZ);
            glmTEMP(ShaderControl->oPrevColor, XYZZ);
            glmCONST(255.0f);

        /* Round. */
        glmOPCODE(ADD, temp2, XYZ);
            glmTEMP(temp1, XYZZ);
            glmCONST(0.5f);

        /* Remove the fraction. */
        glmOPCODE(FLOOR, temp3, XYZ);
            glmTEMP(temp2, XYZZ);

        /* Convert back to 0..1 range. */
        glmOPCODE(MUL, ShaderControl->oColor, XYZ);
            glmTEMP(temp3, XYZZ);
            glmCONST(1.0f / 256.0f);

        /* Copy the alpha component. */
        glmOPCODE(MOV, ShaderControl->oColor, W);
            glmTEMP(ShaderControl->oPrevColor, WWWW);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _MapOutput
**
**  Map color output.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS mapOutput(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x ShaderControl=0x%x", gc, ShaderControl);

    do
    {
        /* Assign output. */
        glmASSIGN_OUTPUT(oColor);
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  _InitializeFS
**
**  INitialize common things for a fragment shader.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      ShaderControl
**          Pointer to the current shader object.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS initializeFS(
    __GLcontext * gc,
    glsFSCONTROL_PTR ShaderControl
    )
{
    gceSTATUS status = gcvSTATUS_OK;
gcmHEADER_ARG("gc=0x%x ShaderControl=0x%x", gc, ShaderControl);
    do
    {
        /* Allocate the Color uniform. */
        glmUSING_UNIFORM(uColor);
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  glfGenerateFSFixedFunction
**
**  Generate Fragment Shader code.
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

gceSTATUS glfGenerateFSFixedFunction(
    __GLcontext * gc
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    /* Define internal shader structure. */
    glsFSCONTROL fsControl;
    gcmHEADER_ARG("gc=0x%x", gc);

    do
    {
        gcoOS_ZeroMemory(&fsControl, sizeof(glsFSCONTROL));
        fsControl.i = &chipCtx->currProgram->fs;

        if (fsControl.i->shader == gcvNULL)
        {
            /* Shader may have been deleted. Construct again. */
            gcmONERROR(gcSHADER_Construct(
                gcSHADER_TYPE_FRAGMENT,
                &fsControl.i->shader
                ));
        }

        /* Initialize the fragment shader. */
        gcmERR_BREAK(initializeFS(gc, &fsControl));

        /* Handle clip plane first. */
        gcmERR_BREAK(processClipPlane(gc, &fsControl));

        /* Determine preliminary color output. */
        gcmERR_BREAK(getInitialColor(gc, &fsControl));

        if (chipCtx->hashKey.accumMode != gccACCUM_UNKNOWN)
        {
            gcmERR_BREAK(processAccumOperation(gc, &fsControl));
        }
        else
        {
            /* Process texture. */
            gcmERR_BREAK(processTexture(gc, &fsControl));

            if (gc->state.enables.colorSum)
            {
                gcmERR_BREAK(processColorSum(gc, &fsControl));
            }

            /* Compute the fog color. */
            if (gc->state.enables.fog)
            {
                gcmERR_BREAK(processFog(gc, &fsControl));
            }

            /* Process points. */
            if (chipCtx->pointStates.pointPrimitive)
            {
                /* Compute point smooth. */
                if (chipCtx->pointStates.smooth &&
                    !chipCtx->pointStates.spriteEnable)
                {
                    gcmERR_BREAK(processPointSmooth(gc, &fsControl));
                }

                /* Apply multisampling alpha. */
                if (gc->state.enables.multisample.multisampleOn)
                {
                    gcmERR_BREAK(applyMultisampling(gc, &fsControl));
                }
            }

            if (chipCtx->hashKey.hasPolygonStippleEnabled) {
                gcmERR_BREAK(processPolygonStipple(gc, &fsControl));
            }

            if (chipCtx->hashKey.hasLineStippleEnabled) {
                gcmERR_BREAK(processLineStipple(gc, &fsControl));
            }

            if (chipCtx->hashKey.hashPixelTransfer)
            {
                 gcmERR_BREAK(processPixelTransfer(gc, &fsControl));
            }
        }

        /* Round the result. */
        if (chipCtx->fsRoundingEnabled)
        {
            gcmERR_BREAK(roundResult(gc, &fsControl));
        }

        /* Map output. */
        gcmERR_BREAK(mapOutput(gc, &fsControl));

        /* Pack the shader. */
        gcmERR_BREAK(gcSHADER_Pack(fsControl.i->shader));

        /* Optimize the shader. */
#if !GC_ENABLE_LOADTIME_OPT
        gcmERR_BREAK(gcSHADER_SetOptimizationOption(fsControl.i->shader, gcvOPTIMIZATION_FULL & ~gcvOPTIMIZATION_LOADTIME_CONSTANT));
        gcmERR_BREAK(gcOptimizeShader(fsControl.i->shader, gcvNULL));
#endif
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;

OnError:
    /* Destroy partially generated shader. */
    if (fsControl.i->shader)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(fsControl.i->shader));
        fsControl.i->shader = gcvNULL;
    }

    gcmFOOTER();
    /* Return status. */
    return status;
}
