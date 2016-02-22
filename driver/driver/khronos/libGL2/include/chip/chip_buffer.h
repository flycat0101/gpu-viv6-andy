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


#ifndef __chip_buffer_h_
#define __chip_buffer_h_
#include "gc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __GL_FRONT_BUFFER                    1
#define __GL_BACK_BUFFER                     2
#define __GL_DEPTH_BUFFER                    3
#define __GL_DEPTH_AND_STENCIL_BUFFER        4
#define __GL_TEXTURE_SURFACE                 5
#define __GL_TEXTURE_LEVEL                   6
#define __GL_VERTEX_BUFFER                   7
#define __GL_STENCIL_BUFFER                  8
#define __GL_PBUFFERTEX_BUFFER               9
#define __GL_ACCUM_BUFFER                   10
#define __GL_CONSTANT_BUFFER                11
#define __GL_RESOLVE_BUFFER_FLAG            12

/* Render Buffer create and destroy information */
typedef struct __glsCHIPBUFFERCREATE
{
    GLvoid  *bufInfo;
    GLuint  flags;
    gcePOOL poolType;
    gceSURF_TYPE surfType;

#if defined(USE_LENDIAN)
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
} glsCHIPBUFFERCREATE;

typedef struct _glCHIPBUFFERDESTROY {
    GLvoid  *bufInfo;
    GLuint  flags;
} glCHIPBUFFERDESSTROY;


typedef struct _glsCHIPRENDERBUFFER {
    gcoSURF                     renderTarget;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              renderTargetFormat;
    GLuint                      alignedWidth;
    GLuint                      alignedHeight;
    GLvoid *                    resolveBits;
} glsCHIPRENDERBUFFER;

typedef struct _glsCHIPDEPTHBUFFER {
    gcoSURF                     depthBuffer;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              depthFormat;
} glsCHIPDEPTHBUFFER;

typedef struct _glsCHIPSTENCILBUFFER {
    gcoSURF                     stencilBuffer;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              stencilFormat;
} glsCHIPSTENCILBUFFER;


typedef struct _glsCHIPACCUMBUFFER {
    gcoSURF                     renderTarget;
    gcoSURF                     lockTarget;
    gceSURF_FORMAT              renderTargetFormat;
    gcsTEXTURE                  texture[2];
    glsTEXTUREINFO              textureInfo[2];
    glsTEXTURESAMPLER           sampler[2];
} glsCHIPACCUMBUFFER;

typedef struct _glsCHIPRENDERTEXTURE {
    gcoSURF                     renderTarget;
    gcoSURF                     lockTarget;
    gcoSURF                     tex;
    GLuint                      levels;
    GLuint                      faces;
    gcoSURF                     *renderTargets;  /* RT view array for every faces and levels */
} glsCHIPRENDERTEXTURE;

#if defined(USE_LENDIAN)
typedef struct _glsCHIPBUFFERFLAGS {
    union
    {
        struct
        {
            GLushort vertexBuffer       : 1;
            GLushort indexBuffer        : 1;
            GLushort constantBuffer     : 1;
            GLushort pixelBuffer        : 1;
            GLushort outputBuffer       : 1;
            GLushort textureBuffer      : 1;

            GLushort reserved_1         : 10;
        };
        GLushort bindFlags;
    };

    union
    {
        struct
        {
            GLushort dynamic            : 1;

            GLushort reserved_2         : 15;
        };
        GLushort usageFlags;
    };
} glsCHIPBUFFERFLAGS;
#else
typedef struct _glsCHIPBUFFERFLAGS {
    union
    {
        struct
        {
            GLushort reserved_1         : 10;
            GLushort textureBuffer      : 1;
            GLushort outputBuffer       : 1;
            GLushort pixelBuffer        : 1;
            GLushort constantBuffer     : 1;
            GLushort indexBuffer        : 1;
            GLushort vertexBuffer       : 1;
        };
        GLushort bindFlags;
    };

    union
    {
        struct
        {
            GLushort reserved_2         : 15;
            GLushort dynamic            : 1;
        };
        GLushort usageFlags;
    };
} glsCHIPBUFFERFLAGS;
#endif

#define __GL_PBO_PENDING_TYPE_TEXTURE   0
#define __GL_PBO_PENDING_TYPE_DRAWPIXEL 1

typedef struct _glsCHIPBUFFEROBJECTPENDINGLIST {
    struct _glsCHIPBUFFEROBJECTPENDINGLIST *next;

    GLuint  name;
    GLuint  type;
} glsCHIPBUFFEROBJECTPENDINGLIST;

/*
** This structure is used to save vertex buffer information,
** vertex buffer object privateData and display list privateData points to this
** structure
*/
typedef struct _glsVERTEXBUFFERINFO {
    GLvoid*             bufObject;
    GLsizeiptr          size;           /* buffer size in bytes */
    glsCHIPBUFFERFLAGS  flags;
    GLboolean           fresh;          /* whether buffer had been renamed */

    GLuint              freeOffset;     /* byte offset of free space from buffer start */
    GLuint              freeSize;       /* buffer size - free offset */
    gctPOINTER          bufferMapPointer;

    __GLresidenceType   location;

    /* List contained the objects which source from this buffer object */
    glsCHIPBUFFEROBJECTPENDINGLIST  *toCopyList;

    GLboolean           isMapped;

} glsVERTEXBUFFERINFO;

extern GLvoid __glChipLockBuffer(__GLcontext *gc, void *buffer, GLuint format, GLuint **base, GLuint *pitch);
extern GLvoid __glChipUnlockBuffer(__GLcontext *gc, void *buffer, GLuint format);
#ifdef __cplusplus
}
#endif

#endif /* __chip_buffer_h_ */
