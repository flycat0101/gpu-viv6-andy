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


__GL_INLINE GLvoid __glColor3fv(GLfloat *v)
{
    __GL_SETUP();
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

__GL_INLINE GLvoid __glColor4fv(GLfloat *v)
{
    __GL_SETUP();
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

__GL_INLINE GLvoid __glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    __GL_SETUP();
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

GLvoid APIENTRY __glim_Color3ub(GLubyte r, GLubyte g, GLubyte b)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3ub", DT_GLubyte, r, DT_GLubyte, g, DT_GLubyte, b, DT_GLnull);
#endif

    __glColor4ub(r, g, b, 255);
}

GLvoid APIENTRY __glim_Color3ubv(const GLubyte *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3ubv", DT_GLubyte_ptr, v, DT_GLnull);
#endif

    __glColor4ub(v[0], v[1], v[2], 255);
}

GLvoid APIENTRY __glim_Color3b(GLbyte r, GLbyte g, GLbyte b)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3b", DT_GLbyte, r, DT_GLbyte, g, DT_GLbyte, b, DT_GLnull);
#endif

    __glColor4ub(ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color3bv(const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3bv", DT_GLbyte_ptr, v, DT_GLnull);
#endif

    __glColor4ub(ur, ug, ub, 255);
}

GLvoid APIENTRY __glim_Color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4ub", DT_GLubyte, r, DT_GLubyte, g, DT_GLubyte, b, DT_GLubyte, a, DT_GLnull);
#endif

    __glColor4ub(r, g, b, a);
}

GLvoid APIENTRY __glim_Color4ubv(const GLubyte *v)
{
     GLubyte ur = __GL_UB_TO_FLOAT(v[0]);
     GLubyte ug = __GL_UB_TO_FLOAT(v[1]);
     GLubyte ub = __GL_UB_TO_FLOAT(v[2]);
     GLubyte ua  = __GL_UB_TO_FLOAT(v[3]);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4ubv", DT_GLubyte_ptr, v, DT_GLnull);
#endif

    __glColor4ub(ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4b(GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
    GLubyte ur = __GL_B_TO_UBYTE(r);
    GLubyte ug = __GL_B_TO_UBYTE(g);
    GLubyte ub = __GL_B_TO_UBYTE(b);
    GLubyte ua = __GL_B_TO_UBYTE(a);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4b", DT_GLbyte, r, DT_GLbyte, g, DT_GLbyte, b, DT_GLbyte, a, DT_GLnull);
#endif

    __glColor4ub(ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color4bv(const GLbyte *v)
{
    GLubyte ur = __GL_B_TO_UBYTE(v[0]);
    GLubyte ug = __GL_B_TO_UBYTE(v[1]);
    GLubyte ub = __GL_B_TO_UBYTE(v[2]);
    GLubyte ua = __GL_B_TO_UBYTE(v[3]);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4bv", DT_GLbyte_ptr, v, DT_GLnull);
#endif

    __glColor4ub(ur, ug, ub, ua);
}

GLvoid APIENTRY __glim_Color3f(GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3f", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLnull);
#endif

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3fv(const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glColor3fv( (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4f", DT_GLfloat, r, DT_GLfloat, g, DT_GLfloat, b, DT_GLfloat, a, DT_GLnull);
#endif

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4fv(const GLfloat *v)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4fv", DT_GLfloat_ptr, v, DT_GLnull);
#endif

    __glColor4fv( (GLfloat *)v );
}

GLvoid APIENTRY __glim_Color3d(GLdouble r, GLdouble g, GLdouble b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3d", DT_GLdouble, r, DT_GLdouble, g, DT_GLdouble, b, DT_GLnull);
#endif

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3dv(const GLdouble *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3dv", DT_GLdouble_ptr, *v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3i(GLint r, GLint g, GLint b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3i", DT_GLint, r, DT_GLint, g, DT_GLint, b, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3iv(const GLint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3ui(GLuint r, GLuint g, GLuint b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3ui", DT_GLuint, r, DT_GLuint, g, DT_GLuint, b, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3uiv(const GLuint *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3uiv", DT_GLuint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3s(GLshort r, GLshort g, GLshort b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3s", DT_GLshort, r, DT_GLshort, g, DT_GLshort, b, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3sv(const GLshort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3us(GLushort r, GLushort g, GLushort b)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3us", DT_GLushort, r, DT_GLushort, g, DT_GLushort, b, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color3usv(const GLushort *v)
{
    GLfloat fv[3];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color3usv", DT_GLushort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    __glColor3fv( fv );
}

GLvoid APIENTRY __glim_Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4d", DT_GLdouble, r, DT_GLdouble, g, DT_GLdouble, b, DT_GLdouble, a, DT_GLnull);
#endif

    fv[0] = r;
    fv[1] = g;
    fv[2] = b;
    fv[3] = a;
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4dv(const GLdouble *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4dv", DT_GLdouble_ptr, v, DT_GLnull);
#endif

    fv[0] = v[0];
    fv[1] = v[1];
    fv[2] = v[2];
    fv[3] = v[3];
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4i(GLint r, GLint g, GLint b, GLint a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4i", DT_GLint, r, DT_GLint, g, DT_GLint, b, DT_GLint, a, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(r);
    fv[1] = __GL_I_TO_FLOAT(g);
    fv[2] = __GL_I_TO_FLOAT(b);
    fv[3] = __GL_I_TO_FLOAT(a);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4iv(const GLint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4iv", DT_GLint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_I_TO_FLOAT(v[0]);
    fv[1] = __GL_I_TO_FLOAT(v[1]);
    fv[2] = __GL_I_TO_FLOAT(v[2]);
    fv[3] = __GL_I_TO_FLOAT(v[3]);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4ui(GLuint r, GLuint g, GLuint b, GLuint a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4ui", DT_GLuint, r, DT_GLuint, g, DT_GLuint, b, DT_GLuint, a, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(r);
    fv[1] = __GL_UI_TO_FLOAT(g);
    fv[2] = __GL_UI_TO_FLOAT(b);
    fv[3] = __GL_UI_TO_FLOAT(a);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4uiv(const GLuint *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4uiv", DT_GLuint_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_UI_TO_FLOAT(v[0]);
    fv[1] = __GL_UI_TO_FLOAT(v[1]);
    fv[2] = __GL_UI_TO_FLOAT(v[2]);
    fv[3] = __GL_UI_TO_FLOAT(v[3]);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4s(GLshort r, GLshort g, GLshort b, GLshort a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4s", DT_GLshort, r, DT_GLshort, g, DT_GLshort, b, DT_GLshort, a, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(r);
    fv[1] = __GL_S_TO_FLOAT(g);
    fv[2] = __GL_S_TO_FLOAT(b);
    fv[3] = __GL_S_TO_FLOAT(a);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4sv(const GLshort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4sv", DT_GLshort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_S_TO_FLOAT(v[0]);
    fv[1] = __GL_S_TO_FLOAT(v[1]);
    fv[2] = __GL_S_TO_FLOAT(v[2]);
    fv[3] = __GL_S_TO_FLOAT(v[3]);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4us(GLushort r, GLushort g, GLushort b, GLushort a)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4us", DT_GLushort, r, DT_GLushort, g, DT_GLushort, b, DT_GLushort, a, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(r);
    fv[1] = __GL_US_TO_FLOAT(g);
    fv[2] = __GL_US_TO_FLOAT(b);
    fv[3] = __GL_US_TO_FLOAT(a);
    __glColor4fv( fv );
}

GLvoid APIENTRY __glim_Color4usv(const GLushort *v)
{
    GLfloat fv[4];

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Color4usv", DT_GLushort_ptr, v, DT_GLnull);
#endif

    fv[0] = __GL_US_TO_FLOAT(v[0]);
    fv[1] = __GL_US_TO_FLOAT(v[1]);
    fv[2] = __GL_US_TO_FLOAT(v[2]);
    fv[3] = __GL_US_TO_FLOAT(v[3]);
    __glColor4fv( fv );
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif

