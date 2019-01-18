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
#include "gc_es_object_inline.c"
#include "gc_es_device.h"

extern __GLformatInfo __glFormatInfoTable[];

#define FRAMEBUFFER_CHECK_TARGET_ATTACHMENT()                                               \
    switch (target)                                                                         \
    {                                                                                       \
    case GL_FRAMEBUFFER:                                                                    \
        if (DRAW_FRAMEBUFFER_BINDING_NAME == 0)                                             \
        {                                                                                   \
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);                                           \
        }                                                                                   \
        break;                                                                              \
    case GL_DRAW_FRAMEBUFFER:                                                               \
        if (DRAW_FRAMEBUFFER_BINDING_NAME == 0)                                             \
        {                                                                                   \
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);                                           \
        }                                                                                   \
        break;                                                                              \
    case GL_READ_FRAMEBUFFER:                                                               \
        if (READ_FRAMEBUFFER_BINDING_NAME == 0)                                             \
        {                                                                                   \
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);                                           \
        }                                                                                   \
        break;                                                                              \
    default:                                                                                \
        __GL_ERROR_EXIT(GL_INVALID_ENUM);                                                    \
    }                                                                                       \
    if ((attachment < GL_DEPTH_ATTACHMENT) && (attachment > __GL_COLOR_ATTACHMENTn))        \
    {                                                                                       \
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);                                              \
    }                                                                                       \
    if (((attachment < GL_COLOR_ATTACHMENT0) || (attachment >= GL_DEPTH_ATTACHMENT)) &&     \
        (attachment != GL_DEPTH_ATTACHMENT) && (attachment != GL_STENCIL_ATTACHMENT) &&     \
        (attachment != GL_DEPTH_STENCIL_ATTACHMENT))                                        \
    {                                                                                       \
        __GL_ERROR_EXIT(GL_INVALID_ENUM);                                                    \
    }

extern __GLformatInfo* __glGetFormatInfo(GLenum internalFormat);
extern GLboolean __glGetAttribFromImage(__GLcontext *gc,
                                                 khrEGL_IMAGE_PTR image,
                                                 GLint *internalFormat,
                                                 GLint *format,
                                                 GLint *type,
                                                 GLint *width,
                                                 GLint *height);
extern GLboolean __glDeleteTextureObject(__GLcontext *gc, __GLtextureObject *tex);

GLvoid __glFramebufferRenderbuffer(__GLcontext *gc, __GLframebufferObject *framebufferObj, GLint attachIndex, __GLrenderbufferObject *renderbufferObj);
GLvoid __glBindFramebuffer(__GLcontext *gc, GLenum target, GLuint name);
GLvoid __glBindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer);
GLboolean __glDeleteRenderbufferObject(__GLcontext *gc, __GLrenderbufferObject *renderbuffer);


#define _GC_OBJ_ZONE __GLES3_ZONE_CORE


__GL_INLINE GLvoid __glFramebufferResetAttachPoint(__GLcontext *gc, __GLfboAttachPoint *attachPoint)
{
    attachPoint->objType    = GL_NONE;
    attachPoint->objName    = 0;
    attachPoint->object     = gcvNULL;
    attachPoint->level      = 0;
    attachPoint->face       = 0;
    attachPoint->layer      = 0;
    attachPoint->slice      = 0;
    attachPoint->layered    = GL_FALSE;
    attachPoint->cube       = GL_FALSE;
    attachPoint->isExtMode  = GL_FALSE;
}

__GL_INLINE GLvoid __glRemoveFramebufferAsImageUser(__GLcontext *gc, __GLframebufferObject *framebuffer, __GLfboAttachPoint* attachPoint)
{
    if (!attachPoint || attachPoint->objType == GL_NONE || attachPoint->objName == 0)
    {
        return;
    }

    switch (attachPoint->objType)
    {
    case GL_RENDERBUFFER:
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;

            if (rbo)
            {
                __glRemoveImageUser(gc, &rbo->fboList, framebuffer);

                if (!rbo->fboList)
                {
                    gc->dp.cleanRenderbufferShadow(gc, rbo);
                }

                if (!rbo->bindCount && !rbo->fboList && (rbo->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteRenderbufferObject(gc, rbo);
                    __glFramebufferResetAttachPoint(gc, attachPoint);
                }
            }
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject *tex = (__GLtextureObject*)attachPoint->object;

            if (tex)
            {
                __glRemoveImageUser(gc, &tex->fboList, framebuffer);

                if (!tex->fboList)
                {
                    gc->dp.cleanTextureShadow(gc, tex);
                }

                if (!tex->bindCount && !tex->fboList && !tex->imageList && (tex->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteTextureObject(gc, tex);
                    __glFramebufferResetAttachPoint(gc, attachPoint);
                }
            }
        }

        break;
    }
}

GLvoid __glFramebufferResetAttachIndex(__GLcontext *gc,
                                       __GLframebufferObject *fbo,
                                       GLint attachIndex,
                                       GLboolean drawFbo)
{
    __glFramebufferResetAttachPoint(gc, &fbo->attachPoint[attachIndex]);
}

GLvoid __glInitRenderbufferObject(__GLcontext *gc, __GLrenderbufferObject *renderbuffer, GLuint name)
{
    renderbuffer->bindCount = 0;
    renderbuffer->internalFormat = GL_RGBA4;
    renderbuffer->name = name;
    renderbuffer->formatInfo = &__glFormatInfoTable[__GL_FMT_MAX];
    renderbuffer->eglImage = gcvNULL;
}

GLboolean __glDeleteRenderbufferObject(__GLcontext *gc, __GLrenderbufferObject *renderbuffer)
{
    __GLimageUser *fboUserList = renderbuffer->fboList;
    __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
    __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

    /*
    ** __GL_OBJECT_IS_DELETED is cleared here because we do not want this object
    ** deleted in the following bind functions, __glBindRenderbuffer, __glFramebufferRenderbuffer.
    ** otherwise there will be recursion: __glDeleteRBO-->__glBindObject-->__glDeleteRBO,
    ** and the object will be deleted twice.
    */
    renderbuffer->flag &= ~__GL_OBJECT_IS_DELETED;

    if (renderbuffer == gc->frameBuffer.boundRenderbufObj)
    {
        __glBindRenderbuffer(gc, GL_RENDERBUFFER, 0);
    }

    /* Mark dirty all the fbos this rbo attached to */
    while (fboUserList)
    {
        GLuint i;
        /* fboUserList may be freed in __glFramebufferTexture, get the next in the beginning */
        __GLimageUser *nextUser = fboUserList->next;
        __GLframebufferObject *fbo = (__GLframebufferObject*)fboUserList->imageUser;

        /*  Unbound the renderbuffer only from currently bound framebuffer */
        if (fbo == drawFbo)
        {
            for (i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if (drawFbo->attachPoint[i].objType == GL_RENDERBUFFER &&
                    drawFbo->attachPoint[i].object  == (GLvoid*)renderbuffer)
                {
                    __glFramebufferRenderbuffer(gc, drawFbo, i, gcvNULL);
                }
            }
        }

        if (readFbo != drawFbo && fbo == readFbo)
        {
            for (i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if (readFbo->attachPoint[i].objType == GL_RENDERBUFFER &&
                    readFbo->attachPoint[i].object  == (GLvoid*)renderbuffer)
                {
                    __glFramebufferRenderbuffer(gc, readFbo, i, gcvNULL);
                }
            }
        }

        /* Mark dirty all the fbos this rbo attached to */
        __GL_FRAMEBUFFER_COMPLETE_DIRTY(fbo);

        fboUserList = nextUser;
    }

    /* Detach rbo chip objects from this context*/
    (*gc->dp.detachRenderbuffer)(gc, renderbuffer);

    /* Do not delete the rbObj if there are other contexts bound to it. */
    if (renderbuffer->bindCount != 0 || renderbuffer->fboList)
    {
        /* Set the flag to indicate the object is marked for delete */
        renderbuffer->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    if (renderbuffer->label)
    {
        gc->imports.free(gc, renderbuffer->label);
    }

    /* Notify Dp that this renderbuffer object is deleted.
    */
    (*gc->dp.deleteRenderbuffer)(gc, renderbuffer);

    /* Free fbolist */
    __glFreeImageUserList(gc, &renderbuffer->fboList);

    /* Delete the renderbuffer object structure */
    (*gc->imports.free)(gc, renderbuffer);

    return GL_TRUE;
}

GLvoid __glInitFramebufferObject(__GLcontext *gc, __GLframebufferObject *framebuffer, GLuint name)
{
    GLuint i;

    framebuffer->name = name;
    framebuffer->flag = 0;
    framebuffer->fbIntMask = 0;

    /* Init per attachment state*/
    for (i = 0; i < __GL_MAX_ATTACHMENTS; i++)
    {
        framebuffer->attachPoint[i].objType = name ? GL_NONE : GL_FRAMEBUFFER_DEFAULT;
        framebuffer->attachPoint[i].objName = 0;
        framebuffer->attachPoint[i].face = 0;
        framebuffer->attachPoint[i].layer = 0;
        framebuffer->attachPoint[i].level = 0;
        framebuffer->attachPoint[i].slice = 0;
        framebuffer->attachPoint[i].layered = GL_FALSE;
        framebuffer->attachPoint[i].cube = GL_FALSE;
        framebuffer->attachPoint[i].seqNumber = 0;
    }

    /*Init per framebuffer state*/
    framebuffer->drawBuffers[0]  = GL_COLOR_ATTACHMENT0;
    framebuffer->drawBufferCount = 1;
    for (i = 1; i < __GL_MAX_COLOR_ATTACHMENTS; i++)
    {
        framebuffer->drawBuffers[i] = GL_NONE;
    }

    framebuffer->readBuffer = GL_COLOR_ATTACHMENT0;

    framebuffer->fast = GL_FALSE;

    framebuffer->defaultWidth = 0;
    framebuffer->defaultHeight = 0;
    framebuffer->defaultSamples = 0;
    framebuffer->defaultSamplesUsed = 0;
    framebuffer->defaultFixedSampleLoc = GL_FALSE;
}

GLboolean __glDeleteFramebufferObject(__GLcontext *gc, __GLframebufferObject *framebuffer)
{
    GLuint i;

    /*
    ** If the framebuffer object that is being deleted is currently bound to gc,
    ** unbind it from gc and bind the default obj 0 to gc.
    */
    if (framebuffer == gc->frameBuffer.drawFramebufObj)
    {
        __glBindFramebuffer(gc, GL_DRAW_FRAMEBUFFER, 0);
    }

    if (framebuffer == gc->frameBuffer.readFramebufObj)
    {
        __glBindFramebuffer(gc, GL_READ_FRAMEBUFFER, 0);
    }

    if (framebuffer->label)
    {
        gc->imports.free(gc, framebuffer->label);
    }

    /* Remove fbo from image userlist of attaching image */
    for (i = 0; i < __GL_MAX_ATTACHMENTS; ++i)
    {
        __GLfboAttachPoint *attachPoint = &framebuffer->attachPoint[i];

        if (attachPoint->objType == GL_NONE || attachPoint->objName == 0)
        {
            continue;
        }

        __glRemoveFramebufferAsImageUser(gc, framebuffer, attachPoint);
        __glFramebufferResetAttachIndex(gc, framebuffer, i, GL_TRUE);
    }

    (*gc->imports.free)(gc, framebuffer);

    return GL_TRUE;
}

GLvoid __glBindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer)
{
    __GLrenderbufferObject *renderbufferObj = gcvNULL;
    __GLrenderbufferObject *boundObj = gc->frameBuffer.boundRenderbufObj;

    if (renderbuffer == 0)
    {
        /* Retrieve the default object in __GLcontext.
        */
        renderbufferObj = &gc->frameBuffer.defaultRBO;
        GL_ASSERT(renderbufferObj->name == 0);
    }
    else
    {
        /* Retrieve the object from the "gc->frameBuffer.shared" structure.*/
        renderbufferObj = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer);
    }

    if (renderbufferObj == boundObj)
    {
        return;
    }

    if (gcvNULL == renderbufferObj)
    {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new renderbuffer object and initialize it.
        */
        renderbufferObj = (__GLrenderbufferObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLrenderbufferObject));
        __glInitRenderbufferObject(gc, renderbufferObj, renderbuffer);

        /* Add this renderbuffer object to the "gc->frameBuffer.shared" structure.
        */
        __glAddObject(gc, gc->frameBuffer.rboShared, renderbuffer, renderbufferObj);

        /* Mark the name "renderbuffer" used in the renderbuffer nameArray.
        */
        __glMarkNameUsed(gc, gc->frameBuffer.rboShared, renderbuffer);
    }

    /* Release the previously bound obj for this target.
    ** And install the new object to the target.
    */
    gc->frameBuffer.boundRenderbufObj = renderbufferObj;

    /* Delete the object if there is nothing bound to the object
    */
    if (boundObj->name != 0)
    {
        if ((--boundObj->bindCount) == 0 && !boundObj->fboList && (boundObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteRenderbufferObject(gc, boundObj);
        }
    }

    /* bindCount includes both single context and shared context bindings.
    */
    if (renderbufferObj->name)
    {
        renderbufferObj->bindCount++;
    }

    /* Call the dp interface */
    (*gc->dp.bindRenderbuffer)(gc, renderbufferObj);
}

