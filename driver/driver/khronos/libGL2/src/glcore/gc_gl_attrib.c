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
#include "gc_gl_debug.h"
#include "dri/viv_lock.h"

extern GLvoid APIENTRY __glim_Enable(GLenum);
extern GLvoid APIENTRY __glim_Disable(GLenum);
extern GLvoid __glWriteHitRecord(__GLcontext *gc);
extern GLvoid __glSetTexEnableDimension(__GLcontext *gc, GLuint unit);

GLfloat __glClampSize(GLfloat size, __GLdeviceConstants *constants)
{
    GLfloat min  = constants->pointSizeMinimum;
    GLfloat max  = constants->pointSizeMaximum;
    GLfloat gran = constants->pointSizeGranularity;
    GLint i;

    if (size <= min)
        return min;
    else if (size >= max)
        return max;
    else {
        i = (GLint)((size - min) / gran + 0.5);
        return (min + i * gran);
    }
}

GLfloat __glClampWidth(GLfloat width, __GLdeviceConstants *constants)
{
    GLfloat min  = constants->lineWidthMinimum;
    GLfloat max  = constants->lineWidthMaximum;
    GLfloat gran = constants->lineWidthGranularity;
    GLint i;

    if (width <= min)
        return min;
    else if (width >= max)
        return max;
    else {
        i = (GLint)((width - min) / gran + 0.5);
        return (min + i * gran);
    }
}

__GL_INLINE GLvoid __glViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h)
{
    /* Clamp viewport width and height */
    if (w > (GLint)gc->constants.maxViewportWidth) {
        w = gc->constants.maxViewportWidth;
    }
    if (h > (GLint)gc->constants.maxViewportHeight) {
        h = gc->constants.maxViewportHeight;
    }

    gc->state.viewport.x = x;
    gc->state.viewport.y = y;
    gc->state.viewport.width = w;
    gc->state.viewport.height = h;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_VIEWPORT_BIT);
}

GLvoid __glUpdateViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h)
{
    __glViewport(gc, x, y, w, h);
}

GLvoid APIENTRY APIENTRY __glim_Viewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Viewport", DT_GLint, x, DT_GLint, y, DT_GLsizei, w, DT_GLsizei, h, DT_GLnull);
#endif

    if ((w < 0) || (h < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glViewport(gc, x, y, w, h);
}

GLvoid APIENTRY __glim_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_StencilFunc", DT_GLenum, func, DT_GLint ,ref, DT_GLuint, mask, DT_GLnull);
#endif

    if ((func < GL_NEVER) || (func > GL_ALWAYS)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Clamped reference value */
    if (ref < 0) {
        ref = 0;
    }

    /* Update ARB GL 2.0 state */
    gc->state.stencil.StencilArb.front.testFunc = func;
    gc->state.stencil.StencilArb.front.reference = ref;
    gc->state.stencil.StencilArb.front.mask = mask;

    gc->state.stencil.StencilArb.back.testFunc = func;
    gc->state.stencil.StencilArb.back.reference = ref;
    gc->state.stencil.StencilArb.back.mask = mask;

    /* Update GL_EXT_stencil_two_side state */
    if (gc->state.stencil.activeStencilFace == GL_FRONT)
    {
        gc->state.stencil.stencilExt.front.testFunc = func;
        gc->state.stencil.stencilExt.front.reference = ref;
        gc->state.stencil.stencilExt.front.mask = mask;
    }
    else if (gc->state.stencil.activeStencilFace == GL_BACK)
    {
        gc->state.stencil.stencilExt.back.testFunc = func;
        gc->state.stencil.stencilExt.back.reference = ref;
        gc->state.stencil.stencilExt.back.mask = mask;
    }

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
        __GL_STENCILFUNC_FRONT_BIT | __GL_STENCILFUNC_BACK_BIT);
}

GLvoid APIENTRY __glim_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("StencilFuncSeparate", DT_GLenum, face, DT_GLenum, func, DT_GLint, ref, DT_GLuint, mask, DT_GLnull);
#endif

    if ((func < GL_NEVER) || (func > GL_ALWAYS)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Clamped reference value */
    if (ref < 0) {
        ref = 0;
    }

    switch (face) {
      case GL_FRONT:
          gc->state.stencil.StencilArb.front.testFunc = func;
          gc->state.stencil.StencilArb.front.reference = ref;
          gc->state.stencil.StencilArb.front.mask = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_FRONT_BIT);
          break;

      case GL_BACK:
          gc->state.stencil.StencilArb.back.testFunc = func;
          gc->state.stencil.StencilArb.back.reference = ref;
          gc->state.stencil.StencilArb.back.mask = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILFUNC_BACK_BIT);
          break;

      case GL_FRONT_AND_BACK:
          gc->state.stencil.StencilArb.front.testFunc = func;
          gc->state.stencil.StencilArb.front.reference = ref;
          gc->state.stencil.StencilArb.front.mask = mask;
          gc->state.stencil.StencilArb.back.testFunc = func;
          gc->state.stencil.StencilArb.back.reference = ref;
          gc->state.stencil.StencilArb.back.mask = mask;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
              __GL_STENCILFUNC_FRONT_BIT | __GL_STENCILFUNC_BACK_BIT);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

