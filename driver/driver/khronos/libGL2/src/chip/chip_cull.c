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
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

GLenum setCulling(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceCULL mode;
    GLenum result;

    gcmHEADER_ARG("Context=0x%x", gc);

    if (gc->state.enables.polygon.cullFace)
    {
        if (gc->state.polygon.cullFace == GL_FRONT)
        {
            if (gc->state.polygon.frontFace == GL_CCW)
            {
                mode = gcvCULL_CW;
            }
            else
            {
                mode = gcvCULL_CCW;
            }
        }
        else if (gc->state.polygon.cullFace == GL_BACK)
        {
            if (gc->state.polygon.frontFace == GL_CCW)
            {
                mode = gcvCULL_CCW;
            }
            else
            {
                mode = gcvCULL_CW;
            }
        }
        else
        {
            mode = gcvCULL_NONE;
        }
    }
    else
    {
        mode = gcvCULL_NONE;
    }

    result = glmTRANSLATEHALSTATUS(gco3D_SetCulling(chipCtx->hw, mode));
    chipCtx->hashKey.hashClockwiseFront = (gc->state.polygon.frontFace == GL_CCW);


    gcmFOOTER_ARG("return=0x%04x", result);

    return result;
}

