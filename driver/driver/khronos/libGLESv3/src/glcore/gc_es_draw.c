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


#include "gc_es_context.h"
#include "gc_es_device.h"
#include "gc_es_object_inline.c"

extern GLboolean __glIsTextureComplete(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter,
                                       GLenum magFilter, GLenum compareMode, GLint maxLevelUsed);


#define _GC_OBJ_ZONE __GLES3_ZONE_CORE

/* Set all attribute dirty bits.
 */
GLvoid __glSetAttributeStatesDirty(__GLcontext *gc)
{
    GLuint index, unit;
    __GLSLStage stage;

    /* Initialize global dirty attribute bits */
    for (index = 0; index < __GL_DIRTY_ATTRS_END; index++)
    {
        gc->globalDirtyState[index] = (GLbitfield)(-1);
    }

    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_TRUE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_TRUE);

    for (unit = 0; unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits; unit++)
    {
        gc->texUnitAttrState[unit] = (GLbitfield)(-1);
    }

    gc->drawableDirtyMask = __GL_BUFFER_DRAW_READ_BITS;

    /* Reset the last program/codeSeq to force toggling the dirty. */
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
        gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
    }

    for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
    {
        __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_TRUE);
    }
}

/* Clear all attribute dirty bits
 */
__GL_INLINE GLvoid __glClearAttributeStates(__GLcontext *gc)
{
    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_FALSE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_FALSE);
    __GL_MEMZERO(gc->texUnitAttrState, gc->constants.shaderCaps.maxCombinedTextureImageUnits * sizeof(GLbitfield));
    __GL_MEMZERO(gc->globalDirtyState, __GL_DIRTY_ATTRS_END   * sizeof(GLbitfield));
}

GLvoid __glBuildTexEnableDim(__GLcontext * gc, __GLattribute* cs, __GLattribute* ds)
{
    GLint unit;

    (*gc->dp.buildTexEnableDim)(gc);

    /* Mark texture enable dimension changed */
    for (unit = 0; unit < (GLint)gc->shaderProgram.maxUnit; ++unit)
    {
        if (ds->texture.texUnits[unit].enableDim != cs->texture.texUnits[unit].enableDim)
        {
            ds->texture.texUnits[unit].enableDim = cs->texture.texUnits[unit].enableDim;
            __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
        }
    }
}

/*
** Compute the gc->texture.units[unit].maxLevelUsed, which will be used in texture consistency check
*/
GLint __glCalcTexMaxLevelUsed(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter)
{
    GLboolean useMipmap = GL_FALSE;
    GLint maxLevelUsed;
    GLint base = texObj->params.baseLevel;

    /* Check whether need mipmap */
    switch (texObj->params.mipHint)
    {
    case __GL_TEX_MIP_HINT_AUTO:
    case __GL_TEX_MIP_HINT_AUTO_MIP:
        useMipmap = (minFilter != GL_NEAREST && minFilter != GL_LINEAR) ? GL_TRUE : GL_FALSE;
        break;
    case __GL_TEX_MIP_HINT_FORCE_ON:
        useMipmap = GL_TRUE;
        break;
    case __GL_TEX_MIP_HINT_FORCE_OFF:
        useMipmap = GL_FALSE;
        break;
    default:
        GL_ASSERT(0);
    }

    if (useMipmap)
    {
        if (texObj->immutable)
        {
            maxLevelUsed = texObj->immutableLevels - 1;
        }
        else
        {
            __GLmipMapLevel *baseMipmap = &texObj->faceMipmap[0][base];
            GLint maxSize = __GL_MAX(__GL_MAX(baseMipmap->width, baseMipmap->height), baseMipmap->depth);
            maxLevelUsed = (GLint)__glFloorLog2((GLuint)maxSize) + base;
        }

        if (maxLevelUsed > texObj->params.maxLevel)
        {
            maxLevelUsed = texObj->params.maxLevel;
        }

        if (((texObj->params.mipHint == __GL_TEX_MIP_HINT_FORCE_ON) ||
             (texObj->params.mipHint == __GL_TEX_MIP_HINT_AUTO_MIP)
            )&&
            (maxLevelUsed > texObj->mipMaxLevel)
           )
        {
            maxLevelUsed = texObj->mipMaxLevel;
        }
    }
    else
    {
        maxLevelUsed = base;
    }

    return maxLevelUsed;
}

