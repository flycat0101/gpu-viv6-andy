/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_es_object_inline.c"

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE


extern __GLformatInfo __glFormatInfoTable[];
GLboolean __glDeleteTextureObject(__GLcontext *gc, __GLtextureObject *tex);
GLboolean __glDeleteSamplerObj(__GLcontext *gc, __GLsamplerObject *samplerObj);
extern GLvoid __glFramebufferTexture(__GLcontext *gc,
                                     __GLframebufferObject *framebufferObj,
                                     GLint attachIndex,
                                     __GLtextureObject *texObj,
                                     GLint level,
                                     GLint face,
                                     GLint layer,
                                     GLsizei samples,
                                     GLboolean layered,
                                     GLboolean isExtMode);

extern GLvoid __glUnBindImageTexture(__GLcontext *gc, GLuint unit, __GLtextureObject *texObj);

GLvoid __glFreeDefaultTextureObject(__GLcontext *gc, __GLtextureObject *tex)
{
    /* Notify Dp that this texture object is deleted */
    if (tex->privateData)
    {
        (*gc->dp.deleteTexture)(gc, tex);
    }

    /* Free texture unit list */
    __glFreeImageUserList(gc, &tex->texUnitBoundList);

    (*gc->imports.free)(gc, tex->faceMipmap);
}

GLvoid __glInitTextureObject(__GLcontext *gc, __GLtextureObject *tex, GLuint id, GLuint targetIndex)
{
    GLuint i, j;
    GLuint maxFaces = 1, maxDepths = 1, maxSlices = 1, maxLevels = 1;
    GLint requestedFormat = (targetIndex == __GL_TEXTURE_BINDING_BUFFER_EXT) ? GL_R8 : GL_RGBA;
    GLvoid *pointer = NULL;
    __GLmipMapLevel *mipmaps = NULL;

    tex->bindCount = 0;
    tex->seqNumber = 1;
    tex->immutable = GL_FALSE;

    /* Set default values for external texture. */
    if (targetIndex == __GL_TEXTURE_EXTERNAL_INDEX)
    {
        tex->params.sampler.sWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.tWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.rWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.minFilter = GL_LINEAR;
        tex->params.sampler.magFilter = GL_LINEAR;
    }
    else if (__GL_IS_TEXTURE_MSAA(targetIndex))
    {
        tex->params.sampler.sWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.tWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.rWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.sampler.minFilter = GL_NEAREST;
        tex->params.sampler.magFilter = GL_NEAREST;
    }
    else if (targetIndex == __GL_TEXTURE_BINDING_BUFFER_EXT)
    {
        tex->params.sampler.minFilter = GL_NEAREST;
        tex->params.sampler.magFilter = GL_NEAREST;
    }
    else
    {
        tex->params.sampler.sWrapMode = GL_REPEAT;
        tex->params.sampler.tWrapMode = GL_REPEAT;
        tex->params.sampler.rWrapMode = GL_REPEAT;
        tex->params.sampler.minFilter = GL_NEAREST_MIPMAP_LINEAR;
        tex->params.sampler.magFilter = GL_LINEAR;
    }

    tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;

    /* Initialize LOD */
    tex->params.sampler.minLod = -1000.0;
    tex->params.sampler.maxLod = 1000.0;
    tex->params.baseLevel = 0;
    tex->params.maxLevel = 1000;
    tex->params.dsTexMode = GL_DEPTH_COMPONENT;

    tex->params.sampler.compareMode = GL_NONE;
    tex->params.sampler.compareFunc = GL_LEQUAL;
    tex->params.sampler.maxAnistropy = 1.0f;
    tex->params.swizzle[0] = GL_RED;
    tex->params.swizzle[1] = GL_GREEN;
    tex->params.swizzle[2] = GL_BLUE;
    tex->params.swizzle[3] = GL_ALPHA;

    tex->mipBaseLevel = tex->params.baseLevel;
    tex->mipMaxLevel  = tex->params.maxLevel;

    tex->faceMipmap = NULL;
    tex->imageUpToDate = 0;
    tex->flag = 0;

    /* Initialize the privateData pointer */
    tex->privateData = NULL;

    tex->immutableLevels = 0;
    tex->arrays = 0;

    tex->name = id;
    tex->targetIndex = targetIndex;

    tex->samples = 0;
    tex->fixedSampleLocations = GL_TRUE;
    tex->samplesUsed = 0;

    tex->params.sampler.sRGB = GL_DECODE_EXT;
    tex->params.contentProtected = GL_FALSE;

    __GL_MEMSET(tex->params.sampler.borderColor.fv, 0, 4 * sizeof(GLuint));

    maxLevels = gc->constants.maxNumTextureLevels;
    switch (targetIndex)
    {
    case __GL_TEXTURE_2D_INDEX:
    case __GL_TEXTURE_2D_MS_INDEX:
        maxFaces  = 1;
        maxSlices =
        maxDepths = 1;
        break;
    case __GL_TEXTURE_3D_INDEX:
        maxFaces  = 1;
        maxSlices =
        maxDepths = gc->constants.maxTextureDepthSize;
        break;
    case __GL_TEXTURE_CUBEMAP_INDEX:
        maxSlices =
        maxFaces  = 6;
        maxDepths = 1;
        break;
    case __GL_TEXTURE_2D_ARRAY_INDEX:
    case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
        maxFaces  = 1;
        maxSlices =
        maxDepths = gc->constants.maxTextureArraySize;
        break;
    case __GL_TEXTURE_EXTERNAL_INDEX:
        maxFaces  = 1;
        maxDepths = 1;
        break;
    case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
        maxFaces  = 1;
        maxSlices =
        maxDepths = gc->constants.maxTextureArraySize * 6;
        break;
    case __GL_TEXTURE_BINDING_BUFFER_EXT:
        maxFaces  = 1;
        maxSlices =
        maxDepths = 1;
        maxLevels = 1;
        break;
    default:
 //       GL_ASSERT(0);
        ;
    }

    tex->maxFaces  = maxFaces;
    tex->maxLevels = maxLevels;
    tex->maxDepths = maxDepths;
    tex->maxSlices = maxSlices;

    pointer = gc->imports.calloc(gc, 1, maxFaces * sizeof(__GLmipMapLevel*) +
                                        maxFaces * maxLevels * sizeof(__GLmipMapLevel));

    tex->faceMipmap = (__GLmipMapLevel**)pointer;
    mipmaps = (__GLmipMapLevel*)(tex->faceMipmap + maxFaces);

    for (i = 0; i < maxFaces; i++)
    {
        tex->faceMipmap[i] = mipmaps;
        mipmaps += maxLevels;

        for (j = 0; j < maxLevels; j++)
        {
            tex->faceMipmap[i][j].requestedFormat = requestedFormat;
            tex->faceMipmap[i][j].formatInfo = &__glFormatInfoTable[__GL_FMT_MAX];
        }
    }
}

GLvoid __glFreeTextureState(__GLcontext *gc)
{
    GLuint i, j;

    for (i = 0; i < __GL_MAX_TEXTURE_BINDINGS; ++i)
    {
        /* unbind textures */
        for (j = 0; j < gc->constants.shaderCaps.maxCombinedTextureImageUnits; ++j)
        {
            __glBindTexture(gc, j, i, 0);
        }

        /* Free default texture object state */
        __glFreeDefaultTextureObject(gc, &gc->texture.defaultTextures[i]);
#ifdef OPENGL40
        /* Free proxy texture object state */
        __glFreeDefaultTextureObject(gc, &gc->texture.proxyTextures[i]);
#endif
    }

    /* Free shared texture object table */
    __glFreeSharedObjectState(gc, gc->texture.shared);
}

GLvoid __glInitTextureState(__GLcontext *gc)
{
    __GLtextureObject *tex;
#ifdef OPENGL40
    __GLtextureObject *proxyTex;
    __GLtextureEnvState *tes;
#endif
    GLint i, j;

    gc->state.texture.activeTexIndex = 0;

    /* Texture objects can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->texture.shared);
        gc->texture.shared = gc->shareCtx->texture.shared;
        gcoOS_LockPLS();
        gc->texture.shared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->texture.shared->lock)
        {
            gc->texture.shared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
            (*gc->imports.createMutex)(gc->texture.shared->lock);
        }
        gcoOS_UnLockPLS();
    }
    else
    {
        GL_ASSERT(NULL == gc->texture.shared);

        gc->texture.shared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for texture object */
        gc->texture.shared->maxLinearTableSize = __GL_MAX_TEXOBJ_LINEAR_TABLE_SIZE;
        gc->texture.shared->linearTableSize = __GL_DEFAULT_TEXOBJ_LINEAR_TABLE_SIZE;
        gc->texture.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->texture.shared->linearTableSize * sizeof(GLvoid *));

        gc->texture.shared->hashSize = __GL_TEXOBJ_HASH_TABLE_SIZE;
        gc->texture.shared->hashMask = __GL_TEXOBJ_HASH_TABLE_SIZE - 1;
        gc->texture.shared->refcount = 1;
        gc->texture.shared->deleteObject = (__GLdeleteObjectFunc)__glDeleteTextureObject;
        gc->texture.shared->immediateInvalid = GL_TRUE;
    }

    for (j = 0; j < __GL_MAX_TEXTURE_BINDINGS; j++)
    {
        /* Initialize default texture object state */
        tex = &gc->texture.defaultTextures[j];
        __glInitTextureObject(gc, tex, 0, j);

        for (i = 0; i < (GLint)gc->constants.shaderCaps.maxCombinedTextureImageUnits; i++)
        {
            gc->texture.units[i].boundTextures[j] = tex;

            /* By default, default texture object is bound to all texture units, so
            ** add each texture unit to default texture object's texUnitBoundList.
            */
            __glAddImageUser(gc, &tex->texUnitBoundList, (GLvoid*)(GLintptr)i);
        }
#ifdef OPENGL40
        /* Initialize proxy texture object state */
        proxyTex = &gc->texture.proxyTextures[j];
        __glInitTextureObject(gc, proxyTex, 0, j);
#endif
    }

    for (i = 0; i < (GLint)gc->constants.shaderCaps.maxCombinedTextureImageUnits; i++)
    {
        /* Initialize the current texture to NULL */
        gc->texture.units[i].currentTexture = NULL;
        gc->texture.units[i].boundSampler = NULL;
    }

    __glBitmaskInitAllZero(&gc->texture.currentEnableMask, gc->constants.shaderCaps.maxCombinedTextureImageUnits);
    __glBitmaskInitAllZero(&gc->texture.texConflict, gc->constants.shaderCaps.maxCombinedTextureImageUnits);

