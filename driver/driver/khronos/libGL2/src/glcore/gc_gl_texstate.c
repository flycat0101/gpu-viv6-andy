/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"
#include "gc_gl_names_inline.c"
#include "gc_gl_debug.h"
#include "dri/viv_lock.h"

GLboolean __glDeleteTextureObject(__GLcontext *gc, GLvoid *obj);
extern GLvoid __glReleaseTexImageImplicit(__GLcontext *gc,GLvoid *hPbuffer,GLenum iBuffer,__GLtextureObject *tex);
extern GLvoid __glFrameBufferTexture(__GLcontext *gc, __GLframebufferObject *framebufferObj, GLint attachIndex, __GLtextureObject *texObj, GLint level, GLint face, GLint zoffset, GLboolean layered);
extern GLboolean __glIsIntegerInternalFormat(GLenum internalFormat);

extern const __GLdeviceFormatInfo __glDevfmtInfo[];

GLvoid __glFreeDefaultTextureObj(__GLcontext *gc, __GLtextureObject *tex)
{
    GLuint targetIndex = tex->targetIndex;
    GLuint i, faces;

    /* Notify Dp that this texture object is deleted.
    */
    if(tex->privateData)
        (*gc->dp.deleteTexture)(gc, tex);

#if SWP_PIPELINE_ENABLED
    if(tex->swpPrivateData)
        (*gc->swp.deleteTexture)(gc, tex);
#endif

    /* Delete the texture object's texture image */
    switch(targetIndex)
    {
    case __GL_TEXTURE_CUBEMAP_INDEX:
        faces = 6;
        break;

    case __GL_TEXTURE_1D_ARRAY_INDEX:
    case __GL_TEXTURE_2D_ARRAY_INDEX:
        faces = gc->constants.maxTextureArraySize;
        /* Set levels to 1 for texture array because texture cache is only allocated once for same level */
        /* in different elements of array */
        /* Same level in different elements of array shares one contiguous memory */
        break;

    case __GL_TEXTURE_BUFFER_INDEX:
        if(tex->bufferObj)
            __glRemoveImageUser(gc, &(tex->bufferObj->bufferObjData->bufferObjUserList), tex);
        faces = 1;
        break;

    default:
        faces = 1;
        break;

    }

    for( i = 0; i < faces; i ++)
    {
        (*gc->imports.free)(gc, tex->faceMipmap[i]);
    }

    /* Free texture unit list */
    __glFreeImageUserList(gc, &tex->texUnitBoundList);

    (*gc->imports.free)(gc, tex->faceMipmap);
}

GLvoid __glInitTexObjTemplate(__GLcontext *gc)
{
    __GLtextureObject *tex = &gc->texture.texObjTemplate;

    tex->bindCount = 0;
    tex->seqNumber = 1;

    tex->params.sWrapMode = GL_REPEAT;
    tex->params.tWrapMode = GL_REPEAT;
    tex->params.rWrapMode = GL_REPEAT;
    tex->params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    tex->params.magFilter = GL_LINEAR;

    tex->params.priority = 1.0;

    /* Initialize border color */
    tex->params.borderColor.r = __glZero;
    tex->params.borderColor.g = __glZero;
    tex->params.borderColor.b = __glZero;
    tex->params.borderColor.a = __glZero;

    /* Initialize LOD */
    tex->params.minLod = -1000.0;
    tex->params.maxLod = 1000.0;
    tex->params.baseLevel = 0;
    tex->params.maxLevel = 1000;

    /* Initialize anisotropic */
    tex->params.anisotropicLimit = 1;

    tex->params.generateMipmap = GL_FALSE;
    tex->params.depthTexMode = GL_LUMINANCE;
    tex->params.compareMode = GL_NONE;
    tex->params.compareFunc = GL_LEQUAL;
    tex->params.compareFailValue = 0.0;

    tex->faceMipmap = NULL;

    tex->imageUpToDate = 0;

    tex->flag = 0;

    /* Initialize the privateData pointer */
    tex->privateData = NULL;

    tex->maxLevelUsed = 0;

    tex->forceBaseLeve = GL_FALSE;

    /* init the render texture fields */
    tex->pBufferNumLevels = 0;
    tex->colorBuffer = __GL_NULL_COLORBUFFER;
    tex->hPbuffer = NULL;
    tex->arrays = 0;

    tex->unpackBuffer = 0;
    tex->offsetInPBO  = NULL;
    tex->bufferObj = NULL;
}


GLvoid __glInitTextureObject(__GLcontext *gc, __GLtextureObject *tex, GLuint id, GLuint targetIndex)
{

    GLuint i,j;
    GLuint maxFaces, maxLevels;

    __GL_MEMCOPY(tex, &gc->texture.texObjTemplate, sizeof(__GLtextureObject));

    tex->name = id;
    tex->targetIndex = targetIndex;

    if (targetIndex == __GL_TEXTURE_RECTANGLE_INDEX)
    {
        tex->params.minFilter = GL_LINEAR;
        tex->params.sWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.tWrapMode = GL_CLAMP_TO_EDGE;
        tex->params.rWrapMode = GL_CLAMP_TO_EDGE;
    }

    maxLevels = gc->constants.maxNumTextureLevels;
    switch(targetIndex)
    {
        case __GL_TEXTURE_CUBEMAP_INDEX:
            maxFaces = 6;
            break;

        case __GL_TEXTURE_1D_ARRAY_INDEX:
        case __GL_TEXTURE_2D_ARRAY_INDEX:
            maxFaces = gc->constants.maxTextureArraySize;
            break;

        case __GL_TEXTURE_BUFFER_INDEX:
            maxFaces = 1;
            maxLevels = 1;
            break;

        default:
            maxFaces = 1;
            break;
    }

    tex->maxFaces  = maxFaces;
    tex->maxLevels = maxLevels;

    tex->faceMipmap =(__GLmipMapLevel **)(*gc->imports.malloc)(gc, maxFaces * sizeof(__GLmipMapLevel *) );
    for( i = 0; i < maxFaces; i ++ )
    {
        /*every face has an array of mipmaps*/
        tex->faceMipmap[i] = (__GLmipMapLevel *)(*gc->imports.calloc)(gc, 1, maxLevels * sizeof(__GLmipMapLevel) );
        for(j = 0; j < maxLevels; j++ )
        {
            tex->faceMipmap[i][j].requestedFormat = 1;
            tex->faceMipmap[i][j].seqNumber = 1;
            tex->faceMipmap[i][j].deviceFormat = NULL;
        }
    }
}

GLvoid __glFreeTextureState(__GLcontext *gc)
{
    __GLtextureObject *tex;
    GLuint j;
    GLuint maxbinding;

    /* Free shared texture object table */
    __glFreeSharedObjectState(gc, gc->texture.shared);

    if(__glExtension[INDEX_EXT_texture_array].bEnabled)
        maxbinding = __GL_MAX_TEXTURE_BINDINGS;
    else
        maxbinding = __GL_TEXTURE_RECTANGLE_INDEX + 1;

    for (j = 0; j < maxbinding; j++) {
        /* Free default texture object state */
        tex = (__GLtextureObject *)&gc->texture.defaultTextures[j];
        __glFreeDefaultTextureObj(gc, tex);

        /* Free proxy texture object state */
        tex = (__GLtextureObject *)&gc->texture.proxyTextures[j];
        __glFreeDefaultTextureObj(gc, tex);
    }
}