__GL_INLINE GLvoid __glEvaluateAttribGroup1(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

    __GL_CHECK_ATTR2(__GL_DEPTHRANGE_BIT, depth.zNear, depth.zFar);

    if (localMask & __GL_COLORBUF_ATTR_BITS)
    {
        __GL_CHECK_ATTR4(__GL_BLENDCOLOR_BIT,
            raster.blendColor.r, raster.blendColor.g,
            raster.blendColor.b, raster.blendColor.a);

        if (localMask & __GL_BLENDEQUATION_BIT)
        {
            if ((__GL_MEMCMP(ds->raster.blendEquationRGB,   cs->raster.blendEquationRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)) ||
                (__GL_MEMCMP(ds->raster.blendEquationAlpha, cs->raster.blendEquationAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)))
            {
                __GL_MEMCOPY(ds->raster.blendEquationRGB,   cs->raster.blendEquationRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendEquationAlpha, cs->raster.blendEquationAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLENDEQUATION_BIT);
            }
        }

        if (localMask & __GL_BLENDFUNC_BIT)
        {
            if ((__GL_MEMCMP(ds->raster.blendSrcRGB,   cs->raster.blendSrcRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers))     ||
                (__GL_MEMCMP(ds->raster.blendSrcAlpha, cs->raster.blendSrcAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)) ||
                (__GL_MEMCMP(ds->raster.blendDstRGB,   cs->raster.blendDstRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers))     ||
                (__GL_MEMCMP(ds->raster.blendDstAlpha, cs->raster.blendDstAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)))
            {
                __GL_MEMCOPY(ds->raster.blendSrcRGB,   cs->raster.blendSrcRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendSrcAlpha, cs->raster.blendSrcAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendDstRGB,   cs->raster.blendDstRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendDstAlpha, cs->raster.blendDstAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLENDFUNC_BIT);
            }
        }

        if (localMask & __GL_BLEND_ENDISABLE_BIT)
        {
            if (__GL_MEMCMP(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers))
            {
                __GL_MEMCOPY(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLEND_ENDISABLE_BIT);
            }
        }

        if (localMask & __GL_COLORMASK_BIT)
        {
            if (__GL_MEMCMP(ds->raster.colorMask, cs->raster.colorMask, 4 * sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers))
            {
                __GL_MEMCOPY(ds->raster.colorMask, cs->raster.colorMask, 4 * sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~( __GL_COLORMASK_BIT);
            }
        }
    }

    if (localMask & __GL_DEPTHBUF_ATTR_BITS)
    {
        __GL_CHECK_ATTR1(__GL_DEPTHFUNC_BIT, depth.testFunc);
        __GL_CHECK_ATTR1(__GL_DEPTHTEST_ENDISABLE_BIT, enables.depthTest);
        __GL_CHECK_ATTR1(__GL_DEPTHMASK_BIT, depth.writeEnable);
    }

    if (localMask & __GL_STENCIL_ATTR_BITS)
    {
        __GL_CHECK_ATTR3(__GL_STENCILFUNC_FRONT_BIT,
            stencil.front.testFunc, stencil.front.reference, stencil.front.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_FRONT_BIT,
            stencil.front.fail, stencil.front.depthFail, stencil.front.depthPass);

        __GL_CHECK_ATTR3(__GL_STENCILFUNC_BACK_BIT,
            stencil.back.testFunc, stencil.back.reference, stencil.back.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_BACK_BIT,
            stencil.back.fail, stencil.back.depthFail, stencil.back.depthPass);

        __GL_CHECK_ATTR1(__GL_STENCILMASK_FRONT_BIT, stencil.front.writeMask);
        __GL_CHECK_ATTR1(__GL_STENCILMASK_BACK_BIT, stencil.back.writeMask);
        __GL_CHECK_ATTR1(__GL_STENCILTEST_ENDISABLE_BIT, enables.stencilTest);
    }

    if (localMask & __GL_POLYGON_ATTR_BITS)
    {
        __GL_CHECK_ATTR1(__GL_FRONTFACE_BIT, polygon.frontFace);
        __GL_CHECK_ATTR1(__GL_CULLFACE_BIT, polygon.cullFace);
        __GL_CHECK_ATTR1(__GL_CULLFACE_ENDISABLE_BIT, enables.polygon.cullFace);
        __GL_CHECK_ATTR2(__GL_POLYGONOFFSET_BIT, polygon.factor, polygon.units);
        __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_FILL_ENDISABLE_BIT, enables.polygon.polygonOffsetFill);
    }

    __GL_CHECK_ATTR1(__GL_RASTERIZER_DISCARD_ENDISABLE_BIT, enables.rasterizerDiscard);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_1]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_1] = localMask;
    if (localMask == 0)
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_DIRTY_ATTRS_1);
    }
}

__GL_INLINE GLvoid __glEvaluateAttribGroup2(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

    __GL_CHECK_ATTR4(__GL_VIEWPORT_BIT, viewport.x, viewport.y,
                                        viewport.width, viewport.height);

    __GL_CHECK_ATTR4(__GL_SCISSOR_BIT, scissor.scissorX, scissor.scissorY,
                                       scissor.scissorWidth, scissor.scissorHeight);

    __GL_CHECK_ATTR1(__GL_SCISSORTEST_ENDISABLE_BIT, enables.scissorTest);
    __GL_CHECK_ATTR1(__GL_DITHER_ENDISABLE_BIT, enables.colorBuffer.dither);
    __GL_CHECK_ATTR1(__GL_LINEWIDTH_BIT, line.requestedWidth);
    __GL_CHECK_ATTR2(__GL_SAMPLECOVERAGE_BIT, multisample.coverageValue, multisample.coverageInvert);
    __GL_CHECK_ATTR1(__GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT, enables.multisample.alphaToCoverage);
    __GL_CHECK_ATTR1(__GL_SAMPLE_COVERAGE_ENDISABLE_BIT, enables.multisample.coverage);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MASK_ENDISABLE_BIT, enables.multisample.sampleMask);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MASK_BIT, multisample.sampleMaskValue);
    __GL_CHECK_ATTR1(__GL_SAMPLE_SHADING_ENDISABLE_BIT, enables.multisample.sampleShading);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MIN_SHADING_VALUE_BIT, multisample.minSampleShadingValue);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_2]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_2] = localMask;
    if (localMask == 0)
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_DIRTY_ATTRS_2);
    }
}

__GL_INLINE GLvoid __glEvaluateTextureAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLint  i = -1;
    GLuint localMask;
    GLuint enableDim, realEnableDim, lastRealEnableDim;
    __GLbitmask unitMask = gc->texUnitAttrDirtyMask;
    GLuint lastMaxLevelUsed = 0;
    __GLtextureObject *texObj = gcvNULL;

    while (!__glBitmaskIsAllZero(&unitMask))
    {
        if (!__glBitmaskTestAndClear(&unitMask, ++i))
        {
            continue;
        }

        localMask = gc->texUnitAttrState[i];
        lastMaxLevelUsed = gc->texture.units[i].maxLevelUsed;

        /* Save the last enabled texture dimension and last texture object. */
        enableDim = cs->texture.texUnits[i].enableDim;
        lastRealEnableDim = ds->texture.texUnits[i].realEnableDim;

        /* Please do only use "currentTexture" after the evaluation and in this draw.
        ** Otherwise the texobj might be destroyed in other context but "currentTexture" not cleared.
        */
        gc->texture.units[i].currentTexture = gcvNULL;
        __glBitmaskClear(&gc->texture.currentEnableMask, i);
        if (enableDim < __GL_MAX_TEXTURE_BINDINGS)
        {
            GLenum minFilter, magFilter, compareMode;

            texObj = gc->texture.units[i].boundTextures[enableDim];
            minFilter = gc->texture.units[i].boundSampler
                      ? gc->texture.units[i].boundSampler->params.minFilter
                      : texObj->params.sampler.minFilter;
            magFilter = gc->texture.units[i].boundSampler
                      ? gc->texture.units[i].boundSampler->params.magFilter
                      : texObj->params.sampler.magFilter;
            compareMode = gc->texture.units[i].boundSampler
                        ? gc->texture.units[i].boundSampler->params.compareMode
                        : texObj->params.sampler.compareMode;

            /* maxLevelUsed must be calculated before texture consistent check */
            if (localMask & (__GL_TEX_ENABLE_DIM_CHANGED_BIT |
                             __GL_TEX_IMAGE_CONTENT_CHANGED_BIT |
                             __GL_TEXPARAM_MIP_HINT_BIT |
                             __GL_TEXPARAM_MIN_FILTER_BIT |
                             __GL_TEXPARAM_BASE_LEVEL_BIT |
                             __GL_TEXPARAM_MAX_LEVEL_BIT))
            {

                gc->texture.units[i].maxLevelUsed = __glCalcTexMaxLevelUsed(gc, texObj, minFilter);

                /* Update and set object-only dirty */
                if (texObj->maxLevelUsed != gc->texture.units[i].maxLevelUsed)
                {
                    texObj->maxLevelUsed = gc->texture.units[i].maxLevelUsed;
                    texObj->uObjStateDirty.s.maxLevelUsedDirty = GL_TRUE;
                }
            }

            if (__glIsTextureComplete(gc, texObj, minFilter, magFilter, compareMode, gc->texture.units[i].maxLevelUsed))
            {
                realEnableDim = enableDim;
                gc->texture.units[i].currentTexture = texObj;
                __glBitmaskSet(&gc->texture.currentEnableMask, i);
            }
            else
            {
                /* Set texture to be disabled if inconsistent */
                realEnableDim = __GL_MAX_TEXTURE_BINDINGS;
                __GLES_PRINT("ES30:texture(id=%d) is incomplete", texObj->name);

            }
        }
        else
        {
            realEnableDim = enableDim;
        }

        /* Check if "enableDim" is really changed */
        cs->texture.texUnits[i].realEnableDim = realEnableDim;
        if (realEnableDim != ds->texture.texUnits[i].realEnableDim)
        {
            localMask |= __GL_TEX_ENABLE_DIM_CHANGED_BIT;
        }
        __GL_CHECK_ATTR1(__GL_TEX_ENABLE_DIM_CHANGED_BIT, texture.texUnits[i].realEnableDim);

        if (localMask & __GL_TEX_ENABLE_DIM_CHANGED_BIT)
        {
            /* If dim changed, only need to check param bit if enabled */
            if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS)
            {
                localMask |= __GL_TEXPARAMETER_BITS;
            }
            else
            {
                localMask &= ~__GL_TEXPARAMETER_BITS;
            }

            /* Texture image dirty bits need to be set when enableDim is changed */
            localMask |= __GL_TEXIMAGE_BITS;
        }

        if (localMask & __GL_TEXPARAMETER_BITS)
        {
            __GLsamplerObject *samplerObj = gc->texture.units[i].boundSampler;
            __GLtextureParamState *ds_params = &ds->texture.texUnits[i].commitParams;
            if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS &&
                lastRealEnableDim < __GL_MAX_TEXTURE_BINDINGS)
            {
                /* Parameters may come from either sampler objects or texture objects */
                if (localMask & __GL_SAMPLERPARAMETER_BITS)
                {
                    __GLsamplerParamState *cs_params = samplerObj
                                                     ? &samplerObj->params
                                                     : (__GLsamplerParamState*)&texObj->params;

                    if (__GL_MEMCMP(ds_params, cs_params, sizeof(__GLsamplerParamState)))
                    {
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_S_BIT, sWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_T_BIT, tWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_R_BIT, rWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MIN_FILTER_BIT, minFilter);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MAG_FILTER_BIT, magFilter);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MIN_LOD_BIT, minLod);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MAX_LOD_BIT, maxLod);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_COMPARE_MODE_BIT, compareMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_COMPARE_FUNC_BIT, compareFunc);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_SRGB_BIT, sRGB);

                        if (localMask & __GL_TEXPARAM_BORDER_COLOR_BIT)
                        {
                            /* Update and set object-only state dirty */
                            if (__GL_MEMCMP(cs_params->borderColor.fv, texObj->borderColorUsed.fv, 4 * sizeof(GLfloat)))
                            {
                                __GL_MEMCOPY(texObj->borderColorUsed.fv, cs_params->borderColor.fv, 4 * sizeof(GLfloat));
                                texObj->uObjStateDirty.s.borderColorDirty = GL_TRUE;
                            }
                            __GL_CHECK_SAMPLER_PARAM_ARRAY(__GL_TEXPARAM_BORDER_COLOR_BIT, borderColor.fv, 4, sizeof(GLfloat));

                        }
                    }
                    else
                    {
                        localMask &= ~__GL_SAMPLERPARAMETER_BITS;
                    }
                }

                /* Other parameters must come from texture objects */
                {
                    __GLtextureParamState *cs_params = &texObj->params;
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MIP_HINT_BIT, mipHint);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_BASE_LEVEL_BIT, baseLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAX_LEVEL_BIT, maxLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_R_BIT, swizzle[0]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_G_BIT, swizzle[1]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_B_BIT, swizzle[2]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_A_BIT, swizzle[3]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_D_ST_TEXMODE_BIT, depthStTexMode);
                }
            }
            else if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS &&
                     lastRealEnableDim == __GL_MAX_TEXTURE_BINDINGS)
            {
                __GL_MEMCOPY(ds_params, &texObj->params, sizeof(__GLtextureParamState));
                if (samplerObj)
                {
                    __GL_MEMCOPY(ds_params, &samplerObj->params, sizeof(__GLsamplerParamState));
                }
            }
        }

        if (lastMaxLevelUsed != gc->texture.units[i].maxLevelUsed)
        {
            localMask |= __GL_TEXPARAM_MAX_LEVEL_BIT;
        }

        /* Reassign localMask back to texUnitAttrState[i]
        */
        gc->texUnitAttrState[i] = localMask;
        if (localMask == 0)
        {
            __glBitmaskClear(&gc->texUnitAttrDirtyMask, i);
        }
        else
        {
            GLuint index;
            __GLtexUnit2Sampler *texUnit2Sampler = &gc->shaderProgram.texUnit2Sampler[i];
            /* Mark states of the samplers bound to this unit dirty */
            for (index = 0; index < texUnit2Sampler->numSamplers; ++index)
            {
                __glBitmaskSet(&gc->shaderProgram.samplerStateDirty, texUnit2Sampler->samplers[index]);
            }
        }
    }

    if (__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_TEX_UNIT_ATTRS);
    }
}


