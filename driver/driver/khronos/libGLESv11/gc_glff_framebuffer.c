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


/*******************************************************************************
**  FrameBuffer management for OpenGL ES 1.1 driver.
**
**  Each attachment of a framebuffer object consists of a main owner, either a
**  renderbuffer or a texture.  For both of these owners, we also set aside a
**  render target if the following conditions are met:
**
**  - The IP supports tile status.
**  - The render buffer or texture is nicely aligned to a resolvable size.
*
**  The render target is then used to render into, minimizing bandwidth usage
**  and even allowing for MSAA.  This render target needs to be resolved into
**  the storage for the renderbuffer or texture when te frame buffer changes
**  binding.
**
**  If a render target is attched - special care must be taken if a texture or
**  renderbuffer has been changed while not bound to the active framebuffer.
**  Because we have a special render target, the renderbuffer or texture needs
**  to be copied into the render target if it is dirty.  Luckely, this is easy
**  to manage by using dirty bits.
*/

#include "gc_glff_precomp.h"

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
#if ANDROID_SDK_VERSION >= 16
#      include <ui/ANativeObjectBase.h>
#   else
#      include <private/ui/android_natives_priv.h>
#   endif

#   include <gc_gralloc_priv.h>
#endif

#define _GC_OBJ_ZONE glvZONE_TRACE

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  _DeleteFrameBuffer
**
**  Buffer destructor.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      Object
**          Pointer to the object to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/

static gceSTATUS _DeleteFrameBuffer(
    IN glsCONTEXT_PTR Context,
    IN gctPOINTER Object
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Context=0x%x Object=0x%x", Context, Object);

    do
    {
        /* Cast the object. */
        glsFRAME_BUFFER_PTR object = (glsFRAME_BUFFER_PTR)(((glsNAMEDOBJECT_PTR)Object)->object);

        /* Destroy any render target created for the color attachment. */
        if (object->color.target != gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_Destroy(object->color.target));
            object->color.target = gcvNULL;
        }

        /* Dereference unbind surface. */
        if (object->color.surface != gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_Destroy(object->color.surface));
            object->color.surface= gcvNULL;
        }

        /* Destroy any render target created for the depth attachment. */
        if (object->depth.target != gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_Destroy(object->depth.target));
            object->depth.target = gcvNULL;
        }

        /* Dereference unbind surface. */
        if (object->depth.surface != gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_Destroy(object->depth.surface));
            object->depth.surface = gcvNULL;
        }

        /* Stencil cannot have any attachment. */
        gcmASSERT(object->stencil.target == gcvNULL);
    }
    while (gcvFALSE);

    gcmFOOTER();

    return status;
}


/*******************************************************************************
**
**  _CreateFrameBuffer
**
**  Render buffer constructor.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**      RenderBuffer
**          Name of the render buffer to create.
**
**  OUTPUT:
**
**      Wrapper
**          Points to the created named object.
*/

static gceSTATUS _CreateFrameBuffer(
    IN glsCONTEXT_PTR Context,
    IN gctUINT32 FrameBuffer,
    OUT glsNAMEDOBJECT_PTR * Wrapper
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x FrameBuffer=%d Wrapper=0x%x", Context, FrameBuffer, Wrapper);

    do
    {
        glsFRAME_BUFFER_PTR object;
        glsCONTEXT_PTR shared;

        /* Map shared context. */
        shared = Context;

        /* Attempt to allocate a new buffer. */
        gcmERR_BREAK(glfCreateNamedObject(
            Context,
            shared->frameBufferList,
            FrameBuffer,
            _DeleteFrameBuffer,
            Wrapper
            ));

        /* Get a pointer to the new buffer. */
        object = (glsFRAME_BUFFER_PTR) (*Wrapper)->object;

        /* Reset the buffer. */
        gcoOS_ZeroMemory(object, sizeof(glsFRAME_BUFFER));

        /* Invalidate. */
        object->dirty = GL_TRUE;
    }
    while (gcvFALSE);

    gcmFOOTER();

    /* Return result. */
    return status;
}


/*******************************************************************************
**
**  _MergeDepthAndStencil
**
**  Merge depth and stencil buffers.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

static void _MergeDepthAndStencil(
    IN glsCONTEXT_PTR Context
    )
{
    glsNAMEDOBJECT_PTR depthWrapper, stencilWrapper;
    glsRENDER_BUFFER_PTR depth, stencil;
    gctINT32 depthReferenceCount = 0;

    gcmHEADER_ARG("Context=0x%x", Context);

    depthWrapper   = (glsNAMEDOBJECT_PTR) Context->frameBuffer->depth.object;
    stencilWrapper = (glsNAMEDOBJECT_PTR) Context->frameBuffer->stencil.object;

    if ((depthWrapper == gcvNULL) || (stencilWrapper == gcvNULL))
    {
        gcmFOOTER_NO();
        return;
    }

    depth   = (glsRENDER_BUFFER_PTR) depthWrapper->object;
    stencil = (glsRENDER_BUFFER_PTR) stencilWrapper->object;

    /* Need both depth and stencil to combine. */
    if ((depth == gcvNULL) || (stencil == gcvNULL))
    {
        gcmFOOTER_NO();
        return;
    }

    /* GL_OES_packed_depth_stencil. */
    if (depth == stencil)
    {
        gcmFOOTER_NO();
        return;
    }

    /* Both need to be of renderbuffer type. */
    if (Context->frameBuffer->depth.texture ||
        Context->frameBuffer->stencil.texture)
    {
        gcmFOOTER_NO();
        return;
    }

    /* Already bound together? */
    if (depth->bound   && (depth->combined   == stencil) &&
        stencil->bound && (stencil->combined == depth))
    {
        gcmFOOTER_NO();
        return;
    }

    /* Sizes have to match. */
    if ((depth->width  != stencil->width) ||
        (depth->height != stencil->height))
    {
        Context->frameBuffer->dirty        = GL_FALSE;
        Context->frameBuffer->completeness =
            GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES;

        gcmFOOTER_NO();
        return;
    }

    /* Depth bound to something else other then stencil? */
    if (depth->bound &&
        (depth->combined != gcvNULL) &&
        (depth->combined != stencil))
    {
        Context->frameBuffer->dirty        = GL_FALSE;
        Context->frameBuffer->completeness = GL_FRAMEBUFFER_UNSUPPORTED_OES;

        gcmFOOTER_NO();
        return;
    }

    /* Stencil bound to something else other then depth? */
    if (stencil->bound &&
        (stencil->combined != gcvNULL) &&
        (stencil->combined != depth))
    {
        Context->frameBuffer->dirty        = GL_FALSE;
        Context->frameBuffer->completeness = GL_FRAMEBUFFER_UNSUPPORTED_OES;

        gcmFOOTER_NO();
        return;
    }

    gcoSURF_QueryReferenceCount(depth->surface, &depthReferenceCount);
    while (depthReferenceCount--)
    {
        gcoSURF_ReferenceSurface(stencil->surface);
    }

    /* Delete depth surface. */
    gcoSURF_Destroy(depth->surface);

    /* Combine depth and stencil renderbuffers. */
    depth->surface                      = stencil->surface;
    Context->frameBuffer->depth.surface = stencil->surface;
    depth->combined                     = stencil;
    stencil->combined                   = depth;
    gcmFOOTER_NO();
    return;
}

