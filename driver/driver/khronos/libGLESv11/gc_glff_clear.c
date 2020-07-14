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


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE    gcdZONE_ES11_CLEAR

/*******************************************************************************
**
**  glClear
**
**  glClear sets the bitplane area of the window to values previously selected
**  by glClearColor, glClearDepth, and glClearStencil.
**
**  The pixel ownership test, the scissor test, dithering, and the color buffer
**  masks affect the operation of glClear. The scissor box bounds the cleared
**  region. Alpha function, blend function, logical operation, stenciling,
**  texture mapping, and depth-buffering are ignored by glClear.
**
**  glClear takes a single argument that is the bitwise OR of several values
**  indicating which buffer is to be cleared.
**
**  The values are as follows:
**      GL_COLOR_BUFFER_BIT   - Indicates the color buffer.
**      GL_DEPTH_BUFFER_BIT   - Indicates the depth buffer.
**      GL_STENCIL_BUFFER_BIT - Indicates the stencil buffer.
**
**  The value to which each buffer is cleared depends on the setting of the
**  clear value for that buffer.
**
**  INPUT:
**
**      Mask
**          Bitwise OR of masks that indicate the buffers to be cleared.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glClear(
    GLbitfield Mask
    )
{
    glmENTER1(glmARGHEX, Mask)
    {
        gceSTATUS status    = gcvSTATUS_OK;

        gcmDUMP_API("${ES11 glClear 0x%08X}", Mask);

        if(Mask & (~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)))
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }
        glmPROFILE(context, GLES1_CLEAR, 0);

        /* Update frame buffer. */
        gcmERR_BREAK(glfUpdateFrameBuffer(context));

#if gcdENABLE_BLIT_BUFFER_PRESERVE
        /* Software (color) buffer preserve. */
        if ((context->frameBuffer == gcvNULL) &&
            (context->prevDraw != gcvNULL) &&
            (Mask & GL_COLOR_BUFFER_BIT))
        {
            if (!gcoSURF_QueryFlags(context->draw, gcvSURF_FLAG_CONTENT_UPDATED) &&
                gcoSURF_QueryFlags(context->draw, gcvSURF_FLAG_CONTENT_PRESERVED))
            {
                /* Need preserve when clear is the first operation. */
                if ((context->colorMask[0]) &&
                    (context->colorMask[1]) &&
                    (context->colorMask[2]) &&
                    (context->colorMask[3]))
                {
                    if (context->viewportStates.scissorTest)
                    {
                        gcsRECT clearRect = {
                            context->viewportStates.scissorBox[0],
                            context->viewportStates.scissorBox[1],
                            context->viewportStates.scissorBox[2],
                            context->viewportStates.scissorBox[3]
                        };

                        /* Partial clear full channel, need preserve parts. */
                        gcmVERIFY_OK(gcoSURF_Preserve(
                            context->prevDraw,
                            context->draw,
                            &clearRect
                            ));
                    }
                }
                else
                {
                    /* Need preserve all pixels when not full channel clear. */
                    gcmVERIFY_OK(gcoSURF_Preserve(
                        context->prevDraw,
                        context->draw,
                        gcvNULL
                        ));
                }

                /* Finish preserve. */
                gcmVERIFY_OK(gcoSURF_SetFlags(
                    context->draw,
                    gcvSURF_FLAG_CONTENT_PRESERVED,
                    gcvFALSE
                    ));
            }
        }

        if ((context->frameBuffer == gcvNULL) &&
            (context->draw != gcvNULL))
        {
            /* Set content updated flag. */
            gcmVERIFY_OK(gcoSURF_SetFlags(
                context->draw,
                gcvSURF_FLAG_CONTENT_UPDATED,
                gcvTRUE
                ));
        }
#endif

        do
        {
            if ((Mask & GL_COLOR_BUFFER_BIT) != 0)
            {
                gcsSURF_VIEW drawView = {gcvNULL, 0, 1};

                /* Determine the target surface. */
                if (context->frameBuffer == gcvNULL)
                {
                    drawView.surf = context->draw;
                }
                else
                {
                    if (glfIsFramebufferComplete(context) != GL_FRAMEBUFFER_COMPLETE_OES)
                    {
                        glmERROR(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
                        break;
                    }

                    drawView.surf = glfGetFramebufferSurface(&context->frameBuffer->color);
                }

                /* Do we have a surface to draw on? */
                if (drawView.surf)
                {
                    gcsSURF_CLEAR_ARGS clearArgs;
                    gcsRECT  rect;

                    gcoOS_ZeroMemory(&clearArgs, sizeof(clearArgs));
                    clearArgs.clearRect = gcvNULL;
                    clearArgs.color.r.floatValue = context->clearColor.value[0];
                    clearArgs.color.g.floatValue = context->clearColor.value[1];
                    clearArgs.color.b.floatValue = context->clearColor.value[2];
                    clearArgs.color.a.floatValue = context->clearColor.value[3];
                    clearArgs.color.valueType = gcvVALUE_FLOAT;
                    clearArgs.colorMask =   (gctUINT8)context->colorMask[0]       |
                                           ((gctUINT8)context->colorMask[1] << 1) |
                                           ((gctUINT8)context->colorMask[2] << 2) |
                                           ((gctUINT8)context->colorMask[3] << 3);

                    if ((!gcoSURF_QueryFlags(drawView.surf, gcvSURF_FLAG_CONTENT_PRESERVED)) &&
                        (!gcoSURF_QueryFlags(drawView.surf, gcvSURF_FLAG_CONTENT_UPDATED)))
                    {
                        clearArgs.colorMask = clearArgs.colorMask? 0xF : 0x0;
                    }

                    clearArgs.flags = gcvCLEAR_COLOR;

                    /* Clear color buffer. */
                    if (context->viewportStates.scissorTest)
                    {
                        /* Determine scissor coordinates. */
                        rect.left
                            = context->viewportStates.scissorBox[0];

                        rect.top
                            = context->viewportStates.scissorBox[1];

                        rect.right
                            = rect.left
                            + context->viewportStates.scissorBox[2];

                        rect.bottom
                            = rect.top
                            + context->viewportStates.scissorBox[3];

                        if (context->drawYInverted)
                        {
                            gctINT32 temp;
                            temp = rect.top;
                            rect.top = context->drawHeight - rect.bottom;
                            rect.bottom = context->drawHeight - temp;
                        }

                        clearArgs.clearRect = &rect;
                    }

                    /* Clear the scissor area. */
                    gcmERR_BREAK(gcoSURF_Clear(&drawView, &clearArgs));
                }

                if (drawView.surf != gcvNULL)
                {
                    /*. Set Drawable update flag.*/
                    gcoSURF_SetFlags(drawView.surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }

            if ((Mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0)
            {
                gctUINT flags = 0;
                gcsSURF_VIEW dsView = {gcvNULL, 0, 1};

                /* Compute masks for clearing. */
                if (Mask & GL_DEPTH_BUFFER_BIT)
                {
                    flags |= gcvCLEAR_DEPTH;
                }

                if (Mask & GL_STENCIL_BUFFER_BIT)
                {
                    flags |= gcvCLEAR_STENCIL;
                }

                /* Update stencil states. */
                gcmERR_BREAK(glfUpdateStencil(context));

                /* Determine the target surface. */
                if (context->frameBuffer == gcvNULL)
                {
                    dsView.surf = context->depth;
                }
                else
                {
                    if (glfIsFramebufferComplete(context) != GL_FRAMEBUFFER_COMPLETE_OES)
                    {
                        glmERROR(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
                        break;
                    }

                    dsView.surf = glfGetFramebufferSurface(&context->frameBuffer->depth);
                }

                /* Do we have a surface to draw on? */
                if (dsView.surf != gcvNULL)
                {
                    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
                    gcsSURF_CLEAR_ARGS clearArgs;
                    gcsRECT  rect;

                    gcmVERIFY_OK(gcoSURF_GetFormat(dsView.surf, gcvNULL, &format));

                    gcoOS_ZeroMemory(&clearArgs, sizeof(clearArgs));
                    clearArgs.clearRect = gcvNULL;
                    clearArgs.depth.floatValue = context->depthStates.clearValue;
                    clearArgs.depthMask = context->depthStates.depthMask;
                    clearArgs.stencil = context->stencilStates.clearValue;
                    clearArgs.stencilMask = (context->stencilStates.writeMask & 0xFF);

                    if ((flags & gcvCLEAR_STENCIL) &&
                        (!gcoSURF_QueryFlags(dsView.surf, gcvSURF_FLAG_CONTENT_PRESERVED)) &&
                        (!gcoSURF_QueryFlags(dsView.surf, gcvSURF_FLAG_CONTENT_UPDATED)))
                    {
                        clearArgs.stencilMask = clearArgs.stencilMask? 0xFF : 0x0;
                    }

                    clearArgs.flags = flags;

                    /* Clear depth buffer. */
                    if (context->viewportStates.scissorTest)
                    {
                        /* Determine scissor coordinates. */
                        rect.left
                            = context->viewportStates.scissorBox[0];

                        rect.top
                            = context->viewportStates.scissorBox[1];

                        rect.right
                            = rect.left
                            + context->viewportStates.scissorBox[2];

                        rect.bottom
                            = rect.top
                            + context->viewportStates.scissorBox[3];

                        if (context->drawYInverted)
                        {
                            gctINT32 temp;
                            temp = rect.top;
                            rect.top = context->drawHeight - rect.bottom;
                            rect.bottom = context->drawHeight - temp;
                        }

                        clearArgs.clearRect = &rect;
                    }

                    /* Clear the scissor area. */
                    gcmERR_BREAK(gcoSURF_Clear(&dsView , &clearArgs));
                }

                if (dsView.surf != gcvNULL)
                {
                    /*. Set Drawable update flag.*/
                    gcoSURF_SetFlags(dsView.surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
                }
            }
        }
        while (gcvFALSE);

        /* Error? */
        if (gcmIS_ERROR(status))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }
    }
    glmLEAVE();
}
