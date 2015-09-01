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
#include "gc_gl_names_inline.c"
#include "gc_gl_debug.h"
#include "gl/gl_device.h"
#include "dri/viv_lock.h"

#if GL_EXT_framebuffer_object
extern GLvoid __glGenerateMipmaps(__GLcontext *gc, __GLtextureObject *tex, GLint face, GLint baseLevel);
extern GLboolean __glIsIntegerInternalFormat(GLenum internalFormat);
extern GLboolean __glIsFloatInternalFormat(GLenum internalFormat);

GLvoid __glFramebufferRenderbuffer(__GLcontext *gc, __GLframebufferObject *framebufferObj, GLint attachIndex, __GLrenderbufferObject *renderbufferObj);
GLvoid __glBindFramebuffer(__GLcontext *gc, GLenum target, GLuint name);
GLvoid __glBindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer);

/*
** Function to check if the object (locate by objType and objName) is nolonger used by the framebuffer object,
** If so, remove framebuffer obj from it's userlist.
*/

extern const __GLdeviceFormatInfo __glDevfmtInfo[__GL_DEVFMT_MAX];
GLvoid __glRemoveFramebufferAsImageUser(__GLcontext *gc, __GLframebufferObject *framebuffer, GLenum objType, GLuint objName)
{
    __GLrenderbufferObject *Rbo = NULL;
    __GLtextureObject *tex = NULL;
    GLboolean bUsed = GL_FALSE;
    GLuint i;

    if(objType==GL_NONE || objName == 0)
        return;

    switch(objType)
    {
        case GL_RENDERBUFFER_EXT:

            for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if(framebuffer->attachPoint[i].objName == objName
                    && framebuffer->attachPoint[i].objectType == GL_RENDERBUFFER_EXT )
                {
                        bUsed = GL_TRUE;
                        break;
                }
            }

            /*
            ** If this rbo image is nolonger used by any attachpoint of framebuffer,
            ** Remove framebuffer frome uselist of this rbo.
            */
            if(!bUsed)
            {
                Rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, objName);
                __glRemoveImageUser( gc, &Rbo->fboList, framebuffer);

                Rbo->bindCount--;
                if (Rbo->bindCount == 0 && (Rbo->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteObject(gc, gc->frameBuffer.rboShared, Rbo->name);
                }
            }
            break;

        case GL_TEXTURE:
            for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if(framebuffer->attachPoint[i].objName == objName
                    && framebuffer->attachPoint[i].objectType == GL_TEXTURE)
                {
                        bUsed = GL_TRUE;
                        break;
                }
            }

            if(!bUsed)
            {
                tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, objName);
                __glRemoveImageUser( gc, &tex->fboList, framebuffer);

                tex->bindCount--;
                if (tex->bindCount == 0 && (tex->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteObject(gc, gc->texture.shared, tex->name);
                }
            }

            break;
    }
}


/*
** Check cubemap texture for cube completeness.
*/
GLboolean __glIsCubeBaselevelConsistent(__GLcontext *gc, __GLtextureObject *tex)
{
    __GLmipMapLevel *level = NULL;
    GLint width, height, border;
    GLint base, face;
    GLenum requestedFormat;

    if (tex->targetIndex != __GL_TEXTURE_CUBEMAP_INDEX)
        return GL_FALSE;

    base = tex->params.baseLevel;
    border = tex->faceMipmap[0][base].border;
    width = tex->faceMipmap[0][base].width2;
    height = tex->faceMipmap[0][base].height2;
    requestedFormat = tex->faceMipmap[0][base].requestedFormat;

    /* If each dimension of the level[base] array is positive. */
    if ((tex->faceMipmap[0][base].width == 0) ||
        (tex->faceMipmap[0][base].height == 0) ||
        (tex->faceMipmap[0][base].depth == 0))
    {
        return GL_FALSE;
    }

    /* Check for CubeMap base level consistency for all six faces */
    {
        /* If Cubemap baselevel has square dimension */
        if (width != height) {
            return GL_FALSE;
        }

        /* If the six baselevel images have identical dimension, internalformat */
        for (face = 1; face < 6; face++)
        {
            level = &tex->faceMipmap[face][base];
            if (requestedFormat != level->requestedFormat ||
                border != level->border ||
                width != level->width2 ||
                height != level->height2)
            {
                return GL_FALSE;
            }
        }
    }

    return GL_TRUE;
}


GLvoid __glFramebufferResetAttachpoint(__GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    GLboolean drawFbo)
{
    /* Set all state of "attachment" point to default state. */

    __GLfboAttachPoint *attachState = &fbo->attachPoint[attachIndex];

    attachState->objectType     = GL_NONE;
    attachState->objName        = 0;
    attachState->level          = 0;
    attachState->face           = 0;
    attachState->zoffset        = 0;
    attachState->layered        = GL_FALSE;
}

GLvoid __glInitRenderbufferObject(__GLcontext *gc, __GLrenderbufferObject *renderbuffer, GLuint name)
{
    renderbuffer->bindCount = 0;

    renderbuffer->internalFormat = GL_RGBA;
    renderbuffer->name = name;
    renderbuffer->deviceFormatInfo = NULL;
}