#if GL_ATI_separate_stencil
GLvoid APIENTRY __glim_StencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("StencilFuncSeparateATI", DT_GLenum, frontfunc, DT_GLenum, backfunc, DT_GLint, ref, DT_GLuint, mask, DT_GLnull);
#endif

    if ((frontfunc < GL_NEVER) || (frontfunc > GL_ALWAYS)
        || (backfunc < GL_NEVER) || (backfunc > GL_ALWAYS) )
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Clamped reference value */
    if (ref < 0) {
        ref = 0;
    }


    gc->state.stencil.StencilArb.front.testFunc = frontfunc;
    gc->state.stencil.StencilArb.front.reference = ref;
    gc->state.stencil.StencilArb.front.mask = mask;
    gc->state.stencil.StencilArb.back.testFunc = backfunc;
    gc->state.stencil.StencilArb.back.reference = ref;
    gc->state.stencil.StencilArb.back.mask = mask;
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
        __GL_STENCILFUNC_FRONT_BIT | __GL_STENCILFUNC_BACK_BIT);

}
#endif

GLvoid APIENTRY __glim_StencilOp(GLenum fail, GLenum depthFail, GLenum depthPass)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_StencilOp", DT_GLenum, fail, DT_GLenum,depthFail, DT_GLenum, depthPass, DT_GLnull);
#endif

    switch (fail) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (depthFail) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (depthPass) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL 2.0 state */
    gc->state.stencil.StencilArb.front.fail = fail;
    gc->state.stencil.StencilArb.front.depthFail = depthFail;
    gc->state.stencil.StencilArb.front.depthPass = depthPass;

    gc->state.stencil.StencilArb.back.fail = fail;
    gc->state.stencil.StencilArb.back.depthFail = depthFail;
    gc->state.stencil.StencilArb.back.depthPass = depthPass;

    /* Update GL_EXT_stencil_two_side state */
    if (gc->state.stencil.activeStencilFace == GL_FRONT)
    {
        gc->state.stencil.stencilExt.front.fail = fail;
        gc->state.stencil.stencilExt.front.depthFail = depthFail;
        gc->state.stencil.stencilExt.front.depthPass = depthPass;
    }
    else if (gc->state.stencil.activeStencilFace == GL_BACK)
    {
        gc->state.stencil.stencilExt.back.fail = fail;
        gc->state.stencil.stencilExt.back.depthFail = depthFail;
        gc->state.stencil.stencilExt.back.depthPass = depthPass;
    }

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
        __GL_STENCILOP_FRONT_BIT | __GL_STENCILOP_BACK_BIT);
}

GLvoid APIENTRY __glim_StencilOpSeparate(GLenum face, GLenum fail, GLenum depthFail, GLenum depthPass)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_StencilOpSeparate", DT_GLenum, face, DT_GLenum, fail, DT_GLenum, depthFail, DT_GLenum, depthPass, DT_GLnull);
#endif

    switch (fail) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (depthFail) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (depthPass) {
      case GL_KEEP: case GL_ZERO: case GL_REPLACE:
      case GL_INCR: case GL_DECR: case GL_INVERT:
      case GL_INCR_WRAP: case GL_DECR_WRAP:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (face) {
      case GL_FRONT:
          gc->state.stencil.StencilArb.front.fail = fail;
          gc->state.stencil.StencilArb.front.depthFail = depthFail;
          gc->state.stencil.StencilArb.front.depthPass = depthPass;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_FRONT_BIT);
          break;

      case GL_BACK:
          gc->state.stencil.StencilArb.back.fail = fail;
          gc->state.stencil.StencilArb.back.depthFail = depthFail;
          gc->state.stencil.StencilArb.back.depthPass = depthPass;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILOP_BACK_BIT);
          break;

      case GL_FRONT_AND_BACK:
          gc->state.stencil.StencilArb.front.fail = fail;
          gc->state.stencil.StencilArb.front.depthFail = depthFail;
          gc->state.stencil.StencilArb.front.depthPass = depthPass;
          gc->state.stencil.StencilArb.back.fail = fail;
          gc->state.stencil.StencilArb.back.depthFail = depthFail;
          gc->state.stencil.StencilArb.back.depthPass = depthPass;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
              __GL_STENCILOP_FRONT_BIT | __GL_STENCILOP_BACK_BIT);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

#if GL_EXT_stencil_two_side
GLvoid APIENTRY __glim_ActiveStencilFaceEXT(GLenum face)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ActiveStencilFaceEXT", DT_GLenum, face, DT_GLnull);
#endif

    switch (face)
    {
    case GL_FRONT:
    case GL_BACK:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->state.stencil.activeStencilFace = face;
}
#endif

GLvoid APIENTRY __glim_PolygonMode(GLenum face, GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PolygonMode", DT_GLenum, face, DT_GLenum, mode, DT_GLnull);
#endif

    if (((GL_FRONT != face) && (GL_BACK != face) && (GL_FRONT_AND_BACK != face)) ||
        ((GL_POINT != mode) && (GL_LINE != mode) && (GL_FILL != mode))) {
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    if (GL_BACK != face) {
        gc->state.polygon.frontMode = mode;
    }
    if (GL_FRONT != face) {
        gc->state.polygon.backMode = mode;
    }

    if (gc->state.polygon.frontMode == GL_FILL && gc->state.polygon.backMode == GL_FILL) {
        gc->state.polygon.bothFaceFill = GL_TRUE;
    }
    else {
        gc->state.polygon.bothFaceFill = GL_FALSE;
    }

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONMODE_BIT);

    __GL_INPUTMASK_CHANGED(gc);
}

GLvoid APIENTRY __glim_ShadeModel(GLenum sm)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ShadeModel", DT_GLenum, sm, DT_GLnull);
#endif

    if ((sm != GL_FLAT) && (sm != GL_SMOOTH)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.light.shadingModel, sm);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.light.shadingModel = sm;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_SHADEMODEL_BIT);
}

GLvoid APIENTRY __glim_LineWidth(GLfloat width)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LineWidth", DT_GLfloat, width, DT_GLnull);
#endif

    if (width <= 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.line.requestedWidth, width);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.line.requestedWidth = width;

    /* Round */
    gc->state.line.aliasedWidth = (GLint)((width < 1.0) ? 1 : width + 0.5);

    gc->state.line.smoothWidth = __glClampWidth(width, &gc->constants);

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINEWIDTH_BIT);
}