GLvoid __glInitTextureState(__GLcontext *gc)
{
    __GLtextureObject *tex, *proxyTex;
    __GLtextureEnvState *tes;
    GLint i, j, k;
    GLint maxbinding;

    gc->state.texture.activeTexIndex = 0;

    if (gc->texture.shared == NULL) {
        gc->texture.shared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        /* Initialize a linear lookup table for texture object */
        gc->texture.shared->maxLinearTableSize = __GL_MAX_TEXOBJ_LINEAR_TABLE_SIZE;
        gc->texture.shared->linearTableSize = __GL_DEFAULT_TEXOBJ_LINEAR_TABLE_SIZE;
        gc->texture.shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->texture.shared->linearTableSize * sizeof(GLvoid *) );

        gc->texture.shared->hashSize = __GL_TEXOBJ_HASH_TABLE_SIZE;
        gc->texture.shared->hashMask = __GL_TEXOBJ_HASH_TABLE_SIZE - 1;
        gc->texture.shared->refcount = 1;
        gc->texture.shared->deleteObject = __glDeleteTextureObject;
    }

    if(gc->constants.maxTextureArraySize)
        maxbinding = __GL_MAX_TEXTURE_BINDINGS;
    else
        maxbinding = __GL_TEXTURE_RECTANGLE_INDEX + 1;

    for (j = 0; j < maxbinding; j++) {
        /* Initialize default texture object state */
        tex = &gc->texture.defaultTextures[j];
        __glInitTextureObject(gc, tex, 0, j );

        for (k = 0; k < __GL_MAX_TEXTURE_UNITS; k++) {
            gc->texture.units[k].boundTextures[j] = tex;
            gc->state.texture.texUnits[k].texObj[j].params = tex->params;

            /* in default, default texture object is bound to all texture units, so
            ** add each texture unit to default texture object's texUnitBoundList.
            */
            __glAddImageUser(gc, &tex->texUnitBoundList, (GLvoid*)(GLuint_ptr)k, NULL);
        }

        /* Initialize proxy texture object state */
        proxyTex = &gc->texture.proxyTextures[j];
        __glInitTextureObject(gc, proxyTex, 0, j);
    }

    /* Initialize the current texture coords */
    for (i = 0; i < __GL_MAX_TEXTURE_COORDS; i++) {
        gc->state.current.texture[i].x = __glZero;
        gc->state.current.texture[i].y = __glZero;
        gc->state.current.texture[i].z = __glZero;
        gc->state.current.texture[i].w = __glOne;
    }

    /* Init rest of texture state for each texture unit */
    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++) {
        gc->state.texture.texUnits[i].s.mode = GL_EYE_LINEAR;
        gc->state.texture.texUnits[i].s.eyePlaneEquation.x = __glOne;
        gc->state.texture.texUnits[i].s.objectPlaneEquation.x = __glOne;
        gc->state.texture.texUnits[i].t.mode = GL_EYE_LINEAR;
        gc->state.texture.texUnits[i].t.eyePlaneEquation.y = __glOne;
        gc->state.texture.texUnits[i].t.objectPlaneEquation.y = __glOne;
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

    gc->texture.enabledMask = 0;
    gc->texture.currentEnableMask = 0;
    gc->texture.drawDepthStage = -1;
}

/*
** Used to share texture objects between two different contexts.
*/
GLvoid __glShareTextureObjects(__GLcontext *dst, __GLcontext *src)
{
    if (dst->texture.shared) {
        __glFreeSharedObjectState(dst, dst->texture.shared);
    }

    dst->texture.shared = src->texture.shared;
    dst->texture.shared->refcount++;
}

GLboolean __glIsTextureConsistent(__GLcontext *gc, __GLtextureObject *tex)
{
    __GLmipMapLevel *level;
    GLint width, height, depth, border;
    GLint maxLevel, base, face, i;
    GLenum requestedFormat;
    GLint arrays;

    /* If the texture object is already checked for consistency */
    if (tex->flag & __GL_TEXTURE_IS_CHECKED)
    {
        if (tex->flag & __GL_TEXTURE_IS_CONSISTENT)
            return GL_TRUE;
        else
            return GL_FALSE;
    }

    base = tex->params.baseLevel;
    border = tex->faceMipmap[0][base].border;
    width = tex->faceMipmap[0][base].width2;
    height = tex->faceMipmap[0][base].height2;
    depth = tex->faceMipmap[0][base].depth2;
    requestedFormat = tex->faceMipmap[0][base].requestedFormat;
    arrays = tex->faceMipmap[0][base].arrays;

    /* If each dimension of the level[base] array is positive. */
    if ((tex->faceMipmap[0][base].width == 0) ||
        (tex->faceMipmap[0][base].height == 0) ||
        (tex->faceMipmap[0][base].depth == 0))
    {
        tex->flag |= __GL_TEXTURE_IS_CHECKED;
        return GL_FALSE;
    }

    /* Check for CubeMap base level consistency for all six faces */
    if (tex->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX)
    {
        /* If Cubemap baselevel has square dimension */
        if (width != height) {
            tex->flag |= __GL_TEXTURE_IS_CHECKED;
            return GL_FALSE;
        }

        /* If the six baselevel images have identical dimension, internalformat */
        maxLevel = gc->constants.maxNumTextureLevels;
        for (face = 1; face < 6; face++)
        {
            level = &tex->faceMipmap[face][base];
            if (requestedFormat != level->requestedFormat ||
                border != level->border ||
                width != level->width2 ||
                height != level->height2)
            {
                tex->flag |= __GL_TEXTURE_IS_CHECKED;
                return GL_FALSE;
            }
        }
    }

    /* Buffer texture needn't check filter & mipmap */
    if (tex->targetIndex != __GL_TEXTURE_BUFFER_INDEX)
    {

        if (__glIsIntegerInternalFormat(requestedFormat))
        {
            if (tex->params.magFilter != GL_NEAREST)
            {
                tex->flag |= __GL_TEXTURE_IS_CHECKED;
                return GL_FALSE;
            }
            if ((tex->params.minFilter != GL_NEAREST) && (tex->params.minFilter != GL_NEAREST_MIPMAP_NEAREST))
            {
                tex->flag |= __GL_TEXTURE_IS_CHECKED;
                return GL_FALSE;
            }
        }


        /* If not-mipmapping, we are ok */
        switch (tex->params.minFilter)
        {
            case GL_NEAREST:
            case GL_LINEAR:
                DISABLE_TEXTURE_CONSISTENCY_CHECK(tex);
                return GL_TRUE;
            default:
                break;
        }

        /* If baseLevel is less or equeal to maxLevel */
        maxLevel = tex->maxLevelUsed;
        if (maxLevel < base)
        {
            tex->flag |= __GL_TEXTURE_IS_CHECKED;
            return GL_FALSE;
        }

        /* If the set of mipmap arrays have the same internalformat and border width.
        ** If the dimensions of the arrays follows the *2 sequence.
        */
        i = base;
        while (++i <= maxLevel)
        {
            if (width == 1 && height == 1 && depth == 1) break;
            width >>= 1;
            if (width == 0) width = 1;
            height >>= 1;
            if (height == 0) height = 1;
            depth >>= 1;
            if (depth == 0) depth = 1;

            if (tex->faceMipmap[0][i].border != border ||
                tex->faceMipmap[0][i].requestedFormat != requestedFormat ||
                tex->faceMipmap[0][i].width2 != width ||
                tex->faceMipmap[0][i].height2 != height ||
                tex->faceMipmap[0][i].depth2 != depth)
            {
                tex->flag |= __GL_TEXTURE_IS_CHECKED;
                return GL_FALSE;
            }
        }

        /* Check all mipmaps for remaining 5 faces of the cubemap
        */
        if (tex->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX)
        {
            maxLevel = tex->maxLevelUsed;
            for (face = 1; face < 6; face++)
            {
                border = tex->faceMipmap[face][base].border;
                width = tex->faceMipmap[face][base].width2;
                height = tex->faceMipmap[face][base].height2;
                requestedFormat = tex->faceMipmap[face][base].requestedFormat;

                i = base;
                while (++i <= maxLevel)
                {
                    level = &tex->faceMipmap[face][ i];
                    if (width == 1 && height == 1) break;
                    width >>= 1;
                    if (width == 0) width = 1;
                    height >>= 1;
                    if (height == 0) height = 1;

                    if (level->border != border ||
                        level->requestedFormat != requestedFormat ||
                        level->width2 != width ||
                        level->height2 != height)
                    {
                        tex->flag |= __GL_TEXTURE_IS_CHECKED;
                        return GL_FALSE;
                    }
                }
            }
        }
        else
        {
            i = base;
            while (++i <= maxLevel)
            {
                if(tex->faceMipmap[0][i].arrays != arrays)
                {
                    tex->flag |= __GL_TEXTURE_IS_CHECKED;
                    return GL_FALSE;
                }
            }
        }

    }
    DISABLE_TEXTURE_CONSISTENCY_CHECK(tex);
    return GL_TRUE;
}

