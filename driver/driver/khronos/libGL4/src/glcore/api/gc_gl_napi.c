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
__GL_INLINE GLvoid __glNormal3fv(__GLcontext *gc, GLfloat *v) {
    GLfloat *current;

    if (gc->input.preVertexFormat & __GL_N3F_BIT) {
        if ((gc->input.vertexFormat & __GL_N3F_BIT) == 0) {
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        }

        current = gc->input.normal.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_N3F_BIT;
    } else {
        if (((gc->input.currentInputMask & __GL_INPUT_NORMAL) == 0)
                || (gc->input.beginMode != __GL_IN_BEGIN)) {
            /* If glNormal is not needed in glBegin/glEnd */
            gc->state.current.normal.f.x = v[0];
            gc->state.current.normal.f.y = v[1];
            gc->state.current.normal.f.z = v[2];
            gc->state.current.normal.f.w = 1.0;
        } else if (gc->input.lastVertexIndex == gc->input.vertex.index) {
            if (gc->input.lastVertexIndex != 0) {
                /* The first glNormal after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glNormal after glBegin */
            gc->input.normal.offsetDW = (GLuint)(
                    gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.normal.pointer = (GLubyte*) gc->input.currentDataBufPtr;
            gc->input.normal.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_N3F_BIT;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_N3F_TAG);
        } else if (gc->input.preVertexFormat != 0) {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_N3F_INDEX);

            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
        } else {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE) {
                if ((gc->state.current.normal.f.x == v[0])
                        && (gc->state.current.normal.f.y == v[1])
                        && (gc->state.current.normal.f.z == v[2])) {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.normal.currentPtrDW = (GLfloat*) gc->input.normal.pointer
                    + gc->input.normal.index * gc->input.vertTotalStrideDW;
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

GLvoid APIENTRY __glim_Normal3d(__GLcontext *gc, GLdouble x, GLdouble y,
        GLdouble z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3dv(__GLcontext *gc, const GLdouble *v) {
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3f(__GLcontext *gc, GLfloat x, GLfloat y,
        GLfloat z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3fv(__GLcontext *gc, const GLfloat *v) {
    __glNormal3fv(gc, (GLfloat*) v);
}

GLvoid APIENTRY __glim_Normal3b(__GLcontext *gc, GLbyte x, GLbyte y, GLbyte z) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3bv(__GLcontext *gc, const GLbyte *v) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3s(__GLcontext *gc, GLshort x, GLshort y,
        GLshort z) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3sv(__GLcontext *gc, const GLshort *v) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3i(__GLcontext *gc, GLint x, GLint y, GLint z) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv(gc, fv);
}

GLvoid APIENTRY __glim_Normal3iv(__GLcontext *gc, const GLint *v) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv(gc, fv);
}

/*
 * Following APIs are used for immediate mode OpenGL normal APIs
 * performance improvement
 */
__GL_INLINE void __glSwitchToNormalVertexEntries(__GLcontext *gc) {
    __GLdispatchTable *dispatch;

    if (gc->immedModeDispatch.Vertex3fv == __glim_Vertex3fv_Info) {
        dispatch = &gc->immedModeDispatch;
        dispatch->Vertex3fv = __glim_Normal_Vertex3fv;
        dispatch->Normal3b = __glim_Normal3b_SwitchBack;
        dispatch->Normal3bv = __glim_Normal3bv_SwitchBack;
        dispatch->Normal3d = __glim_Normal3d_SwitchBack;
        dispatch->Normal3dv = __glim_Normal3dv_SwitchBack;
        dispatch->Normal3f = __glim_Normal3f_SwitchBack;
        dispatch->Normal3fv = __glim_Normal3fv_SwitchBack;
        dispatch->Normal3i = __glim_Normal3i_SwitchBack;
        dispatch->Normal3iv = __glim_Normal3iv_SwitchBack;
        dispatch->Normal3s = __glim_Normal3s_SwitchBack;
        dispatch->Normal3sv = __glim_Normal3sv_SwitchBack;

        dispatch->Vertex3f = __glim_Vertex3f_SwitchBack;
        dispatch->Vertex3d = __glim_Vertex3d_SwitchBack;
        dispatch->Vertex3dv = __glim_Vertex3dv_SwitchBack;
        dispatch->Vertex3i = __glim_Vertex3i_SwitchBack;
        dispatch->Vertex3iv = __glim_Vertex3iv_SwitchBack;
        dispatch->Vertex3s = __glim_Vertex3s_SwitchBack;
        dispatch->Vertex3sv = __glim_Vertex3sv_SwitchBack;

    }
}

void __glSwitchToNorVertEntriesFunc(__GLcontext *gc) {
    __glSwitchToNormalVertexEntries(gc);
}

__GL_INLINE void __glSwitchToNormalVertexEntries_Cache(__GLcontext *gc) {
    __GLdispatchTable *dispatch;

    if (gc->immedModeCacheDispatch.Vertex3fv == __glim_Vertex3fv_Cache) {
        dispatch = &gc->immedModeCacheDispatch;
        dispatch->Vertex3fv = __glim_Normal_Vertex3fv_Cache;
        dispatch->Normal3b = __glim_Normal3b_Cache_SwitchBack;
        dispatch->Normal3bv = __glim_Normal3bv_Cache_SwitchBack;
        dispatch->Normal3d = __glim_Normal3d_Cache_SwitchBack;
        dispatch->Normal3dv = __glim_Normal3dv_Cache_SwitchBack;
        dispatch->Normal3f = __glim_Normal3f_Cache_SwitchBack;
        dispatch->Normal3fv = __glim_Normal3fv_Cache_SwitchBack;
        dispatch->Normal3i = __glim_Normal3i_Cache_SwitchBack;
        dispatch->Normal3iv = __glim_Normal3iv_Cache_SwitchBack;
        dispatch->Normal3s = __glim_Normal3s_Cache_SwitchBack;
        dispatch->Normal3sv = __glim_Normal3sv_Cache_SwitchBack;

        dispatch->Vertex3f = __glim_Vertex3f_Cache_SwitchBack;
        dispatch->Vertex3d = __glim_Vertex3d_Cache_SwitchBack;
        dispatch->Vertex3dv = __glim_Vertex3dv_Cache_SwitchBack;
        dispatch->Vertex3i = __glim_Vertex3i_Cache_SwitchBack;
        dispatch->Vertex3iv = __glim_Vertex3iv_Cache_SwitchBack;
        dispatch->Vertex3s = __glim_Vertex3s_Cache_SwitchBack;
        dispatch->Vertex3sv = __glim_Vertex3sv_Cache_SwitchBack;
    }
}

__GL_INLINE void __glNormal3fv_Info(__GLcontext *gc, GLfloat *v) {
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.deferredAttribDirty &= ~__GL_DEFERED_NORMAL_BIT;

    if (gc->input.preVertexFormat & __GL_N3F_BIT) {
        if ((gc->input.vertexFormat & __GL_N3F_BIT) == 0) {
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        }

        current = gc->input.normal.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_N3F_BIT;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_N3F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint*) v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer,
                __GL_INPUT_NORMAL_INDEX);
    } else {
        if ((gc->input.currentInputMask & __GL_INPUT_NORMAL) == 0) {
            /* If glNormal is not needed in glBegin/glEnd */
            gc->state.current.normal.f.x = v[0];
            gc->state.current.normal.f.y = v[1];
            gc->state.current.normal.f.z = v[2];
            gc->state.current.normal.f.w = 1.0;
        } else if (gc->input.lastVertexIndex == gc->input.vertex.index) {
            if (gc->input.lastVertexIndex != 0) {
                /* The first glNormal after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glNormal after glBegin */
            gc->input.normal.offsetDW = (GLuint)(
                    gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.normal.pointer = (GLubyte*) gc->input.currentDataBufPtr;
            gc->input.normal.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_N3F_BIT;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_N3F_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_N3F_TAG;
            vtxinfo->offsetDW = (GLushort)(
                    current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint*) v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer,
                    __GL_INPUT_NORMAL_INDEX);
        } else if (gc->input.preVertexFormat != 0) {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_N3F_INDEX);

            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_N3F_BIT;
        } else {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE) {
                if ((gc->state.current.normal.f.x == v[0])
                        && (gc->state.current.normal.f.y == v[1])
                        && (gc->state.current.normal.f.z == v[2])) {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.normal.currentPtrDW = (GLfloat*) gc->input.normal.pointer
                    + gc->input.normal.index * gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.normal.index += 1;
            gc->input.vertexFormat |= __GL_N3F_BIT;
        }
    }
}

__GL_INLINE void __glNormal3fv_Cache(__GLcontext *gc, GLuint *nor) {
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_N3F_TAG) {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == nor) {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus)) {
                /* Then application data has not been changed, just return */
                gc->pCurrentInfoBufPtr++;
                return;
            }
        }

        buf = (GLuint*) gc->pVertexDataBufPtr + vtxinfo->offsetDW;

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((nor[0] ^ buf[0]) | (nor[1] ^ buf[1]) | (nor[2] ^ buf[2])) == 0) {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {
        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG) {
            __glImmedFlushBuffer_Cache(gc, __GL_N3F_TAG);
            (*gc->currentImmediateDispatch->Normal3fv)(gc, (GLfloat*) nor);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_NORMAL) {
            if (gc->input.beginMode == __GL_IN_BEGIN) {
                /* Switch the current vertex buffer back to the default vertex buffer */
                __glSwitchToDefaultVertexBuffer(gc, __GL_N3F_TAG);

                /* Wirte vertex data into the default vertex buffer */
                (*gc->currentImmediateDispatch->Normal3fv)(gc, (GLfloat*) nor);
            } else {
                /* Save the normal in the shadow current state.
                 */
                gc->input.shadowCurrent.normal.f.x = ((GLfloat*) nor)[0];
                gc->input.shadowCurrent.normal.f.y = ((GLfloat*) nor)[1];
                gc->input.shadowCurrent.normal.f.z = ((GLfloat*) nor)[2];
                gc->input.shadowCurrent.normal.f.w = 1.0;

                gc->input.deferredAttribDirty |= __GL_DEFERED_NORMAL_BIT;

                /* Set glVertex3fv entries to __glim_Normal_Vertex3fv_Cache function.
                 */
                __glSwitchToNormalVertexEntries_Cache(gc);
            }
        } else {
            /* Normal is not needed, just update current normal state */
            gc->state.current.normal.f.x = ((GLfloat*) nor)[0];
            gc->state.current.normal.f.y = ((GLfloat*) nor)[1];
            gc->state.current.normal.f.z = ((GLfloat*) nor)[2];
            gc->state.current.normal.f.w = 1.0;
        }
    }
}

