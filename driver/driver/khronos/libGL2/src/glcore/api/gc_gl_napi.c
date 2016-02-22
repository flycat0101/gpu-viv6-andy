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
#include "gc_gl_api_inline.c"
#include "gc_gl_debug.h"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif

__GL_INLINE GLvoid __glNormal3fv(GLfloat *v)
{
    __GL_SETUP();
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
            gc->state.current.normal.x = v[0];
            gc->state.current.normal.y = v[1];
            gc->state.current.normal.z = v[2];
            gc->state.current.normal.w = 1.0;
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
                if ((gc->state.current.normal.x == v[0]) &&
                    (gc->state.current.normal.y == v[1]) &&
                    (gc->state.current.normal.z == v[2]))
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

GLvoid APIENTRY __glim_Normal3d(GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3d", DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3dv(const GLdouble *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3f", DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3fv(const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glNormal3fv( (GLfloat *)v );
}

GLvoid APIENTRY __glim_Normal3b(GLbyte x, GLbyte y, GLbyte z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3b", DT_GLbyte, x, DT_GLbyte, y, DT_GLbyte, z, DT_GLnull);
#endif

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3bv(const GLbyte *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3bv", DT_GLbyte_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3s(GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3s", DT_GLshort, x, DT_GLshort, y, DT_GLshort, z, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3sv(const GLshort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3i(GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3i", DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv( fv );
}

GLvoid APIENTRY __glim_Normal3iv(const GLint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Normal3iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv( fv );
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