GLvoid APIENTRY __glim_DepthRange(GLdouble zNear, GLdouble zFar)
{
    GLdouble zero = 0.0, one = 1.0;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DepthRange", DT_GLdouble, zNear, DT_GLdouble, zFar, DT_GLnull);
#endif

    /* Clamp depth range to legal values */
    if (zNear < zero)
        zNear = zero;
    if (zNear > one)
        zNear = one;
    if (zFar < zero)
        zFar = zero;
    if (zFar > one)
        zFar = one;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.viewport.zNear = (GLfloat)zNear;
    gc->state.viewport.zFar = (GLfloat)zFar;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHRANGE_BIT);
}

#if GL_EXT_depth_bounds_test
GLvoid APIENTRY __glim_DepthBoundsEXT(GLclampd zMin, GLclampd zMax)
{
    GLdouble zero = 0.0, one = 1.0;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DepthBoundsEXT", DT_GLclampd, zMin, DT_GLclampd, zMax, DT_GLnull);
#endif

    /* Clamp depth range to legal values */
    if (zMin < zero)
        zMin = zero;
    if (zMin > one)
        zMin = one;
    if (zMax < zero)
        zMax = zero;
    if (zMax > one)
        zMax = one;

    if(zMin > zMax)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.depthBoundTest.zMin = (GLfloat)zMin;
    gc->state.depthBoundTest.zMax = (GLfloat)zMax;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHBOUNDTEST_BIT);
}
#endif



GLvoid APIENTRY __glim_FrontFace(GLenum dir)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FrontFace", DT_GLenum, dir, DT_GLnull);
#endif

    if ((GL_CW != dir) && (GL_CCW != dir)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.polygon.frontFace, dir);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.polygon.frontFace = dir;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FRONTFACE_BIT);
}

GLvoid APIENTRY __glim_CullFace(GLenum cfm)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CullFace", DT_GLenum, cfm, DT_GLnull);
#endif

    if ((GL_FRONT != cfm) && (GL_BACK != cfm) && (GL_FRONT_AND_BACK != cfm)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.polygon.cullFace, cfm);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.polygon.cullFace = cfm;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CULLFACE_BIT);

}

GLvoid APIENTRY __glim_PointSize(GLfloat size)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PointSize", DT_GLfloat, size, DT_GLnull);
#endif

    if (size <= 0.0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.point.requestedSize, size);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.point.requestedSize = size;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSIZE_BIT);
}

GLvoid APIENTRY __glim_PointParameterf(GLenum pname, GLfloat param)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PointParameterf", DT_GLenum, pname, DT_GLfloat, param, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = (GLenum)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameterfv(GLenum pname, const GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PointParameterfv", DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_DISTANCE_ATTENUATION:
          gc->state.point.distanceAttenuation[0] = params[0];
          gc->state.point.distanceAttenuation[1] = params[1];
          gc->state.point.distanceAttenuation[2] = params[2];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_DISTANCE_ATTENUATION_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = (GLenum)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameteri(GLenum pname, GLint param)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PointParameteri", DT_GLenum, pname, DT_GLint, param, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameteriv(GLenum pname, const GLint *params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PointParameteriv", DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_DISTANCE_ATTENUATION:
          gc->state.point.distanceAttenuation[0] = (GLfloat)params[0];
          gc->state.point.distanceAttenuation[1] = (GLfloat)params[1];
          gc->state.point.distanceAttenuation[2] = (GLfloat)params[2];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINT_DISTANCE_ATTENUATION_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

__GL_INLINE GLvoid __glScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    /* Update GL state */
    gc->state.scissor.scissorX = x;
    gc->state.scissor.scissorY = y;
    gc->state.scissor.scissorWidth = w;
    gc->state.scissor.scissorHeight = h;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_SCISSOR_BIT);

    /* Re-compute the core clip rectangle */
    __glComputeClipBox(gc);
}

GLvoid __glUpdateScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    __glScissor(gc, x, y, w, h);
}

GLvoid APIENTRY __glim_Scissor(GLint x, GLint y, GLint w, GLint h)
{
    __GLscissor scissor = {x, y, w, h};

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Scissor", DT_GLint, x, DT_GLint, y, DT_GLint, w, DT_GLint, h, DT_GLnull);
#endif

    if (!__GL_MEMCMP(&gc->state.scissor, &scissor, sizeof(__GLscissor)))
        return;

    if ((w < 0) || (h < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glScissor(gc, x, y, w, h);
}

GLvoid APIENTRY __glim_LineStipple(GLint factor, GLushort stipple)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LineStipple", DT_GLint, factor, DT_GLushort, stipple, DT_GLnull);
#endif

    if (factor < 1)
        factor = 1;
    if (factor > 256)
        factor = 256;

    __GL_REDUNDANT_ATTR2(gc->state.line.stippleRepeat, factor, gc->state.line.stipple, stipple);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.line.stippleRepeat = (GLshort)factor;
    gc->state.line.stipple = stipple;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINESTIPPLE_BIT);
}

GLvoid APIENTRY __glim_PolygonStipple(const GLubyte *mask)
{
    GLubyte stipple[4*32];

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PolygonStipple", DT_GLubyte_ptr, mask, DT_GLnull);
#endif

    __glFillImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, mask, stipple);

    if (__GL_MEMCMP(gc->state.polygonStipple.stipple, stipple, sizeof(stipple)) != 0)
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        __GL_MEMCOPY(gc->state.polygonStipple.stipple, stipple, sizeof(stipple));

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONSTIPPLE_BIT);
    }
}

