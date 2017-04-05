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


#ifdef OPENGL40
#include "gc_es_context.h"
#include "gc_gl_api_inline.c"
#include "gc_gl_debug.h"

extern GLuint fmtIndex2InputIndex[];

#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


__GL_INLINE GLvoid __glVertex2fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
//    __GL_SETUP();


    gc->input.vertexFormat |= __GL_V2F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        gc->input.vertex.index++;
    }
    else
    {
        if ((gc->input.preVertexFormat & gc->input.vertexFormat) == gc->input.vertexFormat &&
            (gc->input.deferredAttribDirty & __GL_DEFERED_NORCOL_MASK) == 0)
        {
            /* Fill the missing attributes for the current vertex from the previous vertex */
            __glDuplicatePreviousAttrib(gc);

            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            gc->input.vertex.index++;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            /* The first glVertex after glBegin has different format from the previous primitives */
            if (gc->input.lastVertexIndex != 0) {
                __glConsistentFormatChange(gc);
            }

            /* For the first glVertex after glBegin */
            gc->input.vertex.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.vertex.sizeDW = 2;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 2;
            gc->input.vertTotalStrideDW = gc->input.vertex.offsetDW + 2;
            gc->input.preVertexFormat = gc->input.vertexFormat;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V2F_TAG);
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.vertexFormat &= ~__GL_V2F_BIT;
            gc->input.vertexFormat |= __GL_V4F_BIT;
            if (gc->input.vertexFormat != gc->input.primitiveFormat)
            {
                __glFillMissingAttributes(gc);
            }

            gc->input.vertex.currentPtrDW = (GLfloat*)gc->input.vertex.pointer +
                gc->input.vertex.index * gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = 0.0;
            current[3] = 1.0;
            gc->input.vertex.index++;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (gc->input.vertex.currentPtrDW > gc->input.defaultDataBufEnd))
    {
        __glImmediateFlushBuffer(gc);
    }
}

/* OpenGL vertex APIs */

__GL_INLINE GLvoid __glVertex3fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
//    __GL_SETUP();

    gc->input.vertexFormat |= __GL_V3F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertex.index++;
    }
    else
    {
        if ((gc->input.preVertexFormat & gc->input.vertexFormat) == gc->input.vertexFormat &&
            (gc->input.deferredAttribDirty & __GL_DEFERED_NORCOL_MASK) == 0)
        {
            /* Fill the missing attributes for the current vertex from the previous vertex */
            __glDuplicatePreviousAttrib(gc);

            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertex.index++;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            /* The first glVertex after glBegin has different format from the previous primitives */
            if (gc->input.lastVertexIndex != 0) {
                __glConsistentFormatChange(gc);
            }

            /* For the first glVertex after glBegin */
            gc->input.vertex.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.vertex.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.vertTotalStrideDW = gc->input.vertex.offsetDW + 3;
            gc->input.preVertexFormat = gc->input.vertexFormat;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V3F_TAG);
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.vertexFormat &= ~__GL_V3F_BIT;
            gc->input.vertexFormat |= __GL_V4F_BIT;
            if (gc->input.vertexFormat != gc->input.primitiveFormat)
            {
                __glFillMissingAttributes(gc);
            }

            gc->input.vertex.currentPtrDW = (GLfloat*)gc->input.vertex.pointer +
                gc->input.vertex.index * gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = 1.0;
            gc->input.vertex.index++;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (gc->input.vertex.currentPtrDW > gc->input.defaultDataBufEnd))
    {
        __glImmediateFlushBuffer(gc);
    }
}