#ifdef OPENGL40
    /* Initialize the current texture coords */
    for (i = 0; i < __GL_MAX_TEXTURE_COORDS; i++) {
        gc->state.current.texture[i].f.x = __glZero;
        gc->state.current.texture[i].f.y = __glZero;
        gc->state.current.texture[i].f.z = __glZero;
        gc->state.current.texture[i].f.w = __glOne;
    }

    /* Init rest of texture state for each texture unit */
    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++) {
        gc->state.texture.texUnits[i].s.mode = GL_EYE_LINEAR;
        gc->state.texture.texUnits[i].s.eyePlaneEquation.f.x = __glOne;
        gc->state.texture.texUnits[i].s.objectPlaneEquation.f.x = __glOne;
        gc->state.texture.texUnits[i].t.mode = GL_EYE_LINEAR;
        gc->state.texture.texUnits[i].t.eyePlaneEquation.f.y = __glOne;
        gc->state.texture.texUnits[i].t.objectPlaneEquation.f.y = __glOne;
        gc->state.texture.texUnits[i].r.mode = GL_EYE_LINEAR;
        gc->state.texture.texUnits[i].q.mode = GL_EYE_LINEAR;

        /* Init each texture environment oglGc.state for each texture unit */
        tes = &gc->state.texture.texUnits[i].env;
        tes->mode = GL_MODULATE;
        tes->function.rgb = GL_MODULATE;
        tes->function.alpha = GL_MODULATE;
        tes->source[0].rgb = GL_TEXTURE;
        tes->source[1].rgb = GL_PREVIOUS;
        tes->source[2].rgb = GL_CONSTANT;
        tes->source[0].alpha = GL_TEXTURE;
        tes->source[1].alpha = GL_PREVIOUS;
        tes->source[2].alpha = GL_CONSTANT;
        tes->operand[0].rgb = GL_SRC_COLOR;
        tes->operand[1].rgb = GL_SRC_COLOR;
        tes->operand[2].rgb = GL_SRC_ALPHA;
        tes->operand[0].alpha = GL_SRC_ALPHA;
        tes->operand[1].alpha = GL_SRC_ALPHA;
        tes->operand[2].alpha = GL_SRC_ALPHA;
        tes->rgbScale = 1.0;
        tes->alphaScale = 1.0;
        tes->color.r = __glZero;
        tes->color.g = __glZero;
        tes->color.b = __glZero;
        tes->color.a = __glZero;
        tes->coordReplace = GL_FALSE;

        /* Initialize the current texture to NULL */
        gc->texture.units[i].currentTexture = NULL;
    }
#endif

}

GLboolean __glIsTextureComplete(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter,
                                GLenum magFilter, GLenum compareMode, GLint maxLevelUsed)
{
    __GLformatInfo *baseFmtInfo;
    __GLmipMapLevel *mipmap;
    GLint width, height, depth;
    GLint baseLevel;
    GLint face, level;
    GLint requestedFormat;
    GLint faces, arrays;

    baseLevel = texObj->params.baseLevel;
    width = texObj->faceMipmap[0][baseLevel].width;
    height = texObj->faceMipmap[0][baseLevel].height;
    depth = texObj->faceMipmap[0][baseLevel].depth;
    baseFmtInfo = texObj->faceMipmap[0][baseLevel].formatInfo;
    requestedFormat = texObj->faceMipmap[0][baseLevel].requestedFormat;
    arrays = texObj->faceMipmap[0][baseLevel].arrays;

    /* If each dimension of the level[base] array is positive. */
    if (0 == width || 0 == height || 0 == depth)
    {
        return GL_FALSE;
    }

    if (texObj->targetIndex == __GL_TEXTURE_2D_MS_INDEX ||
        texObj->targetIndex == __GL_TEXTURE_2D_MS_ARRAY_INDEX)
    {
        return GL_TRUE;
    }
    /* If cubemap base level have square dimension */
    if (texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX && (width != height))
    {
        return GL_FALSE;
    }

    if (GL_UNSIGNED_INT == baseFmtInfo->category || GL_INT == baseFmtInfo->category ||
        /* ES30 dis-allows linear filter for "TEXTURE_COMPARE_MODE=NONE" depth texture */
        (gc->apiVersion >= __GL_API_VERSION_ES30 && GL_NONE == compareMode &&
         (GL_DEPTH_COMPONENT == baseFmtInfo->baseFormat || GL_DEPTH_STENCIL == baseFmtInfo->baseFormat)
        )
       )
    {
        if (magFilter != GL_NEAREST)
        {
            return GL_FALSE;
        }
        if (minFilter != GL_NEAREST && minFilter != GL_NEAREST_MIPMAP_NEAREST)
        {
            return GL_FALSE;
        }
    }

    /* If baseLevel is less or equal to maxLevel */
    if (maxLevelUsed < baseLevel)
    {
        return GL_FALSE;
    }

    if (baseFmtInfo->glFormat == GL_DEPTH_STENCIL &&
        texObj->params.dsTexMode == GL_STENCIL_INDEX)
    {
        if (magFilter != GL_NEAREST || minFilter != GL_NEAREST)
        {
            return GL_FALSE;
        }
    }


    /* If the set of mipmap arrays have the same internalformat.
    ** If the dimensions of the arrays follows the *2 sequence.
    */
    faces = (texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX) ? 6 : 1;
    for (level = baseLevel; level <= maxLevelUsed; ++level)
    {
        /* If all the faces images have same dimension */
        for (face = 0; face < faces; ++ face)
        {
            mipmap = &texObj->faceMipmap[face][level];

            if (mipmap->requestedFormat != requestedFormat ||
                mipmap->width  != width ||
                mipmap->height != height ||
                mipmap->depth  != depth ||
                mipmap->arrays != arrays)
            {
                return GL_FALSE;
            }

            /*
            ** Fixme: driver chosen format may be not always same as request internal format because chosen format
            **        also consider "format" and "type" to match the incoming data.
            **        for the case:
            **              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, *level0);
            **              glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, *level1);
            **        The texture should be complete because internalFormats of the 2 levels are same, but actually
            **        level0 used RGB888 while level1 used RGB565, which is incomplete from HW level.
            **        Driver need to re-allocate chosen format consistent mipmaps to guarantee the texture completeness then.
            */
            GL_ASSERT(mipmap->formatInfo && mipmap->formatInfo->drvFormat == baseFmtInfo->drvFormat);
        }

        __GL_HALVE_SIZE(width);
        __GL_HALVE_SIZE(height);
        __GL_HALVE_SIZE(depth);
    }

    return GL_TRUE;
}


GLvoid __glInitImageState(__GLcontext *gc)
{
    GLuint i;
    __GLimageUnitState *imageUnit;
    for (i = 0; i < __GL_MAX_IMAGE_UNITS; i++)
    {
        imageUnit = &gc->state.image.imageUnit[i];
        imageUnit->texObj  = gcvNULL;
        imageUnit->level   = 0;
        imageUnit->layered = GL_FALSE;
        imageUnit->requestLayer = 0;
        imageUnit->access  = GL_READ_ONLY;
        imageUnit->format  = GL_R32UI;

        imageUnit->invalid = GL_TRUE;
    }
}

/***********************************************************************************/

__GL_INLINE GLvoid  __glTexParameterfv(__GLcontext *gc, GLuint unitIdx, GLuint targetIdx, GLenum pname,
                                      const GLfloat *pv)
{
    __GLtextureObject *tex = gc->texture.units[unitIdx].boundTextures[targetIdx];
    GLint param = __glFloat2NearestInt(pv[0]);
    GLuint64 dirty = 0;

    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            if (__GL_TEXTURE_EXTERNAL_INDEX == targetIdx)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_S_BIT;
                break;
            }
            /* fall through */
#ifdef OPENGL40
        case GL_CLAMP:
            if (gc->imports.coreProfile)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_S_BIT;
                break;
            }
#endif
        case GL_CLAMP_TO_EDGE:
            tex->params.sampler.sWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_S_BIT;
            break;
#ifdef OPENGL40
        /* GL_CLAMP_TO_BORDER is same as GL_CLAMP_TO_BORDER_EXT */
#endif
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_S_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_WRAP_T:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            if (__GL_TEXTURE_EXTERNAL_INDEX == targetIdx)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_T_BIT;
                break;
            }
            /* fall through */
#ifdef OPENGL40
        case GL_CLAMP:
            if (gc->imports.coreProfile)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_T_BIT;
                break;
            }
#endif
        case GL_CLAMP_TO_EDGE:
            tex->params.sampler.tWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_T_BIT;
            break;
#ifdef OPENGL40
        /* GL_CLAMP_TO_BORDER is same as GL_CLAMP_TO_BORDER_EXT */
#endif
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                tex->params.sampler.tWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_T_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_WRAP_R:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            if (__GL_TEXTURE_EXTERNAL_INDEX == targetIdx)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_R_BIT;
                break;
            }
            /* fall through */
#ifdef OPENGL40
        case GL_CLAMP:
            if (gc->imports.coreProfile)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            else
            {
                tex->params.sampler.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_R_BIT;
                break;
            }
#endif
        case GL_CLAMP_TO_EDGE:
            tex->params.sampler.rWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_R_BIT;
            break;
#ifdef OPENGL40
        /* GL_CLAMP_TO_BORDER is same as GL_CLAMP_TO_BORDER_EXT */
#endif
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                tex->params.sampler.rWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_R_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MIN_FILTER:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
            if (__GL_TEXTURE_EXTERNAL_INDEX == targetIdx)
            {
                __GL_ERROR_RET(GL_INVALID_ENUM);
            }
            /* fall through */
        case GL_NEAREST:
        case GL_LINEAR:
            tex->params.sampler.minFilter = (GLenum)param;
            dirty = __GL_TEXPARAM_MIN_FILTER_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MAG_FILTER:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_NEAREST:
        case GL_LINEAR:
            tex->params.sampler.magFilter = (GLenum)param;
            dirty = __GL_TEXPARAM_MAG_FILTER_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

#ifdef OPENGL40
      case GL_TEXTURE_PRIORITY:
          tex->params.sampler.priority = __glClampf(pv[0], __glZero, __glOne);
          dirty = __GL_TEXPARAM_PRIORITY_BIT;
          break;

      case GL_TEXTURE_LOD_BIAS:
          tex->params.sampler.lodBias = (GLfloat)pv[0];
          dirty = __GL_TEXPARAM_LOD_BIAS_BIT;
          break;

      case GL_DEPTH_TEXTURE_MODE:
          switch ((GLenum)pv[0]) {
              case GL_LUMINANCE:
              case GL_INTENSITY:
              case GL_ALPHA:
                  tex->params.sampler.depthTexMode = (GLenum)pv[0];
                  dirty = __GL_TEXPARAM_DEPTH_TEX_MODE_BIT;
                  /* border color dirty bit will be set after evaluating border color dirty,
                  ** otherwise the dirty bit may be cleared when evaluate. */
                  break;
              default:
                  __GL_ERROR_RET(GL_INVALID_ENUM);
          }
          break;

      case GL_GENERATE_MIPMAP:
          if (((GLboolean)pv[0]) == GL_TRUE || ((GLboolean)pv[0]) == GL_FALSE) {
              tex->params.sampler.generateMipmap = ((GLboolean)pv[0]);
              dirty = __GL_TEXPARAM_GENERATE_MIPMAP_BIT;
          }
          else {
              __GL_ERROR_RET(GL_INVALID_ENUM);
          }
          break;
      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
          {
              GLfloat f = pv[0];
              if ( f< 0 )     f = 0;
              if ( f> 1.0f )  f = 1.0f;
              tex->params.sampler.compareFailValue = f;
              dirty = __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT;
          }
          break;