GLboolean glfQueryFramebufferState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Name=0x%04x Value=0x%x Type=0x%04x",
                    Context, Name, Value, Type);
    result = GL_TRUE;
    switch (Name)
    {
    case GL_FRAMEBUFFER_BINDING_OES:
        glfGetFromInt(
            Context->curFrameBufferID,
            Value,
            Type
            );
        break;

    default:
        result = GL_FALSE;
    }
    gcmFOOTER_ARG("result=%d", result);

    /* Return result. */
    return result;
}
/*******************************************************************************
**
**  glfIsFramebufferComplete
**
**  Check whether the currently bound framebuffer is complete or not.
**
**  Arguments:
**
**      Context
**          Pointer to the current context.
**
**  Returns:
**
**      GLenum
**          The completess of the frame buffer.  Can be one of the following:
**
**              GL_FRAMEBUFFER_COMPLETE_OES
**                  The framebuffer is complete.
**              GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES
**                  One of the attachments to the framebuffer is incomplete.
**              GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES
**                  There are no attachments to the framebuffer.
**              GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES
**                  The attachments have different dimensions.
**              GL_FRAMEBUFFER_UNSUPPORTED_OES
**                  The attachments cannot be used together.
*/

GLenum glfIsFramebufferComplete(
    IN glsCONTEXT_PTR Context
    )
{
    gceSURF_FORMAT format[3];
    gcsSURF_FORMAT_INFO_PTR info[2];
    gceSTATUS status;
    GLint count = 0, i;
    gctUINT width[3], height[3];

    gcmHEADER_ARG("Context=0x%x", Context);

    /* We are complete if there is no bound framebuffer. */
    if (Context->frameBuffer == gcvNULL)
    {
        gcmFOOTER_ARG("return=%d", GL_FRAMEBUFFER_COMPLETE_OES);
        return GL_FRAMEBUFFER_COMPLETE_OES;
    }

    do
    {
        if (!Context->frameBuffer->dirty)
        {
            /* The framebuffer is not dirty, so we already have a valid
               completeness. */
            break;
        }

        /* Test for incomplete attachment. */
        Context->frameBuffer->completeness =
            GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES;

        /* Test color attachment for completeness. */
        if (Context->frameBuffer->color.object != gcvNULL)
        {
            /* Test for storage. */
            if (Context->frameBuffer->color.surface == gcvNULL)
            {
                break;
            }

            /* Get the format of color attachment. */
            gcmERR_BREAK(gcoSURF_GetFormat(
                Context->frameBuffer->color.surface,gcvNULL, &format[count]
                ));

            gcmERR_BREAK(gcoSURF_QueryFormat(
                format[count], info
                ));

            /* Only (A)RGB surfaces are supported for color attachments. */
            if (info[0]->fmtClass != gcvFORMAT_CLASS_RGBA)
            {
                break;
            }

            /* Get the dimension of the color attachment. */
            gcmERR_BREAK(gcoSURF_GetSize(
                Context->frameBuffer->color.surface,
                &width[count], &height[count], gcvNULL
                ));

            count += 1;
        }

        /* Test depth attachment for completeness. */
        if (Context->frameBuffer->depth.object != gcvNULL)
        {
            /* Test for storage. */
            if (Context->frameBuffer->depth.surface == gcvNULL)
            {
                break;
            }

            /* Get the format of depth attachment. */
            gcmERR_BREAK(gcoSURF_GetFormat(
                Context->frameBuffer->depth.surface, gcvNULL,  &format[count]
                ));

            gcmERR_BREAK(gcoSURF_QueryFormat(
                format[count], info
                ));

            /* Only (S)D surfaces are supported for depth attachments. */
            if ((info[0]->fmtClass != gcvFORMAT_CLASS_DEPTH) ||
                (info[0]->u.depth.depth.width == 0))
            {
                break;
            }

            /* Get the dimension of the depth attachment. */
            gcmERR_BREAK(gcoSURF_GetSize(
                Context->frameBuffer->depth.surface,
                &width[count], &height[count], gcvNULL
                ));

            count += 1;
        }

        /* Test stencil attachment for completeness. */
        if (Context->frameBuffer->stencil.object != gcvNULL)
        {
            /* Test for storage. */
            if (Context->frameBuffer->stencil.surface == gcvNULL)
            {
                break;
            }

            /* Get the format of stencil attachment. */
            gcmERR_BREAK(gcoSURF_GetFormat(
                Context->frameBuffer->stencil.surface, gcvNULL, &format[count]
                ));

            gcmERR_BREAK(gcoSURF_QueryFormat(
                format[count], info
                ));

            /* Only (S)D surfaces are supported for depth attachments. */
            if ((info[0]->fmtClass != gcvFORMAT_CLASS_DEPTH) ||
                (info[0]->u.depth.stencil.width == 0))
            {
                break;
            }

            /* Get the dimension of the stencil attachment. */
            gcmERR_BREAK(gcoSURF_GetSize(
                Context->frameBuffer->stencil.surface,
                &width[count], &height[count], gcvNULL
                ));

            count += 1;
        }

        /* Test for missing attachment. */
        if (count == 0)
        {
            Context->frameBuffer->completeness =
                GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES;

            break;
        }

        /* Test for incorrect dimensions between attachments. */
        for (i = count - 1; i > 0; --i)
        {
            if ((width[0] != width[i]) || (height[0] != height[i]))
            {
                Context->frameBuffer->completeness =
                    GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES;

                break;
            }
        }

        if (i > 0)
        {
            break;
        }

        /* Test for unsupported depth & stencil combination. */
        if ((Context->frameBuffer->depth.object   != gcvNULL) &&
            (Context->frameBuffer->stencil.object != gcvNULL))
        {
            GLint depth = (count == 2) ? 0 : 1;
            gcmASSERT(depth + 1 < count);

            if (format[depth] != format[depth + 1])
            {
                Context->frameBuffer->completeness =
                    GL_FRAMEBUFFER_UNSUPPORTED_OES;

                break;
            }
        }

        /* FrameBuffer is complete. */
        Context->frameBuffer->completeness = GL_FRAMEBUFFER_COMPLETE_OES;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=0x%04x", Context->frameBuffer->completeness);
    /* Return the completeness of the framebuffer. */
    return Context->frameBuffer->completeness;
}


/*******************************************************************************
**
**  glfGetFramebufferSurface
**
**  Check whether the currently bound framebuffer is complete or not.
**
**  Arguments:
**
**      Attachment
**          Pointer to the frame buffer attachment.
**
**  Returns:
**
**      gcoSURF
**          Pointer to the associated surface object.
*/

gcoSURF glfGetFramebufferSurface(
    glsFRAME_BUFFER_ATTACHMENT_PTR Attachment
    )
{
    gcmHEADER_ARG("Attachment=0x%x", Attachment);
    if ((Attachment == gcvNULL) || (Attachment->object == gcvNULL))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    if (Attachment->target != gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%x", Attachment->target);
        return Attachment->target;
    }

    gcmFOOTER_ARG("return=0x%x", Attachment->surface);

    return Attachment->surface;
}

/*******************************************************************************
**
**  glfUpdateFrameBuffer
**
**  Update frame buffer states.
**
**  INPUT:
**
**      Context
**          Current drawing context.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS glfUpdateFrameBuffer(
    IN glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL yInverted = gcvFALSE;
    gcsSURF_VIEW drawView = {gcvNULL, 0, 1};
    gcsSURF_VIEW dsView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("Context=0x%x", Context);

    /* Detect depth only mode */
    gcmONERROR(glfDetectDepthOnly(Context));

    /* Test framebuffer object. */
    if (Context->frameBufferChanged)
    {
        /* Flush the frame buffer. */
        gcmONERROR(gcoSURF_Flush(Context->draw));

        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(Context->hal, gcvFALSE));

        Context->frameBufferChanged = GL_FALSE;

        if (Context->frameBuffer == gcvNULL)
        {
            if(Context->draw)
            {
                yInverted = (gcoSURF_QueryFlags(Context->draw, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            }
            else if(Context->depth)
            {
                yInverted = (gcoSURF_QueryFlags(Context->depth, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            }

            drawView.surf = Context->draw;
            dsView.surf = Context->depth;

            gcmONERROR(gco3D_SetTarget(Context->hw, 0, &drawView, 0));
            gcmONERROR(gco3D_SetDepth(Context->hw, &dsView));

            gcmONERROR(gcoSURF_GetSamples(Context->draw, &Context->drawSamples));

            Context->effectiveWidth  = Context->drawWidth;
            Context->effectiveHeight = Context->drawHeight;

            gcmONERROR(gco3D_SetDepthOnly(Context->hw, gcvFALSE));
        }
        else
        {
            if (glfIsFramebufferComplete(Context) != GL_FRAMEBUFFER_COMPLETE_OES)
            {
                glmERROR(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            Context->frameBuffer->dirty = GL_FALSE;

            /* Get color surface. */
            drawView.surf = glfGetFramebufferSurface(&Context->frameBuffer->color);

            /* Get depth surface. */
            dsView.surf = glfGetFramebufferSurface(&Context->frameBuffer->depth);

            /* Set the render target. */
            gcmONERROR(gco3D_SetTarget(Context->hw, 0, &drawView, 0));

            /* Set the depth buffer. */
            gcmONERROR(gco3D_SetDepth(Context->hw, &dsView));

            if (drawView.surf != gcvNULL)
            {
                gcmONERROR(gcoSURF_GetSize(
                    drawView.surf,
                    &Context->effectiveWidth, &Context->effectiveHeight,
                    gcvNULL
                    ));

                gcmONERROR(gcoSURF_GetSamples(
                    drawView.surf, &Context->drawSamples
                    ));

                gcmONERROR(gco3D_SetDepthOnly(
                    Context->hw, gcvFALSE
                    ));

                if (Context->frameBuffer->color.texture)
                {
                    ((glsTEXTUREWRAPPER_PTR) Context->frameBuffer->color.object)->dirty = gcvTRUE;
                }
            }
            else
            {
                gcmONERROR(gcoSURF_GetSize(
                    dsView.surf, &Context->effectiveWidth, &Context->effectiveHeight,  gcvNULL
                    ));

                gcmONERROR(gco3D_SetDepthOnly(
                    Context->hw, gcvTRUE
                    ));
            }

            if (dsView.surf != gcvNULL)
            {
                gcmONERROR(gcoSURF_GetSamples(
                    dsView.surf, &Context->drawSamples
                    ));

                if (Context->frameBuffer->depth.texture)
                {
                    ((glsTEXTUREWRAPPER_PTR) Context->frameBuffer->depth.object)->dirty = GL_TRUE;
                }
            }

            Context->frameBuffer->needResolve = GL_TRUE;
        }

        gco3D_SetColorCacheMode(Context->hw);

        /* Invalidate the clipping box. */
        Context->viewportStates.recomputeClipping = GL_TRUE;

        if(Context->drawYInverted != yInverted)
        {
            Context->drawYInverted = yInverted;
            Context->fsUniformDirty.uPointSpriteCoordDirty = GL_TRUE;
            Context->recomputeViewport = GL_TRUE;
            Context->reprogramCulling = GL_TRUE;
        }
    }

    /* Easy return on error. */
OnError:

    gcmFOOTER_ARG("status=%d", status);
    /* Return status. */
    return status;
}


/******************************************************************************\
**************************** Buffer Management Code ****************************
\******************************************************************************/

/*******************************************************************************
**
**  glGenFramebuffersOES (OES_framebuffer_object)
**
**  Generate a number of framebuffer objects.
**
**  Arguments:
**
**      GLsizei Count
**          Number of framebuffers to generate.
**
**      GLuint * FrameBuffers
**          Pointer to an array of GLuint values that will receive the names of
**          the generated buffers.
**
*/
#ifdef _GC_OBJ_ZONE
#undef _GC_OBJ_ZONE
#endif
#define _GC_OBJ_ZONE    glvZONE_BUFFER

GL_API void GL_APIENTRY glGenFramebuffersOES(
    GLsizei Count,
    GLuint * FrameBuffers
    )
{
    glmENTER2(glmARGINT, Count, glmARGPTR, FrameBuffers)
    {
        GLsizei i;
        glsNAMEDOBJECT_PTR wrapper;

        /* Validate count. */
        if (Count < 0)
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Don't do anything if FrameBuffers is NULL. */
        if (FrameBuffers == gcvNULL)
        {
            break;
        }

        /* Generate buffers. */
        for (i = 0; i < Count; i++)
        {
            /* Create a new wrapper. */
            if (gcmIS_SUCCESS(_CreateFrameBuffer(context, 0, &wrapper)))
            {
                FrameBuffers[i] = wrapper->name;
            }
            else
            {
                FrameBuffers[i] = 0;
            }
        }

        gcmDUMP_API("${ES11 glGenFramebuffersOES 0x%08X (0x%08X)", Count, FrameBuffers);
        gcmDUMP_API_ARRAY(FrameBuffers, Count);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glDeleteFramebuffersOES (OES_framebuffer_object)
**
**  Delete a specified number of framebuffers.
**
**  Arguments:
**
**      GLsizei Count
**          Number of framebuffers to delete.
**
**      const GLUint * FrameBuffers
**          Pointer to the list of framebuffers to delete.
**
*/

GL_API void GL_APIENTRY glDeleteFramebuffersOES(
    GLsizei Count,
    const GLuint * FrameBuffers
    )
{
    glmENTER2(glmARGINT, Count, glmARGPTR, FrameBuffers)
    {
        GLsizei i;
        glsCONTEXT_PTR shared;

        /* Map shared context. */
        shared = context;

        /* Validate count. */
        if (Count < 0)
        {
            glmERROR(GL_INVALID_VALUE);
            break;
        }

        /* Don't do anything if Buffers is NULL. */
        if (FrameBuffers == gcvNULL)
        {
            break;
        }

        gcmDUMP_API("${ES11 glDeleteFramebuffersOES 0x%08X (0x%08X)", Count, FrameBuffers);
        gcmDUMP_API_ARRAY(FrameBuffers, Count);
        gcmDUMP_API("$}");

        /* Iterate through the buffer names. */
        for (i = 0; i < Count; i++)
        {
            glsNAMEDOBJECT_PTR wrapper = glfFindNamedObject(shared->frameBufferList, FrameBuffers[i]);

            if (gcvNULL == wrapper)
            {
                continue;
            }

            /* Unbound from current */
            if (context->frameBuffer == (glsFRAME_BUFFER_PTR)wrapper->object)
            {
                context->frameBuffer         = gcvNULL;
                context->curFrameBufferID    = 0;
                context->frameBufferChanged  = gcvTRUE;
                context->stencilStates.dirty = gcvTRUE;
            }

            gcmVERIFY_OK(glfDeleteNamedObject(
                context, shared->frameBufferList, FrameBuffers[i]
                ));
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glIsFramebufferOES (OES_framebuffer_object)
**
**  Check whether the specified framebuffer is a framebuffer object or not.
**
**  Arguments:
**
**      GLuint FrameBuffer
**          FrameBuffer name to test.
**
**  Returns:
**
**      GLboolean
**          GL_TRUE if the specified framebuffer is indeed a framebuffer object.
**          GL_FALSE otherwise.
**
*/

GL_API GLboolean GL_APIENTRY glIsFramebufferOES(
    GLuint FrameBuffer
    )
{
    GLboolean result = GL_FALSE;

    glmENTER1(glmARGUINT, FrameBuffer)
    {
        glsCONTEXT_PTR shared;

        /* Map shared context. */
        shared = context;

        result = glfFindNamedObject(shared->frameBufferList, FrameBuffer)
               ? GL_TRUE
               : GL_FALSE;

        gcmDUMP_API("${ES11 glIsFramebufferOES 0x%08X := 0x%08X}", FrameBuffer, result);
    }
    glmLEAVE();

    /* Return result. */
    return result;
}


/*******************************************************************************
**
**  glBindFramebufferOES (OES_framebuffer_object)
**
**  Bind a framebuffer to the specified target or unbind the current framebuffer
**  from the specified target.
**
**  Arguments:
**
**      GLenum Target
**          Target to bind the buffer to.  This must be GL_FRAMEBUFFER_OES.
**
**      GLuint FrameBuffer
**          FrameBuffer to bind or 0 to unbind the current framebuffer.
**
*/

GL_API void GL_APIENTRY glBindFramebufferOES(
    GLenum Target,
    GLuint FrameBuffer
    )
{
    glmENTER2(glmARGENUM, Target, glmARGUINT, FrameBuffer)
    {
        gceSTATUS status;
        glsFRAME_BUFFER_PTR object;
        glsNAMEDOBJECT_PTR wrapper;
        glsCONTEXT_PTR shared;
        gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
        gcsSURF_VIEW tgtView  = {gcvNULL, 0, 1};
        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};

        gcmDUMP_API("${ES11 glBindFramebufferOES 0x%08X 0x%08X}", Target, FrameBuffer);

        /* Map shared context. */
        shared = context;

        /* Verify the target. */
        if (Target != GL_FRAMEBUFFER_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Some application will try to unbind current buffer by binding */
        /* a invalid buffer name. We need special check here.            */
        if (FrameBuffer > 0x1000)
        {
            /* Find the object. */
            wrapper = glfFindNamedObject(shared->frameBufferList, FrameBuffer);

            /* No such object yet? Force frame buffer to 0. */
            if (wrapper == gcvNULL)
            {
                FrameBuffer = 0;
            }
        }

        /* Remove current binding? */
        if (FrameBuffer == 0)
        {
            object = gcvNULL;
        }
        else
        {
            /* Find the object. */
            wrapper = glfFindNamedObject(shared->frameBufferList, FrameBuffer);

            /* No such object yet? */
            if (wrapper == gcvNULL)
            {
                /* Create a new RenderBuffer. */
                if (gcmIS_ERROR(_CreateFrameBuffer(context, FrameBuffer, &wrapper)))
                {
                    glmERROR(GL_OUT_OF_MEMORY);
                    break;
                }
            }

            /* Get the object. */
            object = (glsFRAME_BUFFER_PTR) wrapper->object;
        }

        /* Already bound? */
        if (context->frameBuffer == object)
        {
            break;
        }

        /* Unbind any current framebuffer. */
        if (context->frameBuffer != gcvNULL)
        {
            if(context->frameBuffer->color.surface != gcvNULL)
            {
                gcmERR_BREAK(gcoSURF_SetOrientation(
                    context->frameBuffer->color.surface,
                    gcvORIENTATION_TOP_BOTTOM
                ));
            }

            if ((context->frameBuffer->color.target != gcvNULL) &&
                context->frameBuffer->needResolve &&
                (context->frameBuffer->color.surface != gcvNULL))
            {
                /* Set orientation. */
                gcmERR_BREAK(gcoSURF_SetOrientation(
                    context->frameBuffer->color.target,
                    gcvORIENTATION_TOP_BOTTOM
                    ));

                /* Resolve color render target into texture. */
                surfView.surf = context->frameBuffer->color.surface;
                tgtView.surf  = context->frameBuffer->color.target;
                gcmERR_BREAK(gcoSURF_ResolveRect(&tgtView, &surfView, gcvNULL));
            }

            if(context->frameBuffer->depth.surface != gcvNULL)
            {
                gcmERR_BREAK(gcoSURF_SetOrientation(
                    context->frameBuffer->depth.surface,
                    gcvORIENTATION_TOP_BOTTOM
                    ));
            }


            if ((context->frameBuffer->depth.target != gcvNULL) &&
                context->frameBuffer->needResolve)
            {
                /* Set orientation. */
                gcmERR_BREAK(gcoSURF_SetOrientation(
                    context->frameBuffer->depth.target,
                    gcvORIENTATION_TOP_BOTTOM
                    ));

                /* Resolve depth render target into texture. */
                surfView.surf = context->frameBuffer->depth.surface;
                tgtView.surf  = context->frameBuffer->depth.target;
                gcmERR_BREAK(gcoSURF_ResolveRect(&tgtView, &surfView, gcvNULL));
            }

            /* Mark as resolved. */
            context->frameBuffer->needResolve = gcvFALSE;

            /* Unbind. */
            context->frameBuffer     = gcvNULL;
        }

        /* Check for uploading into the render target. */
        if ((object != gcvNULL) &&
            (object->color.target != gcvNULL) &&
            (object->color.object != gcvNULL) &&
             object->color.texture)
        {
            if (((glsTEXTUREWRAPPER_PTR) object->color.object)->dirty)
            {
                tmpView.surf = object->color.surface;
                gcmERR_BREAK(gcoSURF_DisableTileStatus(
                    &tmpView, gcvTRUE
                    ));

                surfView.surf = object->color.surface;
                tgtView.surf  = object->color.target;
                gcmERR_BREAK(gcoSURF_ResolveRect(&surfView, &tgtView, gcvNULL));
            }
        }

        /* Check for uploading into the depth buffer. */
        if ((object != gcvNULL) &&
            (object->depth.target != gcvNULL) &&
            (object->depth.object != gcvNULL) &&
             object->depth.texture)
        {
            if (((glsTEXTUREWRAPPER_PTR) object->depth.object)->dirty)
            {
                tmpView.surf = object->depth.surface;
                gcmERR_BREAK(gcoSURF_DisableTileStatus(
                    &tmpView, gcvTRUE
                    ));

                surfView.surf = object->depth.surface;
                tgtView.surf  = object->depth.target;
                gcmERR_BREAK(gcoSURF_ResolveRect(&surfView, &tgtView, gcvNULL));
            }
        }

        context->frameBuffer         = object;
        context->curFrameBufferID    = FrameBuffer;
        context->frameBufferChanged  = gcvTRUE;
        context->stencilStates.dirty = gcvTRUE;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glFramebufferTexture2DOES (OES_framebuffer_object)
**
**  Attach a texture to the currently bound framebuffer.
**
**  Arguments:
**
**      GLenum Target
**          This must be GL_FRAMEBUFFER_OES.
**
**      GLenum Attachment
**          Attachment to attach.  This can be one of the following:
**
**              GL_COLOR_ATTACHMENT0_OES
**                  Attach a color attachment.
**              GL_DEPTH_ATTACHMENT_OES
**                  Attach a depth attachment.
**              GL_STENCIL_ATTACHMENT_OES
**                  Attach a stencil attachment.
**
**      GLenum Textarget
**          This must be GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP.
**
**      GLuint Texture
**          Name of the texture to attach.
**
**      GLint Level
**          This must be 0.
*/

GL_API void GL_APIENTRY glFramebufferTexture2DOES(
    GLenum Target,
    GLenum Attachment,
    GLenum Textarget,
    GLuint Texture,
    GLint Level
    )
{
    glmENTER5(glmARGENUM, Target, glmARGENUM, Attachment, glmARGENUM, Textarget,
              glmARGHEX, Texture, glmARGINT, Level)
    {
        gceSTATUS status;
        glsTEXTUREWRAPPER_PTR texture;
        gcoSURF surface;
        gcoSURF target;
        gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
        gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
        gctUINT width, height, depth;
        gctBOOL firstTimeConstruct = gcvFALSE;

        gcmDUMP_API("${ES11 glFramebufferTexture2DOES 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X}",
            Target, Attachment, Textarget, Texture, Level);

        /* Verify the target. */
        if (Target != GL_FRAMEBUFFER_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Make sure there is a framebuffer object bound. */
        if (context->frameBuffer == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Reset the new render taret surfafce. */
        target = gcvNULL;

        /* Remove current attachment? */
        if (Texture == 0)
        {
            texture = gcvNULL;
            surface = gcvNULL;
        }
        else
        {
            gctBOOL redo = gcvFALSE;

            /* Find the texture object. */
            texture = glfFindTexture(context, Texture);

            /* No such texture? */
            if (texture == gcvNULL)
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            if (Level < 0)
            {
                glmERROR(GL_INVALID_VALUE);
                break;
            }

            if (Textarget != GL_TEXTURE_2D)
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            if (texture->direct.source != gcvNULL)
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            if ((texture->image.source != gcvNULL)
            &&  (Level == 0)
            )
            {
                gcoSURF mipmap = gcvNULL;
                gctBOOL dirty;

                /* Get dirty state. */
                dirty = texture->image.dirty;

                texture->image.dirty = gcvFALSE;

                /* Clear direct sample flag, we're using indirect rednering. */
                texture->image.directSample = gcvFALSE;

                if (texture->object == gcvNULL)
                {

                    /* Dynamically allocate mipmap level 0. */
                    status = gcoTEXTURE_ConstructEx(
                        context->hal,
                        gcvTEXTURE_2D,
                        &texture->object
                        );

                    /* Verify creation. */
                    if (gcmIS_ERROR(status))
                    {
                        glmERROR(GL_OUT_OF_MEMORY);
                        break;
                    }
                }

                /* Get mipmap surface. */
                status = gcoTEXTURE_GetMipMap(
                    texture->object,
                    0,
                    &mipmap
                    );

                if (gcmIS_ERROR(status))
                {
                    /* Allocate mipmap for later uploading. */
                    gctUINT width;
                    gctUINT height;

                    gceSURF_FORMAT srcFormat;

                    /* Clear status. */
                    status = gcvSTATUS_OK;

                    /* Get source format. */
                    if (gcmIS_ERROR(gcoSURF_GetFormat(texture->image.source,
                                                      gcvNULL,
                                                      &srcFormat)))
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /* Get source size. */
                    if (gcmIS_ERROR(gcoSURF_GetSize(texture->image.source,
                                                    &width,
                                                    &height,
                                                    gcvNULL)))
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /* Create mipmap level 0. */
                    status = gcoTEXTURE_AddMipMap(
                        texture->object,
                        0,
                        gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
                        texture->image.textureFormat,
                        width,
                        height,
                        0,
                        gcvFACE_NONE,
                        gcvPOOL_DEFAULT,
                        &mipmap
                        );

                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("%s: add mipmap fail", __FUNCTION__);

                        /* Destroy the texture. */
                        gcmVERIFY_OK(gcoTEXTURE_Destroy(texture->object));
                        texture->object = gcvNULL;

                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /* Set shared lock.*/
                    gcmVERIFY_OK(gcoSURF_SetSharedLock(mipmap,
                        context->texture.textureList->sharedLock));

                    /* Force dirty flag. */
                    dirty = gcvTRUE;
                }

                if (dirty)
                {
                    gceSURF_FORMAT srcFormat;

                    /* Get source format. */
                    if (gcmIS_ERROR(gcoSURF_GetFormat(texture->image.source,
                                                      gcvNULL,
                                                      &srcFormat)))
                    {
                        glmERROR(GL_INVALID_VALUE);
                        break;
                    }

                    /*
                     * Android has following formats, but hw does not support
                     * Need software upload for such formats.
                     */
                    if ((srcFormat == gcvSURF_NV16)
                    ||  (srcFormat == gcvSURF_NV61)
                    ||  (srcFormat == gcvSURF_R4G4B4A4)
                    ||  (srcFormat == gcvSURF_R5G5B5A1)
                    )
                    {
                        gctUINT width;
                        gctUINT height;
                        gctPOINTER memory[3] = {gcvNULL};
                        gctINT stride[3];

                        gcmVERIFY_OK(gcoSURF_GetSize(
                            texture->image.source,
                            &width,
                            &height,
                            gcvNULL
                            ));

                        gcmVERIFY_OK(gcoSURF_GetAlignedSize(
                            texture->image.source,
                            gcvNULL,
                            gcvNULL,
                            stride
                            ));

                        /* Lock source surface for read. */
                        gcmERR_BREAK(gcoSURF_Lock(
                            texture->image.source,
                            gcvNULL,
                            memory
                            ));

                        if ((srcFormat == gcvSURF_NV16)
                        ||  (srcFormat == gcvSURF_NV61)
                        )
                        {
                            /* UV stride should be same as Y stride. */
                            stride[1] = stride[0];

                            /* Upload NV16/NV61 to YUY2 by software. */
                            status = gcoTEXTURE_UploadYUV(
                                texture->object,
                                gcvFACE_NONE,
                                width,
                                height,
                                0,
                                memory,
                                stride,
                                srcFormat
                                );
                        }
                        else
                        {
                            /* Upload by software. */
                            status = gcoTEXTURE_Upload(
                                texture->object,
                                0,
                                gcvFACE_NONE,
                                width,
                                height,
                                0,
                                memory[0],
                                stride[0],
                                srcFormat,
                                gcvSURF_COLOR_SPACE_LINEAR
                                );
                        }

                        /* Unlock. */
                        gcmVERIFY_OK(gcoSURF_Unlock(
                            texture->image.source,
                            memory[0]
                            ));
                    }
                    else
                    {
                        gcsSURF_VIEW imgView = {texture->image.source, 0, 1};
                        gcsSURF_VIEW mipView = {mipmap, 0, 1};

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                        android_native_buffer_t * native;
                        struct private_handle_t * hnd = gcvNULL;
                        struct gc_native_handle_t * handle = gcvNULL;

                        /* Cast to android native buffer. */
                        native = (android_native_buffer_t *) texture->image.nativeBuffer;

                        if (native != gcvNULL)
                        {
                            /* Get private handle. */
                            hnd = (struct private_handle_t *) native->handle;
                            handle = gc_native_handle_get(native->handle);
                        }

                        /* Check composition signal. */
                        if (handle != gcvNULL && handle->hwDoneSignal != 0)
                        {
                            gcmVERIFY_OK(gcoOS_Signal(
                                gcvNULL,
                                (gctSIGNAL) (gctUINTPTR_T) handle->hwDoneSignal,
                                gcvFALSE
                                ));
                        }
#endif

                        /* Use resolve to upload texture. */
                        status = gcoSURF_ResolveRect(&imgView, &mipView, gcvNULL);

                        if (gcmIS_ERROR(status))
                        {
                            gcmFATAL("%s: Failed to upload texture.", __FUNCTION__);
                            break;
                        }

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                        if (handle != gcvNULL && handle->hwDoneSignal != 0)
                        {
                            /* Signal the signal, so CPU apps
                             * can lock again once resolve is done. */
                            gcsHAL_INTERFACE iface;

                            iface.command            = gcvHAL_SIGNAL;
                            iface.u.Signal.signal    = handle->hwDoneSignal;
                            iface.u.Signal.auxSignal = 0;
                            /* Stuff the client's PID. */
                            iface.u.Signal.process   = handle->clientPID;
                            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

                            /* Schedule the event. */
                            gcmVERIFY_OK(gcoHAL_ScheduleEvent(gcvNULL, &iface));
                        }
#endif

                        /* Wait all the pixels done. */
                        gcmVERIFY_OK(gco3D_Semaphore(
                            context->hw,
                            gcvWHERE_RASTER,
                            gcvWHERE_PIXEL,
                            gcvHOW_SEMAPHORE_STALL
                            ));
                    }

                    /* Set dirty flag (for later flush). */
                    texture->dirty = gcvTRUE;
                }
            }

            do
            {
                status = gcoTEXTURE_GetMipMap(texture->object, Level, &surface);

                if (gcmIS_ERROR(status))
                {
                    surface = gcvNULL;
                }

                else if (redo)
                {
                    redo = gcvFALSE;
                }

                else
                {
                    gceSTATUS renderableStatus = gcoTEXTURE_IsRenderable(texture->object, Level);

                    gctBOOL unaligned = (renderableStatus == gcvSTATUS_NOT_ALIGNED ||
                                         renderableStatus == gcvSTATUS_NOT_MULTI_PIPE_ALIGNED);

                    gcmONERROR(gcoSURF_GetSize(surface, &width, &height, &depth));

                    /* We create a new render target for IPs that support tile status,
                       when the textures are not properly aligned, or when multi-sampling
                       is turned on. */
                    if ((  (context->hasTileStatus || (context->drawSamples > 1))
                        && (width > 128) && (height > 128)
                        && (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TEXTURE_TILE_STATUS_READ) != gcvSTATUS_TRUE)
                        )
                    ||  renderableStatus != gcvSTATUS_OK
                    )
                    {
                        gceSURF_TYPE type;
                        gceSURF_FORMAT format;
                        gctUINT width, height, depth;

                        gcmVERIFY_OK(gcoSURF_GetSize(surface, &width, &height, &depth));
                        gcmVERIFY_OK(gcoSURF_GetFormat(surface, &type, &format));
                        gcmVERIFY_OK(gco3D_GetClosestRenderFormat(gcvNULL,format, &format));

                        /* Don't create a render target for packed depth/stencil buffers. */
                        if (format == gcvSURF_D24S8)
                        {
                            target = gcvNULL;
                            break;
                        }

                        switch (format)
                        {
                        case gcvSURF_D16:
                        case gcvSURF_D24X8:
                        case gcvSURF_D32:
                        case gcvSURF_X24S8:
                            type = gcvSURF_DEPTH_TS_DIRTY;
                            break;

                        default:
                            type = unaligned ? gcvSURF_RENDER_TARGET_NO_TILE_STATUS
                                         :gcvSURF_RENDER_TARGET_TS_DIRTY;
                            break;
                        }

                        status = gcoSURF_Construct(context->hal,
                                                   width, height, depth,
                                                   type, format,
                                                   gcvPOOL_DEFAULT, &target);

                        if (gcmIS_SUCCESS(status))
                        {
                            firstTimeConstruct = gcvTRUE;

                            if (type != gcvSURF_DEPTH)
                            {
                                if (gcmIS_ERROR(gcoSURF_SetSamples(target, 0)))
                                {
                                    glmERROR(GL_INVALID_OPERATION);
                                    goto OnError;
                                }
                            }
                        }
                    }

                    if (target == gcvNULL)
                    {
                        /* Render directly into the mipmap. */
                        gcmVERIFY_OK(gcoTEXTURE_RenderIntoMipMap(texture->object, Level));

                        /* Re-get the surface - it might have been changed. */
                        redo = gcvTRUE;
                    }
                }
            }
            while (redo);
        }

        /* Dispatch on attachment. */
        switch (Attachment)
        {
        case GL_COLOR_ATTACHMENT0_OES:
            if(context->frameBuffer->color.surface != gcvNULL)
            {
                /* Set orientation. */
                gcoSURF_SetOrientation(
                    context->frameBuffer->color.surface,
                    gcvORIENTATION_TOP_BOTTOM
                    );
            }

            if(context->frameBuffer->color.target != gcvNULL)
            {
                /* Set orientation. */
                gcoSURF_SetOrientation(
                    context->frameBuffer->color.target,
                    gcvORIENTATION_TOP_BOTTOM
                    );
            }

            if ((context->frameBuffer->color.target != gcvNULL)
                &&  (context->frameBuffer->color.target != target))
            {
                /* Resolve the surface before destroying, if it needs to. */
                if ( context->frameBuffer->needResolve )
                {
                    surfView.surf = context->frameBuffer->color.surface;
                    tgtView.surf  = context->frameBuffer->color.target;
                    /* Resolve color render target into texture. */
                    gcmONERROR(gcoSURF_ResolveRect(&tgtView, &surfView, gcvNULL));

                    context->frameBuffer->needResolve = gcvFALSE;
                }

                gcmVERIFY_OK(gcoSURF_Destroy(context->frameBuffer->color.target));
            }

            /* Set color attachment. */
            context->frameBuffer->color.texture = gcvTRUE;
            context->frameBuffer->color.object  = texture;
            context->frameBuffer->color.offset  = 0;
            context->frameBuffer->color.target  = target;
            context->frameBuffer->dirty         = GL_TRUE;

            if(context->frameBuffer->color.surface)
            {
                /* Dereference unbind surface. */
                gcoSURF_Destroy(context->frameBuffer->color.surface);
            }
            context->frameBuffer->color.surface = surface;

            if(context->frameBuffer->color.surface)
            {
                /* Reference bound surface. */
                gcoSURF_ReferenceSurface(context->frameBuffer->color.surface);
            }

            if (context->frameBuffer->color.surface != gcvNULL &&
                context->frameBuffer->color.target != gcvNULL)
            {
                surfView.surf = context->frameBuffer->color.surface;
                tgtView.surf  = context->frameBuffer->color.target;
                gcmVERIFY_OK(gcoSURF_ResolveRect(&surfView, &tgtView, gcvNULL));
            }
            break;

        case GL_DEPTH_ATTACHMENT_OES:
            if ((context->frameBuffer->depth.target != gcvNULL)
                &&  (context->frameBuffer->depth.target != target)
                &&  (context->frameBuffer->depth.target !=
                     context->frameBuffer->stencil.target))
            {
                /* Resolve the surface before destroying, if it needs to. */
                if ( context->frameBuffer->needResolve )
                {
                    /* Resolve depth render target into texture. */
                    surfView.surf = context->frameBuffer->depth.surface;
                    tgtView.surf  = context->frameBuffer->depth.target;
                    gcmONERROR(gcoSURF_ResolveRect(&tgtView, &surfView, gcvNULL));

                    context->frameBuffer->needResolve = gcvFALSE;
                }

                gcmVERIFY_OK(gcoSURF_Destroy(context->frameBuffer->depth.target));
            }

            /* Set depth attachment. */
            context->frameBuffer->depth.texture = gcvTRUE;
            context->frameBuffer->depth.object  = texture;
            context->frameBuffer->depth.offset  = 0;
            context->frameBuffer->dirty         = GL_TRUE;

            if(context->frameBuffer->depth.surface)
            {
                /* Dereference unbind surface. */
                gcoSURF_Destroy(context->frameBuffer->depth.surface);
            }
            context->frameBuffer->depth.surface = surface;

            if(context->frameBuffer->depth.surface)
            {
                /* Reference bound surface. */
                gcoSURF_ReferenceSurface(context->frameBuffer->depth.surface);
            }

            context->frameBuffer->depth.target  = target;

            if (!firstTimeConstruct && target != gcvNULL)
            {
                gcmVERIFY_OK(
                    gcoSURF_ReferenceSurface(target));
            }
            break;

        case GL_STENCIL_ATTACHMENT_OES:
            if (!firstTimeConstruct && target != gcvNULL)
            {
                gcmVERIFY_OK(
                    gcoSURF_ReferenceSurface(target));
            }

            if ((context->frameBuffer->stencil.target != gcvNULL)
                &&  (context->frameBuffer->stencil.target != target)
                &&  (context->frameBuffer->stencil.target !=
                     context->frameBuffer->depth.target))
            {
                gcmVERIFY_OK(gcoSURF_Destroy(context->frameBuffer->stencil.target));
            }

            /* Set stencil attachment. */
            context->frameBuffer->stencil.texture = gcvTRUE;
            context->frameBuffer->stencil.object  = texture;
            context->frameBuffer->stencil.surface = surface;
            context->frameBuffer->stencil.offset  = 0;
            context->frameBuffer->stencil.target  = target;
            context->frameBuffer->dirty           = GL_TRUE;

            break;

        default:
            glmERROR(GL_INVALID_ENUM);
        }
        context->frameBufferChanged  = gcvTRUE;
        context->stencilStates.dirty = gcvTRUE;

        if ((texture != gcvNULL) &&
            (texture->image.source != gcvNULL) &&
            (texture->image.image != gcvNULL) &&
            (surface != gcvNULL))
        {
            surface = (target != gcvNULL) ? target : surface;

            /* Update latest EGLImage sibling. */
            glfSetEGLImageSrc(texture->image.image, surface);
        }
OnError:;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glFramebufferRenderbufferOES (OES_framebuffer_object)
**
**  Attach a renderbuffer to the currently bound framebuffer.
**
**  Arguments:
**
**      GLenum Target
**          This must be GL_FRAMEBUFFER_OES.
**
**      GLenum Attachment
**          Attachment to attach.  This can be one of the following:
**
**              GL_COLOR_ATTACHMENT0_OES
**                  Attach a color attachment.
**              GL_DEPTH_ATTACHMENT_OES
**                  Attach a depth attachment.
**              GL_STENCIL_ATTACHMENT_OES
**                  Attach a stencil attachment.
**
**      GLenum Renderbuffertarget
**          This must be GL_RENDERBUFFER_OES.
**
**      GLuint Renderbuffer
**          Name of the renderbuffer to attach.
*/

GL_API void GL_APIENTRY glFramebufferRenderbufferOES(
    GLenum Target,
    GLenum Attachment,
    GLenum RenderBufferTarget,
    GLuint RenderBuffer
    )
{
    glmENTER4(glmARGENUM, Target, glmARGENUM, Attachment,
              glmARGENUM, RenderBufferTarget, glmARGUINT, RenderBuffer)
    {
        gceSTATUS status;
        gcoSURF surface;
        glsRENDER_BUFFER_PTR object;
        glsNAMEDOBJECT_PTR wrapper = gcvNULL;
        gcoSURF target = gcvNULL;
        gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
        gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
        gceORIENTATION srcOrient,dstOrient;

        gcmDUMP_API("${ES11 glFramebufferRenderbufferOES 0x%08X 0x%08X 0x%08X 0x%08X}",
            Target, Attachment, RenderBufferTarget, RenderBuffer);

        /* Verify the target. */
        if (Target != GL_FRAMEBUFFER_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Make sure there is a framebuffer object bound. */
        if (context->frameBuffer == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Make sure renderbuffertarget has the right value. */
        if (RenderBufferTarget != GL_RENDERBUFFER_OES)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Determine the render buffer object. */
        if (RenderBuffer == 0)
        {
            object  = gcvNULL;
            surface = gcvNULL;
        }
        else
        {
            wrapper = glfFindNamedObject(context->renderBufferList, RenderBuffer);

            /* Test if the render buffer was found. */
            if (wrapper == gcvNULL)
            {
                glmERROR(GL_INVALID_OPERATION);
                break;
            }

            object  = (glsRENDER_BUFFER_PTR) wrapper->object;
            surface = object->surface;
        }


        /* Check if the RBO renderable, here don't need render into mipmap */
        if (surface != gcvNULL &&
            gcoSURF_IsRenderable(surface) != gcvSTATUS_OK)
        {
            /* Create an RT surface */
            gceSURF_TYPE type;
            gctUINT width, height, depth;
            gceSURF_FORMAT srcFormat,rtFormat;

            gcmONERROR(gcoSURF_GetSize(surface,&width,&height,&depth));

            gcmONERROR(gcoSURF_GetFormat(surface,gcvNULL,&srcFormat));

            gcmONERROR(gco3D_GetClosestRenderFormat(gcvNULL,srcFormat,&rtFormat));

            switch (rtFormat)
            {
            case gcvSURF_D16:
            case gcvSURF_D24X8:
            case gcvSURF_D32:
                type = gcvSURF_DEPTH_TS_DIRTY;
                break;

            default:
                type = gcvSURF_RENDER_TARGET_TS_DIRTY;
                break;
            }

            gcmONERROR(gcoSURF_Construct(context->hal,
                                           width, height, depth,
                                           type,
                                           rtFormat,
                                           gcvPOOL_DEFAULT,
                                           &target));
        }

        /* Dispatch on attachment. */
        switch (Attachment)
        {
        case GL_COLOR_ATTACHMENT0_OES:
            if (context->frameBuffer->color.target != gcvNULL &&
                context->frameBuffer->color.surface != gcvNULL &&
                context->frameBuffer->needResolve)
            {
                gcmONERROR(
                    gcoSURF_QueryOrientation(context->frameBuffer->color.target,
                                             &srcOrient));

                gcmONERROR(
                    gcoSURF_QueryOrientation(context->frameBuffer->color.surface,
                                             &dstOrient));

                gcmONERROR(
                    gcoSURF_SetOrientation(context->frameBuffer->color.surface,
                                           srcOrient));

                /* Resolve color render target into texture. */
                surfView.surf = context->frameBuffer->color.surface;
                tgtView.surf  = context->frameBuffer->color.target;
                gcmONERROR(gcoSURF_ResolveRect(&tgtView, &surfView, gcvNULL));

                gcmONERROR(
                    gcoSURF_SetOrientation(context->frameBuffer->color.surface,
                                           dstOrient));

                    context->frameBuffer->needResolve = gcvFALSE;
            }

            if(context->frameBuffer->color.target != gcvNULL )
            {
                gcmVERIFY_OK(gcoSURF_Destroy(
                    context->frameBuffer->color.target));
            }

            glfDereferenceNamedObject(context,
                                      context->frameBuffer->color.object);

            /* Copy renderbuffer to color attachment. */
            context->frameBuffer->color.texture = gcvFALSE;
            context->frameBuffer->color.object  = wrapper;
            context->frameBuffer->color.offset  = 0;
            context->frameBuffer->color.target  = target;
            context->frameBuffer->dirty         = GL_TRUE;

            if(context->frameBuffer->color.surface)
            {
                /* Dereference unbind surface. */
                gcoSURF_Destroy(context->frameBuffer->color.surface);
            }
            context->frameBuffer->color.surface = surface;

            if (context->frameBuffer->color.target != gcvNULL &&
                context->frameBuffer->color.surface != gcvNULL)
            {
                gcmONERROR(
                    gcoSURF_QueryOrientation(context->frameBuffer->color.target,
                                             &srcOrient));

                gcmONERROR(
                    gcoSURF_QueryOrientation(context->frameBuffer->color.surface,
                                             &dstOrient));

                gcmONERROR(
                    gcoSURF_SetOrientation(context->frameBuffer->color.surface,
                                           srcOrient));

                surfView.surf = context->frameBuffer->color.surface;
                tgtView.surf  = context->frameBuffer->color.target;
                gcmONERROR(gcoSURF_ResolveRect(&surfView, &tgtView, gcvNULL));

                gcmONERROR(
                        gcoSURF_SetOrientation(context->frameBuffer->color.surface,
                                               dstOrient));
            }

            glfReferenceNamedObject(wrapper);
            break;

        case GL_DEPTH_ATTACHMENT_OES:
            /* Delete any render target created for the depth attachment. */
            if (context->frameBuffer->depth.target != gcvNULL)
            {
                gcmVERIFY_OK(gcoSURF_Destroy(
                    context->frameBuffer->depth.target
                    ));
            }

            glfDereferenceNamedObject(context,
                                      context->frameBuffer->depth.object);

            /* Copy renderbuffer to depth attachment. */
            context->frameBuffer->depth.texture = gcvFALSE;
            context->frameBuffer->depth.object  = wrapper;
            context->frameBuffer->depth.offset  = 0;
            context->frameBuffer->depth.target  = target;
            context->frameBuffer->dirty         = GL_TRUE;

            if(context->frameBuffer->depth.surface)
            {
                /* Dereference unbind surface. */
                gcoSURF_Destroy(context->frameBuffer->depth.surface);
            }
            context->frameBuffer->depth.surface = surface;

            /* Merge the depth and stencil buffers. */
            _MergeDepthAndStencil(context);

            if (object != gcvNULL)
            {
                /* Mark the renderbuffer as bound. */
                object->bound = gcvTRUE;
            }

            glfReferenceNamedObject(wrapper);
            break;

        case GL_STENCIL_ATTACHMENT_OES:
            gcmASSERT(context->frameBuffer->stencil.target == gcvNULL);

            glfDereferenceNamedObject(context,
                                      context->frameBuffer->stencil.object);

            /* Copy renderbuffer to stencil attachment. */
            context->frameBuffer->stencil.texture = gcvFALSE;
            context->frameBuffer->stencil.object  = wrapper;
            context->frameBuffer->stencil.surface = surface;
            context->frameBuffer->stencil.offset  = 0;
            context->frameBuffer->stencil.target  = target;
            context->frameBuffer->dirty           = GL_TRUE;

            /* Merge the depth and stencil buffers. */
            _MergeDepthAndStencil(context);

            if (object != gcvNULL)
            {
                /* Mark the renderbuffer as bound. */
                object->bound = gcvTRUE;
            }

            glfReferenceNamedObject(wrapper);
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
        }
        context->frameBufferChanged  = gcvTRUE;
        context->stencilStates.dirty = gcvTRUE;

OnError:;
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glCheckFramebufferStatusOES (OES_framebuffer_object)
**
**  Check whether the currently bound framebuffer is complete or not.  A
**  complete framebuffer can be used to render into.
**
**  Arguments:
**
**      GLenum Target
**          This must be GL_FRAMEBUFFER_OES.
**
**  Returns:
**
**      GLenum
**          The completess of the frame buffer or 0 if there is an error.  Can
**          be one of the following:
**
**              GL_FRAMEBUFFER_COMPLETE
**                  The framebuffer is complete.
**              GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
**                  One of the attachments to the framebuffer is incomplete.
**              GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
**                  There are no attachments to the framebuffer.
**              GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
**                  The attachments have different dimensions.
**              GL_FRAMEBUFFER_UNSUPPORTED
**                  The attachments cannot be used together.
*/

GL_API GLenum GL_APIENTRY glCheckFramebufferStatusOES(
    GLenum Target
    )
{
    GLenum result = 0;

    glmENTER1(glmARGENUM, Target)
    {
        /* Make sure target has the right value. */
        if (Target != GL_FRAMEBUFFER_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Check if the current framebuffer is complete or not. */
        result = glfIsFramebufferComplete(context);

        gcmDUMP_API("${ES11 glCheckFramebufferStatusOES 0x%08X := 0x%08X}",
            Target, result);
    }
    glmLEAVE();

    /* Return the completeness. */
    return result;
}


/*******************************************************************************
**
**  glGetFramebufferAttachmentParameterivOES (OES_framebuffer_object)
**
**  Query information about an attachment of a framebuffer.
**
**  Arguments:
**
**      GLenum Target
**          This must be GL_FRAMEBUFFER_OES.
**
**      GLenum Attachment
**          Attachment to query.  This can be one of the following:
**
**              GL_COLOR_ATTACHMENT0_OES
**                  Query information about the color attachment.
**              GL_DEPTH_ATTACHMENT_OES
**                  Query information about the depth attachment.
**              GL_STENCIL_ATTACHMENT_OES
**                  Query information about the stencil attachment.
**
**      GLenum Name
**          Parameter name to query.  This can be one of the following:
**
**              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES
**                  Query the object type of the attachment.
**              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES
**                  Query the object name of the attachment.
**              GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES
**                  Query the texture level of the attachment.  The attachment
**                  has to be a texture object and the returned level is always
**                  0.
**              GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES
**                  Query the cube map face of the attachment.  The attachment
**                  has to be a cube map texture object.
**
**      GLint * Params
**          Pointer to a variable that will receive the information.
*/

GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES(
    GLenum Target,
    GLenum Attachment,
    GLenum Name,
    GLint * Params
    )
{
    glmENTER4(glmARGENUM, Target, glmARGENUM, Attachment, glmARGENUM, Name,
              glmARGPTR, Params)
    {
        glsFRAME_BUFFER_ATTACHMENT_PTR attachment;

        /* Verify the target. */
        if (Target != GL_FRAMEBUFFER_OES)
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Make sure there is a framebuffer object bound. */
        if (context->frameBuffer == gcvNULL)
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Get requested attachment. */
        if (Attachment == GL_COLOR_ATTACHMENT0_OES)
        {
            /* Get color attachment. */
            attachment = &context->frameBuffer->color;
        }

        else if (Attachment == GL_DEPTH_ATTACHMENT_OES)
        {
            /* Get depth attachment. */
            attachment = &context->frameBuffer->depth;
        }

        else if (Attachment == GL_STENCIL_ATTACHMENT_OES)
        {
            /* Get stencil attachment. */
            attachment = &context->frameBuffer->stencil;
        }

        else
        {
            glmERROR(GL_INVALID_ENUM);
            break;
        }

        /* Parse name. */
        switch (Name)
        {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES:
            /* Test for no attachment. */
            if (attachment->object == gcvNULL)
            {
                * Params = GL_NONE_OES;
            }

            /* Test for texture attachment. */
            else if (attachment->texture)
            {
                * Params = GL_TEXTURE;
            }

            /* Render buffer attachment. */
            else
            {
                * Params = GL_RENDERBUFFER_OES;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES:
            /* Return attachment name. */
            if (attachment->object == gcvNULL)
            {
                * Params = 0;
            }
            else if (attachment->texture)
            {
                /* Cast to texture. */
                glsTEXTUREWRAPPER_PTR texture = attachment->object;

                /* Set the name. */
                * Params = texture->name;
            }
            else
            {
                /* Cast to named object wrapper. */
                glsNAMEDOBJECT_PTR wrapper = attachment->object;

                /* Set the name. */
                * Params = wrapper->name;
            }
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES:
            /* Return attached texture level. */
            if ((attachment->object == gcvNULL) || !attachment->texture)
            {
                glmERROR(GL_INVALID_ENUM);
                break;
            }

            /* This has to be zero. */
            * Params = 0;
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES:
            /* Return attached texture cube map face. */
            if ((attachment->object == gcvNULL) || !attachment->texture)
            {
                glmERROR(GL_INVALID_ENUM);
                break;
            }

            /* Not a cube map. */
            * Params = 0;
            break;

        default:
            glmERROR(GL_INVALID_ENUM);
        }

        gcmDUMP_API("${ES11 glGetFramebufferAttachmentParameterivOES 0x%08X 0x%08X 0x%08X (0x%08X)",
            Target, Attachment, Name, Params);
        gcmDUMP_API_ARRAY(Params, 1);
        gcmDUMP_API("$}");
    }
    glmLEAVE();
}
