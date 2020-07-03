/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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

extern GLuint fmtIndex2InputIndex[];

#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


__GL_INLINE GLvoid __glVertex2fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;

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

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2fv(__GLcontext *gc, const GLfloat *v)
{
    __glVertex2fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2i(__GLcontext *gc, GLint x, GLint y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2s(__GLcontext *gc, GLshort x, GLshort y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex2sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3fv(__GLcontext *gc, const GLfloat *v)
{
    __glVertex3fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4fv(__GLcontext *gc, const GLfloat *v)
{
    __glVertex4fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv( gc, fv );
}

GLvoid APIENTRY __glim_Vertex4sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv( gc, fv );
}

/*
 * Following APIs are used for OpenGL vertex APIs performance improvement
 */
__GL_INLINE GLvoid __glVertex2fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.vertexFormat |= __GL_V2F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        gc->input.vertex.index++;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_V2F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V2F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V2F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            /* Void the condition (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET) */
            vtxinfo = gc->input.defaultInfoBuffer;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET))
    {
        __glImmediateFlushBuffer(gc);
    }
}

__GL_INLINE GLvoid __glVertex3fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.vertexFormat |= __GL_V3F_BIT;

    if (gc->input.vertexFormat == gc->input.preVertexFormat)
    {
        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertex.index++;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_V3F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V3F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V3F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            /* Void the condition (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET) */
            vtxinfo = gc->input.defaultInfoBuffer;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET))
    {
        __glImmediateFlushBuffer(gc);
    }
}

__GL_INLINE GLvoid __glVertex4fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

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

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_V4F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V4F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V4F_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_V4F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
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

            /* Void the condition (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET) */
            vtxinfo = gc->input.defaultInfoBuffer;
        }
    }

    gc->input.vertexFormat = 0;
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER) ||
        (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET))
    {
        __glImmediateFlushBuffer(gc);
    }
}

__GL_INLINE GLvoid __glVertex2fv_Cache(__GLcontext *gc, GLuint *vtx)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_V2F_TAG)
    {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == vtx)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data has not been changed, just return */
                gc->pCurrentInfoBufPtr++;
                return;
            }
        }

        buf = (GLuint *)gc->pVertexDataBufPtr + vtxinfo->offsetDW;

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((vtx[0] ^ buf[0]) | (vtx[1] ^ buf[1])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {
        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_V2F_TAG);
            (*gc->currentImmediateDispatch->Vertex2fv)(gc, (GLfloat *)vtx);
            return;
        }

        /* Switch the current vertex buffer back to the default vertex buffer */
        __glSwitchToDefaultVertexBuffer(gc, __GL_V2F_TAG);

        /* Write vertex data into the default vertex buffer */
        (*gc->currentImmediateDispatch->Vertex2fv)(gc, (GLfloat *)vtx);
    }
}

__GL_INLINE GLvoid __glVertex3fv_Cache(__GLcontext *gc, GLuint *vtx)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_V3F_TAG)
    {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == vtx)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data has not been changed, just return */
                gc->pCurrentInfoBufPtr++;
                return;
            }
        }

        buf = (GLuint *)gc->pVertexDataBufPtr + vtxinfo->offsetDW;

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((vtx[0] ^ buf[0]) | (vtx[1] ^ buf[1]) | (vtx[2] ^ buf[2])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {

        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_V3F_TAG);
            (*gc->currentImmediateDispatch->Vertex3fv)(gc, (GLfloat *)vtx);
            return;
        }

        /* Switch the current vertex buffer back to the default vertex buffer */
        __glSwitchToDefaultVertexBuffer(gc, __GL_V3F_TAG);

        /* Write vertex data into the default vertex buffer */
        (*gc->currentImmediateDispatch->Vertex3fv)(gc, (GLfloat *)vtx);
    }
}

__GL_INLINE GLvoid __glVertex4fv_Cache(__GLcontext *gc, GLuint *vtx)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_V4F_TAG)
    {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == vtx)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data has not been changed, just return */
                gc->pCurrentInfoBufPtr++;
                return;
            }
        }

        buf = (GLuint *)gc->pVertexDataBufPtr + vtxinfo->offsetDW;

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((vtx[0] ^ buf[0]) | (vtx[1] ^ buf[1]) | (vtx[2] ^ buf[2]) | (vtx[3] ^ buf[3])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {

        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_V4F_TAG);
            (*gc->currentImmediateDispatch->Vertex4fv)(gc, (GLfloat *)vtx);
            return;
        }

        /* Switch the current vertex buffer back to the default vertex buffer */
        __glSwitchToDefaultVertexBuffer(gc, __GL_V4F_TAG);

        /* Write vertex data into the default vertex buffer */
        (*gc->currentImmediateDispatch->Vertex4fv)(gc, (GLfloat *)vtx);
    }
}

