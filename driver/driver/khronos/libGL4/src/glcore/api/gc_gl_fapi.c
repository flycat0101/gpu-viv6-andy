/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
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



__GL_INLINE GLvoid __glFogCoordfv(__GLcontext *gc, GLfloat *fc)
{

    if (gc->input.preVertexFormat & __GL_FOG1F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_FOG1F_BIT) == 0)
        {
            gc->input.fog.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        *gc->input.fog.currentPtrDW = *fc;
        gc->input.vertexFormat |= __GL_FOG1F_BIT;
    }
    else
    {
        if ((gc->input.currentInputMask & __GL_INPUT_FOGCOORD) == 0)
        {
            /* If glfog is not needed in glBegin/glEnd */
            gc->state.current.fog = *fc;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first glfog after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glfog after glBegin */
            gc->input.fog.offsetDW = (GLuint)(gc->input.currentDataBufPtr - gc->input.primBeginAddr);
            gc->input.fog.currentPtrDW = gc->input.currentDataBufPtr;
            gc->input.fog.pointer = (GLubyte*)gc->input.currentDataBufPtr;
            gc->input.fog.sizeDW = 1;
            gc->input.currentDataBufPtr = gc->input.currentDataBufPtr + 1;
            gc->input.preVertexFormat |= __GL_FOG1F_BIT;
            *gc->input.fog.currentPtrDW = *fc;
            gc->input.vertexFormat |= __GL_FOG1F_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_FOG1F_TAG);
        }
        else if (gc->input.preVertexFormat != 0)
        {
            /* If a new vertex attribute occurs in the middle of glBegin and glEnd */
            __glSwitchToNewPrimtiveFormat(gc, __GL_FOG1F_INDEX);

            gc->input.fog.currentPtrDW += gc->input.vertTotalStrideDW;
            *gc->input.fog.currentPtrDW = *fc;
            gc->input.vertexFormat |= __GL_FOG1F_BIT;
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if (gc->state.current.fog == *fc)
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.fog.currentPtrDW = (GLfloat*)gc->input.fog.pointer +
                gc->input.fog.index * gc->input.vertTotalStrideDW;
            *gc->input.fog.currentPtrDW = *fc;
            gc->input.fog.index += 1;
            gc->input.vertexFormat |= __GL_FOG1F_BIT;
        }
    }
}

/* OpenGL fog APIs */

GLvoid APIENTRY __glim_FogCoordd(__GLcontext *gc, GLdouble coord)
{
    GLfloat fv[1];

    fv[0] = coord;
    __glFogCoordfv( gc, fv );
}

GLvoid APIENTRY __glim_FogCoorddv(__GLcontext *gc, const GLdouble *coord)
{
    GLfloat fv[1];

    fv[0] = *coord;
    __glFogCoordfv(gc, fv );
}

GLvoid APIENTRY __glim_FogCoordf(__GLcontext *gc, GLfloat coord)
{
    __glFogCoordfv(gc, &coord );
}

GLvoid APIENTRY __glim_FogCoordfv(__GLcontext *gc, const GLfloat *coord)
{
    __glFogCoordfv(gc, (GLfloat *)coord );
}


#if defined(_WIN32)
#pragma warning(default: 4244)
#endif


