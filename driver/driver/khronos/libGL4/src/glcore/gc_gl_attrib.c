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

extern GLvoid __glWriteHitRecord(__GLcontext *gc);

GLvoid APIENTRY __glim_PolygonMode(__GLcontext *gc, GLenum face, GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (((GL_FRONT != face) && (GL_BACK != face) && (GL_FRONT_AND_BACK != face)) ||
        ((GL_POINT != mode) && (GL_LINE != mode) && (GL_FILL != mode))) {
            __glSetError(gc, GL_INVALID_ENUM);
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
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_POLYGONMODE_BIT);

    __GL_INPUTMASK_CHANGED(gc);
}

GLvoid APIENTRY __glim_ShadeModel(__GLcontext *gc,GLenum sm)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if ((sm != GL_FLAT) && (sm != GL_SMOOTH)) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.light.shadingModel, sm);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.light.shadingModel = sm;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_LIGHTING_ATTRS, __GL_SHADEMODEL_BIT);
}

GLvoid APIENTRY __glim_ClearAccum(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat one = __glOne, minusOne = __glMinusOne;
    __GL_SETUP_NOT_IN_BEGIN(gc);

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
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CLEARACCUM_BIT);
}

GLvoid APIENTRY __glim_ClearIndex(__GLcontext *gc, GLfloat val)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.clearIndex = val;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_CLEARCOLOR_BIT);
}

GLvoid APIENTRY __glim_ClearDepth(__GLcontext *gc, GLdouble z)
{
    __gles_ClearDepthf(gc, (GLfloat)z);
}

GLvoid APIENTRY __glim_DepthRange(__GLcontext *gc, GLdouble zNear, GLdouble zFar)
{
    __gles_DepthRange(gc, (GLfloat)zNear, (GLfloat)zFar);
}

GLvoid APIENTRY __glim_IndexMask(__GLcontext *gc, GLuint mask)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.writeMask = mask;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
}

GLint APIENTRY __glim_RenderMode(__GLcontext *gc, GLenum mode)
{
    GLint    rv = 0;
    __GL_SETUP_NOT_IN_BEGIN_RET(gc, 0);

    switch (mode) {
      case GL_RENDER:
          break;
      case GL_FEEDBACK:
          if (!(gc->feedback.resultBase)) {
              __glSetError(gc, GL_INVALID_OPERATION);
              return 0;
          }
          break;
      case GL_SELECT:
          if (!(gc->select.buffer)) {
              __glSetError(gc, GL_INVALID_OPERATION);
              return 0;
          }
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
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
              __glSetError(gc, GL_INVALID_OPERATION);
              return rv;
          }
          gc->feedback.result = gc->feedback.resultBase;
          gc->feedback.overFlowed = GL_FALSE;
          break;
      case GL_SELECT:
          if (!gc->select.buffer) {
              __glSetError(gc, GL_INVALID_OPERATION);
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

GLvoid APIENTRY __glim_ClipPlane(__GLcontext *gc, GLenum pi, const GLdouble *pv)
{
    __GLcoord mvEqu, eyeEqu;
    __GL_SETUP_NOT_IN_BEGIN(gc);

    pi -= GL_CLIP_PLANE0;
    /* pi is GLenum type, unsigned, no need for negative check*/
    if ( (pi >= gc->constants.numberOfClipPlanes)) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    mvEqu.f.x = (GLfloat)pv[0];
    mvEqu.f.y = (GLfloat)pv[1];
    mvEqu.f.z = (GLfloat)pv[2];
    mvEqu.f.w = (GLfloat)pv[3];

    /* Transform ClipPlane equation into eye space.
    */
    __glTransformVector(gc, &eyeEqu, &mvEqu, gc->transform.modelView, GL_FALSE);

    /* Update GL state */
    gc->state.transform.eyeClipPlanes[pi].f.x = eyeEqu.f.x;
    gc->state.transform.eyeClipPlanes[pi].f.y = eyeEqu.f.y;
    gc->state.transform.eyeClipPlanes[pi].f.z = eyeEqu.f.z;
    gc->state.transform.eyeClipPlanes[pi].f.w = eyeEqu.f.w;

    /* The lower 16 bits of globaldirtyState[__GL_CLIP_ATTRS] are for
    * clipPlane parameters.
    */
    gc->globalDirtyState[__GL_CLIP_ATTRS] |= (1 << pi);
    gc->globalDirtyState[__GL_ALL_ATTRS] |= (1 << __GL_CLIP_ATTRS);
}

GLvoid APIENTRY __glim_LogicOp(__GLcontext *gc, GLenum op)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if ((op < GL_CLEAR) || (op > GL_SET)) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.raster.logicOp = op;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LOGICOP_BIT);
}

