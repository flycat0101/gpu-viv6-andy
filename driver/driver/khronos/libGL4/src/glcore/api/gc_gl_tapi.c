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


#include "gc_es_context.h"
#include "gc_gl_api_inline.c"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#pragma warning(disable: 4003)
#pragma warning(disable: 4133)
#endif



__GL_INLINE GLvoid __glTexCoord2fv(__GLcontext *gc, GLuint unit, GLfloat *v)
{
    GLfloat *current;
    GLuint64 tc2fMask = (__GL_ONE_64 << (__GL_TC2F_INDEX + unit));

    if (gc->input.preVertexFormat & tc2fMask)
    {
        if ((gc->input.vertexFormat & tc2fMask) == 0)
        {
            gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.texture[unit].currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        gc->input.vertexFormat |= tc2fMask;
    }
    else
    {
        GLuint64 tc3fMask = (__GL_ONE_64 << (__GL_TC3F_INDEX + unit));
        GLuint64 tc4fMask = (__GL_ONE_64 << (__GL_TC4F_INDEX + unit));

        if (((gc->input.currentInputMask & (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + unit))) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))
        {
            /* If glTexCoord is not needed in glBegin/glEnd */
            gc->state.current.texture[unit].fTex.s = v[0];
            gc->state.current.texture[unit].fTex.t = v[1];
            gc->state.current.texture[unit].fTex.r = 0.0;
            gc->state.current.texture[unit].fTex.q = 1.0;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (tc3fMask | tc4fMask))
            {
                /* Discard the previous glTexCoord3fv or glTexCoord4fv */
                gc->input.vertexFormat &= ~(tc3fMask | tc4fMask);

                /* The first glTexCoord after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glTexCoord after glBegin */
            gc->input.texture[unit].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.texture[unit].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.texture[unit].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.texture[unit].sizeDW = 2;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 2;
            gc->input.preVertexFormat |= tc2fMask;
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            gc->input.vertexFormat |= tc2fMask;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_TC2F_TAG + unit);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (tc3fMask | tc4fMask)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            if (gc->state.current.texture[unit].fTex.r == 0.0f &&
                gc->state.current.texture[unit].fTex.q == 1.0f)
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_TC2F_INDEX + unit);

                gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.texture[unit].currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                gc->input.vertexFormat |= tc2fMask;
            }
            else
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_TC4F_INDEX + unit);

                gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.texture[unit].currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = 0.0;
                current[3] = 1.0;
                gc->input.vertexFormat |= tc4fMask;
            }
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (tc3fMask | tc4fMask)) == 0 &&
                    (gc->state.current.texture[unit].fTex.s == v[0]) &&
                    (gc->state.current.texture[unit].fTex.t == v[1]) &&
                    (gc->state.current.texture[unit].fTex.r == 0.0f) &&
                    (gc->state.current.texture[unit].fTex.q == 1.0f))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (tc3fMask | tc4fMask)))
            {
                gc->input.texture[unit].currentPtrDW = (GLfloat*)gc->input.texture[unit].pointer +
                gc->input.texture[unit].index * gc->input.vertTotalStrideDW;
                gc->input.texture[unit].index += 1;
            }
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = 0.0;
            current[3] = 1.0;
            gc->input.vertexFormat |= tc4fMask;
        }
    }
}

/* OpenGL single texture coordinate APIs */

__GL_INLINE GLvoid __glTexCoord3fv(__GLcontext *gc, GLuint unit, GLfloat *v)
{
    GLfloat *current;
    GLuint64 tc3fMask = (__GL_ONE_64 << (__GL_TC3F_INDEX + unit));

    if (gc->input.preVertexFormat & tc3fMask)
    {
        if ((gc->input.vertexFormat & tc3fMask) == 0)
        {
            gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.texture[unit].currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= tc3fMask;
    }
    else
    {
        GLuint64 tc2fMask = (__GL_ONE_64 << (__GL_TC2F_INDEX + unit));
        GLuint64 tc4fMask = (__GL_ONE_64 << (__GL_TC4F_INDEX + unit));

        if ((gc->input.currentInputMask & (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + unit))) == 0)
        {
            /* If glTexCoord is not needed in glBegin/glEnd */
            gc->state.current.texture[unit].fTex.s = v[0];
            gc->state.current.texture[unit].fTex.t = v[1];
            gc->state.current.texture[unit].fTex.r = v[2];
            gc->state.current.texture[unit].fTex.q = 1.0;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (tc2fMask | tc4fMask))
            {
                /* Discard the previous glTexCoord2fv or glTexCoord4fv */
                gc->input.vertexFormat &= ~(tc2fMask | tc4fMask);

                /* The first glTexCoord after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glTexCoord after glBegin */
            gc->input.texture[unit].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.texture[unit].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.texture[unit].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.texture[unit].sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= tc3fMask;
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= tc3fMask;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_TC3F_TAG + unit);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (tc2fMask | tc4fMask)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            if (gc->state.current.texture[unit].fTex.q == 1.0f)
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_TC3F_INDEX + unit);

                gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.texture[unit].currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                gc->input.vertexFormat |= tc3fMask;
            }
            else
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_TC4F_INDEX + unit);

                gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.texture[unit].currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                current[3] = 1.0;
                gc->input.vertexFormat |= tc4fMask;
            }
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (tc2fMask | tc4fMask)) == 0 &&
                    (gc->state.current.texture[unit].fTex.s == v[0]) &&
                    (gc->state.current.texture[unit].fTex.t == v[1]) &&
                    (gc->state.current.texture[unit].fTex.r == v[2]) &&
                    (gc->state.current.texture[unit].fTex.q == 1.0f))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (tc2fMask | tc4fMask)))
            {
                gc->input.texture[unit].currentPtrDW = (GLfloat*)gc->input.texture[unit].pointer +
                gc->input.texture[unit].index * gc->input.vertTotalStrideDW;
                gc->input.texture[unit].index += 1;
            }
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = 1.0;
            gc->input.vertexFormat |= tc4fMask;
        }
    }
}

