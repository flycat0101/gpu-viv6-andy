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


/*
** Fog code.
**
*/
#include "gc_es_context.h"


__GL_INLINE GLvoid
__glFogfv(__GLcontext *gc, GLenum pname, GLfloat pv[])
{
    __GLfogState *fs = &(gc->state.fog);
    GLenum param;
    GLfloat r;

    switch (pname) {
      case GL_FOG_COLOR:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        __GL_MEMCOPY(&fs->color, pv, 4*sizeof(GLfloat));
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGCOLOR_BIT);
        break;

      case GL_FOG_DENSITY:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        if (pv[0] < 0.0) {
            __glSetError(gc, GL_INVALID_VALUE);
            return;
        }
        fs->density = (GLfloat)pv[0];
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGDENSITY_BIT);
        break;

      case GL_FOG_END:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        fs->end = (GLfloat)pv[0];
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGEND_BIT);
        break;

      case GL_FOG_START:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        fs->start = (GLfloat)pv[0];
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGSTART_BIT);
        break;

      case GL_FOG_INDEX:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        r = (GLfloat)(((GLint)pv[0]) & ((GLint)((1 << gc->modes.rgbaBits) - 1)));
        fs->index = r;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGINDEX_BIT);
        break;

      case GL_FOG_MODE:
        param = (GLenum)pv[0];
        switch(param) {
        case GL_LINEAR:
        case GL_EXP:
        case GL_EXP2:
            break;
        default:
            __glSetError(gc, GL_INVALID_ENUM);
            return;
        }

        __GL_VERTEX_BUFFER_FLUSH(gc);
        fs->mode = param;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGMODE_BIT);
        break;

      case GL_FOG_COORD_SRC:
        __GL_VERTEX_BUFFER_FLUSH(gc);
        param = (GLenum)pv[0];
        switch(param) {
          case GL_FRAGMENT_DEPTH:
          case GL_FOG_COORD:
            break;
          default:
            __glSetError(gc, GL_INVALID_ENUM);
            return;
        }
        fs->coordSource = param;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_FOGCOORDSRC_BIT);
        /* xiangchen __GL_INPUTMASK_CHANGED(gc);*/
        break;

      default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_Fogfv(__GLcontext *gc,GLenum pname, const GLfloat *pv)
{
    __GL_SETUP_NOT_IN_BEGIN(gc);

    __glFogfv(gc, pname, (GLfloat*)pv);
}

GLvoid APIENTRY __glim_Fogf(__GLcontext *gc,GLenum pname, GLfloat f)
{
    GLfloat tmpf[4];
   __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_START:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
      case GL_FOG_COORD_SRC:
        tmpf[0] = f;
        __glFogfv(gc, pname, (GLfloat*)tmpf);
        break;
      default:
        __glSetError(gc, GL_INVALID_ENUM);
        break;
    }
}

GLvoid APIENTRY __glim_Fogiv(__GLcontext *gc,GLenum pname, const GLint *pv)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN(gc);

    switch (pname) {
      case GL_FOG_COLOR:
        tmpf[0] = __GL_I_TO_FLOAT(pv[0]);
        tmpf[1] = __GL_I_TO_FLOAT(pv[1]);
        tmpf[2] = __GL_I_TO_FLOAT(pv[2]);
        tmpf[3] = __GL_I_TO_FLOAT(pv[3]);
        break;
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_START:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
      case GL_FOG_COORD_SRC:
        tmpf[0] = (GLfloat)pv[0];
        break;
      default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __glFogfv(gc, pname, (GLfloat*)tmpf);
}

GLvoid APIENTRY __glim_Fogi(__GLcontext *gc,GLenum pname, GLint i)
{
    GLfloat tmpf[4];
    __GL_SETUP_NOT_IN_BEGIN(gc);

    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_START:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
      case GL_FOG_COORD_SRC:
         tmpf[0] = (GLfloat)i;
        __glFogfv(gc, pname, (GLfloat*)tmpf);
        break;
      default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }
}

GLvoid __glInitFogState(__GLcontext *gc)
{
    __GLfogState *fs = &gc->state.fog;

    fs->coordSource = GL_FRAGMENT_DEPTH;
    fs->mode = GL_EXP;
    fs->density = __glOne;
    fs->end = __glOne;
    fs->color.r = fs->color.g = fs->color.b = fs->color.a = __glZero;
}
