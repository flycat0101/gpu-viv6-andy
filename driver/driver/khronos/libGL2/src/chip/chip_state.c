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
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

extern GLenum setClearDepth(glsCHIPCONTEXT_PTR  chipCtx, GLfloat clearValue);
extern GLenum setDepthRange(glsCHIPCONTEXT_PTR chipCtx, GLfloat zNear, GLfloat zFar);
extern GLenum setDepthMask(glsCHIPCONTEXT_PTR  chipCtx, GLboolean depthMask);
extern GLenum setClearDepth(glsCHIPCONTEXT_PTR  chipCtx, GLfloat clearValue);
extern GLenum setStencilCompareFunction(glsCHIPCONTEXT_PTR chipCtx, GLenum Function, GLint Reference, GLuint Mask, GLenum face);
extern GLvoid setWriteMask(glsCHIPCONTEXT_PTR chipCtx, GLuint Mask);
extern GLenum setStencilOperations(glsCHIPCONTEXT_PTR chipCtx, GLenum Fail, GLenum ZFail, GLenum ZPass, GLenum face);
extern GLenum setClearStencil(glsCHIPCONTEXT_PTR chipCtx, GLint clearValue);
extern GLvoid updateStageEnable(glsCHIPCONTEXT_PTR chipCtx, glsTEXTURESAMPLER_PTR Sampler, GLboolean Enabled);
extern GLboolean setTexCoordGenMode(glsCHIPCONTEXT_PTR chipCtx, glsTEXTURESAMPLER_PTR Sampler, const GLvoid* Value, gleTYPE Type, GLuint index);
extern GLenum setAlphaTestReference(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Function,
    GLfloat Value
    );

extern glfDOTEXTUREFUNCTION _TextureFunctions[];

#define glmSETTEXPARAMETER(Name, value) \
    { \
       halTexture->Name = value; \
    }

__GL_INLINE GLvoid updateTexGenState(__GLcontext *gc, GLuint unit, GLuint64 localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLtextureUnitState *tex = &gc->state.texture.texUnits[unit];
    __GLTextureEnableState * es = &gc->state.enables.texUnits[unit];
    glsTEXTURESAMPLER_PTR sampler;

    if (localMask & (__GL_TEXGEN_S_ENDISABLE_BIT |
        __GL_TEXGEN_T_ENDISABLE_BIT |
        __GL_TEXGEN_R_ENDISABLE_BIT |
        __GL_TEXGEN_Q_ENDISABLE_BIT))
    {
        /* right now if one of them is enabled, texture gen will be enabled. */
        chipCtx->texture.sampler[unit].genEnable = (es->texGen[0] | (es->texGen[1] << 1) | (es->texGen[2] << 2) | (es->texGen[3] << 3));
        /* Update the hash key. */
        glmSETHASH_4BITS(hashTexCoordGenEnable, chipCtx->texture.sampler[unit].genEnable,
            unit);
    }

    /* Now assume tex gen mode is always the same for s, t, r, q */
    sampler = &chipCtx->texture.sampler[unit];
    if (localMask &  __GL_TEXGEN_S_BIT) {
        setTexCoordGenMode(chipCtx, sampler, &tex->s.mode, glvINT, 0);
    }
    if (localMask &  __GL_TEXGEN_T_BIT) {
        setTexCoordGenMode(chipCtx, sampler, &tex->t.mode, glvINT, 1);
    }
    if (localMask &  __GL_TEXGEN_R_BIT) {
        setTexCoordGenMode(chipCtx, sampler, &tex->r.mode, glvINT, 2);
    }
    if (localMask &  __GL_TEXGEN_Q_BIT) {
        setTexCoordGenMode(chipCtx, sampler, &tex->q.mode, glvINT, 3);
    }
}

#define __GL_ISFILLTYPE(PT, frontMode, backMode)  \
    ((((frontMode) == GL_FILL) && ((backMode) == GL_FILL)) &&   \
     (((PT) == GL_TRIANGLES) ||  \
      ((PT) == GL_TRIANGLE_FAN) || \
      ((PT) == GL_TRIANGLE_STRIP) || \
      ((PT) == GL_QUADS) || \
      ((PT) == GL_QUAD_STRIP) ||    \
      ((PT) == GL_POLYGON) ||   \
      ((PT) == GL_TRIANGLES_ADJACENCY_EXT) ||   \
      ((PT) == GL_TRIANGLE_STRIP_ADJACENCY_EXT)))

__GL_INLINE GLboolean isFillRendering(__GLcontext *gc)
{
    if (gc->shaderProgram.geomShaderEnable)
    {
        return __GL_ISFILLTYPE(gc->shaderProgram.geomOutputType,
            gc->state.polygon.frontMode,
            gc->state.polygon.backMode);
    }
    else
    {
        return __GL_ISFILLTYPE(gc->vertexStreams.primMode,
            gc->state.polygon.frontMode,
            gc->state.polygon.backMode);
    }
}

/********************************************************************
**
**  initPolygonStipplePatch
**
**  Initialize polygon stipple patch
**  Create stipple resource.
**
**  Parameters:
**  Return Value:
**
********************************************************************/
GLvoid initPolygonStipplePatch(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx)
{
    gceSTATUS status;
    glsTEXTUREINFO * textureInfo = &chipCtx->polygonStippleTextureInfo;
    glsTEXTURESAMPLER * polygonStippleSampler = &chipCtx->polygonStippleSampler;;

    chipCtx->polygonStippleTextureStage = -1;

    gcoTEXTURE_InitParams(chipCtx->hal, &chipCtx->polygonStippleTexture);
    chipCtx->polygonStippleTexture.magFilter = gcvTEXTURE_POINT;
    chipCtx->polygonStippleTexture.minFilter = gcvTEXTURE_POINT;
    chipCtx->polygonStippleTexture.mipFilter = gcvTEXTURE_NONE;

    textureInfo->imageFormat = textureInfo->residentFormat = gcvSURF_L8;
    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }
    textureInfo->residentLevels = 1;

    textureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
    textureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
    textureInfo->format = GL_LUMINANCE;

    status = gcoTEXTURE_AddMipMap(
           textureInfo->object,
           0,
           gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
           textureInfo->residentFormat,
           32,
           32, 0,
           0,
           gcvPOOL_DEFAULT,
           gcvNULL
    );

    if (gcmIS_ERROR(status)) {
        return;
    }

    polygonStippleSampler->binding = textureInfo;
    polygonStippleSampler->genEnable = 0;
    polygonStippleSampler->coordType    = gcSHADER_FLOAT_X2;
    polygonStippleSampler->coordSwizzle = gcSL_SWIZZLE_XYYY;
    /* modulate */
    polygonStippleSampler->doTextureFunction = _TextureFunctions[1];
}

/********************************************************************
**
**  freereePolygonStipplePatch
**
**  Free the stipple texture
**
**  Parameters:
**  Return Value:
**
********************************************************************/
GLvoid freePolygonStipplePatch(__GLcontext *gc,  glsCHIPCONTEXT_PTR chipCtx)
{
    glsTEXTUREINFO * textureInfo = &chipCtx->polygonStippleTextureInfo;

    if (textureInfo->object) {
        gcoTEXTURE_Destroy(textureInfo->object);
        textureInfo->object = NULL;
    }
}

/********************************************************************
**
**  loadPolygonStippleImage
**
**  Load polygon stipple image, and convert it into L8 texture.
**  Then BLT to the stipple resource.
**
**  Parameters:
**  Return Value:
**
********************************************************************/
GLvoid loadPolygonStippleImage(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx)
{
    GLint   line, pixel, shiftBit;
    GLuint  index = 0;
    GLubyte *stipplePattern = (GLubyte *)&gc->state.polygonStipple.stipple[0];
    GLubyte texImage[32][32];
    GLubyte *texImagePointer = &texImage[0][0];
    glsTEXTUREINFO * textureInfo = &chipCtx->polygonStippleTextureInfo;

    GLuint * pattern = (GLuint *)stipplePattern;
    GLuint * cachedPattern = (GLuint *)chipCtx->cachedStipplePattern;
    GLboolean diff = GL_FALSE;

    chipCtx->isSolidPolygonStipple = GL_TRUE;

    /* Convert bit mask to image */
    for (line = 31; line >= 0; line--)
    {
        if (pattern[line] != cachedPattern[line])
        {
            cachedPattern[line] = pattern[line];
            diff = GL_TRUE;
        }

        for (pixel = 0; pixel < 4; pixel++)
        {
            /* Revert bit */
            for (shiftBit = 7; shiftBit >= 0; shiftBit--)
            {
                if (stipplePattern[index] & (1 << shiftBit))
                {
                    *texImagePointer++ = 0xFF;
                }
                else
                {
                    *texImagePointer++ = 0;
                    chipCtx->isSolidPolygonStipple = GL_FALSE;
                }
            }
            index++;
        }
    }

    /* pattern is same or pattern is solid, do not need to upload the pattern */
    if(!diff || chipCtx->isSolidPolygonStipple)
        return;

    if (chipCtx->drawRT[0]) {
        gcoSURF_Flush(chipCtx->drawRT[0]);
        /* Commit command buffer. */
        gcoHAL_Commit(chipCtx->hal, gcvTRUE);
    }

    if (textureInfo->object) {
        gcoTEXTURE_Upload(textureInfo->object,
                          0,
                          0,
                          32,
                          32,
                          0,
                          &texImage[0][0],
                          0, /* stride */
                          textureInfo->imageFormat,
                          gcvSURF_COLOR_SPACE_LINEAR);
    }
    textureInfo->dirty = GL_TRUE;

    CHIP_TEX_IMAGE_UPTODATE(textureInfo, 0);
}