__GL_INLINE void __glNormal3fv_Outside(__GLcontext *gc, GLfloat *v) {
    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_NORMAL) == 0) {
        /* glNormal is not needed in glBegin/glEnd.
         */
        gc->state.current.normal.f.x = v[0];
        gc->state.current.normal.f.y = v[1];
        gc->state.current.normal.f.z = v[2];
        gc->state.current.normal.f.w = 1.0;

        gc->input.shadowCurrent.normal = gc->state.current.normal;
        gc->input.deferredAttribDirty &= ~__GL_DEFERED_NORMAL_BIT;
    } else {
        /* glNormal is needed in glBegin/glEnd.
         **
         ** Save the normal in the shadow current state.
         */
        gc->input.shadowCurrent.normal.f.x = v[0];
        gc->input.shadowCurrent.normal.f.y = v[1];
        gc->input.shadowCurrent.normal.f.z = v[2];
        gc->input.shadowCurrent.normal.f.w = 1.0;

        gc->input.deferredAttribDirty |= __GL_DEFERED_NORMAL_BIT;

        /* Set glVertex3fv entries to __glim_Normal_Vertex3fv function.
         */
        __glSwitchToNormalVertexEntries(gc);
        __glSwitchToNormalVertexEntries_Cache(gc);
    }
}

