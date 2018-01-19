/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_gl_fbo_h__
#define __gc_gl_fbo_h__

#define __GL_MAX_FBOBJ_LINEAR_TABLE_SIZE        1024
#define __GL_DEFAULT_FBOBJ_LINEAR_TABLE_SIZE    256
#define __GL_FBOBJ_HASH_TABLE_SIZE              1024

#define __GL_MAX_RBOBJ_LINEAR_TABLE_SIZE        1024
#define __GL_DEFAULT_RBOBJ_LINEAR_TABLE_SIZE    256
#define __GL_RBOBJ_HASH_TABLE_SIZE              1024

/* The enum of max supported color attachment point */
#define __GL_COLOR_ATTACHMENTn              (GL_COLOR_ATTACHMENT0 + __GL_MAX_COLOR_ATTACHMENTS - 1)

#define __GL_COLOR_ATTACHMENT31             (GL_COLOR_ATTACHMENT0 + 31)

#define __GL_DEPTH_ATTACHMENT_POINT_INDEX   (__GL_MAX_COLOR_ATTACHMENTS)
#define __GL_STENCIL_ATTACHMENT_POINT_INDEX (__GL_MAX_COLOR_ATTACHMENTS + 1)

/* Bit masks for framebuffer obj completeness check and attrib flag */
#define __GL_FRAMEBUFFER_IS_CHECKED             (1 << 0)
#define __GL_FRAMEBUFFER_IS_COMPLETE            (1 << 1)
#define __GL_FRAMEBUFFER_ATTACH_TEX             (1 << 2)
#define __GL_FRAMEBUFFER_ATTACH_RBO             (1 << 3)

/* Macro to enable/disable framebuffer complete check */
#define __GL_FRAMEBUFFER_COMPLETE_DIRTY(fbo) \
    (fbo)->flag &= ~(__GL_FRAMEBUFFER_IS_CHECKED | __GL_FRAMEBUFFER_IS_COMPLETE | \
                     __GL_FRAMEBUFFER_ATTACH_TEX | __GL_FRAMEBUFFER_ATTACH_RBO);

/*
** Renderbuffer object structure.
*/
typedef struct __GLrenderbufferObjectRec
{
    /* Indicate how many targets the object is currently bound to */
    GLuint bindCount;

    /* List of FBO this renderbuffer object attached to  */
    __GLimageUser *fboList;

    /* State per renderbuffer object */
    GLuint name;
    GLuint targetIndex;
    GLint  width;
    GLint  height;
    GLint  samples;         /* samples per pixel, 0 means no multisample */
    GLint  samplesUsed;     /* The actual samples chosen by implementation */

    GLenum internalFormat;  /* Requested internal format */
    __GLformatInfo * formatInfo;

    /* Internal flag for generic object management  */
    GLbitfield flag;

    /* Indicate this object constructed by extension func call*/
    GLboolean   isExtMode;

    GLvoid *privateData;

    /* egl image */
    GLvoid *eglImage;

    GLchar *label;
} __GLrenderbufferObject;

/*
** Framebuffer object structure.
*/
typedef struct __GLfboAttachPointRec
{
    GLenum      objType;
    GLuint      objName;
    GLvoid*     object;
    GLint       level;
    GLint       face;
    GLint       layer;
    GLint       slice; /* Same as face if cube map, and layer otherwise. */

    /* Below two just for GL_EXT_multisampled_render_to_texture */
    GLsizei     samples;
    GLsizei     samplesUsed;

    GLboolean   layered;
    GLboolean   cube;   /* Is cube texture attached? */
    GLboolean   isExtMode;   /* Indicate extension func call*/

    GLuint      seqNumber;
} __GLfboAttachPoint;

