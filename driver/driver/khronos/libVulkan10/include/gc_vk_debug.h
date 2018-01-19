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


#ifndef __gc_vk_debug_h__
#define __gc_vk_debug_h__

/* Debug levels. */
#define __VK_DBG_LEVEL_ALWAYS    -1
#define __VK_DBG_LEVEL_ERROR      0
#define __VK_DBG_LEVEL_WARNING    1
#define __VK_DBG_LEVEL_INFO       2
#define __VK_DBG_LEVEL_TRACE      3
#define __VK_DBG_LEVEL_VERBOSE    4

#define __VK_PRINT(...)     gcmPRINT(__VA_ARGS__)

#if (defined(DBG) && DBG) || defined(DEBUG) || defined(_DEBUG)

extern int32_t __vkDebugLevel;

#define __VK_DEBUG_PRINT(__level__, ...) \
    if (__level__ <= __vkDebugLevel) \
    { \
        char buffer[512]; \
        unsigned int n=0; \
        gcoOS_PrintStrSafe(buffer, 512, &n, "%s:%d %s: ", __FILE__, __LINE__, __FUNCTION__); \
        gcoOS_PrintStrSafe(buffer, 512, &n, __VA_ARGS__); \
        OutputDebugString(buffer); \
    }

#define __VK_ASSERT(__Expression__) \
    if (!(__Expression__)) \
    { \
        char buffer[512]; \
        unsigned int offset=0; \
        gcoOS_PrintStrSafe(buffer, 512, &offset, "\n\nASSERT in "); \
        gcoOS_PrintStrSafe(buffer, 512, &offset, "%s:%d %s (%s) \n\n", __FILE__, __LINE__, __FUNCTION__, #__Expression__); \
        OutputDebugString(buffer); \
        gcmBREAK(); \
    }

#define __VK_DEBUG_ONLY(x)                  x

#else

#define __VK_DEBUG_PRINT(__level__, ...)    __VK_NOTHING

#define __VK_ASSERT(__Expression__)         __VK_NOTHING

#define __VK_DEBUG_ONLY(x)

#endif

#endif /* __gc_vk_debug_h__ */


