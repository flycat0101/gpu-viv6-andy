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


#ifndef __chip_misc_h__
#define __chip_misc_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __GLchipDrawableRec
{
    __GLchipStencilOpt *stencilOpt;
} __GLchipDrawable;

typedef struct __GLchipXfbHeaderRec
{
    gcsSURF_NODE headerNode;
    gctPOINTER   headerLocked;
} __GLchipXfbHeader;

#if defined(OPENGL40) && defined(GL4_DRI_BUILD)
typedef struct _glsSwapInfo {
    /* Swap surface. */
    gcoSURF swapSurface;
    GLuint  swapBitsPerPixel;
    GLuint  bitsAlignedWidth;
    GLuint  bitsAlignedHeight;
    GLvoid * swapBits;
} glsSwapInfo;

struct _glsCHIPRENDERBUFFER {
    gcoSURF                     renderTarget;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              renderTargetFormat;
    GLuint                      alignedWidth;
    GLuint                      alignedHeight;
    GLvoid *                    resolveBits;
};

struct _glsCHIPDEPTHBUFFER {
    gcoSURF                     depthBuffer;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              depthFormat;
};

struct _glsCHIPSTENCILBUFFER {
    gcoSURF                     stencilBuffer;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              stencilFormat;
};

struct _glCHIPBUFFERDESTROY {
    GLvoid  *bufInfo;
    GLuint  flags;
};

/* Render Buffer create and destroy information */
struct __glsCHIPBUFFERCREATE
{
    GLvoid  *bufInfo;
    GLuint  flags;
    gcePOOL poolType;
    gceSURF_TYPE surfType;

#ifdef __GL_LITTLE_ENDIAN
    union {
        struct {
            GLuint targetIndex  : 16;
            GLuint levels       : 16;
        };
        GLuint  subFlags;
    };
#else
    union {
        struct {
            GLuint levels       : 16;
            GLuint targetIndex  : 16;
        };
        GLuint  subFlags;
    };
#endif

    GLuint  sampleBuffers;
    GLuint  samples;
};



/* Index for Drawbuffers*/
#define __GL_FRONT_BUFFER_INDEX          __GL_DRAWBUFFER_FRONTLEFT_INDEX
#define __GL_BACK_BUFFER0_INDEX          __GL_DRAWBUFFER_BACKLEFT_INDEX
#define __GL_BACK_BUFFER1_INDEX          __GL_DRAWBUFFER_BACKRIGHT_INDEX
typedef struct _glsCHIPDRAWABLE
{
    __GLcontext * gc;
    /* Set if VG pipe is present and this surface was created when the current
       API was set to OpenVG. */
    gctBOOL openVG;
    /* Direct frame buffer access. */
    gcoSURF fbWrapper;
    gctBOOL fbDirect;

    GLint width;
    GLint height;
    GLenum internalFormatColorBuffer;

    /* Render target array. */
    gctBOOL   renderListEnable;
    gctUINT   renderListCount;

    GLboolean haveRenderBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;
    GLboolean haveAccumBuffer;

    glsCHIPRENDERBUFFER * originalFrontBuffer;
    glsCHIPRENDERBUFFER* originalFrontBuffer1;

    glsCHIPRENDERBUFFER** drawBuffers[__GL_MAX_DRAW_BUFFERS];
    glsCHIPDEPTHBUFFER * depthBuffer;
    glsCHIPSTENCILBUFFER * stencilBuffer;
    glsCHIPACCUMBUFFER * accumBuffer;

    /* Resolve surface. */
    glsCHIPRENDERBUFFER*  resolveBuffer;

    glsSwapInfo swapInfo;

    glsCHIPRENDERBUFFER   displayBuffer;

    gctPOINTER                 workerMutex;
} glsCHIPDRAWABLE;

typedef glsCHIPDRAWABLE glsCHIPREADABLE;
typedef glsCHIPDRAWABLE * glsCHIPDRAWABLE_PTR;
typedef glsCHIPREADABLE * glsCHIPREADABLE_PTR;
#endif

typedef struct __GLchipQueryHeaderRec
{
    gcsSURF_NODE headerNode;
    gctUINT32    headerSize;
    gctINT32     headerIndex;
    gctPOINTER   headerLocked;
    gceSURF_TYPE headerSurfType;
}__GLchipQueryHeader;

typedef struct __GLchipQueryObjectRec
{
    /* Query information */
    gctSIGNAL querySignal;
    __GLchipQueryHeader *queryHeader;
    gceQueryType  type;
}__GLchipQueryObject;

extern GLboolean
__glChipBeginQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipEndQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipGetQueryObject(
    __GLcontext *gc,
    GLenum pname,
    __GLqueryObject *queryObj
    );

extern GLvoid
__glChipDeleteQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipCreateSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    );

extern GLboolean
__glChipDeleteSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    );

extern GLenum
__glChipWaitSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject,
    GLuint64 timeout
    );

extern GLboolean
__glChipSyncImage(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetImageSrc(
    void* eglImage,
    gcoSURF surface
    );

GLvoid
__glChipBindXFB(
    __GLcontext *gc,
    __GLxfbObject   *xfbObj
    );

GLvoid
__glChipDeleteXFB(
    __GLcontext *gc,
    __GLxfbObject   *xfbObj
    );

extern GLvoid
__glChipBeginXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    );

extern GLvoid
__glChipEndXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    );

extern GLvoid
__glChipPauseXFB(
    __GLcontext *gc
    );

extern GLvoid
__glChipResumeXFB(
    __GLcontext *gc
    );

extern GLvoid
__glChipGetXFBVarying(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufSize,
    GLsizei* length,
    GLsizei* size,
    GLenum* type,
    GLchar* name
    );

extern GLboolean
__glChipCheckXFBBufSizes(
    __GLcontext *gc,
    __GLxfbObject *xfbObj,
    GLsizei count
    );

GLvoid
__glChipGetSampleLocation(
    __GLcontext *gc,
     GLuint index,
     GLfloat *val
     );

GLvoid
__glChipMemoryBarrier(
    __GLcontext *gc,
    GLbitfield barriers
    );

GLvoid
__glChipBlendBarrier(
    __GLcontext *gc
    );

extern GLboolean
gcChipCheckRecompileEnable(
    __GLcontext *gc,
    gceSURF_FORMAT format
    );

#ifdef __cplusplus
}
#endif

#endif /* __chip_misc_h__ */