#ifdef GL_EXT_gpu_shader4
__GL_INLINE GLvoid __glVertex4fv(__GLcontext *gc, GLfloat *v)
{
    GLuint *current;
    GLuint *iv = (GLuint *)v;
//    __GL_SETUP();

    gc->input.vertexFormat |= __GL_V4F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = (GLuint *)gc->input.vertex.currentPtrDW;
        current[0] = iv[0];
        current[1] = iv[1];
        current[2] = iv[2];
        current[3] = iv[3];
        gc->input.vertex.index++;
    }
    else
    {
        if ((gc->input.preVertexFormat & gc->input.vertexFormat) == gc->input.vertexFormat &&
            (gc->input.deferredAttribDirty & __GL_DEFERED_NORCOL_MASK) == 0)
        {
            /* Fill the missing attributes for the current vertex from the previous vertex */
            __glDuplicatePreviousAttrib(gc);

            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            current = (GLuint *)gc->input.vertex.currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.vertex.index++;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            /* The first glVertex after glBegin has different format from the previous primitives */
            if (gc->input.lastVertexIndex != 0) {
                __glConsistentFormatChange(gc);
            }

            /* For the first glVertex after glBegin */
            gc->input.vertex.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.vertex.sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.vertTotalStrideDW = gc->input.vertex.offsetDW + 4;
            gc->input.preVertexFormat = gc->input.vertexFormat;
            current = (GLuint *)gc->input.vertex.currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V4F_TAG);
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                __glSwitchToInconsistentFormat(gc);
            }

            if (gc->input.vertexFormat != gc->input.primitiveFormat)
            {
                __glFillMissingAttributes(gc);
            }

            gc->input.vertex.currentPtrDW = (GLfloat*)gc->input.vertex.pointer +
                gc->input.vertex.index * gc->input.vertTotalStrideDW;
            current = (GLuint *)gc->input.vertex.currentPtrDW;
            current[0] = iv[0];
            current[1] = iv[1];
            current[2] = iv[2];
            current[3] = iv[3];
            gc->input.vertex.index++;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (gc->input.vertex.currentPtrDW > gc->input.defaultDataBufEnd))
    {
        __glImmediateFlushBuffer(gc);
    }
}
#else
__GL_INLINE GLvoid __glVertex4fv(__GLcontext *gc, GLfloat *v)
{
    __GL_SETUP();
//    GLfloat *current;

    gc->input.vertexFormat |= __GL_V4F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        current[3] = v[3];
        gc->input.vertex.index++;
    }
    else
    {
        if ((gc->input.preVertexFormat & gc->input.vertexFormat) == gc->input.vertexFormat &&
            (gc->input.deferredAttribDirty & __GL_DEFERED_NORCOL_MASK) == 0)
        {
            /* Fill the missing attributes for the current vertex from the previous vertex */
            __glDuplicatePreviousAttrib(gc);

            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertex.index++;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            /* The first glVertex after glBegin has different format from the previous primitives */
            if (gc->input.lastVertexIndex != 0) {
                __glConsistentFormatChange(gc);
            }

            /* For the first glVertex after glBegin */
            gc->input.vertex.offsetDW = gc->input.currentDataBufPtr - gc->input.primBeginAddr;
            gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.vertex.sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.vertTotalStrideDW = gc->input.vertex.offsetDW + 4;
            gc->input.preVertexFormat = gc->input.vertexFormat;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V4F_TAG);
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                __glSwitchToInconsistentFormat(gc);
            }

            if (gc->input.vertexFormat != gc->input.primitiveFormat)
            {
                __glFillMissingAttributes(gc);
            }

            gc->input.vertex.currentPtrDW = (GLfloat*)gc->input.vertex.pointer +
                gc->input.vertex.index * gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertex.index++;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (gc->input.vertex.currentPtrDW > gc->input.defaultDataBufEnd))
    {
        __glImmediateFlushBuffer(gc);
    }
}
#endif


GLvoid APIENTRY __glim_Vertex2f(__GLcontext *gc, GLfloat x, GLfloat y)
{

    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2f", DT_GLfloat, x, DT_GLfloat, y, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2fv(__GLcontext *gc, const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2fv",DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glVertex2fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2d", DT_GLdouble, x, DT_GLdouble, y, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif


    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2i(__GLcontext *gc, GLint x, GLint y)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2i", DT_GLint, x, DT_GLint, y, DT_GLnull);
#endif


    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2iv", DT_GLint_ptr, v, DT_GLnull);
#endif


    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2s(__GLcontext *gc, GLshort x, GLshort y)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2s", DT_GLshort, x, DT_GLshort, y, DT_GLnull);
#endif


    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex2sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3f", DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLnull);
#endif


    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3fv(__GLcontext *gc, const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glVertex3fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3d", DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLnull);
#endif


    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif


    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3i", DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3s", DT_GLshort, x, DT_GLshort, y, DT_GLshort, z, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex3sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4f", DT_GLfloat, x, DT_GLfloat, y, DT_GLfloat, z, DT_GLfloat, w, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4fv(__GLcontext *gc, const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glVertex4fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4d", DT_GLdouble, x, DT_GLdouble, y, DT_GLdouble, z, DT_GLdouble, w, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4i", DT_GLint, x, DT_GLint, y, DT_GLint, z, DT_GLint, w, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4s",DT_GLshort, x, DT_GLshort, y, DT_GLshort, z, DT_GLshort, w, DT_GLnull);
#endif

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Vertex4sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

#endif