GLboolean __glDeleteRenderbufferObject(__GLcontext *gc, GLvoid *obj)
{
    GLuint i;
    __GLrenderbufferObject *renderbuffer = obj;
    __GLimageUser *imageUserList = renderbuffer->fboList;
    __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
    __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

    /* dirty all fbo this rbo attached to */
    while(imageUserList)
    {
        /*
        ** Spec.: If a renderbuffer object is deleted while its image is attached to
        **        one or more attachment points in the currently bound framebuffer,
        **        then it is as if FramebufferRenderbufferEXT() had been called, with
        **        a <renderbuffer> of 0, for each attachment point to which this image
        **        was attached in the currently bound framebuffer.
        */
        if(imageUserList->imageUser == drawFbo)
        {
            for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
            {
                if(drawFbo->attachPoint[i].objName == renderbuffer->name &&
                   drawFbo->attachPoint[i].objectType == GL_RENDERBUFFER_EXT)
                {
                    __glFramebufferRenderbuffer(gc, drawFbo, i, NULL);
                }
            }
        }

        if(readFbo != drawFbo)
        {
            if(imageUserList->imageUser == readFbo)
            {
                for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
                {
                    if(readFbo->attachPoint[i].objName == renderbuffer->name &&
                       readFbo->attachPoint[i].objectType == GL_RENDERBUFFER_EXT)
                    {
                       __glFramebufferRenderbuffer(gc, readFbo, i, NULL);
                    }
                }
            }
        }

        imageUserList = imageUserList->next;
    }

    /* Do not delete the rbObj if there are other contexts bound to it. */
    if (renderbuffer->bindCount != 0)
    {
        /* Set the flag to indicate the object is marked for delete */
        renderbuffer->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    /* The object is truly deleted here, so delete the object name from name list. */
    __glDeleteNamesFrList(gc, gc->frameBuffer.rboShared, renderbuffer->name, 1);

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

    framebuffer->bindCount = 0;

    framebuffer->name = name;
    framebuffer->flag = 0;
    framebuffer->seqNumber = 0;
    framebuffer->fbInteger = GL_FALSE;

    /* Init per attachment state*/
    for(i = 0; i < __GL_MAX_ATTACHMENTS; i++){
        framebuffer->attachPoint[i].objectType = GL_NONE;
        framebuffer->attachPoint[i].objName = 0;
        framebuffer->attachPoint[i].face = 0;
        framebuffer->attachPoint[i].zoffset = 0;
        framebuffer->attachPoint[i].level = 0;
        framebuffer->attachPoint[i].layered = GL_FALSE;
    }

    /*Init per framebuffer state*/
    framebuffer->drawBuffer[0]   = GL_COLOR_ATTACHMENT0_EXT;
    framebuffer->drawBufferCount = 1;
    for(i = 1; i < __GL_MAX_COLOR_ATTACHMENTS; i++){
        framebuffer->drawBuffer[i] = GL_NONE;
    }

    framebuffer->readBuffer = GL_COLOR_ATTACHMENT0_EXT;

    if(name == 0)
    {
        framebuffer->flag |= (__GL_FRAMEBUFFER_IS_CHECKED | __GL_FRAMEBUFFER_IS_COMPLETENESS);
        framebuffer->checkCode = GL_FRAMEBUFFER_COMPLETE_EXT;
    }
}

GLboolean __glDeleteFramebufferObject(__GLcontext *gc, GLvoid *obj)
{
    GLuint i;
    __GLframebufferObject *framebuffer = obj;

    /*
    ** If the framebuffer object that is being deleted is currently bound to gc,
    ** unbind it from gc and bind the default obj 0 to gc.
    */
    if(framebuffer->name == gc->frameBuffer.drawFramebufObj->name)
    {
        __glBindFramebuffer(gc, GL_DRAW_FRAMEBUFFER_EXT, 0);
    }

    if(framebuffer->name == gc->frameBuffer.readFramebufObj->name)
    {
        __glBindFramebuffer(gc, GL_READ_FRAMEBUFFER_EXT, 0);
    }


    /* Do not delete the framebufferObj if there are other contexts bound to it. */
    if (framebuffer->bindCount != 0)
    {
        /* Set the flag to indicate the object is marked for delete */
        framebuffer->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    /* The object is truly deleted here, so delete the object name from name list. */
    __glDeleteNamesFrList(gc, gc->frameBuffer.fboShared, framebuffer->name, 1);

    /* Remove fbo from image userlist of attaching image */
    for(i = 0; i < __GL_MAX_ATTACHMENTS; i++){
        GLenum objType = framebuffer->attachPoint[i].objectType;
        GLuint objName = framebuffer->attachPoint[i].objName;

        if(objType==GL_NONE || objName == 0)
            continue;

        __glFramebufferResetAttachpoint(gc, framebuffer, i, GL_TRUE);
        __glRemoveFramebufferAsImageUser(gc, framebuffer, objType, objName);
    }

    (*gc->imports.free)(gc, framebuffer);

    return GL_TRUE;
}

GLvoid __glFreeFramebufferStates(__GLcontext *gc)
{
    __glFreeSharedObjectState(gc, gc->frameBuffer.fboShared);

    __glFreeSharedObjectState(gc, gc->frameBuffer.rboShared);
}

GLvoid __glInitFramebufferStates(__GLcontext *gc)
{
    __GLsharedObjectMachine *fboShared = NULL, *rboShared = NULL;

    if (gc->frameBuffer.fboShared == NULL) {
        gc->frameBuffer.fboShared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        fboShared = gc->frameBuffer.fboShared;
        /* Initialize a linear lookup table for framebuffer object */
        fboShared->maxLinearTableSize = __GL_MAX_FBOBJ_LINEAR_TABLE_SIZE;
        fboShared->linearTableSize = __GL_DEFAULT_FBOBJ_LINEAR_TABLE_SIZE;
        fboShared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, fboShared->linearTableSize * sizeof(GLvoid *) );

        fboShared->hashSize = __GL_FBOBJ_HASH_TABLE_SIZE;
        fboShared->hashMask = __GL_FBOBJ_HASH_TABLE_SIZE - 1;
        fboShared->refcount = 1;
        fboShared->deleteObject = __glDeleteFramebufferObject;
    }

    if (gc->frameBuffer.rboShared == NULL) {
        gc->frameBuffer.rboShared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        rboShared = gc->frameBuffer.rboShared;
        /* Initialize a linear lookup table for renderbuffer object */
        rboShared->maxLinearTableSize = __GL_MAX_RBOBJ_LINEAR_TABLE_SIZE;
        rboShared->linearTableSize = __GL_DEFAULT_RBOBJ_LINEAR_TABLE_SIZE;
        rboShared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, rboShared->linearTableSize * sizeof(GLvoid *) );

        rboShared->hashSize = __GL_RBOBJ_HASH_TABLE_SIZE;
        rboShared->hashMask = __GL_RBOBJ_HASH_TABLE_SIZE - 1;
        rboShared->refcount = 1;
        rboShared->deleteObject = __glDeleteRenderbufferObject;
    }

    /*Default renderbuffer object*/
    __glInitRenderbufferObject(gc, &gc->frameBuffer.defaultRBO, 0);

    /*Default framebuffer object*/
    __glInitFramebufferObject(gc, &gc->frameBuffer.defaultFBO, 0);

    /* initializely bind to the default object (name 0)*/
    gc->frameBuffer.drawFramebufObj = &gc->frameBuffer.defaultFBO;
    gc->frameBuffer.readFramebufObj = &gc->frameBuffer.defaultFBO;
    gc->frameBuffer.boundRenderbufObj = &gc->frameBuffer.defaultRBO;

}