#endif

    case GL_TEXTURE_MIN_LOD:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        tex->params.sampler.minLod = pv[0];
        dirty = __GL_TEXPARAM_MIN_LOD_BIT;
        break;

    case GL_TEXTURE_MAX_LOD:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        tex->params.sampler.maxLod = pv[0];
        dirty = __GL_TEXPARAM_MAX_LOD_BIT;
        break;

    case GL_TEXTURE_BASE_LEVEL:
        if (param >= 0)
        {
            tex->params.baseLevel = tex->immutable
                                  ? __glClampi(param, 0, tex->immutableLevels - 1)
                                  : param;
            dirty = __GL_TEXPARAM_BASE_LEVEL_BIT;

            if (tex->params.mipHint != __GL_TEX_MIP_HINT_AUTO &&
                tex->params.baseLevel < tex->mipBaseLevel)
            {
                tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;
                dirty |= __GL_TEXPARAM_MIP_HINT_BIT;
            }

            if ((param > 0) &&
                ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) || (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx)))
            {
                __GL_ERROR_RET(GL_INVALID_OPERATION);
            }

            tex->uObjStateDirty.s.baseLevelDirty = GL_TRUE;
        }
        else
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        break;

    case GL_TEXTURE_MAX_LEVEL:
        if (param >= 0)
        {
            tex->params.maxLevel = tex->immutable
                                 ? __glClampi(param, tex->params.baseLevel, tex->immutableLevels - 1)
                                 : param;
            dirty = __GL_TEXPARAM_MAX_LEVEL_BIT;

            if (tex->params.mipHint != __GL_TEX_MIP_HINT_AUTO &&
                tex->params.maxLevel > tex->mipMaxLevel)
            {
                tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;
                dirty |= __GL_TEXPARAM_MIP_HINT_BIT;
            }
        }
        else
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        break;

    case GL_TEXTURE_COMPARE_MODE:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_NONE:
        case GL_COMPARE_REF_TO_TEXTURE:
            tex->params.sampler.compareMode = (GLenum)param;
            dirty = __GL_TEXPARAM_COMPARE_MODE_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_COMPARE_FUNC:
        if ((__GL_TEXTURE_2D_MS_INDEX == targetIdx) ||
            (__GL_TEXTURE_2D_MS_ARRAY_INDEX == targetIdx))
        {
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

        switch (param)
        {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            tex->params.sampler.compareFunc = (GLenum)param;
            dirty = __GL_TEXPARAM_COMPARE_FUNC_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_SWIZZLE_R:
    case GL_TEXTURE_SWIZZLE_G:
    case GL_TEXTURE_SWIZZLE_B:
    case GL_TEXTURE_SWIZZLE_A:
        switch (param)
        {
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_ZERO:
        case GL_ONE:
            {
                GLuint channel = pname - GL_TEXTURE_SWIZZLE_R;
                if (tex->params.swizzle[channel] != (GLenum)param)
                {
                    tex->params.swizzle[channel] = (GLenum)param;
                    dirty = __GL_TEXPARAM_SWIZZLE_R_BIT << channel;
                    tex->uObjStateDirty.s.swizzleDirty = GL_TRUE;
                }
            }
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_DEPTH_STENCIL_TEXTURE_MODE:
        switch (param)
        {
        case GL_DEPTH_COMPONENT:
        case GL_STENCIL_INDEX:
            if (tex->params.dsTexMode != (GLenum)param)
            {
                tex->params.dsTexMode = (GLenum)param;
                dirty = __GL_TEXPARAM_DS_TEXMODE_BIT;
                tex->uObjStateDirty.s.dsModeDirty = GL_TRUE;
            }
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        if (pv[0] >= 1.0f)
        {
            tex->params.sampler.maxAnistropy = pv[0];
            dirty = __GL_TEXPARAM_MAX_ANISOTROPY_BIT;
        }
        else
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        break;

    case GL_TEXTURE_SRGB_DECODE_EXT:
        switch(param)
        {
        case GL_DECODE_EXT:
        case GL_SKIP_DECODE_EXT:
            tex->params.sampler.sRGB = (GLenum)param;
            dirty = __GL_TEXPARAM_SRGB_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;
#ifdef OPENGL40
        /* GL_TEXTURE_BORDER_COLOR is same as GL_TEXTURE_BORDER_COLOR_EXT */
#endif
    case GL_TEXTURE_BORDER_COLOR_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
        {
            __GL_MEMCOPY(tex->params.sampler.borderColor.fv, pv, 4 * sizeof(GLfloat));
            dirty = __GL_TEXPARAM_BORDER_COLOR_BIT;
            break;
        }

    case GL_TEXTURE_PROTECTED_VIV:
        tex->params.contentProtected = (GLboolean)param;
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    __GL_SET_TEX_UNIT_BIT(gc, unitIdx, dirty);
    tex->seqNumber++;
}

GLvoid GL_APIENTRY __gles_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *pv)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    __GLimageUser *imageUserList;

    __GL_HEADER();

    switch (target)
    {
#ifdef OPENGL40
    case GL_TEXTURE_1D:
        targetIdx = __GL_TEXTURE_1D_INDEX;
        break;
#endif
    case GL_TEXTURE_2D:
        targetIdx = __GL_TEXTURE_2D_INDEX;
        break;
#ifdef OPENGL40
    case GL_TEXTURE_1D_ARRAY:
        targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;
#endif
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIdx = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, pv);
        }
        imageUserList = imageUserList->next;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat f)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_HEADER();

    /* Accept only enumerators that correspond to single values */
    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
    case GL_TEXTURE_WRAP_T:
    case GL_TEXTURE_WRAP_R:
    case GL_TEXTURE_MIN_FILTER:
    case GL_TEXTURE_MAG_FILTER:
    case GL_TEXTURE_MIN_LOD:
    case GL_TEXTURE_MAX_LOD:
    case GL_TEXTURE_BASE_LEVEL:
    case GL_TEXTURE_MAX_LEVEL:
    case GL_TEXTURE_COMPARE_MODE:
    case GL_TEXTURE_COMPARE_FUNC:
    case GL_TEXTURE_SWIZZLE_R:
    case GL_TEXTURE_SWIZZLE_G:
    case GL_TEXTURE_SWIZZLE_B:
    case GL_TEXTURE_SWIZZLE_A:
    case GL_DEPTH_STENCIL_TEXTURE_MODE:
    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
    case GL_TEXTURE_SRGB_DECODE_EXT:
    case GL_TEXTURE_PROTECTED_VIV:
#ifdef OPENGL40
    case GL_TEXTURE_PRIORITY:
    case GL_TEXTURE_LOD_BIAS:
    case GL_GENERATE_MIPMAP:
    case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
    case GL_TEXTURE_RESIDENT:
#endif
        break;
#ifdef OPENGL40
    case GL_DEPTH_TEXTURE_MODE:
        if (gc->imports.coreProfile)
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        else
        {
            break;
        }
#endif
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (target)
    {
#ifdef OPENGL40
    case GL_TEXTURE_1D:
        targetIdx = __GL_TEXTURE_1D_INDEX;
        break;
#endif
    case GL_TEXTURE_2D:
      targetIdx = __GL_TEXTURE_2D_INDEX;
      break;
#ifdef OPENGL40
    case GL_TEXTURE_1D_ARRAY:
        targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;
#endif
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
      targetIdx = __GL_TEXTURE_3D_INDEX;
      break;
    case GL_TEXTURE_CUBE_MAP:
       targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
       break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    tmpf[0] = f;

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, tmpf);
        }
        imageUserList = imageUserList->next;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *pv)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_HEADER();

    switch (target)
    {
#ifdef OPENGL40
    case GL_TEXTURE_1D:
        targetIdx = __GL_TEXTURE_1D_INDEX;
        break;
#endif
    case GL_TEXTURE_2D:
        targetIdx = __GL_TEXTURE_2D_INDEX;
        break;
#ifdef OPENGL40
    case GL_TEXTURE_1D_ARRAY:
        targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;
#endif
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIdx = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(tmpf, pv, 4 * sizeof(GLfloat));
    }
    else
    {
        tmpf[0] = (GLfloat)pv[0];
    }

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, tmpf);
        }
        imageUserList = imageUserList->next;
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint val)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_HEADER();

    /* Accept only enumerators that correspond to single values */
    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
    case GL_TEXTURE_WRAP_T:
    case GL_TEXTURE_WRAP_R:
    case GL_TEXTURE_MIN_FILTER:
    case GL_TEXTURE_MAG_FILTER:
    case GL_TEXTURE_MIN_LOD:
    case GL_TEXTURE_MAX_LOD:
    case GL_TEXTURE_BASE_LEVEL:
    case GL_TEXTURE_MAX_LEVEL:
    case GL_TEXTURE_COMPARE_MODE:
    case GL_TEXTURE_COMPARE_FUNC:
    case GL_TEXTURE_SWIZZLE_R:
    case GL_TEXTURE_SWIZZLE_G:
    case GL_TEXTURE_SWIZZLE_B:
    case GL_TEXTURE_SWIZZLE_A:
    case GL_DEPTH_STENCIL_TEXTURE_MODE:
    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
    case GL_TEXTURE_SRGB_DECODE_EXT:
    case GL_TEXTURE_PROTECTED_VIV:
#ifdef OPENGL40
    case GL_TEXTURE_PRIORITY:
    case GL_TEXTURE_LOD_BIAS:
    case GL_GENERATE_MIPMAP:
    case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
    case GL_TEXTURE_RESIDENT:
#endif
        break;
#ifdef OPENGL40
    case GL_DEPTH_TEXTURE_MODE:
        if (gc->imports.coreProfile)
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        else
        {
            break;
        }
#endif
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (target)
    {
#ifdef OPENGL40
    case GL_TEXTURE_1D:
        targetIdx = __GL_TEXTURE_1D_INDEX;
        break;
#endif
    case GL_TEXTURE_2D:
        targetIdx = __GL_TEXTURE_2D_INDEX;
        break;
#ifdef OPENGL40
    case GL_TEXTURE_1D_ARRAY:
        targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;
#endif
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIdx = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    tmpf[0] = (GLfloat)val;

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, tmpf);
        }
        imageUserList = imageUserList->next;
    }

OnError:
    __GL_FOOTER();
}

