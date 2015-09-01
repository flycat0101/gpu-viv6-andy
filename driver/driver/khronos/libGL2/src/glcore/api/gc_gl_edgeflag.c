/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"
#include "gc_gl_debug.h"



__GL_INLINE GLvoid __glEdgeFlag(GLboolean tag)
{
    __GL_SETUP();

    if (gc->input.preVertexFormat & __GL_EDGEFLAG_BIT)
    {
        gc->input.edgeflag.pointer[gc->input.vertex.index] = tag;
        gc->input.vertexFormat |= __GL_EDGEFLAG_BIT;
    }
    else
    {
        if ((gc->input.requiredInputMask & __GL_INPUT_EDGEFLAG) == 0)
        {
            /* If gledgeflag is not needed in glBegin/glEnd */
            gc->state.current.edgeflag = tag;
        }
        else if (gc->input.lastVertexIndex == gc->input.vertex.index)
        {
            if (gc->input.lastVertexIndex != 0)
            {
                /* The first gledgeflag after glBegin has different format from the previous primitives */
                __glConsistentFormatChange(gc);
            }

            /* For the first glEdgeflag after glBegin */
            gc->input.edgeflag.pointer[gc->input.vertex.index] = tag;
            gc->input.vertexFormat |= __GL_EDGEFLAG_BIT;
            gc->input.preVertexFormat |= __GL_EDGEFLAG_BIT;
            __GL_PRIM_ELEMENT(gc->input.primElemSequence, __GL_EDGEFLAG_TAG);
        }
        else
        {
            /* The vertex format is changed in the middle of glBegin/glEnd. */
            if (gc->input.inconsistentFormat == GL_FALSE)
            {
                if (gc->state.current.edgeflag == tag)
                {
                    return;
                }

                __glSwitchToInconsistentFormat(gc);
            }

            gc->input.edgeflag.pointer[gc->input.edgeflag.index++] = tag;
            gc->input.vertexFormat |= __GL_EDGEFLAG_BIT;
        }
    }
}

/* OpenGL edge flag APIs */

GLvoid APIENTRY __glim_EdgeFlag(GLboolean tag)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EdgeFlag", DT_GLboolean, tag, DT_GLnull);
#endif

    __glEdgeFlag(tag);
}

GLvoid APIENTRY __glim_EdgeFlagv(const GLboolean *tag)
{
#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_EdgeFlagv", DT_GLboolean_ptr, tag, DT_GLnull);
#endif
    __glEdgeFlag(tag[0]);
}

