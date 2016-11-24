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


/*
 * OS specific functions.
 */

#ifndef __gc_vk_os_h__
#define __gc_vk_os_h__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR 1
#   define WIN32_LEAN_AND_MEAN 1
#   include <windows.h>
#   include <tchar.h>
#elif defined(__ANDROID__)
#   define VK_USE_PLATFORM_ANDROID_KHR 1
#   include <dlfcn.h>
#   include <unistd.h>
#   include "stdarg.h"
#   define OutputDebugString(szBuff) fprintf(stderr, szBuff)
#elif defined(__unix__)
#   include <dlfcn.h>
#   include <unistd.h>
#   include "stdarg.h"
#   include <stdio.h>
#   define OutputDebugString(szBuff) fprintf(stderr, szBuff)
#elif defined (__QNXNTO__)
#   include <dlfcn.h>
#   include <unistd.h>
#   include <semaphore.h>
#endif

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#   define __VK_NON_DISPATCHABLE_HANDLE_CAST(type, obj) ((type) (obj))
#else
#   define __VK_NON_DISPATCHABLE_HANDLE_CAST(type, obj) ((type) (uintptr_t) (obj))
#endif


#ifdef __cplusplus
}
#endif

#endif /* __gc_vk_os_h__ */


