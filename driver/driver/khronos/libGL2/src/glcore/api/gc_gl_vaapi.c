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
#pragma warning(disable: 4003)
#pragma warning(disable: 4133)
#endif

#ifdef GL_EXT_gpu_shader4
__GL_INLINE GLvoid __glVertexAttrib4fv(GLuint index, GLfloat *v)
{
    __GL_SETUP();
    GLuint *current;
    GLuint *iv = (GLuint *)v;
    GLuint64 at4fMask = (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + index));

    if (index == 0)
    {
        (*gc->currentImmediateTable->dispatch.Vertex4fv)(v);
        return;
    }

    if (gc->input.preVertexFormat & at4fMask)
    {
        if ((gc->input.vertexFormat & at4fMask) == 0)
        {
            gc->input.attribute[index].currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = ( GLuint * ) gc->input.attribute[index].currentPtrDW;
        current[0] = iv[0];
        current[1] = iv[1];
        current[2] = iv[2];
        current[3] = iv[3];
        gc->input.vertexFormat |= at4fMask;
    }
    else
    {
        if ((gc->input.currentInputMask & (__GL_ONE_32 << (__GL_INPUT_ATT0_INDEX + index))) == 0)
        {
            /* If glAttribute is not needed in glBegin/glEnd */
            gc->state.current.attribute[index].ix = iv[0];
            gc->state.current.attribute[index].iy = iv[1];
            gc->state.current.attribute[index].iz = iv[2];
            gc->state.current.attribute[index].iw = iv[3];
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first glAttribute after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glAttribute after glBegin */
            gc->input.attribute[index].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.attribute[index].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.attribute[index].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.attribute[index].sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.preVertexFormat |= at4fMask;
            current = ( GLuint * )gc->input.attribute[index].currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.vertexFormat |= at4fMask;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_AT4F_I0_TAG + index);
        }
        else if (gc->input.preVertexFormat != 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_AT4F_I0_INDEX + index);

            gc->input.attribute[index].currentPtrDW += gc->input.vertTotalStrideDW;
            current =( GLuint * ) gc->input.attribute[index].currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.vertexFormat |= at4fMask;
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->state.current.attribute[index].ix == iv[0]) &&
                    (gc->state.current.attribute[index].iy == iv[1]) &&
                    (gc->state.current.attribute[index].iz == iv[2]) &&
                    (gc->state.current.attribute[index].iw == iv[3]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.attribute[index].currentPtrDW = (GLfloat*)gc->input.attribute[index].pointer +
                gc->input.attribute[index].index * gc->input.vertTotalStrideDW;
            current = ( GLuint * )gc->input.attribute[index].currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.attribute[index].index += 1;
            gc->input.vertexFormat |= at4fMask;
        }
    }
}

#define __GL_VERTEXATTRIB4F(index, x, y, z, w)              \
    fv[0] = x;                                              \
    fv[1] = y;                                              \
    fv[2] = z;                                              \
    fv[3] = w;                                              \
    __glVertexAttrib4fv( index, (GLfloat *)fv );            \


#else

__GL_INLINE GLvoid __glVertexAttrib4fv(GLuint index, GLfloat *v)
{
    __GL_SETUP();
    GLfloat *current;
    GLuint64 at4fMask = (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + index));

    if (index == 0)
    {
        (*gc->currentImmediateTable->dispatch.Vertex4fv)(v);
        return;
    }

    if (gc->input.preVertexFormat & at4fMask)
    {
        if ((gc->input.vertexFormat & at4fMask) == 0)
        {
            gc->input.attribute[index].currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.attribute[index].currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        current[3] = v[3];
        gc->input.vertexFormat |= at4fMask;
    }
    else
    {
        if ((gc->input.currentInputMask & (__GL_ONE_32 << (__GL_INPUT_ATT0_INDEX + index))) == 0)
        {
            /* If glAttribute is not needed in glBegin/glEnd */
            gc->state.current.attribute[index].x = v[0];
            gc->state.current.attribute[index].y = v[1];
            gc->state.current.attribute[index].z = v[2];
            gc->state.current.attribute[index].w = v[3];
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first glAttribute after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glAttribute after glBegin */
            gc->input.attribute[index].offsetDW = gc->input.currentDataBufPtr - gc->input.primBeginAddr;
            gc->input.attribute[index].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.attribute[index].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.attribute[index].sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.preVertexFormat |= at4fMask;
            current = gc->input.attribute[index].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= at4fMask;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_AT4F_I0_TAG + index);
        }
        else if (gc->input.preVertexFormat != 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_AT4F_I0_INDEX + index);

            gc->input.attribute[index].currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.attribute[index].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= at4fMask;
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->state.current.attribute[index].x == v[0]) &&
                    (gc->state.current.attribute[index].y == v[1]) &&
                    (gc->state.current.attribute[index].z == v[2]) &&
                    (gc->state.current.attribute[index].w == v[3]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.attribute[index].currentPtrDW = (GLfloat*)gc->input.attribute[index].pointer +
                gc->input.attribute[index].index * gc->input.vertTotalStrideDW;
            current = gc->input.attribute[index].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.attribute[index].index += 1;
            gc->input.vertexFormat |= at4fMask;
        }
    }
}