GLvoid __glBindFramebuffer(__GLcontext *gc, GLenum target, GLuint name)
{
    __GLframebufferObject *prevDrawObj = gcvNULL;
    __GLframebufferObject *prevReadObj = gcvNULL;

    __GLframebufferObject *curDrawObj = gcvNULL;
    __GLframebufferObject *curReadObj = gcvNULL;

    /* Redundant check: If the name is current name, return. */
    switch (target)
    {
    case GL_FRAMEBUFFER:
        if ((gc->frameBuffer.drawFramebufObj->name == name) &&
            (gc->frameBuffer.readFramebufObj->name == name))
        {
            return;
        }
        break;

    case GL_DRAW_FRAMEBUFFER:
        if (gc->frameBuffer.drawFramebufObj->name == name)
        {
            return;
        }
        break;

    case GL_READ_FRAMEBUFFER:
        if (gc->frameBuffer.readFramebufObj->name == name)
        {
            return;
        }
        break;
    }

    GL_ASSERT(gcvNULL != gc->frameBuffer.fboManager);

    prevDrawObj = gc->frameBuffer.drawFramebufObj;
    prevReadObj = gc->frameBuffer.readFramebufObj;

    if (name)
    {
        __GLframebufferObject *fbo = (__GLframebufferObject*)__glGetObject(gc, gc->frameBuffer.fboManager, name);

        if (gcvNULL == fbo)
        {
            fbo = (__GLframebufferObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLframebufferObject));

            __glInitFramebufferObject(gc, fbo, name);

            /* Add this frameBuffer object to the "gc->frameBuffer.shared" structure. */
            __glAddObject(gc, gc->frameBuffer.fboManager, name, fbo);

            /* Mark the name "frameBuffer" used in the frameBuffer nameArray. */
            __glMarkNameUsed(gc, gc->frameBuffer.fboManager, name);
        }

        switch (target)
        {
        case GL_FRAMEBUFFER:
            curDrawObj = fbo;
            curReadObj = fbo;
            break;
        case GL_DRAW_FRAMEBUFFER:
            curDrawObj = fbo;
            curReadObj = prevReadObj;
            break;
        case GL_READ_FRAMEBUFFER:
            curDrawObj = prevDrawObj;
            curReadObj = fbo;
            break;
        }
    }
    else
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
            curDrawObj = &gc->frameBuffer.defaultDrawFBO;
            curReadObj = &gc->frameBuffer.defaultReadFBO;
            break;
        case GL_DRAW_FRAMEBUFFER:
            curDrawObj = &gc->frameBuffer.defaultDrawFBO;
            curReadObj = prevReadObj;
            break;
        case GL_READ_FRAMEBUFFER:
            curDrawObj = prevDrawObj;
            curReadObj = &gc->frameBuffer.defaultReadFBO;
            break;
        }
    }

    if (prevDrawObj != curDrawObj)
    {
        GLboolean retVal;

        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;
        gc->frameBuffer.drawFramebufObj = curDrawObj;
        retVal = (*gc->dp.bindDrawFramebuffer)(gc, prevDrawObj, curDrawObj);
        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

    if (prevReadObj != curReadObj)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
        gc->frameBuffer.readFramebufObj = curReadObj;
        (*gc->dp.bindReadFramebuffer)(gc, prevReadObj, curReadObj);
    }
}

GLboolean __glFboIsRboAttached(__GLcontext *gc, __GLframebufferObject *fbo, __GLrenderbufferObject *rbo)
{
    /* Rbo cannot be bound to default fbo */
    if (rbo && fbo && fbo->name > 0)
    {
        GLuint i;

        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
        {
            if (fbo->attachPoint[i].objType == GL_RENDERBUFFER &&
                fbo->attachPoint[i].objName == rbo->name)
            {
                return GL_TRUE;
            }
        }
    }

    return GL_FALSE;
}