/********************************************************************
**
**  initLineStipplePatch
**
**  Initialize polygon stipple patch
**  Create stipple resource.
**
**  Parameters:
**  Return Value:
**
********************************************************************/
GLvoid initLineStipplePatch(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx)
{
    gceSTATUS status;
    glsTEXTUREINFO * textureInfo = &chipCtx->lineStippleTextureInfo;
    glsTEXTURESAMPLER * lineStippleSampler = &chipCtx->lineStippleSampler;

    chipCtx->lineStippleTextureStage = -1;

    gcoTEXTURE_InitParams(chipCtx->hal, &chipCtx->lineStippleTexture);
    chipCtx->lineStippleTexture.magFilter = gcvTEXTURE_POINT;
    chipCtx->lineStippleTexture.minFilter = gcvTEXTURE_POINT;
    chipCtx->lineStippleTexture.mipFilter = gcvTEXTURE_NONE;

    textureInfo->imageFormat = textureInfo->residentFormat = gcvSURF_L8;
    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }

    textureInfo->residentLevels = 1;

    textureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
    textureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
    textureInfo->format = GL_LUMINANCE;

    lineStippleSampler->binding = textureInfo;
    lineStippleSampler->genEnable = 0;
    lineStippleSampler->coordType    = gcSHADER_FLOAT_X1;
    lineStippleSampler->coordSwizzle = gcSL_SWIZZLE_XXXX;
    /* modulate */
    lineStippleSampler->doTextureFunction = _TextureFunctions[1];
}

/********************************************************************
**
**  freeLineStipplePatch
**
**  Free the stipple texture
**
**  Parameters:
**  Return Value:
**
********************************************************************/
GLvoid freeLineStipplePatch(__GLcontext *gc,  glsCHIPCONTEXT_PTR chipCtx)
{
    glsTEXTUREINFO * textureInfo = &chipCtx->lineStippleTextureInfo;

    if (textureInfo->object) {
        gcoTEXTURE_Destroy(textureInfo->object);
        textureInfo->object = NULL;
    }
}