__GL_INLINE GLvoid
__glGetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *v)
{
    __GLtextureUnit *unit = &(gc->texture.units[gc->state.texture.activeTexIndex]);
    __GLtextureParamState *params;
    __GLtextureObject *tex;

    switch (target)
    {
#ifdef OPENGL40
      case GL_TEXTURE_1D:
          tex = unit->boundTextures[__GL_TEXTURE_1D_INDEX];
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          tex = unit->boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          tex = unit->boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];
          break;
#endif
#endif
    case GL_TEXTURE_2D:
        tex = unit->boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_2D_ARRAY:
        tex = unit->boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_3D:
        tex = unit->boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP:
        tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        tex = unit->boundTextures[__GL_TEXTURE_EXTERNAL_INDEX];
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        tex = unit->boundTextures[__GL_TEXTURE_2D_MS_INDEX];
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        tex = unit->boundTextures[__GL_TEXTURE_2D_MS_ARRAY_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            break;
        }
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    params = &tex->params;
    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
        v[0] = (GLfloat)params->sampler.sWrapMode;
        break;
    case GL_TEXTURE_WRAP_T:
        v[0] = (GLfloat)params->sampler.tWrapMode;
        break;
    case GL_TEXTURE_WRAP_R:
        v[0] = (GLfloat)params->sampler.rWrapMode;
        break;
    case GL_TEXTURE_MIN_FILTER:
        v[0] = (GLfloat)params->sampler.minFilter;
        break;
    case GL_TEXTURE_MAG_FILTER:
        v[0] = (GLfloat)params->sampler.magFilter;
        break;
    case GL_TEXTURE_MIN_LOD:
        v[0] = (GLfloat)params->sampler.minLod;
        break;
    case GL_TEXTURE_MAX_LOD:
        v[0] = (GLfloat)params->sampler.maxLod;
        break;
    case GL_TEXTURE_COMPARE_MODE:
        v[0] = (GLfloat)params->sampler.compareMode;
        break;
    case GL_TEXTURE_COMPARE_FUNC:
        v[0] = (GLfloat)params->sampler.compareFunc;
        break;
    case GL_DEPTH_STENCIL_TEXTURE_MODE:
        v[0] = (GLfloat)params->dsTexMode;
        break;
    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        v[0] = (GLfloat)params->sampler.maxAnistropy;
        break;
    case GL_TEXTURE_BASE_LEVEL:
        v[0] = (GLfloat)params->baseLevel;
        break;
    case GL_TEXTURE_MAX_LEVEL:
        v[0] = (GLfloat)params->maxLevel;
        break;
    case GL_TEXTURE_SWIZZLE_R:
        v[0] = (GLfloat)params->swizzle[0];
        break;
    case GL_TEXTURE_SWIZZLE_G:
        v[0] = (GLfloat)params->swizzle[1];
        break;
    case GL_TEXTURE_SWIZZLE_B:
        v[0] = (GLfloat)params->swizzle[2];
        break;
    case GL_TEXTURE_SWIZZLE_A:
        v[0] = (GLfloat)params->swizzle[3];
        break;
    case GL_TEXTURE_IMMUTABLE_FORMAT:
        v[0] = (GLfloat)tex->immutable;
        break;
    case GL_TEXTURE_IMMUTABLE_LEVELS:
        v[0] = (GLfloat)tex->immutableLevels;
        break;
    case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
        v[0] = 1;
        break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        v[0] = (GLfloat)params->sampler.sRGB;
        break;
        /* Special handling for cl_khr_gl_sharing. */
#define GL_TEXTURE_WIDTH            0x1000
#define GL_TEXTURE_HEIGHT           0x1001
#define GL_TEXTURE_DEPTH            0x8071
#define GL_TEXTURE_INTERNAL_FORMAT  0x1003

    case GL_TEXTURE_WIDTH:
        v[0] = (GLfloat)tex->faceMipmap[0][0].width;
        break;
    case GL_TEXTURE_HEIGHT:
        v[0] = (GLfloat)tex->faceMipmap[0][0].height;
        break;
    case GL_TEXTURE_DEPTH:
        v[0] = (GLfloat)tex->faceMipmap[0][0].depth;
        break;
    case GL_TEXTURE_INTERNAL_FORMAT:
        v[0] = (GLfloat)tex->faceMipmap[0][0].baseFormat;
        break;
    case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
        v[0] = (GLfloat)GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;
        break;

    case GL_TEXTURE_BORDER_COLOR_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
        {
            __GL_MEMCOPY(v, params->sampler.borderColor.fv, 4 * sizeof(GLfloat));
            break;
        }

    case GL_TEXTURE_PROTECTED_VIV:
        v[0] = (GLfloat) params->contentProtected;
        break;
#ifdef OPENGL40
      case GL_DEPTH_TEXTURE_MODE:
          v[0] = (GLfloat)params->sampler.depthTexMode;
          break;
      case GL_TEXTURE_LOD_BIAS:
          v[0] = (GLfloat)params->sampler.lodBias;
          break;
      case GL_TEXTURE_PRIORITY:
          v[0] = (GLfloat)params->sampler.priority;
          break;
      case GL_GENERATE_MIPMAP:
          v[0] = (GLfloat)params->sampler.generateMipmap;
          break;
      case GL_TEXTURE_RESIDENT:
         /*
          v[0] = (*gc->dp.isTextureResident)(gc, tex);
          */
          v[0] = 0.0;
          break;
#endif
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }
}


GLvoid GL_APIENTRY __gles_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat v[])
{
    __GL_HEADER();

    __glGetTexParameterfv(gc, target, pname, v);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0f};

    __GL_HEADER();

    __glGetTexParameterfv(gc, target, pname, tmpf);

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(v, tmpf, 4 * sizeof(GLfloat));
    }
    else
    {
        if (tmpf[0] < 0)
        {
            v[0] = (GLint)(tmpf[0] - 0.5f);
        }
        else
        {
            v[0] = (GLint)(tmpf[0] + 0.5f);
        }
    }

    __GL_FOOTER();
}

__GL_INLINE GLvoid
__glGetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GLtextureUnit *unit = &(gc->texture.units[gc->state.texture.activeTexIndex]);
    __GLtextureObject *tex;
    GLuint face = 0;
    __GLmipMapLevel *faceMipmap;
    __GLformatInfo *formatInfo;
    GLint max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    if (level < 0 || level > max_lod)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    switch (target)
    {
#ifdef OPENGL40
      case GL_TEXTURE_1D:
          tex = unit->boundTextures[__GL_TEXTURE_1D_INDEX];
          break;
      case GL_PROXY_TEXTURE_1D:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_1D_INDEX];
          break;
      case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
          break;
      case GL_PROXY_TEXTURE_2D:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_2D_INDEX];
          break;
      case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
          break;
      case GL_PROXY_TEXTURE_3D:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_3D_INDEX];
          break;
      case GL_PROXY_TEXTURE_CUBE_MAP:
          tex = &gc->texture.proxyTextures[__GL_TEXTURE_CUBEMAP_INDEX];
          break;
#endif
    case GL_TEXTURE_2D:
        tex = unit->boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_2D_ARRAY:
        tex = unit->boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_3D:
        tex = unit->boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        tex = unit->boundTextures[__GL_TEXTURE_EXTERNAL_INDEX];
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        tex = unit->boundTextures[__GL_TEXTURE_2D_MS_INDEX];
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        tex = unit->boundTextures[__GL_TEXTURE_2D_MS_ARRAY_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            break;
        }
    case GL_TEXTURE_BUFFER_EXT:
        tex = unit->boundTextures[__GL_TEXTURE_BINDING_BUFFER_EXT];
        if (level != 0)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE)
        }
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    faceMipmap = &tex->faceMipmap[face][level];
    formatInfo = faceMipmap->formatInfo;

    switch(pname)
    {
    case GL_TEXTURE_WIDTH:
        params[0] = faceMipmap->width;
        break;

    case GL_TEXTURE_HEIGHT:
        params[0] = faceMipmap->height;
        break;

    case GL_TEXTURE_DEPTH:
        params[0] = __GL_IS_TEXTURE_ARRAY(tex->targetIndex) ? faceMipmap->arrays : faceMipmap->depth;
        break;

    case GL_TEXTURE_SAMPLES:
        params[0] = tex->samplesUsed;
        break;

    case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
        params[0] = tex->fixedSampleLocations;
        break;

    case GL_TEXTURE_INTERNAL_FORMAT:
#ifdef OPENGL40
        params[0] = faceMipmap->requestedFormat;
#else
        params[0] = formatInfo->glFormat;
#endif
        break;

    case GL_TEXTURE_RED_SIZE:
        params[0] = formatInfo->redSize;
        break;

    case GL_TEXTURE_GREEN_SIZE:
        params[0] = formatInfo->greenSize;
        break;

    case GL_TEXTURE_BLUE_SIZE:
        params[0] = formatInfo->blueSize;
        break;

    case GL_TEXTURE_ALPHA_SIZE:
        params[0] = formatInfo->alphaSize;
        break;

    case GL_TEXTURE_DEPTH_SIZE:
        params[0] = formatInfo->depthSize;
        break;

    case GL_TEXTURE_STENCIL_SIZE:
        params[0] = formatInfo->stencilSize;
        break;

    case GL_TEXTURE_SHARED_SIZE:
        params[0] = formatInfo->sharedSize;
        break;

    case GL_TEXTURE_RED_TYPE:
        params[0] = formatInfo->redType;
        break;

    case GL_TEXTURE_GREEN_TYPE:
        params[0] = formatInfo->greenType;
        break;

    case GL_TEXTURE_BLUE_TYPE:
        params[0] = formatInfo->blueType;
        break;

   case GL_TEXTURE_ALPHA_TYPE:
        params[0] = formatInfo->alphaType;
        break;

    case GL_TEXTURE_DEPTH_TYPE:
        params[0] = formatInfo->depthType;
        break;

    case GL_TEXTURE_COMPRESSED:
        params[0] = formatInfo->compressed;
        break;

    case GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT:
        params[0] = tex->bufObj ? (GLint)tex->bufObj->name : 0;
        break;

    case GL_TEXTURE_BUFFER_OFFSET_EXT:
        params[0] = tex->bufOffset;
        break;

    case GL_TEXTURE_BUFFER_SIZE_EXT:
        params[0] = tex->bufSize ? tex->bufSize : (tex->bufObj ? (GLint)tex->bufObj->size: 0);
        break;
#ifdef OPENGL40
      case GL_TEXTURE_BORDER:
          params[0] = faceMipmap->border;
          break;
      case GL_TEXTURE_LUMINANCE_SIZE:
          switch (formatInfo->dataFormat) {
          case GL_LUMINANCE:
          case GL_LUMINANCE_ALPHA:
              params[0] = formatInfo->redSize;
              break;
          default:
              params[0] = 0;
              break;
          }
          break;
      case GL_TEXTURE_INTENSITY_SIZE:
          switch (formatInfo->dataFormat) {
          case GL_INTENSITY:
              params[0] = formatInfo->redSize;
              break;
          default:
              params[0] = 0;
              break;
          }
          break;
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
          switch (target) {
              case GL_PROXY_TEXTURE_1D:
              case GL_PROXY_TEXTURE_2D:
              case GL_PROXY_TEXTURE_3D:
              case GL_PROXY_TEXTURE_CUBE_MAP:
                  __GL_ERROR_RET(GL_INVALID_ENUM);
          }
          if (faceMipmap->compressed == GL_FALSE) {
              __GL_ERROR_RET(GL_INVALID_OPERATION)
          }
          params[0] = faceMipmap->compressedSize;
          break;
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
          switch (formatInfo->dataFormat) {
          case GL_LUMINANCE:
          case GL_LUMINANCE_ALPHA:
              params[0] = formatInfo->redType;
              break;
          default:
              params[0] = 0;
              break;
          }
          break;
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
          switch (formatInfo->dataFormat) {
          case GL_INTENSITY:
              params[0] = formatInfo->redType;
              break;
          default:
              params[0] = 0;
              break;
          }
          break;
#endif
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

}


GLvoid GL_APIENTRY __gles_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level,
                                                 GLenum pname, GLint *params)
{
    __GL_HEADER();

    __glGetTexLevelParameteriv(gc, target, level, pname, params);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level,
                                                 GLenum pname, GLfloat *params)
{
    GLint tmpi = 0;

    __GL_HEADER();

    __glGetTexLevelParameteriv(gc, target, level, pname, &tmpi);

    params[0] = (GLfloat)tmpi;

    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_ActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GL_HEADER();

    if ((texture > (GL_TEXTURE0 + gc->constants.shaderCaps.maxCombinedTextureImageUnits - 1)) ||
        (texture < GL_TEXTURE0))
    {
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    texture = texture - GL_TEXTURE0;
    gc->state.texture.activeTexIndex = texture;

    __GL_FOOTER();
}

/*
** This function actually performs bind texture.
*/
GLvoid __glBindTexture(__GLcontext *gc, GLuint unitIdx, GLuint targetIndex, GLuint texture)
{
    __GLtextureObject *texObj;
    __GLtextureObject *boundTexObj = gc->texture.units[unitIdx].boundTextures[targetIndex];

    if (texture == 0)
    {
        /* Retrieve the default texture object in __GLcontext.
        */
        texObj = &gc->texture.defaultTextures[targetIndex];
        GL_ASSERT(texObj && texObj->name == 0);
    }
    else
    {
        /*
        ** Retrieve the texture object from the "gc->texture.shared" structure.
        */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
    }

    /* If same object was bound */
    if (texObj == boundTexObj)
    {
        return;
    }

    if (NULL == texObj)
    {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new texture object and initialize it.
        */
        texObj = (__GLtextureObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLtextureObject));
        __glInitTextureObject(gc, texObj, texture, targetIndex);

        /* Add this texture object to the "gc->texture.shared" structure.
        */
        __glAddObject(gc, gc->texture.shared, texture, texObj);

        /* Mark the name "texture" used in the texture nameArray.
        */
        __glMarkNameUsed(gc, gc->texture.shared, texture);
    }
    else
    {
        /*
        ** Retrieved an existing texture object.  Do some sanity checks.
        */
        if (texObj->targetIndex != targetIndex)
        {
            __GL_ERROR_RET(GL_INVALID_OPERATION);
        }
    }

    /* Release the previously bound texture for this target.
    ** And install the new texture object to the target.
    */
    gc->texture.units[unitIdx].boundTextures[targetIndex] = texObj;

    /* Add current active texture unit index to current bound texture object's texUnitBoundList */
    __glAddImageUser(gc, &texObj->texUnitBoundList, (GLvoid*)(GLintptr)unitIdx);

    /* Delete boundTexObj if there is nothing bound to the object */
    if (boundTexObj && boundTexObj->name != 0)
    {
        if ((--boundTexObj->bindCount) == 0 && !boundTexObj->fboList && !boundTexObj->imageList && (boundTexObj->flag & __GL_OBJECT_IS_DELETED)) {
            __glDeleteTextureObject(gc, boundTexObj);
        }
    }

    /* bindCount includes both single context and shared context bindings.
    */
    if (texObj->name)
    {
        texObj->bindCount++;
    }

    /* Set all texParamter and texture image dirty bits.
    */
    __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS);

    /* Call the dp interface */
    (*gc->dp.bindTexture)(gc, texObj);
}