GLvoid APIENTRY __glim_PolygonOffset(GLfloat factor, GLfloat units)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_PolygonOffset", DT_GLfloat, factor, DT_GLfloat, units, DT_GLnull);
#endif

    __GL_REDUNDANT_ATTR2(gc->state.polygon.factor, factor, gc->state.polygon.units, units);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.polygon.factor = factor;
    gc->state.polygon.units = units;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONOFFSET_BIT);
}

GLvoid APIENTRY __glim_BlendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BlendColor", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLfloat, a, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.blendColor.r = r;
    gc->state.raster.blendColor.g = g;
    gc->state.raster.blendColor.b = b;
    gc->state.raster.blendColor.a = a;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDCOLOR_BIT);
}

GLvoid APIENTRY __glim_BlendFunc(GLenum sfactor, GLenum dfactor)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BlendFunc", DT_GLenum, sfactor, DT_GLenum, dfactor, DT_GLnull);
#endif

    if (!gc->modes.rgbMode)
        return;

    switch (sfactor) {
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (dfactor) {
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
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    if ((gc->state.raster.blendSrcRGB != sfactor) ||
        (gc->state.raster.blendSrcAlpha != sfactor) ||
        (gc->state.raster.blendDstRGB != dfactor) ||
        (gc->state.raster.blendDstAlpha != dfactor))
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        /* Update GL state */
        gc->state.raster.blendSrcRGB = sfactor;
        gc->state.raster.blendSrcAlpha = sfactor;
        gc->state.raster.blendDstRGB = dfactor;
        gc->state.raster.blendDstAlpha = dfactor;

        /* flip attribute dirty bit */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);
    }
}

GLvoid APIENTRY __glim_BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BlendFuncSeparate", DT_GLenum, sfactorRGB, DT_GLenum, dfactorRGB, DT_GLenum, sfactorAlpha, DT_GLenum, dfactorAlpha, DT_GLnull);
#endif

    if (!gc->modes.rgbMode)
        return;

    switch (sfactorRGB) {
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (sfactorAlpha) {
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
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (dfactorRGB) {
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
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    switch (dfactorAlpha) {
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
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.blendSrcRGB = sfactorRGB;
    gc->state.raster.blendDstRGB = dfactorRGB;
    gc->state.raster.blendSrcAlpha = sfactorAlpha;
    gc->state.raster.blendDstAlpha = dfactorAlpha;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);
}

GLvoid APIENTRY __glim_BlendEquation(GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BlendEquation", DT_GLenum, mode, DT_GLnull);
#endif

    if (!gc->modes.rgbMode)
        return;

    switch (mode) {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
    case GL_LOGIC_OP:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.blendEquationRGB = mode;
    gc->state.raster.blendEquationAlpha = mode;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);
}

GLvoid APIENTRY __glim_BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BlendEquationSeparate", DT_GLenum, modeRGB, DT_GLenum, modeAlpha, DT_GLnull);
#endif

    if (!gc->modes.rgbMode)
        return;

    switch (modeRGB) {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
    case GL_LOGIC_OP:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (modeAlpha) {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
    case GL_MIN:
    case GL_MAX:
    case GL_LOGIC_OP:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.blendEquationRGB = modeRGB;
    gc->state.raster.blendEquationAlpha = modeAlpha;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDEQUATION_BIT);
}


GLvoid APIENTRY __glim_AlphaFunc(GLenum func, GLfloat ref)
{
    GLfloat zero = __glZero, one = __glOne;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_AlphaFunc", DT_GLenum, func, DT_GLfloat, ref, DT_GLnull);
#endif

    if ((func < GL_NEVER) || (func > GL_ALWAYS)) {
        __glSetError(GL_INVALID_VALUE);
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

}

GLvoid APIENTRY __glim_DepthFunc(GLenum zfunc)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DepthFunc", DT_GLenum, zfunc, DT_GLnull);
#endif

    if ((zfunc < GL_NEVER) || (zfunc > GL_ALWAYS)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.depth.testFunc = zfunc,

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHFUNC_BIT);
}

GLvoid APIENTRY __glim_ClipPlane(GLenum pi, const GLdouble *pv)
{
    __GLcoord mvEqu, eyeEqu;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClipPlane", DT_GLenum, pi, DT_GLdouble_ptr, pv, DT_GLnull);
#endif

    pi -= GL_CLIP_PLANE0;
    /* pi is GLenum type, unsigned, no need for negative check*/
    if ( (pi >= gc->constants.numberOfClipPlanes)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    mvEqu.x = (GLfloat)pv[0];
    mvEqu.y = (GLfloat)pv[1];
    mvEqu.z = (GLfloat)pv[2];
    mvEqu.w = (GLfloat)pv[3];

    /* Transform ClipPlane equation into eye space.
    */
    __glTransformVector(gc, &eyeEqu, &mvEqu, gc->transform.modelView, GL_FALSE);

    /* Update GL state */
    gc->state.transform.eyeClipPlanes[pi].x = eyeEqu.x;
    gc->state.transform.eyeClipPlanes[pi].y = eyeEqu.y;
    gc->state.transform.eyeClipPlanes[pi].z = eyeEqu.z;
    gc->state.transform.eyeClipPlanes[pi].w = eyeEqu.w;

    /* The lower 16 bits of globaldirtyState[__GL_CLIP_ATTRS] are for
    * clipPlane parameters.
    */
    gc->globalDirtyState[__GL_CLIP_ATTRS] |= (1 << pi);
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);
}

GLvoid __glDrawBuffersForFBO(__GLcontext *gc, GLsizei n, const GLenum *bufs)
{
    GLsizei i = 0;

    /* Error check */
    for(i = 0; i < n; i++)
    {
        if((bufs[i] != GL_NONE) && (bufs[i] < GL_COLOR_ATTACHMENT0_EXT || bufs[i] >__GL_COLOR_ATTACHMENTn_EXT))
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
    }

    /* Redundancy check */
    for(i = 0; i < (GLint)gc->constants.maxDrawBuffers; i++)
    {
        if(i < n)
        {
            if(gc->frameBuffer.drawFramebufObj->drawBuffer[i] != bufs[i])
            {
                __GL_VERTEX_BUFFER_FLUSH(gc);
                break;
            }
        }
        else
        {
            if(gc->frameBuffer.drawFramebufObj->drawBuffer[i] != GL_NONE)
            {
                __GL_VERTEX_BUFFER_FLUSH(gc);
                break;
            }
        }
    }

    if(i >= (GLint)gc->constants.maxDrawBuffers)
    {
        /* No really change, just return. */
        return;
    }

    /* Set per FBO state(may be we should change it to per context state) */
    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        if(i < n)
        {
            gc->frameBuffer.drawFramebufObj->drawBuffer[i] = bufs[i];
        }
        else
        {
            gc->frameBuffer.drawFramebufObj->drawBuffer[i] = GL_NONE;
        }
    }

    /* Store the draw buffer count */
    gc->frameBuffer.drawFramebufObj->drawBufferCount = n;

    /* Must notify DP immediately since it affects glClear */
    (*gc->dp.drawBuffers)(gc);

    /* flip attribute dirty bit */
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);


    /*__GL_FRAMEBUFFER_DRAWBUFFER_DIRTY*/
    FRAMEBUFFER_COMPLETENESS_DIRTY(gc->frameBuffer.drawFramebufObj);

    /*
    ** Immediate mode vertex data caching already be turned off
    ** when none zero FBO bound.
    */
}


GLvoid __glReadBufferForFBO(__GLcontext *gc, GLenum buf)
{
        __GL_REDUNDANT_ATTR(gc->frameBuffer.readFramebufObj->readBuffer, buf);

        /* Error check */
        if((buf != GL_NONE) && (buf < GL_COLOR_ATTACHMENT0_EXT || buf >__GL_COLOR_ATTACHMENTn_EXT)){
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        __GL_VERTEX_BUFFER_FLUSH(gc);

        /* Set per FBO state */
        gc->frameBuffer.readFramebufObj->readBuffer = buf;

        /* Must notify DP immediately since it affects glClear */
        (*gc->dp.readBuffer)(gc);

        /* flip attribute dirty bit */
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_READBUFFER_BIT);

        FRAMEBUFFER_COMPLETENESS_DIRTY(gc->frameBuffer.readFramebufObj);

}


GLvoid APIENTRY __glim_DrawBuffers( GLsizei n, const GLenum *bufs)
{
    GLint i, j, times;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawBuffers", DT_GLsizei, n, DT_GLenum_ptr, bufs, DT_GLnull);
#endif


    /*
    ** Gc is binding to a FBO, set the state to fbo not gc.
    */
    if(DRAW_FRAMEBUFFER_BINDING_NAME != 0){
        __glDrawBuffersForFBO(gc, n, bufs);
        return;
    }
    else
    {
        /*
        ** Error check
        */
        if((n <= 0) || (n > (GLsizei)gc->constants.maxDrawBuffers ))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        for(i = 0; i < n; i++)
        {
            if((bufs[i] == GL_FRONT) || (bufs[i] == GL_BACK) ||
                (bufs[i] == GL_LEFT) || (bufs[i] == GL_RIGHT) ||
                (bufs[i] == GL_FRONT_AND_BACK))
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            if(!gc->modes.stereoMode && ((bufs[i] == GL_FRONT_RIGHT)||(bufs[i] == GL_BACK_RIGHT)))
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            if(!gc->modes.doubleBufferMode && ((bufs[i] == GL_BACK_LEFT) ||(bufs[i] == GL_BACK_RIGHT)))
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }
            if(gc->modes.numAuxBuffers < (GLint)(bufs[i] - GL_AUX0))
            {
                __glSetError(GL_INVALID_OPERATION);
                return;
            }

        }

        for(i = GL_FRONT_LEFT; i <= GL_AUX3; i++)
        {
            times = 0;
            if((i <= GL_BACK_RIGHT) || (i >= GL_AUX0))
            {
                for(j = 0; j < n; j++)
                {
                    if(bufs[j] == i)
                    {
                        times++;
                        if(times > 1)
                        {
                            __glSetError(GL_INVALID_OPERATION);
                            return;
                        }
                    }
                }
            }
        }


        /* Redundancy check */
        for(i = 0; i < (GLint)gc->constants.maxDrawBuffers; i++)
        {
            if(i < n)
            {
                if(gc->state.raster.drawBuffer[i] != bufs[i])
                {
                    __GL_VERTEX_BUFFER_FLUSH(gc);
                    break;
                }
            }
            else
            {
                if(gc->state.raster.drawBuffer[i] != GL_NONE)
                {
                    __GL_VERTEX_BUFFER_FLUSH(gc);
                    break;
                }
            }
        }

        if(i >= n)
            return;

        for(i = 0; i < n; i++)
            gc->state.raster.drawBuffer[i] = bufs[i];

        __glEvaluateFramebufferChange(gc);

        __glDispatchDrawableChange(gc);

        LINUX_LOCK_FRAMEBUFFER( gc );

        /* Must notify DP immediately since it affects glClear */
        (*gc->dp.drawBuffer)(gc);

        LINUX_UNLOCK_FRAMEBUFFER( gc );

        /* flip attribute dirty bit */
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);

    }
}

