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



__GL_INLINE GLvoid __glEdgeFlag(__GLcontext *gc, GLboolean tag)
{
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

GLvoid APIENTRY __glim_EdgeFlag(__GLcontext *gc, GLboolean tag)
{
    __glEdgeFlag(gc, tag);
}

GLvoid APIENTRY __glim_EdgeFlagv(__GLcontext *gc, const GLboolean *tag)
{
    __glEdgeFlag(gc, tag[0]);
}