GLvoid GL_APIENTRY __gles_BindTexture(__GLcontext *gc, GLenum target, GLuint texture)
{
    GLuint targetIndex;

    __GL_HEADER();

#ifdef OPENGL40
    if (gc->imports.coreProfile && texture &&!__glIsNameDefined(gc, gc->texture.shared, texture))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
#endif

    switch (target)
    {
#ifdef OPENGL40
    case GL_TEXTURE_1D:
        targetIndex = __GL_TEXTURE_1D_INDEX;
        break;
#endif
    case GL_TEXTURE_2D:
        targetIndex = __GL_TEXTURE_2D_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIndex = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIndex = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
/* It is same as GL_TEXTURE_2D_ARRAY_EXT */
    case GL_TEXTURE_2D_ARRAY:
        targetIndex = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;

    case GL_TEXTURE_EXTERNAL_OES:
        targetIndex = __GL_TEXTURE_EXTERNAL_INDEX;
        break;

    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIndex = __GL_TEXTURE_2D_MS_INDEX;
        break;

    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIndex = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;

#ifdef OPENGL40

#if GL_ARB_texture_rectangle
    case GL_TEXTURE_RECTANGLE_ARB:
        targetIndex = __GL_TEXTURE_RECTANGLE_INDEX;
        break;
#endif
    case GL_TEXTURE_1D_ARRAY_EXT:
        targetIndex = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;
#endif

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIndex = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }

    case GL_TEXTURE_BUFFER_EXT:
        if(__glExtension[__GL_EXTID_EXT_texture_buffer].bEnabled)
        {
            targetIndex = __GL_TEXTURE_BINDING_BUFFER_EXT;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __glBindTexture(gc,gc->state.texture.activeTexIndex, targetIndex, texture);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /*
    ** If a texture that is being deleted is currently bound,
    ** bind the default texture to its target.
    */
    for (i = 0; i < n; i++)
    {
        /* skip default textures */
        if (textures[i])
        {
            __glDeleteObject(gc, gc->texture.shared, textures[i]);
        }
    }

OnError:
    __GL_FOOTER();
}

GLboolean __glDeleteTextureObject(__GLcontext *gc, __GLtextureObject *tex)
{
    GLuint i;
    GLuint targetIndex = tex->targetIndex;
    __GLimageUser *texUserList = tex->texUnitBoundList;
    __GLimageUser *fboUserList = tex->fboList;
    __GLimageUser *imageList = tex->imageList;
    __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
    __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

    /*
    ** __GL_OBJECT_IS_DELETED is cleared here because we do not want this object
    ** deleted in the following bind functions, __glBindTexture, __glFramebufferTexture.
    ** otherwise there will be recursion: __glDeleteTextureObject-->__glBindObject-->__glDeleteTextureObject,
    ** and the object will be deleted twice.
    */
    tex->flag &= ~__GL_OBJECT_IS_DELETED;

    /*
    ** If the texture object that is being deleted is currently bound to any texture units,
    ** unbind the texobj from texture units and bind the default texobj 0 to texture units.
    */
    while (texUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(texUserList->imageUser);

        if (tex == gc->texture.units[unitIdx].boundTextures[targetIndex])
        {
            __glBindTexture(gc, unitIdx, targetIndex, 0);
        }

        if (tex == gc->texture.units[unitIdx].currentTexture)
        {
            gc->texture.units[unitIdx].currentTexture = NULL;
        }

        texUserList = texUserList->next;
    }

    /* Unbind the texobj from the FBOs that use this texobj.
    */
    while (fboUserList)
    {
        /* fboUserList may be freed in __glFramebufferTexture, get the next in the beginning */
        __GLimageUser *nextUser = fboUserList->next;
        __GLframebufferObject *fbo = (__GLframebufferObject*)fboUserList->imageUser;

        if (fbo == drawFbo)
        {
            for (i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if (drawFbo->attachPoint[i].objType == GL_TEXTURE &&
                    drawFbo->attachPoint[i].object  == (GLvoid*)tex)
                {
                    __glFramebufferTexture(gc, drawFbo, i, NULL, 0, 0, 0, 0, GL_FALSE, GL_FALSE);
                }
            }
        }

        if (readFbo != drawFbo && fbo == readFbo)
        {
            for (i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if (readFbo->attachPoint[i].objType == GL_TEXTURE &&
                    readFbo->attachPoint[i].object  == (GLvoid*)tex)
                {
                    __glFramebufferTexture(gc, readFbo, i, NULL, 0, 0, 0, 0, GL_FALSE, GL_FALSE);
                }
            }
        }

        /* Mark dirty all the fbos this texture attached to */
        __GL_FRAMEBUFFER_COMPLETE_DIRTY(fbo);

        fboUserList = nextUser;
    }

    while (imageList)
    {
        __GLimageUser *nextUser = imageList->next;
        GLuint unit = __GL_PTR2INT(imageList->imageUser);
        __glUnBindImageTexture(gc, unit, tex);
        imageList = nextUser;
    }

    /* Detach texture chip objects from this context */
    (*gc->dp.detachTexture)(gc, tex);

    /* Do not delete the texObj if there are other texture units or contexts bound to it. */
    if (tex->bindCount != 0 || tex->fboList)
    {
        /* Set the flag to indicate the object is marked for delete */
        tex->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    if (tex->label)
    {
        gc->imports.free(gc, tex->label);
    }

    /* Notify Dp that this texture object is deleted.
    */
    if (tex->privateData)
    {
        (*gc->dp.deleteTexture)(gc, tex);
    }

    /* Delete the texture object's texture image */
    if (tex->faceMipmap)
    {
        (*gc->imports.free)(gc, tex->faceMipmap);
        tex->faceMipmap = NULL;
    }

    /* Free fbolist */
    __glFreeImageUserList(gc, &tex->fboList);

    /* Free texture unit list */
    __glFreeImageUserList(gc, &tex->texUnitBoundList);

    /* Free image list */
    __glFreeImageUserList(gc, &tex->imageList);

    /* Delete the texture object structure */
    (*gc->imports.free)(gc, tex);

    return GL_TRUE;
}

GLvoid GL_APIENTRY __gles_GenTextures(__GLcontext *gc, GLsizei n, GLuint *textures)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (NULL == textures)
    {
        __GL_EXIT();
    }

    GL_ASSERT(NULL != gc->texture.shared);

    start = __glGenerateNames(gc, gc->texture.shared, n);

    for (i = 0; i < n; i++)
    {
        textures[i] = start + i;
    }

    if (gc->texture.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->texture.shared, (start + n));
    }

OnError:
OnExit:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsTexture(__GLcontext *gc, GLuint texture)
{
    return (NULL != __glGetObject(gc, gc->texture.shared, texture));
}

__GL_INLINE __GLsamplerObject* __glGetSamplerObject(__GLcontext *gc, GLuint name)
{
    __GLsamplerObject *samplerObj;
    GL_ASSERT(gc->sampler.shared);

    /* Not a valid name of returned by previous call to glGenSamplers */
    if (!__glIsNameDefined(gc, gc->sampler.shared, name))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, NULL);
    }

    samplerObj = (__GLsamplerObject *)__glGetObject(gc, gc->sampler.shared, name);
    if (!samplerObj)
    {
        samplerObj = (__GLsamplerObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLsamplerObject));
        __glAddObject(gc, gc->sampler.shared, name, samplerObj);
        samplerObj->name = name;
        samplerObj->bindCount = 0;
        samplerObj->flags = 0;
        samplerObj->params.sWrapMode = GL_REPEAT;
        samplerObj->params.tWrapMode = GL_REPEAT;
        samplerObj->params.rWrapMode = GL_REPEAT;
        samplerObj->params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
        samplerObj->params.magFilter = GL_LINEAR;
        samplerObj->params.minLod = -1000.0;
        samplerObj->params.maxLod = 1000.0;
        samplerObj->params.compareMode = GL_NONE;
        samplerObj->params.compareFunc = GL_LEQUAL;
        samplerObj->params.maxAnistropy = 1.0f;
        samplerObj->params.sRGB = GL_DECODE_EXT;
    }

    return samplerObj;
}

GLvoid __glBindSampler(__GLcontext *gc, GLuint unit, GLuint sampler)
{
    __GLsamplerObject *samplerObj = NULL;
    __GLsamplerObject *boundSamplerObj = gc->texture.units[unit].boundSampler;

    if (sampler == 0)
    {
        samplerObj = NULL;
    }
    else
    {
        samplerObj = __glGetSamplerObject(gc, sampler);
        if (!samplerObj)
        {
            return;
        }
    }

    /* Bind the same object */
    if (samplerObj == boundSamplerObj)
    {
        return;
    }

    gc->texture.units[unit].boundSampler = samplerObj;

    /* Delete the object if there is nothing bound to the object */
    if (boundSamplerObj)
    {
        if ((--boundSamplerObj->bindCount) == 0 && (boundSamplerObj->flags & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteSamplerObj(gc, boundSamplerObj);
        }
    }

    /* bindCount includes both single context and shared context bindings.
    */
    if (samplerObj)
    {
        /* Add current active texture unit index to current bound sampler object's texUnitBoundList */
        __glAddImageUser(gc, &samplerObj->texUnitBoundList, (GLvoid*)(GLintptr)unit);

        samplerObj->bindCount++;
    }

    /* Set all texParamter and texture image dirty bits */
    __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_SAMPLERPARAMETER_BITS);
}

