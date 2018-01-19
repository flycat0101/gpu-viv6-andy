/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vk_types_h__
#define __gc_vk_types_h__

/*
** Low level data types.
*/
#include "gc_hal_user.h"

/*
** Define VK_PROTOTYPES before #include <vulkan.h>
*/
#ifndef VK_PROTOTYPES
#define VK_PROTOTYPES
#endif

#include <vulkan/vulkan.h>
#if defined(_WINDOWS)
#ifndef inline
#define inline __inline
#endif
#endif
#ifndef bool
#define bool VkBool32
#endif
#include <vulkan/vk_icd.h>

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 24)
#  include <vulkan/vk_android_native_buffer.h>
#endif


/************************************************************************/

#if defined(_WINDOWS)
#define __VK_INLINE static __forceinline
#define inline __inline
#else
#define __VK_INLINE static __inline
#endif

/* packed alignment defines for struct */
#if defined(_WINDOWS) || defined(_WIN32)
#define __VK_ATTRIB_ALIGN(n)    __declspec(align(n))
#else
#define __VK_ATTRIB_ALIGN(n)    __attribute__((aligned(n)))
#endif

/* Align with power of two value. */
#define __VK_ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))

#define __VK_PTR2UINT(ptr)  ((uint32_t)(gctUINTPTR_T)(ptr))
#define __VK_PTR2INT(ptr)   ((int32_t)(gctUINTPTR_T)(ptr))

#define __VK_PTR2SIZE(v)    ((gctSIZE_T)(void*)(v))
#define __VK_SIZE2PTR(v)    ((void*)(gctSIZE_T)(v))

#define __VK_TABLE_SIZE(x)  (uint32_t)(sizeof((x)) / sizeof((x)[0]))

/* Memory Operations */
#define __VK_MEMCOPY(to,from,count)     memcpy((void *)(to),(void *)(from),(size_t)(count))
#define __VK_MEMZERO(to,count)          memset(to,0,(size_t)(count))
#define __VK_MEMCMP(buf1, buf2, count)  memcmp(buf1, buf2, (size_t)(count))
#define __VK_MEMSET(to,value,count)     memset((to),(value),(count))


typedef struct VkBool3D {
    VkBool32    x;
    VkBool32    y;
    VkBool32    z;
} VkBool3D;

#endif /* __gc_vk_types_h__ */


