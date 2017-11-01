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
     GLubyte ur = __GL_UB_TO_FLOAT(v[0]);
     GLubyte ug = __GL_UB_TO_FLOAT(v[1]);
     GLubyte ub = __GL_UB_TO_FLOAT(v[2]);
     GLubyte ua  = __GL_UB_TO_FLOAT(v[3]);

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

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

#endif