GLvoid APIENTRY __glim_PointSize(__GLcontext *gc, GLfloat size)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    if (size <= 0.0) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    __GL_REDUNDANT_ATTR(gc->state.point.requestedSize, size);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Update GL state */
    gc->state.point.requestedSize = size;

    /* flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINTSIZE_BIT);
}

GLvoid APIENTRY __glim_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = (GLenum)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_DISTANCE_ATTENUATION:
          gc->state.point.distanceAttenuation[0] = params[0];
          gc->state.point.distanceAttenuation[1] = params[1];
          gc->state.point.distanceAttenuation[2] = params[2];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_DISTANCE_ATTENUATION_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = (GLenum)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameteri(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = (GLfloat)param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = param;
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname) {
      case GL_POINT_SIZE_MIN:
          gc->state.point.sizeMin = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MIN_BIT);
          break;
      case GL_POINT_SIZE_MAX:
          gc->state.point.sizeMax = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_SIZE_MAX_BIT);
          break;
      case GL_POINT_FADE_THRESHOLD_SIZE:
          gc->state.point.fadeThresholdSize = (GLfloat)params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_FADE_THRESHOLD_SIZE_BIT);
          break;
      case GL_POINT_DISTANCE_ATTENUATION:
          gc->state.point.distanceAttenuation[0] = (GLfloat)params[0];
          gc->state.point.distanceAttenuation[1] = (GLfloat)params[1];
          gc->state.point.distanceAttenuation[2] = (GLfloat)params[2];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINT_DISTANCE_ATTENUATION_BIT);
          break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
          gc->state.point.coordOrigin = params[0];
          __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_3, __GL_POINTSPRITE_COORD_ORIGIN_BIT);
          break;
      default:
          __glSetError(gc, GL_INVALID_ENUM);
          return;
    }
}

GLvoid APIENTRY __glim_LineStipple(__GLcontext *gc, GLint factor, GLushort stipple)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

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

GLvoid APIENTRY __glim_PolygonStipple(__GLcontext *gc, const GLubyte *mask)
{
    GLubyte stipple[4*32];

    __GL_SETUP_NOT_IN_BEGIN(gc);

    __glFillImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, mask, stipple);

    if (__GL_MEMCMP(gc->state.polygonStipple.stipple, stipple, sizeof(stipple)) != 0)
    {
        __GL_VERTEX_BUFFER_FLUSH(gc);

        __GL_MEMCOPY(gc->state.polygonStipple.stipple, stipple, sizeof(stipple));

        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_POLYGONSTIPPLE_BIT);
    }
}

#if GL_ARB_color_buffer_float
GLvoid APIENTRY __glim_ClampColor(__GLcontext *gc, GLenum target, GLenum clamp)
{
    GLenum* pClampColor;
    GLenum clampBit;
    __GL_SETUP_NOT_IN_BEGIN(gc);

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
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    if(clamp != GL_TRUE && clamp != GL_FALSE && clamp != GL_FIXED_ONLY_ARB)
    {
        __glSetError(gc, GL_INVALID_ENUM);
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