GLvoid APIENTRY __glim_DrawBuffer(GLenum mode)
{
    GLenum modeOffset = 0;
    GLenum originalDrawBuffer;
    GLuint i;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DrawBuffer", DT_GLenum, mode, DT_GLnull);
#endif
    /*
    ** Gc is binding to a FBO, set the state to fbo not gc. */
    if(DRAW_FRAMEBUFFER_BINDING_NAME != 0){
        __glDrawBuffersForFBO(gc, 1, &mode);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.raster.drawBufferReturn, mode);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if ((mode & GL_FRONT_LEFT) && (mode >= GL_AUX0)) {
        modeOffset = mode - GL_AUX0;
        mode = GL_AUX0;
    }

    originalDrawBuffer = gc->state.raster.drawBuffer[0];
    gc->state.raster.drawBuffer[0] = mode;

    switch (mode) {
      case GL_NONE:
          gc->state.raster.drawBuffer[0] = GL_NONE;
          break;

      case GL_FRONT_RIGHT:
          if (!gc->modes.stereoMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          break;

      case GL_FRONT_LEFT:
          break;

      case GL_BACK_RIGHT:
          if (!(gc->modes.stereoMode && gc->modes.doubleBufferMode)) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          break;

      case GL_BACK_LEFT:
          if (!gc->modes.doubleBufferMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          break;

      case GL_FRONT:
          if (!gc->modes.stereoMode) {
              gc->state.raster.drawBuffer[0] = GL_FRONT_LEFT;
          }
          break;

      case GL_BACK:
          if (!gc->modes.doubleBufferMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          else if (!gc->modes.stereoMode) {
              gc->state.raster.drawBuffer[0] = GL_BACK_LEFT;
          }
          break;

      case GL_LEFT:
          if (!gc->modes.doubleBufferMode) {
              gc->state.raster.drawBuffer[0] = GL_FRONT_LEFT;
          }
          break;

      case GL_RIGHT:
          if (!gc->modes.stereoMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          else if (!gc->modes.doubleBufferMode) {
              gc->state.raster.drawBuffer[0] = GL_FRONT_RIGHT;
          }
          break;

      case GL_FRONT_AND_BACK:
          {
              if (!gc->modes.stereoMode) {
                  if(!gc->modes.doubleBufferMode){
                      gc->state.raster.drawBuffer[0] = GL_FRONT_LEFT;
                  } else {
                      gc->state.raster.drawBuffer[0] = GL_LEFT;
                  }
              } else {
                  if (!gc->modes.doubleBufferMode)
                  {
                      gc->state.raster.drawBuffer[0] = GL_FRONT;
                  }
              }
          }
          break;

      case GL_AUX0:
          if (modeOffset >= (GLenum)gc->modes.numAuxBuffers) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.raster.drawBuffer[0] = originalDrawBuffer;
              return;
          }
          mode = modeOffset + GL_AUX0;
          gc->state.raster.drawBuffer[0] = mode;
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    for(i = 1; i < gc->constants.maxDrawBuffers; i++)
        gc->state.raster.drawBuffer[i] = GL_NONE;

    /* Update GL state */
    gc->state.raster.drawBufferReturn = mode;

    LINUX_LOCK_FRAMEBUFFER( gc );

    /* Must notify DP immediately since it affects glClear */
    (*gc->dp.drawBuffer)(gc);

    LINUX_UNLOCK_FRAMEBUFFER( gc );

    /* flip attribute dirty bit */
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);
}

GLvoid APIENTRY __glim_ReadBuffer(GLenum mode)
{
    GLenum modeOffset = 0;
    GLenum originalReadBuffer;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ReadBuffer", DT_GLenum, mode, DT_GLnull);
#endif
    /*
    ** Gc is binding to a FBO, set the readbuffer state to fbo not gc. */
    if(READ_FRAMEBUFFER_BINDING_NAME != 0){
        __glReadBufferForFBO(gc, mode);
        return;
    }

    /*
    ** gc binding to the window buffer */

    __GL_REDUNDANT_ATTR(gc->state.pixel.readBufferReturn, mode);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if ((mode & GL_FRONT_LEFT) && (mode >= GL_AUX0)) {
        modeOffset = mode - GL_AUX0;
        mode = GL_AUX0;
    }

    originalReadBuffer = gc->state.pixel.readBuffer;
    gc->state.pixel.readBuffer = mode;

    switch (mode) {
      case GL_NONE:
          gc->state.pixel.readBuffer = GL_NONE;
          break;

      case GL_FRONT_RIGHT:
          if (!gc->modes.stereoMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          break;

      case GL_FRONT_LEFT:
          break;

      case GL_BACK_RIGHT:
          if (!(gc->modes.stereoMode && gc->modes.doubleBufferMode)) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          break;

      case GL_BACK_LEFT:
          if (!gc->modes.doubleBufferMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          break;

      case GL_FRONT:
          gc->state.pixel.readBuffer = GL_FRONT_LEFT;
          break;

      case GL_BACK:
          if (!gc->modes.doubleBufferMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          gc->state.pixel.readBuffer = GL_BACK_LEFT;
          break;

      case GL_LEFT:
              gc->state.pixel.readBuffer = GL_FRONT_LEFT;
          break;

      case GL_RIGHT:
          if (!gc->modes.stereoMode) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          gc->state.pixel.readBuffer = GL_FRONT_RIGHT;
          break;

      case GL_AUX0:
          if (modeOffset >= (GLenum)gc->modes.numAuxBuffers) {
              __glSetError(GL_INVALID_OPERATION);
              gc->state.pixel.readBuffer = originalReadBuffer;
              return;
          }
          mode = modeOffset + GL_AUX0;
          gc->state.pixel.readBuffer = mode;
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Update GL state */
    gc->state.pixel.readBufferReturn = mode;

    /* Call HW-specific function */
    (*gc->dp.readBuffer)(gc);

    /* flip attribute dirty bit */
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_READBUFFER_BIT);
}

GLvoid APIENTRY __glim_ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearColor", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLfloat, a, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.clear.r = r;
    gc->state.raster.clear.g = g;
    gc->state.raster.clear.b = b;
    gc->state.raster.clear.a = a;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARCOLOR_BIT);
}

GLvoid APIENTRY __glim_ClearIndex(GLfloat val)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearIndex", DT_GLfloat, val, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.clearIndex = val;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARCOLOR_BIT);
}

GLvoid APIENTRY __glim_ClearColorIiEXT(GLint r, GLint g, GLint b, GLint a)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearColorIiEXT", DT_GLint, r, DT_GLint, g, DT_GLint, b, DT_GLint, a, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.clearIi.r = r;
    gc->state.raster.clearIi.g = g;
    gc->state.raster.clearIi.b = b;
    gc->state.raster.clearIi.a = a;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARCOLOR_BIT);
}


GLvoid APIENTRY __glim_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearColorIuiEXT", DT_GLuint, r, DT_GLuint, g, DT_GLuint, b, DT_GLuint, a, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.clearIui.r = r;
    gc->state.raster.clearIui.g = g;
    gc->state.raster.clearIui.b = b;
    gc->state.raster.clearIui.a = a;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARCOLOR_BIT);
}