__GL_INLINE GLvoid __glEvaluateImageAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    __GLbitmask unitMask = gc->imageUnitDirtyMask;
    GLint i = -1;
    __GLtextureObject *texObj;
    __GLbitmask localMask = unitMask;

    while (!__glBitmaskIsAllZero(&unitMask))
    {
        if (!__glBitmaskTestAndClear(&unitMask, ++i))
        {
            continue;
        }

        texObj = cs->image.imageUnit[i].texObj;

        if (texObj)
        {
            GLint maxLevelUsed = __glCalcTexMaxLevelUsed(gc, texObj, texObj->params.sampler.minFilter);
            __GLsamplerParamState * samplerParam = &texObj->params.sampler;

            if (GL_FALSE == __glIsTextureComplete(gc, texObj, samplerParam->minFilter, samplerParam->magFilter,
                                                  samplerParam->compareMode, maxLevelUsed))
            {
                cs->image.imageUnit[i].invalid = GL_TRUE;
                break;
            }
        }

        if (!__GL_MEMCMP(&cs->image.imageUnit[i], &ds->image.imageUnit[i], sizeof(__GLimageUnitState)))
        {
            __glBitmaskClear(&localMask, i);
        }
        else
        {
            __GL_MEMCOPY(&ds->image.imageUnit[i], &cs->image.imageUnit[i], sizeof(__GLimageUnitState));
        }
    }

    if (__glBitmaskIsAllZero(&localMask))
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_IMG_UNIT_ATTRS);
    }
    else
    {
        gc->imageUnitDirtyMask = localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateProgramAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_PROGRAM_ATTRS];
    __GLshaderProgramMachine *spMachine = &gc->shaderProgram;
    GLbitfield stageDirties[] =
    {
        __GL_DIRTY_GLSL_VS_SWITCH,
        __GL_DIRTY_GLSL_TCS_SWITCH,
        __GL_DIRTY_GLSL_TES_SWITCH,
        __GL_DIRTY_GLSL_GS_SWITCH,
        __GL_DIRTY_GLSL_FS_SWITCH,
        __GL_DIRTY_GLSL_CS_SWITCH
    };
    __GLSLStage stage;

    if (localMask & __GL_DIRTY_GLSL_MODE_SWITCH)
    {
        localMask |= __GL_DIRTY_GLSL_PROGRAM_SWITCH;

        /* Reset the last program/codeSeq to force toggling the dirty. */
        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
            gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
        }
    }

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        __GLprogramObject *progObj = spMachine->activeProgObjs[stage];

        if (progObj)
        {
            GLenum dirtyBit = stageDirties[stage];

            if (localMask & dirtyBit)
            {
                /* Check whether stage program and its code really changed */
                if (spMachine->lastProgObjs[stage] == progObj &&
                    spMachine->lastCodeSeqs[stage] == progObj->programInfo.codeSeq)
                {
                    localMask &= ~(dirtyBit);
                }
                else
                {
                    spMachine->lastProgObjs[stage] = progObj;
                    spMachine->lastCodeSeqs[stage] = progObj->programInfo.codeSeq;
                }
            }
        }
    }


    if (localMask & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        GLuint index;
        gcePATCH_ID patchId = gcvPATCH_INVALID;
        /* When program switch, even sampler->texUnit mapping sometimes may not change
        ** The sampler type might changed, need to set the flag to rebuild texEnableDim
        */
        localMask |= __GL_DIRTY_GLSL_SAMPLER;
        localMask |= __GL_DIRTY_GLSL_UNIFORM;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_REALRACING)
        {
            gc->vertexArray.varrayDirty = 0x3F;
        }
        /* Mark all array binding points dirty when program switched */
        for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
        {
            __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_TRUE);
        }
    }

    if (localMask & __GL_DIRTY_GLSL_SAMPLER)
    {
        GLuint sampler;
        GLuint *newSampler2TexUnit = cs->program.sampler2TexUnit;
        GLuint *oldSampler2TexUnit = ds->program.sampler2TexUnit;

        /* Current sampler2TexUnit will be build in __glBuildTexEnableDim */
        __glBuildTexEnableDim(gc, cs, ds);

        for (sampler = 0; sampler < gc->shaderProgram.maxSampler; ++sampler)
        {
            if (oldSampler2TexUnit[sampler] != newSampler2TexUnit[sampler])
            {
                /* Program switch depend on the comparison to set samplerMapDirty */
                __glBitmaskSet(&gc->shaderProgram.samplerMapDirty, sampler);
                oldSampler2TexUnit[sampler] = newSampler2TexUnit[sampler];
            }
            else
            {
                __glBitmaskClear(&gc->shaderProgram.samplerMapDirty, sampler);
            }
        }
    }

    gc->globalDirtyState[__GL_PROGRAM_ATTRS] = localMask;
}