GLvoid __glRenderbufferStorage(__GLcontext* gc,
                               GLenum target,
                               GLsizei samples,
                               GLenum internalformat,
                               GLsizei width,
                               GLsizei height,
                               GLboolean isExtMode)
{
    GLint formatSamples = 0;
    __GLformatInfo *formatInfo;
    __GLrenderbufferObject *curRbo = gcvNULL;

    __GL_HEADER();

    if (target != GL_RENDERBUFFER)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if ((width < 0 || height < 0) || samples < 0 ||
        ((width > (GLsizei)gc->constants.maxRenderBufferSize) || (height > (GLsizei)gc->constants.maxRenderBufferSize)))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch(internalformat)
    {
    case GL_RGBA8:
    case GL_RGBA4:
    case GL_RGB8:
    case GL_RGB565:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
    case GL_R8:
    case GL_RG8:
        break;

    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
    case GL_RGBA8UI:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
    case GL_RGB10_A2UI:
        break;

    case GL_SRGB8_ALPHA8:
        if (!__glExtension[__GL_EXTID_EXT_sRGB].bEnabled && gc->apiVersion < __GL_API_VERSION_ES30)
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        break;

    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32_OES:
    case GL_DEPTH_COMPONENT32F:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        break;

    case GL_STENCIL_INDEX8:
        break;

    case GL_STENCIL_INDEX1_OES:
        if (!__glExtension[__GL_EXTID_OES_stencil1].bEnabled)
        {
           __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        break;

    case GL_STENCIL_INDEX4_OES:
        if (!__glExtension[__GL_EXTID_OES_stencil4].bEnabled)
        {
           __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        break;

    case GL_R16F:
    case GL_RG16F:
    case GL_RGB16F:
    case GL_RGBA16F:
        if (!__glExtension[__GL_EXTID_EXT_color_buffer_half_float].bEnabled)
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        break;

    case GL_R32F:
    case GL_RG32F:
    case GL_RGB32F:
    case GL_RGBA32F:
    case GL_R11F_G11F_B10F:
        if (!__glExtension[__GL_EXTID_EXT_color_buffer_float].bEnabled)
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    formatInfo = __glGetFormatInfo(internalformat);

    (*gc->dp.queryFormatInfo)(gc, formatInfo->drvFormat, gcvNULL, &formatSamples, 1);

    /* Must be renderable formats */
    if (!formatInfo->renderable)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (samples > formatSamples)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    curRbo = gc->frameBuffer.boundRenderbufObj;

    /* There is no renderbuffer object corresponding to the name zero,
     * so client attempts to modify or query renderbuffer state while zero is bound will generate errors.
     */
    if (curRbo->name == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Redundancy check */
    if ((curRbo->width == width) &&
        (curRbo->height == height) &&
        (curRbo->internalFormat == internalformat) &&
        (curRbo->samples == samples))
    {
        __GL_EXIT();
    }

    curRbo->width           = width;
    curRbo->height          = height;
    curRbo->internalFormat  = internalformat;
    curRbo->samples         = samples;
    curRbo->formatInfo      = formatInfo;
    curRbo->isExtMode       = isExtMode;

    /* Call the dp interface: must map app request internal format to implement format */
    if (!(*gc->dp.renderbufferStorage)(gc, curRbo))
    {
        __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
    }

    if (curRbo->fboList)
    {
        __GLimageUser *imageUserList = curRbo->fboList;
        __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
        __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

        /* Mark dirty all the fbos this rbo attached to */
        while (imageUserList)
        {
            __GL_FRAMEBUFFER_COMPLETE_DIRTY((__GLframebufferObject*)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }

        if (drawFbo == readFbo)
        {
            /* If draw and read FBO are same, check once. */
            if (__glFboIsRboAttached(gc, drawFbo, curRbo))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_DRAW_READ_BITS;
            }
        }
        else
        {
            if (__glFboIsRboAttached(gc, drawFbo, curRbo))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;
            }

            if (__glFboIsRboAttached(gc, readFbo, curRbo))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
            }
        }
    }
OnError:

OnExit:
    __GL_FOOTER();
    return;

}

GLenum __createEglImageRenderbuffer(__GLcontext* gc, GLenum renderbuffer, GLvoid* eglImage)
{
    __GLrenderbufferObject *rbo = gcvNULL;

    if (gc->frameBuffer.rboShared == gcvNULL)
    {
        return EGL_BAD_PARAMETER;
    }
    rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer);

    if (rbo == gcvNULL)
    {
        return EGL_BAD_PARAMETER;
    }

    return (*gc->dp.createEglImageRenderbuffer)(gc, rbo, eglImage);
}

GLvoid __eglImageTargetRenderbufferStorageOES(__GLcontext* gc, GLenum target, GLvoid *eglImage)
{
    khrEGL_IMAGE * image = (khrEGL_IMAGE_PTR)(eglImage);
    __GLrenderbufferObject *curRbo = gcvNULL;
    __GLformatInfo *formatInfo;
    GLboolean retVal;

    __GL_HEADER();

    curRbo = gc->frameBuffer.boundRenderbufObj;

    if (!__glGetAttribFromImage(gc,image,(GLint*)(&curRbo->internalFormat),gcvNULL,gcvNULL,
                                &curRbo->width,&curRbo->height))
    {
        __GL_EXIT();
    }

    formatInfo = __glGetFormatInfo(curRbo->internalFormat);
    curRbo->formatInfo      = formatInfo;
    curRbo->samples         = 0;
    curRbo->eglImage = eglImage;

    retVal = (*gc->dp.eglImageTargetRenderbufferStorageOES)(gc, curRbo, target, eglImage);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid __glFramebufferTexture(__GLcontext *gc,
                              __GLframebufferObject *framebufferObj,
                              GLint attachIndex,
                              __GLtextureObject *texObj,
                              GLint level,
                              GLint face,
                              GLint layer,
                              GLsizei samples,
                              GLboolean layered,
                              GLboolean isExtMode)
{
    __GLfboAttachPoint *attachPoint;
    __GLfboAttachPoint preAttach;
    __GL_HEADER();

    /* Check parameters */
    GL_ASSERT(attachIndex >= 0 && attachIndex < __GL_MAX_ATTACHMENTS);

    attachPoint  = &framebufferObj->attachPoint[attachIndex];

    /* Check and skip for same bound. */
    if (attachPoint->object == (GLvoid*)texObj)
    {
        if (!texObj)
        {
            /* Skip if still bound to 0 */
            __GL_EXIT();
        }
        else if (attachPoint->objName   == texObj->name &&
                 attachPoint->face      == face &&
                 attachPoint->level     == level &&
                 attachPoint->layer     == layer &&
                 attachPoint->layered   == layered &&
                 attachPoint->seqNumber == texObj->seqNumber)
        {
            /* Skip if bound with same obj+face+level+slice */
            __GL_EXIT();
        }
    }

    /* If there is previously attached obj, remove fbo from it's owner list. */
    __glRemoveFramebufferAsImageUser(gc, framebufferObj, attachPoint);
    __GL_MEMCOPY(&preAttach, attachPoint, gcmSIZEOF(preAttach));

    if (texObj)
    {
        attachPoint->objType = GL_TEXTURE;
        attachPoint->objName = texObj->name;
        attachPoint->object  = (GLvoid*)texObj;
        attachPoint->level   = level;
        attachPoint->face    = face;
        attachPoint->layer   = layer;
        attachPoint->slice   = face > 0 ? face : layer;
        attachPoint->samples = samples;
        attachPoint->layered = layered;
        attachPoint->cube    = texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX ? GL_TRUE : GL_FALSE;
        attachPoint->isExtMode = isExtMode;

        /* Add fbo to texture obj's image userlist. */
        __glAddImageUser(gc, &texObj->fboList, framebufferObj);
        attachPoint->seqNumber = texObj->seqNumber;
    }
    else
    {
        /* Detach image */
        __glFramebufferResetAttachIndex(gc, framebufferObj, attachIndex, GL_TRUE);
    }

    if (!gc->dp.frameBufferTexture(gc,
                                   framebufferObj,
                                   attachIndex,
                                   texObj,
                                   level,
                                   face,
                                   samples,
                                   layer,
                                   layered,
                                   &preAttach))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    if(texObj)
    {
        texObj->params.mipHint = __GL_TEX_MIP_HINT_AUTO;
    }

    /* Dirty this framebuffer object */
    __GL_FRAMEBUFFER_COMPLETE_DIRTY(framebufferObj);

    if (framebufferObj == gc->frameBuffer.drawFramebufObj)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;
    }

    if (framebufferObj == gc->frameBuffer.readFramebufObj)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
    }

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid __glFramebufferRenderbuffer(__GLcontext *gc,
                                   __GLframebufferObject *framebufferObj,
                                   GLint attachIndex,
                                   __GLrenderbufferObject *renderbufferObj)
{
    __GLfboAttachPoint* attachPoint;
    __GLfboAttachPoint preAttach;
    __GL_HEADER();

    GL_ASSERT(attachIndex >= 0 && attachIndex < __GL_MAX_ATTACHMENTS);

    attachPoint  = &framebufferObj->attachPoint[attachIndex];

    /* Check and skip for same bound. */
    if (attachPoint->object == (GLvoid*)renderbufferObj)
    {
        if (!renderbufferObj)
        {
            /* Skip if still bound to 0 */
            __GL_EXIT();
        }
        else if (attachPoint->objName == renderbufferObj->name)
        {
            /* Skip if bound with same object */
            __GL_EXIT();;
        }
    }

    /* If there is previously attached obj, remove fbo from it's owner list. */
    __glRemoveFramebufferAsImageUser(gc, framebufferObj, attachPoint);
    __GL_MEMCOPY(&preAttach, attachPoint, gcmSIZEOF(preAttach));

    if (renderbufferObj != gcvNULL)
    {
        /* Attach new image*/
        attachPoint->objType = GL_RENDERBUFFER;
        attachPoint->objName = renderbufferObj->name;
        attachPoint->object  = renderbufferObj;
        attachPoint->isExtMode = renderbufferObj->isExtMode;

        /* Add fbo to renderbuffer obj's image userlist. */
        __glAddImageUser(gc, &renderbufferObj->fboList, framebufferObj);
    }
    else
    {
        /* Detach image */
        __glFramebufferResetAttachIndex(gc, framebufferObj, attachIndex, GL_TRUE);
    }

    gc->dp.framebufferRenderbuffer(gc, framebufferObj, attachIndex, renderbufferObj, &preAttach);

    /* Dirty this framebuffer object */
    __GL_FRAMEBUFFER_COMPLETE_DIRTY(framebufferObj);

    if (framebufferObj == gc->frameBuffer.drawFramebufObj)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;
    }

    if (framebufferObj == gc->frameBuffer.readFramebufObj)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
    }

OnExit:
    __GL_FOOTER();
    return;
}