GLvoid APIENTRY __glim_ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    GLint i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorMask", DT_GLboolean, r, DT_GLboolean, g, DT_GLboolean, b, DT_GLboolean, a, DT_GLnull);
#endif

    __GL_DLIST_BUFFER_FLUSH(gc);

    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        gc->state.raster.colorMask[i].redMask = r;
        gc->state.raster.colorMask[i].greenMask = g;
        gc->state.raster.colorMask[i].blueMask = b;
        gc->state.raster.colorMask[i].alphaMask = a;
    }

    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
}

GLvoid APIENTRY __glim_IndexMask(GLuint mask)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IndexMask", DT_GLuint, mask, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.writeMask = mask;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
}

GLvoid APIENTRY __glim_LogicOp(GLenum op)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_LogicOp", DT_GLenum, op, DT_GLnull);
#endif

    if ((op < GL_CLEAR) || (op > GL_SET)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.logicOp = op;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_LOGICOP_BIT);
}

GLvoid APIENTRY __glim_DepthMask(GLboolean flag)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DepthMask", DT_GLboolean, flag, DT_GLnull);
#endif

    __GL_DLIST_BUFFER_FLUSH(gc);

    gc->state.depth.writeEnable = flag;

    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_DEPTHMASK_BIT );
}

GLvoid APIENTRY __glim_ClearDepth(GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearDepth", DT_GLdouble, z, DT_GLnull);
#endif

    if (z < 0.0)
        z = 0.0;
    if (z > 1.0)
        z = 1.0;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.depth.clear = (GLfloat)z;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARDEPTH_BIT);
}

GLvoid APIENTRY __glim_StencilMask(GLuint sm)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_StencilMask", DT_GLuint, sm, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL 2.0 state */
    gc->state.stencil.StencilArb.front.writeMask = sm;
    gc->state.stencil.StencilArb.back.writeMask = sm;

    /* Update GL_EXT_stencil_two_side state */
    if (gc->state.stencil.activeStencilFace == GL_FRONT)
    {
        gc->state.stencil.stencilExt.front.writeMask = sm;
    }
    else if (gc->state.stencil.activeStencilFace == GL_BACK)
    {
        gc->state.stencil.stencilExt.back.writeMask = sm;
    }
    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
        __GL_STENCILMASK_FRONT_BIT | __GL_STENCILMASK_BACK_BIT);
}

GLvoid APIENTRY __glim_StencilMaskSeparate(GLenum face, GLuint sm)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_StencilMaskSeparate", DT_GLenum, face, DT_GLuint, sm, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (face) {
      case GL_FRONT:
          gc->state.stencil.StencilArb.front.writeMask = sm;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_FRONT_BIT);
          break;

      case GL_BACK:
          gc->state.stencil.StencilArb.back.writeMask = sm;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_STENCILMASK_BACK_BIT);
          break;

      case GL_FRONT_AND_BACK:
          gc->state.stencil.StencilArb.front.writeMask = sm;
          gc->state.stencil.StencilArb.back.writeMask = sm;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,
              __GL_STENCILMASK_FRONT_BIT | __GL_STENCILMASK_BACK_BIT);
          break;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_ClearStencil(GLint s)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearStencil", DT_GLint, s, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.stencil.clear = s;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARSTENCIL_BIT);
}

GLvoid APIENTRY __glim_Hint(GLenum target, GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Hint", DT_GLenum, target, DT_GLenum, mode, DT_GLnull);
#endif

    switch (mode) {
      case GL_DONT_CARE:
      case GL_FASTEST:
      case GL_NICEST:
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* Because we don't use hint value now, we don't need flush here */
    /* __GL_VERTEX_BUFFER_FLUSH(gc); */

    switch (target) {
      case GL_PERSPECTIVE_CORRECTION_HINT:
          gc->state.hints.perspectiveCorrection = mode;
          break;

      case GL_POINT_SMOOTH_HINT:
          gc->state.hints.pointSmooth = mode;
          break;

      case GL_LINE_SMOOTH_HINT:
          gc->state.hints.lineSmooth = mode;
          break;

      case GL_POLYGON_SMOOTH_HINT:
          gc->state.hints.polygonSmooth = mode;
          break;

      case GL_FOG_HINT:
          gc->state.hints.fog = mode;
          break;

      case GL_GENERATE_MIPMAP_HINT:
          gc->state.hints.generateMipmap = mode;
          break;

      case GL_TEXTURE_COMPRESSION_HINT:
          gc->state.hints.textureCompressionHint = mode;
          break ;

      default:
          __glSetError(GL_INVALID_ENUM);
          return;
    }

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_HINT_BIT);
}

