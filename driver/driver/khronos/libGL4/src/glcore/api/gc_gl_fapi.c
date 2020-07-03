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

/*
 * Following APIs are used for immediate mode OpenGL fog APIs
 * performance improvement
 */

__GL_INLINE void __glFogCoordfv_Info(__GLcontext *gc, GLfloat *fc)
{
    __GLvertexInfo *vtxinfo;

    if (gc->input.preVertexFormat & __GL_FOG1F_BIT)
    {
        if ((gc->input.vertexFormat & __GL_FOG1F_BIT) == 0)
        {
            gc->input.fog.currentPtrDW += gc->input.vertTotalStrideDW;
        }
        *gc->input.fog.currentPtrDW = *fc;
        gc->input.vertexFormat |= __GL_FOG1F_BIT;

        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = __GL_FOG1F_TAG;
        vtxinfo->offsetDW = (GLushort)(gc->input.fog.currentPtrDW - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)fc;
        vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, fc);
        __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_FOGCOORD_INDEX);
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

            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_FOG1F_TAG;
            vtxinfo->offsetDW = (GLushort)(gc->input.fog.currentPtrDW - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)fc;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, fc);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_FOGCOORD_INDEX);
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

__GL_INLINE void __glFogCoordfv_Cache(__GLcontext *gc, GLuint *fog)
{
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLuint *buf;

    vtxinfo = gc->pCurrentInfoBufPtr;

    /* If the inputTag matches the incoming data */
    if (vtxinfo->inputTag == __GL_FOG1F_TAG)
    {
        /* If the cached vertex data pointer matches the incoming pointer */
        if (vtxinfo->appDataPtr == fog)
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
        if ((fog[0] ^ buf[0]) == 0)
        {
            gc->pCurrentInfoBufPtr++;
            return;
        }
    }

    {

        /* If it is the end of vertex cache buffer then flush the vertex data */
        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, __GL_FOG1F_TAG);
            (*gc->currentImmediateDispatch->FogCoordfv)(gc, (GLfloat *)fog);
            return;
        }

        if (gc->input.currentInputMask & __GL_INPUT_FOGCOORD)
        {
            /* Switch the current vertex buffer back to the default vertex buffer */
            __glSwitchToDefaultVertexBuffer(gc, __GL_FOG1F_TAG);

            /* Write vertex data into the default vertex buffer */
            (*gc->currentImmediateDispatch->FogCoordfv)(gc, (GLfloat *)fog);
        }
        else
        {
            /* FogCoord is not needed, just update current FogCoord state */
            gc->state.current.fog = *(GLfloat *)fog;
        }
    }
}

__GL_INLINE void __glFogCoordfv_Outside(__GLcontext *gc, GLfloat *fc)
{
    __GL_DLIST_BUFFER_FLUSH(gc);

    if ((gc->input.currentInputMask & __GL_INPUT_FOGCOORD) == 0 ||
        gc->input.beginMode != __GL_SMALL_DRAW_BATCH)
    {
        /* FogCoord is not needed in glBegin/glEnd.
        */
        gc->state.current.fog = *fc;
    }
    else
    {
        /* FogCoord is needed in glBegin/glEnd.
        */
        if (gc->input.prevPrimInputMask & __GL_INPUT_FOGCOORD)
        {
            /* If previous primitive has FogCoord in glBegin/glEnd.
            */
            __glPrimitiveBatchEnd(gc);

            gc->state.current.fog = *fc;
        }
        else
        {
            /* Previous primitive has no FogCoord (but it needs FogCoord) in glBegin/glEnd.
            */
            if (gc->state.current.fog != *fc)
            {
                __glPrimitiveBatchEnd(gc);

                gc->state.current.fog = *fc;
            }
        }
    }
}

/***********************************************************************************/
GLvoid APIENTRY __glim_FogCoordd_Info(__GLcontext *gc, GLdouble coord)
{
    GLfloat fv[1];

    fv[0] = coord;
    __glFogCoordfv_Info(gc, fv );
}

GLvoid APIENTRY __glim_FogCoorddv_Info(__GLcontext *gc, const GLdouble *coord)
{
    GLfloat fv[1];

    fv[0] = *coord;
    __glFogCoordfv_Info(gc, fv );
}

GLvoid APIENTRY __glim_FogCoordf_Info(__GLcontext *gc, GLfloat coord)
{
    __glFogCoordfv_Info(gc, &coord );
}

GLvoid APIENTRY __glim_FogCoordfv_Info(__GLcontext *gc, const GLfloat *coord)
{
    __glFogCoordfv_Info(gc, (GLfloat *)coord );
}

/************************************************************************/

GLvoid APIENTRY __glim_FogCoordd_Cache(__GLcontext *gc, GLdouble coord)
{
    GLfloat fv[1];

    fv[0] = coord;
    __glFogCoordfv_Cache(gc, (GLuint *)fv );
}

GLvoid APIENTRY __glim_FogCoorddv_Cache(__GLcontext *gc, const GLdouble *coord)
{
    GLfloat fv[1];

    fv[0] = *coord;
    __glFogCoordfv_Cache(gc, (GLuint *)fv );
}

GLvoid APIENTRY __glim_FogCoordf_Cache(__GLcontext *gc, GLfloat coord)
{
    __glFogCoordfv_Cache(gc, (GLuint *)&coord );
}

GLvoid APIENTRY __glim_FogCoordfv_Cache(__GLcontext *gc, const GLfloat *coord)
{
    __glFogCoordfv_Cache(gc, (GLuint *)coord );
}

/************************************************************************/

GLvoid APIENTRY __glim_FogCoordd_Outside(__GLcontext *gc, GLdouble coord)
{
    GLfloat fv[1];

    fv[0] = coord;
    __glFogCoordfv_Outside(gc, fv );
}

GLvoid APIENTRY __glim_FogCoorddv_Outside(__GLcontext *gc, const GLdouble *coord)
{
    GLfloat fv[1];

    fv[0] = *coord;
    __glFogCoordfv_Outside(gc, fv );
}

GLvoid APIENTRY __glim_FogCoordf_Outside(__GLcontext *gc, GLfloat coord)
{
    __glFogCoordfv_Outside(gc, &coord );
}

GLvoid APIENTRY __glim_FogCoordfv_Outside(__GLcontext *gc, const GLfloat *coord)
{
    __glFogCoordfv_Outside(gc, (GLfloat *)coord );
}

#if defined(_WIN32)
#pragma warning(default: 4244)
#endif


