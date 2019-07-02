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


#ifndef __gc_vk_query_h__
#define __gc_vk_query_h__

typedef struct __vkQueryRec
{
    VkQueryType type;
    VkQueryPool queryPool;
    uint32_t queryPoolIndex;
    VkQueryControlFlags flags;
    VkEvent event;
    VkBool32 isBegin;
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