GLvoid APIENTRY __glim_Vertex2f_Info(__GLcontext *gc, GLfloat x, GLfloat y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2fv_Info(__GLcontext *gc, const GLfloat *v)
{

    __glVertex2fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex2d_Info(__GLcontext *gc, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2i_Info(__GLcontext *gc, GLint x, GLint y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2s_Info(__GLcontext *gc, GLshort x, GLshort y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex2sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3f_Info(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3fv_Info(__GLcontext *gc, const GLfloat *v)
{
    __glVertex3fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex3d_Info(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3i_Info(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3s_Info(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex3sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4f_Info(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4fv_Info(__GLcontext *gc, const GLfloat *v)
{
    __glVertex4fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Vertex4d_Info(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4i_Info(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4s_Info(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Vertex4sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Info(gc,  fv );
}

/************************************************************************/

GLvoid APIENTRY __glim_Vertex2f_Cache(__GLcontext *gc, GLfloat x, GLfloat y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glVertex2fv_Cache(gc,  (GLuint *)v );
}

GLvoid APIENTRY __glim_Vertex2d_Cache(__GLcontext *gc, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2i_Cache(__GLcontext *gc, GLint x, GLint y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2s_Cache(__GLcontext *gc, GLshort x, GLshort y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex2sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glVertex2fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3f_Cache(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3d_Cache(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3i_Cache(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3s_Cache(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex3sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glVertex3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4f_Cache(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glVertex4fv_Cache(gc,  (GLuint *)v );
}

GLvoid APIENTRY __glim_Vertex4d_Cache(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4i_Cache(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4s_Cache(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Vertex4sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glVertex4fv_Cache(gc,  (GLuint *)fv );
}

/************************************************************************/

GLvoid APIENTRY __glim_Vertex3f_SwitchBack(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3f_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3d_SwitchBack(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3d_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3dv_SwitchBack(__GLcontext *gc, const GLdouble *v)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3dv_Info(gc, v);
}

GLvoid APIENTRY __glim_Vertex3i_SwitchBack(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3i_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3iv_SwitchBack(__GLcontext *gc, const GLint *v)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3iv_Info(gc, v);
}

GLvoid APIENTRY __glim_Vertex3s_SwitchBack(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3s_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3sv_SwitchBack(__GLcontext *gc, const GLshort *v)
{
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;

    __glim_Vertex3sv_Info(gc, v);
}

GLvoid APIENTRY __glim_Vertex3f_Cache_SwitchBack(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3f_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3d_Cache_SwitchBack(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3d_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3dv_Cache_SwitchBack(__GLcontext *gc, const GLdouble *v)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3dv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Vertex3i_Cache_SwitchBack(__GLcontext *gc, GLint x, GLint y, GLint z)
{

    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3i_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3iv_Cache_SwitchBack(__GLcontext *gc, const GLint *v)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3iv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Vertex3s_Cache_SwitchBack(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3s_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Vertex3sv_Cache_SwitchBack(__GLcontext *gc, const GLshort *v)
{
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;

    __glim_Vertex3sv_Cache(gc, v);
}

/************************************************************************/

GLvoid APIENTRY __glim_Vertex2f_Outside(__GLcontext *gc, GLfloat x, GLfloat y)
{
}

GLvoid APIENTRY __glim_Vertex2fv_Outside(__GLcontext *gc, const GLfloat *v)
{
}

GLvoid APIENTRY __glim_Vertex2d_Outside(__GLcontext *gc, GLdouble x, GLdouble y)
{
}

GLvoid APIENTRY __glim_Vertex2dv_Outside(__GLcontext *gc, const GLdouble *v)
{
}

GLvoid APIENTRY __glim_Vertex2i_Outside(__GLcontext *gc, GLint x, GLint y)
{

}

GLvoid APIENTRY __glim_Vertex2iv_Outside(__GLcontext *gc, const GLint *v)
{
}

GLvoid APIENTRY __glim_Vertex2s_Outside(__GLcontext *gc, GLshort x, GLshort y)
{
}

GLvoid APIENTRY __glim_Vertex2sv_Outside(__GLcontext *gc, const GLshort *v)
{
}

GLvoid APIENTRY __glim_Vertex3f_Outside(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
}

GLvoid APIENTRY __glim_Vertex3fv_Outside(__GLcontext *gc, const GLfloat *v)
{
}

GLvoid APIENTRY __glim_Vertex3d_Outside(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
}

GLvoid APIENTRY __glim_Vertex3dv_Outside(__GLcontext *gc, const GLdouble *v)
{
}

GLvoid APIENTRY __glim_Vertex3i_Outside(__GLcontext *gc, GLint x, GLint y, GLint z)
{
}

GLvoid APIENTRY __glim_Vertex3iv_Outside(__GLcontext *gc, const GLint *v)
{
}

GLvoid APIENTRY __glim_Vertex3s_Outside(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
}

GLvoid APIENTRY __glim_Vertex3sv_Outside(__GLcontext *gc, const GLshort *v)
{
}

GLvoid APIENTRY __glim_Vertex4f_Outside(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}

GLvoid APIENTRY __glim_Vertex4fv_Outside(__GLcontext *gc, const GLfloat *v)
{

}

GLvoid APIENTRY __glim_Vertex4d_Outside(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
}

GLvoid APIENTRY __glim_Vertex4dv_Outside(__GLcontext *gc, const GLdouble *v)
{
}

GLvoid APIENTRY __glim_Vertex4i_Outside(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
}

GLvoid APIENTRY __glim_Vertex4iv_Outside(__GLcontext *gc, const GLint *v)
{
}

GLvoid APIENTRY __glim_Vertex4s_Outside(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
}

GLvoid APIENTRY __glim_Vertex4sv_Outside(__GLcontext *gc, const GLshort *v)
{
}

/************************************************************************/
GLvoid APIENTRY __glim_Vertex3fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glVertex3fv_Cache(gc,  (GLuint *)v );
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

#endif