GLboolean __glDeleteSamplerObj(__GLcontext *gc, __GLsamplerObject *samplerObj)
{
    __GLimageUser *texUnitBoundList = samplerObj->texUnitBoundList;

    /* __GL_OBJECT_IS_DELETED is cleared here because we do not want this object
    ** deleted in the following Bind function, otherwise there will be recursion:
    ** __glDeleteSamplerObj-->__glBindObject-->__glDeleteSamplerObj, and the object
    ** will be deleted twice.
    */
    samplerObj->flags &= ~__GL_OBJECT_IS_DELETED;

    while (texUnitBoundList)
    {
        GLuint unit = __GL_PTR2UINT(texUnitBoundList->imageUser);

        /* If the sampler object is still bound to the unit of current context */
        if (samplerObj == gc->texture.units[unit].boundSampler)
        {
            __glBindSampler(gc, unit, 0);
        }

        texUnitBoundList = texUnitBoundList->next;
    }

    if (samplerObj->bindCount)
    {
        samplerObj->flags |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    if (samplerObj->label)
    {
        gc->imports.free(gc, samplerObj->label);
    }

    /* Free texture unit list */
    __glFreeImageUserList(gc, &samplerObj->texUnitBoundList);

    (*gc->imports.free)(gc, samplerObj);

    return GL_TRUE;
}

__GL_INLINE GLvoid __glSamplerParameterfv(__GLcontext *gc, __GLsamplerObject *samplerObj, GLenum pname,
                                          const GLfloat *pv)
{
    GLbitfield dirty;
    __GLimageUser *texUnitBoundList;
    GLint param = __glFloat2NearestInt(pv[0]);

    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
        switch (param)
        {
        case GL_CLAMP_TO_EDGE:
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            samplerObj->params.sWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_S_BIT;
            break;
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                samplerObj->params.sWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_S_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_WRAP_T:
        switch (param)
        {
        case GL_CLAMP_TO_EDGE:
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            samplerObj->params.tWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_T_BIT;
            break;
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                samplerObj->params.tWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_T_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_WRAP_R:
        switch (param)
        {
        case GL_CLAMP_TO_EDGE:
        case GL_REPEAT:
        case GL_MIRRORED_REPEAT:
            samplerObj->params.rWrapMode = (GLenum)param;
            dirty = __GL_TEXPARAM_WRAP_R_BIT;
            break;
        case GL_CLAMP_TO_BORDER_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
            {
                samplerObj->params.rWrapMode = (GLenum)param;
                dirty = __GL_TEXPARAM_WRAP_R_BIT;
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MIN_FILTER:
        switch (param)
        {
        case GL_NEAREST:
        case GL_LINEAR:
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
            samplerObj->params.minFilter = (GLenum)param;
            dirty = __GL_TEXPARAM_MIN_FILTER_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MAG_FILTER:
        switch (param)
        {
        case GL_NEAREST:
        case GL_LINEAR:
            samplerObj->params.magFilter = (GLenum)param;
            dirty = __GL_TEXPARAM_MAG_FILTER_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MIN_LOD:
        samplerObj->params.minLod = pv[0];
        dirty = __GL_TEXPARAM_MIN_LOD_BIT;
        break;

    case GL_TEXTURE_MAX_LOD:
        samplerObj->params.maxLod = pv[0];
        dirty = __GL_TEXPARAM_MAX_LOD_BIT;
        break;

    case GL_TEXTURE_COMPARE_MODE:
        switch (param)
        {
        case GL_NONE:
        case GL_COMPARE_REF_TO_TEXTURE:
            samplerObj->params.compareMode = (GLenum)param;
            dirty = __GL_TEXPARAM_COMPARE_MODE_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_COMPARE_FUNC:
        switch (param)
        {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            samplerObj->params.compareFunc = (GLenum)param;
            dirty = __GL_TEXPARAM_COMPARE_FUNC_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        if (pv[0] >= 1.0f)
        {
            samplerObj->params.maxAnistropy = pv[0];
            dirty = __GL_TEXPARAM_MAX_ANISOTROPY_BIT;
        }
        else
        {
             __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        break;

    case GL_TEXTURE_SRGB_DECODE_EXT:
        switch (param)
        {
        case GL_DECODE_EXT:
        case GL_SKIP_DECODE_EXT:
            samplerObj->params.sRGB = (GLenum)param;
            dirty = __GL_TEXPARAM_SRGB_BIT;
            break;
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }
        break;

    case GL_TEXTURE_BORDER_COLOR_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
        {
            __GL_MEMCOPY(samplerObj->params.borderColor.fv, pv, 4 * sizeof(GLfloat));
            dirty = __GL_TEXPARAM_BORDER_COLOR_BIT;
            break;
        }
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    texUnitBoundList = samplerObj->texUnitBoundList;
    while (texUnitBoundList)
    {
        GLuint unit = __GL_PTR2UINT(texUnitBoundList->imageUser);

        /* Dirty the texture unit the sampler object is currently bound to */
        if (samplerObj == gc->texture.units[unit].boundSampler)
        {
            __GL_SET_TEX_UNIT_BIT(gc, unit, dirty);
        }

        texUnitBoundList = texUnitBoundList->next;
    }
}

__GL_INLINE GLvoid __glGetSamplerParameterfv(__GLcontext *gc, __GLsamplerObject *samplerObj, GLenum pname,
                                             GLfloat *params)
{
    switch (pname)
    {
      case GL_TEXTURE_WRAP_S:
          params[0] = (GLfloat)samplerObj->params.sWrapMode;
          break;
      case GL_TEXTURE_WRAP_T:
          params[0] = (GLfloat)samplerObj->params.tWrapMode;
          break;
      case GL_TEXTURE_WRAP_R:
          params[0] = (GLfloat)samplerObj->params.rWrapMode;
          break;
      case GL_TEXTURE_MIN_FILTER:
          params[0] = (GLfloat)samplerObj->params.minFilter;
          break;
      case GL_TEXTURE_MAG_FILTER:
          params[0] = (GLfloat)samplerObj->params.magFilter;
          break;
      case GL_TEXTURE_MIN_LOD:
          params[0] = (GLfloat)samplerObj->params.minLod;
          break;
      case GL_TEXTURE_MAX_LOD:
          params[0] = (GLfloat)samplerObj->params.maxLod;
          break;
      case GL_TEXTURE_COMPARE_MODE:
          params[0] = (GLfloat)samplerObj->params.compareMode;
          break;
      case GL_TEXTURE_COMPARE_FUNC:
          params[0] = (GLfloat)samplerObj->params.compareFunc;
          break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
          params[0] = (GLfloat)samplerObj->params.maxAnistropy;
          break;
      case GL_TEXTURE_SRGB_DECODE_EXT:
          params[0] = (GLfloat)samplerObj->params.sRGB;
          break;
      case GL_TEXTURE_BORDER_COLOR_EXT:
          if (__glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled)
          {
              __GL_MEMCOPY(params, samplerObj->params.borderColor.fv, 4 * sizeof(GLfloat));
              break;
          }
      default:
          __GL_ERROR_RET(GL_INVALID_ENUM);
    }
}

GLvoid __glInitSamplerState(__GLcontext *gc)
{
    /* Sampler objects can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->sampler.shared);
        gc->sampler.shared = gc->shareCtx->sampler.shared;
        gcoOS_LockPLS();
        gc->sampler.shared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->sampler.shared->lock)
        {
            gc->sampler.shared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
            (*gc->imports.createMutex)(gc->sampler.shared->lock);
        }
        gcoOS_UnLockPLS();

    }
    else
    {
        GL_ASSERT(NULL == gc->sampler.shared);

        gc->sampler.shared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for Sampler object */
        gc->sampler.shared->maxLinearTableSize = __GL_MAX_SAMPLEROBJ_LINEAR_TABLE_SIZE;
        gc->sampler.shared->linearTableSize = __GL_DEFAULT_SAMPLEROBJ_LINEAR_TABLE_SIZE;
        gc->sampler.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->sampler.shared->linearTableSize * sizeof(GLvoid*));

        gc->sampler.shared->hashSize = __GL_SAMPLER_HASH_TABLE_SIZE;
        gc->sampler.shared->hashMask = __GL_SAMPLER_HASH_TABLE_SIZE - 1;
        gc->sampler.shared->refcount = 1;
        gc->sampler.shared->deleteObject = (__GLdeleteObjectFunc)__glDeleteSamplerObj;
        gc->sampler.shared->immediateInvalid = GL_TRUE;
    }
}

GLvoid __glFreeSamplerState(__GLcontext *gc)
{
    GLuint unit;

    /* unbound sampler object from all units. */
    for (unit = 0; unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits; ++unit)
    {
        __glBindSampler(gc, unit, 0);
    }

    /* Free shared Sampler object table */
    __glFreeSharedObjectState(gc, gc->sampler.shared);
}

GLvoid GL_APIENTRY __gles_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers)
{
    GLint start, i;

    __GL_HEADER();

    if (count < 0)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    if (NULL == samplers)
    {
        __GL_EXIT();
    }

    GL_ASSERT(NULL != gc->sampler.shared);

    start = __glGenerateNames(gc, gc->sampler.shared, count);

    for (i = 0; i < count; i++)
    {
        samplers[i] = start + i;
    }

    if (gc->sampler.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->sampler.shared, (start + count));
    }

OnExit:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers)
{
    GLint i;

    __GL_HEADER();

    if (count < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < count; i++)
    {
        if (samplers[i])
        {
            __glDeleteObject(gc, gc->sampler.shared, samplers[i]);
        }
    }

OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsSampler(__GLcontext *gc, GLuint sampler)
{
    GLboolean ret = GL_FALSE;

    __GL_HEADER();

    if (__glIsNameDefined(gc, gc->sampler.shared, sampler))
    {
        __glGetSamplerObject(gc, sampler);
        ret = GL_TRUE;
    }

    __GL_FOOTER();

    return ret;
}

GLvoid GL_APIENTRY __gles_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler)
{
    __GL_HEADER();

    if (unit >= gc->constants.shaderCaps.maxCombinedTextureImageUnits)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glBindSampler(gc, unit, sampler);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param)
{
    GLfloat ftemp[4];
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    ftemp[0] = (GLfloat)param;

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, ftemp);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param)
{
    GLfloat ftemp[4];
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(ftemp, param, 4 * sizeof(GLfloat));
    }
    else
    {
        ftemp[0] = (GLfloat)param[0];
    }

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, ftemp);
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param)
{
    GLfloat ftemp[4];

    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    ftemp[0] = param;

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, ftemp);
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, param);
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params)
{
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);
    GLfloat ftemp[4] = {0.f};

    __GL_HEADER();

    if ((samplerObj != gcvNULL))
    {
        __glGetSamplerParameterfv(gc, samplerObj, pname, ftemp);

        if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
        {
            __GL_MEMCOPY(params, ftemp, 4 * sizeof(GLfloat));
        }
        else
        {
            if (ftemp[0] < 0)
            {
                params[0] = (GLint)(ftemp[0] - 0.5f);
            }
            else
            {
                params[0] = (GLint)(ftemp[0] + 0.5f);
            }
        }
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params)
{
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if ((samplerObj != gcvNULL))
    {
        __glGetSamplerParameterfv(gc, samplerObj, pname, params);
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D:
        targetIdx = __GL_TEXTURE_2D_INDEX;
        break;
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIdx = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(tmpf, params, 4 * sizeof(GLfloat));
    }
    else
    {
        tmpf[0] = (GLfloat)params[0];
    }

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, tmpf);
        }
        imageUserList = imageUserList->next;
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D:
        targetIdx = __GL_TEXTURE_2D_INDEX;
        break;
    case GL_TEXTURE_2D_ARRAY:
        targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;
    case GL_TEXTURE_3D:
        targetIdx = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        targetIdx = __GL_TEXTURE_EXTERNAL_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        targetIdx = __GL_TEXTURE_2D_MS_INDEX;
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        targetIdx = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            targetIdx = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(tmpf, params, 4 * sizeof(GLfloat));
    }
    else
    {
        tmpf[0] = (GLfloat)params[0];
    }

    /* Update the tex parameter in the bind tex object. The texture parameter dirty bits
    ** of ALL the texture units that bound to the same text object need to be set.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = __GL_PTR2UINT(imageUserList->imageUser);
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, tmpf);
        }
        imageUserList = imageUserList->next;
    }

OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param)
{
    GLfloat ftemp[4];
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(ftemp, param, 4 * sizeof(GLfloat));
    }
    else
    {
        ftemp[0] = (GLfloat)param[0];
    }

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, ftemp);
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param)
{
    GLfloat ftemp[4];
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);

    __GL_HEADER();

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(ftemp, param, 4 * sizeof(GLfloat));
    }
    else
    {
        ftemp[0] = (GLfloat)param[0];
    }

    if ((samplerObj != gcvNULL))
    {
        __glSamplerParameterfv(gc, samplerObj, pname, ftemp);
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    GLfloat tmpf[4] = {0.0f};

    __GL_HEADER();

    __glGetTexParameterfv(gc, target, pname, tmpf);

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
        __GL_MEMCOPY(params, tmpf, 4 * sizeof(GLfloat));
    }
    else
    {
        if (tmpf[0] < 0)
        {
            params[0] = (GLint)(tmpf[0] - 0.5f);
        }
        else
        {
            params[0] = (GLint)(tmpf[0] + 0.5f);
        }
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params)
{
    GLfloat tmpf[4] = {0.0f};

    __GL_HEADER();

    __glGetTexParameterfv(gc, target, pname, tmpf);

    if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
    {
       __GL_MEMCOPY(params, tmpf, 4 * sizeof(GLfloat));
    }
    else
    {
        if (tmpf[0] < 0)
        {
            params[0] = (GLint)(tmpf[0] - 0.5f);
        }
        else
        {
            params[0] = (GLint)(tmpf[0] + 0.5f);
        }
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params)
{
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);
    GLfloat tmpf[4] = {0.0f};

    __GL_HEADER();

    if ((samplerObj != gcvNULL))
    {
        __glGetSamplerParameterfv(gc, samplerObj, pname, tmpf);
        if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
        {
            __GL_MEMCOPY(params, tmpf, 4 * sizeof(GLfloat));
        }
        else
        {
            if (tmpf[0] < 0)
            {
                params[0] = (GLint)(tmpf[0] - 0.5f);
            }
            else
            {
                params[0] = (GLint)(tmpf[0] + 0.5f);
            }
        }
    }

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetSamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params)
{
    __GLsamplerObject *samplerObj = __glGetSamplerObject(gc, sampler);
    GLfloat tmpf[4] = {0.0f};

    __GL_HEADER();

    if ((samplerObj != gcvNULL))
    {
        __glGetSamplerParameterfv(gc, samplerObj, pname, tmpf);
        if (pname == GL_TEXTURE_BORDER_COLOR_EXT)
        {
            __GL_MEMCOPY(params, tmpf, 4 * sizeof(GLfloat));
        }
        else
        {
            if (tmpf[0] < 0)
            {
                params[0] = (GLint)(tmpf[0] - 0.5f);
            }
            else
            {
                params[0] = (GLint)(tmpf[0] + 0.5f);
            }
        }
    }

    __GL_FOOTER();
}

#ifdef OPENGL40

__GL_INLINE GLvoid
__glTexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *pv)
{
    GLuint ui = gc->state.texture.activeTexIndex;
    __GLtextureEnvState *tes;
    GLenum e;
    GLenum mode;

    /* Error check the parameters first.
    */
    switch (target) {
      case GL_POINT_SPRITE:
          if (pname != GL_COORD_REPLACE) {
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_FILTER_CONTROL:
          if (pname != GL_TEXTURE_LOD_BIAS) {
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_ENV:

          switch (pname) {
          case GL_TEXTURE_ENV_MODE:
              switch (e = (GLenum)pv[0]) {
                  case GL_REPLACE:
                  case GL_MODULATE:
                  case GL_DECAL:
                  case GL_BLEND:
                  case GL_ADD:
                  case GL_COMBINE:
                  case __GL_STIPPLE:
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_TEXTURE_ENV_COLOR:
              break;

          case GL_COMBINE_RGB:
              switch (e = (GLenum)pv[0]) {
                  case GL_REPLACE:
                  case GL_MODULATE:
                  case GL_ADD:
                  case GL_ADD_SIGNED:
                  case GL_INTERPOLATE:
                  case GL_SUBTRACT:
                  case GL_DOT3_RGB:
                  case GL_DOT3_RGBA:

#if GL_EXT_texture_env_dot3
                  case GL_DOT3_RGB_EXT:
                  case GL_DOT3_RGBA_EXT:
#endif

#if GL_ATI_texture_env_combine3
                  case GL_MODULATE_ADD_ATI:
                  case GL_MODULATE_SIGNED_ADD_ATI:
                  case GL_MODULATE_SUBTRACT_ATI:
#endif
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_COMBINE_ALPHA:
              switch (e = (GLenum)pv[0]) {
                  case GL_REPLACE:
                  case GL_MODULATE:
                  case GL_ADD:
                  case GL_ADD_SIGNED:
                  case GL_INTERPOLATE:
                  case GL_SUBTRACT:
#if GL_ATI_texture_env_combine3
                  case GL_MODULATE_ADD_ATI:
                  case GL_MODULATE_SIGNED_ADD_ATI:
                  case GL_MODULATE_SUBTRACT_ATI:
#endif
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_SOURCE0_RGB:
          case GL_SOURCE1_RGB:
          case GL_SOURCE2_RGB:
          case GL_SOURCE0_ALPHA:
          case GL_SOURCE1_ALPHA:
          case GL_SOURCE2_ALPHA:
              switch (e = (GLenum)pv[0]) {
                  case GL_CONSTANT:
                  case GL_PRIMARY_COLOR:
                  case GL_PREVIOUS:
                  case GL_TEXTURE:
                  case GL_TEXTURE0:
                  case GL_TEXTURE1:
                  case GL_TEXTURE2:
                  case GL_TEXTURE3:
                  case GL_TEXTURE4:
                  case GL_TEXTURE5:
                  case GL_TEXTURE6:
                  case GL_TEXTURE7:
#if GL_ATI_texture_env_combine3
                  case GL_ZERO:
                  case GL_ONE:
#endif
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_OPERAND0_RGB:
          case GL_OPERAND1_RGB:
          case GL_OPERAND2_RGB:
              switch (e = (GLenum)pv[0]) {
                  case GL_SRC_COLOR:
                  case GL_ONE_MINUS_SRC_COLOR:
                  case GL_SRC_ALPHA:
                  case GL_ONE_MINUS_SRC_ALPHA:
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_OPERAND0_ALPHA:
          case GL_OPERAND1_ALPHA:
          case GL_OPERAND2_ALPHA:
              switch (e = (GLenum)pv[0]) {
                  case GL_SRC_ALPHA:
                  case GL_ONE_MINUS_SRC_ALPHA:
                      break;
                  default:
                      __glSetError(gc, GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_RGB_SCALE:
          case GL_ALPHA_SCALE:
              break;

          default:
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;

      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Set texEnv parameters.
    */
    switch (target) {
      case GL_POINT_SPRITE:
          gc->state.texture.texUnits[ui].env.coordReplace = (GLboolean)pv[0];
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_COORD_REPLACE_BIT);
          break;

      case GL_TEXTURE_FILTER_CONTROL:
          gc->state.texture.texUnits[ui].lodBias = (GLfloat) pv[0];
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEX_UNIT_LODBIAS_BIT);
          break;

      case GL_TEXTURE_ENV:
          tes = &gc->state.texture.texUnits[ui].env;
          switch (pname) {
          case GL_TEXTURE_ENV_MODE:
              mode = (GLenum)pv[0];
              tes->mode = mode;
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_MODE_BIT);
              break;

          case GL_TEXTURE_ENV_COLOR:
              __GL_MEMCOPY(&tes->color, pv, sizeof(GLfloat)*4);
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_COLOR_BIT);
              break;

          case GL_COMBINE_ALPHA:
              tes->function.alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_COMBINE_ALPHA_BIT);
              break;

          case GL_COMBINE_RGB:
              tes->function.rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_COMBINE_RGB_BIT);
              break;

          case GL_SOURCE0_RGB:
              tes->source[0].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE0_RGB_BIT);
              break;

          case GL_SOURCE1_RGB:
              tes->source[1].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE1_RGB_BIT);
              break;

          case GL_SOURCE2_RGB:
              tes->source[2].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE2_RGB_BIT);
              break;

          case GL_SOURCE0_ALPHA:
              tes->source[0].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE0_ALPHA_BIT);
              break;

          case GL_SOURCE1_ALPHA:
              tes->source[1].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE1_ALPHA_BIT);
              break;

          case GL_SOURCE2_ALPHA:
              tes->source[2].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_SOURCE2_ALPHA_BIT);
              break;

          case GL_OPERAND0_RGB:
              tes->operand[0].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND0_RGB_BIT);
              break;

          case GL_OPERAND1_RGB:
              tes->operand[1].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND1_RGB_BIT);
              break;

          case GL_OPERAND2_RGB:
              tes->operand[2].rgb = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND2_RGB_BIT);
              break;

          case GL_OPERAND0_ALPHA:
              tes->operand[0].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND0_ALPHA_BIT);
              break;

          case GL_OPERAND1_ALPHA:
              tes->operand[1].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND1_ALPHA_BIT);
              break;

          case GL_OPERAND2_ALPHA:
              tes->operand[2].alpha = (GLenum)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_OPERAND2_ALPHA_BIT);
              break;

          case GL_RGB_SCALE:
              tes->rgbScale = (GLfloat)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_RGB_SCALE_BIT);
              break;

          case GL_ALPHA_SCALE:
              tes->alphaScale = (GLfloat)pv[0];
              __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXENV_ALPHA_SCALE_BIT);
              break;
          }
          break;
    }
}

GLvoid GL_APIENTRY __glim_TexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __glTexEnvfv(gc, target, pname, (GLfloat *)pv);
}

GLvoid GL_APIENTRY __glim_TexEnvf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat f)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    if (pname != GL_TEXTURE_ENV_COLOR) {
        tmpf[0] = f;
        __glTexEnvfv(gc, target, pname, (GLfloat *)tmpf);
    }
    else {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
}

GLvoid GL_APIENTRY __glim_TexEnviv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (pname == GL_TEXTURE_ENV_COLOR) {
        tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
        tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
        tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
        tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
    }
    else {
        tmpf[0] = (GLfloat)pv[0];
    }
    __glTexEnvfv(gc, target, pname, (GLfloat *)tmpf);
}

GLvoid GL_APIENTRY __glim_TexEnvi(__GLcontext *gc, GLenum target, GLenum pname, GLint i)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    if (pname != GL_TEXTURE_ENV_COLOR) {
        tmpf[0] = (GLfloat)i;
        __glTexEnvfv(gc, target, pname, (GLfloat *)tmpf);
    }
    else {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
}

/*
* Specify modes the texture generation flavors
*/
__GL_INLINE GLvoid
__glTexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat *pv)
{
    GLuint ui = gc->state.texture.activeTexIndex;
    __GLtextureCoordState *tcs;
    __GLcoord equ;

    /* flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (coord) {
      case GL_S:
          tcs = &gc->state.texture.texUnits[ui].s;
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXGEN_S_BIT);
          __GL_INPUTMASK_CHANGED(gc);
          break;

      case GL_T:
          tcs = &gc->state.texture.texUnits[ui].t;
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXGEN_T_BIT);
          __GL_INPUTMASK_CHANGED(gc);
          break;

      case GL_R:
          tcs = &gc->state.texture.texUnits[ui].r;
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXGEN_R_BIT);
          __GL_INPUTMASK_CHANGED(gc);
          break;

      case GL_Q:
          tcs = &gc->state.texture.texUnits[ui].q;
          __GL_SET_TEX_UNIT_BIT(gc, ui, __GL_TEXGEN_Q_BIT);
          break;

      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          switch ((GLenum)pv[0]) {
            case GL_EYE_LINEAR:
            case GL_OBJECT_LINEAR:
              tcs->mode = (GLenum)pv[0];
              break;
            case GL_SPHERE_MAP:
              if ((coord == GL_R) || (coord == GL_Q)) {
                  __glSetError(gc, GL_INVALID_ENUM);
                  return;
              }
              tcs->mode = (GLenum)pv[0];
              break;
            case GL_REFLECTION_MAP:
            case GL_NORMAL_MAP:
              if (coord == GL_Q) {
                  __glSetError(gc, GL_INVALID_ENUM);
                  return;
              }
              tcs->mode = (GLenum)pv[0];
              break;
            default:
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;
      case GL_OBJECT_PLANE:
          tcs->objectPlaneEquation.f.x = pv[0];
          tcs->objectPlaneEquation.f.y = pv[1];
          tcs->objectPlaneEquation.f.z = pv[2];
          tcs->objectPlaneEquation.f.w = pv[3];
          break;
      case GL_EYE_PLANE:
          equ.f.x = pv[0];
          equ.f.y = pv[1];
          equ.f.z = pv[2];
          equ.f.w = pv[3];

          /* Transform plane equation into eye space.
          */
          __glTransformVector(gc, &tcs->eyePlaneEquation, &equ, gc->transform.modelView, GL_FALSE);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid GL_APIENTRY __glim_TexGendv(__GLcontext *gc, GLenum coord, GLenum pname, const GLdouble *pv)
{
    GLfloat tmpf[4]={0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          tmpf[0] = (GLfloat)pv[0];
          break;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
          tmpf[0] = (GLfloat)pv[0];
          tmpf[1] = (GLfloat)pv[1];
          tmpf[2] = (GLfloat)pv[2];
          tmpf[3] = (GLfloat)pv[3];
          break;
    }

    __glTexGenfv(gc, coord, pname, tmpf);
}

GLvoid GL_APIENTRY __glim_TexGend(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble d)
{
    GLfloat tmpf = (GLfloat)d;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, &tmpf);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid GL_APIENTRY __glim_TexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __glTexGenfv(gc, coord, pname, (GLfloat *)pv);
}