/*
** Used to share Fbuffer objects between two different contexts.
*/
GLvoid __glShareFrameBufferObjects(__GLcontext *dst, __GLcontext *src)
{
    if (dst->frameBuffer.fboShared)
    {
        __glFreeSharedObjectState(dst, dst->frameBuffer.fboShared);
    }

    dst->frameBuffer.fboShared = src->frameBuffer.fboShared;
    dst->frameBuffer.fboShared->refcount++;

    if (dst->frameBuffer.rboShared)
    {
        __glFreeSharedObjectState(dst, dst->frameBuffer.rboShared);
    }

    dst->frameBuffer.rboShared = src->frameBuffer.rboShared;
    dst->frameBuffer.rboShared->refcount++;
}


#define FRAMEBUFFER_CHECK_TARGET_ATTACHMENT()                                               \
    switch(target)                                                                          \
    {                                                                                       \
    case GL_FRAMEBUFFER_EXT:                                                                \
        if((DRAW_FRAMEBUFFER_BINDING_NAME == 0) || (READ_FRAMEBUFFER_BINDING_NAME == 0))    \
        {                                                                                   \
            __glSetError(GL_INVALID_OPERATION);                                             \
            return;                                                                         \
        }                                                                                   \
        break;                                                                              \
    case GL_DRAW_FRAMEBUFFER_EXT:                                                           \
        if(DRAW_FRAMEBUFFER_BINDING_NAME == 0)                                              \
        {                                                                                   \
            __glSetError(GL_INVALID_OPERATION);                                             \
            return;                                                                         \
        }                                                                                   \
        break;                                                                              \
    case GL_READ_FRAMEBUFFER_EXT:                                                           \
        if(READ_FRAMEBUFFER_BINDING_NAME == 0)                                              \
        {                                                                                   \
            __glSetError(GL_INVALID_OPERATION);                                             \
            return;                                                                         \
        }                                                                                   \
        break;                                                                              \
    default:                                                                                \
        __glSetError(GL_INVALID_ENUM);                                                      \
        return;                                                                             \
    }                                                                                       \
    if((attachment < GL_COLOR_ATTACHMENT0_EXT) && (attachment > __GL_COLOR_ATTACHMENTn_EXT) \
    && (attachment != GL_DEPTH_ATTACHMENT_EXT)                                              \
    && (attachment != GL_STENCIL_ATTACHMENT_EXT))                                           \
    {                                                                                       \
        __glSetError(GL_INVALID_ENUM);                                                      \
        return;                                                                             \
    }

/*
** OpenGL Frame buffer object APIs
*/
GLboolean APIENTRY __glim_IsRenderbufferEXT(GLuint renderbuffer)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(GL_FALSE);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsRenderbufferEXT", DT_GLuint, renderbuffer, DT_GLnull);
#endif

    return __glIsNameDefined(gc, gc->frameBuffer.rboShared, renderbuffer);
}

GLvoid __glBindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer)
{
    __GLrenderbufferObject *renderbufferObj = NULL, *boundObj = NULL;

    if (gc->frameBuffer.boundRenderbufObj->name == renderbuffer){
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    GL_ASSERT(NULL != gc->frameBuffer.rboShared);

    if (renderbuffer == 0) {
        /* Retrieve the default object in __GLcontext.
        */
        renderbufferObj = &gc->frameBuffer.defaultRBO;
        GL_ASSERT(renderbufferObj->name == 0);
    }
    else {
        /*
        ** Retrieve the object from the "gc->frameBuffer.shared" structure.
        */
        renderbufferObj = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer);
    }

    if (NULL == renderbufferObj) {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new renderbuffer object and initialize it.
        */
        renderbufferObj = (__GLrenderbufferObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLrenderbufferObject) );
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
    boundObj = gc->frameBuffer.boundRenderbufObj;
    gc->frameBuffer.boundRenderbufObj = renderbufferObj;

    /* Remove gc from boundObj->userList */
    if (boundObj->name != 0) {
        boundObj->bindCount--;

        if (boundObj->bindCount == 0 && (boundObj->flag & __GL_OBJECT_IS_DELETED)) {
            __glDeleteObject(gc, gc->frameBuffer.rboShared, boundObj->name);
        }
    }

    /* Add gc to renderbufferObj->userList if renderbuffer is not default renderbuffer object.
    */
    if (renderbufferObj->name != 0) {
        renderbufferObj->bindCount++;
    }

    /* Call the dp interface */
    (*gc->dp.bindRenderbuffer)(gc, renderbufferObj);

}

GLvoid APIENTRY __glim_BindRenderbufferEXT(GLenum target, GLuint renderbuffer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindRenderbufferEXT", DT_GLenum, target, DT_GLuint, renderbuffer, DT_GLnull);
#endif

    if(target != GL_RENDERBUFFER_EXT){
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __glBindRenderbuffer(gc, target, renderbuffer);
}

GLvoid APIENTRY __glim_DeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers)
{
    GLint i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteRenderbuffersEXT", DT_GLsizei, n, DT_GLuint_ptr, renderbuffers, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*
    ** If a renderbuffer that is being deleted is currently bound,
    ** bind the default renderbuffer to its target.
    */
    for (i = 0; i < n; i++){
        /* skip default textures */
        if (renderbuffers[i]){
            __glDeleteObject(gc, gc->frameBuffer.rboShared, renderbuffers[i]);
        }
    }

}

