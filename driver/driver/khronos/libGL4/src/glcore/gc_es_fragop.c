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

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE

#ifdef OPENGL40
GLvoid APIENTRY __glim_AlphaFunc(__GLcontext *gc, GLenum func, GLfloat ref)
{
    GLfloat zero = __glZero, one = __glOne;
    __GL_HEADER();
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if ((func < GL_NEVER) || (func > GL_ALWAYS)) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (ref < zero)
        ref = zero;
    if (ref > one)
        ref = one;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.alphaFunction = func;
    gc->state.raster.alphaReference = ref;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_ALPHAFUNC_BIT);

    __GL_FOOTER();
}
#endif
GLvoid GL_APIENTRY __gles_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask)
{
    __GL_HEADER();

    if ((func < GL_NEVER) || (func > GL_ALWAYS))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Clamped reference value */
    if (ref < 0)
    {
        ref = 0;
    }

    mask = mask & 0xff;

    /* Update stencil state */
    gc->state.stencil.front.testFunc  = func;
    gc->state.stencil.front.reference = ref;
    gc->state.stencil.front.mask      = mask;

    gc->state.stencil.back.testFunc   = func;
    gc->state.stencil.back.reference  = ref;
    gc->state.stencil.back.mask       = mask;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_FRONT_BIT | __GL_STENCILFUNC_BACK_BIT);

OnError:
    __GL_FOOTER();
    return;

}

GLvoid GL_APIENTRY __gles_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_HEADER();

    if ((func < GL_NEVER) || (func > GL_ALWAYS))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Clamped reference value */
    if (ref < 0)
    {
        ref = 0;
    }

    switch (face)
    {
      case GL_FRONT:
          gc->state.stencil.front.testFunc  = func;
          gc->state.stencil.front.reference = ref;
          gc->state.stencil.front.mask      = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_FRONT_BIT);
          break;

      case GL_BACK:
          gc->state.stencil.back.testFunc  = func;
          gc->state.stencil.back.reference = ref;
          gc->state.stencil.back.mask      = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_BACK_BIT);
          break;

      case GL_FRONT_AND_BACK:
          gc->state.stencil.front.testFunc = func;
          gc->state.stencil.front.reference = ref;
          gc->state.stencil.front.mask = mask;
          gc->state.stencil.back.testFunc = func;
          gc->state.stencil.back.reference = ref;
          gc->state.stencil.back.mask = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_FRONT_BIT | __GL_STENCILFUNC_BACK_BIT);
          break;

      default:
          __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_StencilMask(__GLcontext *gc, GLuint sm)
{
    __GL_HEADER();
    /* Update stencil state */
    gc->state.stencil.front.writeMask = sm;
    gc->state.stencil.back.writeMask  = sm;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_FRONT_BIT | __GL_STENCILMASK_BACK_BIT);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint sm)
{
    __GL_HEADER();

    switch (face)
    {
    case GL_FRONT:
        gc->state.stencil.front.writeMask = sm;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_FRONT_BIT);
        break;

    case GL_BACK:
        gc->state.stencil.back.writeMask = sm;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_BACK_BIT);
        break;

    case GL_FRONT_AND_BACK:
        gc->state.stencil.front.writeMask = sm;
        gc->state.stencil.back.writeMask = sm;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_FRONT_BIT | __GL_STENCILMASK_BACK_BIT);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_StencilOp(__GLcontext *gc, GLenum fail, GLenum depthFail, GLenum depthPass)
{
    __GL_HEADER();

    switch (fail)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (depthFail)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (depthPass)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update stencil state */
    gc->state.stencil.front.fail = fail;
    gc->state.stencil.front.depthFail = depthFail;
    gc->state.stencil.front.depthPass = depthPass;

    gc->state.stencil.back.fail = fail;
    gc->state.stencil.back.depthFail = depthFail;
    gc->state.stencil.back.depthPass = depthPass;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_FRONT_BIT | __GL_STENCILOP_BACK_BIT);

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail,
                                            GLenum depthFail, GLenum depthPass)
{
    __GL_HEADER();

    switch (fail)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (depthFail)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (depthPass)
    {
    case GL_KEEP: case GL_ZERO: case GL_REPLACE:
    case GL_INCR: case GL_DECR: case GL_INVERT:
    case GL_INCR_WRAP: case GL_DECR_WRAP:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (face)
    {
    case GL_FRONT:
        gc->state.stencil.front.fail = fail;
        gc->state.stencil.front.depthFail = depthFail;
        gc->state.stencil.front.depthPass = depthPass;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_FRONT_BIT);
        break;

    case GL_BACK:
        gc->state.stencil.back.fail = fail;
        gc->state.stencil.back.depthFail = depthFail;
        gc->state.stencil.back.depthPass = depthPass;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_BACK_BIT);
        break;

    case GL_FRONT_AND_BACK:
        gc->state.stencil.front.fail = fail;
        gc->state.stencil.front.depthFail = depthFail;
        gc->state.stencil.front.depthPass = depthPass;
        gc->state.stencil.back.fail = fail;
        gc->state.stencil.back.depthFail = depthFail;
        gc->state.stencil.back.depthPass = depthPass;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_FRONT_BIT | __GL_STENCILOP_BACK_BIT);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_DepthRangef(__GLcontext *gc, GLfloat zNear, GLfloat zFar)
{
    __GL_HEADER();
    /* Clamp depth range to legal values */
    if (zNear < __glZero)
    {
        zNear = __glZero;
    }
    if (zNear > __glOne)
    {
        zNear = __glOne;
    }

    if (zFar < __glZero)
    {
        zFar = __glZero;
    }
    if (zFar > __glOne)
    {
        zFar = __glOne;
    }

    /* Update viewport state */
    gc->state.depth.zNear = zNear;
    gc->state.depth.zFar = zFar;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHRANGE_BIT);

    __GL_FOOTER();
}

