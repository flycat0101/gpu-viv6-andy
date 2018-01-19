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
#include "gc_es_device.h"

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE

GLfloat __glClampWidth(GLfloat width, __GLdeviceConstants *constants)
{
    GLfloat min  = constants->lineWidthMin;
    GLfloat max  = constants->lineWidthMax;
    GLfloat gran = constants->lineWidthGranularity;
    GLint i;

    if (width <= min)
    {
        return min;
    }
    else if (width >= max)
    {
        return max;
    }
    else
    {
        i = (GLint)((width - min) / gran + 0.5);
        return (min + i * gran);
    }
}

__GL_INLINE GLvoid __glViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h)
{
    /* Clamp viewport width and height */
    if (w > (GLint)gc->constants.maxViewportWidth)
    {
        w = gc->constants.maxViewportWidth;
    }
    if (h > (GLint)gc->constants.maxViewportHeight)
    {
        h = gc->constants.maxViewportHeight;
    }

    gc->state.viewport.x = x;
    gc->state.viewport.y = y;
    gc->state.viewport.width = w;
    gc->state.viewport.height = h;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_VIEWPORT_BIT);
}

GLvoid __glUpdateViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h)
{
    __glViewport(gc, x, y, w, h);
}

GLvoid GL_APIENTRY GL_APIENTRY __gles_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GL_HEADER();

    if ((w < 0) || (h < 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glViewport(gc, x, y, w, h);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_FrontFace(__GLcontext *gc, GLenum dir)
{
    __GL_HEADER();

    if ((GL_CW != dir) && (GL_CCW != dir))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update polygon state */
    gc->state.polygon.frontFace = dir;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_FRONTFACE_BIT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_CullFace(__GLcontext *gc, GLenum cfm)
{
    __GL_HEADER();

    if ((GL_FRONT != cfm) && (GL_BACK != cfm) && (GL_FRONT_AND_BACK != cfm))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Update polygon state */
    gc->state.polygon.cullFace = cfm;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_CULLFACE_BIT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_LineWidth(__GLcontext *gc, GLfloat width)
{
    __GL_HEADER();

    if (width <= 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Update line state */
    gc->state.line.requestedWidth = width;

    width = gcmCLAMP(width, 0, gc->constants.lineWidthMax);

    /* Round */
    gc->state.line.aliasedWidth = (GLint)((width < 1.0) ? 1 : width + 0.5);

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINEWIDTH_BIT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units)
{
    __GL_HEADER();

    /* Update polygon state */
    gc->state.polygon.factor = factor;
    gc->state.polygon.units = units;

    /* Flip attribute dirty bit */
    __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_POLYGONOFFSET_BIT);

    __GL_FOOTER();
}
