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


#ifndef __gc_gl_fbo_h_
#define __gc_gl_fbo_h_

#define __GL_MAX_FBOBJ_LINEAR_TABLE_SIZE        1024
#define __GL_DEFAULT_FBOBJ_LINEAR_TABLE_SIZE    256
#define __GL_FBOBJ_HASH_TABLE_SIZE              1024

#define __GL_MAX_RBOBJ_LINEAR_TABLE_SIZE        1024
#define __GL_DEFAULT_RBOBJ_LINEAR_TABLE_SIZE    256
#define __GL_RBOBJ_HASH_TABLE_SIZE              1024

/* The enum of max supported color attachment point */
#define __GL_COLOR_ATTACHMENTn_EXT  (GL_COLOR_ATTACHMENT0_EXT+__GL_MAX_COLOR_ATTACHMENTS-1)

/**/
#define __GL_DEPTH_ATTACHMENT_POINT_INDEX (__GL_MAX_COLOR_ATTACHMENTS)
#define __GL_STENCIL_ATTACHMENT_POINT_INDEX (__GL_MAX_COLOR_ATTACHMENTS + 1 )

/*
Bit masks for framebuffer obj completeness check flag
Note: Bit 0~7 are reserved for generic object management.
*/
#define __GL_FRAMEBUFFER_IS_CHECKED             (1<<8)
#define __GL_FRAMEBUFFER_IS_COMPLETENESS        (1<<9)

/*macro to enable/disable  consistency check*/
#define FRAMEBUFFER_COMPLETENESS_DIRTY(fbo) \
    (fbo)->flag &= ~(__GL_FRAMEBUFFER_IS_CHECKED | __GL_FRAMEBUFFER_IS_COMPLETENESS);\
    (fbo)->seqNumber++;


typedef struct __GLrenderbufferFormatInfoRec
{
    GLuint redMask;
    GLuint greenMask;
    GLuint blueMask;
    GLuint alphaMask;
    GLuint depthMask;
    GLuint stencilMask;

    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint depthSize;
    GLint stencilSize;
    GLenum choosenFormat;

    GLint bitsPerPixel;

}__GLrenderbufferFormatInfo;

/*
** Renderbuffer object structure.
*/
typedef struct __GLrenderbufferObjectRec
{
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;

    /* List of FBO this renderbuffer object attached to
    */
    __GLimageUser *fboList;

    /*state per renderbuffer object
    */
    GLuint name;
    GLuint targetIndex;
    GLint  width;
    GLint  height;
    GLenum internalFormat;  /* Requested internal format */
    GLint  samples; /* samples per pixel, 0 means no multisample */
    GLint  samplesUsed; /*the actual number of samples allocated for the renderbuffer image is implementation dependent*/

    /* Internal flag for generic object management
    */
    GLbitfield flag;

    /*
    **Implement dependent states. *Size is the actual resolutions. */
    GLuint bufferType;
    const __GLdeviceFormatInfo * deviceFormatInfo;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object. pointer to __GLdpRenderBufferInfo
    */
    GLvoid *privateData;

} __GLrenderbufferObject;

/*
** Framebuffer object structure.
*/
typedef struct __GLfboAttachPointRec
{
    GLenum        objectType;           /* GL_RENDERBUFFER_EXT or GL_TEXTURE */
    GLuint      objName;
    GLint       level;
    GLint       face;                  /*(0~5). GetFramebufferAttachmentParam should return GL_TEXTURE_CUBE_MAP_NEGATIVE_X...*/
    union
    {
        GLint   zoffset;
        GLint   layer;
    };

    GLboolean   layered;              /* geometry_shader4 */
}__GLfboAttachPoint;