/*
** Framebuffer object structure.
*/
typedef struct __GLframebufferObjectRec
{
    GLuint name;
    GLuint targetIndex;

    /* Framebuffer attachment points: 0~(__GL_MAX_COLOR_ATTACHMENTS - 1) are color attachment,
    ** __GL_MAX_COLOR_ATTACHMENTS is depth attachment;
    ** __GL_MAX_COLOR_ATTACHMENTS+1 is stencil attachment point.
    */
    __GLfboAttachPoint attachPoint[__GL_MAX_ATTACHMENTS];

    /* Per famebuffer object states
    */
    GLenum drawBuffers[__GL_MAX_DRAW_BUFFERS];
    GLenum readBuffer;
    GLsizei drawBufferCount;

    /* Internal flag for generic object management and framebuffer object consistency check.
    ** __GL_FRAMEBUFFER_IS_CHECKED
    */
    GLbitfield flag;

    /* Error enum of completeness check result. App get it by CheckFramebufferStatus.
    */
    GLenum checkCode;

    /*
    ** Dimention of this framebuffer: valid only after completeness check .
    ** all image attaching to this framebuffer have same dimension.
    */
    GLuint fbWidth, fbHeight;

    /*
    ** Samples of this framebuffer.
    ** All image attaching to this framebuffer should have same samples
    */
    GLuint fbSamples;

    /*
    ** Format flag composition:
    ** fbIntMask   fbUIntMask  fbUnormMask   fbFloatMask        RESULT
    **    +            +            -             -             Unsigned Integer
    **    +            -            -             -             Signed  Integer
    **    -            -            +             -             unsigned normalize
    **    -            -            -             -             signed nomralize
    **    -            -            -             +             float
    */

    GLuint fbIntMask;

    /*
    ** The bit is marked when the corresponding attachpoint is unsigned integer format.
    */
    GLuint fbUIntMask;

    /*
    ** The bit is marked when the corresponding attachpoint is unsigned normalized format.
    */
    GLuint fbUnormMask;

    /*
    ** The bit is marked when the corresponding attachpoint is is float internal format.
    */
    GLuint fbFloatMask;

    /*
    ** If FBO is layered rendering.
    */
    GLboolean fbLayered;
    GLuint fbMaxLayers;

    /*
    ** Patch flags.
    */
    GLboolean fast;

    /* One of fbo's attach point is to render to shadow surface */
    GLboolean shadowRender;

    /* For framebuffer_no_attachments */
    GLint defaultWidth;
    GLint defaultHeight;
    GLint defaultSamples;
    GLint defaultSamplesUsed;
    GLboolean defaultFixedSampleLoc;
    GLint defaultLayers;

    GLboolean useDefault;

    GLchar *label;
} __GLframebufferObject;

/*
Framebuffer Per context  manage structure
*/
typedef struct __GLframebufObjMachineRec
{
    /* Framebuffer objects are not shared between contexts according to the Spec.
    ** we just use the __GLsharedObjectMachine to manage the framebuffer objects.
    */
    __GLsharedObjectMachine *fboManager;

    /* Renderbuffer objects can be shared between contexts according to the Spec.
    */
    __GLsharedObjectMachine *rboShared;

    /* Default framebuffer object per context */
    __GLframebufferObject defaultDrawFBO;
    __GLframebufferObject defaultReadFBO;

    /* Default renderbuffer object */
    __GLrenderbufferObject defaultRBO;

    /* Current framebuffer object bind to DRAW_FRAMEBUFFER or READ_FRAMEBUFFER */
    __GLframebufferObject *drawFramebufObj;
    __GLframebufferObject *readFramebufObj;

    /* Current renderbuffer object bind to target RENDERBUFFER */
    __GLrenderbufferObject *boundRenderbufObj;

} __GLframebufObjMachine;

/* The currently bound fbo name */
#define FRAMEBUFFER_BINDING_NAME       (gc->frameBuffer.boundFramebufObj->name)
#define DRAW_FRAMEBUFFER_BINDING_NAME  (gc->frameBuffer.drawFramebufObj->name)
#define READ_FRAMEBUFFER_BINDING_NAME  (gc->frameBuffer.readFramebufObj->name)

/* Map "attachment" to index */
__GL_INLINE GLint __glMapAttachmentToIndex(GLenum attachment)
{
    GLint attachIndex = -1;

    if ((attachment >= GL_COLOR_ATTACHMENT0) && (attachment <= __GL_COLOR_ATTACHMENTn))
    {
        attachIndex = attachment - GL_COLOR_ATTACHMENT0;
    }
    else if (attachment == GL_DEPTH_ATTACHMENT)
    {
        attachIndex = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
    }
    else if (attachment == GL_STENCIL_ATTACHMENT)
    {
        attachIndex = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
    }

    return attachIndex;
}

__GL_INLINE GLint __glMapIndexToAttachment(GLint attachIndex)
{
    GL_ASSERT((attachIndex>=0) && (attachIndex < __GL_MAX_ATTACHMENTS));

    if (attachIndex < __GL_MAX_COLOR_ATTACHMENTS)
    {
        return (GL_COLOR_ATTACHMENT0 + attachIndex);
    }
    else if (attachIndex == __GL_DEPTH_ATTACHMENT_POINT_INDEX)
    {
        return GL_DEPTH_ATTACHMENT;
    }
    else if (attachIndex == __GL_STENCIL_ATTACHMENT_POINT_INDEX)
    {
        return GL_STENCIL_ATTACHMENT;
    }

    return -1;
}


__GLformatInfo* __glGetFramebufferFormatInfo(__GLcontext *gc, __GLframebufferObject *framebufferObj, GLenum attachment);

#endif /* __gc_gl_fbo_h__ */
