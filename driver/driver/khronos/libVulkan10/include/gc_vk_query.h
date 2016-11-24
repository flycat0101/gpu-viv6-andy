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


#ifndef __gc_vk_query_h__
#define __gc_vk_query_h__

typedef enum
{
    __VK_QUERY_UNDEFINED,
    __VK_QUERY_RESET,
    __VK_QUERY_BEGIN,
    __VK_QUERY_ISSUED,
    __VK_QUERY_END
} __vkQueryState;

typedef struct __vkQueryRec
{
    VkQueryType type;
    VkQueryPool queryPool;
    uint32_t queryPoolIndex;
    __vkQueryState state;
    VkQueryControlFlags flags;
    VkEvent event;
} __vkQuery;

typedef struct __vkQueryPoolRec
{
    __vkObject obj; /* Must be the first field */

    /* QueryPool specific fields */
    uint32_t queryCount;
    __vkQuery *pQueries;
    VkBuffer queryBuffer;
} __vkQueryPool;
#endif /* __gc_vk_query_h__ */


