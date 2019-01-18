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


/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __gralloc_fb_h_
#define __gralloc_fb_h_

#if ANDROID_SDK_VERSION <= 19
#include <asm/page.h>
#endif
#include <hardware/gralloc.h>
#include <cutils/native_handle.h>

inline size_t
roundUpToPageSize(size_t x)
{
    return (x + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
}

/* Do not change the following function prototypes. */
int fb_device_open(const hw_module_t* module,
        const char* name, hw_device_t** device);

int gralloc_alloc_framebuffer(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride);

int gralloc_free_framebuffer(alloc_device_t* dev,
        buffer_handle_t handle);

#endif