/*
** Function to validate framebuffer change before any render action, and update device pipe if necessary.
** Currently handle only render to FBO.
*/
GLvoid __glEvaluateFramebufferChange(__GLcontext *gc, GLbitfield flags)
{
    GLboolean skipDraw = GL_FALSE;
    GLboolean drawChecked = GL_FALSE;
    __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
    __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

    __GL_HEADER();

    if (flags & __GL_BUFFER_DRAW_BIT)
    {
        GLboolean complete = gc->dp.isFramebufferComplete(gc, drawFbo);

        if (complete)
        {
            /* Also skip draw if default fbo is zero sized, but not generate any error. */
            if (drawFbo->name == 0 && (gc->drawablePrivate->flags & __GL_DRAWABLE_FLAG_ZERO_WH))
            {
                skipDraw = GL_TRUE;
            }
        }
        else
        {
            skipDraw = GL_TRUE;
            __GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
        }

        drawChecked = GL_TRUE;
    }

    /* If read FBO was different than drawFBO or drawFBO wasn't checked */
    if ((flags & __GL_BUFFER_READ_BIT) && (readFbo != drawFbo || !drawChecked))
    {
        GLboolean complete = gc->dp.isFramebufferComplete(gc, readFbo);

        if (complete)
        {
            /* Also skip draw if default fbo is zero sized, but not generate any error. */
            if (readFbo->name == 0 && (gc->readablePrivate->flags & __GL_DRAWABLE_FLAG_ZERO_WH))
            {
                skipDraw = GL_TRUE;
            }
        }
        else
        {
            skipDraw = GL_TRUE;
            __GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
        }
    }

    /* Check completeness */
    if (skipDraw)
    {
        gc->flags |= __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }
    else
    {
        gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }

    __GL_FOOTER();
}

__GLformatInfo* __glGetFramebufferFormatInfo(__GLcontext *gc, __GLframebufferObject *framebufferObj, GLenum attachment)
{
    __GLformatInfo* formatInfo = gcvNULL;
    __GLfboAttachPoint *attachPoint;
    GLint attachIndex = __glMapAttachmentToIndex(attachment);

    __GL_HEADER();

    if (!framebufferObj || -1 == attachIndex)
    {
        __GL_EXIT();
    }

    attachPoint = &framebufferObj->attachPoint[attachIndex];
    switch (attachPoint->objType)
    {
    case GL_RENDERBUFFER:
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
            GL_ASSERT(rbo);
            formatInfo = rbo->formatInfo;
        }
        break;

    case GL_TEXTURE:
        {
            __GLmipMapLevel *mipmap;
            __GLtextureObject *tex = (__GLtextureObject*)attachPoint->object;
            GL_ASSERT(tex);
            mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];
            formatInfo = mipmap->formatInfo;
        }
        break;

    default:
        break;
    }

OnExit:
    __GL_FOOTER();
    return formatInfo;
}

GLvoid __glInitFramebufferStates(__GLcontext *gc)
{
    __GLsharedObjectMachine *fboManager = gcvNULL, *rboShared = gcvNULL;

    __GL_HEADER();

    /* FBO can NOT be shared across contexts */
    GL_ASSERT(gc->frameBuffer.fboManager == gcvNULL);
    gc->frameBuffer.fboManager = (__GLsharedObjectMachine *)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

    fboManager = gc->frameBuffer.fboManager;
    /* Initialize a linear lookup table for framebuffer object */
    fboManager->maxLinearTableSize = __GL_MAX_FBOBJ_LINEAR_TABLE_SIZE;
    fboManager->linearTableSize = __GL_DEFAULT_FBOBJ_LINEAR_TABLE_SIZE;
    fboManager->linearTable = (GLvoid **)
        (*gc->imports.calloc)(gc, 1, fboManager->linearTableSize * sizeof(GLvoid *));

    fboManager->hashSize = __GL_FBOBJ_HASH_TABLE_SIZE;
    fboManager->hashMask = __GL_FBOBJ_HASH_TABLE_SIZE - 1;
    fboManager->refcount = 1;
    fboManager->deleteObject = (__GLdeleteObjectFunc)__glDeleteFramebufferObject;
    fboManager->immediateInvalid = GL_FALSE;

    /* RBO can be shared across contexts */
    if (gc->shareCtx)
    {
        GL_ASSERT(gc->shareCtx->frameBuffer.rboShared);
        gc->frameBuffer.rboShared = gc->shareCtx->frameBuffer.rboShared;
        gcoOS_LockPLS();
        gc->frameBuffer.rboShared->refcount++;

        /* Allocate VEGL lock */
        if (gcvNULL == gc->frameBuffer.rboShared->lock)
        {
            gc->frameBuffer.rboShared->lock = (*gc->imports.calloc)(gc, 1, sizeof(VEGLLock));
            (*gc->imports.createMutex)(gc->frameBuffer.rboShared->lock);
        }
        gcoOS_UnLockPLS();
    }
    else
    {
        GL_ASSERT(gcvNULL == gc->frameBuffer.rboShared);

        gc->frameBuffer.rboShared = (__GLsharedObjectMachine*)(*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        rboShared = gc->frameBuffer.rboShared;
        /* Initialize a linear lookup table for renderbuffer object */
        rboShared->maxLinearTableSize = __GL_MAX_RBOBJ_LINEAR_TABLE_SIZE;
        rboShared->linearTableSize = __GL_DEFAULT_RBOBJ_LINEAR_TABLE_SIZE;
        rboShared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, rboShared->linearTableSize * sizeof(GLvoid *));

        rboShared->hashSize = __GL_RBOBJ_HASH_TABLE_SIZE;
        rboShared->hashMask = __GL_RBOBJ_HASH_TABLE_SIZE - 1;
        rboShared->refcount = 1;
        rboShared->deleteObject = (__GLdeleteObjectFunc)__glDeleteRenderbufferObject;
        rboShared->immediateInvalid = GL_TRUE;
    }

    /* Default renderbuffer object */
    __glInitRenderbufferObject(gc, &gc->frameBuffer.defaultRBO, 0);

    /* Default framebuffer object */
    __glInitFramebufferObject(gc, &gc->frameBuffer.defaultDrawFBO, 0);
    __glInitFramebufferObject(gc, &gc->frameBuffer.defaultReadFBO, 0);

    /* Initially bind to the default object (name 0) */
    gc->frameBuffer.drawFramebufObj = &gc->frameBuffer.defaultDrawFBO;
    gc->frameBuffer.readFramebufObj = &gc->frameBuffer.defaultReadFBO;
    gc->frameBuffer.boundRenderbufObj = &gc->frameBuffer.defaultRBO;

    __GL_FOOTER();
}

GLvoid __glFreeFramebufferStates(__GLcontext *gc)
{
    __GL_HEADER();
    /* bind RBO to default 0 */
    __glBindRenderbuffer(gc, GL_RENDERBUFFER, 0);

    __glFreeSharedObjectState(gc, gc->frameBuffer.fboManager);
    __glFreeSharedObjectState(gc, gc->frameBuffer.rboShared);

    /* Free default RBO chip objects */
    (*gc->dp.deleteRenderbuffer)(gc, &gc->frameBuffer.defaultRBO);

    __GL_FOOTER();
}

/*
** OpenGL Frame buffer object APIs
*/
GLboolean GL_APIENTRY __gles_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer)
{
    return (gcvNULL != __glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer));
}

GLvoid GL_APIENTRY __gles_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer)
{
    if (target != GL_RENDERBUFFER)
    {
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    __glBindRenderbuffer(gc, target, renderbuffer);
}

GLvoid GL_APIENTRY __gles_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint *renderbuffers)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /*
    ** If a renderbuffer that is being deleted is currently bound,
    ** bind the default renderbuffer to its target.
    */
    for (i = 0; i < n; i++)
    {
        /* Skip default textures */
        if (renderbuffers[i])
        {
            __glDeleteObject(gc, gc->frameBuffer.rboShared, renderbuffers[i]);
        }
    }
OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint *renderbuffers)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gcvNULL == renderbuffers)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gcvNULL != gc->frameBuffer.rboShared);

    start = __glGenerateNames(gc, gc->frameBuffer.rboShared, n);

    for (i = 0; i < n; i++)
    {
        renderbuffers[i] = start + i;
    }

    if (gc->frameBuffer.rboShared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->frameBuffer.rboShared, (start + n));
    }

OnError:
OnExit:
    __GL_FOOTER();
    return;
}

/*
** Establish the data storage format and dimensions of a renderbuffer object's image
*/
GLvoid GL_APIENTRY __gles_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat,
                                              GLsizei width, GLsizei height)
{
    __glRenderbufferStorage(gc, target, 0, internalformat, width, height, GL_FALSE);
}

GLvoid GL_APIENTRY __gles_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GL_HEADER();
    /* Error check */
    if (target != GL_RENDERBUFFER)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (gc->frameBuffer.boundRenderbufObj->name == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
    {
    case GL_RENDERBUFFER_WIDTH:
        *params = gc->frameBuffer.boundRenderbufObj->width;
        break;
    case GL_RENDERBUFFER_HEIGHT:
        *params = gc->frameBuffer.boundRenderbufObj->height;
        break;
    case GL_RENDERBUFFER_SAMPLES:
        *params = gc->frameBuffer.boundRenderbufObj->samplesUsed;
        break;
    case GL_RENDERBUFFER_INTERNAL_FORMAT:
        *params = gc->frameBuffer.boundRenderbufObj->internalFormat;
        break;
    case GL_RENDERBUFFER_RED_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->redSize;
        }
        else
        {
            *params = 0;
        }
        break;
    case GL_RENDERBUFFER_GREEN_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->greenSize;
        }
        else
        {
            *params = 0;
        }
        break;
    case GL_RENDERBUFFER_BLUE_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->blueSize;
        }
        else
        {
            *params = 0;
        }
        break;
    case GL_RENDERBUFFER_ALPHA_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->alphaSize;
        }
        else
        {
            *params = 0;
        }
        break;
    case GL_RENDERBUFFER_DEPTH_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->depthSize;
        }
        else
        {
            *params = 0;
        }
        break;
    case GL_RENDERBUFFER_STENCIL_SIZE:
        if(gc->frameBuffer.boundRenderbufObj->formatInfo)
        {
            *params = gc->frameBuffer.boundRenderbufObj->formatInfo->stencilSize;
        }
        else
        {
            *params = 0;
        }
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return;
}