#define __GL_VERTEXATTRIB4F(index, x, y, z, w)              \
    fv[0] = x;                                              \
    fv[1] = y;                                              \
    fv[2] = z;                                              \
    fv[3] = w;                                              \
    __glVertexAttrib4fv( index, fv );                       \

#endif

/* OpenGL vertex attribute APIs */

GLvoid APIENTRY __glim_VertexAttrib1s(GLuint index, GLshort x)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1s", DT_GLuint, index, DT_GLshort, x, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib1f(GLuint index, GLfloat x)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1f", DT_GLuint, index, DT_GLfloat, x, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib1d(GLuint index, GLdouble x)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1d", DT_GLuint, index, DT_GLdouble, x, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2s", DT_GLuint, index, DT_GLshort, x, DT_GLshort, y, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2f", DT_GLuint, index, DT_GLfloat, x, DT_GLfloat, y, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2d", DT_GLuint, index, DT_GLdouble, x, DT_GLdouble, y, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3s", DT_GLuint, index, DT_GLshort, x, DT_GLshort, y, DT_GLshort, z, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3f", DT_GLuint, index, DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3d", DT_GLuint, index, DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4s", DT_GLuint, index, DT_GLshort, x, DT_GLshort, y, DT_GLshort, z, DT_GLshort, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, w);
}

GLvoid APIENTRY __glim_VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4f", DT_GLuint, index, DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLfloat, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, w);
}

GLvoid APIENTRY __glim_VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4d", DT_GLuint, index, DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLdouble, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, w);
}