__GL_INLINE GLvoid __glTexCoord4fv(__GLcontext *gc, GLuint unit, GLfloat *v)
{
    GLfloat *current;
    GLuint64 tc4fMask = (__GL_ONE_64 << (__GL_TC4F_INDEX + unit));

    if (gc->input.preVertexFormat & tc4fMask)
    {
        if ((gc->input.vertexFormat & tc4fMask) == 0)
        {
            gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.texture[unit].currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        current[3] = v[3];
        gc->input.vertexFormat |= tc4fMask;
    }
    else
    {
        GLuint64 tc2fMask = (__GL_ONE_64 << (__GL_TC2F_INDEX + unit));
        GLuint64 tc3fMask = (__GL_ONE_64 << (__GL_TC3F_INDEX + unit));

        if ((gc->input.currentInputMask & (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + unit))) == 0)
        {
            /* If glTexCoord is not needed in glBegin/glEnd */
            gc->state.current.texture[unit].fTex.s = v[0];
            gc->state.current.texture[unit].fTex.t = v[1];
            gc->state.current.texture[unit].fTex.r = v[2];
            gc->state.current.texture[unit].fTex.q = v[3];
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (tc2fMask | tc3fMask))
            {
                /* Discard the previous glTexCoord2fv or glTexCoord3fv */
                gc->input.vertexFormat &= ~(tc2fMask | tc3fMask);

                /* The first glTexCoord after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glTexCoord after glBegin */
            gc->input.texture[unit].offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.texture[unit].currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.texture[unit].pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.texture[unit].sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.preVertexFormat |= tc4fMask;
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= tc4fMask;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_TC4F_TAG + unit);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (tc2fMask | tc3fMask)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_TC4F_INDEX + unit);

            gc->input.texture[unit].currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= tc4fMask;
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (tc2fMask | tc3fMask)) == 0 &&
                    (gc->state.current.texture[unit].fTex.s == v[0]) &&
                    (gc->state.current.texture[unit].fTex.t == v[1]) &&
                    (gc->state.current.texture[unit].fTex.r == v[2]) &&
                    (gc->state.current.texture[unit].fTex.q == v[3]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (tc2fMask | tc3fMask)))
            {
                gc->input.texture[unit].currentPtrDW = (GLfloat*)gc->input.texture[unit].pointer +
                gc->input.texture[unit].index * gc->input.vertTotalStrideDW;
                gc->input.texture[unit].index += 1;
            }
            current = gc->input.texture[unit].currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= tc4fMask;
        }
    }
}



/***********************************************************************************/

#define __GL_MULT_TEXCOORD2FV(gc, texture, fv)                                          \
    if ((texture >= GL_TEXTURE0) &&                                                 \
        (texture <= (GL_TEXTURE0 + __GL_MAX_TEXTURE_COORDS - 1))) {                 \
        texture -= GL_TEXTURE0;                                                     \
    } else {                                                                        \
        __glSetError(gc, GL_INVALID_ENUM);                                              \
        return;                                                                     \
    }                                                                               \
                                                                                    \
    __glTexCoord2fv( gc, texture, fv )


#define __GL_MULT_TEXCOORD3FV(gc, texture, fv)                                          \
    if ((texture >= GL_TEXTURE0) &&                                                 \
        (texture <= (GL_TEXTURE0 + __GL_MAX_TEXTURE_COORDS - 1))) {                 \
        texture -= GL_TEXTURE0;                                                     \
    } else {                                                                        \
        __glSetError(gc, GL_INVALID_ENUM);                                              \
        return;                                                                     \
    }                                                                               \
                                                                                    \
    __glTexCoord3fv( gc, texture, fv )