__GL_INLINE GLvoid __glScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    __GL_HEADER();
    /* Update scissor state */
    gc->state.scissor.scissorX = x;
    gc->state.scissor.scissorY = y;
    gc->state.scissor.scissorWidth = w;
    gc->state.scissor.scissorHeight = h;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SCISSOR_BIT);

    __GL_FOOTER();
}

GLvoid __glUpdateScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    __glScissor(gc, x, y, w, h);
}

GLvoid GL_APIENTRY __gles_Scissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    __GLscissor scissor = {x, y, w, h};

    __GL_HEADER();

    if (!__GL_MEMCMP(&gc->state.scissor, &scissor, sizeof(__GLscissor)))
    {
        __GL_EXIT();
    }

    if ((w < 0) || (h < 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glScissor(gc, x, y, w, h);

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_BlendColor(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GL_HEADER();
    /* Update blend state */
    gc->state.raster.blendColor.r = r;
    gc->state.raster.blendColor.g = g;
    gc->state.raster.blendColor.b = b;
    gc->state.raster.blendColor.a = a;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDCOLOR_BIT);
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_BlendEquation(__GLcontext *gc, GLenum mode)
{
    GLuint i;

    __GL_HEADER();

    if (!gc->modes.rgbMode)
    {
        __GL_EXIT();
    }

    switch (mode)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
    case GL_MULTIPLY_KHR:
    case GL_SCREEN_KHR:
    case GL_OVERLAY_KHR:
    case GL_DARKEN_KHR:
    case GL_LIGHTEN_KHR:
    case GL_COLORDODGE_KHR:
    case GL_COLORBURN_KHR:
    case GL_HARDLIGHT_KHR:
    case GL_SOFTLIGHT_KHR:
    case GL_DIFFERENCE_KHR:
    case GL_EXCLUSION_KHR:
    case GL_HSL_HUE_KHR:
    case GL_HSL_SATURATION_KHR:
    case GL_HSL_COLOR_KHR:
    case GL_HSL_LUMINOSITY_KHR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update blend state */
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        gc->state.raster.blendEquationRGB[i] = mode;
        gc->state.raster.blendEquationAlpha[i] = mode;
    }

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha)
{
    GLuint i;

    __GL_HEADER();

    if (!gc->modes.rgbMode)
    {
        __GL_EXIT();
    }

    switch (modeRGB)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (modeAlpha)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update blend state */
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        gc->state.raster.blendEquationRGB[i] = modeRGB;
        gc->state.raster.blendEquationAlpha[i] = modeAlpha;
    }

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor)
{
    GLuint i;

    __GL_HEADER();

    if (!gc->modes.rgbMode)
    {
        __GL_EXIT();
    }

    switch (sfactor)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactor)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA_SATURATE:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        if ((gc->state.raster.blendSrcRGB[i] != sfactor) ||
            (gc->state.raster.blendSrcAlpha[i] != sfactor) ||
            (gc->state.raster.blendDstRGB[i] != dfactor) ||
            (gc->state.raster.blendDstAlpha[i] != dfactor))
        {
            /* Update blend state */
            gc->state.raster.blendSrcRGB[i] = sfactor;
            gc->state.raster.blendSrcAlpha[i] = sfactor;
            gc->state.raster.blendDstRGB[i] = dfactor;
            gc->state.raster.blendDstAlpha[i] = dfactor;

            /* Flip attribute dirty bit */
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);
        }
    }
OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_BlendFuncSeparate(__GLcontext *gc, GLenum sfactorRGB, GLenum dfactorRGB,
                                            GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    GLuint i;

    __GL_HEADER();

    if (!gc->modes.rgbMode)
    {
        __GL_EXIT();
    }

    switch (sfactorRGB)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (sfactorAlpha)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactorRGB)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactorAlpha)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        /* Update blend state */
        gc->state.raster.blendSrcRGB[i] = sfactorRGB;
        gc->state.raster.blendDstRGB[i] = dfactorRGB;
        gc->state.raster.blendSrcAlpha[i] = sfactorAlpha;
        gc->state.raster.blendDstAlpha[i] = dfactorAlpha;
    }

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode)
{
    __GL_HEADER();

    if (buf >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (mode)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
    case GL_MULTIPLY_KHR:
    case GL_SCREEN_KHR:
    case GL_OVERLAY_KHR:
    case GL_DARKEN_KHR:
    case GL_LIGHTEN_KHR:
    case GL_COLORDODGE_KHR:
    case GL_COLORBURN_KHR:
    case GL_HARDLIGHT_KHR:
    case GL_SOFTLIGHT_KHR:
    case GL_DIFFERENCE_KHR:
    case GL_EXCLUSION_KHR:
    case GL_HSL_HUE_KHR:
    case GL_HSL_SATURATION_KHR:
    case GL_HSL_COLOR_KHR:
    case GL_HSL_LUMINOSITY_KHR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((gc->state.raster.blendEquationRGB[buf] != mode) ||
        (gc->state.raster.blendEquationAlpha[buf] != mode))
    {
        gc->state.raster.blendEquationRGB[buf] = mode;
        gc->state.raster.blendEquationAlpha[buf] = mode;

        /* Flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);
    }

OnError:
    __GL_FOOTER();
    return;
}


GLvoid GL_APIENTRY __gles_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_HEADER();

    if (buf >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (modeRGB)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (modeAlpha)
    {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((gc->state.raster.blendEquationRGB[buf] != modeRGB) ||
        (gc->state.raster.blendEquationAlpha[buf] != modeAlpha))
    {
        /* Update blend state */
        gc->state.raster.blendEquationRGB[buf] = modeRGB;
        gc->state.raster.blendEquationAlpha[buf] = modeAlpha;

        /* Flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);
    }

OnError:
    __GL_FOOTER();
    return;
}


GLvoid GL_APIENTRY __gles_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GL_HEADER();

    if (buf >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (sfactor)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactor)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA_SATURATE:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((gc->state.raster.blendSrcRGB[buf] != sfactor)   ||
        (gc->state.raster.blendSrcAlpha[buf] != sfactor) ||
        (gc->state.raster.blendDstRGB[buf] != dfactor)   ||
        (gc->state.raster.blendDstAlpha[buf] != dfactor))
    {
        /* Update blend state */
        gc->state.raster.blendSrcRGB[buf] = sfactor;
        gc->state.raster.blendSrcAlpha[buf] = sfactor;
        gc->state.raster.blendDstRGB[buf] = dfactor;
        gc->state.raster.blendDstAlpha[buf] = dfactor;

        /* Flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);
    }
OnError:
    __GL_FOOTER();
    return;

}

GLvoid GL_APIENTRY __gles_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GL_HEADER();
    if (buf >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (sfactorRGB)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (sfactorAlpha)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactorRGB)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (dfactorAlpha)
    {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((gc->state.raster.blendSrcRGB[buf] != sfactorRGB)     ||
        (gc->state.raster.blendDstRGB[buf] != dfactorRGB)     ||
        (gc->state.raster.blendSrcAlpha[buf] != sfactorAlpha) ||
        (gc->state.raster.blendDstAlpha[buf] != dfactorAlpha))
    {
        /* Update blend state */
        gc->state.raster.blendSrcRGB[buf] = sfactorRGB;
        gc->state.raster.blendDstRGB[buf] = dfactorRGB;
        gc->state.raster.blendSrcAlpha[buf] = sfactorAlpha;
        gc->state.raster.blendDstAlpha[buf] = dfactorAlpha;

        /* Flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);
    }

OnError:
    __GL_FOOTER();
    return;
}


GLvoid GL_APIENTRY __gles_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_HEADER();

    if (buf >= gc->constants.shaderCaps.maxDrawBuffers)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((gc->state.raster.colorMask[buf].redMask != r)||
        (gc->state.raster.colorMask[buf].greenMask != g) ||
        (gc->state.raster.colorMask[buf].blueMask != b) ||
        (gc->state.raster.colorMask[buf].alphaMask != a))
    {
        gc->state.raster.colorMask[buf].redMask = r;
        gc->state.raster.colorMask[buf].greenMask = g;
        gc->state.raster.colorMask[buf].blueMask = b;
        gc->state.raster.colorMask[buf].alphaMask = a;

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
    }

OnError:
    __GL_FOOTER();
    return;

}

GLvoid GL_APIENTRY __gles_DepthFunc(__GLcontext *gc, GLenum zfunc)
{
    __GL_HEADER();

    if ((zfunc < GL_NEVER) || (zfunc > GL_ALWAYS))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (zfunc ^ gc->state.depth.testFunc)
    {
        /* Update depth state */
        gc->state.depth.testFunc = zfunc;

        /* Flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHFUNC_BIT);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_ClearColor(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GL_HEADER();
    /* Update clear state */
    gc->state.raster.clearColor.clear.r = r;
    gc->state.raster.clearColor.clear.g = g;
    gc->state.raster.clearColor.clear.b = b;
    gc->state.raster.clearColor.clear.a = a;
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ColorMask(__GLcontext *gc, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    GLuint i;

    __GL_HEADER();
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        gc->state.raster.colorMask[i].redMask = r;
        gc->state.raster.colorMask[i].greenMask = g;
        gc->state.raster.colorMask[i].blueMask = b;
        gc->state.raster.colorMask[i].alphaMask = a;
    }

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DepthMask(__GLcontext *gc, GLboolean flag)
{
    __GL_HEADER();
    gc->state.depth.writeEnable = flag;
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_DEPTHMASK_BIT );
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ClearDepthf(__GLcontext *gc, GLfloat z)
{
    __GL_HEADER();
    /* Update clear state */
    gc->state.depth.clear = z;
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ClearStencil(__GLcontext *gc, GLint s)
{
    __GL_HEADER();
    gc->state.stencil.clear = s;
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_SampleCoverage(__GLcontext *gc, GLclampf value, GLboolean invert)
{
    __GL_HEADER();
    if (value < __glZero)
    {
        value = __glZero;
    }
    if (value > __glOne)
    {
        value = __glOne;
    }

    gc->state.multisample.coverageValue = value;
    gc->state.multisample.coverageInvert = invert;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLECOVERAGE_BIT);
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val)
{
    GLuint currentSamples = 0;

    __GL_HEADER();

    if (pname != GL_SAMPLE_POSITION)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj))
    {
        currentSamples = gc->frameBuffer.drawFramebufObj->fbSamples;
    }

    if (index >= currentSamples)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);

    (*gc->dp.getSampleLocation)(gc, index, val);

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask)
{
    __GL_HEADER();
    if (maskNumber >= gc->constants.maxSampleMaskWords)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    gc->state.multisample.sampleMaskValue = mask;

    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_MASK_BIT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_MinSampleShading(__GLcontext *gc, GLfloat value)
{
    __GL_HEADER();
    /* <value> is clamped to [0,1] when specified. */
    if (value < 0.0)
    {
        value = 0.0;
    }
    else if (value > 1.0)
    {
        value = 1.0;
    }

    gc->state.multisample.minSampleShadingValue = value;

    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_SAMPLE_MIN_SHADING_VALUE_BIT);

    __GL_FOOTER();
}



