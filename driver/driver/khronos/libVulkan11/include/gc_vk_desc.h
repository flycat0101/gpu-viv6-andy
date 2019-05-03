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


#ifndef __gc_vk_desc_h__
#define __gc_vk_desc_h__

enum __VK_DESC_RESOURCE_INFO_TYPE
{
    __VK_DESC_RESOURCE_INVALID_INFO     = 0,
    __VK_DESC_RESOURCE_IMAGEVIEW_INFO   = 1,
    __VK_DESC_RESOURCE_BUFFERVIEW_INFO  = 2,
    __VK_DESC_RESOURCE_BUFFER_INFO      = 3,
};

typedef struct __vkDescriptorBufferInfoRec
{
    __vkBuffer *buffer;
    VkDeviceSize offset;
    VkDeviceSize range;
}__vkDescriptorBufferInfo;

typedef struct __vkDescriptorImageInfoRec
{
    __vkImageView *imageView;
    VkImageLayout layout;
}__vkDescriptorImageInfo;

typedef struct __vkDescriptorResourceInfoRec
{
    enum __VK_DESC_RESOURCE_INFO_TYPE type;

    union
    {
        __vkDescriptorImageInfo imageInfo;
        __vkDescriptorBufferInfo bufferInfo;
        __vkBufferView  *bufferView;
    } u;
} __vkDescriptorResourceInfo;

typedef struct __vkDescriptorResourceRegionRec
{
    uint32_t resource;     /* size in bytes */
    uint32_t sampler;      /* size in bytes */
}__vkDescriptorResourceRegion;

typedef struct __vkDescriptorSetEntryRec
{
    VkDescriptorSet descSet;
    VkBool32 isUsed;
}__vkDescriptorSetEntry;

typedef struct __vkDescriptorPoolRec
{
    __vkObject obj; /* Must be the first field */

    VkAllocationCallbacks memCb;
    VkDescriptorPoolCreateFlags flags;
    uint32_t maxSets;

    uint32_t allocatedSets;
    __vkDescriptorResourceInfo  *resourceInfo;
    __vkSampler **sampler;
    __vkDescriptorResourceRegion  size;     /* total size of pDescriptorView and pSampler */
    __vkDescriptorResourceRegion  cur;      /* current available point */

    __vkDescriptorSetEntry *pDescSets;
} __vkDescriptorPool;

typedef struct __vkDescriptorSetLayoutBindingRec
{
    VkDescriptorSetLayoutBinding std;
    __vkDescriptorResourceRegion offset; /* offset within this layout */
    __vkDescriptorResourceRegion perElementSize; /* region size of each element */

}__vkDescriptorSetLayoutBinding;

typedef struct __vkDescriptorSetLayoutRec
{
    __vkObject obj; /* Must be the first field */

    uint32_t bindingCount;

    __vkDescriptorSetLayoutBinding  *binding;

    __vkDescriptorResourceRegion size;  /* all region size of this set layout */

    uint32_t dynamicDescriptorCount;

    uint32_t samplerDescriptorCount; /* includes sampler, sampledImage,combinedSampledImage */

    uint32_t samplerBufferDescriptorCount; /* uniform texel buffer descriptor count  */

    uint32_t inputAttachmentDescriptorCount; /* input attachment descriptor count */

    uint32_t totalEntries;

}__vkDescriptorSetLayout;


typedef struct __vkDescriptorSetRec
{
    __vkObject obj; /* Must be the first field */

    /* pointer to the DescriptorPool */
    __vkDescriptorPool *descriptorPool;

    /* pointer to the descripto set layout */
    __vkDescriptorSetLayout *descSetLayout;

    __vkDescriptorResourceRegion  begin; /* starting point in descriptor pool */

    __vkDescriptorResourceRegion  size; /* allocated size of this descriptor set, if zero, means not allocated */

    __vkDescriptorResourceInfo *resInfos;

    __vkSampler **samplers;

    void *chipPriv;

    struct __vkDescriptorSetRec *next;

} __vkDescriptorSet;


typedef struct __VkDescriptorUpdateTemplateRec
{
    __vkObject obj; /* Must be the first field */

    VkDescriptorUpdateTemplateEntry * desUpdateTemplateEntry;
    uint32_t                          desUpdateEntryCount;
} __vkDescriptorUpdateTemplate;

#endif /* __gc_vk_desc_h__ */