#define __GL_MULT_TEXCOORD4FV(gc, texture, fv)                                          \
    if ((texture >= GL_TEXTURE0) &&                                                 \
        (texture <= (GL_TEXTURE0 + __GL_MAX_TEXTURE_COORDS - 1))) {                 \
        texture -= GL_TEXTURE0;                                                     \
    } else {                                                                        \
        __glSetError(gc, GL_INVALID_ENUM);                                              \
        return;                                                                     \
    }                                                                               \
                                                                                    \
    __glTexCoord4fv( gc, texture, fv )


/***********************************************************************************/

GLvoid APIENTRY __glim_TexCoord1d(__GLcontext *gc, GLdouble x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1f(__GLcontext *gc, GLfloat x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1i(__GLcontext *gc, GLint x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1s(__GLcontext *gc, GLshort x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1fv(__GLcontext *gc, const GLfloat *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord1sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2i(__GLcontext *gc, GLint x, GLint y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2s(__GLcontext *gc, GLshort x, GLshort y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2fv(__GLcontext *gc, const GLfloat *v)
{
    __glTexCoord2fv(gc, 0, (GLfloat *)v);
}

GLvoid APIENTRY __glim_TexCoord2iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord2sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __glTexCoord2fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3fv(__GLcontext *gc, const GLfloat *v)
{
    __glTexCoord3fv(gc, 0, (GLfloat *)v);
}

GLvoid APIENTRY __glim_TexCoord3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glTexCoord3fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4fv(__GLcontext *gc, const GLfloat *v)
{
    __glTexCoord4fv(gc, 0, (GLfloat *)v);
}

GLvoid APIENTRY __glim_TexCoord4iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glTexCoord4fv(gc, 0, fv);
}

GLvoid APIENTRY __glim_TexCoord4sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glTexCoord4fv(gc, 0, fv);
}

/* OpenGL multiple texture coordinate APIs */

GLvoid APIENTRY __glim_MultiTexCoord1d(__GLcontext *gc, GLenum texture, GLdouble x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1f(__GLcontext *gc, GLenum texture, GLfloat x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1i(__GLcontext *gc, GLenum texture, GLint x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1s(__GLcontext *gc, GLenum texture, GLshort x)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord1sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = 0.0f;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2d(__GLcontext *gc, GLenum texture, GLdouble x, GLdouble y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2f(__GLcontext *gc, GLenum texture, GLfloat x, GLfloat y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2i(__GLcontext *gc, GLenum texture, GLint x, GLint y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2s(__GLcontext *gc, GLenum texture, GLshort x, GLshort y)
{
    GLfloat fv[2];

    fv[0] = x;
    fv[1] = y;
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GL_MULT_TEXCOORD2FV(gc, texture, (GLfloat *)v);
}

GLvoid APIENTRY __glim_MultiTexCoord2iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord2sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    GLfloat fv[2];

    fv[0] = v[0];
    fv[1] = v[1];
    __GL_MULT_TEXCOORD2FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3d(__GLcontext *gc, GLenum texture, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3f(__GLcontext *gc, GLenum texture, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3i(__GLcontext *gc, GLenum texture, GLint x, GLint y, GLint z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3s(__GLcontext *gc, GLenum texture, GLshort x, GLshort y, GLshort z)
{
    GLfloat fv[3];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GL_MULT_TEXCOORD3FV(gc, texture, (GLfloat *)v);
}

GLvoid APIENTRY __glim_MultiTexCoord3iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord3sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __GL_MULT_TEXCOORD3FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4d(__GLcontext *gc, GLenum texture, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4f(__GLcontext *gc, GLenum texture, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4i(__GLcontext *gc, GLenum texture, GLint x, GLint y, GLint z, GLint w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4s(__GLcontext *gc, GLenum texture, GLshort x, GLshort y, GLshort z, GLshort w)
{
    GLfloat fv[4];

    fv[0] = x;
    fv[1] = y;
    fv[2] = z;
    fv[3] = w;
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4dv(__GLcontext *gc, GLenum texture, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4fv(__GLcontext *gc, GLenum texture, const GLfloat *v)
{
    __GL_MULT_TEXCOORD4FV(gc, texture, (GLfloat *)v);
}

GLvoid APIENTRY __glim_MultiTexCoord4iv(__GLcontext *gc, GLenum texture, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}

GLvoid APIENTRY __glim_MultiTexCoord4sv(__GLcontext *gc, GLenum texture, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __GL_MULT_TEXCOORD4FV(gc, texture, fv);
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#pragma warning(default: 4003)
#pragma warning(default: 4133)
#endif