/******** Draw functions *****************/

#if __GL_TIMEING_DRAW
extern BOOLEAN logTimeEnable;
extern GLuint64 tTick;
extern FILE*  timeingLogFile ;
extern GLbyte *printBuffer ;
extern GLbyte* curOutput, *endSlot;

__GL_INLINE GLuint64 GetCycleCount()
{
    __asm _emit 0x0F
    __asm _emit 0x31
}

#define ENTERFUNC_TM() \
    if(logTimeEnable) tTick = GetCycleCount();

#define LEAVEFUNC_TM()                                  \
    if (logTimeEnable) {                                \
        tTick = GetCycleCount() - tTick;                \
        curOutput += sprintf(curOutput, "%d\t", tTick); \
        if (curOutput > endSlot ) {                     \
            if(timeingLogFile) {                        \
                sprintf(curOutput, "\0");               \
                fprintf(timeingLogFile, printBuffer);   \
                curOutput = printBuffer;                \
            }                                           \
        }                                               \
    }

#else
#define ENTERFUNC_TM()
#define LEAVEFUNC_TM()
#endif

__GL_INLINE GLboolean __glDrawBegin(__GLcontext *gc, GLenum mode)
{
    return (*gc->dp.drawBegin)(gc, mode);
}

__GL_INLINE GLboolean __glDrawValidateState(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    __GLattribute *ds = &gc->commitState;

    if (gc->invalidCommonCommit || gc->invalidDrawCommit)
    {
        /* Full dirty to flush all */
        __glOverturnCommitStates(gc);
        __glSetAttributeStatesDirty(gc);

        gc->shaderProgram.maxSampler = gc->constants.shaderCaps.maxTextureSamplers;
        gc->shaderProgram.maxUnit = gc->constants.shaderCaps.maxCombinedTextureImageUnits;
    }

    if (gc->globalDirtyState[__GL_ALL_ATTRS])
    {
        if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1])
        {
            __glEvaluateAttribGroup1(gc, cs, ds);
        }

        if (gc->globalDirtyState[__GL_DIRTY_ATTRS_2])
        {
            __glEvaluateAttribGroup2(gc, cs, ds);
        }

        /* Program state must be checked before texture */
        if (gc->globalDirtyState[__GL_PROGRAM_ATTRS])
        {
            __glEvaluateProgramAttrib(gc, cs, ds);
        }

        if (!__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
        {
            __glEvaluateTextureAttrib(gc, cs, ds);
        }

        if (!__glBitmaskIsAllZero(&gc->imageUnitDirtyMask))
        {
            __glEvaluateImageAttrib(gc, cs, ds);
        }
    }

    /*
    ** Note "drawValidateState" is designed to never fail. Otherwise GL states might still not
    ** be flush to HW before their dirty bits are cleared in the next draw's evaluations.
    ** If there are HW limitations that the draw must be skipped, set DrawNothing function in chip layer
    ** for those cases.
    */

    /* TODO: When we cover all logic of this function with dirty bits, we can check dirty before call into it*/
    return gc->dp.drawValidateState(gc);
}


