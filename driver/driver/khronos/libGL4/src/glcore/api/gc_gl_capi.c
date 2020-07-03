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
//#include "gc_gl_context.h"
#include "gc_es_context.h"
#include "gc_gl_api_inline.c"


#if defined(_WIN32)
#pragma warning(disable: 4244)
#endif


__GL_INLINE GLvoid __glColor3fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;

    if (gc->input.preVertexFormat & __GL_C3F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C3F_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }

        current = gc->input.color.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_C3F_BIT;
    }
    else
    {
        if (((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = v[0];
            gc->state.current.color.g = v[1];
            gc->state.current.color.b = v[2];
            gc->state.current.color.a = 1.0;

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT))
            {
                /* Discard the previous glColor4fv or glColor4ub */
                gc->input.vertexFormat &= ~(__GL_C4F_BIT | __GL_C4UB_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_C3F_BIT;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_C3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C3F_TAG);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            if (gc->state.current.color.a == 1.0f)
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_C3F_INDEX);

                gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.color.currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                gc->input.vertexFormat |= __GL_C3F_BIT;
            }
            else
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_C4F_INDEX);

                gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.color.currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                current[3] = 1.0;
                gc->input.vertexFormat |= __GL_C4F_BIT;
            }
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)) == 0 &&
                    (gc->state.current.color.r == v[0]) &&
                    (gc->state.current.color.g == v[1]) &&
                    (gc->state.current.color.b == v[2]) &&
                    (gc->state.current.color.a == 1.0f))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if (!(vertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = 1.0;
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}

__GL_INLINE GLvoid __glColor4fv(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;

    if (gc->input.preVertexFormat & __GL_C4F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C4F_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.color.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        current[3] = v[3];
        gc->input.vertexFormat |= __GL_C4F_BIT;
    }
    else
    {
        if (((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = v[0];
            gc->state.current.color.g = v[1];
            gc->state.current.color.b = v[2];
            gc->state.current.color.a = v[3];

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT))
            {
                /* Discard the previous glColor3fv or glColor4ub */
                gc->input.vertexFormat &= ~(__GL_C3F_BIT | __GL_C4UB_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.preVertexFormat |= __GL_C4F_BIT;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C4F_TAG);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_C4F_INDEX);

            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)) == 0 &&
                    (gc->state.current.color.r == v[0]) &&
                    (gc->state.current.color.g == v[1]) &&
                    (gc->state.current.color.b == v[2]) &&
                    (gc->state.current.color.a == v[3]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if (!(vertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}

__GL_INLINE GLvoid __glColor4ub(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GLuint *uintptr;
    GLfloat *current;
    GLfloat fr, fg, fb, fa;

    if (gc->input.preVertexFormat & __GL_C4UB_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C4UB_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        uintptr = (GLuint*)gc->input.color.currentPtrDW;
        *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
        gc->input.vertexFormat |= __GL_C4UB_BIT;
    }
    else
    {
        if (((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0) ||
            (gc->input.beginMode != __GL_IN_BEGIN))
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = __GL_UB_TO_FLOAT(r);
            gc->state.current.color.g = __GL_UB_TO_FLOAT(g);
            gc->state.current.color.b = __GL_UB_TO_FLOAT(b);
            gc->state.current.color.a = __GL_UB_TO_FLOAT(a);

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT))
            {
                /* Discard the previous glColor3fv or glColor4fv */
                gc->input.vertexFormat &= ~(__GL_C3F_BIT | __GL_C4F_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 1; /* 1 DWORD */
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 1;
            gc->input.preVertexFormat |= __GL_C4UB_BIT;
            uintptr = (GLuint*)gc->input.color.currentPtrDW;
            *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
            gc->input.vertexFormat |= __GL_C4UB_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C4UB_TAG);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_C4UB_INDEX);

            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
            uintptr = (GLuint*)gc->input.color.currentPtrDW;
            *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
            gc->input.vertexFormat |= __GL_C4UB_BIT;
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)) == 0)
                {
                    fr = __GL_UB_TO_FLOAT(r);
                    fg = __GL_UB_TO_FLOAT(g);
                    fb = __GL_UB_TO_FLOAT(b);
                    fa = __GL_UB_TO_FLOAT(a);
                    if ((gc->state.current.color.r == fr) &&
                        (gc->state.current.color.g == fg) &&
                        (gc->state.current.color.b == fb) &&
                        (gc->state.current.color.a == fa))
                    {
                        return;
                    }
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if (!(vertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = __GL_UB_TO_FLOAT(r);
            current[1] = __GL_UB_TO_FLOAT(g);
            current[2] = __GL_UB_TO_FLOAT(b);
            current[3] = __GL_UB_TO_FLOAT(a);
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}


/* OpenGL primary color APIs */



GLvoid APIENTRY __glim_Color3ub(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    __glColor4ub(gc, r, g, b, 255);
}

GLvoid APIENTRY __glim_Color3ubv(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub(gc, v[0], v[1], v[2], 255);
}

GLvoid APIENTRY __glim_Color3b(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);

    __glColor4ub(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color3bv(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);

    __glColor4ub(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color4ub(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __glColor4ub(gc, r, g, b, a);
}

GLvoid APIENTRY __glim_Color4ubv(__GLcontext *gc, const GLubyte *v)
{
    GLubyte ur = v[0];
    GLubyte ug = v[1];
    GLubyte ub = v[2];
    GLubyte ua = v[3];

    __glColor4ub(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4b(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);
    GLubyte ua = __GL_B_TO_UBYTE(a);

    __glColor4ub(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4bv(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);
    GLubyte ua = __GL_B_TO_UBYTE(v[3]);

    __glColor4ub(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color3f(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3fv(__GLcontext *gc, const GLfloat *v)
{
    __glColor3fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color4f(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4fv(__GLcontext *gc, const GLfloat *v)
{
    __glColor4fv( gc, (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color3d(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3i(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3ui(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3uiv(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3s(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3us(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color3usv(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glColor3fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4d(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4dv(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4i(__GLcontext *gc, GLint r, GLint g, GLint b, GLint a)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    fv[3] = __GL_I_TO_FLOAT(a);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4iv(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    fv[3] = __GL_I_TO_FLOAT(v[3]);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4ui(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    fv[3] = __GL_UI_TO_FLOAT(a);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4uiv(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    fv[3] = __GL_UI_TO_FLOAT(v[3]);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4s(__GLcontext *gc, GLshort r, GLshort g, GLshort b, GLshort a)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    fv[3] = __GL_S_TO_FLOAT(a);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4sv(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    fv[3] = __GL_S_TO_FLOAT(v[3]);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4us(__GLcontext *gc, GLushort r, GLushort g, GLushort b, GLushort a)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    fv[3] = __GL_US_TO_FLOAT(a);
    __glColor4fv( gc, fv );
}

GLvoid APIENTRY __glim_Color4usv(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    fv[3] = __GL_US_TO_FLOAT(v[3]);
    __glColor4fv( gc, fv );
}

/*
 * Following APIs are used for immediate mode OpenGL primary color APIs performance improvement
 */
__GL_INLINE GLvoid __glColor3fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;

    if (gc->input.preVertexFormat & __GL_C3F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C3F_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }

        current = gc->input.color.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        gc->input.vertexFormat |= __GL_C3F_BIT;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_C3F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_DIFFUSE_INDEX);
    }
    else
    {
        if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = v[0];
            gc->state.current.color.g = v[1];
            gc->state.current.color.b = v[2];
            gc->state.current.color.a = 1.0;

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT))
            {
                /* Discard the previous glColor4fv or glColor4ub */
                gc->input.vertexFormat &= ~(__GL_C4F_BIT | __GL_C4UB_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 3;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 3;
            gc->input.preVertexFormat |= __GL_C3F_BIT;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            gc->input.vertexFormat |= __GL_C3F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C3F_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_C3F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_DIFFUSE_INDEX);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            if (gc->state.current.color.a == 1.0f)
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_C3F_INDEX);

                gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.color.currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                gc->input.vertexFormat |= __GL_C3F_BIT;
            }
            else
            {
                __glSwitchToNewPrimtiveFormat(gc, __GL_C4F_INDEX);

                gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
                current = gc->input.color.currentPtrDW;
                current[0] = v[0];
                current[1] = v[1];
                current[2] = v[2];
                current[3] = 1.0;
                gc->input.vertexFormat |= __GL_C4F_BIT;
            }
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)) == 0 &&
                    (gc->state.current.color.r == v[0]) &&
                    (gc->state.current.color.g == v[1]) &&
                    (gc->state.current.color.b == v[2]) &&
                    (gc->state.current.color.a == 1.0f))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (__GL_C4F_BIT | __GL_C4UB_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = 1.0;
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}

__GL_INLINE GLvoid __glColor4fv_Info(__GLcontext *gc, GLfloat *v)
{
    GLfloat *current;
    __GLvertexInfo *vtxinfo;

    gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;

    if (gc->input.preVertexFormat & __GL_C4F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C4F_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        current = gc->input.color.currentPtrDW;
        current[0] = v[0];
        current[1] = v[1];
        current[2] = v[2];
        current[3] = v[3];
        gc->input.vertexFormat |= __GL_C4F_BIT;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_C4F_TAG;
        vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)v;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_DIFFUSE_INDEX);
    }
    else
    {
        if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = v[0];
            gc->state.current.color.g = v[1];
            gc->state.current.color.b = v[2];
            gc->state.current.color.a = v[3];

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT))
            {
                /* Discard the previous glColor3fv or glColor4ub */
                gc->input.vertexFormat &= ~(__GL_C3F_BIT | __GL_C4UB_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 4;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 4;
            gc->input.preVertexFormat |= __GL_C4F_BIT;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C4F_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_C4F_TAG;
            vtxinfo->offsetDW = (GLushort)(current - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)v;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, v);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_DIFFUSE_INDEX);
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_C4F_INDEX);

            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)) == 0 &&
                    (gc->state.current.color.r == v[0]) &&
                    (gc->state.current.color.g == v[1]) &&
                    (gc->state.current.color.b == v[2]) &&
                    (gc->state.current.color.a == v[3]))
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (__GL_C3F_BIT | __GL_C4UB_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = v[0];
            current[1] = v[1];
            current[2] = v[2];
            current[3] = v[3];
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}

__GL_INLINE GLvoid __glColor4ub_Info(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GLuint *uintptr;
    GLfloat *current;
    __GLvertexInfo *vtxinfo;
    GLfloat fr, fg, fb, fa;

    gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;

    if (gc->input.preVertexFormat & __GL_C4UB_BIT)
    {
        if ((gc->input.vertexFormat & __GL_C4UB_BIT) == 0)
        {
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        uintptr = (GLuint*)gc->input.color.currentPtrDW;
        *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
        gc->input.vertexFormat |= __GL_C4UB_BIT;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_C4UB_TAG;
        vtxinfo->offsetDW = (GLushort)(uintptr - (GLuint *)gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = NULL;
        vtxinfo->ptePointer = NULL;
    }
    else
    {
        if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
        {
            /* If glColor is not needed in glBegin/glEnd */
            gc->state.current.color.r = __GL_UB_TO_FLOAT(r);
            gc->state.current.color.g = __GL_UB_TO_FLOAT(g);
            gc->state.current.color.b = __GL_UB_TO_FLOAT(b);
            gc->state.current.color.a = __GL_UB_TO_FLOAT(a);

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0 ||
                gc->input.vertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT))
            {
                /* Discard the previous glColor3fv or glColor4fv */
                gc->input.vertexFormat &= ~(__GL_C3F_BIT | __GL_C4F_BIT);

                /* The first glColor after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glColor after glBegin */
            gc->input.color.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.color.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.color.sizeDW = 1; /* 1 DWORD */
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 1;
            gc->input.preVertexFormat |= __GL_C4UB_BIT;
            uintptr = (GLuint*)gc->input.color.currentPtrDW;
            *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
            gc->input.vertexFormat |= __GL_C4UB_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_C4UB_TAG);

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_C4UB_TAG;
            vtxinfo->offsetDW = (GLushort)(uintptr - (GLuint *)gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = NULL;
            vtxinfo->ptePointer = NULL;
        }
        else if (gc->input.preVertexFormat != 0 &&
            (gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)) == 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_C4UB_INDEX);

            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
            uintptr = (GLuint*)gc->input.color.currentPtrDW;
            *uintptr = __GL_PACK_COLOR4UB(r, g, b, a);
            gc->input.vertexFormat |= __GL_C4UB_BIT;
        }
        else
        {
            GLuint64 vertexFormat = gc->input.vertexFormat;
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if ((gc->input.preVertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)) == 0)
                {
                    fr = __GL_UB_TO_FLOAT(r);
                    fg = __GL_UB_TO_FLOAT(g);
                    fb = __GL_UB_TO_FLOAT(b);
                    fa = __GL_UB_TO_FLOAT(a);
                    if ((gc->state.current.color.r == fr) &&
                        (gc->state.current.color.g == fg) &&
                        (gc->state.current.color.b == fb) &&
                        (gc->state.current.color.a == fa))
                    {
                        return;
                    }
                }

                __glSwitchToInconsistentFormat(gc);
            }

            if(!(vertexFormat & (__GL_C3F_BIT | __GL_C4F_BIT)))
            {
                gc->input.color.currentPtrDW = (GLfloat*)gc->input.color.pointer +
                gc->input.color.index * gc->input.vertTotalStrideDW;
                gc->input.color.index += 1;
            }
            current = gc->input.color.currentPtrDW;
            current[0] = __GL_UB_TO_FLOAT(r);
            current[1] = __GL_UB_TO_FLOAT(g);
            current[2] = __GL_UB_TO_FLOAT(b);
            current[3] = __GL_UB_TO_FLOAT(a);
            gc->input.vertexFormat |= __GL_C4F_BIT;
        }
    }
}

__GL_INLINE GLvoid __glColor3fv_Cache(__GLcontext *gc, GLuint *col)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_C3F_TAG)
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
            __glImmedFlushBuffer_Cache(gc, __GL_C3F_TAG);
            (*gc->currentImmediateDispatch->Color3fv)(gc, (GLfloat *)col);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_DIFFUSE)
        {
            if (gc->input.beginMode == __GL_IN_BEGIN)
            {
                /* Switch the current vertex buffer back to the default vertex buffer */
                __glSwitchToDefaultVertexBuffer(gc, __GL_C3F_TAG);

                /* Write vertex data into the default vertex buffer */
                (*gc->currentImmediateDispatch->Color3fv)(gc, (GLfloat *)col);
            }
            else
            {
                /* Save the color in the shadow current state */
                gc->input.shadowCurrent.color.r = ((GLfloat *)col)[0];
                gc->input.shadowCurrent.color.g = ((GLfloat *)col)[1];
                gc->input.shadowCurrent.color.b = ((GLfloat *)col)[2];
                gc->input.shadowCurrent.color.a = 1.0;

                gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;

                /* Call __glim_Begin_Cache_First next to evaluate the deferred attribute */
                // __GL_OVERWRITE_OPENGL32_BEGIN_ENTRY()
            }
        }
        else
        {
            /* Color is not needed, just update current color state */
            gc->state.current.color.r = ((GLfloat *)col)[0];
            gc->state.current.color.g = ((GLfloat *)col)[1];
            gc->state.current.color.b = ((GLfloat *)col)[2];
            gc->state.current.color.a = 1.0;

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
    }
}

__GL_INLINE GLvoid __glColor4fv_Cache(__GLcontext *gc, GLuint *col)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_C4F_TAG)
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
        if (((col[0] ^ buf[0]) | (col[1] ^ buf[1]) | (col[2] ^ buf[2]) | (col[3] ^ buf[3])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {
        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_C4F_TAG);
            (*gc->currentImmediateDispatch->Color4fv)(gc, (GLfloat *)col);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_DIFFUSE)
        {
            if (gc->input.beginMode == __GL_IN_BEGIN)
            {
                /* Switch the current vertex buffer back to the default vertex buffer */
                __glSwitchToDefaultVertexBuffer(gc, __GL_C4F_TAG);

                /* Write vertex data into the default vertex buffer */
                (*gc->currentImmediateDispatch->Color4fv)(gc, (GLfloat *)col);
            }
            else
            {
                /* Save the color in the shadow current state */
                gc->input.shadowCurrent.color.r = ((GLfloat *)col)[0];
                gc->input.shadowCurrent.color.g = ((GLfloat *)col)[1];
                gc->input.shadowCurrent.color.b = ((GLfloat *)col)[2];
                gc->input.shadowCurrent.color.a = ((GLfloat *)col)[3];

                gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;
            }
        }
        else
        {
            /* Color is not needed, just update current color state */
            gc->state.current.color.r = ((GLfloat *)col)[0];
            gc->state.current.color.g = ((GLfloat *)col)[1];
            gc->state.current.color.b = ((GLfloat *)col)[2];
            gc->state.current.color.a = ((GLfloat *)col)[3];

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
    }
}

__GL_INLINE GLvoid __glColor4ub_Cache(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GLuint col, *buf;
    __GLvertexInfo *vtxinfo;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_C4UB_TAG)
    {
        buf = (GLuint *)gc->pVertexDataBufPtr + vtxinfo->offsetDW;
        col = __GL_PACK_COLOR4UB(r, g, b, a);

        /* If incoming vertex data are the same as cached vertex data, just return */
        if (((col ^ buf[0])) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {
        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_C4UB_TAG);
            (*gc->currentImmediateDispatch->Color4ub)(gc, r, g, b, a);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_DIFFUSE)
        {
            if (gc->input.beginMode == __GL_IN_BEGIN)
            {
                /* Switch the current vertex buffer back to the default vertex buffer */
                __glSwitchToDefaultVertexBuffer(gc, __GL_C4UB_TAG);

                /* Write vertex data into the default vertex buffer */
                (*gc->currentImmediateDispatch->Color4ub)(gc, r, g, b, a);
            }
            else
            {
                /* Save the color in the shadow current state */
                gc->input.shadowCurrent.color.r = __GL_UB_TO_FLOAT(r);
                gc->input.shadowCurrent.color.g = __GL_UB_TO_FLOAT(g);
                gc->input.shadowCurrent.color.b = __GL_UB_TO_FLOAT(b);
                gc->input.shadowCurrent.color.a = __GL_UB_TO_FLOAT(a);

                gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;

                /* Call __glim_Begin_Cache_First next to evaluate the deferred attribute */
                // __GL_OVERWRITE_OPENGL32_BEGIN_ENTRY()
            }
        }
        else
        {
            /* Color is not needed, just update current color state */
            gc->state.current.color.r = __GL_UB_TO_FLOAT(r);
            gc->state.current.color.g = __GL_UB_TO_FLOAT(g);
            gc->state.current.color.b = __GL_UB_TO_FLOAT(b);
            gc->state.current.color.a = __GL_UB_TO_FLOAT(a);

            /* Use the current color to update material state if color material is enabled */
            if (gc->state.enables.lighting.colorMaterial) {
                __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
            }
        }
    }
}

__GL_INLINE GLvoid __glColor3fv_Outside(__GLcontext *gc, GLfloat *v)
{
    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
    {
        /* glColor is not needed in glBegin/glEnd.
        */
        gc->state.current.color.r = v[0];
        gc->state.current.color.g = v[1];
        gc->state.current.color.b = v[2];
        gc->state.current.color.a = 1.0;

        gc->input.shadowCurrent.color = gc->state.current.color;
        gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;
    }
    else
    {
        /* glColor is needed in glBegin/glEnd.
        **
        ** Save the color in the shadow current state.
        */
        gc->input.shadowCurrent.color.r = v[0];
        gc->input.shadowCurrent.color.g = v[1];
        gc->input.shadowCurrent.color.b = v[2];
        gc->input.shadowCurrent.color.a = 1.0;

        gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;
    }

    /* Use the current color to update material state if color material is enabled */
    if (gc->state.enables.lighting.colorMaterial &&
        !(gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT))
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }
}

__GL_INLINE GLvoid __glColor4fv_Outside(__GLcontext *gc, GLfloat *v)
{
    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
    {
        /* glColor is not needed in glBegin/glEnd.
        */
        gc->state.current.color.r = v[0];
        gc->state.current.color.g = v[1];
        gc->state.current.color.b = v[2];
        gc->state.current.color.a = v[3];

        gc->input.shadowCurrent.color = gc->state.current.color;
        gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;
    }
    else
    {
        /* glColor is needed in glBegin/glEnd.
        **
        ** Save the color in the shadow current state.
        */
        gc->input.shadowCurrent.color.r = v[0];
        gc->input.shadowCurrent.color.g = v[1];
        gc->input.shadowCurrent.color.b = v[2];
        gc->input.shadowCurrent.color.a = v[3];

        gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;
    }

    /* Use the current color to update material state if color material is enabled */
    if (gc->state.enables.lighting.colorMaterial &&
        !(gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT))
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }
}

__GL_INLINE GLvoid __glColor4ub_Outside(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GLfloat fr = __GL_UB_TO_FLOAT(r);
    GLfloat fg = __GL_UB_TO_FLOAT(g);
    GLfloat fb = __GL_UB_TO_FLOAT(b);
    GLfloat fa = __GL_UB_TO_FLOAT(a);

    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_DIFFUSE) == 0)
    {
        /* glColor is not needed in glBegin/glEnd.
        */
        gc->state.current.color.r = fr;
        gc->state.current.color.g = fg;
        gc->state.current.color.b = fb;
        gc->state.current.color.a = fa;

        gc->input.shadowCurrent.color = gc->state.current.color;
        gc->input.deferredAttribDirty &= ~__GL_DEFERED_COLOR_BIT;
    }
    else
    {
        /* glColor is needed in glBegin/glEnd.
        **
        ** Save the color in the shadow current state.
        */
        gc->input.shadowCurrent.color.r = fr;
        gc->input.shadowCurrent.color.g = fg;
        gc->input.shadowCurrent.color.b = fb;
        gc->input.shadowCurrent.color.a = fa;

        gc->input.deferredAttribDirty |= __GL_DEFERED_COLOR_BIT;
    }

    /* Use the current color to update material state if color material is enabled */
    if (gc->state.enables.lighting.colorMaterial &&
        !(gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT))
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }
}

GLvoid APIENTRY __glim_Color3ub_Info(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    __glColor4ub_Info(gc, r, g, b, 255);
}

GLvoid APIENTRY __glim_Color3ubv_Info(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Info(gc, v[0], v[1], v[2], 255);
}

GLvoid APIENTRY __glim_Color3b_Info(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);

    __glColor4ub_Info(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color3bv_Info(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);

    __glColor4ub_Info(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color4ub_Info(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __glColor4ub_Info(gc, r, g, b, a);
}

GLvoid APIENTRY __glim_Color4ubv_Info(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Info(gc, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_Color4b_Info(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);
    GLubyte ua = __GL_B_TO_UBYTE(a);

    __glColor4ub_Info(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4bv_Info(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);
    GLubyte ua = __GL_B_TO_UBYTE(v[3]);

    __glColor4ub_Info(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color3f_Info(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3fv_Info(__GLcontext *gc, const GLfloat *v)
{
    __glColor3fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color4f_Info(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4fv_Info(__GLcontext *gc, const GLfloat *v)
{
    __glColor4fv_Info(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color3d_Info(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3i_Info(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3ui_Info(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3uiv_Info(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3s_Info(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3us_Info(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color3usv_Info(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glColor3fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4d_Info(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4dv_Info(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4i_Info(__GLcontext *gc, GLint r, GLint g, GLint b, GLint a)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    fv[3] = __GL_I_TO_FLOAT(a);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4iv_Info(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    fv[3] = __GL_I_TO_FLOAT(v[3]);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4ui_Info(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    fv[3] = __GL_UI_TO_FLOAT(a);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4uiv_Info(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    fv[3] = __GL_UI_TO_FLOAT(v[3]);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4s_Info(__GLcontext *gc, GLshort r, GLshort g, GLshort b, GLshort a)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    fv[3] = __GL_S_TO_FLOAT(a);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4sv_Info(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    fv[3] = __GL_S_TO_FLOAT(v[3]);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4us_Info(__GLcontext *gc, GLushort r, GLushort g, GLushort b, GLushort a)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    fv[3] = __GL_US_TO_FLOAT(a);
    __glColor4fv_Info(gc,  fv );
}

GLvoid APIENTRY __glim_Color4usv_Info(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    fv[3] = __GL_US_TO_FLOAT(v[3]);
    __glColor4fv_Info(gc,  fv );
}

/*************************************************************************/

GLvoid APIENTRY __glim_Color3ub_Cache(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    __glColor4ub_Cache(gc, r, g, b, 255);
}

GLvoid APIENTRY __glim_Color3ubv_Cache(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Cache(gc, v[0], v[1], v[2], 255);
}

GLvoid APIENTRY __glim_Color3b_Cache(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);

    __glColor4ub_Cache(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color3bv_Cache(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);

    __glColor4ub_Cache(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color4ubv_Cache(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Cache(gc, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_Color4b_Cache(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);
    GLubyte ua = __GL_B_TO_UBYTE(a);

    __glColor4ub_Cache(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4bv_Cache(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);
    GLubyte ua = __GL_B_TO_UBYTE(v[3]);

    __glColor4ub_Cache(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color3f_Cache(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4f_Cache(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glColor4fv_Cache(gc,  (GLuint *)v );
}

GLvoid APIENTRY __glim_Color3d_Cache(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3i_Cache(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3ui_Cache(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3uiv_Cache(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3s_Cache(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3us_Cache(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color3usv_Cache(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glColor3fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4d_Cache(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4dv_Cache(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4i_Cache(__GLcontext *gc, GLint r, GLint g, GLint b, GLint a)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    fv[3] = __GL_I_TO_FLOAT(a);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4iv_Cache(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    fv[3] = __GL_I_TO_FLOAT(v[3]);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4ui_Cache(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    fv[3] = __GL_UI_TO_FLOAT(a);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4uiv_Cache(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    fv[3] = __GL_UI_TO_FLOAT(v[3]);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4s_Cache(__GLcontext *gc, GLshort r, GLshort g, GLshort b, GLshort a)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    fv[3] = __GL_S_TO_FLOAT(a);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4sv_Cache(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    fv[3] = __GL_S_TO_FLOAT(v[3]);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4us_Cache(__GLcontext *gc, GLushort r, GLushort g, GLushort b, GLushort a)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    fv[3] = __GL_US_TO_FLOAT(a);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

GLvoid APIENTRY __glim_Color4usv_Cache(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    fv[3] = __GL_US_TO_FLOAT(v[3]);
    __glColor4fv_Cache(gc,  (GLuint *)fv );
}

/*************************************************************************/

GLvoid APIENTRY __glim_Color3ub_Outside(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b)
{
    __glColor4ub_Outside(gc, r, g, b, 255);
}

GLvoid APIENTRY __glim_Color3ubv_Outside(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Outside(gc, v[0], v[1], v[2], 255);
}

GLvoid APIENTRY __glim_Color3b_Outside(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);

    __glColor4ub_Outside(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color3bv_Outside(__GLcontext *gc, const GLbyte *v)
{

    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);

    __glColor4ub_Outside(gc, ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color4ub_Outside(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __glColor4ub_Outside(gc, r, g, b, a);
}

GLvoid APIENTRY __glim_Color4ubv_Outside(__GLcontext *gc, const GLubyte *v)
{
    __glColor4ub_Outside(gc, v[0], v[1], v[2], v[3]);
}

GLvoid APIENTRY __glim_Color4b_Outside(__GLcontext *gc, GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);
    GLubyte ua = __GL_B_TO_UBYTE(a);

    __glColor4ub_Outside(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4bv_Outside(__GLcontext *gc, const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);
    GLubyte ua = __GL_B_TO_UBYTE(v[3]);

    __glColor4ub_Outside(gc, ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color3f_Outside(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3fv_Outside(__GLcontext *gc, const GLfloat *v)
{
    __glColor3fv_Outside(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color4f_Outside(__GLcontext *gc, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4fv_Outside(__GLcontext *gc, const GLfloat *v)
{
    __glColor4fv_Outside(gc,  (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color3d_Outside(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3dv_Outside(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[3];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3i_Outside(__GLcontext *gc, GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3iv_Outside(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3ui_Outside(__GLcontext *gc, GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3uiv_Outside(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[3];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3s_Outside(__GLcontext *gc, GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3sv_Outside(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3us_Outside(__GLcontext *gc, GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color3usv_Outside(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[3];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glColor3fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4d_Outside(__GLcontext *gc, GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    GLfloat fv[4];

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4dv_Outside(__GLcontext *gc, const GLdouble *v)
{
    GLfloat fv[4];

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4i_Outside(__GLcontext *gc, GLint r, GLint g, GLint b, GLint a)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    fv[3] = __GL_I_TO_FLOAT(a);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4iv_Outside(__GLcontext *gc, const GLint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    fv[3] = __GL_I_TO_FLOAT(v[3]);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4ui_Outside(__GLcontext *gc, GLuint r, GLuint g, GLuint b, GLuint a)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    fv[3] = __GL_UI_TO_FLOAT(a);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4uiv_Outside(__GLcontext *gc, const GLuint *v)
{
    GLfloat fv[4];

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    fv[3] = __GL_UI_TO_FLOAT(v[3]);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4s_Outside(__GLcontext *gc, GLshort r, GLshort g, GLshort b, GLshort a)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    fv[3] = __GL_S_TO_FLOAT(a);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4sv_Outside(__GLcontext *gc, const GLshort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    fv[3] = __GL_S_TO_FLOAT(v[3]);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4us_Outside(__GLcontext *gc, GLushort r, GLushort g, GLushort b, GLushort a)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    fv[3] = __GL_US_TO_FLOAT(a);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4usv_Outside(__GLcontext *gc, const GLushort *v)
{
    GLfloat fv[4];

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    fv[3] = __GL_US_TO_FLOAT(v[3]);
    __glColor4fv_Outside(gc,  fv );
}

GLvoid APIENTRY __glim_Color4ubv_4fv(__GLcontext *gc, const GLubyte *v)
{
    GLfloat fv[4];

    fv[0] = __GL_UB_TO_FLOAT(v[0]);
    fv[1] = __GL_UB_TO_FLOAT(v[1]);
    fv[2] = __GL_UB_TO_FLOAT(v[2]);
    fv[3] = __GL_UB_TO_FLOAT(v[3]);
    __glColor4fv(gc, fv);
}

/*************************************************************************/
GLvoid APIENTRY __glim_Color4ub_Cache(__GLcontext *gc, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __glColor4ub_Cache(gc, r, g, b, a);
}

GLvoid APIENTRY __glim_Color3fv_Cache(__GLcontext *gc, const GLfloat *v)
{
    __glColor3fv_Cache(gc,  (GLuint *)v );
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

#endif