GLvoid APIENTRY __glim_GenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers)
{
    GLint start, i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenRenderbuffersEXT", DT_GLsizei, n, DT_GLuint_ptr, renderbuffers, DT_GLnull);
#endif

    if (n < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (NULL == renderbuffers)
        return;

    GL_ASSERT(NULL != gc->frameBuffer.rboShared);

    start = __glGenerateNames(gc, gc->frameBuffer.rboShared, n);

    for (i = 0; i < n; i++){
        renderbuffers[i] = start + i;
    }

    if (gc->frameBuffer.rboShared->linearTable){
        __glCheckLinearTableSize(gc, gc->frameBuffer.rboShared, (start + n));
    }
}

GLvoid __glRenderbufferStorage(__GLcontext* gc,
    GLenum target,
    GLsizei samples,
    GLenum internalformat,
    GLsizei width,
    GLsizei height)
{
    const __GLdeviceConstants *devContants = &gc->constants;
    __GLrenderbufferObject *curRbo = NULL;
    __GLdeviceFormat chosenFormat;
    GLuint i;

    if(target != GL_RENDERBUFFER_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(samples > (GLint)devContants->maxSamples)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if((width > (GLint)devContants->maxRenderBufferWidth) || (height > (GLint)devContants->maxRenderBufferWidth))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(width <=0 || height <= 0)
        return;

    switch(internalformat){
        /* color-renderable format */
        case GL_RGB:
        case GL_RGBA:
        case GL_FLOAT_R_NV:
        case GL_FLOAT_RG_NV:
        case GL_FLOAT_RGB_NV:
        case GL_FLOAT_RGBA_NV:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
            /* depth-renderable */
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            /* stencil-renderable */
        case GL_STENCIL_INDEX:
        case GL_STENCIL_INDEX1_EXT:
        case GL_STENCIL_INDEX4_EXT:
        case GL_STENCIL_INDEX8_EXT:
        case GL_STENCIL_INDEX16_EXT:
            break;
            /*GL_EXT_texture_integer*/
        case GL_RGBA32UI_EXT:
        case GL_RGB32UI_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGBA8UI_EXT:
        case GL_RGB8UI_EXT:
        case GL_RGBA32I_EXT:
        case GL_RGB32I_EXT:
        case GL_RGBA16I_EXT:
        case GL_RGB16I_EXT:
        case GL_RGBA8I_EXT:
        case GL_RGB8I_EXT:
            if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
            {
                __glSetError(GL_INVALID_ENUM);
                return;
            }
            break;
        case GL_RGB9_E5_EXT:
            if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
            {
                __glSetError(GL_INVALID_ENUM);
                return;
            }
            break;
        case GL_R11F_G11F_B10F_EXT:
            if(!__glExtension[INDEX_EXT_packed_float].bEnabled)
            {
                __glSetError(GL_INVALID_ENUM);
                return;
            }
            break;

        case GL_RGBA32F_ARB:
        case GL_RGBA16F_ARB:
        case GL_RGB32F_ARB:
            /*cann't support downsampling for D2, but support on D3
            **only RGBA16F support Fog on D3, others not
            */
            if(!__glExtension[INDEX_ARB_texture_float].bEnabled)
            {
                __glSetError(GL_INVALID_ENUM);
                return;
            }
            break;
        case GL_RGB16F_ARB:
        case GL_ALPHA32F_ARB:
        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_INTENSITY16F_ARB:
            /*cann't be RT format for D2/D3*/

        default:
            GL_ASSERT(0);
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalformat,GL_FALSE,samples);

    curRbo = gc->frameBuffer.boundRenderbufObj;

    /*Redundant check*/
    if( (curRbo->width == width) &&
        (curRbo->height == height) &&
        (curRbo->internalFormat == internalformat) &&
        (curRbo->samples == samples) )
    {
        return;
    }

    /* Flush vertex buffer if state change */
    __GL_VERTEX_BUFFER_FLUSH(gc);

    curRbo->width           = width;
    curRbo->height          = height;
    curRbo->internalFormat  = internalformat;
    curRbo->samples         = samples;
    curRbo->deviceFormatInfo = &__glDevfmtInfo[chosenFormat];

    /* Call the dp interface: must map app request internal format to implement format */
    if(!(*gc->dp.renderbufferStorage)(gc, curRbo)){
        __glSetError(GL_OUT_OF_MEMORY);
        return;
    }

    /* Dirty all fbo this renderbuffer obj is attached to. */
    if(DRAW_FRAMEBUFFER_BINDING_NAME != 0){
        __GLframebufferObject *curFbo = gc->frameBuffer.drawFramebufObj;

        for(i = 0; i < __GL_MAX_ATTACHMENTS; i++){

            if((curFbo->attachPoint[i].objectType == GL_RENDERBUFFER_EXT)
                && (curRbo->name == curFbo->attachPoint[i].objName) ){

                    FRAMEBUFFER_COMPLETENESS_DIRTY(curFbo);
            }
        }
    }

    if(READ_FRAMEBUFFER_BINDING_NAME != 0){
        __GLframebufferObject *curFbo = gc->frameBuffer.readFramebufObj;

        for(i = 0; i < __GL_MAX_ATTACHMENTS; i++){

            if((curFbo->attachPoint[i].objectType == GL_RENDERBUFFER_EXT)
                && (curRbo->name == curFbo->attachPoint[i].objName) ){

                    FRAMEBUFFER_COMPLETENESS_DIRTY(curFbo);

            }
        }
    }

}


/*
** Establish the data storage format and dimensions of a renderbuffer object's image
*/
GLvoid APIENTRY __glim_RenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_RenderbufferStorageEXT", DT_GLenum, target, DT_GLenum, internalformat, DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif

    __glRenderbufferStorage(gc, target, 0, internalformat, width, height);
}

GLvoid APIENTRY __glim_GetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetRenderbufferParameterivEXT", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    /* Error check */
    if(target != GL_RENDERBUFFER_EXT){
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if(gc->frameBuffer.boundRenderbufObj->name == 0){
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    switch(pname){
        case GL_RENDERBUFFER_WIDTH_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->width;
            break;
        case GL_RENDERBUFFER_HEIGHT_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->height;
            break;
        case GL_RENDERBUFFER_SAMPLES_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->samples;
            break;
        case GL_RENDERBUFFER_INTERNAL_FORMAT_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->internalFormat;
            break;
        case GL_RENDERBUFFER_RED_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->redSize;
            break;
        case GL_RENDERBUFFER_GREEN_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->greenSize;
            break;
        case GL_RENDERBUFFER_BLUE_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->blueSize;
            break;
        case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->alphaSize;
            break;
        case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->depthSize;
            break;
        case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
            *params = gc->frameBuffer.boundRenderbufObj->deviceFormatInfo->stencilSize;
            break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

}

GLboolean APIENTRY __glim_IsFramebufferEXT(GLuint framebuffer)
{
    __GL_SETUP_NOT_IN_BEGIN_RET(GL_FALSE);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_IsFramebufferEXT", DT_GLuint, framebuffer, DT_GLnull);
#endif

    return __glIsNameDefined(gc, gc->frameBuffer.fboShared, framebuffer);
}

GLvoid __glBindFramebuffer(__GLcontext *gc, GLenum target, GLuint name)
{
    __GLframebufferObject *framebufferObj = NULL;

    __GLframebufferObject *prevDrawObj = NULL;
    __GLframebufferObject *prevReadObj = NULL;

    __GLframebufferObject *curDrawObj = NULL;
    __GLframebufferObject *curReadObj = NULL;

    /* Redundant check: If the name is current name, return. */
    switch(target){
    case GL_FRAMEBUFFER_EXT :
        if ( (gc->frameBuffer.drawFramebufObj->name == name) &&
             (gc->frameBuffer.readFramebufObj->name == name) )
        {
            return;
        }
        break;

    case GL_DRAW_FRAMEBUFFER_EXT :
        if (gc->frameBuffer.drawFramebufObj->name == name)
        {
            return;
        }
        break;

    case GL_READ_FRAMEBUFFER_EXT :
        if (gc->frameBuffer.readFramebufObj->name == name)
        {
            return;
        }
        break;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    GL_ASSERT(NULL != gc->frameBuffer.fboShared);

    if (name == 0) {
        /* Retrieve the default object in __GLcontext. */
        framebufferObj = &gc->frameBuffer.defaultFBO;
        GL_ASSERT(framebufferObj->name == 0);
        /* clear invalid flag */
        gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE;
    }
    else {
        /*
        ** Retrieve the object from the "gc->frameBuffer.shared" structure.
        */
        framebufferObj = (__GLframebufferObject *)__glGetObject(gc, gc->frameBuffer.fboShared, name);
    }

    if (NULL == framebufferObj) {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new frameBuffer object and initialize it.
        */
        framebufferObj = (__GLframebufferObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLframebufferObject) );
        __glInitFramebufferObject(gc, framebufferObj, name);

        /* Add this frameBuffer object to the "gc->frameBuffer.shared" structure. */
        __glAddObject(gc, gc->frameBuffer.fboShared, name, framebufferObj);

        /* Mark the name "frameBuffer" used in the frameBuffer nameArray. */
        __glMarkNameUsed(gc, gc->frameBuffer.fboShared, name);
    }

    /* Add gc to framebufferObj->userList if framebufferObj is not default frambuffer object. */
    if (framebufferObj->name != 0) {
        framebufferObj->bindCount++;
    }


    prevDrawObj = gc->frameBuffer.drawFramebufObj;
    prevReadObj = gc->frameBuffer.readFramebufObj;

    switch(target){
    case GL_FRAMEBUFFER_EXT :
        {
            curDrawObj = framebufferObj;
            curReadObj = framebufferObj;
        }
        break;

    case GL_DRAW_FRAMEBUFFER_EXT:
        {
            curDrawObj = framebufferObj;
            curReadObj = prevReadObj;
        }
        break;

    case GL_READ_FRAMEBUFFER_EXT:
        {
            curDrawObj = prevDrawObj;
            curReadObj = framebufferObj;
        }
        break;
    }

    /*
    ** Install new frame buffer object and notify dp render target changed
    */
    LINUX_LOCK_FRAMEBUFFER(gc);

    if(prevDrawObj != curDrawObj)
    {
        gc->frameBuffer.drawFramebufObj = curDrawObj;

        (*gc->dp.bindDrawFramebuffer)(gc, prevDrawObj, curDrawObj);

        gc->frameBuffer.drawFBSeqNumber = gc->frameBuffer.drawFramebufObj->seqNumber;

        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_DRAWBUFFER_BIT);
    }

    if(prevReadObj != curReadObj)
    {
        gc->frameBuffer.readFramebufObj = curReadObj;

        (*gc->dp.bindReadFramebuffer)(gc, prevReadObj, curReadObj);

        gc->frameBuffer.readFBSeqNumber = gc->frameBuffer.readFramebufObj->seqNumber;

        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_READBUFFER_BIT);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*
     ** Remove gc from user list of prev draw obj, and delete old obj if there is no more context
     ** bound to the object and the object is marked for delete already
     */
    if(prevDrawObj == prevReadObj)
    {
        if((prevDrawObj != curDrawObj) && (prevDrawObj != curReadObj))
        {
            if( (prevDrawObj->name != 0) && (prevDrawObj != curReadObj) )
            {
                prevDrawObj->bindCount--;

                if (prevDrawObj->bindCount == 0 && (prevDrawObj->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteObject(gc, gc->frameBuffer.fboShared, prevDrawObj->name);
                }
            }
        }
    }
    else
    {
        if((prevDrawObj != curDrawObj) && (prevDrawObj != curReadObj))
        {
            if(prevDrawObj->name != 0)
            {
                prevDrawObj->bindCount--;

                if (prevDrawObj->bindCount == 0 && (prevDrawObj->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteObject(gc, gc->frameBuffer.fboShared, prevDrawObj->name);
                }
            }
        }

        if((prevReadObj != curDrawObj) && (prevReadObj != curReadObj))
        {
            if(prevReadObj->name != 0)
            {
                prevReadObj->bindCount--;

                if (prevReadObj->bindCount == 0 && (prevReadObj->flag & __GL_OBJECT_IS_DELETED))
                {
                    __glDeleteObject(gc, gc->frameBuffer.fboShared, prevReadObj->name);
                }
            }
        }

    }
}

GLvoid APIENTRY __glim_BindFramebufferEXT(GLenum target, GLuint framebuffer)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_BindFramebufferEXT", DT_GLenum, target, DT_GLuint, framebuffer, DT_GLnull);
#endif

    if( (target != GL_FRAMEBUFFER_EXT) &&
        (target != GL_DRAW_FRAMEBUFFER_EXT) &&
        (target != GL_READ_FRAMEBUFFER_EXT) )
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __glBindFramebuffer(gc, target, framebuffer);
}

GLvoid APIENTRY __glim_DeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers)
{
    GLint i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_DeleteFramebuffersEXT", DT_GLsizei, n, DT_GLuint_ptr, framebuffers, DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*
    ** If a framebuffer that is being deleted is currently bound,
    ** bind the default framebuffer to its target.
    */
    for (i = 0; i < n; i++){
        /* skip default textures */
        if (framebuffers[i]) {
            __glDeleteObject(gc, gc->frameBuffer.fboShared, framebuffers[i]);
        }
    }
}

GLvoid APIENTRY __glim_GenFramebuffersEXT(GLsizei n, GLuint *framebuffers)
{
    GLint start, i;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenFramebuffersEXT", DT_GLsizei, n, DT_GLuint_ptr, framebuffers, DT_GLnull);
#endif

    if (NULL == framebuffers)
        return;

    if (n < 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    GL_ASSERT(NULL != gc->frameBuffer.fboShared);

    start = __glGenerateNames(gc, gc->frameBuffer.fboShared, n);

    for (i = 0; i < n; i++){
        framebuffers[i] = start + i;
    }

    if (gc->frameBuffer.fboShared->linearTable){
        __glCheckLinearTableSize(gc, gc->frameBuffer.fboShared, (start + n));
    }
}

GLenum APIENTRY __glim_CheckFramebufferStatusEXT(GLenum target)
{
    GLenum retCode = 0;

    __GL_SETUP_NOT_IN_BEGIN_RET(0);

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CheckFramebufferStatusEXT", DT_GLenum, target, DT_GLnull);
#endif

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        {
            if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj))
            {
                retCode = gc->frameBuffer.drawFramebufObj->checkCode;
            }
            else
            {
                gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj);
                retCode = gc->frameBuffer.readFramebufObj->checkCode;
            }
        }
        break;

    case GL_DRAW_FRAMEBUFFER_EXT:
        {
            gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj);
            retCode = gc->frameBuffer.drawFramebufObj->checkCode;
        }
        break;

    case GL_READ_FRAMEBUFFER_EXT:
        {
            gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj);
            retCode = gc->frameBuffer.readFramebufObj->checkCode;
        }
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        break;
    }

    return retCode;
}


