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


#ifndef __gc_es_device_h__
#define __gc_es_device_h__

#define __GL_MAX_DEVICE_NUMBER        2
typedef struct _glsCONFIGDEPTH {
    GLubyte depthSize;
    GLubyte stencilSize;
} glsCONFIGDEPTH;

typedef struct _glsCONFIGCOLORALPHA {
    GLubyte  cColorBits;
    GLubyte  cRedBits;
    GLubyte  cRedShift;
    GLubyte  cGreenBits;
    GLubyte  cGreenShift;
    GLubyte  cBlueBits;
    GLubyte  cBlueShift;
    GLubyte  cAlphaBits;
    GLubyte  cAlphaShift;
} glsCONFIGCOLORALPHA;

typedef struct _glsCONFIGACCUM {
    GLubyte  cAccumBits;
    GLubyte  cAccumRedBits;
    GLubyte  cAccumGreenBits;
    GLubyte  cAccumBlueBits;
    GLubyte  cAccumAlphaBits;
} glsCONFIGACCUM;

typedef struct _glsCONFIGMULTISAMPLE {
    GLuint                extFlags;
    GLuint                 sampleQuality;
} glsCONFIGMULTISAMPLE;

typedef struct DisplayInfoStructRec
{
    GLuint Width;
    GLuint Height;
    GLuint Depth;
    GLuint RefreshRate;
}DisplayInfoStruct;

typedef struct __GLdeviceStructRec
{
    GLboolean (*devUpdateDrawable)(__GLdrawablePrivate *drawable);

    GLvoid (*devDestroyDrawable)(__GLdrawablePrivate *drawable);

    /* Create a HW specific context that is attached to gc->dp.privateData */
    GLboolean (*devCreateContext)(__GLcontext *gc);

    GLboolean (*devGetConstants)(__GLcontext *gc, struct __GLdeviceConstantsRec *constants);

#if defined(_LINUX_) && defined(DRI_PIXMAPRENDER_GL)
    /* Create a HW specific drawable structure that is attached to drawInfo->private */
    GLvoid (*devCreateDrawable)(struct __GLdrawablePrivateRec *drawInfo, GLvoid *window);
    /* Initialize/Deinitialize device */
    GLvoid (*devInitialize)(GLvoid *);
    GLvoid (*devDeinitialize)(GLvoid *);
#else
    GLboolean (*devDeinitialize)();
    GLvoid (*devInitialize)();
#ifdef _WINDOWS
    GLint (*devDescribePixelFormat)(GLint iPixelFormat, GLuint nBytes, GLvoid* ppfd, GLboolean bDisplayableOnly);
    GLboolean (*devSetPixelFormat)(GLint iPixelFormat);
    DisplayInfoStruct DisplayInfo;
#endif
#endif

    GLboolean IsRotated;
    GLboolean IsEXCLUSIVE_MODE;
} __GLdeviceStruct;

#ifdef __cplusplus
extern "C" __GLdeviceStruct *__glDevice;
#else
extern __GLdeviceStruct *__glDevice;
#endif

#ifdef __cplusplus
extern "C" __GLdeviceStruct __glDevicePipe;
#else
extern __GLdeviceStruct __glDevicePipe;
#endif

#endif /* __gc_es_device_h__ */
