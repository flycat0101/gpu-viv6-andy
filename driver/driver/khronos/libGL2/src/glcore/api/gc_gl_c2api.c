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


__GL_INLINE GLvoid __glSecondaryColor3fv(GLfloat *v)
{
    __GL_SETUP();
    GLfloat *current;

    if (gc->input.preVertexFormat & __GL_SC3F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_SC3F_BIT) == 0)
        {
            gc->input.color2.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.color2.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_SC3F_BIT;
    }
    else
    {
        if (((gc->input.currentInputMask & __GL_INPUT_SPECULAR) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))

        {
            /* If glSecondaryColor is not needed in glBegin/glEnd */
            gc->state.current.color2.r = v[0];
            gc->state.current.color2.g = v[1];
            gc->state.current.color2.b = v[2];
            gc->state.current.color2.a = 1.0;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first glSecondaryColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glSecondaryColor after glBegin */
            gc->input.color2.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color2.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color2.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color2.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_SC3F_BIT;
            current = gc->input.color2.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_SC3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_SC3F_TAG);
        }
        else if (gc->input.preVertexFormat != 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_SC3F_INDEX);

            gc->input.color2.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.color2.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_SC3F_BIT;
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->state.current.color2.r == v[0]) &&
                    (gc->state.current.color2.g == v[1]) &&
                    (gc->state.current.color2.b == v[2]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.color2.currentPtrDW = (GLfloat*)gc->input.color2.pointer +
                gc->input.color2.index * gc->input.vertTotalStrideDW;
            current = gc->input.color2.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.color2.index += 1;
            gc->input.vertexFormat |= __GL_SC3F_BIT;
        }
    }
}

/* OpenGL secondary color APIs */

GLvoid APIENTRY __glim_SecondaryColor3b(GLbyte r, GLbyte g, GLbyte b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3b",DT_GLbyte, r, DT_GLbyte,g, DT_GLbyte, b, DT_GLnull);
#endif


    fv[0] = __GL_B_TO_FLOAT(r);
    fv[1] = __GL_B_TO_FLOAT(g);
    fv[2] = __GL_B_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3bv(const GLbyte *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3bv",DT_GLbyte_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3d(GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3d", DT_GLdouble, r, DT_GLdouble, g, DT_GLdouble, b, DT_GLnull);
#endif

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3dv(const GLdouble *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3f", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLnull);
#endif


    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3fv(const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glSecondaryColor3fv( (GLfloat *)v );
}

GLvoid APIENTRY __glim_SecondaryColor3i(GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3iv(const GLint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3s(GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3s", DT_GLshort, r, DT_GLshort, g, DT_GLshort, b, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3sv(const GLshort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3ub", DT_GLbyte, r, DT_GLbyte, g, DT_GLbyte, b, DT_GLnull);
#endif

    fv[0] = __GL_UB_TO_FLOAT(r);
    fv[1] = __GL_UB_TO_FLOAT(g);
    fv[2] = __GL_UB_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ubv(const GLubyte *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3ubv", DT_GLubyte_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ui(GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3ui", DT_GLuint, r, DT_GLuint, g, DT_GLuint, b, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3uiv(const GLuint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3uiv", DT_GLuint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3us(GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3us", DT_GLushort, r, DT_GLushort, g, DT_GLushort, b, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glSecondaryColor3fv( fv );
}

GLvoid APIENTRY __glim_SecondaryColor3usv(const GLushort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SecondaryColor3usv", DT_GLushort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glSecondaryColor3fv( fv );
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