GLvoid __glFrameBufferTexture(__GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLint zoffset,
    GLboolean layered)
{
    __GLfboAttachPoint      *attachPoint;
    GLenum                  boundObjType; /* Temps to save current attach state.*/
    GLuint                  boundName;

    /* Check parameters */
    GL_ASSERT(attachIndex >= 0 && attachIndex < __GL_MAX_ATTACHMENTS);

    attachPoint  = &framebufferObj->attachPoint[attachIndex];
    boundObjType = attachPoint->objectType;
    boundName    = attachPoint->objName;

    /* Bind again? */
    if (boundObjType == GL_TEXTURE && texObj && texObj->name == boundName
            && attachPoint->face == face && attachPoint->level == level &&
                attachPoint->zoffset == zoffset && attachPoint->layered == layered)
    {
        /* Just return */
        return;
    }

    /* Detach image */
    __glFramebufferResetAttachpoint(gc, framebufferObj, attachIndex, GL_TRUE);

    if(texObj != NULL)
    {
        attachPoint->objectType = GL_TEXTURE;
        attachPoint->objName    = texObj->name;
        attachPoint->level      = level;
        attachPoint->face       = face;
        attachPoint->zoffset    = zoffset;
        attachPoint->layered    = layered;

        /* Increase bindCount */
        if (texObj->name != 0)
        {
            texObj->bindCount++;
        }

        /* Add fbo to texture obj's image userlist. */
        __glAddImageUser(gc, &texObj->fboList, framebufferObj, NULL);
    }

    /* If there is previously attached obj, remove fbo from it's owner list. */
    if(boundObjType != GL_NONE)
    {
        __glRemoveFramebufferAsImageUser(gc, framebufferObj, boundObjType, boundName);
    }

    /* Dirty this framebuffer object */
    FRAMEBUFFER_COMPLETENESS_DIRTY(framebufferObj);

    /* Call dp function */
    gc->dp.frameBufferTexture(gc, framebufferObj, attachIndex, texObj, level, face, zoffset, layered);
}