/*
** Framebuffer object structure.
*/
typedef struct __GLframebufferObjectRec
{
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;

    GLuint name;
    GLuint targetIndex;

    /* Framebuffer attachment points: 0~(__GL_MAX_COLOR_ATTACHMENTS - 1) are color attachment,
    ** __GL_MAX_COLOR_ATTACHMENTS is depth attachment;
    ** __GL_MAX_COLOR_ATTACHMENTS+1 is stencil attachment point.
    */
    __GLfboAttachPoint attachPoint[__GL_MAX_ATTACHMENTS];

    /* Per famebuffer object states
    */
    GLenum drawBuffer[__GL_MAX_COLOR_ATTACHMENTS];
    GLenum readBuffer;
    GLsizei drawBufferCount;

    /* Internal flag for generic object management and framebuffer object consistency check.
    ** __GL_FRAMEBUFFER_IS_CHECKED ..
    */
    GLbitfield flag;

    /* Error enum of completness check result. App get it by CheckFramebufferStatus. */
    GLenum checkCode;

    /*
    ** Dimention of this framebuffer: valid only after completemness check .
    ** all image attaching to this framebuffer have same dimention.
    */
    GLuint fbWidth, fbHeight;

    /*
    ** Samples of this framebuffer.
    ** All image attaching to this framebuffer should have same samples
    */
    GLuint fbSamples;

    /*
    **Integer fbo if one of its attachpoint is integer internal format.
    **EXT_framebuffer_object require all color attachments have same internal format
    */
    GLboolean fbInteger;

    /*
    ** Float fbo if one of its attachpoint is float internal format.
    */
    GLboolean fbFloat;

    /* For multi context check, increase this number when any state of this FBO changed. */
    GLuint seqNumber;
} __GLframebufferObject;

/*
Framebuffer Per context  manage structure
*/
typedef struct __GLframebufObjMachineRec
{
    __GLsharedObjectMachine *fboShared;
    __GLsharedObjectMachine *rboShared;

    /* Default framebuffer object per context*/
    __GLframebufferObject defaultFBO;

    /* Default renderbuffer object*/
    __GLrenderbufferObject defaultRBO;

    /* Current framebuffer object bind to DRAW_FRAMEBUFFER_EXT or READ_FRAMEBUFFER_EXT */
    union {
        __GLframebufferObject *boundFramebufObj;
        __GLframebufferObject *drawFramebufObj;
    };
    __GLframebufferObject *readFramebufObj;

    /* Sequence number for multi context check:
    ** sync from fbo when bindFramebuffer;
    ** Check with seqNumber of framebuffer object to see if this fbo is changed by other context before any render action.
    ** Sync from fbo after check.
    **/
    union {
        GLuint fbSeqNumber;
        GLuint drawFBSeqNumber;
    };
    GLuint readFBSeqNumber;

    /* Current renderbuffer object bind to target RENDERBUFFER_EXT */
    __GLrenderbufferObject *boundRenderbufObj;

} __GLframebufObjMachine;

/* The current fbo name bound */
#define FRAMEBUFFER_BINDING_NAME  (gc->frameBuffer.boundFramebufObj->name)
#define DRAW_FRAMEBUFFER_BINDING_NAME  (gc->frameBuffer.drawFramebufObj->name)
#define READ_FRAMEBUFFER_BINDING_NAME  (gc->frameBuffer.readFramebufObj->name)

/* Map "attachment" to index */
__GL_INLINE GLint __glMapAttachmentToIndex(GLenum attachment)
{
    GLint attachIndex = -1;

    if((attachment >= GL_COLOR_ATTACHMENT0_EXT) && (attachment <= __GL_COLOR_ATTACHMENTn_EXT)){
        attachIndex = attachment - GL_COLOR_ATTACHMENT0_EXT;
        attachIndex = (attachIndex < __GL_MAX_COLOR_ATTACHMENTS)? attachIndex : -1;
    }
    else if(attachment == GL_DEPTH_ATTACHMENT_EXT){
        attachIndex = __GL_MAX_COLOR_ATTACHMENTS ;
    }
    else if(attachment == GL_STENCIL_ATTACHMENT_EXT){
        attachIndex = __GL_MAX_COLOR_ATTACHMENTS + 1;
    }

    return attachIndex;
}

__GL_INLINE GLint __glMapIndexToAttachment(GLint attachIndex)
{
    GL_ASSERT((attachIndex>=0) && (attachIndex < __GL_MAX_ATTACHMENTS));

    if(attachIndex < __GL_MAX_COLOR_ATTACHMENTS)
        return (GL_COLOR_ATTACHMENT0_EXT+attachIndex);

    if(attachIndex == __GL_MAX_COLOR_ATTACHMENTS)
        return GL_DEPTH_ATTACHMENT_EXT;
    else
        return GL_STENCIL_ATTACHMENT_EXT;

}

#endif /* __gc_gl_fbo_h_ */