GLboolean GL_APIENTRY __gles_IsFramebuffer(__GLcontext *gc, GLuint framebuffer)
{
    return (gcvNULL != __glGetObject(gc, gc->frameBuffer.fboManager, framebuffer));
}

GLvoid GL_APIENTRY __gles_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer)
{
    __GL_HEADER();

    if (target != GL_FRAMEBUFFER && target != GL_DRAW_FRAMEBUFFER && target != GL_READ_FRAMEBUFFER)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __glBindFramebuffer(gc, target, framebuffer);

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint *framebuffers)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        /* Skip default textures */
        if (framebuffers[i])
        {
            __glDeleteObject(gc, gc->frameBuffer.fboManager, framebuffers[i]);
        }
    }
OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint *framebuffers)
{
    GLint start, i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gcvNULL == framebuffers)
    {
        __GL_EXIT();
    }

    GL_ASSERT(gcvNULL != gc->frameBuffer.fboManager);

    start = __glGenerateNames(gc, gc->frameBuffer.fboManager, n);

    for (i = 0; i < n; i++)
    {
        framebuffers[i] = start + i;
    }

    if (gc->frameBuffer.fboManager->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->frameBuffer.fboManager, (start + n));
    }

OnError:
OnExit:
    __GL_FOOTER();
    return;
}

GLenum GL_APIENTRY __gles_CheckFramebufferStatus(__GLcontext *gc, GLenum target)
{
    __GLframebufferObject *fbo = gcvNULL;

    __GL_HEADER();

    switch(target)
    {
    case GL_FRAMEBUFFER:
    case GL_DRAW_FRAMEBUFFER:
        fbo = gc->frameBuffer.drawFramebufObj;
        gc->dp.isFramebufferComplete(gc, fbo);
        break;

    case GL_READ_FRAMEBUFFER:
        fbo = gc->frameBuffer.readFramebufObj;
        gc->dp.isFramebufferComplete(gc, fbo);
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
    return fbo ? fbo->checkCode : 0;
}

GLvoid GL_APIENTRY __gles_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment,
                                               GLenum textarget, GLuint texture, GLint level)
{
    __GLtextureObject *texObj;
    GLuint targetIdx = __GL_MAX_TEXTURE_BINDINGS;
    GLuint face = 0;
    GLuint i;
    GLuint numAttachments = 0;
    GLenum attachIndices[2];

    __GL_HEADER();

    /* Parameter error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if (texture != 0)
    {
        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if (!texObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        switch (textarget)
        {
        case GL_TEXTURE_2D:
            targetIdx = __GL_TEXTURE_2D_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
            face = textarget - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            targetIdx = __GL_TEXTURE_2D_MS_INDEX;

            if (level != 0)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            break;

        default:
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }

        if (texObj->targetIndex != targetIdx)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (level >= (GLint)gc->constants.maxNumTextureLevels || level < 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }
    else
    {
        texObj = gcvNULL;
    }

    if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
    {
        numAttachments = 2;
        attachIndices[0] = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        attachIndices[1] = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }
    else
    {
        numAttachments = 1;
        attachIndices[0] = __glMapAttachmentToIndex(attachment);
    }

    for (i = 0; i < numAttachments; ++i)
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndices[i],
                                   texObj, level, face, 0, 0, GL_FALSE, GL_FALSE);
            break;
        case GL_READ_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndices[i],
                                   texObj, level, face, 0, 0, GL_FALSE, GL_FALSE);
            break;
        }
    }
OnError:
    __GL_FOOTER();
    return;
}

/*
** Functions to attach renderbuffer to current framebuffer object
*/
GLvoid GL_APIENTRY __gles_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment,
                                                  GLenum renderbuffertarget, GLuint renderbuffer)
{
    GLuint i;
    __GLrenderbufferObject *renderbufferObj = gcvNULL;
    GLuint numAttachments = 0;
    GLenum attachIndices[2];

    __GL_HEADER();

    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if (renderbuffer != 0)
    {
        renderbufferObj = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer);
        if (!renderbufferObj)
        {
            /* If renderbuffer is neither 0 nor the name of existing renderbuffer object, set error. */
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        else
        {
            if (renderbuffertarget != GL_RENDERBUFFER)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
        }
    }
    else
    {
        renderbufferObj = gcvNULL;
    }

    if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
    {
        numAttachments = 2;
        attachIndices[0] = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        attachIndices[1] = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }
    else
    {
        numAttachments = 1;
        attachIndices[0] = __glMapAttachmentToIndex(attachment);
    }

    for (i = 0; i < numAttachments; ++i)
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            __glFramebufferRenderbuffer(gc, gc->frameBuffer.drawFramebufObj, attachIndices[i], renderbufferObj);
            break;
        case GL_READ_FRAMEBUFFER:
            __glFramebufferRenderbuffer(gc, gc->frameBuffer.readFramebufObj, attachIndices[i], renderbufferObj);
            break;
        }
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment,
                                                              GLenum pname, GLint *params)
{
    __GLframebufferObject *framebufferObj;
    __GLfboAttachPoint *attachPoint;
    __GLformatInfo *formatInfo = gcvNULL;

    __GL_HEADER();

    switch (target)
    {
    case GL_FRAMEBUFFER:
    case GL_DRAW_FRAMEBUFFER:
        framebufferObj = gc->frameBuffer.drawFramebufObj;
        break;

    case GL_READ_FRAMEBUFFER:
        framebufferObj = gc->frameBuffer.readFramebufObj;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (framebufferObj->name == 0)
    {
        if (__GL_API_VERSION_ES20 == gc->apiVersion)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        /*
        ** ES3.0 spec allow query default frame buffer object
        */
        if (attachment == GL_BACK)
        {
            formatInfo = gc->drawablePrivate->rtFormatInfo;
        }
        else if ((attachment == GL_DEPTH) || (attachment == GL_STENCIL))
        {
            formatInfo = gc->drawablePrivate->dsFormatInfo;
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }
        /*
        ** Just pick the first one as they are all same;
        */
        attachPoint = &framebufferObj->attachPoint[0];
    }
    else
    {
        if ((attachment < GL_COLOR_ATTACHMENT0 || attachment > __GL_COLOR_ATTACHMENTn) &&
            (attachment != GL_DEPTH_ATTACHMENT) &&
            (attachment != GL_STENCIL_ATTACHMENT) &&
            (attachment != GL_DEPTH_STENCIL_ATTACHMENT))
        {
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }

        if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
        {
            __GLfboAttachPoint *depthAttachPoint = &framebufferObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];
            __GLfboAttachPoint *stencilAttachPoint = &framebufferObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];

            /* If attachment is DEPTH_STENCIL_ATTACHMENT, and different objects are
            ** bound to the depth and stencil attachment points of target, the query will fail and
            ** generate an INVALID_OPERATION error.
            */
            if ((depthAttachPoint->objType != stencilAttachPoint->objType) ||
                (depthAttachPoint->objName != stencilAttachPoint->objName))
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }

            attachPoint = depthAttachPoint;
            formatInfo = __glGetFramebufferFormatInfo(gc, framebufferObj, GL_DEPTH_ATTACHMENT);
        }
        else
        {
            GLint attachIndex = __glMapAttachmentToIndex(attachment);
            if (-1 == attachIndex)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            attachPoint = &framebufferObj->attachPoint[attachIndex];
            formatInfo = __glGetFramebufferFormatInfo(gc, framebufferObj, attachment);
        }
    }

    /* According to the ES2.0 spec, if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is NONE,
    then querying any other pname will generate INVALID_ENUM */
    if((2 == gc->constants.majorVersion) &&
       (GL_NONE == attachPoint->object) &&
       (GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE != pname))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /*
    ** If no object is bound on this point, only TYPE and NAME can be queried.
    */
    if ((GL_NONE == attachPoint->objType) &&
        (GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE != pname) &&
        (GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME != pname))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (pname)
    {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            *params = formatInfo ? attachPoint->objType : GL_NONE;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            if (framebufferObj->name == 0)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = attachPoint->objName;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            if (GL_TEXTURE != attachPoint->objType)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = attachPoint->level;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT:
            if (GL_TEXTURE != attachPoint->objType)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = attachPoint->layered;
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            if (GL_TEXTURE != attachPoint->objType)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = attachPoint->layer;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT:
            if (GL_TEXTURE != attachPoint->objType)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = (GLint)attachPoint->samples;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            if (GL_TEXTURE != attachPoint->objType)
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }
            *params = attachPoint->cube ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + attachPoint->face : GL_NONE;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
            *params = formatInfo->redSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
            *params = formatInfo->greenSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
            *params = formatInfo->blueSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
            *params = formatInfo->alphaSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
            *params = formatInfo->depthSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
            *params = formatInfo->stencilSize;
            __GL_EXIT();

        case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            /* 3.x spec: This query can not be performed for a combined depth+stencil (GL_DEPTH_STENCIL_ATTACHMENT) attachment,
            since it does not have a single format */
            if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
            else
            {
                *params = formatInfo->category;
            }
            return;

        case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            *params = formatInfo->encoding;
            __GL_EXIT();

        default:
            __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