/*
** Functions to attach texture image to current framebuffer object
*/
GLvoid APIENTRY __glim_FramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GLtextureObject *texObj;
    GLint attachIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTexture1DEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLenum, textarget, DT_GLuint, texture, DT_GLint, level, DT_GLnull);
#endif

    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0){
        if(textarget != GL_TEXTURE_1D){
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        if(texObj->targetIndex != __GL_TEXTURE_1D_INDEX){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
            __glSetError(GL_INVALID_VALUE);
            return;
        }
    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, 0, GL_FALSE);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, 0, GL_FALSE);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, 0, GL_FALSE);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, 0, GL_FALSE);
        break;
    }
}

GLvoid APIENTRY __glim_FramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GLtextureObject *texObj;
    GLint attachIndex;
    GLuint targetIdx, face = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTexture2DEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLenum, textarget, DT_GLuint, texture, DT_GLint, level, DT_GLnull);
#endif

    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0){

        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        switch (textarget) {
          case GL_TEXTURE_2D:
              targetIdx = __GL_TEXTURE_2D_INDEX;
              break;

          case GL_TEXTURE_RECTANGLE_ARB:
              targetIdx = __GL_TEXTURE_RECTANGLE_INDEX;
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
              __glSetError(GL_INVALID_ENUM);
              return;
        }

        if(texObj->targetIndex != targetIdx){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        {
            if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
                __glSetError(GL_INVALID_VALUE);
                return;
            }
        }
    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        break;
    }
}

GLvoid APIENTRY __glim_FramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GLtextureObject *texObj;
    GLint attachIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTexture3DEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLenum, textarget, DT_GLuint, texture, DT_GLint, level, DT_GLint, zoffset, DT_GLnull);
#endif

    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0){
        if(textarget != GL_TEXTURE_3D){
            __glSetError(GL_INVALID_ENUM);
            return;
        }

        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        if(texObj->targetIndex != __GL_TEXTURE_3D_INDEX){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        /* Check level */
        if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
            __glSetError(GL_INVALID_VALUE);
            return;
        }

        /* Check zoffset */
        if(zoffset > (GLint)gc->constants.maxTextureSize){
            __glSetError(GL_INVALID_VALUE);
            return;
        }
    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, zoffset, GL_FALSE);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, zoffset, GL_FALSE);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, zoffset, GL_FALSE);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, zoffset, GL_FALSE);
        break;
    }
}