__GL_INLINE GLboolean __glDrawEnd(__GLcontext *gc)
{
    GLboolean ret = gc->dp.drawEnd(gc);

    if (ret)
    {
        GLuint index;
        __GLprogramObject *progObj;
        __GLSLStage stage;

        for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
        {
            __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_FALSE);
        }

        __glBitmaskSetAll(&gc->shaderProgram.samplerMapDirty, GL_FALSE);
        gc->shaderProgram.samplerStateDirty = gc->shaderProgram.samplerStateKeepDirty;
        __glBitmaskSetAll(&gc->shaderProgram.samplerStateKeepDirty, GL_FALSE);

        if (gc->globalDirtyState[__GL_ALL_ATTRS])
        {
            /* Clear all the attribute dirty bits */
            __glClearAttributeStates(gc);
        }

        gc->vertexArray.varrayDirty = 0;

        if (gc->invalidCommonCommit || gc->invalidDrawCommit)
        {
            gc->invalidCommonCommit = GL_FALSE;
            gc->invalidDrawCommit = GL_FALSE;

            if (gc->shaderProgram.mode == __GLSL_MODE_GRAPHICS)
            {
                /* reset valude.*/
                gc->shaderProgram.maxSampler = 0;
                gc->shaderProgram.maxUnit = 0;

                for (stage = __GLSL_STAGE_VS; stage <= __GLSL_STAGE_FS; ++stage)
                {
                    progObj = __glGetCurrentStageProgram(gc, stage);
                    if (progObj)
                    {
                        gc->shaderProgram.maxSampler = gcmMAX(gc->shaderProgram.maxSampler, progObj->maxSampler);
                        gc->shaderProgram.maxUnit = gcmMAX(gc->shaderProgram.maxUnit, progObj->maxUnit);
                    }
                }
            }
        }

        /* Temp disable the dirty in case to affect perf */
    }

    return ret;
}


