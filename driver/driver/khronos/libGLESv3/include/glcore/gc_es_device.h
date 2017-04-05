/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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

typedef struct __GLdeviceStructRec
{
    GLboolean (*devUpdateDrawable)(__GLdrawablePrivate *drawable, GLvoid* rtHandle, GLvoid* depthHandle, GLvoid *stencilHandle);

    GLvoid (*devDestroyDrawable)(__GLdrawablePrivate *drawable);

    /* Create a HW specific context that is attached to gc->dp.privateData */
    GLboolean (*devCreateContext)(__GLcontext *gc);

    GLboolean (*devGetConstants)(__GLcontext *gc, struct __GLdeviceConstantsRec *constants);

    GLboolean (*devDeinitialize)();

} __GLdeviceStruct;

#ifdef __cplusplus
extern "C" __GLdeviceStruct __glDevicePipe;
#else
extern __GLdeviceStruct __glDevicePipe;
#endif

#endif /* __gc_es_device_h__ */