__GL_INLINE GLboolean __glBlitFramebufferBegin(__GLcontext *gc,
                                               GLint srcX0,
                                               GLint srcY0,
                                               GLint srcX1,
                                               GLint srcY1,
                                               GLint dstX0,
                                               GLint dstY0,
                                               GLint dstX1,
                                               GLint dstY1,
                                               GLbitfield mask,
                                               GLenum filter)
{
    GLboolean ret;
    __GL_HEADER();

    if (gc->flags & __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER)
    {
        ret = GL_FALSE;
    }
    else
    {
        ret = (*gc->dp.blitFramebufferBegin)(gc);
    }

    __GL_FOOTER();
    return ret;
}


__GL_INLINE GLvoid __glBlitFramebufferValidateState(__GLcontext *gc, GLbitfield mask)
{
    GLboolean retVal = (*gc->dp.blitFramebufferValidateState)(gc, mask);

    __GL_HEADER();

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __GL_FOOTER();

    return;
}

__GL_INLINE GLvoid __glBlitFramebufferEnd(__GLcontext *gc)
{
    GLboolean retVal;

    __GL_HEADER();

    retVal = (*gc->dp.blitFramebufferEnd)(gc);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __GL_FOOTER();
    return;
}



GLvoid GL_APIENTRY __gles_BlitFramebuffer(__GLcontext *gc,
                                          GLint srcX0,
                                          GLint srcY0,
                                          GLint srcX1,
                                          GLint srcY1,
                                          GLint dstX0,
                                          GLint dstY0,
                                          GLint dstX1,
                                          GLint dstY1,
                                          GLbitfield mask,
                                          GLenum filter)
{
    __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;
    __GLframebufferObject *drawFBO = gc->frameBuffer.drawFramebufObj;
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    __GLdrawablePrivate *readable = gc->readablePrivate;
    __GLfboAttachPoint *readAttachPoint = gcvNULL;
    __GLfboAttachPoint *drawAttachPoint = gcvNULL;
    __GLformatInfo *readFormatInfo = gcvNULL;
    __GLformatInfo *drawFormatInfo = gcvNULL;
    GLboolean bHaveDrawbuffer = GL_FALSE;
    GLboolean xReverse = GL_FALSE;
    GLboolean yReverse = GL_FALSE;
    GLint i = 0;
    GLuint rAttachPointMask = 0;
    GLuint wAttachPointMask = 0;

    __GL_HEADER();

    /* Arguments check */
    if (mask & ~(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
         __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (filter != GL_LINEAR && filter != GL_NEAREST)
    {
         __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (GL_LINEAR == filter && (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* If either draw or read frame buffer is not complete.
    ** In fact it will also be checked in __glEvaluateDrawableChange, but sometimes
    ** it will be skipped because mask was cleared.
    */
    if (!gc->dp.isFramebufferComplete(gc, drawFBO) ||
        !gc->dp.isFramebufferComplete(gc, readFBO))
    {
        __GL_ERROR_EXIT(GL_INVALID_FRAMEBUFFER_OPERATION);
    }

    if (mask == 0)
    {
        /* Ignore it */
        __GL_EXIT();
    }

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        /* If a buffer is specified in mask and does not exist in both the read and draw
        ** framebuffers, the corresponding bit is silently ignored.
        */
        if (readFBO->name ? GL_NONE == readFBO->readBuffer : GL_NONE == gc->state.raster.readBuffer)
        {
            mask &= ~GL_COLOR_BUFFER_BIT;
        }
        else
        {
            GLboolean srcFixOrFloat = GL_TRUE, dstFixOrFloat = GL_TRUE;
            GLboolean srcInt = GL_FALSE, dstInt = GL_FALSE;
            GLboolean srcUint = GL_FALSE, dstUint = GL_FALSE;

            if (readFBO->name)
            {
                GLint attachIndex = __glMapAttachmentToIndex(readFBO->readBuffer);


                if (attachIndex != -1)
                {
                    readAttachPoint = &readFBO->attachPoint[attachIndex];
                    rAttachPointMask = 0x1 << attachIndex;

                    if ((readFBO->fbIntMask & rAttachPointMask) && GL_LINEAR == filter)
                    {
                        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    }

                    if (0 == readAttachPoint->objName)
                    {
                        mask &= ~GL_COLOR_BUFFER_BIT;
                    }

                    srcFixOrFloat = !(rAttachPointMask & readFBO->fbIntMask) && !(rAttachPointMask & readFBO->fbUIntMask);
                    srcInt        = (rAttachPointMask & readFBO->fbIntMask) && !(rAttachPointMask & readFBO->fbUIntMask);
                    srcUint       = (GLboolean)(rAttachPointMask & readFBO->fbUIntMask);
                }
                else
                {
                    mask &= ~GL_COLOR_BUFFER_BIT;
                }
            }
            else
            {
                if (!readable->rtHandles[0])
                {
                    mask &= ~GL_COLOR_BUFFER_BIT;
                }

                srcFixOrFloat = GL_TRUE;
                srcInt        = GL_FALSE;
                srcUint       = GL_FALSE;
            }

            if (drawFBO->name)
            {
                for (i = 0; i < drawFBO->drawBufferCount; i++)
                {
                    GLint attachIndex = __glMapAttachmentToIndex(drawFBO->drawBuffers[i]);

                    if (-1 == attachIndex)
                    {
                        continue;
                    }

                    drawAttachPoint = &drawFBO->attachPoint[attachIndex];
                    if (drawAttachPoint->objName)
                    {
                        bHaveDrawbuffer = GL_TRUE;

                        /* Surface of named drawFBO must be different than default readFBO */
                        if (readFBO->name && readAttachPoint &&
                            (readAttachPoint->objType == drawAttachPoint->objType) &&
                            (readAttachPoint->objName == drawAttachPoint->objName) &&
                            (readAttachPoint->level   == drawAttachPoint->level) &&
                            (readAttachPoint->face    == drawAttachPoint->face) &&
                            (readAttachPoint->layer   == drawAttachPoint->layer))
                        {
                            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                        }
                    }
                }
            }
            else
            {
                if (drawable->rtHandles[0])
                {
                    bHaveDrawbuffer = GL_TRUE;
                }

                /* Surface of default drawFBO must be different than named readFBO */
                if ((0 == readFBO->name) && (readable->rtHandles[0] == drawable->rtHandles[0]))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }

                dstFixOrFloat = GL_TRUE;
                dstInt        = GL_FALSE;
                dstUint       = GL_FALSE;
            }

            if (!bHaveDrawbuffer)
            {
                mask &= ~GL_COLOR_BUFFER_BIT;
            }

             if (drawFBO->name)
            {
                for (i = 0; i < drawFBO->drawBufferCount; i++)
                {
                    GLint attachIndex = __glMapAttachmentToIndex(drawFBO->drawBuffers[i]);
                    wAttachPointMask = 0x1 << attachIndex;

                    if (-1 == attachIndex)
                    {
                        continue;
                    }

                    dstFixOrFloat = !(wAttachPointMask & drawFBO->fbIntMask) && !(wAttachPointMask & drawFBO->fbUIntMask);
                    dstInt        = (wAttachPointMask & drawFBO->fbIntMask) && !(wAttachPointMask & drawFBO->fbUIntMask);
                    dstUint       = (GLboolean)(wAttachPointMask & drawFBO->fbUIntMask);

                    if ((mask & GL_COLOR_BUFFER_BIT) && (srcFixOrFloat != dstFixOrFloat || srcInt != dstInt || srcUint != dstUint))
                    {
                        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    }

                }
            }

        }
    }

    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        if (readFBO->name)
        {
            readAttachPoint = &readFBO->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];
            if (readAttachPoint->objName)
            {
                readFormatInfo = __glGetFramebufferFormatInfo(gc, readFBO, GL_DEPTH_ATTACHMENT);
            }
            else
            {
                mask &= ~GL_DEPTH_BUFFER_BIT;
            }
        }
        else
        {
            if (readable->depthHandle)
            {
                readFormatInfo = readable->dsFormatInfo;
            }
            else
            {
                mask &= ~GL_DEPTH_BUFFER_BIT;
            }
        }

        if (drawFBO->name)
        {
            drawAttachPoint = &drawFBO->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];
            if (drawAttachPoint->objName)
            {
                /* Surface of named drawFBO must be different than default readFBO */
                if (readFBO->name &&
                    (readAttachPoint->objType == drawAttachPoint->objType) &&
                    (readAttachPoint->objName == drawAttachPoint->objName) &&
                    (readAttachPoint->level   == drawAttachPoint->level) &&
                    (readAttachPoint->face    == drawAttachPoint->face) &&
                    (readAttachPoint->layer   == drawAttachPoint->layer))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }

                drawFormatInfo = __glGetFramebufferFormatInfo(gc, drawFBO, GL_DEPTH_ATTACHMENT);
            }
            else
            {
                mask &= ~GL_DEPTH_BUFFER_BIT;
            }
        }
        else
        {
            if (drawable->depthHandle)
            {
                /* Surface of default drawFBO must be different than named readFBO */
                if ((0 == readFBO->name) && (readable->depthHandle == drawable->depthHandle))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }

                drawFormatInfo = drawable->dsFormatInfo;
            }
            else
            {
                mask &= ~GL_DEPTH_BUFFER_BIT;
            }
        }

        if ((mask & GL_DEPTH_BUFFER_BIT) && (drawFormatInfo != gcvNULL) &&
            (readFormatInfo != gcvNULL) && (drawFormatInfo->glFormat != readFormatInfo->glFormat))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    if (mask & GL_STENCIL_BUFFER_BIT)
    {
        if (readFBO->name)
        {
            readAttachPoint = &readFBO->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];
            if (readAttachPoint->objName)
            {
                readFormatInfo = __glGetFramebufferFormatInfo(gc, readFBO, GL_STENCIL_ATTACHMENT);
            }
            else
            {
                mask &= ~GL_STENCIL_BUFFER_BIT;
            }
        }
        else
        {
            if (readable->stencilHandle)
            {
                readFormatInfo = readable->dsFormatInfo;
            }
            else
            {
                mask &= ~GL_STENCIL_BUFFER_BIT;
            }
        }

        if (drawFBO->name)
        {
            drawAttachPoint = &drawFBO->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];
            if (drawAttachPoint->objName)
            {
                /* Surface of named drawFBO must be different than default readFBO */
                if (readFBO->name &&
                    (readAttachPoint->objType == drawAttachPoint->objType) &&
                    (readAttachPoint->objName == drawAttachPoint->objName) &&
                    (readAttachPoint->level   == drawAttachPoint->level) &&
                    (readAttachPoint->face    == drawAttachPoint->face) &&
                    (readAttachPoint->layer   == drawAttachPoint->layer))
                {
                    __GL_ERROR_EXIT(GL_STENCIL_BUFFER_BIT);
                }

                drawFormatInfo = __glGetFramebufferFormatInfo(gc, drawFBO, GL_STENCIL_ATTACHMENT);
            }
            else
            {
                mask &= ~GL_STENCIL_BUFFER_BIT;
            }

        }
        else
        {
            if (drawable->stencilHandle)
            {
                /* Surface of default drawFBO must be different than named readFBO */
                if ((0 == readFBO->name) && (readable->stencilHandle == drawable->stencilHandle))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }

                drawFormatInfo = drawable->dsFormatInfo;
            }
            else
            {
                mask &= ~GL_STENCIL_BUFFER_BIT;
            }
        }

        if ((mask & GL_STENCIL_BUFFER_BIT) && (drawFormatInfo != gcvNULL) &&
            (readFormatInfo != gcvNULL) && (drawFormatInfo->glFormat != readFormatInfo->glFormat))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    /* If draw samples are not zero, set error and return */
    if (drawFBO->name ? drawFBO->fbSamples > 1 : drawable->modes.samples > 1)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* If read samples are not zero, bounds and format need match */
    if (readFBO->name ? readFBO->fbSamples > 1 : readable->modes.samples > 1)
    {
        /* If bounds region not equal, set error and return */
        if ((srcX0 != dstX0) || (srcY0 != dstY0) ||
            (srcX1 != dstX1) || (srcY1 != dstY1))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (mask & GL_COLOR_BUFFER_BIT)
        {
            /* If format not equal, set error and return */
            readFormatInfo = readFBO->name
                           ? __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer)
                           : readable->rtFormatInfo;

            if (drawFBO->name)
            {
                for (i = 0; i < drawFBO->drawBufferCount; i++)
                {
                    drawFormatInfo = __glGetFramebufferFormatInfo(gc, drawFBO, drawFBO->drawBuffers[i]);
                    if ((readFormatInfo != gcvNULL) && (drawFormatInfo != gcvNULL) &&
                        (drawFormatInfo->glFormat != readFormatInfo->glFormat))
                    {
                        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    }
                }
            }
            else
            {
                drawFormatInfo = drawable->rtFormatInfo;
                if ((readFormatInfo != gcvNULL) && (drawFormatInfo != gcvNULL) &&
                    (drawFormatInfo->glFormat != readFormatInfo->glFormat))
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }
    }

    /* Check whether need to reverse the copy rect */
    if (srcX0 > srcX1)
    {
        __GL_SWAP(GLint, srcX0, srcX1);
        xReverse = !xReverse;
    }
    if (dstX0 > dstX1)
    {
        __GL_SWAP(GLint, dstX0, dstX1);
        xReverse = !xReverse;
    }
    if (srcY0 > srcY1)
    {
        __GL_SWAP(GLint, srcY0, srcY1);
        yReverse = !yReverse;
    }
    if (dstY0 > dstY1)
    {
        __GL_SWAP(GLint, dstY0, dstY1);
        yReverse = !yReverse;
    }

    /* If either src or dst rect are 0, skip the blit. */
    if ((srcX1 == srcX0 && srcY1 == srcY0) || (dstX1 == dstX0 && dstY1 == dstY0))
    {
        __GL_EXIT();
    }

    /**/
    if ((srcX1 - srcX0 == dstX1 - dstX0) &&
        (srcY1 - srcY0 == dstY1 - dstY0))
    {
        filter = GL_NEAREST;
    }

    if (mask)
    {
        __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

        if (GL_TRUE == __glBlitFramebufferBegin(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter))
        {
            __glBlitFramebufferValidateState(gc, mask);

            (*gc->dp.blitFramebuffer)(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, xReverse, yReverse, filter);

            __glBlitFramebufferEnd(gc);
        }
    }

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment,
                                                  GLuint texture, GLint level, GLint layer)
{
    GLuint i;
    __GLtextureObject *texObj;
    GLuint numAttachments = 0;
    GLenum attachIndices[2];

    __GL_HEADER();

    /* Parameter error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if (texture != 0)
    {
        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if (!texObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        if ((level >= (GLint)gc->constants.maxNumTextureLevels) ||(level < 0))
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }

        if (layer < 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }

        switch (texObj->targetIndex)
        {
        case __GL_TEXTURE_3D_INDEX:
            if (layer >= (GLint)gc->constants.maxTextureSize)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            break;
        case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
            if (0 != level)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            /* fall through */
        case __GL_TEXTURE_2D_ARRAY_INDEX:
        case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
            if (layer >= (GLint)gc->constants.maxTextureArraySize)
            {
                __GL_ERROR_EXIT(GL_INVALID_VALUE);
            }
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        texObj = gcvNULL;
    }

    if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
    {
        numAttachments = 2;
        attachIndices[0] = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        attachIndices[1] = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }
    else
    {
        numAttachments = 1;
        attachIndices[0] = __glMapAttachmentToIndex(attachment);
    }

    for (i = 0; i < numAttachments; ++i)
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndices[i],
                                   texObj, level, 0, layer, 0, GL_FALSE, GL_FALSE);
            break;
        case GL_READ_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndices[i],
                                   texObj, level, 0, layer, 0, GL_FALSE, GL_FALSE);
            break;
        }
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_RenderbufferStorageMultisample(__GLcontext *gc,
                                                         GLenum target,
                                                         GLsizei samples,
                                                         GLenum internalformat,
                                                         GLsizei width,
                                                         GLsizei height)
{
    __glRenderbufferStorage(gc, target, samples, internalformat, width, height, GL_FALSE);
}

