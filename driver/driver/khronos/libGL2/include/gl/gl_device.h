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


#ifndef __gl_device_h__
#define __gl_device_h__

#define __GL_MAX_DEVICE_NUMBER        2

typedef struct DisplayInfoStructRec
{
    GLuint Width;
    GLuint Height;
    GLuint Depth;
    GLuint RefreshRate;
}DisplayInfoStruct;

typedef struct __GLdeviceStructRec
{
#ifdef _LINUX_
    /* Initialize/Deinitialize device */
    GLvoid (*devInitialize)(__GLscreenPrivate *);
    GLvoid (*devDeInitialize)(__GLscreenPrivate *);
#endif

    /* Reinitialize device when device configuration changes */
    GLint (*devConfigChangeEnter)(GLvoid);
    GLint (*devConfigChangeExit)(__GLcontext *gc);
    GLint (*devUpdateDefaultVB)(__GLcontext *gc);

    /* Registers / Unregisters the thread */
    GLint (*devThreadAttach)(GLint threadIdx);
    GLboolean (*devThreadDetach)(GLuint threadID, GLvoid (*)(GLvoid *));

    /* Cleans up after an abnormal exit */
    GLvoid (*devSignal)(GLint signo);

    /* Function that returns device specific pixel formats */
    GLboolean (*devGetPixelFormats)(GLint numPixelFormat, GLvoid* pixelFormats, GLboolean bFloat);

    /* Create a HW specific context that is attached to gc->dp.ctx.privateData */
    GLvoid (*devCreateContext)(__GLcontext *gc);

    /* Create a HW specific drawable structure that is attached to drawInfo->private */
    GLvoid (*devCreateDrawable)(struct __GLdrawablePrivateRec *drawInfo, GLvoid *window);

    GLvoid (*devGetConstants)(struct __GLdeviceConstantsRec *constants);

    GLboolean (*devInitPixelFormats)(GLvoid);

    GLboolean (*devSetPixelFormat)(GLint iPixelFormat);

    GLint (*devDescribePixleFormat)(GLint iPixelFormat, GLuint nBytes, GLvoid* ppfd, GLboolean bDisplayableOnly);
    GLvoid (*devSetNonDisplayablePixelIndex)(GLint startIndex);

    GLvoid (*devUpdateDesktopInfo)();

    GLint (*devDeInitialization)(GLint processID);

    /*Create Pbuffer*/
    GLvoid (*devCreatePbufferARB)(__GLcontext *gc, __GLdrawablePrivate *glPriv);

    /* Query supported device format according to internalFormat
    ** which should be used by all buffers(texture,rbo,window renderbuffer)
    */
    GLuint (*devQueryDeviceFormat)(GLenum,GLboolean,GLint);

    GLboolean IsEXCLUSIVE_MODE;
#ifdef _LINUX_
#else
    LUID   adapterLuid;
    GLint  iUserRequiredVSync;
    DisplayInfoStruct DisplayInfo;
#endif
    GLboolean IsRotated;
} __GLdeviceStruct;

#ifdef __cplusplus
extern "C" __GLdeviceStruct *__glDevice;
#else
extern __GLdeviceStruct *__glDevice;
#endif

#endif /* __gl_device_h__ */