GLvoid APIENTRY __glim_Normal3d_Info(__GLcontext *gc, GLdouble x, GLdouble y,
        GLdouble z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3dv_Info(__GLcontext *gc, const GLdouble *v) {
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3f_Info(__GLcontext *gc, GLfloat x, GLfloat y,
        GLfloat z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3fv_Info(__GLcontext *gc, const GLfloat *v) {
    __glNormal3fv_Info(gc, (GLfloat*) v);
}

GLvoid APIENTRY __glim_Normal3b_Info(__GLcontext *gc, GLbyte x, GLbyte y,
        GLbyte z) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3bv_Info(__GLcontext *gc, const GLbyte *v) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3s_Info(__GLcontext *gc, GLshort x, GLshort y,
        GLshort z) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3sv_Info(__GLcontext *gc, const GLshort *v) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3i_Info(__GLcontext *gc, GLint x, GLint y, GLint z) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv_Info(gc, fv);
}

GLvoid APIENTRY __glim_Normal3iv_Info(__GLcontext *gc, const GLint *v) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv_Info(gc, fv);
}

/*************************************************************************/

GLvoid APIENTRY __glim_Normal3d_Cache(__GLcontext *gc, GLdouble x, GLdouble y,
        GLdouble z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3dv_Cache(__GLcontext *gc, const GLdouble *v) {
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3f_Cache(__GLcontext *gc, GLfloat x, GLfloat y,
        GLfloat z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3b_Cache(__GLcontext *gc, GLbyte x, GLbyte y,
        GLbyte z) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3bv_Cache(__GLcontext *gc, const GLbyte *v) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3s_Cache(__GLcontext *gc, GLshort x, GLshort y,
        GLshort z) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3sv_Cache(__GLcontext *gc, const GLshort *v) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3i_Cache(__GLcontext *gc, GLint x, GLint y,
        GLint z) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

GLvoid APIENTRY __glim_Normal3iv_Cache(__GLcontext *gc, const GLint *v) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv_Cache(gc, (GLuint*) fv);
}

/*************************************************************************/

GLvoid APIENTRY __glim_Normal3d_Outside(__GLcontext *gc, GLdouble x, GLdouble y,
        GLdouble z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3dv_Outside(__GLcontext *gc, const GLdouble *v) {
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3f_Outside(__GLcontext *gc, GLfloat x, GLfloat y,
        GLfloat z) {
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3fv_Outside(__GLcontext *gc, const GLfloat *v) {

    __glNormal3fv_Outside(gc, (GLfloat*) v);
}

GLvoid APIENTRY __glim_Normal3b_Outside(__GLcontext *gc, GLbyte x, GLbyte y,
        GLbyte z) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(x);
    fv[1] = __GL_B_TO_FLOAT(y);
    fv[2] = __GL_B_TO_FLOAT(z);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3bv_Outside(__GLcontext *gc, const GLbyte *v) {
    GLfloat fv[3];

    fv[0] = __GL_B_TO_FLOAT(v[0]);
    fv[1] = __GL_B_TO_FLOAT(v[1]);
    fv[2] = __GL_B_TO_FLOAT(v[2]);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3s_Outside(__GLcontext *gc, GLshort x, GLshort y,
        GLshort z) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(x);
    fv[1] = __GL_S_TO_FLOAT(y);
    fv[2] = __GL_S_TO_FLOAT(z);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3sv_Outside(__GLcontext *gc, const GLshort *v) {
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3i_Outside(__GLcontext *gc, GLint x, GLint y,
        GLint z) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(x);
    fv[1] = __GL_I_TO_FLOAT(y);
    fv[2] = __GL_I_TO_FLOAT(z);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3iv_Outside(__GLcontext *gc, const GLint *v) {
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glNormal3fv_Outside(gc, fv);
}

GLvoid APIENTRY __glim_Normal3fv_Cache(__GLcontext *gc, const GLfloat *v) {
    __glNormal3fv_Cache(gc, (GLuint*) v);
}

GLvoid APIENTRY __glim_Normal3d_SwitchBack(__GLcontext *gc, GLdouble x,
        GLdouble y, GLdouble z) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3d = __glim_Normal3d_Info;

    __glim_Normal3d_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3d_Cache_SwitchBack(__GLcontext *gc, GLdouble x,
        GLdouble y, GLdouble z) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3d = __glim_Normal3d_Cache;

    __glim_Normal3d_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3dv_SwitchBack(__GLcontext *gc, const GLdouble *v) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3dv = __glim_Normal3dv_Info;

    __glim_Normal3dv_Info(gc, v);
}

GLvoid APIENTRY __glim_Normal3dv_Cache_SwitchBack(__GLcontext *gc,
        const GLdouble *v) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3dv = __glim_Normal3dv_Cache;

    __glim_Normal3dv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Normal3f_SwitchBack(__GLcontext *gc, GLfloat x,
        GLfloat y, GLfloat z) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3f = __glim_Normal3f_Info;

    __glim_Normal3f_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3f_Cache_SwitchBack(__GLcontext *gc, GLfloat x,
        GLfloat y, GLfloat z) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3f = __glim_Normal3f_Cache;

    __glim_Normal3f_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3fv_SwitchBack(__GLcontext *gc, const GLfloat *v) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3fv = __glim_Normal3fv_Info;

    __glim_Normal3fv_Info(gc, v);
}

GLvoid APIENTRY __glim_Normal3fv_Cache_SwitchBack(__GLcontext *gc,
        const GLfloat *v) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3fv = __glim_Normal3fv_Cache;

    __glim_Normal3fv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Normal3b_SwitchBack(__GLcontext *gc, GLbyte x, GLbyte y,
        GLbyte z) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3b = __glim_Normal3b_Info;

    __glim_Normal3b_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3b_Cache_SwitchBack(__GLcontext *gc, GLbyte x,
        GLbyte y, GLbyte z) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3b = __glim_Normal3b_Cache;

    __glim_Normal3b_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3bv_SwitchBack(__GLcontext *gc, const GLbyte *v) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3bv = __glim_Normal3bv_Info;

    __glim_Normal3bv_Info(gc, v);
}

GLvoid APIENTRY __glim_Normal3bv_Cache_SwitchBack(__GLcontext *gc,
        const GLbyte *v) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3bv = __glim_Normal3bv_Cache;

    __glim_Normal3bv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Normal3s_SwitchBack(__GLcontext *gc, GLshort x,
        GLshort y, GLshort z) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3s = __glim_Normal3s_Info;

    __glim_Normal3s_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3s_Cache_SwitchBack(__GLcontext *gc, GLshort x,
        GLshort y, GLshort z) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3s = __glim_Normal3s_Cache;

    __glim_Normal3s_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3sv_SwitchBack(__GLcontext *gc, const GLshort *v) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3sv = __glim_Normal3sv_Info;

    __glim_Normal3sv_Info(gc, v);
}

GLvoid APIENTRY __glim_Normal3sv_Cache_SwitchBack(__GLcontext *gc,
        const GLshort *v) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3sv = __glim_Normal3sv_Cache;

    __glim_Normal3sv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Normal3i_SwitchBack(__GLcontext *gc, GLint x, GLint y,
        GLint z) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3i = __glim_Normal3i_Info;

    __glim_Normal3i_Info(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3i_Cache_SwitchBack(__GLcontext *gc, GLint x,
        GLint y, GLint z) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3i = __glim_Normal3i_Cache;

    __glim_Normal3i_Cache(gc, x, y, z);
}

GLvoid APIENTRY __glim_Normal3iv_SwitchBack(__GLcontext *gc, const GLint *v) {
    gc->immedModeDispatch.Vertex3fv = __glim_Vertex3fv_Info;
    gc->immedModeDispatch.Normal3iv = __glim_Normal3iv_Info;

    __glim_Normal3iv_Info(gc, v);
}

GLvoid APIENTRY __glim_Normal3iv_Cache_SwitchBack(__GLcontext *gc,
        const GLint *v) {
    gc->immedModeCacheDispatch.Vertex3fv = __glim_Vertex3fv_Cache;
    gc->immedModeCacheDispatch.Normal3iv = __glim_Normal3iv_Cache;

    __glim_Normal3iv_Cache(gc, v);
}

GLvoid APIENTRY __glim_Normal_Vertex3fv(__GLcontext *gc, const GLfloat *v) {
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);

    if ((gc->input.vertexFormat | __GL_N3F_BIT | __GL_V3F_BIT)
            == gc->input.preVertexFormat) {
        gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.normal.currentPtrDW;
        current[0] = gc->input.shadowCurrent.normal.f.x;
        current[1] = gc->input.shadowCurrent.normal.f.y;
        current[2] = gc->input.shadowCurrent.normal.f.z;

        gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.vertex.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertex.index++;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_N3F_V3F_TAG;
        vtxinfo->offsetDW = (GLushort)(
                gc->input.normal.currentPtrDW - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint*) v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, (GLfloat*) v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer,
                __GL_INPUT_VERTEX_INDEX);
    } else {
        if (gc->input.vertex.index != 0
                && (gc->input.preVertexFormat & gc->input.vertexFormat)
                        == gc->input.vertexFormat
                && (gc->input.deferredAttribDirty & __GL_DEFERED_NORCOL_MASK)
                        == 0) {
            /*
             When partially cache matches and the number of elements in cache greater than
             new primitive format, then need to duplicate the previous attributes to the new
             primitive
             */
            gc->input.vertexFormat |= (__GL_N3F_BIT | __GL_V3F_BIT);
            /* Fill the missing attributes for the current vertex from the previous vertex */
            __glDuplicatePreviousAttrib(gc);

            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = gc->input.shadowCurrent.normal.f.x;
            current[1] = gc->input.shadowCurrent.normal.f.y;
            current[2] = gc->input.shadowCurrent.normal.f.z;
            gc->input.normal.index++;

            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertex.index++;

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_N3F_V3F_TAG;
            vtxinfo->offsetDW = (GLushort)(
                    current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint*) v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc,
                    (GLfloat*) v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer,
                    __GL_INPUT_VERTEX_INDEX);
        } else if (gc->input.lastVertexIndex == gc->input.vertex.index) {
            /* The first glVertex after glBegin has different format from the previous primitives */
            if (gc->input.lastVertexIndex != 0) {
                __glConsistentFormatChange(gc);
            }

            /* For the first glNormal after glBegin */
            gc->input.normal.offsetDW = (GLuint)(
                    gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.normal.pointer = (GLubyte*) gc->input.currentDataBufPtr;
            gc->input.normal.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            current = gc->input.normal.currentPtrDW;
            current[0] = gc->input.shadowCurrent.normal.f.x;
            current[1] = gc->input.shadowCurrent.normal.f.y;
            current[2] = gc->input.shadowCurrent.normal.f.z;

            /* For the first glVertex after glBegin */
            gc->input.vertex.offsetDW = (GLuint)(
                    gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.vertex.pointer = (GLubyte*) gc->input.currentDataBufPtr;
            gc->input.vertex.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.vertTotalStrideDW = gc->input.vertex.offsetDW + 3;
            gc->input.preVertexFormat = (gc->input.vertexFormat | __GL_N3F_BIT
                    | __GL_V3F_BIT);
            current = gc->input.vertex.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertex.index++;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_N3F_TAG);
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_V3F_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_N3F_V3F_TAG;
            vtxinfo->offsetDW = (GLushort)(
                    gc->input.normal.currentPtrDW - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint*) v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc,
                    (GLfloat*) v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer,
                    __GL_INPUT_VERTEX_INDEX);
        } else {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE) {
                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.vertexFormat |= (__GL_N3F_BIT | __GL_V4F_BIT);
            if (gc->input.vertexFormat != gc->input.primitiveFormat) {
                __glFillMissingAttributes(gc);
            }

            gc->input.normal.currentPtrDW = (GLfloat*) gc->input.normal.pointer
                    + gc->input.normal.index * gc->input.vertTotalStrideDW;
            current = gc->input.normal.currentPtrDW;
            current[0] = gc->input.shadowCurrent.normal.f.x;
            current[1] = gc->input.shadowCurrent.normal.f.y;
            current[2] = gc->input.shadowCurrent.normal.f.z;
            gc->input.normal.index++;

            gc->input.vertex.currentPtrDW = (GLfloat*) gc->input.vertex.pointer
                    + gc->input.vertex.index * gc->input.vertTotalStrideDW;
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
    if ((gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            || (vtxinfo->offsetDW > __GL_MAX_VERTEX_BUFFER_DW_OFFSET)) {
        __glImmediateFlushBuffer(gc);
    }
}

GLvoid APIENTRY __glim_Normal_Vertex3fv_Cache(__GLcontext *gc, const GLfloat *v) {
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf, *nor, *vtx;

    nor = (GLuint*) (&gc->input.shadowCurrent.normal);
    vtx = (GLuint*) v;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_N3F_V3F_TAG) {
        /* If this is the first entry after switching to Normal_Vertex3fv entrt function */
        if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT) {
            buf = (GLuint*) gc->pVertexDataBufPtr + vtxinfo->offsetDW;

            /* If normal in shadowCurrent are not the same as cached normal data */
            if (((nor[0] ^ buf[0]) | (nor[1] ^ buf[1]) | (nor[2] ^ buf[2]))
                    != 0) {
                goto SwitchToDefault;
            }

            gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);
        }

        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == vtx) {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus)) {
                /* Then application data has not been changed, just return */
                gc->pCurrentInfoBufPtr++;
                return;
            }
        }

        buf = (GLuint*) gc->pVertexDataBufPtr + vtxinfo->offsetDW + 3;

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((vtx[0] ^ buf[0]) | (vtx[1] ^ buf[1]) | (vtx[2] ^ buf[2])) == 0) {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    /* If it is the end of vertex cache buffer then flush the vertex data */
    if (vtxinfo->inputTag == __GL_BATCH_END_TAG) {
        __glImmedFlushBuffer_Cache(gc, __GL_N3F_V3F_TAG);
        (*gc->currentImmediateDispatch->Vertex3fv)(gc, (GLfloat*) vtx);
        return;
    }

    SwitchToDefault:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, __GL_N3F_V3F_TAG);

    /* Wirte vertex data into the default vertex buffer */
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, (GLfloat*) vtx);
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