GLvoid GL_APIENTRY __glim_TexGenf(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat f)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, (GLfloat *)&f);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid GL_APIENTRY __glim_TexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          tmpf[0] = (GLfloat)pv[0];
          break;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
          tmpf[0] = (GLfloat)pv[0];
          tmpf[1] = (GLfloat)pv[1];
          tmpf[2] = (GLfloat)pv[2];
          tmpf[3] = (GLfloat)pv[3];
          break;
    }
    __glTexGenfv(gc, coord, pname, tmpf);
}

GLvoid GL_APIENTRY __glim_TexGeni(__GLcontext *gc, GLenum coord, GLenum pname, GLint i)
{
    GLfloat tmpf = (GLfloat)i;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, (GLfloat *)&tmpf);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}


__GL_INLINE GLvoid
__glGetTexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *v)
{
    GLuint ui = gc->state.texture.activeTexIndex;

    switch (target) {
      case GL_POINT_SPRITE:
          if (pname == GL_COORD_REPLACE) {
              v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.coordReplace;
          }
          else {
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_FILTER_CONTROL:
          if (pname == GL_TEXTURE_LOD_BIAS) {
              v[0] = gc->state.texture.texUnits[ui].lodBias;
          }
          else {
              __glSetError(gc, GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_ENV:
          switch (pname) {
              case GL_TEXTURE_ENV_MODE:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.mode;
                  break;

              case GL_TEXTURE_ENV_COLOR:
                  v[0] = (gc->state.texture.texUnits[ui].env.color.r);
                  v[1] = (gc->state.texture.texUnits[ui].env.color.g);
                  v[2] = (gc->state.texture.texUnits[ui].env.color.b);
                  v[3] = (gc->state.texture.texUnits[ui].env.color.a);
                  break;

              case GL_COMBINE_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.function.rgb;
                  break;

              case GL_COMBINE_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.function.alpha;
                  break;

              case GL_SOURCE0_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[0].rgb;
                  break;

              case GL_SOURCE1_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[1].rgb;
                  break;

              case GL_SOURCE2_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[2].rgb;
                  break;

              case GL_SOURCE0_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[0].alpha;
                  break;

              case GL_SOURCE1_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[1].alpha;
                  break;

              case GL_SOURCE2_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.source[2].alpha;
                  break;

              case GL_OPERAND0_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[0].rgb;
                  break;

              case GL_OPERAND1_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[1].rgb;
                  break;

              case GL_OPERAND2_RGB:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[2].rgb;
                  break;

              case GL_OPERAND0_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[0].alpha;
                  break;

              case GL_OPERAND1_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[1].alpha;
                  break;

              case GL_OPERAND2_ALPHA:
                  v[0] = (GLfloat)gc->state.texture.texUnits[ui].env.operand[2].alpha;
                  break;

              case GL_RGB_SCALE:
                  v[0] = (gc->state.texture.texUnits[ui].env.rgbScale);
                  break;

              case GL_ALPHA_SCALE:
                  v[0] = (gc->state.texture.texUnits[ui].env.alphaScale);
                  break;

              default:
                  __glSetError(gc, GL_INVALID_ENUM);
                  return;
          }
          break;

      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}


GLvoid GL_APIENTRY __glim_GetTexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexEnvfv(gc, target, pname, v);
}

GLvoid GL_APIENTRY __glim_GetTexEnviv(__GLcontext *gc, GLenum target, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexEnvfv(gc, target, pname, tmpf);
    v[0] = (GLint)(tmpf[0]);
    v[1] = (GLint)(tmpf[1]);
    v[2] = (GLint)(tmpf[2]);
    v[3] = (GLint)(tmpf[3]);
}

__GL_INLINE GLvoid
__glGetTexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat *v)
{
    GLuint ui = gc->state.texture.activeTexIndex;
    __GLtextureCoordState *tcs;

    switch (coord) {
      case GL_S:
          tcs = &gc->state.texture.texUnits[ui].s;
          break;
      case GL_T:
          tcs = &gc->state.texture.texUnits[ui].t;
          break;
      case GL_R:
          tcs = &gc->state.texture.texUnits[ui].r;
          break;
      case GL_Q:
          tcs = &gc->state.texture.texUnits[ui].q;
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          v[0] = (GLfloat)tcs->mode;
          break;
      case GL_OBJECT_PLANE:
          v[0] = tcs->objectPlaneEquation.f.x;
          v[1] = tcs->objectPlaneEquation.f.y;
          v[2] = tcs->objectPlaneEquation.f.z;
          v[3] = tcs->objectPlaneEquation.f.w;
          break;
      case GL_EYE_PLANE:
          v[0] = tcs->eyePlaneEquation.f.x;
          v[1] = tcs->eyePlaneEquation.f.y;
          v[2] = tcs->eyePlaneEquation.f.z;
          v[3] = tcs->eyePlaneEquation.f.w;
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid GL_APIENTRY __glim_GetTexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexGenfv(gc, coord, pname, v);
}

GLvoid GL_APIENTRY __glim_GetTexGendv(__GLcontext *gc,GLenum coord, GLenum pname, GLdouble v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexGenfv(gc, coord, pname, tmpf);

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          v[0] = tmpf[0];
          break;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
          v[0] = tmpf[0];
          v[1] = tmpf[1];
          v[2] = tmpf[2];
          v[3] = tmpf[3];
          break;
    }
}

GLvoid GL_APIENTRY __glim_GetTexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};

    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexGenfv(gc, coord, pname, tmpf);

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          v[0] = (GLint)tmpf[0];
          break;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
          v[0] = (GLint)tmpf[0];
          v[1] = (GLint)tmpf[1];
          v[2] = (GLint)tmpf[2];
          v[3] = (GLint)tmpf[3];
          break;
    }
}

/* ES already got one, keep it for later referecne */

GLvoid GL_APIENTRY __glim_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat v[])
{
    GLint ti = 0;
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexLevelParameteriv(gc, target, level, pname, &ti);
    v[0] = (GLfloat)ti;
}

GLvoid GL_APIENTRY __glim_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint v[])
{
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexLevelParameteriv(gc, target, level, pname, v);
}

/* ES already got one, keep it for later reference */

GLvoid GL_APIENTRY __glim_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexParameterfv(gc, target, pname, v);
}

GLvoid GL_APIENTRY __glim_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexParameterfv(gc, target, pname, tmpf);

    switch(pname) {
      case GL_TEXTURE_BORDER_COLOR:
          v[0] = __GL_FLOAT_TO_I(tmpf[0]);
          v[1] = __GL_FLOAT_TO_I(tmpf[1]);
          v[2] = __GL_FLOAT_TO_I(tmpf[2]);
          v[3] = __GL_FLOAT_TO_I(tmpf[3]);
          break;
      default:
          v[0] = (GLint)(tmpf[0]);
          break;
    }
}

GLvoid GL_APIENTRY __glim_GetTexParameterIivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexParameterfv(gc, target, pname, tmpf);

    params[0] = (GLint)(tmpf[0]);
}

GLvoid GL_APIENTRY __glim_GetTexParameterIuivEXT(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN(gc);
    __glGetTexParameterfv(gc, target, pname, tmpf);

    params[0] = (GLuint)(tmpf[0]);

}
GLvoid GL_APIENTRY __glim_ClientActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if ((texture > (GL_TEXTURE0 + __GL_MAX_TEXTURE_COORDS - 1)) ||
        (texture < GL_TEXTURE0))
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    texture = texture - GL_TEXTURE0;

    gc->vertexArray.boundVAO->vertexArray.clientActiveUnit = texture;
}

GLvoid GL_APIENTRY __glim_PrioritizeTextures(__GLcontext *gc, GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
    GLint i;
    __GLtextureObject *tex;
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    for (i = 0; i < n; i++) {
        /* silently ignore default texture */
        if (0 == textures[i]) continue;

        tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, textures[i]);

        /* silently ignore non-texture */
        if (NULL == tex) continue;

        tex->params.priority = __glClampf(priorities[i], __glZero, __glOne);
        tex->seqNumber++;

        /* Update the tex parameter states in the active unit and the bind tex object.
        ** If there are other texture units bound to the same text object, we need to update
        ** the texture parameters(here is texture priority) in
        ** gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
        */
        imageUserList = tex->texUnitBoundList;
        while (imageUserList)
        {
            GLuint unitIdx = (GLuint)__GL_TO_UINT32(imageUserList->imageUser);

            /* redundancy check, so texture unit's states can be updated correctly when
            ** a texture object is shared by two or more contexts.
            */
            if (tex == gc->texture.units[unitIdx].boundTextures[tex->targetIndex])
            {
                /* Make sure the priority is set into the texture unit that the texture object is bound to. */
                gc->state.texture.texUnits[unitIdx].texObj[tex->targetIndex].params.priority = tex->params.priority;
                __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_PRIORITY_BIT);
            }
            imageUserList = imageUserList->next;
        }
    }
}

GLboolean GL_APIENTRY __glim_AreTexturesResident(__GLcontext *gc, GLsizei n, const GLuint *textures, GLboolean *residences)
{
    GLint i;
    __GLtextureObject *tex;
    GLboolean allResident = GL_TRUE;

    __GL_SETUP_NOT_IN_BEGIN_RET(gc, 0);

    for (i = 0; i < n; i++) {
        /* Can't query a default texture. */
        if (0 == textures[i]) {
            __glSetError(gc, GL_INVALID_VALUE);
            return GL_FALSE;
        }

        tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, textures[i]);

        /*
        ** Ensure that all of the names have corresponding textures.
        */
        if (NULL == tex) {
            __glSetError(gc, GL_INVALID_VALUE);
            return GL_FALSE;
        }
/* Currently no resident support */
            allResident = GL_FALSE;
            residences[i] = GL_FALSE;

    }

    return allResident;
}
#endif