/*
** name : name of the renderbuffer object to be attached.
*/
GLvoid __glFramebufferRenderbuffer(__GLcontext *gc,
    __GLframebufferObject   *framebufferObj,
    GLint attachIndex,
    __GLrenderbufferObject *renderbufferObj)
{
    __GLfboAttachPoint      *attachPoint;
    GLenum                  boundObjType; /* Temps to save current attach state.*/
    GLuint                  boundName;

    GL_ASSERT(attachIndex >= 0 && attachIndex < __GL_MAX_ATTACHMENTS);

    attachPoint  = &framebufferObj->attachPoint[attachIndex];
    boundObjType = attachPoint->objectType;
    boundName    = attachPoint->objName;

    /* Detach image */
    __glFramebufferResetAttachpoint(gc, framebufferObj, attachIndex, GL_TRUE);

    if(renderbufferObj != NULL)
    {
        /* Attach new image*/
        attachPoint->objectType = GL_RENDERBUFFER_EXT ;
        attachPoint->objName = renderbufferObj->name;

        if(renderbufferObj->name != 0)
        {
            renderbufferObj->bindCount++;
        }

        /* Add fbo to renderbuffer obj's image userlist. */
        __glAddImageUser(gc, &renderbufferObj->fboList, framebufferObj, NULL);
    }

    /* If there is previously attached obj, remove fbo from it's owner list. */
    if(boundObjType != GL_NONE)
    {
        __glRemoveFramebufferAsImageUser(gc, framebufferObj, boundObjType, boundName);
    }

    /* Dirty this framebuffer object */
    FRAMEBUFFER_COMPLETENESS_DIRTY(framebufferObj);

    gc->dp.framebufferRenderbuffer(gc, framebufferObj, attachIndex, renderbufferObj);
}

/*
** Functions to attach renderbuffer to current framebuffer object
*/
GLvoid APIENTRY __glim_FramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GLrenderbufferObject *renderbufferObj = NULL;
    GLint attachIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferRenderbufferEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLenum, renderbuffertarget, DT_GLuint, renderbuffer, DT_GLnull);
#endif

    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    __GL_VERTEX_BUFFER_FLUSH(gc);

    if(renderbuffer != 0){
        renderbufferObj = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, renderbuffer);
        if(!renderbufferObj){
            /* If renderbuffer is neither 0 nor the name of existing renderbuffer object, set error. */
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        else{
            if(renderbuffertarget != GL_RENDERBUFFER_EXT){
                __glSetError(GL_INVALID_ENUM);
                return;
            }
        }
    }
    else{
        /* renderbuffer == 0, renderbuffertarget check is ignored. */
    }

    attachIndex = __glMapAttachmentToIndex(attachment);


    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        __glFramebufferRenderbuffer(gc, gc->frameBuffer.drawFramebufObj, attachIndex, renderbufferObj);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFramebufferRenderbuffer(gc, gc->frameBuffer.readFramebufObj, attachIndex, renderbufferObj);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFramebufferRenderbuffer(gc, gc->frameBuffer.drawFramebufObj, attachIndex, renderbufferObj);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFramebufferRenderbuffer(gc, gc->frameBuffer.readFramebufObj, attachIndex, renderbufferObj);
        break;
    }
}

GLvoid APIENTRY __glim_GetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    __GLframebufferObject *framebufferObj;
    __GLfboAttachPoint *attachPoint;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetFramebufferAttachmentParameterivEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
    case GL_DRAW_FRAMEBUFFER_EXT:
        framebufferObj = gc->frameBuffer.drawFramebufObj;
        break;

    case GL_READ_FRAMEBUFFER_EXT:
        framebufferObj = gc->frameBuffer.readFramebufObj;
        break;
    }

    attachPoint = &framebufferObj->attachPoint[__glMapAttachmentToIndex(attachment)];
    switch(pname){
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT:
            *params = attachPoint->objectType;
            return;

        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT:
            *params = attachPoint->objName;
            return;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT:
            *params = attachPoint->level;
            return;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT:
            *params = GL_TEXTURE_CUBE_MAP_POSITIVE_X + attachPoint->face;
            return;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT:
            *params = attachPoint->zoffset;
            return;

#if GL_EXT_geometry_shader4
        case GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT:
            *params = attachPoint->layered;
            return;
#endif
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }
}


GLvoid APIENTRY __glim_GenerateMipmapEXT(GLenum target)
{
    GLuint i, activeUnit, face;
    __GLtextureObject *tex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GenerateMipmapEXT", DT_GLenum, target, DT_GLnull);
#endif


    activeUnit = gc->state.texture.activeTexIndex;
    switch(target)
    {
    case GL_TEXTURE_1D:
        face = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];
        break;
    case GL_TEXTURE_2D:
        face = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_RECTANGLE_ARB:
        face = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP:
        face = 6;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        /* Check if this cubmap is cube complete */
        if(!__glIsCubeBaselevelConsistent(gc, tex)){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    case GL_TEXTURE_3D:
        face = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
        break;

    case GL_TEXTURE_1D_ARRAY_EXT:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
        face = tex->faceMipmap[0][tex->params.baseLevel].arrays;
        break;
    case GL_TEXTURE_2D_ARRAY_EXT:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        face = tex->faceMipmap[0][tex->params.baseLevel].arrays;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    LINUX_LOCK_FRAMEBUFFER( gc );

    for(i = 0; i < face; i++)
    {
        __glGenerateMipmaps(gc, tex, i, tex->params.baseLevel);
    }

    LINUX_UNLOCK_FRAMEBUFFER( gc );
}

/*
** Function to validate framebuffer change before any render action, and update device pipe if necessary.
** Currently handle only render to FBO.
*/
GLvoid __glEvaluateFramebufferChange(__GLcontext *gc)
{
    __GLframebufferObject *drawFbo;
    __GLframebufferObject *readFbo;

    if((DRAW_FRAMEBUFFER_BINDING_NAME == 0) && (READ_FRAMEBUFFER_BINDING_NAME == 0))
        return;

    drawFbo = gc->frameBuffer.drawFramebufObj;
    readFbo = gc->frameBuffer.readFramebufObj;

    /* Check completeness */
    if(!gc->dp.isFramebufferComplete(gc, drawFbo) || !gc->dp.isFramebufferComplete(gc, readFbo))
    {
        __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
        gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE;
    }
    else
    {
        gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE;
    }

    if (gc->frameBuffer.drawFramebufObj->fbInteger && gc->modes.rgbMode)
    {
        if (!gc->shaderProgram.fragShaderEnable)
        {
            __glSetError(GL_INVALID_OPERATION);
        }
    }

    /* Framebuffer changed, update state to dp */
    if(gc->frameBuffer.drawFBSeqNumber != drawFbo->seqNumber)
    {
        /* Notify dp rendertarget changed */
        (*gc->dp.bindDrawFramebuffer)(gc, drawFbo, drawFbo);
        gc->frameBuffer.drawFBSeqNumber = drawFbo->seqNumber;
    }

    if(gc->frameBuffer.readFBSeqNumber != readFbo->seqNumber)
    {
        /* Notify dp rendertarget changed */
        (*gc->dp.bindReadFramebuffer)(gc, readFbo, readFbo);
        gc->frameBuffer.readFBSeqNumber = readFbo->seqNumber;
    }

}

#if GL_EXT_framebuffer_blit
GLvoid APIENTRY __glim_BlitFramebufferEXT(GLint srcX0,
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
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
    {
        dbgLogFullApi("__glim_BlitFramebufferEXT",
            DT_GLint, srcX0, DT_GLint, srcY0, DT_GLint, srcX1, DT_GLint, srcY1,
            DT_GLint, dstX0, DT_GLint, dstY0, DT_GLint, dstX1, DT_GLint, dstY1,
            DT_GLbitfield, mask, DT_GLenum, filter, DT_GLnull);
    }
#endif

    /* If either draw or read frame buffer is not complete, set error and return */
    if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj) ||
       !gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj) )
    {
        __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
        return;
    }

    /* If copy depth or stencil while filter is not nearest, set error and return */
    if(mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        if(filter != GL_NEAREST)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    /* If both read samples and draw samples are not zero and don't match, set error and return */
    if( gc->frameBuffer.readFramebufObj->fbSamples &&
        gc->frameBuffer.drawFramebufObj->fbSamples &&
        (gc->frameBuffer.readFramebufObj->fbSamples != gc->frameBuffer.drawFramebufObj->fbSamples) )
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /*
    ** If draw frame buffer is equal to read frame buffer and src rect is equal to dst rect,
    ** skip this copy.
    */
    if(gc->frameBuffer.drawFramebufObj == gc->frameBuffer.readFramebufObj)
    {
        if( (srcX0 == dstX0) && (srcX1 == dstX1) &&
            (srcY0 == dstY0) && (srcY1 == dstY1) )
        {
            return;
        }
    }

    /* Get the latest frame buffer information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /*
    ** Evaluate drawable and frame buffer change first.
    */
    __glEvaluateAttribDrawableChange(gc);

    /* Let dp to do real blit operation */
    GL_ASSERT(gc->dp.blitFramebuffer);
    gc->dp.blitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

#if GL_EXT_framebuffer_multisample
GLvoid APIENTRY __glim_RenderbufferStorageMultisampleEXT(GLenum target,
    GLsizei samples,
    GLenum internalformat,
    GLsizei width,
    GLsizei height)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_RenderbufferStorageMultisampleEXT",
            DT_GLenum, target, DT_GLsizei, samples, DT_GLenum, internalformat,
            DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif

    __glRenderbufferStorage(gc, target, samples, internalformat, width, height);
}
#endif /* GL_EXT_framebuffer_multisample */




