/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_gl_api_inline.c"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif
__GL_INLINE GLvoid __glNormal3fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;

    if (gc->input.preVertexFormat & __GL_N3F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_N3F_BIT) == 0)
        {
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        }

        current = gc->input.normal.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_N3F_BIT;
    }
    else
    {
        if (((gc->input.currentInputMask & __GL_INPUT_NORMAL) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))
        {
            /* If glNormal is not needed in glBegin/glEnd */
            gc->state.current.normal.f.x = v[0];
            gc->state.current.normal.f.y = v[1];
            gc->state.current.normal.f.z = v[2];
            gc->state.current.normal.f.w = 1.0;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first glNormal after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glNormal after glBegin */
            gc->input.normal.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.normal.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.normal.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_N3F_BIT;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_N3F_TAG);
        }
        else if (gc->input.preVertexFormat != 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_N3F_INDEX);

            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->state.current.normal.f.x == v[0]) &&
                    (gc->state.current.normal.f.y == v[1]) &&
                    (gc->state.current.normal.f.z == v[2]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.normal.currentPtrDW = (GLfloat*)gc->input.normal.pointer +
                gc->input.normal.index * gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.normal.index += 1;
            gc->input.vertexFormat |= __GL_N3F_BIT;
        }
    }
}

/* OpenGL normal APIs */

GLvoid APIENTRY __glim_Normal3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3fv(__GLcontext *gc, const GLfloat *v)
{
    __glNormal3fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Normal3b(__GLcontext *gc, GLbyte x, GLbyte y, GLbyte z)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3bv(__GLcontext *gc, const GLbyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv( gc, fv );
}

GLvoid APIENTRY __glim_Normal3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv( gc, fv );
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