/*
** Compute the tex->maxLevelUsed, which will be used in texture consistency check
*/
GLvoid __glSetTexMaxLevelUsed(__GLtextureObject *tex)
{
    GLint maxLevelUsed;
    GLint base = tex->params.baseLevel;

    if (tex->targetIndex == __GL_TEXTURE_BUFFER_INDEX)
    {
        tex->maxLevelUsed = 0;
    }
    else
    {
        switch (tex->params.minFilter)
        {
            case GL_NEAREST:
            case GL_LINEAR:
                /*without mipmap*/
                maxLevelUsed = base;
                break;
            default:
                /*with mipmap*/
                maxLevelUsed = tex->faceMipmap[0][base].widthLog2;
                if (maxLevelUsed < tex->faceMipmap[0][base].heightLog2)
                    maxLevelUsed = tex->faceMipmap[0][base].heightLog2;
                if (maxLevelUsed < tex->faceMipmap[0][base].depthLog2)
                    maxLevelUsed = tex->faceMipmap[0][base].depthLog2;
                maxLevelUsed += base;
                if (tex->params.maxLevel < maxLevelUsed)
                    maxLevelUsed = tex->params.maxLevel;
                break;
        }
        if (tex->forceBaseLeve)
        {
            tex->maxLevelUsed = tex->params.baseLevel;
        }
        else
        {
            tex->maxLevelUsed = maxLevelUsed;
        }
    }
}

/***********************************************************************************/


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
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_FILTER_CONTROL:
          if (pname != GL_TEXTURE_LOD_BIAS) {
              __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
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
                      __glSetError(GL_INVALID_ENUM);
                      return;
              }
              break;

          case GL_RGB_SCALE:
          case GL_ALPHA_SCALE:
              break;

          default:
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
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

GLvoid APIENTRY __glim_TexEnvfv(GLenum target, GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexEnvfv", DT_GLenum, target, DT_GLenum,pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    __glTexEnvfv(gc, target, pname, (GLfloat *)pv);
}

GLvoid APIENTRY __glim_TexEnvf(GLenum target, GLenum pname, GLfloat f)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexEnvf", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    if (pname != GL_TEXTURE_ENV_COLOR) {
        tmpf[0] = f;
        __glTexEnvfv(gc, target, pname, (GLfloat *)tmpf);
    }
    else {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_TexEnviv(GLenum target, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexEnviv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

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

GLvoid APIENTRY __glim_TexEnvi(GLenum target, GLenum pname, GLint i)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexEnvi", DT_GLenum, target, DT_GLenum, pname, DT_GLint, i, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    if (pname != GL_TEXTURE_ENV_COLOR) {
        tmpf[0] = (GLfloat)i;
        __glTexEnvfv(gc, target, pname, (GLfloat *)tmpf);
    }
    else {
        __glSetError(GL_INVALID_ENUM);
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
          __glSetError(GL_INVALID_ENUM);
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
                  __glSetError(GL_INVALID_ENUM);
                  return;
              }
              tcs->mode = (GLenum)pv[0];
              break;
            case GL_REFLECTION_MAP:
            case GL_NORMAL_MAP:
              if (coord == GL_Q) {
                  __glSetError(GL_INVALID_ENUM);
                  return;
              }
              tcs->mode = (GLenum)pv[0];
              break;
            default:
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          break;
      case GL_OBJECT_PLANE:
          tcs->objectPlaneEquation.x = pv[0];
          tcs->objectPlaneEquation.y = pv[1];
          tcs->objectPlaneEquation.z = pv[2];
          tcs->objectPlaneEquation.w = pv[3];
          break;
      case GL_EYE_PLANE:
          equ.x = pv[0];
          equ.y = pv[1];
          equ.z = pv[2];
          equ.w = pv[3];

          /* Transform plane equation into eye space.
          */
          __glTransformVector(gc, &tcs->eyePlaneEquation, &equ, gc->transform.modelView, GL_FALSE);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_TexGendv(GLenum coord, GLenum pname, const GLdouble *pv)
{
    GLfloat tmpf[4]={0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGendv", DT_GLenum, coord, DT_GLenum, pname, DT_GLdouble_ptr, pv, DT_GLnull);
#endif

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

GLvoid APIENTRY __glim_TexGend(GLenum coord, GLenum pname, GLdouble d)
{
    GLfloat tmpf = (GLfloat)d;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGend", DT_GLenum, coord, DT_GLenum, pname, DT_GLdouble, d, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, &tmpf);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_TexGenfv(GLenum coord, GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGenfv", DT_GLenum, coord, DT_GLenum, pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    __glTexGenfv(gc, coord, pname, (GLfloat *)pv);
}

GLvoid APIENTRY __glim_TexGenf(GLenum coord, GLenum pname, GLfloat f)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGenf", DT_GLenum, coord, DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, (GLfloat *)&f);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_TexGeniv(GLenum coord, GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGeniv", DT_GLenum, coord, DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

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

GLvoid APIENTRY __glim_TexGeni(GLenum coord, GLenum pname, GLint i)
{
    GLfloat tmpf = (GLfloat)i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexGeni", DT_GLenum, coord, DT_GLenum, pname, DT_GLint, i, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          __glTexGenfv(gc, coord, pname, (GLfloat *)&tmpf);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

__GL_INLINE GLvoid
__glTexParameterfv(__GLcontext *gc, GLuint unitIdx, GLuint targetIdx, GLenum pname, GLfloat *pv, GLboolean clampBorderColor)
{
    __GLtextureObject *tex = gc->texture.units[unitIdx].boundTextures[targetIdx];
    __GLtextureObjAttr *att = &gc->state.texture.texUnits[unitIdx].texObj[targetIdx];
    GLenum e;

    /* Save the pevious min filter, to reduce chance of trigering consistency check */
    GLenum preMinFilter = tex->params.minFilter;

    switch (pname) {

      case GL_TEXTURE_WRAP_S:
          switch (e = (GLenum)pv[0]) {
              case GL_CLAMP:
              case GL_CLAMP_TO_EDGE:
              case GL_CLAMP_TO_BORDER:
                  tex->params.sWrapMode = att->params.sWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_S_BIT);
                  break;
              case GL_REPEAT:
              case GL_MIRRORED_REPEAT:
                  if (targetIdx == __GL_TEXTURE_RECTANGLE_INDEX)
                  {
                      __glSetError(GL_INVALID_ENUM);
                      return;
                  }
                  tex->params.sWrapMode = att->params.sWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_S_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_WRAP_T:
          switch (e = (GLenum)pv[0]) {
              case GL_CLAMP:
              case GL_CLAMP_TO_EDGE:
              case GL_CLAMP_TO_BORDER:
                  tex->params.tWrapMode = att->params.tWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_T_BIT);
                  break;
              case GL_REPEAT:
              case GL_MIRRORED_REPEAT:
                  if (targetIdx == __GL_TEXTURE_RECTANGLE_INDEX)
                  {
                      __glSetError(GL_INVALID_ENUM);
                      return;
                  }
                  tex->params.tWrapMode = att->params.tWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_T_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_WRAP_R:
          switch (e = (GLenum)pv[0]) {
              case GL_CLAMP:
              case GL_CLAMP_TO_EDGE:
              case GL_CLAMP_TO_BORDER:
                  tex->params.rWrapMode = att->params.rWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_R_BIT);
                  break;
              case GL_REPEAT:
              case GL_MIRRORED_REPEAT:
                  if (targetIdx == __GL_TEXTURE_RECTANGLE_INDEX)
                  {
                      __glSetError(GL_INVALID_ENUM);
                      return;
                  }
                  tex->params.rWrapMode = att->params.rWrapMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_WRAP_R_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_MIN_FILTER:
          switch (e = (GLenum)pv[0]) {
              case GL_NEAREST:
              case GL_LINEAR:
                  tex->params.minFilter = att->params.minFilter = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MIN_FILTER_BIT);
                  break;
              case GL_NEAREST_MIPMAP_NEAREST:
              case GL_NEAREST_MIPMAP_LINEAR:
              case GL_LINEAR_MIPMAP_NEAREST:
              case GL_LINEAR_MIPMAP_LINEAR:
                  if (targetIdx == __GL_TEXTURE_RECTANGLE_INDEX)
                  {
                      __glSetError(GL_INVALID_ENUM);
                      return;
                  }
                  tex->params.minFilter = att->params.minFilter = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MIN_FILTER_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_MAG_FILTER:
          switch (e = (GLenum)pv[0]) {
              case GL_NEAREST:
              case GL_LINEAR:
                  tex->params.magFilter = att->params.magFilter = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MAG_FILTER_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_BORDER_COLOR:
          if (clampBorderColor)
              __glClampColorf(&tex->params.borderColor, pv);
          else
          {
              tex->params.borderColorIi.r = *((GLint*)&pv[0]);
              tex->params.borderColorIi.g = *((GLint*)&pv[1]);
              tex->params.borderColorIi.b = *((GLint*)&pv[2]);
              tex->params.borderColorIi.a = *((GLint*)&pv[3]);
          }

          att->params.borderColor = tex->params.borderColor;
          __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_BORDER_COLOR_BIT);
          break;

      case GL_TEXTURE_PRIORITY:
          tex->params.priority = __glClampf(pv[0], __glZero, __glOne);
          att->params.priority = tex->params.priority;
          __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_PRIORITY_BIT);
          break;

      case GL_TEXTURE_MIN_LOD:
          tex->params.minLod = att->params.minLod = (GLfloat) pv[0];
          __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MIN_LOD_BIT);
          break;

      case GL_TEXTURE_MAX_LOD:
          tex->params.maxLod = att->params.maxLod = (GLfloat) pv[0];
          __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MAX_LOD_BIT);
          break;

      case GL_TEXTURE_BASE_LEVEL:
          if (((GLint) pv[0]) >= 0) {
              e = (GLenum)pv[0];
              tex->params.baseLevel = att->params.baseLevel = e;
              __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_BASE_LEVEL_BIT);
          } else {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          break;

      case GL_TEXTURE_MAX_LEVEL:
          if (((GLint) pv[0]) >= 0) {
              e = (GLenum)pv[0];
              tex->params.maxLevel = att->params.maxLevel = e;
              __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MAX_LEVEL_BIT);
          } else {
              __glSetError(GL_INVALID_VALUE);
              return;
          }
          break;

      case GL_TEXTURE_LOD_BIAS:
          tex->params.lodBias = att->params.lodBias = (GLfloat)pv[0];
          __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_LOD_BIAS_BIT);
          break;

      case GL_DEPTH_TEXTURE_MODE:
          switch (e = (GLenum)pv[0]) {
              case GL_LUMINANCE:
              case GL_INTENSITY:
              case GL_ALPHA:
                  tex->params.depthTexMode = att->params.depthTexMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_DEPTH_TEX_MODE_BIT);
                  /* border color dirty bit will be set after evaluating border color dirty,
                  ** otherwise the dirty bit may be cleared when evaluate. */
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_COMPARE_MODE:
          switch (e = (GLenum)pv[0]) {
              case GL_NONE:
              case GL_COMPARE_R_TO_TEXTURE:
                  tex->params.compareMode = att->params.compareMode = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_COMPARE_MODE_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_TEXTURE_COMPARE_FUNC:
          switch (e = (GLenum)pv[0]) {
              case GL_LEQUAL:
              case GL_GEQUAL:
              case GL_LESS:
              case GL_GREATER:
              case GL_EQUAL:
              case GL_NOTEQUAL:
              case GL_ALWAYS:
              case GL_NEVER:
                  tex->params.compareFunc = att->params.compareFunc = e;
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_COMPARE_FUNC_BIT);
                  break;
              default:
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      case GL_GENERATE_MIPMAP:
          e = (GLboolean)pv[0];
          if (e == GL_TRUE || e == GL_FALSE) {
              tex->params.generateMipmap = att->params.generateMipmap = (GLboolean) e;
              __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_GENERATE_MIPMAP_BIT);
          }
          else {
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
          {
              if ( (GLint)pv[0] >= 0 || (GLuint)pv[0] <= gc->constants.maxTextureMaxAnisotropy) {
                  tex->params.anisotropicLimit = att->params.anisotropicLimit = pv[0];
                  __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_MAX_ANISOTROPY_BIT);
              }
              else {
                  __glSetError(GL_INVALID_ENUM);
                  return;
              }
          }
          break;

      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
          {
              GLfloat f = pv[0];
              if ( f< 0 )     f = 0;
              if ( f> 1.0f )  f = 1.0f;
              tex->params.compareFailValue = att->params.compareFailValue = f;
              __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT);
          }
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (pname) {
        case GL_TEXTURE_MIN_FILTER:
            {
                if(preMinFilter != att->params.minFilter)
                {
                    __glSetTexMaxLevelUsed(tex);

                    /* Enable texture consistency check */
                    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);
                }
            }
            break;
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
            __glSetTexMaxLevelUsed(tex);
            /* Enable texture consistency check */
            ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);
            break;
    }

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_TexParameterfv(GLenum target, GLenum pname, const GLfloat *pv)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, pv, DT_GLnull);
#endif

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)pv, GL_TRUE);
        }
        imageUserList = imageUserList->next;
    }
}

GLvoid APIENTRY __glim_TexParameterf(GLenum target, GLenum pname, GLfloat f)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameterf", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat, f, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
      case GL_TEXTURE_LOD_BIAS:
      case GL_DEPTH_TEXTURE_MODE:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
      case GL_GENERATE_MIPMAP:
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    tmpf[0] = f;

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)tmpf, GL_TRUE);
        }
        imageUserList = imageUserList->next;
    }
}

GLvoid APIENTRY __glim_TexParameteriv(GLenum target, GLenum pname, const GLint *pv)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, pv, DT_GLnull);
#endif

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (pname == GL_TEXTURE_BORDER_COLOR) {
        tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
        tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
        tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
        tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
    }
    else {
        tmpf[0] = (GLfloat)pv[0];
    }

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)tmpf, GL_TRUE);
        }
        imageUserList = imageUserList->next;
    }
}

GLvoid APIENTRY __glim_TexParameteri(GLenum target, GLenum pname, GLint val)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4];
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameteri", DT_GLenum, target, DT_GLenum, pname, DT_GLint, val, DT_GLnull);
#endif

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
      case GL_TEXTURE_LOD_BIAS:
      case GL_DEPTH_TEXTURE_MODE:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
      case GL_GENERATE_MIPMAP:
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_1D_ARRAY_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          targetIdx = __GL_TEXTURE_2D_ARRAY_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    tmpf[0] = (GLfloat)val;

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, this makes sure that we can update the right texture unit's
        ** states in current context when a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)tmpf, GL_TRUE);
        }
        imageUserList = imageUserList->next;
    }
}


GLvoid APIENTRY __glim_TexParameterIivEXT(GLenum target, GLenum pname, GLint *params)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameterIivEXT", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    tmpf[0] = (GLfloat)params[0];

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)tmpf, GL_FALSE);
        }
        imageUserList = imageUserList->next;
    }
}

GLvoid APIENTRY __glim_TexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params)
{
    GLuint targetIdx;
    __GLtextureObject *activeTex;
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexParameterIuivEXT", DT_GLenum, target, DT_GLenum, pname, DT_GLuint_ptr, params, DT_GLnull);
#endif

    switch (target) {
      case GL_TEXTURE_1D:
          targetIdx = __GL_TEXTURE_1D_INDEX;
          break;
      case GL_TEXTURE_2D:
          targetIdx = __GL_TEXTURE_2D_INDEX;
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
          break;
#endif
      case GL_TEXTURE_3D:
          targetIdx = __GL_TEXTURE_3D_INDEX;
          break;
      case GL_TEXTURE_CUBE_MAP:
          targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Flush the primitive buffer
    */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    tmpf[0] = (GLfloat)params[0];

    /* Update the tex parameter states in the active unit and the bind tex object.
    ** If there are other texture units bound to the same text object, we need to update
    ** the texture parameters in gc->state.texture.texUnits[unitIdx].texObj[targetIdx] as well.
    */
    activeTex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[targetIdx];

    imageUserList = activeTex->texUnitBoundList;
    while (imageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (activeTex == gc->texture.units[unitIdx].boundTextures[targetIdx])
        {
            __glTexParameterfv(gc, unitIdx, targetIdx, pname, (GLfloat*)tmpf, GL_FALSE);
        }
        imageUserList = imageUserList->next;
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
              __glSetError(GL_INVALID_ENUM);
              return;
          }
          break;

      case GL_TEXTURE_FILTER_CONTROL:
          if (pname == GL_TEXTURE_LOD_BIAS) {
              v[0] = gc->state.texture.texUnits[ui].lodBias;
          }
          else {
              __glSetError(GL_INVALID_ENUM);
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
                  __glSetError(GL_INVALID_ENUM);
                  return;
          }
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}


GLvoid APIENTRY __glim_GetTexEnvfv(GLenum target, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexEnvfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glGetTexEnvfv(gc, target, pname, v);
}

GLvoid APIENTRY __glim_GetTexEnviv(GLenum target, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexEnvfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, v, DT_GLnull);
#endif

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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
          v[0] = (GLfloat)tcs->mode;
          break;
      case GL_OBJECT_PLANE:
          v[0] = tcs->objectPlaneEquation.x;
          v[1] = tcs->objectPlaneEquation.y;
          v[2] = tcs->objectPlaneEquation.z;
          v[3] = tcs->objectPlaneEquation.w;
          break;
      case GL_EYE_PLANE:
          v[0] = tcs->eyePlaneEquation.x;
          v[1] = tcs->eyePlaneEquation.y;
          v[2] = tcs->eyePlaneEquation.z;
          v[3] = tcs->eyePlaneEquation.w;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_GetTexGenfv(GLenum coord, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexGenfv", DT_GLenum, coord, DT_GLenum, pname, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glGetTexGenfv(gc, coord, pname, v);
}

GLvoid APIENTRY __glim_GetTexGendv(GLenum coord, GLenum pname, GLdouble v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexGendv", DT_GLenum, coord, DT_GLenum, pname, DT_GLdouble_ptr, v, DT_GLnull);
#endif

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

GLvoid APIENTRY __glim_GetTexGeniv(GLenum coord, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexGeniv", DT_GLenum, coord, DT_GLenum, pname, DT_GLint_ptr, v, DT_GLnull);
#endif

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

__GL_INLINE GLvoid
__glGetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *v)
{
    __GLtextureUnit *unit = &(gc->texture.units[gc->state.texture.activeTexIndex]);
    __GLtextureObject *tex;
    __GLmipMapLevel *lp;
    GLint face = 0;

    /*make sure parameter level is valid*/
    GLint max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);
    if (level < 0 || level > max_lod)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (target) {
      case GL_TEXTURE_1D:
          tex = unit->boundTextures[__GL_TEXTURE_1D_INDEX];
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          tex = unit->boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
          break;
      case GL_TEXTURE_2D:
          tex = unit->boundTextures[__GL_TEXTURE_2D_INDEX];
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          tex = unit->boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          tex = unit->boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];
          break;
#endif
      case GL_TEXTURE_3D:
          tex = unit->boundTextures[__GL_TEXTURE_3D_INDEX];
          break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
          face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
          tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
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
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    lp = &tex->faceMipmap[face][level];

    switch (pname) {
      case GL_TEXTURE_WIDTH:
          v[0] = lp->width;
          break;
      case GL_TEXTURE_HEIGHT:
          v[0] = lp->height;
          break;
      case GL_TEXTURE_DEPTH:
          v[0] = lp->depth;
          break;
      case GL_TEXTURE_INTERNAL_FORMAT:
          v[0] = lp->requestedFormat;
          break;
      case GL_TEXTURE_BORDER:
          v[0] = lp->border;
          break;
      case GL_TEXTURE_RED_SIZE :
          v[0] = lp->deviceFormat->redSize;
          break;
      case GL_TEXTURE_GREEN_SIZE:
          v[0] = lp->deviceFormat->greenSize;
          break;
      case GL_TEXTURE_BLUE_SIZE:
          v[0] = lp->deviceFormat->blueSize;
          break;
      case GL_TEXTURE_ALPHA_SIZE:
          v[0] = lp->deviceFormat->alphaSize;
          break;
      case GL_TEXTURE_LUMINANCE_SIZE:
          v[0] = lp->deviceFormat->luminanceSize;
          break;
      case GL_TEXTURE_INTENSITY_SIZE:
          v[0] = lp->deviceFormat->intensitySize;
          break;
      case GL_TEXTURE_DEPTH_SIZE:
          v[0] = lp->deviceFormat->depthSize;
          break;
      case GL_TEXTURE_SHARED_SIZE_EXT:
          v[0] = lp->deviceFormat->sharedSize;
          break;
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
          switch (target) {
              case GL_PROXY_TEXTURE_1D:
              case GL_PROXY_TEXTURE_2D:
              case GL_PROXY_TEXTURE_3D:
              case GL_PROXY_TEXTURE_CUBE_MAP:
                  __glSetError(GL_INVALID_OPERATION);
                  return;
          }
          if (lp->compressed == GL_FALSE) {
              __glSetError(GL_INVALID_OPERATION);
              return;
          }
          v[0] = lp->compressedSize;
          break;
      case GL_TEXTURE_COMPRESSED:
          v[0] = lp->compressed;
          break;

      case GL_TEXTURE_RED_TYPE_ARB:
          v[0] = lp->deviceFormat->redType;
          break;
      case GL_TEXTURE_GREEN_TYPE_ARB:
          v[0] = lp->deviceFormat->greenType;
          break;
      case GL_TEXTURE_BLUE_TYPE_ARB:
          v[0] = lp->deviceFormat->blueType;
          break;
      case GL_TEXTURE_ALPHA_TYPE_ARB:
          v[0] = lp->deviceFormat->alphaType;
          break;
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
          v[0] = lp->deviceFormat->luminanceType;
          break;
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
          v[0] = lp->deviceFormat->intensityType;
          break;
      case GL_TEXTURE_DEPTH_TYPE_ARB:
          v[0] = lp->deviceFormat->depthType;
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat v[])
{
    GLint ti = 0;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexLevelParameterfv", DT_GLenum, target, DT_GLint, level, DT_GLenum, pname, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glGetTexLevelParameteriv(gc, target, level, pname, &ti);
    v[0] = (GLfloat)ti;
}

GLvoid APIENTRY __glim_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint v[])
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexLevelParameteriv", DT_GLenum, target, DT_GLint, level, DT_GLenum, pname, DT_GLint_ptr, v, DT_GLnull);
#endif

    __glGetTexLevelParameteriv(gc, target, level, pname, v);
}


