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


#ifdef OPENGL40
#include "gc_es_context.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif

__GL_INLINE GLvoid __glRect(__GLcontext *gc, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1)
{

    (*gc->currentImmediateTable->Begin)(gc, GL_QUADS);
    (*gc->currentImmediateTable->Vertex2f)(gc, x0, y0);
    (*gc->currentImmediateTable->Vertex2f)(gc, x1, y0);
    (*gc->currentImmediateTable->Vertex2f)(gc, x1, y1);
    (*gc->currentImmediateTable->Vertex2f)(gc, x0, y1);
    (*gc->currentImmediateTable->End)(gc);

}

GLvoid APIENTRY __glim_Rectd(__GLcontext *gc, GLdouble ax, GLdouble ay, GLdouble bx, GLdouble by)
{
    __glRect(gc, ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectdv(__GLcontext *gc, const GLdouble *v1, const GLdouble *v2)
{
    __glRect(gc, v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Rectf(__GLcontext *gc, GLfloat ax, GLfloat ay, GLfloat bx, GLfloat by)
{
    __glRect(gc, ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectfv(__GLcontext *gc, const GLfloat *v1, const GLfloat *v2)
{
    __glRect(gc, v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Recti(__GLcontext *gc, GLint ax, GLint ay, GLint bx, GLint by)
{
    __glRect(gc, ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectiv(__GLcontext *gc, const GLint *v1, const GLint *v2)
{
    __glRect(gc, v1[0], v1[1], v2[0], v2[1]);
}

GLvoid APIENTRY __glim_Rects(__GLcontext *gc, GLshort ax, GLshort ay, GLshort bx, GLshort by)
{
    __glRect(gc, ax, ay, bx, by);
}

GLvoid APIENTRY __glim_Rectsv(__GLcontext *gc, const GLshort *v1, const GLshort *v2)
{
    __glRect(gc, v1[0], v1[1], v2[0], v2[1]);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
#endif