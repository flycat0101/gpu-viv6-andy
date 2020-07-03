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


#include "gc_es_context.h"
#include "gc_gl_api_inline.c"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


__GL_INLINE GLvoid __glSecondaryColor3fv(__GLcontext *gc, GLfloat *v)
{
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

GLvoid APIENTRY __glim_SecondaryColor3b(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(r);
    fv[1] = __GL_B_TO_FLOAT(g);
    fv[2] = __GL_B_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3d(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3f(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v)
{
    __glSecondaryColor3fv(gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_SecondaryColor3i(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3s(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ub(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(r);
    fv[1] = __GL_UB_TO_FLOAT(g);
    fv[2] = __GL_UB_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ui(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3us(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glSecondaryColor3fv(gc, fv );
}

GLvoid APIENTRY __glim_SecondaryColor3usv(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glSecondaryColor3fv(gc, fv );
}

/*
 * Following APIs are used for immediate mode OpenGL secondary color APIs performance improvement
 */
__GL_INLINE GLvoid __glSecondaryColor3fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

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

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_SC3F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_SPECULAR_INDEX);
    }
    else
    {
        if ((gc->input.currentInputMask & __GL_INPUT_SPECULAR) == 0)
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_SC3F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_SPECULAR_INDEX);
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

__GL_INLINE GLvoid __glSecondaryColor3fv_Cache(__GLcontext *gc, GLuint *col)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_SC3F_TAG)
    {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == col)
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
        if (((col[0] ^ buf[0]) | (col[1] ^ buf[1]) | (col[2] ^ buf[2])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {
        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_SC3F_TAG);
            (*gc->currentImmediateDispatch->SecondaryColor3fv)(gc, (GLfloat *)col);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_SPECULAR)
        {
            /* Switch the current vertex buffer back to the default vertex buffer */
            __glSwitchToDefaultVertexBuffer(gc, __GL_SC3F_TAG);

            /* Write vertex data into the default vertex buffer */
            (*gc->currentImmediateDispatch->SecondaryColor3fv)(gc, (GLfloat *)col);
        }
        else
        {
            /* Color is not needed, just update current color state */
            gc->state.current.color2.r = ((GLfloat *)col)[0];
            gc->state.current.color2.g = ((GLfloat *)col)[1];
            gc->state.current.color2.b = ((GLfloat *)col)[2];
            gc->state.current.color2.a = 1.0;
        }
    }
}

__GL_INLINE GLvoid __glSecondaryColor3fv_Outside(__GLcontext *gc, GLfloat *v)
{
    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_SPECULAR) == 0 ||
        gc->input.beginMode != __GL_SMALL_DRAW_BATCH)
    {
        /* glSecondaryColor is not needed in glBegin/glEnd.
        */
        gc->state.current.color2.r = v[0];
        gc->state.current.color2.g = v[1];
        gc->state.current.color2.b = v[2];
        gc->state.current.color2.a = 1.0;
    }
    else
    {
        /* glSecondaryColor is needed in glBegin/glEnd.
        */
        if (gc->input.prevPrimInputMask & __GL_INPUT_SPECULAR)
        {
            /* If previous primitive has glSecondaryColor in glBegin/glEnd.
            */
            __glPrimitiveBatchEnd(gc);

            gc->state.current.color2.r = v[0];
            gc->state.current.color2.g = v[1];
            gc->state.current.color2.b = v[2];
        }
        else
        {
            /* Previous primitive has no glSecondaryColor (but it needs color2) in glBegin/glEnd.
            */
            if (gc->state.current.color2.r != v[0] ||
                gc->state.current.color2.g != v[1] ||
                gc->state.current.color2.b != v[2])
            {
                __glPrimitiveBatchEnd(gc);

                gc->state.current.color2.r = v[0];
                gc->state.current.color2.g = v[1];
                gc->state.current.color2.b = v[2];
            }
        }
    }
}

GLvoid APIENTRY __glim_SecondaryColor3b_Info(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(r);
    fv[1] = __GL_B_TO_FLOAT(g);
    fv[2] = __GL_B_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3bv_Info(__GLcontext *gc, const GLbyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3d_Info(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3f_Info(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3fv_Info(__GLcontext *gc, const GLfloat *v)
{
    __glSecondaryColor3fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_SecondaryColor3i_Info(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3s_Info(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ub_Info(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(r);
    fv[1] = __GL_UB_TO_FLOAT(g);
    fv[2] = __GL_UB_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ubv_Info(__GLcontext *gc, const GLubyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ui_Info(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3uiv_Info(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3us_Info(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glSecondaryColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3usv_Info(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Info(gc,  fv );
}

/*************************************************************************/

GLvoid APIENTRY __glim_SecondaryColor3b_Cache(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(r);
    fv[1] = __GL_B_TO_FLOAT(g);
    fv[2] = __GL_B_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3bv_Cache(__GLcontext *gc, const GLbyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3d_Cache(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3f_Cache(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)v );
}

GLvoid APIENTRY __glim_SecondaryColor3i_Cache(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3s_Cache(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc, (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ub_Cache(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(r);
    fv[1] = __GL_UB_TO_FLOAT(g);
    fv[2] = __GL_UB_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ubv_Cache(__GLcontext *gc, const GLubyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ui_Cache(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3uiv_Cache(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3us_Cache(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_SecondaryColor3usv_Cache(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Cache(gc,  (GLuint *)fv );
}

/*************************************************************************/

GLvoid APIENTRY __glim_SecondaryColor3b_Outside(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(r);
    fv[1] = __GL_B_TO_FLOAT(g);
    fv[2] = __GL_B_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3bv_Outside(__GLcontext *gc, const GLbyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3d_Outside(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3dv_Outside(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3f_Outside(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3fv_Outside(__GLcontext *gc, const GLfloat *v)
{
    __glSecondaryColor3fv_Outside(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_SecondaryColor3i_Outside(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3iv_Outside(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3s_Outside(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3sv_Outside(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ub_Outside(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(r);
    fv[1] = __GL_UB_TO_FLOAT(g);
    fv[2] = __GL_UB_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ubv_Outside(__GLcontext *gc, const GLubyte *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3ui_Outside(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3uiv_Outside(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3us_Outside(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_SecondaryColor3usv_Outside(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glSecondaryColor3fv_Outside(gc,  fv );
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif
