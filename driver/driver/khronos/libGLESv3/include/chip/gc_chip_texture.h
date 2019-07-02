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


#ifndef __chip_texture_h__
#define __chip_texture_h__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
****** YUV direct texture information structure (GL_VIV_direct_texture). *******
\******************************************************************************/
typedef struct __GLchipDirectTextureRec
{
    gctBOOL         dirty;              /* Direct texture change flag. */
    gcoSURF         source;             /* Surface exposed to the user. */
    gctBOOL         directSample;
    gctBOOL         directRender;
    gceSURF_FORMAT  textureFormat;      /* closet texture format if indirect sampling. */
} __GLchipDirectTexture;

typedef struct __GLchipEGLImageRec
{
    gctBOOL         dirty;
    gcoSURF         source;
    gctBOOL         directSample;
    gceSURF_FORMAT  textureFormat;      /* closet texture format if indirect sampling. */

    gctPOINTER      nativeBuffer;       /* EGL_ANDROID_image_native_buffer. */
    GLvoid         *image;              /* Reference EGLImageKHR. */
} __GLchipEGLImage;

#define CHIP_TEX_IMAGE_UPTODATE(texInfo, level) \
    texInfo->imageUpToDate |= (__GL_ONE_32 << (level))

#define CHIP_TEX_IMAGE_OUTDATED(texInfo, level) \
    texInfo->imageUpToDate &= ~(__GL_ONE_32 << (level))

#define CHIP_TEX_IMAGE_OUTDATE_ALL(texInfo) \
    texInfo->imageUpToDate = 0;

#define CHIP_TEX_IMAGE_IS_UPTODATE(texInfo, level) \
    (texInfo->imageUpToDate & (__GL_ONE_32 << (level)))

#define CHIP_TEX_IMAGE_IS_ANY_LEVEL_DIRTY( texInfo, baseLevel, maxLevel) \
    (~(texInfo->imageUpToDate)) & ( (__GL_ONE_32 << (maxLevel + 1)) - (__GL_ONE_32 << baseLevel))


/*
** This structure is for shadow recourse management.
** Shadow resource is for indirect rendering.
** Master resource is for texture sampling.
*/
typedef struct __GLchipResourceShadowRec
{
    /* Master surface has content updated if TRUE */
    GLboolean masterDirty;

    /* Shadow surface has content updated if TRUE */
    GLboolean shadowDirty;

    /*Shadow surface, if exist we will always render to shadow surface for now */
    gcoSURF surface;
}__GLchipResourceShadow;


/*
** This struct record the info of chip layer mip level.
*/
typedef struct __GLchipMipmapInfoRec
{
    /* An array with index of depth[zoffset], for indirect RTT */
    __GLchipResourceShadow *shadow;

    __GLchipStencilOpt *stencilOpt;

    /* format mapping information */
    __GLchipFmtMapInfo *formatMapInfo;

    /* ASTC compressed data */
    GLubyte    *astcData;
    gctSIZE_T   astcBytes;  /* Size in byte of the astcData */
    gcoSURF     astcSurf;
} __GLchipMipmapInfo;


/******************************************************************************\
*************************** Main texture structure. ****************************
\******************************************************************************/
typedef struct __GLchipTexureInfoRec
{
    gcoTEXTURE object;                     /* Texture object. */

    /* Bit fields telling if the texture image in device memory is uptodate, one bit controls one level.
    ** if dp is going to use this texture, we must make sure that the texture image is uptodate (in device memory)
    */
    GLbitfield imageUpToDate;

    /* Any sub-resource of the texture was ever rendered, but still not sync to texture surface? */
    GLboolean rendered;
    __GLchipMipmapInfo *mipLevels;

    /* Save texDirectVIV info  */
    __GLchipDirectTexture direct;

    /* Save eglImage Info */
    __GLchipEGLImage  eglImage;

#if gcdUSE_NPOT_PATCH
    /* Is this texture a non-power-of-2 one? */
    GLboolean isNP2;
#endif

#if __GL_CHIP_PATCH_ENABLED
    GLboolean isFboRendered;
#endif

} __GLchipTextureInfo;

typedef struct __GLchipTextureRec
{
    /* Which stage texture was used by the current draw */
    GLbitfield curStageMask;

    /* Which stage texture was used since last txFlush */
    GLbitfield preStageMask;

    gcsTEXTURE halTexture[__GL_MAX_TEXTURE_UNITS];
} __GLchipTexture;

extern GLvoid
__glChipBindTexture(
    __GLcontext* gc,
    __GLtextureObject *texObj
    );