__GL_INLINE GLvoid
__glGetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *v)
{
    __GLtextureUnit *unit = &(gc->texture.units[gc->state.texture.activeTexIndex]);
    __GLtextureParamState *params;
    __GLtextureObject *tex;

    switch (target) {
      case GL_TEXTURE_1D:
          tex = unit->boundTextures[__GL_TEXTURE_1D_INDEX];
          break;
      case GL_TEXTURE_1D_ARRAY_EXT:
          tex = unit->boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
          break;
      case GL_TEXTURE_2D:
          tex = unit->boundTextures[__GL_TEXTURE_2D_INDEX];
          break;
      case GL_TEXTURE_2D_ARRAY_EXT:
          tex = unit->boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
          break;
#if GL_ARB_texture_rectangle
      case GL_TEXTURE_RECTANGLE_ARB:
          tex = unit->boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];
          break;
#endif
      case GL_TEXTURE_3D:
          tex = unit->boundTextures[__GL_TEXTURE_3D_INDEX];
          break;
      case GL_TEXTURE_CUBE_MAP:
          tex = unit->boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    params = &tex->params;
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
          v[0] = (GLfloat)params->sWrapMode;
          break;
      case GL_TEXTURE_WRAP_T:
          v[0] = (GLfloat)params->tWrapMode;
          break;
      case GL_TEXTURE_WRAP_R:
          v[0] = (GLfloat)params->rWrapMode;
          break;
      case GL_TEXTURE_MIN_FILTER:
          v[0] = (GLfloat)params->minFilter;
          break;
      case GL_TEXTURE_MAG_FILTER:
          v[0] = (GLfloat)params->magFilter;
          break;
      case GL_TEXTURE_BORDER_COLOR:
          v[0] = (params->borderColor.r);
          v[1] = (params->borderColor.g);
          v[2] = (params->borderColor.b);
          v[3] = (params->borderColor.a);
          break;
      case GL_TEXTURE_PRIORITY:
          v[0] = (GLfloat)params->priority;
          break;
      case GL_TEXTURE_MIN_LOD:
          v[0] = (GLfloat)params->minLod;
          break;
      case GL_TEXTURE_MAX_LOD:
          v[0] = (GLfloat)params->maxLod;
          break;
      case GL_TEXTURE_BASE_LEVEL:
          v[0] = (GLfloat)params->baseLevel;
          break;
      case GL_TEXTURE_MAX_LEVEL:
          v[0] = (GLfloat)params->maxLevel;
          break;
      case GL_TEXTURE_LOD_BIAS:
          v[0] = (GLfloat)params->lodBias;
          break;
      case GL_DEPTH_TEXTURE_MODE:
          v[0] = (GLfloat)params->depthTexMode;
          break;
      case GL_TEXTURE_COMPARE_MODE:
          v[0] = (GLfloat)tex->params.compareMode;
          break;
      case GL_TEXTURE_COMPARE_FUNC:
          v[0] = (GLfloat)tex->params.compareFunc;
          break;
      case GL_GENERATE_MIPMAP:
          v[0] = (GLfloat)params->generateMipmap;
          break;
      case GL_TEXTURE_RESIDENT:
          v[0] = (*gc->dp.isTextureResident)(gc, tex);
          break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
          v[0] = (GLfloat)params->anisotropicLimit;
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_GetTexParameterfv(GLenum target, GLenum pname, GLfloat v[])
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glGetTexParameterfv(gc, target, pname, v);
}

GLvoid APIENTRY __glim_GetTexParameteriv(GLenum target, GLenum pname, GLint v[])
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, v, DT_GLnull);
#endif

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

GLvoid APIENTRY __glim_GetTexParameterIivEXT(GLenum target, GLenum pname, GLint *params)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexParameterIivEXT", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    __glGetTexParameterfv(gc, target, pname, tmpf);

    params[0] = (GLint)(tmpf[0]);
}

GLvoid APIENTRY __glim_GetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint *params)
{
    GLfloat tmpf[4] = {0.0, 0.0, 0.0, 0.0};
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexParameterIuivEXT", DT_GLenum, target, DT_GLenum, pname, DT_GLuint_ptr, params, DT_GLnull);
#endif

    __glGetTexParameterfv(gc, target, pname, tmpf);

    params[0] = (GLuint)(tmpf[0]);

}