GLvoid __glInvalidateFramebuffer(__GLcontext *gc,
                                 GLenum target,
                                 GLsizei numAttachments,
                                 const GLenum* attachments,
                                 GLint x,
                                 GLint y,
                                 GLsizei width,
                                 GLsizei height)
{
    __GLframebufferObject *framebufferObj = gcvNULL;
    GLsizei i;

    __GL_HEADER();

    switch (target)
    {
    case GL_FRAMEBUFFER:
    case GL_DRAW_FRAMEBUFFER:
        framebufferObj = gc->frameBuffer.drawFramebufObj;
        break;

    case GL_READ_FRAMEBUFFER:
        framebufferObj = gc->frameBuffer.readFramebufObj;
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (numAttachments < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (0 == numAttachments || gcvNULL == attachments)
    {
        /* Ignore it*/
        __GL_EXIT();
    }

    GL_ASSERT(framebufferObj);

    if (framebufferObj->name)
    {
        __GLfboAttachPoint *attachPoints[__GL_MAX_ATTACHMENTS];

        __GL_MEMZERO(attachPoints, __GL_MAX_ATTACHMENTS * sizeof(__GLfboAttachPoint*));
        /* Go through all specified attachments first to remove duplicated ones */
        for (i = 0; i < numAttachments; ++i)
        {
            if (attachments[i] == GL_DEPTH_STENCIL_ATTACHMENT)
            {
                attachPoints[__GL_DEPTH_ATTACHMENT_POINT_INDEX]   = &framebufferObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];
                attachPoints[__GL_STENCIL_ATTACHMENT_POINT_INDEX] = &framebufferObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];
            }
            else
            {
                GLint attachIdx = __glMapAttachmentToIndex(attachments[i]);
                if (-1 == attachIdx)
                {
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
                attachPoints[attachIdx] = &framebufferObj->attachPoint[attachIdx];
            }
        }

        if (!gc->dp.isFramebufferComplete(gc, framebufferObj))
        {
            /* Ignore if the framebuffer object is not complete */
            __GL_EXIT();
        }

        for (i = 0; i < __GL_MAX_ATTACHMENTS; ++i)
        {
            /* Ignore if app doesn't specify the attachment, or the attachments does not exist. */
            if (attachPoints[i] && attachPoints[i]->objName)
            {
                /* Hard to get FBO size in GLcore layer, set it to maximum and chip layer will calc the intersection */
                gc->dp.invalidateFramebuffer(gc, framebufferObj, attachPoints[i], x, y, width, height);
            }
        }
    }
    else
    {
        for (i = 0; i < numAttachments; ++i)
        {
            switch (attachments[i])
            {
            case GL_COLOR:
            case GL_DEPTH:
            case GL_STENCIL:
                break;
            default:
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }

            if (!gc->dp.isFramebufferComplete(gc, framebufferObj))
            {
                /* Ignore if the framebuffer object is not complete */
                __GL_EXIT();
            }

            gc->dp.invalidateDrawable(gc, x, y, width, height);
        }
    }

OnError:

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    /* Inconvenient to get FBO size in GLcore layer, set -1 and let chip layer do it */
    __glInvalidateFramebuffer(gc, target, numAttachments, attachments, 0, 0, (GLsizei)(-1), (GLsizei)(-1));
}

GLvoid GL_APIENTRY __gles_InvalidateSubFramebuffer(__GLcontext *gc,
                                                   GLenum target,
                                                   GLsizei numAttachments,
                                                   const GLenum* attachments,
                                                   GLint x,
                                                   GLint y,
                                                   GLsizei width,
                                                   GLsizei height)
{
    __GL_HEADER();

    if (width < 0 || height < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    __glInvalidateFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);

OnError:

    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GLformatInfo *formatInfo = gcvNULL;

    __GL_HEADER();

    if (!params || 0 == bufSize)
    {
        __GL_EXIT();
    }

    if (bufSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if ((GL_RENDERBUFFER != target)           &&
        (GL_TEXTURE_2D_MULTISAMPLE != target) &&
        (GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES != target))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (GL_SAMPLES != pname && GL_NUM_SAMPLE_COUNTS != pname)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    formatInfo = __glGetFormatInfo(internalformat);

    if (!formatInfo->renderable)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    switch (pname)
    {
    case GL_NUM_SAMPLE_COUNTS:
        (*gc->dp.queryFormatInfo)(gc, formatInfo->drvFormat, params, gcvNULL, 0);
        break;
    case GL_SAMPLES:
        (*gc->dp.queryFormatInfo)(gc, formatInfo->drvFormat, gcvNULL, params, bufSize);
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:

OnExit:

    __GL_FOOTER();
    return;
}

#if GL_EXT_discard_framebuffer
GLvoid GL_APIENTRY __gles_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    /* Inconvenient to get FBO size in GLcore layer, set -1 and let chip layer do it */
    __glInvalidateFramebuffer(gc, target, numAttachments, attachments, 0, 0, (GLsizei)(-1), (GLsizei)(-1));
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GL_APIENTRY __gles_RenderbufferStorageMultisampleEXT(
    __GLcontext *gc,
    GLenum target,
    GLsizei samples,
    GLenum internalformat,
    GLsizei width,
    GLsizei height)
{
    __glRenderbufferStorage(gc, target, samples, internalformat, width, height, GL_TRUE);
}

GLvoid GL_APIENTRY __gles_FramebufferTexture2DMultisampleEXT(
    __GLcontext *gc,
    GLenum target,
    GLenum attachment,
    GLenum textarget,
    GLuint texture,
    GLint level,
    GLsizei samples
    )
{
    __GLtextureObject *texObj;
    GLuint targetIdx = __GL_MAX_TEXTURE_BINDINGS;
    GLuint face = 0;
    GLuint i;
    GLuint numAttachments = 0;
    GLenum attachIndices[2];

    __GL_HEADER();

    /* Parameter error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if (texture != 0)
    {
        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if (!texObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        switch (textarget)
        {
          case GL_TEXTURE_2D:
              targetIdx = __GL_TEXTURE_2D_INDEX;
              break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
              targetIdx = __GL_TEXTURE_CUBEMAP_INDEX;
              face = textarget - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
              break;

          default:
              __GL_ERROR_EXIT(GL_INVALID_ENUM);
        }

        if (texObj->targetIndex != targetIdx)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (level >= (GLint)gc->constants.maxNumTextureLevels || level < 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
    }
    else
    {
        texObj = gcvNULL;
    }

    if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
    {
        numAttachments = 2;
        attachIndices[0] = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        attachIndices[1] = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }
    else
    {
        numAttachments = 1;
        attachIndices[0] = __glMapAttachmentToIndex(attachment);
    }

    for (i = 0; i < numAttachments; ++i)
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndices[i],
                                   texObj, level, face, 0, samples, GL_FALSE, GL_TRUE);
            break;
        case GL_READ_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndices[i],
                                   texObj, level, face, 0, samples, GL_FALSE, GL_TRUE);
            break;
        }
    }
OnError:
    __GL_FOOTER();
    return;
}
#endif

GLvoid GL_APIENTRY __gles_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GLframebufferObject *framebufferObj = gcvNULL;

    __GL_HEADER();

    switch (target)
    {
    case GL_DRAW_FRAMEBUFFER:
    case GL_FRAMEBUFFER:
        if (DRAW_FRAMEBUFFER_BINDING_NAME == 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        framebufferObj = gc->frameBuffer.drawFramebufObj;
        break;

    case GL_READ_FRAMEBUFFER:
        if (READ_FRAMEBUFFER_BINDING_NAME == 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        framebufferObj = gc->frameBuffer.readFramebufObj;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

    switch (pname)
    {
    case GL_FRAMEBUFFER_DEFAULT_WIDTH:
        if ((param > (GLint)gc->constants.maxTextureSize) || (param < 0))
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        framebufferObj->defaultWidth = param;
        break;
    case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
        if ((param > (GLint)gc->constants.maxTextureSize) || (param < 0))
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        framebufferObj->defaultHeight = param;
        break;

    case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
        if ((param > (GLint)gc->constants.maxSamples) || (param < 0))
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        framebufferObj->defaultSamples = param;
        framebufferObj->defaultSamplesUsed = param;
        break;
    case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
        framebufferObj->defaultFixedSampleLoc = (GLboolean)param;
        break;

    case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            framebufferObj->defaultLayers = param;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

    __GL_FRAMEBUFFER_COMPLETE_DIRTY(framebufferObj);

OnError:
    __GL_FOOTER();
    return;

}

GLvoid GL_APIENTRY __gles_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GLframebufferObject *framebufferObj = gcvNULL;

    __GL_HEADER();

    switch (target)
    {
    case GL_DRAW_FRAMEBUFFER:
    case GL_FRAMEBUFFER:
        if (DRAW_FRAMEBUFFER_BINDING_NAME == 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        framebufferObj = gc->frameBuffer.drawFramebufObj;
        break;

    case GL_READ_FRAMEBUFFER:
        if (READ_FRAMEBUFFER_BINDING_NAME == 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        framebufferObj = gc->frameBuffer.readFramebufObj;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

    switch (pname)
    {
    case GL_FRAMEBUFFER_DEFAULT_WIDTH:
        *params = framebufferObj->defaultWidth;
        break;

    case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
        *params = framebufferObj->defaultHeight;
        break;

    case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
        *params = framebufferObj->defaultSamplesUsed;
        break;
    case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
        *params = (GLint)framebufferObj->defaultFixedSampleLoc;
        break;
    case GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            *params = framebufferObj->defaultLayers;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

OnError:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __gles_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    GLuint i;
    __GLtextureObject *texObj;
    GLuint numAttachments = 0;
    GLenum attachIndices[2];
    GLboolean layered = GL_TRUE;

    __GL_HEADER();

    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if (texture != 0)
    {
        GLint maxLevels;
        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if (!texObj)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        maxLevels = texObj->immutable ? texObj->immutableLevels : texObj->mipMaxLevel;
        maxLevels = __GL_MIN(maxLevels, (GLint)gc->constants.maxNumTextureLevels);
        if ((level >= maxLevels) || (level < 0))
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }

        switch (texObj->targetIndex)
        {
        case __GL_TEXTURE_3D_INDEX:
        case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
        case __GL_TEXTURE_2D_ARRAY_INDEX:
        case __GL_TEXTURE_CUBEMAP_INDEX:
        case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
            break;

        case __GL_TEXTURE_2D_INDEX:
        case __GL_TEXTURE_2D_MS_INDEX:
        case __GL_TEXTURE_EXTERNAL_INDEX:
            layered = GL_FALSE;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    else
    {
        texObj = gcvNULL;
    }

    if (GL_DEPTH_STENCIL_ATTACHMENT == attachment)
    {
        numAttachments = 2;
        attachIndices[0] = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        attachIndices[1] = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }
    else
    {
        numAttachments = 1;
        attachIndices[0] = __glMapAttachmentToIndex(attachment);
    }

    for (i = 0; i < numAttachments; ++i)
    {
        switch (target)
        {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndices[i],
                                   texObj, level, 0, 0, 0, layered, GL_FALSE);
            break;
        case GL_READ_FRAMEBUFFER:
            __glFramebufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndices[i],
                                   texObj, level, 0, 0, 0, layered, GL_FALSE);
            break;
        }
    }
OnError:
    __GL_FOOTER();
    return;
}