GLvoid loadLineStippleImage(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx)
{
    GLuint bit;
    GLuint repeat;
    gceSTATUS status;
    glsTEXTUREINFO *textureInfo = &chipCtx->lineStippleTextureInfo;
    /* buffer to hold maximum repeat count, but later part might not be used */
    GLubyte pTexImage[16 * 256];
    GLubyte *pTexImagePointer = pTexImage;
    GLushort stipplePattern = gc->state.line.stipple;
    GLuint repeatCnt = (GLuint)gc->state.line.stippleRepeat;

    GL_ASSERT(repeatCnt >0 && repeatCnt <= 256);

    chipCtx->isSolidLineStipple = GL_TRUE;
    for (bit = 0; bit < 16; ++bit) {
        if (stipplePattern & (1 << bit)) {
            for (repeat = 0; repeat < repeatCnt; ++repeat) {
                *pTexImagePointer++ = 0xFF;
            }
        } else {
            for (repeat = 0; repeat < repeatCnt; ++repeat) {
                *pTexImagePointer++ = 0;
            }
            chipCtx->isSolidLineStipple = GL_FALSE;
        }
    }

    /* pattern is solid, do not need to upload the pattern */
    if(chipCtx->isSolidLineStipple)
        return;

    if (chipCtx->drawRT[0]) {
        gcoSURF_Flush(chipCtx->drawRT[0]);
        /* Commit command buffer. */
        gcoHAL_Commit(chipCtx->hal, gcvTRUE);
    }

    status = gcoTEXTURE_AddMipMap(
        textureInfo->object,
        0,
        gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
        textureInfo->residentFormat,
        16*repeatCnt, 1, 0,
        0,
        gcvPOOL_DEFAULT,
        gcvNULL
        );

    if (gcmIS_ERROR(status)) {
        return;
    }

    if (textureInfo->object) {
        gcoTEXTURE_Upload(textureInfo->object,
                          0,
                          0,
                          16*repeatCnt,
                          1,
                          0,
                          pTexImage,
                          0, /* stride */
                          textureInfo->imageFormat,
                          gcvSURF_COLOR_SPACE_LINEAR);
    }

    /* repeat count was programmed as immediate in FS, so different repeat count will generate different FS. */
    chipCtx->hashKey.lineStippleRepeat = (repeatCnt - 1);
    textureInfo->dirty = GL_TRUE;

    CHIP_TEX_IMAGE_UPTODATE(textureInfo, 0);
}
extern GLboolean setTextureFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCurrentColor(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineAlphaFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineColorFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineColorSource(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineAlphaSource(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineColorOperand(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineAlphaOperand(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setColorScale(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setAlphaScale(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
__GL_INLINE GLvoid updateTextureEnv(__GLcontext *gc, GLuint unit, GLuint64 localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTURESAMPLER_PTR sampler;
    __GLtextureEnvState *tes = &gc->state.texture.texUnits[unit].env;

    /* Set current active sampler */
    sampler = &chipCtx->texture.sampler[unit];

    if (localMask & __GL_TEXENV_MODE_BIT) {
        setTextureFunction(chipCtx, sampler, &tes->mode, glvINT);
    }

    if (localMask & __GL_TEXENV_COLOR_BIT) {
        setCurrentColor(chipCtx, sampler, &tes->color, glvFLOAT);
    }

    if (localMask & __GL_TEXENV_COMBINE_ALPHA_BIT) {
        setCombineAlphaFunction(chipCtx, sampler, &tes->function.alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_COMBINE_RGB_BIT) {
        setCombineColorFunction(chipCtx, sampler, &tes->function.rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_SOURCE0_RGB_BIT) {
        setCombineColorSource(chipCtx, GL_SRC0_RGB, sampler, &tes->source[0].rgb, glvINT);
    }
    if (localMask & __GL_TEXENV_SOURCE1_RGB_BIT) {
        setCombineColorSource(chipCtx, GL_SRC1_RGB, sampler, &tes->source[1].rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_SOURCE2_RGB_BIT) {
        setCombineColorSource(chipCtx, GL_SRC2_RGB, sampler, &tes->source[2].rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_SOURCE0_ALPHA_BIT) {
        setCombineAlphaSource(chipCtx, GL_SRC0_ALPHA, sampler, &tes->source[0].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_SOURCE1_ALPHA_BIT) {
        setCombineAlphaSource(chipCtx, GL_SRC1_ALPHA, sampler, &tes->source[1].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_SOURCE2_ALPHA_BIT) {
        setCombineAlphaSource(chipCtx, GL_SRC2_ALPHA, sampler, &tes->source[2].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND0_RGB_BIT) {
        setCombineColorOperand(chipCtx, GL_OPERAND0_RGB, sampler, &tes->operand[0].rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND1_RGB_BIT) {
        setCombineColorOperand(chipCtx, GL_OPERAND1_RGB, sampler, &tes->operand[1].rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND2_RGB_BIT) {
        setCombineColorOperand(chipCtx, GL_OPERAND2_RGB, sampler, &tes->operand[2].rgb, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND0_ALPHA_BIT) {
        setCombineAlphaOperand(chipCtx, GL_OPERAND0_ALPHA, sampler, &tes->operand[0].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND1_ALPHA_BIT) {
        setCombineAlphaOperand(chipCtx, GL_OPERAND1_ALPHA, sampler, &tes->operand[1].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_OPERAND2_ALPHA_BIT) {
        setCombineAlphaOperand(chipCtx, GL_OPERAND2_ALPHA, sampler, &tes->operand[2].alpha, glvINT);
    }

    if (localMask & __GL_TEXENV_RGB_SCALE_BIT) {
        setColorScale(chipCtx, sampler, &tes->rgbScale, glvFLOAT);
    }

    if (localMask & __GL_TEXENV_ALPHA_SCALE_BIT) {
        setAlphaScale(chipCtx, sampler, &tes->alphaScale, glvFLOAT);
    }

    if (localMask & __GL_TEXENV_COORD_REPLACE_BIT) {
        sampler->coordReplace = GL_TRUE;
    }
}

__GL_INLINE GLboolean setMinFilter(GLenum minFilter, GLuint * value, GLint anisotropicLimit)
{
    if (anisotropicLimit <= 1) {
        switch (minFilter) {
            case GL_NEAREST:
                /* mipfilter case */
                if (anisotropicLimit < 0) {
                    *value = gcvTEXTURE_NONE;
                    break;
                }
                /* otherwise fall through */
            case GL_NEAREST_MIPMAP_NEAREST:
            case GL_LINEAR_MIPMAP_NEAREST:
                *value = gcvTEXTURE_POINT;
                break;
            case GL_LINEAR:
                /* mipfilter case */
                if (anisotropicLimit < 0) {
                    *value = gcvTEXTURE_NONE;
                    break;
                }
                /* otherwise fall through */
            case GL_NEAREST_MIPMAP_LINEAR:
            case GL_LINEAR_MIPMAP_LINEAR:
                *value = gcvTEXTURE_LINEAR;
                break;
            default:
                return GL_FALSE;
        }
    } else {
        *value = gcvTEXTURE_ANISOTROPIC;
    }
    return GL_TRUE;
}

__GL_INLINE GLboolean setWrapMode(GLenum mode, GLuint * wrapMode)
{
    switch (mode) {
        case GL_REPEAT:
            *wrapMode = gcvTEXTURE_WRAP;
            break;
        case GL_CLAMP_TO_EDGE:
        case GL_CLAMP:
            *wrapMode = gcvTEXTURE_CLAMP;
            break;
        case GL_CLAMP_TO_BORDER:
            *wrapMode = gcvTEXTURE_BORDER;
            break;
        case GL_MIRRORED_REPEAT:
            *wrapMode = gcvTEXTURE_MIRROR;
            break;
        default:
            return GL_FALSE;
    }
    return GL_TRUE;
}

__GL_INLINE GLvoid  updateTextureParameter(__GLcontext *gc, __GLtextureObject *tex, GLuint unit, GLuint64 localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * texInfo =NULL;
    gcsTEXTURE * halTexture;
    GLuint value;

    static gceTEXTURE_FILTER halMagFilter[] =
    {
        0,
        GL_NEAREST,         /* gcvTEXTURE_POINT  */
        GL_LINEAR,          /* gcvTEXTURE_LINEAR */
    };

    if (tex==NULL) return;

    texInfo= (glsTEXTUREINFO *)tex->privateData;

    if (!texInfo) {
        return;
    }

    halTexture = &chipCtx->texture.halTexture[unit];

    if(localMask & __GL_TEXPARAM_WRAP_S_BIT)
    {
        if (setWrapMode(tex->params.sWrapMode, &value)) {
            glmSETTEXPARAMETER(s, value);
        }
    }

    if(localMask & __GL_TEXPARAM_WRAP_T_BIT)
    {
        if (setWrapMode(tex->params.tWrapMode, &value)) {
            glmSETTEXPARAMETER(t, value);
        }
    }

    if(localMask & __GL_TEXPARAM_WRAP_R_BIT)
    {
        if (setWrapMode(tex->params.rWrapMode, &value)) {
            glmSETTEXPARAMETER(r, value);
        }
    }

    /*
    ** HW behavior does not match ES30 Spec requirements:
    ** Spec required:
    ** 1. Calc LAMBDA.base according to primitive shape.
    ** 2. (BIAS.shader + BIAS.texunit + BIAS.texobj) is clamp to [-BIAS.max, BIAS.max], then add to LAMBDA.base to get LAMBDA.skim
    ** 3. LAMBDA.skim is clamp to [LOD.min, LOD.max] to get LAMBDA.real
    ** 4. LAMBDA.real will be added with LEVEL.base, 2 integer level d1 and d2 will be gotten based on the sum;
    ** 5. both d1 and d2 will be clamp to the range [LEVEL.base, min(LEVEL.max, LEVEL.maxused)]
    **
    ** What HW did now is:
    ** 1. Calc LAMBDA.base according to primitive shape.
    ** 2. BIAS.shader is added to LAMBDA.base to get LAMBDA.real.
    ** 3. LAMBDA.real is added with LEVEL.base, 2 S5.5 level lod0 and lod1 will be gotten based on the sum;
    ** 3. both lod0 and lod1 will be clamp to the range [LOD.min, LOD.max]
    **
    ** Here we just combine some calc in driver to maximum match HW, but still some cases may fail.
    */
    if (localMask & (__GL_TEXPARAM_MIN_LOD_BIT | __GL_TEXPARAM_BASE_LEVEL_BIT))
    {
        GLfloat minLod = tex->params.minLod + (GLfloat)tex->params.baseLevel;
        glmSETTEXPARAMETER(lodMin, minLod);
    }

    if (localMask & (__GL_TEXPARAM_MAX_LOD_BIT | __GL_TEXPARAM_BASE_LEVEL_BIT | __GL_TEXPARAM_MAX_LEVEL_BIT))
    {
        GLfloat maxLod = tex->params.maxLod + (GLfloat)tex->params.baseLevel;
        maxLod = __GL_MIN(gc->constants.maxNumTextureLevels - 1, maxLod);
        maxLod = __GL_MIN(tex->params.maxLevel, maxLod);
        maxLod = __GL_MIN(tex->maxLevelUsed, maxLod);
        glmSETTEXPARAMETER(lodMax, maxLod);
        glmSETTEXPARAMETER(maxLevel, (gctINT32)maxLod);
    }

    if (localMask & __GL_TEXPARAM_BASE_LEVEL_BIT)
    {
        GLfloat lBias = tex->params.lodBias + gc->state.texture.texUnits[unit].lodBias + tex->params.baseLevel;
        halTexture->lodBias = (gctFLOAT)tex->params.baseLevel;
        glmSETTEXPARAMETER(lodBias, lBias);
    }

    if(localMask & (__GL_TEXPARAM_MAX_ANISOTROPY_BIT | __GL_TEXPARAM_MIN_FILTER_BIT | __GL_TEXPARAM_MAG_FILTER_BIT) )
    {
        GLint anisotropicLimit;

        anisotropicLimit = (GLint)tex->params.anisotropicLimit;

        if (localMask & (__GL_TEXPARAM_MIN_FILTER_BIT | __GL_TEXPARAM_MAX_ANISOTROPY_BIT) )
        {
            if (setMinFilter(tex->params.minFilter, &value, anisotropicLimit)) {
                glmSETTEXPARAMETER(minFilter, value);
            }

            if (setMinFilter(tex->params.minFilter, &value, -1)) {
              glmSETTEXPARAMETER(mipFilter, value);
            }
        }

        if (localMask & __GL_TEXPARAM_MAG_FILTER_BIT)
        {
            if (glfConvertGLEnum(
                (GLenum *)&halMagFilter[0],
                gcmCOUNTOF(halMagFilter),
                &tex->params.magFilter, glvINT,
                &value
                ) != GL_FALSE)
            {
                glmSETTEXPARAMETER(magFilter, value);
            }
        }

        if (localMask & __GL_TEXPARAM_MAX_ANISOTROPY_BIT)
        {
            glmSETTEXPARAMETER(anisoFilter, glmFLOAT2INT(*(GLfloat *)&tex->params.anisotropicLimit));
        }
    }

    /* Hardware does not depth texture */
    if(localMask & __GL_TEXPARAM_COMPARE_MODE_BIT)
    {
        switch(tex->params.compareMode)
        {
            case GL_NONE:
                break;
            case GL_COMPARE_R_TO_TEXTURE:
                break;
        }
    }

    if(localMask & __GL_TEXPARAM_COMPARE_FUNC_BIT)
    {
    }

    if(localMask & __GL_TEXPARAM_COMPARE_FAIL_VALUE_BIT)
    {
    }

    if ((chipCtx->hasTxDescriptor) &&
        (localMask & (__GL_TEXPARAM_BASE_LEVEL_BIT | __GL_TEXPARAM_MAX_LEVEL_BIT)))
    {
        gcoTEXTURE_SetDescDirty(texInfo->object);
    }
}

__GL_INLINE GLvoid updateTextureBorderColor(__GLcontext *gc,  __GLtextureObject *tex, GLuint unit)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * texInfo =NULL;
    gcsTEXTURE * halTexture;

    if (tex==NULL) return;

    texInfo= (glsTEXTUREINFO *)tex->privateData;

    if (!texInfo) return;

    halTexture = &chipCtx->texture.halTexture[unit];

    glmSETTEXPARAMETER(border[0], (gctUINT8)__GL_FLOORF(tex->params.borderColor.b * 255.0 + 0.5));
    glmSETTEXPARAMETER(border[1], (gctUINT8)__GL_FLOORF(tex->params.borderColor.g * 255.0 + 0.5));
    glmSETTEXPARAMETER(border[2], (gctUINT8)__GL_FLOORF(tex->params.borderColor.r * 255.0 + 0.5));
    glmSETTEXPARAMETER(border[3], (gctUINT8)__GL_FLOORF(tex->params.borderColor.a * 255.0 + 0.5));

    if (chipCtx->hasTxDescriptor)
    {
        gcoTEXTURE_SetDescDirty(texInfo->object);
    }
}

gceSTATUS setViewport(glsCHIPCONTEXT_PTR chipCtx,
    gctINT32 left,
    gctINT32 top,
    gctINT32 right,
    gctINT32 bottom)
{
    gceSTATUS status;

    /* Set viewport. */
    status = gco3D_SetViewport(
        chipCtx->hw, left, bottom, right, top
        );

    return status;
}

gceSTATUS setScissor(glsCHIPCONTEXT_PTR chipCtx,
    gctINT32 left,
    gctINT32 top,
    gctINT32 right,
    gctINT32 bottom)
{
    gceSTATUS status;

    /* Set scissor. */
    status = gco3D_SetScissors(chipCtx->hw, left, top, right, bottom);

    return status;
}

GLenum setLogicOp(glsCHIPCONTEXT_PTR chipCtx, GLenum opCode, GLboolean enable)
{
    GLenum result = GL_NO_ERROR;

    static gctUINT8 ropTable[] =
    {
        /* GL_CLEAR         */ 0x00,
        /* GL_AND           */ 0x88,
        /* GL_AND_REVERSE   */ 0x44,
        /* GL_COPY          */ 0xCC,
        /* GL_AND_INVERTED  */ 0x22,
        /* GL_NOOP          */ 0xAA,
        /* GL_XOR           */ 0x66,
        /* GL_OR            */ 0xEE,
        /* GL_NOR           */ 0x11,
        /* GL_EQUIV         */ 0x99,
        /* GL_INVERT        */ 0x55,
        /* GL_OR_REVERSE    */ 0xDD,
        /* GL_COPY_INVERTED */ 0x33,
        /* GL_OR_INVERTED   */ 0xBB,
        /* GL_NAND          */ 0x77,
        /* GL_SET           */ 0xFF,
    };

    /* Determine whether the operation should be performed. */
    /* Set ROP code in the hardware if supported. */
    if (chipCtx->hwLogicOp)
    {
        gctUINT8 rop = ropTable[opCode - GL_CLEAR];
        /* Determine the proper ROP2. */
        gctUINT8 rop2 = enable
            ? (rop & 0xF)
            : 0xC;

        /* Disable software logicOp. */
        chipCtx->logicOp.perform = GL_FALSE;

        /* Set ROP2. */
        result = glmTRANSLATEHALSTATUS(gco3D_SetLogicOp(chipCtx->hw, rop2));
    }
    else
    {
        /* Determine whether the software logicOp should be performed. */
        chipCtx->logicOp.perform
            =   enable
            && (opCode != GL_COPY);
    }

    return result;
}

GLenum setColorMask(
    glsCHIPCONTEXT_PTR chipCtx,
    GLboolean Red,
    GLboolean Green,
    GLboolean Blue,
    GLboolean Alpha
    )
{
    gctUINT8 enable
        =  (gctUINT8) Red
        | ((gctUINT8) Green << 1)
        | ((gctUINT8) Blue  << 2)
        | ((gctUINT8) Alpha << 3);
    GLenum result;

    result = glmTRANSLATEHALSTATUS(gco3D_SetColorWrite(chipCtx->hw, enable));

    return result;
}

GLenum setClearColor(
    glsCHIPCONTEXT_PTR chipCtx,
    GLvoid* ClearColor,
    gleTYPE Type
    )
{
    gltFRACTYPE clearColor[4];
    glsVECTOR   color;

    GLenum result;

    /* Set with [0..1] clamping. */
    glfSetClampedVector4(&color, ClearColor, Type);

    /* Query back the color. */
    glfGetFromVector4(
        &color,
        clearColor,
        glvFRACTYPEENUM
        );

    /* Set color value. */
    result = glmTRANSLATEHALSTATUS(gco3D_SetClearColorFrac(
        chipCtx->hw,
        clearColor[0],
        clearColor[1],
        clearColor[2],
        clearColor[3]
        ));

    return result;
}


GLenum setEnableDither(
    glsCHIPCONTEXT_PTR chipCtx,
    GLboolean Enable
    )
{
    GLenum status;
    status = glmTRANSLATEHALSTATUS(gco3D_EnableDither(chipCtx->hw, Enable));
    return status;
}


GLenum setEnableMultisampling(
    glsCHIPCONTEXT_PTR chipCtx,
    GLboolean Enable
    )
{
    GLenum status;
    status = glmTRANSLATEHALSTATUS(gco3D_SetAntiAlias(chipCtx->hw, Enable));
    return status;
}

/********************************************************************
**
**  setDrawBuffers
**
**  Set render target/depth/stencil surf into chipCtx.
**  Set chipDirty according to draw buffers change.
**
**  Parameters:      Describe the calling sequence
**  Return Value:    Describe the returning sequence
**
********************************************************************/
GLvoid setDrawBuffers(glsCHIPCONTEXT_PTR chipCtx,
                      GLboolean          invertY,
                      GLboolean          integerRT,
                      GLboolean          floatRT,
                      gcoSURF*           rtSurfs,
                      gcoSURF            dSurf,
                      gcoSURF            sSurf)
{
    gcoSURF firstRTSurf = NULL;
    GLuint  i;

    /*
    **  Set yInvert for drawing
    */
    if (chipCtx->drawInvertY != invertY)
    {
        chipCtx->drawInvertY = invertY;
    }

    /*
    **  Set Integer for drawing
    */
    if (chipCtx->drawInteger != integerRT)
    {
        chipCtx->drawInteger = integerRT;
        /* Dirty sample alpha to coverage, Atest and blending for Integer framebufferOBJ */
    }

    /*
    ** Set float for drawing
    */
    if (chipCtx->drawFloat ^ floatRT)
    {
        chipCtx->drawFloat = (GLboolean)floatRT;
    }

    /*
    **  Set correct render target for drawing
    */
    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        if (chipCtx->drawRT[i] != rtSurfs[i])
        {
            chipCtx->drawRT[i]  = rtSurfs[i];
            /* consider to set up MRT later */
        }

        if (!firstRTSurf && rtSurfs[i])
        {
            firstRTSurf = rtSurfs[i];
        }
    }

    /* Set surfaces into hardware context. */
    gco3D_SetTarget(chipCtx->hw, 0, firstRTSurf, 0, 0);
    if (firstRTSurf) {
        gcoSURF_GetSize(firstRTSurf, &chipCtx->drawRTWidth, &chipCtx->drawRTHeight, gcvNULL);
    }
    gco3D_SetDepth(chipCtx->hw, dSurf, 0);
    gco3D_SetColorCacheMode(chipCtx->hw);
    chipCtx->drawDepth = dSurf;
    /* Since depth and stencil always share the same surface, so we don't need to set stencil surface.*/
    chipCtx->drawStencil = sSurf;
    /* reset current program if render target is changed */
    chipCtx->currProgram = 0;
}


GLvoid setReadBuffers(glsCHIPCONTEXT_PTR chipCtx,
                      GLboolean          invertY,
                      GLboolean          integerRT,
                      gcoSURF            rtSurf,
                      gcoSURF            dSurf,
                      gcoSURF            sSurf)
{
    GLuint          rtWidth = 0;
    GLuint          rtHeight = 0;

    chipCtx->readInvertY = invertY;
    chipCtx->readInteger = integerRT;
    chipCtx->readRT      = rtSurf;
    chipCtx->readDepth   = dSurf;
    chipCtx->readStencil = sSurf;
    if (rtSurf) {
        gcoSURF_GetSize(rtSurf, &rtWidth, &rtHeight, gcvNULL);
    }

    chipCtx->readRTWidth  = rtWidth;
    chipCtx->readRTHeight = rtHeight;
    chipCtx->readRTMsaaMode = 0;
}

#define CHIP_UPDATEVIEWPORT() do { \
        left    = pViewport->x; \
        right   = pViewport->x + pViewport->width; \
        if(chipCtx->drawInvertY) \
        { \
            top     = rtHeight - pViewport->y - pViewport->height; \
            bottom  = __GL_MAX(top, rtHeight - pViewport->y); \
        } \
        else \
        { \
            top     = pViewport->y; \
            bottom  = __GL_MAX(top, pViewport->y + pViewport->height); \
        } \
    } while(0)


GLvoid validateViewport(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLviewport    *pViewport = &(gc->state.viewport);

    /* Validate viewport */
    GLint left, right, top, bottom;

    GLint rtHeight = (GLint)(chipCtx->drawRTHeight);

    CHIP_UPDATEVIEWPORT();

    {
        GLint dx = left, dy = top, sx = 0, sy = 0, w = pViewport->width, h = pViewport->height;
        if (calculateArea(
            &dx, &dy, &sx, &sy, &w, &h,
            chipCtx->drawRTWidth, chipCtx->drawRTHeight,
            pViewport->width, pViewport->height))
        {
            setViewport(chipCtx, left, top, right, bottom);
        }
        else
        {
            setViewport(chipCtx, 0, 0, 0, 0);
        }
    }
}


GLvoid validateScissor(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLscissor     *pScissor = &(gc->state.scissor);
    __GLviewport    *pViewport = &(gc->state.viewport);
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];
    GLint left, right, top, bottom;
    GLint rtWidth = (GLint)(chipCtx->drawRTWidth);
    GLint rtHeight = (GLint)(chipCtx->drawRTHeight);

    if(gc->state.enables.scissor)
    {
        left    = __GL_MAX(0, pScissor->scissorX);
        right   = __GL_MAX(left, pScissor->scissorX + pScissor->scissorWidth);

        if(chipCtx->drawInvertY)
        {
            top     = __GL_MAX(0, rtHeight - pScissor->scissorY - pScissor->scissorHeight);
            bottom  = __GL_MAX(top,  rtHeight - pScissor->scissorY);
        }
        else
        {
            top     = __GL_MAX(0, pScissor->scissorY);
            bottom  = __GL_MAX(top, pScissor->scissorY + pScissor->scissorHeight);
        }
    }
    else
    {
        /* when scissor is not enabled and viewport is enabled, we should calculate the default scissor as validateviewport does */
        if (localMask|__GL_VIEWPORT_BIT)
        {
            CHIP_UPDATEVIEWPORT();
        }
        else
        {
            left    = 0;
            right   = rtWidth;
            top     = 0;
            bottom  = rtHeight;
        }
    }

    {
        GLint dx = left, dy = top, sx = 0, sy = 0, w = right - left, h = bottom - top;
        if (calculateArea(
            &dx, &dy, &sx, &sy, &w, &h,
            chipCtx->drawRTWidth, chipCtx->drawRTHeight,
           right - left, bottom - top))
        {
            setScissor(chipCtx, dx, dy, dx+w, dy+h);
        }
        else
        {
            setScissor(chipCtx, 0, 0, 0, 0);
        }
    }
}

GLvoid validateStencil(__GLcontext *gc, GLbitfield localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    /* we need to implement ARB and EXT for stencil later */
    if (localMask & __GL_STENCILTEST_ENDISABLE_BIT) {
        if (!gc->state.enables.stencilTest) {
            gco3D_SetStencilMode(chipCtx->hw, gcvSTENCIL_SINGLE_SIDED);

            setStencilCompareFunction(chipCtx, GL_ALWAYS,
                gc->state.stencil.current.front.reference,
                gc->state.stencil.current.front.mask, GL_FRONT);

            setStencilCompareFunction(chipCtx, GL_ALWAYS,
                gc->state.stencil.current.back.reference,
                gc->state.stencil.current.back.mask, GL_BACK);

            setStencilOperations(chipCtx,
                gc->state.stencil.current.front.fail,
                GL_KEEP,
                GL_KEEP, GL_FRONT);
            setStencilOperations(chipCtx,
                gc->state.stencil.current.back.fail,
                GL_KEEP,
                GL_KEEP, GL_BACK);
            gco3D_SetStencilMode(chipCtx->hw, gcvSTENCIL_SINGLE_SIDED);
        } else {
            setStencilOperations(chipCtx,
                gc->state.stencil.current.front.fail,
                gc->state.stencil.current.front.depthFail,
                gc->state.stencil.current.front.depthPass, GL_FRONT);
            setStencilOperations(chipCtx,
                gc->state.stencil.current.back.fail,
                gc->state.stencil.current.back.depthFail,
                gc->state.stencil.current.back.depthPass, GL_BACK);
            setStencilCompareFunction(chipCtx, gc->state.stencil.current.front.testFunc,
                gc->state.stencil.current.front.reference,
                gc->state.stencil.current.front.mask, GL_FRONT);
            setStencilCompareFunction(chipCtx, gc->state.stencil.current.back.testFunc,
                gc->state.stencil.current.back.reference,
                gc->state.stencil.current.back.mask, GL_BACK);
            setWriteMask(chipCtx, gc->state.stencil.current.front.writeMask);
            setWriteMask(chipCtx, gc->state.stencil.current.back.writeMask);
            setClearStencil(chipCtx, gc->state.stencil.clear);
        }
    }

    if (gc->state.enables.stencilTest) {
        /* set front face stencil information */
        if (localMask & __GL_STENCILOP_FRONT_BIT) {
            setStencilOperations(chipCtx,
                gc->state.stencil.current.front.fail,
                gc->state.stencil.current.front.depthFail,
                gc->state.stencil.current.front.depthPass, GL_FRONT);
        }
        if (localMask & __GL_CLEARSTENCIL_BIT) {
            setClearStencil(chipCtx,
                gc->state.stencil.clear);
        }
        if (localMask &  __GL_STENCILFUNC_FRONT_BIT) {
            setStencilCompareFunction(chipCtx, gc->state.stencil.current.front.testFunc,
                gc->state.stencil.current.front.reference,
                gc->state.stencil.current.front.mask, GL_FRONT);
        }
        if (localMask &  __GL_STENCILMASK_FRONT_BIT) {
            setWriteMask(chipCtx, gc->state.stencil.current.front.writeMask);
        }
        /* set back face stencil information */
        if (localMask & __GL_STENCILOP_BACK_BIT) {
            setStencilOperations(chipCtx,
                gc->state.stencil.current.back.fail,
                gc->state.stencil.current.back.depthFail,
                gc->state.stencil.current.back.depthPass, GL_BACK);
        }
        if (localMask &  __GL_STENCILFUNC_BACK_BIT) {
            setStencilCompareFunction(chipCtx, gc->state.stencil.current.back.testFunc,
                gc->state.stencil.current.back.reference,
                gc->state.stencil.current.back.mask, GL_FRONT);
        }
        if (localMask &  __GL_STENCILMASK_BACK_BIT) {
            setWriteMask(chipCtx, gc->state.stencil.current.back.writeMask);
        }
        if (localMask & __GL_STENCIL_ATTR_BITS) {
            gco3D_SetStencilMode(chipCtx->hw,
                gc->state.enables.stencilTestTwoSideExt ? gcvSTENCIL_DOUBLE_SIDED : gcvSTENCIL_SINGLE_SIDED);
        }
    }
}
extern GLenum updateDepthEnableAndRange(__GLcontext *gc);
extern GLenum setDepthCompareFunction(glsCHIPCONTEXT_PTR chipCtx, GLenum testFunction, GLboolean testEnable);
GLvoid validateDepth(__GLcontext *gc, GLbitfield localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (localMask & (__GL_DEPTHTEST_ENDISABLE_BIT | __GL_DEPTHRANGE_BIT)) {
        updateDepthEnableAndRange(gc);
        setDepthCompareFunction(chipCtx, gc->state.depth.testFunc, gc->state.enables.depthBuffer.test);
        if (!gc->state.enables.depthBuffer.test) {
            setDepthMask(chipCtx, GL_FALSE);
        }
        else {
            setDepthMask(chipCtx, gc->state.depth.writeEnable);
        }
        setClearDepth(chipCtx, gc->state.depth.clear);
    } else {

        if (localMask & __GL_DEPTHFUNC_BIT) {
            setDepthCompareFunction(chipCtx, gc->state.depth.testFunc, gc->state.enables.depthBuffer.test);
        }

        if (localMask & __GL_DEPTHMASK_BIT) {
            setDepthMask(chipCtx, gc->state.depth.writeEnable);
        }

        if (localMask & __GL_CLEARDEPTH_BIT) {
            setClearDepth(chipCtx, gc->state.depth.clear);
        }
    }
}

GLvoid  validateClearColor(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLfloat clearColor[4];

    /* Program for each RT if HW support different clearvalue for different RT */
    clearColor[0] = gc->state.raster.clear.r;
    clearColor[1] = gc->state.raster.clear.g;
    clearColor[2] = gc->state.raster.clear.b;
    clearColor[3] = gc->state.raster.clear.a;
    setClearColor(chipCtx, clearColor, glvFLOAT);
}
extern GLenum setBlendEquationSeparate(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum modeRGB,
    GLenum modeAlpha
    );
extern GLenum setBlendFuncSeparate(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum SrcRGB,
    GLenum DstRGB,
    GLenum SrcAlpha,
    GLenum DstAlpha
    );
GLvoid validateAlphaBlend(__GLcontext *gc, GLbitfield localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (localMask & __GL_ALPHATEST_ENDISABLE_BIT) {
        gco3D_SetAlphaTest(chipCtx->hw, gc->state.enables.colorBuffer.alphaTest);
    }

    if (localMask & __GL_BLEND_ENDISABLE_BIT) {
        /* Consider MRTs later */
        gco3D_EnableBlending(chipCtx->hw, gc->state.enables.colorBuffer.blend[0]);
    }

    if (localMask & __GL_ALPHAFUNC_BIT) {
        setAlphaTestReference(chipCtx, gc->state.raster.alphaFunction, gc->state.raster.alphaReference);
    }

    if (localMask & __GL_BLENDCOLOR_BIT) {
    }

    if (localMask & __GL_BLENDEQUATION_BIT) {
        setBlendEquationSeparate(chipCtx, gc->state.raster.blendEquationRGB, gc->state.raster.blendEquationAlpha);
    }

    if (localMask & __GL_BLENDFUNC_BIT) {
        setBlendFuncSeparate(chipCtx,
            gc->state.raster.blendSrcRGB,
            gc->state.raster.blendDstRGB,
            gc->state.raster.blendSrcAlpha,
            gc->state.raster.blendDstAlpha);
    }
}

GLvoid validateLineState(__GLcontext *gc, GLbitfield localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (localMask & __GL_LINESMOOTH_ENDISABLE_BIT) {
        gco3D_SetAntiAliasLine(chipCtx->hw, gc->state.enables.line.smooth);
    }

    if (localMask & (__GL_LINEWIDTH_BIT | __GL_LINESMOOTH_ENDISABLE_BIT)) {
        /* looks like HW does not support normal wide line, so turned on aaline if line width is greater than 1 */
        if (gc->state.line.aliasedWidth > 1) {
            gco3D_SetAntiAliasLine(chipCtx->hw, GL_TRUE);
        } else {
            if (!gc->state.enables.line.smooth) {
                gco3D_SetAntiAliasLine(chipCtx->hw, GL_FALSE);
            }
        }
        gco3D_SetAALineWidth(chipCtx->hw, (GLfloat)gc->state.line.aliasedWidth);
    }

    if (localMask & __GL_LINESTIPPLE_BIT) {
        loadLineStippleImage(gc, chipCtx);
    }

    if (localMask & (__GL_LINESTIPPLE_ENDISABLE_BIT | __GL_PRIMMODE_BIT)) {
        if ((gc->state.enables.line.stippleRequested) &&
            (gc->vertexStreams.primMode == GL_LINES || gc->vertexStreams.primMode == GL_LINE_LOOP || gc->vertexStreams.primMode == GL_LINE_STRIP) &&
            !chipCtx->isSolidLineStipple)
        {
            glmSETHASH_1BIT(hasLineStippleEnabled, 1, 0);
        } else {
            glmSETHASH_1BIT(hasLineStippleEnabled, 0, 0);
        }
    }
}

GLvoid validatePointState(__GLcontext *gc, GLbitfield localMask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (localMask & __GL_POINTSIZE_BIT) {
    }

    if (localMask & __GL_POINTSMOOTH_ENDISABLE_BIT) {
        chipCtx->pointStates.smooth=1;
    }
}

GLvoid updatePrimitive(__GLcontext *gc)
{
    GLboolean ptSizeEnable;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    /* Invalidate point sprite state. */
    chipCtx->pointStates.spriteDirty = GL_TRUE;

    switch (gc->vertexStreams.primMode) {
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_QUADS:
        case GL_QUAD_STRIP:
        case GL_POLYGON:
            chipCtx->hashKey.hashTwoSidedLighting =
            chipCtx->lightingStates.doTwoSidedlighting = gc->state.light.model.twoSided;
            break;
        default:
            break;
    }

    ptSizeEnable = (gc->vertexStreams.primMode == GL_POINTS) ? GL_TRUE
                                       : GL_FALSE;

    /* Update point states. */
    chipCtx->hashKey.hashPointPrimitive = ptSizeEnable;

    if (chipCtx->pointStates.pointPrimitive != ptSizeEnable)
    {
        chipCtx->pointStates.pointPrimitive = ptSizeEnable;

        /* Program point size. */
        gco3D_SetPointSizeEnable(chipCtx->hw,
                                          chipCtx->pointStates.pointPrimitive);
    }

    return;
}

GLvoid updateColorSum(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (gc->state.enables.colorSum)
    {
        glmSETHASH_1BIT(hasColorSum, 1, 0);
    }
    else
    {
        glmSETHASH_1BIT(hasColorSum, 1, 0);
    }

    if (gc->state.enables.lighting.lighting &&
        !gc->state.enables.program.vertexProgram &&
        !gc->shaderProgram.vertShaderEnable &&
        gc->state.light.model.colorControl == GL_SEPARATE_SPECULAR_COLOR)
    {
        glmSETHASH_1BIT(hasColorSum, 1, 0);
    }

    if ((!gc->state.enables.lighting.lighting) &&
        gc->state.enables.colorSum)
    {
        glmSETHASH_1BIT(hasSecondaryColorOutput, 1, 0);
    }
    else
    {
        glmSETHASH_1BIT(hasSecondaryColorOutput, 0, 0);
    }

    return;
}

extern GLenum setCulling(__GLcontext *gc);
extern GLenum validatePolygonOffset(__GLcontext *gc);

GLvoid validateState(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    if(gc->globalDirtyState[__GL_DIRTY_ATTRS_1])
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

        if(localMask & __GL_VIEWPORT_BIT)
        {
            validateViewport(gc);
        }

        if(localMask & (__GL_SCISSOR_BIT | __GL_SCISSORTEST_ENDISABLE_BIT | __GL_VIEWPORT_BIT))
        {
            validateScissor(gc);
        }

        /* Color Mask */
        if (localMask & __GL_COLORMASK_BIT)
        {
            setColorMask(chipCtx,
                gc->state.raster.colorMask[0].redMask,
                gc->state.raster.colorMask[0].greenMask,
                gc->state.raster.colorMask[0].blueMask,
                gc->state.raster.colorMask[0].alphaMask);
        }

        if (localMask & __GL_CLEARCOLOR_BIT)
        {
            validateClearColor(gc);
        }

        if (localMask & __GL_STENCILOP_BITS)
        {
            validateStencil(gc, localMask);
            /* If stencil is enabled/disable, depth enable/disable state should be updated, too. */
            if (localMask & __GL_STENCILTEST_ENDISABLE_BIT)
            {
                localMask |= __GL_DEPTHTEST_ENDISABLE_BIT;
            }
        }

        if (localMask & (__GL_DEPTH_BITS))
        {
            validateDepth(gc, localMask);
        }

        if (localMask & (__GL_LOGICOP_BIT | __GL_LOGICOP_ENDISABLE_BIT)) {
            setLogicOp(chipCtx, gc->state.raster.logicOp, gc->state.enables.colorBuffer.colorLogicOp);
        }

        if (localMask & __GL_DITHER_ENDISABLE_BIT) {
            gco3D_EnableDither(chipCtx->hw, gc->state.enables.colorBuffer.dither);
        }

        if (localMask & __GL_ALPHABLEND_BITS) {
            validateAlphaBlend(gc, localMask);
        }

        if (localMask & __GL_CLEARACCUM_BIT) {
        }

        if (localMask & (__GL_DEPTHBOUNDTEST_BIT | __GL_DEPTHBOUNDTESTENABLE_BIT)) {
        }

        if (localMask & __GL_CLAMP_VERTEX_COLOR_BIT) {
        }

        if (localMask & __GL_CLAMP_FRAG_COLOR_BIT) {
        }

        if (localMask & __GL_DEPTHBOUNDTESTENABLE_BIT)
        {
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if(gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_ATTR2_MASK)
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

        /* set fog key for shaders */
        if (localMask & (__GL_FOGMODE_BIT | __GL_FOG_ENDISABLE_BIT) ) {
            if (localMask & __GL_FOGMODE_BIT) {
                GLenum mode = glvLINEARFOG;
                switch (gc->state.fog.mode) {
                    case GL_LINEAR:
                        mode = glvLINEARFOG;
                        chipCtx->hashKey.hashFogMode = glvLINEARFOG;
                        break;
                    case GL_EXP:
                        mode = glvEXPFOG;
                        chipCtx->hashKey.hashFogMode = glvEXPFOG;
                        break;
                    case GL_EXP2:
                        mode = glvEXP2FOG;
                        break;
                }
                glmSETHASH_2BITS(hashFogMode,
                    mode,
                    0);
            }
            if (localMask & __GL_FOG_ENDISABLE_BIT) {
                glmSETHASH_1BIT(hashFogEnabled,
                    gc->state.enables.fog,
                    0);
            }
        }

        if(localMask & (__GL_POINTSIZE_BIT | __GL_POINTSMOOTH_ENDISABLE_BIT |
            __GL_POINT_SIZE_MIN_BIT | __GL_POINT_SIZE_MAX_BIT)) {
            validatePointState(gc, localMask);
        }

        if (localMask & (__GL_FRONTFACE_BIT | __GL_CULLFACE_BIT | __GL_CULLFACE_ENDISABLE_BIT)) {
            setCulling(gc);
        }

        if (localMask & __GL_POLYGONMODE_BIT) {
            /* HW only supports both faces have the same mode, so use front face */
            gco3D_SetFill(
                chipCtx->hw,
                gc->state.polygon.frontMode - GL_POINT
                );
        }

        if (localMask & (__GL_POLYGONOFFSET_FILL_ENDISABLE_BIT | __GL_POLYGONOFFSET_BIT)) {
            validatePolygonOffset(gc);
        }

        if (localMask & (__GL_LINE_ATTR_BITS | __GL_PRIMMODE_BIT)) {
            validateLineState(gc, localMask);
        }

        if (localMask & __GL_PRIMMODE_BIT) {
            updatePrimitive(gc);
        }

        if(localMask & __GL_POLYGONSTIPPLE_BIT) {
            loadPolygonStippleImage(gc, chipCtx);
        }

        if(localMask & (__GL_POLYGONSTIPPLE_ENDISABLE_BIT | __GL_POLYGONSTIPPLE_BIT | __GL_PRIMMODE_BIT | __GL_POLYGONMODE_BIT)) {
            if ((gc->state.enables.polygon.stipple) && isFillRendering(gc) && !chipCtx->isSolidPolygonStipple) {
                glmSETHASH_1BIT(hasPolygonStippleEnabled, 1, 0);
            } else {
                glmSETHASH_1BIT(hasPolygonStippleEnabled, 0, 0);
            }
        }

        if (localMask & (__GL_POINTSMOOTH_ENDISABLE_BIT | __GL_POINTSPRITE_ENDISABLE_BIT | __GL_POLYGONMODE_BIT | __GL_PRIMMODE_BIT))
        {
            if (localMask & __GL_POINTSMOOTH_ENDISABLE_BIT) {
                glmSETHASH_1BIT(hashPointSmoothEnabled,
                    gc->state.enables.pointSmooth,
                    0);
            }

            if (localMask & __GL_POINTSPRITE_ENDISABLE_BIT) {
                glmSETHASH_1BIT(hashPointSpriteEnabled,
                    gc->state.enables.pointSprite,
                    0);
            }

            if(localMask & __GL_PRIMMODE_BIT) {
                glmSETHASH_1BIT(hashPointPrimitive,
                    (gc->vertexStreams.primMode == GL_POINTS),
                    0);
            }
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if(gc->globalDirtyState[__GL_DIRTY_ATTRS_3] & __GL_ATTR3_MASK)
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_3];

        if(localMask & (__GL_PROJECTION_TRANSFORM_BIT | __GL_MODELVIEW_TRANSFORM_BIT)) {
            glmSETHASH_1BIT(hashProjectionIdentity,
                (gc->transform.projection->matrix.matrixType == __GL_MT_IDENTITY),
                0);
            glmSETHASH_1BIT(hashModelViewIdentity,
                (gc->transform.modelView->matrix.matrixType == __GL_MT_IDENTITY),
                0);
            glmSETHASH_1BIT(hashModelViewProjectionIdentity,
                (gc->transform.modelView->mvp.matrixType == __GL_MT_IDENTITY),
                0);
            glmSETHASH_1BIT(hashModelViewProjectionIdentity,
                (gc->transform.modelView->inverseTranspose.matrixType == __GL_MT_IDENTITY),
                0);
            if (gc->transform.modelView->updateInverse) {
                (*gc->transform.matrix.invertTranspose)(&gc->transform.modelView->inverseTranspose,
                    &gc->transform.modelView->matrix);
                gc->transform.modelView->updateInverse = GL_FALSE;
            }
        }

        if(localMask & __GL_NORMALIZE_ENDISABLE_BIT) {
            glmSETHASH_1BIT(hashNormalizeNormal, gc->state.enables.transform.normalize, 0);
        }

        if(localMask & __GL_RESCALENORMAL_ENDISABLE_BIT) {
            glmSETHASH_1BIT(hashRescaleNormal, gc->state.enables.transform.rescaleNormal, 0);
        }

        if (localMask & __GL_MULTISAMPLE_ENDISABLE_BIT) {
            glmSETHASH_1BIT(hashMultisampleEnabled, gc->state.enables.multisample.multisampleOn, 0);
        }

        if (localMask & __GL_COLORSUM_ENDISABLE_BIT) {
            updateColorSum(gc);
            glmSETHASH_1BIT(hasColorSum, gc->state.enables.colorSum, 0);
        }

        if(localMask & __GL_RENDERMODE_BIT)
        {
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if (gc->globalDirtyState[__GL_LIGHTING_ATTRS] & __GL_LIGHTING_MASK)
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_LIGHTING_ATTRS];
        /* We need to consider back face material
        __GLmaterialState * back = &gc->state.light.back; */
        __GLmaterialState * front = &gc->state.light.front;
        __GLmaterialState * back = &gc->state.light.back;

        if (localMask & __GL_LIGHTING_ENDISABLE_BIT) {
            glmSETHASH_1BIT(hashLightingEnabled, gc->state.enables.lighting.lighting, 0);
            updateColorSum(gc);
        } else {
            if (gc->state.enables.lighting.lighting) {
                if (localMask & __GL_LIGHTMODEL_COLORCONTROL_BIT) {
                    updateColorSum(gc);
                }
            }
        }

        if (localMask & __GL_LIGHTMODEL_LOCALVIEWER_BIT) {
            glmSETHASH_1BIT(hasLocalViewer, gc->state.light.model.localViewer, 0);
        }

        if (localMask & __GL_COLORMATERIAL_ENDISABLE_BIT) {
            glmSETHASH_1BIT(hashMaterialEnabled, gc->state.enables.lighting.colorMaterial, 0);
        }
        if (localMask & __GL_LIGHTMODEL_TWOSIDE_BIT) {
            updatePrimitive(gc);
        }
        if (localMask & __GL_SHADEMODEL_BIT) {
            if (gc->state.light.shadingModel == GL_FLAT) {
                gco3D_SetShading(chipCtx->hw, gcvSHADING_FLAT_OPENGL);
            } else {
                gco3D_SetShading(chipCtx->hw, gcvSHADING_SMOOTH);
            }
            __glChipUpdateShadingMode(gc, gc->state.light.shadingModel);
        }

        if (localMask & __GL_MATERIAL_EMISSION_FRONT_BIT) {
            GLboolean zero = (front->emissive.r == 0.0 &&
                              front->emissive.g == 0.0 &&
                              front->emissive.b == 0.0);
            glmSETHASH_1BIT(hashZeroEcm, zero, 0);
        }

        if (localMask & __GL_MATERIAL_EMISSION_BACK_BIT) {
            GLboolean zero = (back->emissive.r == 0.0 &&
                              back->emissive.g == 0.0 &&
                              back->emissive.b == 0.0);
            glmSETHASH_1BIT(hashZeroEcm, zero, 1);
        }


        if (localMask & __GL_MATERIAL_SPECULAR_FRONT_BIT) {
            GLboolean zero = (front->specular.r == 0.0 &&
                              front->specular.g == 0.0 &&
                              front->specular.b == 0.0);

            glmSETHASH_1BIT(hashZeroScm, zero, 0);
        }

        if (localMask & __GL_MATERIAL_SPECULAR_BACK_BIT) {
            GLboolean zero = (back->specular.r == 0.0 &&
                              back->specular.g == 0.0 &&
                              back->specular.b == 0.0);

            glmSETHASH_1BIT(hashZeroScm, zero, 1);
        }

        if (localMask & __GL_MATERIAL_AMBIENT_FRONT_BIT) {
            GLboolean zero = (front->ambient.r == 0.0 &&
                              front->ambient.g == 0.0 &&
                              front->ambient.b == 0.0);

            glmSETHASH_1BIT(hashZeroAcm, zero, 0);
        }

        if (localMask & __GL_MATERIAL_AMBIENT_BACK_BIT) {
            GLboolean zero = (back->ambient.r == 0.0 &&
                              back->ambient.g == 0.0 &&
                              back->ambient.b == 0.0);

            glmSETHASH_1BIT(hashZeroAcm, zero, 1);
        }

        if (localMask & __GL_MATERIAL_DIFFUSE_FRONT_BIT) {
            GLboolean zero = (front->diffuse.r == 0.0 &&
                              front->diffuse.g == 0.0 &&
                              front->diffuse.b == 0.0);

            glmSETHASH_1BIT(hashZeroDcm, zero, 0);
        }

        if (localMask & __GL_MATERIAL_DIFFUSE_BACK_BIT) {
            GLboolean zero = (back->diffuse.r == 0.0 &&
                              back->diffuse.g == 0.0 &&
                              back->diffuse.b == 0.0);

            glmSETHASH_1BIT(hashZeroDcm, zero, 1);
        }

        if (localMask & __GL_MATERIAL_SHININESS_FRONT_BIT) {
            glmSETHASH_1BIT(hashZeroSrm, (front->specularExponent == 0.0f), 0);
        }

        if (localMask & __GL_MATERIAL_SHININESS_BACK_BIT) {
            glmSETHASH_1BIT(hashZeroSrm, (back->specularExponent == 0.0f), 1);
        }

        if (localMask & __GL_LIGHTMODEL_AMBIENT_BIT) {
            GLboolean zero =  (gc->state.light.model.ambient.r == 0.0f &&
                              gc->state.light.model.ambient.g == 0.0f &&
                              gc->state.light.model.ambient.b == 0.0f);
            glmSETHASH_1BIT(hashZeroAcs, zero, 0);
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if (gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS]) {
        GLbitfield localMask = gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];
        GLuint lightIndex = 0;
        while (localMask) {
            GLbitfield lightAttr = gc->lightAttrState[lightIndex];
            __GLlightSourceState * src = &gc->state.light.source[lightIndex];

            if (lightAttr & __GL_LIGHT_ENDISABLE_BIT) {
                glmSETHASH_1BIT(hashLightEnabled,
                    gc->state.enables.lighting.light[lightIndex],
                    lightIndex);
                if (gc->state.enables.lighting.light[lightIndex]) {
                    chipCtx->lightingStates.lightEnabled |= (1 << lightIndex);
                } else {
                    chipCtx->lightingStates.lightEnabled &= ~(1 << lightIndex);
                }
            }

            if (lightAttr & __GL_LIGHT_AMBIENT_BIT) {
                GLboolean zero = (src->ambient.r == 0.0 &&
                                  src->ambient.g == 0.0 &&
                                  src->ambient.b == 0.0);
                glmSETHASH_1BIT(hashZeroAcl, zero, lightIndex);
            }
            if (lightAttr & __GL_LIGHT_DIFFUSE_BIT) {
                GLboolean zero = (src->diffuse.r == 0.0 &&
                                  src->diffuse.g == 0.0 &&
                                  src->diffuse.b == 0.0);
                glmSETHASH_1BIT(hashZeroDcl, zero, lightIndex);
            }
            if (lightAttr & __GL_LIGHT_SPECULAR_BIT) {
                GLboolean zero = (src->specular.r == 0.0 &&
                                  src->specular.g == 0.0 &&
                                  src->specular.b == 0.0);
                glmSETHASH_1BIT(hashZeroScl, zero, lightIndex);
            }
            if (lightAttr & __GL_LIGHT_CONSTANTATT_BIT) {
                glmSETHASH_1BIT(hashOneK0, (src->constantAttenuation == 1.0f), lightIndex);
            }
            if (lightAttr & __GL_LIGHT_LINEARATT_BIT) {
                glmSETHASH_1BIT(hashZeroK1, (src->linearAttenuation == 0.0f), lightIndex);
            }
            if (lightAttr & __GL_LIGHT_QUADRATICATT_BIT) {
                glmSETHASH_1BIT(hashZeroK2, (src->quadraticAttenuation == 0.0f), lightIndex);
            }
            if (lightAttr & __GL_LIGHT_SPOTCUTOFF_BIT) {
                glmSETHASH_1BIT(hashCrl_180, (src->spotLightCutOffAngle == 180.0), lightIndex);
            }
            if (lightAttr & __GL_LIGHT_POSITION_BIT) {
                glmSETHASH_1BIT(hashDirectionalLight, (src->positionEye.w == 0.0), lightIndex);
            }
            localMask >>= 1;
            lightIndex++;
        }
    }

    if (gc->globalDirtyState[__GL_CLIP_ATTRS] & __GL_CLIPPLANE_ENDISABLE_BITS)
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_CLIP_ATTRS] & __GL_CLIPPLANE_ENDISABLE_BITS;
        if (localMask) {
            chipCtx->hashKey.hashClipPlaneEnabled = gc->state.enables.transform.clipPlanesMask;
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if (gc->texUnitAttrDirtyMask)
    {
        GLuint unit = 0;
        GLuint64 unitMask = gc->texUnitAttrDirtyMask;

        while (unitMask)
        {
            GLuint64 localMask;
            __GLtextureObject* tex;
            __GL_BitScanForward64(&unit, unitMask);
            __GL_BitTestAndReset64(&unitMask, unit);

            localMask = gc->texUnitAttrState[unit];
            tex = gc->texture.units[unit].currentTexture;

            if(unit >= gc->constants.numberOfTextureUnits) {
                break;
            }

            if (localMask & (__GL_TEX_ENABLE_DIM_CHANGED_BIT | __GL_TEX_IMAGE_CONTENT_CHANGED_BIT)) {
                if (gc->texture.currentEnableMask & (1 << unit)) {
                    if (tex) {
                        glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)tex->privateData;
                        textureInfo->dirty = GL_TRUE;
                        chipCtx->texture.sampler[unit].binding = tex->privateData;
                        updateStageEnable(chipCtx, &chipCtx->texture.sampler[unit], GL_TRUE);
                    }
                } else {
                    chipCtx->texture.sampler[unit].binding = gcvNULL;
                    updateStageEnable(chipCtx, &chipCtx->texture.sampler[unit], GL_FALSE);
                }
            }

            if (localMask & __GL_TEXTURE_TRANSFORM_BIT)
            {
                glmSETHASH_1BIT(hashTextureIdentity,
                    (gc->transform.texture[unit]->matrix.matrixType == __GL_MT_IDENTITY),
                    0);
            }
/*
            if (!tex) {
                continue;
            }
*/
            if (localMask & __GL_SAMPLER_BITS) {
                gctUINT8 hashTextureWrapBit = 0;

                updateTextureParameter(gc, tex, unit, localMask);

                /* simulate the wrap mode GL_CLAMP in LINEAR */
                if (tex && (tex->params.minFilter == GL_LINEAR) &&
                    (tex->params.magFilter == GL_LINEAR))
                {
                    if (tex->params.sWrapMode == GL_CLAMP)
                    {
                        hashTextureWrapBit = 0x1;
                    }

                    if (tex->params.tWrapMode == GL_CLAMP)
                    {
                        hashTextureWrapBit |= (0x1 << 1);
                    }
                }

                glmSETHASH_2BITS(hashTextureWrap, hashTextureWrapBit, unit);
            }

            /* hardware does not support border color */
            if(localMask & __GL_TEXPARAM_BORDER_COLOR_BIT) {
                updateTextureBorderColor(gc, tex, unit);
            }

            if (localMask & __GL_TEXENV_BITS) {
                updateTextureEnv(gc, unit, localMask);
            }

            if (localMask & __GL_TEXGEN_BITS) {
                updateTexGenState(gc, unit, localMask);
            }
        }

        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_VP_TWO_SIDE_ENABLE)
    {
    }
}


GLvoid clampColorChanged(__GLcontext *gc, GLuint mask)
{
}

/************************************************************************/
/* Implementation for device state API                                  */
/************************************************************************/
GLvoid __glChipAttributeChange(__GLcontext *gc)
{
    clampColorChanged(gc, gc->globalDirtyState[__GL_DIRTY_ATTRS_1]);

    /* validate program  */
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS])
    {
        gc->dp.beginIndex = __GL_DP_GENERIC_PATH;
    }

    validateState(gc);
}

GLvoid __glChipUpdateShadingMode(__GLcontext *gc, GLenum value)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if (value == GL_FLAT)
        chipCtx->hashKey.hashShadingMode = 1;
    else
        chipCtx->hashKey.hashShadingMode = 0;
}


