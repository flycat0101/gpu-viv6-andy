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


#include "gc_gl_context.h"
#include "gc_gl_debug.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif

__GL_INLINE GLvoid __glRect(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1)
{
    __GL_SETUP();

    (*gc->currentImmediateTable->dispatch.Begin)(GL_QUADS);
    (*gc->currentImmediateTable->dispatch.Vertex2f)(x0, y0);
    (*gc->currentImmediateTable->dispatch.Vertex2f)(x1, y0);
    (*gc->currentImmediateTable->dispatch.Vertex2f)(x1, y1);
    (*gc->currentImmediateTable->dispatch.Vertex2f)(x0, y1);
    (*gc->currentImmediateTable->dispatch.End)();
}

GLvoid APIENTRY __glim_Rectd(GLdouble ax, GLdouble ay, GLdouble bx, GLdouble by)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectd", DT_GLdouble, ax, DT_GLdouble, ay, DT_GLdouble, bx, DT_GLdouble, by, DT_GLnull);
#endif

    __glRect(ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectdv", DT_GLdouble_ptr, v1, DT_GLdouble_ptr, v2, DT_GLnull);
#endif

    __glRect(v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Rectf(GLfloat ax, GLfloat ay, GLfloat bx, GLfloat by)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectf", DT_GLfloat, ax, DT_GLfloat, ay, DT_GLfloat, bx, DT_GLfloat, by, DT_GLnull);
#endif

    __glRect(ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectfv", DT_GLfloat_ptr, v1, DT_GLfloat_ptr, v2, DT_GLnull);
#endif

    __glRect(v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Recti(GLint ax, GLint ay, GLint bx, GLint by)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Recti", DT_GLint, ax, DT_GLint, ay, DT_GLint, bx, DT_GLint, by, DT_GLnull);
#endif

    __glRect(ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectiv(const GLint *v1, const GLint *v2)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectiv", DT_GLint_ptr, v1, DT_GLint_ptr, v2, DT_GLnull);
#endif

    __glRect(v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Rects(GLshort ax, GLshort ay, GLshort bx, GLshort by)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rects", DT_GLshort, ax, DT_GLshort, ay, DT_GLshort, bx, DT_GLshort, by, DT_GLnull);
#endif

    __glRect(ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectsv(const GLshort *v1, const GLshort *v2)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Rectsv", DT_GLshort_ptr, v1, DT_GLshort_ptr, v2, DT_GLnull);
#endif

    __glRect(v1[0], v1[1], v2[0], v2[1]);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