#endif /* GL_EXT_framebuffer_blit */

#if GL_EXT_geometry_shader4
GLvoid APIENTRY __glim_FramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GLtextureObject *texObj;
    GLint attachIndex;
    GLboolean layered = GL_FALSE;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTextureEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLuint, texture, DT_GLint, level, DT_GLnull);
#endif

    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0){

        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        /* Check level */
        if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
            __glSetError(GL_INVALID_VALUE);
            return;
        }

        switch(texObj->targetIndex)
        {
        /* can't be buffer texture */
        case __GL_TEXTURE_BUFFER_INDEX:
            __glSetError(GL_INVALID_OPERATION);
            return;
        /* set layered flag for 3D, cube, 1d array, 2d array texture */
        case __GL_TEXTURE_3D_INDEX:
        case __GL_TEXTURE_CUBEMAP_INDEX:
        case __GL_TEXTURE_1D_ARRAY_INDEX:
        case __GL_TEXTURE_2D_ARRAY_INDEX:
            layered = GL_TRUE;
            break;
        }

    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, 0, layered);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, 0, layered);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, 0, 0, layered);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, 0, 0, layered);
        break;
    }
}

GLvoid APIENTRY __glim_FramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GLtextureObject *texObj;
    GLint attachIndex;
    GLuint face = 0, zoffset = 0;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTextureLayerEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLuint, texture, DT_GLint, level, DT_GLint, layer, DT_GLnull);
#endif

    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0){

        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        if(layer < 0){
            __glSetError(GL_INVALID_VALUE);
            return;
        }

        /* Check level */
        if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
            __glSetError(GL_INVALID_VALUE);
            return;
        }

        switch(texObj->targetIndex)
        {
        case __GL_TEXTURE_3D_INDEX:
            face = 0;
            zoffset = layer;
            break;
        case __GL_TEXTURE_1D_ARRAY_INDEX:
        case __GL_TEXTURE_2D_ARRAY_INDEX:
            face = layer;
            zoffset = 0;
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }


    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, zoffset, GL_FALSE);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, zoffset, GL_FALSE);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, zoffset, GL_FALSE);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, zoffset, GL_FALSE);
        break;
    }
}

GLvoid APIENTRY __glim_FramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
    __GLtextureObject *texObj;
    GLint attachIndex;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_FramebufferTextureFaceEXT", DT_GLenum, target, DT_GLenum, attachment, DT_GLuint, texture, DT_GLint, level, DT_GLenum, face, DT_GLnull);
#endif


    /* Param/error check */
    FRAMEBUFFER_CHECK_TARGET_ATTACHMENT();

    if(texture != 0)
    {
        /* Tex object must exist */
        texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);
        if(!texObj)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        switch (face)
        {
          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
              face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;
          default:
              __glSetError(GL_INVALID_ENUM);
              return;
        }


        if(texObj->targetIndex != __GL_TEXTURE_CUBEMAP_INDEX){
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        /* check level */
        if((level > (GLint)gc->constants.maxNumTextureLevels) ||(level < 0)){
            __glSetError(GL_INVALID_VALUE);
            return;
        }
    }
    else
    {
        texObj = NULL;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    attachIndex = __glMapAttachmentToIndex(attachment);

    switch(target)
    {
    case GL_FRAMEBUFFER_EXT:
        /* Attach into both draw and read frame buffer */
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);

        if (gc->frameBuffer.drawFramebufObj != gc->frameBuffer.readFramebufObj)
        {
            __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        }
        break;
    case GL_DRAW_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.drawFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        break;
    case GL_READ_FRAMEBUFFER_EXT:
        __glFrameBufferTexture(gc, gc->frameBuffer.readFramebufObj, attachIndex, texObj, level, face, 0, GL_FALSE);
        break;
    }
}
#endif /* GL_EXT_geometry_shader4 */

#endif /* GL_EXT_framebuffer_object */