GLvoid APIENTRY __glim_VertexAttrib1sv(GLuint index, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1sv", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib1fv(GLuint index, const GLfloat *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1fv", DT_GLuint, index, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib1dv(GLuint index, const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib1dv", DT_GLuint, index, DT_GLdouble_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], 0.0f, 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2sv(GLuint index, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2sv", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2fv(GLuint index, const GLfloat *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2fv", DT_GLuint, index, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib2dv(GLuint index, const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib2dv", DT_GLuint, index, DT_GLdouble_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], 0.0f, 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3sv(GLuint index, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3sv", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3fv(GLuint index, const GLfloat *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3fv", DT_GLuint, index, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib3dv(GLuint index, const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib3dv", DT_GLuint, index, DT_GLdouble_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], 1.0f);
}

GLvoid APIENTRY __glim_VertexAttrib4bv(GLuint index, const GLbyte *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4bv", DT_GLuint, index, DT_GLbyte_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4sv(GLuint index, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4sv", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4iv(GLuint index, const GLint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4iv", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4ubv", DT_GLuint, index, DT_GLubyte_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4usv(GLuint index, const GLushort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4usv", DT_GLuint, index, DT_GLushort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4uiv(GLuint index, const GLuint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4uiv", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4fv(GLuint index, const GLfloat *v)
{

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4fv", DT_GLuint, index, DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glVertexAttrib4fv( index, (GLfloat *)v );
}

GLvoid APIENTRY __glim_VertexAttrib4dv(GLuint index, const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4dv", DT_GLuint, index, DT_GLdouble_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nub", DT_GLuint, index, DT_GLubyte, x, DT_GLubyte, y, DT_GLubyte, z, DT_GLubyte, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, __GL_UB_TO_FLOAT(x), __GL_UB_TO_FLOAT(y),
        __GL_UB_TO_FLOAT(z), __GL_UB_TO_FLOAT(w));
}

GLvoid APIENTRY __glim_VertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nbv", DT_GLuint, index, DT_GLbyte_ptr, v, DT_GLnull);
#endif


    __GL_VERTEXATTRIB4F(index, __GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
        __GL_B_TO_FLOAT(v[2]), __GL_B_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nsv", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, __GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
        __GL_S_TO_FLOAT(v[2]), __GL_S_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Niv(GLuint index, const GLint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Niv", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif


    __GL_VERTEXATTRIB4F(index, __GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
        __GL_I_TO_FLOAT(v[2]), __GL_I_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nubv", DT_GLuint, index, DT_GLubyte_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, __GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
        __GL_UB_TO_FLOAT(v[2]), __GL_UB_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nusv", DT_GLuint, index, DT_GLushort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, __GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
        __GL_US_TO_FLOAT(v[2]), __GL_US_TO_FLOAT(v[3]));
}

GLvoid APIENTRY __glim_VertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttrib4Nuiv", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, __GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
        __GL_UI_TO_FLOAT(v[2]), __GL_UI_TO_FLOAT(v[3]));
}


#if GL_EXT_gpu_shader4
GLvoid APIENTRY __glim_VertexAttribI1iEXT(GLuint index, GLint x)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI1iEXT", DT_GLuint, index, DT_GLint, x, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, 0, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI2iEXT(GLuint index, GLint x, GLint y)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI2iEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI3iEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, 0);
}

GLvoid APIENTRY __glim_VertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4iEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLint, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, w);
}

GLvoid APIENTRY __glim_VertexAttribI1uiEXT(GLuint index, GLuint x)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI1uiEXT", DT_GLuint, index, DT_GLint, x, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, 0, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI2uiEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI3uiEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, 0);
}

GLvoid APIENTRY __glim_VertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z,
                            GLuint w)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4uiEXT", DT_GLuint, index, DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLint, w, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, x, y, z, w);
}

GLvoid APIENTRY __glim_VertexAttribI1ivEXT(GLuint index, const GLint *v)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI1ivEXT", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], 0, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI2ivEXT(GLuint index, const GLint *v)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI2ivEXT", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI3ivEXT(GLuint index, const GLint *v)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI3ivEXT", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], 0);
}

GLvoid APIENTRY __glim_VertexAttribI4ivEXT(GLuint index, const GLint *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4ivEXT", DT_GLuint, index, DT_GLint_ptr, v, DT_GLnull);
#endif

    __glVertexAttrib4fv(index, (GLfloat*)v);
}

GLvoid APIENTRY __glim_VertexAttribI1uivEXT(GLuint index, const GLuint *v)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI1uivEXT", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], 0, 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI2uivEXT(GLuint index, const GLuint *v)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI2uivEXT", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], 0, 0);
}

GLvoid APIENTRY __glim_VertexAttribI3uivEXT(GLuint index, const GLuint *v)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI3uivEXT", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], 0);
}

GLvoid APIENTRY __glim_VertexAttribI4uivEXT(GLuint index, const GLuint *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4uivEXT", DT_GLuint, index, DT_GLuint_ptr, v, DT_GLnull);
#endif

    __glVertexAttrib4fv(index, (GLfloat*)v );
}

GLvoid APIENTRY __glim_VertexAttribI4bvEXT(GLuint index, const GLbyte *v)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4bvEXT", DT_GLuint, index, DT_GLbyte_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttribI4svEXT(GLuint index, const GLshort *v)
{
    GLint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4svEXT", DT_GLuint, index, DT_GLshort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttribI4ubvEXT(GLuint index, const GLubyte *v)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4ubvEXT", DT_GLuint, index, DT_GLubyte_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_VertexAttribI4usvEXT(GLuint index, const GLushort *v)
{
    GLuint fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_VertexAttribI4usvEXT", DT_GLuint, index, DT_GLushort_ptr, v, DT_GLnull);
#endif

    __GL_VERTEXATTRIB4F(index, v[0], v[1], v[2], v[3]);
}
#endif

/************************************************************************/

#if defined(_WIN32)
#pragma warning(default: 4244)
#pragma warning(default: 4003)
#pragma warning(default: 4133)
#endif

