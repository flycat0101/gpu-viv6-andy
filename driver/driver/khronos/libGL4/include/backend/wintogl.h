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


#ifndef __win_gl_h_
#define __win_gl_h_

#include "EGL/egl.h"
#include "gc_egl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __GLmemoryStatusEnum{
    __GL_MMUSAGE_PHYS_TOTAL = 0,
    __GL_MMUSAGE_PHYS_AVAIL = 1,
    __GL_MMUSAGE_VIRTUAL_TOTAL = 2,
    __GL_MMUSAGE_VIRTUAL_AVAIL = 3,
}__GLmemoryStatus;


#define __GL_DRAWABLE_PENDING_RESIZE        0x1
#define __GL_DRAWABLE_PENDING_MOVE          0x2
#define __GL_DRAWABLE_PENDING_DESTROY       0x4
#define __GL_DRAWABLE_PENDING_SWAP          0x8
#define __GL_DRAWABLE_PENDING_SWITCH        0x10
#define __GL_DRAWABLE_PENDING_RT_RESIDENT   0x20
#define __GL_DRAWABLE_PENDING_CLIPLIST      0x40
#define __GL_DRAWABLE_PENDING_PRIMARY_LOST  0x80

#ifdef __cplusplus
}
#endif

#endif