GLint APIENTRY __glim_RenderMode(GLenum mode)
{
    GLint    rv = 0;
    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_RenderMode",DT_GLenum, mode, DT_GLnull);
#endif

    switch (mode) {
      case GL_RENDER:
          break;
      case GL_FEEDBACK:
          if (!(gc->feedback.resultBase)) {
              __glSetError(GL_INVALID_OPERATION);
              return 0;
          }
          break;
      case GL_SELECT:
          if (!(gc->select.buffer)) {
              __glSetError(GL_INVALID_OPERATION);
              return 0;
          }
          break;
      default:
          __glSetError(GL_INVALID_ENUM);
          return 0;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Switch out of old render mode.  Get return value. */
    switch (gc->renderMode) {
      case GL_RENDER:
          rv = 0;
          break;
      case GL_FEEDBACK:
          rv = gc->feedback.overFlowed ? -1 : (GLint)(gc->feedback.result - gc->feedback.resultBase);
          break;
      case GL_SELECT:
          if (gc->select.hitFlag)
          {
              __glWriteHitRecord(gc);
          }

          rv = gc->select.overFlowed ? -1 : gc->select.numHit;
          break;
    }

    /* Check redundancy */
    if (gc->renderMode == mode) {
        return rv;
    }

    /* Switch to new render mode */
    gc->renderMode = mode;

    switch (mode) {
      case GL_FEEDBACK:
          if (!gc->feedback.resultBase) {
              __glSetError(GL_INVALID_OPERATION);
              return rv;
          }
          gc->feedback.result = gc->feedback.resultBase;
          gc->feedback.overFlowed = GL_FALSE;
          break;
      case GL_SELECT:
          if (!gc->select.buffer) {
              __glSetError(GL_INVALID_OPERATION);
              return rv;
          }

          gc->select.overFlowed = GL_FALSE;
          gc->select.hitFlag = GL_FALSE;
          gc->select.hitMaxZ = 0.0f;
          gc->select.hitMinZ = 1.0f;
          gc->select.bufferWrittenCount = 0;
          gc->select.numHit = 0;
          break;
    }

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_RENDERMODE_BIT);

    __GL_INPUTMASK_CHANGED(gc);

    return rv;
}

GLvoid APIENTRY __glim_ClearAccum(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat one = __glOne, minusOne = __glMinusOne;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClearAccum", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLfloat, a, DT_GLnull);
#endif

    if (r < minusOne)
        r = minusOne;
    if (r > one)
        r = one;
    if (g < minusOne)
        g = minusOne;
    if (g > one)
        g = one;
    if (b < minusOne)
        b = minusOne;
    if (b > one)
        b = one;
    if (a < minusOne)
        a = minusOne;
    if (a > one)
        a = one;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.accum.clear.r = r;
    gc->state.accum.clear.g = g;
    gc->state.accum.clear.b = b;
    gc->state.accum.clear.a = a;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CLEARACCUM_BIT);
}

GLvoid APIENTRY __glim_SampleCoverage(GLclampf value, GLboolean invert)
{
    GLfloat zero = __glZero;
    GLfloat one = __glOne;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SampleCoverage", DT_GLclampf, value, DT_GLboolean, invert, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if (value < zero)
        value = zero;
    if (value > one)
        value = one;

    gc->state.multisample.coverageValue = value;
    gc->state.multisample.coverageInvert = invert;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_SAMPLECOVERAGE_BIT);
}

#if GL_EXT_draw_buffers2

GLvoid APIENTRY __glim_ColorMaskIndexedEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorMaskIndexedEXT", DT_GLuint, buf, DT_GLboolean, r, DT_GLboolean, g, DT_GLboolean, b, DT_GLboolean, a);
#endif

    GL_ASSERT(buf < __GL_MAX_DRAW_BUFFERS);

    __GL_DLIST_BUFFER_FLUSH(gc);

    gc->state.raster.colorMask[buf].redMask = r;
    gc->state.raster.colorMask[buf].greenMask = g;
    gc->state.raster.colorMask[buf].blueMask = b;
    gc->state.raster.colorMask[buf].alphaMask = a;
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
}

#endif

#if GL_ARB_color_buffer_float
GLvoid APIENTRY __glim_ClampColorARB(GLenum target, GLenum clamp)
{
    GLenum* pClampColor;
    GLenum clampBit;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ClampColorARB", DT_GLenum, target, DT_GLenum, clamp);
#endif

    switch(target)
    {
    case GL_CLAMP_VERTEX_COLOR_ARB:
        pClampColor = &gc->state.light.clampVertexColor;
        clampBit = __GL_CLAMP_VERTEX_COLOR_BIT;
        break;
    case GL_CLAMP_FRAGMENT_COLOR_ARB:
        pClampColor = &gc->state.raster.clampFragColor;
        clampBit = __GL_CLAMP_FRAG_COLOR_BIT;
        break;
    case GL_CLAMP_READ_COLOR_ARB:
        pClampColor = &gc->state.raster.clampReadColor;
        clampBit = 0;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(clamp != GL_TRUE && clamp != GL_FALSE && clamp != GL_FIXED_ONLY_ARB)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(*pClampColor ^ clamp)
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);
        *pClampColor = clamp;
         __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, clampBit);
    }

}
#endif