__GL_INLINE GLboolean  __glDrawPattern(__GLcontext *gc)
{
    GLboolean matched = GL_FALSE;

    if (gc->pattern.enable)
    {
        gc->pattern.lastPattern = gc->pattern.matchPattern;
        gc->pattern.matchPattern = gcvNULL;

        if (gc->pattern.patternMatchMask == 0)
        {
            /* Reset if match nothing, */
            gc->pattern.patternMatchMask = ( 1 << GLES_PATTERN_GFX0 | 1 << GLES_PATTERN_GFX1);
            gc->pattern.matchCount = 0;
            gc->pattern.apiCount = 0;

            if (gc->pattern.lastPattern != gcvNULL)
            {
                /* Fail from last pattern*/
                /*gcmPRINT("Fail from last pattern");*/

                /* For this pattern, we need update blend color/func, and depth function commit state and dirty */
                gc->commitState.raster.blendColor.r = gc->state.raster.blendColor.r + 1;
                gc->commitState.raster.blendColor.g = gc->state.raster.blendColor.g + 1;
                gc->commitState.raster.blendColor.b = gc->state.raster.blendColor.b + 1;
                gc->commitState.raster.blendColor.a = gc->state.raster.blendColor.a + 1;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDCOLOR_BIT);

                gc->commitState.raster.blendSrcRGB[0] = gc->state.raster.blendSrcRGB[0] + 1;
                gc->commitState.raster.blendSrcAlpha[0] = gc->state.raster.blendSrcAlpha[0] + 1;
                gc->commitState.raster.blendDstRGB[0] = gc->state.raster.blendDstRGB[0] + 1;
                gc->commitState.raster.blendDstAlpha[0] = gc->state.raster.blendDstAlpha[0] + 1;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);

                gc->commitState.depth.testFunc = gc->state.depth.testFunc;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHFUNC_BIT);
            }

            gc->pattern.state = GLES_PATTERN_STATE_CHECK;
        }
        else
        {
            GLuint i;

            for (i = 0; i < GLES_PATTERN_COUNT; i++)
            {
                /* get the match APIs, only could get one */
                if (gc->pattern.patternMatchMask == (gc->pattern.patternMatchMask & (1 << i)))
                {
                    if (gc->pattern.patterns[i]->apiCount == gc->pattern.matchCount)
                    {
                        gc->pattern.matchPattern = gc->pattern.patterns[i];
                        gc->pattern.state = GLES_PATTERN_STATE_MATCHED;
                        gc->pattern.matchCount = 0;
                        gc->pattern.apiCount = 0;

                        /* the same with the last one, send command */
                        if (gc->pattern.lastPattern == gc->pattern.matchPattern)
                        {
                            /* match! */
                            /*gcmPRINT("pattern found, run HW command, and go!");*/

                            if (i == GLES_PATTERN_GFX0 || i == GLES_PATTERN_GFX1)
                            {
                                matched = (*gc->dp.drawPattern)(gc);
                            }

                            return matched;
                        }
                        else
                        {
                            /* match! */
                            /*gcmPRINT("pattern found, get another round!");*/
                        }
                    }
                    else
                    {
                        /* Not full matched, continue check */
                        gc->pattern.state = GLES_PATTERN_STATE_CHECK;
                    }
                    break;
                }
            }
        }
    }

    return matched;
}


static GLboolean __glCheckVBOSize(__GLcontext *gc)
{
    GLboolean ret = GL_TRUE;
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    if (__glExtension[__GL_EXTID_KHR_robust_buffer_access_behavior].bEnabled)
    {
        return GL_TRUE;
    }

    if (!(vertexArray->multidrawIndirect || vertexArray->drawIndirect))
    {
        GLsizeiptr endBytes;
        __GLbufferObject *boundIdxObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
        GLuint indexCount = vertexArray->indexCount;

        if (boundIdxObj && (indexCount != 0))
        {
            GLuint sizeofIndex = 0;
            switch (vertexArray->indexType)
            {
            case GL_UNSIGNED_BYTE:
                sizeofIndex = 1;
                break;
            case GL_UNSIGNED_SHORT:
                sizeofIndex = 2;
                break;
            case GL_UNSIGNED_INT:
                sizeofIndex = 4;
                break;
            }

            endBytes = (GLsizeiptr)(indexCount * sizeofIndex + __GL_PTR2UINT(vertexArray->indices));

            if (endBytes > boundIdxObj->size)
            {
                ret = GL_FALSE;
            }
        }
        else if (indexCount == 0)
        {
            GLuint index = 0;
            GLuint instanceCount = vertexArray->instanceCount;
            __GLvertexArrayState *curVertexArray = &gc->vertexArray.boundVAO->vertexArray;
            GLuint attribEnabled = curVertexArray->attribEnabled;
            __GLprogramObject *vsProgObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_VS);
            GLuint vsInputArrayMask = 0;
            if (vsProgObj)
            {
                vsInputArrayMask = vsProgObj->bindingInfo.vsInputArrayMask;
            }


            while (attribEnabled & vsInputArrayMask)
            {
                if ((attribEnabled & vsInputArrayMask) & 0x1)
                {
                    GLuint remain;
                    __GLvertexAttrib *pAttrib = &curVertexArray->attribute[index];
                    __GLvertexAttribBinding *pAttribBinding = &curVertexArray->attributeBinding[pAttrib->attribBinding];
                    __GLbufferObject *boundVBObj = pAttribBinding->boundArrayObj;

                    if (boundVBObj)
                    {
                        if (pAttribBinding->divisor)
                        {
                            remain = (instanceCount % pAttribBinding->divisor) ? 1 : 0;
                            endBytes = pAttrib->relativeOffset + pAttribBinding->offset + ((instanceCount / pAttribBinding->divisor) + remain - 1) *  pAttribBinding->stride +
                                __glUtilCalculateStride(pAttrib->size, pAttrib->type);
                            if (endBytes > boundVBObj->size)
                            {
                                ret = GL_FALSE;
                                break;
                            }
                        }
                        else
                        {
                            endBytes = pAttrib->relativeOffset + pAttribBinding->offset + (vertexArray->end - 1) * pAttribBinding->stride +
                                __glUtilCalculateStride(pAttrib->size, pAttrib->type);
                            if (endBytes > boundVBObj->size)
                            {
                                ret = GL_FALSE;
                                break;
                            }
                        }
                    }
                }

                index++;
                vsInputArrayMask >>= 1;
                attribEnabled >>= 1;
            }
        }
    }

    return ret;
}