GLvoid APIENTRY __glim_ClientActiveTexture(GLenum texture)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClientActiveTexture", DT_GLenum, texture, DT_GLnull);
#endif

    if ((texture > (GL_TEXTURE0 + __GL_MAX_TEXTURE_COORDS - 1)) ||
        (texture < GL_TEXTURE0))
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    texture = texture - GL_TEXTURE0;

    gc->clientState.vertexArray.clientActiveUnit = texture;
}


GLvoid APIENTRY __glim_ActiveTexture(GLenum texture)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ActiveTexture", DT_GLenum, texture, DT_GLnull);
#endif

    if ((texture > (GL_TEXTURE0 + __GL_MAX_TEXTURE_UNITS - 1)) ||
        (texture < GL_TEXTURE0)) {
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    texture = texture - GL_TEXTURE0;
    gc->state.texture.activeTexIndex = texture;

    /* Call the dp interface */
    (*gc->dp.activeTexture)(gc, texture);
}

/*
** This routine actually performs the bind texture.
*/
#if (defined(DEBUG) || (defined(_DEBUG)))
GLuint textureTarget = 0;
#endif
GLvoid __glBindTexture(__GLcontext *gc, GLuint unitIdx, GLuint targetIndex, GLuint texture)
{
    __GLtextureObject *texObj, *boundTexObj;
    __GLtextureObjAttr *texObjAttr = &gc->state.texture.texUnits[unitIdx].texObj[targetIndex];

#if (defined(DEBUG) || (defined(_DEBUG)))
    if (texture == textureTarget)
        textureTarget = textureTarget;
#endif

    if (texObjAttr->name == texture)
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    GL_ASSERT(NULL != gc->texture.shared);

    if (texture == 0) {
        /* Retrieve the default texture object in __GLcontext.
        */
        texObj = &gc->texture.defaultTextures[targetIndex];
        GL_ASSERT(NULL != texObj);
        GL_ASSERT(texObj->name == 0);
    }
    else {
        /*
        ** Retrieve the texture object from the "gc->texture.shared" structure.
        */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
    }

    if (NULL == texObj) {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new texture object and initialize it.
        */
        texObj = (__GLtextureObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLtextureObject) );
        __glInitTextureObject(gc, texObj, texture, targetIndex);

        /* Add this texture object to the "gc->texture.shared" structure.
        */
        __glAddObject(gc, gc->texture.shared, texture, texObj);

        /* Mark the name "texture" used in the texture nameArray.
        */
        __glMarkNameUsed(gc, gc->texture.shared, texture);

    }
    else {
        /*
        ** Retrieved an existing texture object.  Do some sanity checks.
        */
        if (texObj->targetIndex != targetIndex) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    /* Release the previously bound texture for this target.
    ** And install the new texture object to the target.
    */
    boundTexObj = gc->texture.units[unitIdx].boundTextures[targetIndex];
    gc->texture.units[unitIdx].boundTextures[targetIndex] = texObj;

    /* DO NOT call __glRemoveImageUser(gc, &boundTexObj->texUnitBoundList, (GLvoid*)unitIdx) here.
    ** reason: consider boundTexObj is bound to both gc1 and gc2's texture unit 1 (1 is in texUnitBoundList),
    ** in gc1, texture unit 1 may then bind to another texture object, if we call __glRemoveImageUser here,
    ** texUnitBoundList will be empty. When switching to gc2, the link between boundTexObj and gc2's
    ** texture unit 1 is also deleted. So don't call __glRemoveImageUser here may result in redundancy of
    ** texUnitBoundList in single context, but it makes sure the correctness in shared context.
    */

    /* add current active texture unit index to current bound texture object's texUnitBoundList */
    __glAddImageUser(gc, &texObj->texUnitBoundList, (GLvoid*)(GLuint_ptr)unitIdx, NULL);

    /* Copy texParameters in __GLcontext back to the boundTexObj */
    boundTexObj->params = texObjAttr->params;
    GL_ASSERT(boundTexObj->name == texObjAttr->name);

    /* Remove gc from boundTexObj->userList if there are no other tex units bound to boundTexObj */
    if (boundTexObj->name != 0) {
        boundTexObj->bindCount--;

        if (boundTexObj->bindCount == 0 && (boundTexObj->flag & __GL_OBJECT_IS_DELETED)) {
            __glDeleteObject(gc, gc->texture.shared, boundTexObj->name);
        }
    }

    /* Now __GLcontext adopts the params and name of the newly bound texture object */
    texObjAttr->params = texObj->params;
    texObjAttr->name = texObj->name;

    /* Add gc to texObj->userList if texObj is not default texture object.
    */
    if (texObj->name != 0) {
        texObj->bindCount++;
    }

    /* Set all texParamter and texture image dirty bits.
    */
    __GL_SET_TEX_UNIT_BIT(gc, unitIdx, __GL_TEXPARAMETER_BITS | __GL_TEXIMAGE_BITS);

    /* Call the dp interface */
    (*gc->dp.bindTexture)(gc, texObj);
}

