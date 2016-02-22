/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_fbo_h_
#define __chip_fbo_h_

typedef struct glsRenderBufferObjectRec{
    union{
        glsCHIPRENDERBUFFER *rtBufferInfo;
        glsCHIPDEPTHBUFFER *dBufferInfo;
        glsCHIPSTENCILBUFFER *sBufferInfo;
    };
} glsRenderBufferObject;

typedef struct glsFrameBufferObjectRec{
    glsCHIPRENDERBUFFER *rtBufferInfo;
    glsCHIPDEPTHBUFFER *dBufferInfo;
    glsCHIPSTENCILBUFFER *sBufferInfo;
} glsFrameBufferObject;

GLvoid pickReadBufferForFBO(__GLcontext *gc);
GLvoid pickDrawBufferForFBO(__GLcontext *gc);

/*  Declaration for DP interface                                              */
GLvoid __glChipBindRenderBufferObject(__GLcontext *gc,
    __GLrenderbufferObject *renderbuf);
GLboolean __glChipRenderbufferStorage(__GLcontext *gc,
    __GLrenderbufferObject *renderbuf);
GLvoid __glChipDeleteRenderbufferObject(__GLcontext *gc,
    __GLrenderbufferObject *renderbuf);
GLvoid __glChipBindDrawFrameBuffer(__GLcontext *gc,
    __GLframebufferObject *prebuf,
    __GLframebufferObject *curbuf);
GLvoid __glChipBindReadFrameBuffer(__GLcontext *gc,
    __GLframebufferObject *prebuf,
    __GLframebufferObject *curbuf);
GLvoid __glChipBlitFrameBuffer(__GLcontext *gc,
    GLint srcX0,
    GLint srcY0,
    GLint srcX1,
    GLint srcY1,
    GLint dstX0,
    GLint dstY0,
    GLint dstX1,
    GLint dstY1,
    GLbitfield mask,
    GLenum filter);
GLboolean __glChipIsFramebufferComplete(__GLcontext *gc, __GLframebufferObject *framebufferObj);
GLvoid __glChipFramebufferRenderbuffer(__GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLrenderbufferObject *renderbufferObj);
GLvoid __glChipFrameBufferTexture(__GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLint zoffset,
    GLboolean layered);
#endif /* __chip_fbo_h_ */