/* Vertex array mode draw function */
GLvoid  __glDrawPrimitive(__GLcontext *gc, GLenum mode)
{
    ENTERFUNC_TM()

    if (mode != gc->vertexArray.primMode)
    {
        gc->vertexArray.primMode = mode;
        __GL_SET_VARRAY_MODE_BIT(gc);
    }

    if (!__glCheckVBOSize(gc))
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if (__GLSL_MODE_GRAPHICS != gc->shaderProgram.mode)
    {
        gc->shaderProgram.mode = __GLSL_MODE_GRAPHICS;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_MODE_SWITCH);
    }

#if gcdPATTERN_FAST_PATH
    if (__glDrawPattern(gc))
    {
        LEAVEFUNC_TM();
        return;
    }

#endif

    /* Realize render buffers change*/
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);

    if (__glDrawBegin(gc, mode))
    {
        GLboolean failed = GL_TRUE;

        do
        {
            /* Try to validate all states to HW */
            if (!__glDrawValidateState(gc))
            {
                break;
            }

            /* Try to issue draw command to HW */
            if (!gc->dp.drawPrimitive(gc))
            {
                break;
            }

            /* Try to clean up for this draw */
            if (!__glDrawEnd(gc))
            {
                break;
            }

            failed = GL_FALSE;
        } while (GL_FALSE);

        if (failed)
        {
            /* Mark commit states as invalid */
            gc->invalidCommonCommit = GL_TRUE;
            gc->invalidDrawCommit = GL_TRUE;
            gcmPRINT("ES30: some draw get error and skipped during validation");
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

    LEAVEFUNC_TM();
}

/***** End of draw functions *****************/

/****  Start of dispatch compute */
__GL_INLINE GLboolean __glComputeBegin(__GLcontext *gc)
{
    return (*gc->dp.computeBegin)(gc);
}


__GL_INLINE GLboolean __glComputeValidateState(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    __GLattribute *ds = &gc->commitState;

    if (gc->invalidCommonCommit)
    {
        /* Full dirty to flush all */
        __glOverturnCommitStates(gc);
        __glSetAttributeStatesDirty(gc);

        gc->invalidCommonCommit = GL_FALSE;
    }

    /* Program state must be checked before texture */
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS])
    {
        __glEvaluateProgramAttrib(gc, cs, ds);
    }

    if (!__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
    {
        __glEvaluateTextureAttrib(gc, cs, ds);
    }

    if (!__glBitmaskIsAllZero(&gc->imageUnitDirtyMask))
    {
        __glEvaluateImageAttrib(gc, cs, ds);
    }

    /*
    ** Note "computeValidateState" is designed to never fail. Otherwise GL states might still not
    ** be flush to HW before their dirty bits are cleared in the next draw's evaluations.
    ** If there are HW limitations that the draw must be skipped, set DrawNothing function in chip layer
    ** for those cases.
    */

    /* TODO: When we cover all logic of this function with dirty bits, we can check dirty before call into it*/
    return gc->dp.computeValidateState(gc);
}

__GL_INLINE GLboolean __glComputeEnd(__GLcontext *gc)
{
    (*gc->dp.computeEnd)(gc);

    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_UNIFORM_BUFFER_INDEX], GL_FALSE);
    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_ATOMIC_COUNTER_BUFFER_INDEX], GL_FALSE);
    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_SHADER_STORAGE_BUFFER_INDEX], GL_FALSE);

    __glBitmaskSetAll(&gc->shaderProgram.samplerMapDirty, GL_FALSE);
    gc->shaderProgram.samplerStateDirty = gc->shaderProgram.samplerStateKeepDirty;
    __glBitmaskSetAll(&gc->shaderProgram.samplerStateKeepDirty, GL_FALSE);

    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_FALSE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_FALSE);
    __GL_MEMZERO(gc->texUnitAttrState, gc->constants.shaderCaps.maxCombinedTextureImageUnits * sizeof(GLbitfield));
    gc->globalDirtyState[__GL_PROGRAM_ATTRS] = 0;

    gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_PROGRAM_ATTRS |
                                              __GL_ONE_32 << __GL_TEX_UNIT_ATTRS |
                                              __GL_ONE_32 << __GL_IMG_UNIT_ATTRS);

    return GL_TRUE;
}

GLvoid __glDispatchCompute(__GLcontext *gc)
{
    if (__GLSL_MODE_COMPUTE != gc->shaderProgram.mode)
    {
        gc->shaderProgram.mode = __GLSL_MODE_COMPUTE;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_MODE_SWITCH);
    }

    if (__glComputeBegin(gc))
    {
        GLboolean failed = GL_TRUE;

        do
        {
            /* Try to validate all states to HW */
            if (!__glComputeValidateState(gc))
            {
                break;
            }

            /* Try to issue compute command to HW */
            if (!gc->dp.dispatchCompute(gc))
            {
                break;
            }

            /* Try to clean up for this dispatch */
            if (!__glComputeEnd(gc))
            {
                break;
            }

            failed = GL_FALSE;
        } while (GL_FALSE);


        if (failed)
        {
            /* Mark commit states as invalid */
            gc->invalidCommonCommit = GL_TRUE;
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }
}

