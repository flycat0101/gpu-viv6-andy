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


#ifndef __chip_fbo_h__
#define __chip_fbo_h__

typedef struct __GLchipRenderbufferObjectRec
{
    gcoSURF surface;

    /* Shadow resource information. */
    __GLchipResourceShadow shadow;

    __GLchipStencilOpt *stencilOpt;

    __GLchipFmtMapInfo *formatMapInfo;
} __GLchipRenderbufferObject;


gceSTATUS
gcChipPickReadBufferForFBO(
    __GLcontext *gc
    );

gceSTATUS
gcChipPickDrawBufferForFBO(
    __GLcontext *gc
    );

/* Declaration for DP interface */
GLvoid
__glChipBindRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *renderbuf
    );

GLboolean
__glChipRenderbufferStorage(
    __GLcontext *gc,
    __GLrenderbufferObject *renderbuf
    );

GLvoid
__glChipDeleteRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *renderbuf
    );

GLvoid
__glChipDetachRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    );

GLboolean
__glChipBindDrawFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    );

GLvoid
__glChipBindReadFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    );

GLvoid
__glChipBlitFramebuffer(__GLcontext *gc,
                        GLint srcX0,
                        GLint srcY0,
                        GLint srcX1,
                        GLint srcY1,
                        GLint dstX0,
                        GLint dstY0,
                        GLint dstX1,
                        GLint dstY1,
                        GLbitfield mask,
                        GLboolean  xReverse,
                        GLboolean  yReverse,
                        GLenum     filter
                        );

GLboolean
__glChipIsFramebufferComplete(
    __GLcontext *gc,
    __GLframebufferObject *framebufferObj
    );

GLvoid
__glChipFramebufferRenderbuffer(
    __GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLrenderbufferObject *renderbufferObj,
    __GLfboAttachPoint *preAttach
    );

GLboolean
__glChipFramebufferTexture(
    __GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLsizei samples,
    GLint zoffset,
    GLboolean layered,
    __GLfboAttachPoint *preAttach
    );

GLvoid
__glChipCleanTextureShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj
    );

GLvoid
__glChipCleanRenderbufferShadow(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    );

gceSTATUS
gcChipRellocShadowResource(
    __GLcontext *gc,
    gcoSURF master,
    GLuint samples,
    __GLchipResourceShadow *shadow,
    __GLchipFmtMapInfo *formatMapInfo,
    GLboolean isFromTexture
    );

gceSTATUS
gcChipFBOMarkShadowRendered(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLbitfield mask
    );

gceSTATUS
gcChipTexMipSliceSyncFromShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint depth
    );

gceSTATUS
gcChipRboSyncFromShadow(
    __GLcontext* gc,
    __GLrenderbufferObject *rbo
    );

gceSTATUS
gcChipTexDirectSourceSyncFromMipSlice(
    __GLcontext *gc,
    __GLtextureObject *texObj
    );

gceSTATUS
gcChipTexSyncFromShadow(
    __GLcontext *gc,
    GLuint unit,
    __GLtextureObject *texObj
    );

gceSTATUS
gcChipFboSyncFromMasterSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    GLboolean read
    );

gcsSURF_VIEW
gcChipFboSyncFromShadowSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    GLboolean read
    );

gceSTATUS
gcChipFboSyncFromShadow(
    __GLcontext *gc,
    __GLframebufferObject *fbo
    );

gceSTATUS
gcChipFboSyncFromShadowFreon(
    __GLcontext *gc,
    __GLframebufferObject *fbo
    );

gceSTATUS
gcChipFBOSyncEGLImageNativeBuffer(
    __GLcontext *gc,
    gcoSURF surface,
    GLboolean read
    );

GLenum
__glChipCreateEglImageRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo,
    GLvoid *image
    );

GLboolean
__glChipEglImageTargetRenderbufferStorageOES(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo,
    GLenum target,
    GLvoid *eglImage
    );

gcsSURF_VIEW
gcChipGetFramebufferAttachedSurfaceAndImage(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLenum attachment,
    GLvoid **image
    );

GLboolean
__glChipBlitFramebufferBegin(
    __GLcontext *gc
    );

GLboolean
__glChipBlitFramebufferValidateState(
    __GLcontext *gc,
    GLbitfield mask
    );

GLboolean
__glChipBlitFramebufferEnd(
    __GLcontext *gc
    );


#endif /* __chip_fbo_h__ */