extern GLvoid
__glChipDeleteTexture(
    __GLcontext* gc,
    __GLtextureObject *texObj
    );

extern GLvoid
__glChipDetachTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    );

extern GLboolean
__glChipTexImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    );

extern GLboolean
__glChipTexImage3D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid *buf
    );

extern GLboolean
__glChipTexSubImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint width,
    GLint height,
    const GLvoid* buf
    );

extern GLboolean
__glChipTexSubImage3D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid* buf
    );

extern GLboolean __glChipCopyTexImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x,
    GLint y
    );

extern GLboolean
__glChipCopyTexSubImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x,
    GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset
    );

extern GLboolean
__glChipCopyTexSubImage3D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint x,
    GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset
    );

extern GLboolean
__glChipCompressedTexImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid* buf
    );

extern GLboolean
__glChipCompressedTexSubImage2D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint width,
    GLint height,
    const GLvoid* buf,
    GLsizei size
    );

extern GLboolean
__glChipCompressedTexImage3D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid* buf
    );

extern GLboolean
__glChipCompressedTexSubImage3D(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid* buf,
    GLsizei size
    );

extern GLboolean
__glChipGenerateMipMap(
    __GLcontext* gc,
    __GLtextureObject* texObj,
    GLint faces,
    GLint *maxLevel
    );


extern GLboolean
__glChipCopyTexBegin(
    __GLcontext* gc
    );

extern GLvoid
__glChipCopyTexValidateState(
    __GLcontext* gc
    );

extern GLvoid
__glChipCopyTexEnd(
    __GLcontext* gc
    );

/* EGL image */
extern GLboolean
__glChipBindTexImage(
    IN  __GLcontext *gc,
    IN  __GLtextureObject *texObj,
    IN  GLint level,
    IN  void * surface,
    OUT void ** pBinder
    );

extern GLvoid
__glChipFreeTexImage(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level
    );

extern GLenum
__glChipCreateEglImageTexture(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint depth,
    GLvoid * image
    );

extern GLboolean
__glChipEglImageTargetTexture2DOES(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLvoid *eglImage
    );

extern GLboolean
__glChipGetTextureAttribFromImage(
    __GLcontext *gc,
    GLvoid *eglImage,
    GLint *width,
    GLint *height,
    GLint *stride,
    gceSURF_FORMAT *format,
    GLint *glFormat,
    GLint *glInternalFormat,
    GLint *glType,
    GLint *level,
    GLuint *sliceIndex,
    GLvoid **pixel
    );

/* VIV_texture_direct */
extern GLboolean
__glChipTexDirectVIV(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint width,
    GLint height,
    GLenum format,
    GLvoid ** pixels
    );

extern GLboolean
__glChipTexDirectInvalidateVIV(
    __GLcontext* gc,
    __GLtextureObject *texObj
    );

extern GLboolean
__glChipTexDirectVIVMap(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLvoid ** logical,
    const GLuint * physical,
    GLboolean tiled
    );

extern GLboolean
__glChipCopyImageSubData(
    __GLcontext *gc,
    GLvoid * srcObject,
    GLint srcType,
    GLint srcLevel,
    GLint srcX,
    GLint srcY,
    GLint srcZ,
    GLvoid * dstObject,
    GLint dstType,
    GLint dstLevel,
    GLint dstX,
    GLint dstY,
    GLint dstZ,
    GLsizei width,
    GLsizei height,
    GLsizei depth
    );

extern gceSTATUS
gcChipTexSyncEGLImage(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    gctBOOL stall
    );

extern gceSTATUS
gcChipTexSyncDirectVIV(
    __GLcontext* gc,
    __GLtextureObject *texObj
    );

extern void
gcChipTexCheckDirtyStateKeep(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLuint samplerID
    );

extern gceSTATUS
gcChipTexGetFormatInfo(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    gcsSURF_FORMAT_INFO_PTR * txFormatInfo
    );

extern gceSTATUS
gcChipInitializeSampler(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipDeinitializeSampler(
    __GLcontext *gc
    );

extern GLboolean
gcChipTexNeedShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    __GLchipTextureInfo *texInfo,
    __GLchipFmtMapInfo *fmtMapInfo,
    GLint samples,
    GLint *samplesUsed
    );

GLvoid
gcChipUtilGetImageFormat(
    GLenum format,
    GLenum type,
    gceSURF_FORMAT *pImageFormat,
    gctSIZE_T *pBpp
    );

#ifdef __cplusplus
}
#endif

#endif /* __chip_texture_h__ */