GLvoid APIENTRY __glim_BindTexture(GLenum target, GLuint texture)
{
    GLuint targetIndex;
    /*
    ** Need to validate in case a new texture was popped into
    ** the state immediately prior to this call.
    */
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindTexture", DT_GLenum, target, DT_GLuint, texture, DT_GLnull);
#endif


    switch (target) {
    case GL_TEXTURE_1D:
        targetIndex = __GL_TEXTURE_1D_INDEX;
        break;
    case GL_TEXTURE_2D:
        targetIndex = __GL_TEXTURE_2D_INDEX;
        break;
#if GL_ARB_texture_rectangle
    case GL_TEXTURE_RECTANGLE_ARB:
        targetIndex = __GL_TEXTURE_RECTANGLE_INDEX;
        break;
#endif
    case GL_TEXTURE_3D:
        targetIndex = __GL_TEXTURE_3D_INDEX;
        break;
    case GL_TEXTURE_CUBE_MAP:
        targetIndex = __GL_TEXTURE_CUBEMAP_INDEX;
        break;

    case GL_TEXTURE_1D_ARRAY_EXT:
        targetIndex = __GL_TEXTURE_1D_ARRAY_INDEX;
        break;

    case GL_TEXTURE_2D_ARRAY_EXT:
        targetIndex = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;

    case GL_TEXTURE_BUFFER_EXT:
        targetIndex = __GL_TEXTURE_BUFFER_INDEX;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __glBindTexture(gc,gc->state.texture.activeTexIndex, targetIndex, texture);

    /* Need to reset the current texture and such. */
}

GLvoid APIENTRY __glim_DeleteTextures(GLsizei n, const GLuint* textures)
{
    GLint i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteTextures", DT_GLsizei, n, DT_GLuint_ptr, textures, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*
    ** If a texture that is being deleted is currently bound,
    ** bind the default texture to its target.
    */
    LINUX_LOCK_FRAMEBUFFER(gc);
    for (i = 0; i < n; i++)
    {
        /* skip default textures */
        if (textures[i])
        {
#if (defined(DEBUG) || (defined(_DEBUG)))
            if (textures[i] == textureTarget)
                textureTarget = textureTarget;
#endif
            __glDeleteObject(gc, gc->texture.shared, textures[i]);
        }
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLboolean __glDeleteTextureObject(__GLcontext *gc, GLvoid *obj)
{
    __GLtextureObject *tex = obj;
    GLuint targetIndex = tex->targetIndex;
    GLuint i, faces;
    __GLimageUser *texImageUserList;
#if GL_EXT_framebuffer_object
    __GLimageUser *imageUserList = tex->fboList;
    __GLimageUser *nextUser = NULL;
    __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
    __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;
#endif

    /*
    ** If the texture object that is being deleted is currently bound to gc,
    ** unbind the texobj from gc and bind the default texobj 0 to gc.
    */
    texImageUserList = tex->texUnitBoundList;
    while (texImageUserList)
    {
        GLuint unitIdx = (GLuint)glALL_TO_UINT32(texImageUserList->imageUser);

        /* redundancy check, so texture unit's states can be updated correctly when
        ** a texture object is shared by two or more contexts.
        */
        if (tex == gc->texture.units[unitIdx].boundTextures[targetIndex])
        {
            /*
            ** __GL_OBJECT_IS_DELETED is cleared because we do not want this object
            ** deleted in the following function, otherwise there will be recursion:
            ** __glDeleteObject-->__glBindObject-->glDeleteObject, and the object will
            ** be deleted twice.
            */
            tex->flag &= ~__GL_OBJECT_IS_DELETED;
            __glBindTexture(gc, unitIdx, targetIndex, 0);
        }

        if( tex == gc->texture.units[unitIdx].currentTexture )
        {
            gc->texture.units[unitIdx].currentTexture = NULL;
        }

        texImageUserList = texImageUserList->next;
    }

    /* If the texture is bound to a colorbuffer, release it first. */
    if (tex->hPbuffer != NULL)
    {
        __glReleaseTexImageImplicit(gc,tex->hPbuffer,tex->colorBuffer,tex);
    }

#if GL_EXT_framebuffer_object
    /* dirty all fbo this rbo attached to */
    while(imageUserList)
    {
        /*
        ** Spec.: If a texture object is deleted while its image is attached to one or
        **        more attachment points in the currently bound framebuffer, then it
        **        is as if FramebufferTexture{1D|2D|3D}EXT() had been called, with a
        **        <texture> of 0, for each attachment point to which this image was
        **        attached in the currently bound framebuffer.
        */

        /*
        ** imageUserList may be freed in __glFrameBufferTexture,
        ** so get the next in the beginning
        */
        nextUser = imageUserList->next;

        if(imageUserList->imageUser == drawFbo)
        {
            for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if(drawFbo->attachPoint[i].objName == tex->name && drawFbo->attachPoint[i].objectType == GL_TEXTURE)
                {
                    tex->flag &= ~__GL_OBJECT_IS_DELETED;
                    __glFrameBufferTexture(gc, drawFbo, i, NULL, 0, 0, 0, GL_FALSE);
                }
            }
        }

        if(readFbo != drawFbo)
        {
            if(imageUserList->imageUser == readFbo)
            {
                for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
                {
                    if(readFbo->attachPoint[i].objName == tex->name && readFbo->attachPoint[i].objectType == GL_TEXTURE)
                    {
                        tex->flag &= ~__GL_OBJECT_IS_DELETED;
                        __glFrameBufferTexture(gc, readFbo, i, NULL, 0, 0, 0, GL_FALSE);
                    }
                }
            }
        }

        imageUserList = nextUser;
    }
#endif

    /* Do not delete the texObj if there are other contexts bound to it. */
    if (tex->bindCount != 0)
    {
        /* Set the flag to indicate the object is marked for delete */
        tex->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    /* The object is truly deleted here, so delete the object name from name list. */
    __glDeleteNamesFrList(gc, gc->texture.shared, tex->name, 1);

    /* Notify Dp that this texture object is deleted.
    */
    if (tex->privateData )
    {
        (*gc->dp.deleteTexture)(gc, tex);
    }

#if SWP_PIPELINE_ENABLED
    if(tex->swpPrivateData)
    {
        (*gc->swp.deleteTexture)(gc, tex);
    }
#endif

    switch(tex->targetIndex)
    {
        case __GL_TEXTURE_CUBEMAP_INDEX:
            faces = 6;
            break;

        case __GL_TEXTURE_1D_ARRAY_INDEX:
        case __GL_TEXTURE_2D_ARRAY_INDEX:
            faces = gc->constants.maxTextureArraySize;
            /* Set levels to 1 for texture array because texture cache is only allocated once for same level */
            /* in different elements of array */
            /* Same level in different elements of array shares one comtigious memeory */
            break;

        case __GL_TEXTURE_BUFFER_INDEX:
            faces = 1;
            if(tex->bufferObj)
                __glRemoveImageUser(gc, &(tex->bufferObj->bufferObjData->bufferObjUserList), tex);
            break;

        default:
            faces = 1;
            break;
    }

    /* Delete the texture object's texture image */
    for( i = 0; i < faces; i ++)
    {
        if (tex->faceMipmap[i])
        {
            (*gc->imports.free)(gc, tex->faceMipmap[i]);
            tex->faceMipmap[i] = NULL;
        }
    }
    if (tex->faceMipmap )
    {
        (*gc->imports.free)(gc, tex->faceMipmap);
        tex->faceMipmap = NULL;
    }

    /* Free fbolist */
    __glFreeImageUserList(gc, &tex->fboList);

    /* Free texture unit list */
    __glFreeImageUserList(gc, &tex->texUnitBoundList);

    /* Delete the texture object structure */
    (*gc->imports.free)(gc, tex);

    return GL_TRUE;
}

GLvoid APIENTRY __glim_GenTextures(GLsizei n, GLuint *textures)
{
    GLint start, i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenTextures", DT_GLsizei, n, DT_GLuint_ptr, textures, DT_GLnull);
#endif

    if (NULL == textures)
        return;

    if (n < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
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
}

GLboolean APIENTRY __glim_IsTexture(GLuint texture)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsTexture", DT_GLuint, texture, DT_GLnull);
#endif

    return __glIsNameDefined(gc, gc->texture.shared, texture);
}

GLvoid APIENTRY __glim_PrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
    GLint i;
    __GLtextureObject *tex;
    __GLimageUser *imageUserList;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PrioritizeTextures", DT_GLsizei, n, DT_GLuint_ptr, textures, DT_GLfloat_ptr, priorities, DT_GLnull);
#endif

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
            GLuint unitIdx = (GLuint)glALL_TO_UINT32(imageUserList->imageUser);

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

GLboolean APIENTRY __glim_AreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    GLint i;
    __GLtextureObject *tex;
    GLboolean allResident = GL_TRUE;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_AreTexturesResident", DT_GLsizei, n, DT_GLuint_ptr, textures, DT_GLboolean_ptr, residences, DT_GLnull);
#endif

    for (i = 0; i < n; i++) {
        /* Can't query a default texture. */
        if (0 == textures[i]) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }

        tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, textures[i]);

        /*
        ** Ensure that all of the names have corresponding textures.
        */
        if (NULL == tex) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }

        if ((*gc->dp.isTextureResident)(gc, tex)) {
            residences[i] = GL_TRUE;
        }
        else {
            allResident = GL_FALSE;
            residences[i] = GL_FALSE;
        }
    }

    return allResident;
}


