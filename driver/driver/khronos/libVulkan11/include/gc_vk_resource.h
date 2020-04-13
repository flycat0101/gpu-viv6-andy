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


#ifndef __gc_vk_resource_h__
#define __gc_vk_resource_h__

#define __VK_FORMAT_SAMPLE_IMAGE_FEATURES \
    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT               | \
    VK_FORMAT_FEATURE_BLIT_SRC_BIT                    | \
    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR            | \
    VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR

#define __VK_FORMAT_SAMPLE_IMAGE_FILTERABLE_FEATURES \
    __VK_FORMAT_SAMPLE_IMAGE_FEATURES                 | \
    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT

#define __VK_FORMAT_STORAGE_IMAGE_FEATURES \
    VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT               | \
    VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT

#define __VK_FORMAT_STORAGE_TEXEL_BUFFER_FEATURES  \
    VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT        | \
    VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT

#define __VK_FORMAT_SAMPLE_TEXEL_BUFFER_FEATURES \
    VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT

#define __VK_FORMAT_COLOR_FEATURES \
    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT            | \
    VK_FORMAT_FEATURE_BLIT_DST_BIT

#define __VK_FORMAT_COLOR_BLEND_FEATURES    \
    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT            | \
    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT      | \
    VK_FORMAT_FEATURE_BLIT_DST_BIT

#define __VK_FORMAT_VERTEX_FEATURES \
    VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT

#define __VK_FORMAT_DEPTH_FEATURES \
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT

#define __VK_FORMAT_YCBCR_CONVERSION_FEATURES \
    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT               | \
    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR            | \
    VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR

enum
{
    __VK_FMT_CATEGORY_UNORM = 0,
    __VK_FMT_CATEGORY_SNORM,
    __VK_FMT_CATEGORY_USCALED,
    __VK_FMT_CATEGORY_SSCALED,
    __VK_FMT_CATEGORY_UFLOAT,
    __VK_FMT_CATEGORY_SFLOAT,
    __VK_FMT_CATEGORY_UINT,
    __VK_FMT_CATEGORY_SINT,
    __VK_FMT_CATEGORY_SRGB,

    __VK_FMT_CATEGORY_MAX
};

typedef struct __vkFormatInfoRec
{
    uint32_t            category;
    VkBool32            compressed;
    VkExtent2D          blockSize;
    int32_t             bitsPerBlock;
    uint32_t            partCount;
    uint32_t            residentImgFormat;
    VkFormatProperties  formatProperties;
} __vkFormatInfo;

typedef struct __vkTileStatusRec
{
    gcsSURF_NODE tsNode;

    /* ts related information. */
    VkBool32 **tileStatusDisable;
    uint32_t **fcValue;
    uint32_t **fcValueUpper;
    VkBool32 compressed;
    int32_t compressedFormat;
    uint32_t tileStatusFiller;
    uint32_t tileStatusInvalidFiller;
    uint32_t mipLevels;
    uint32_t devAddr;
    gctPOINTER hostAddr;
} __vkTileStatus;

typedef struct __vkDeviceMemoryRec
{
    __vkObject obj; /* Must be the first field */

    __vkDevContext *devCtx;

    VkAllocationCallbacks memCb;

    VkMemoryAllocateInfo allocInfo;
    uint32_t memTypeIdx;
    VkBool32 mapped;

    gctUINT align;
    gctSIZE_T size;
    gcsSURF_NODE node;

    uint32_t devAddr;
    gctPOINTER hostAddr;

    VkDeviceSize mappedOffset;
    VkDeviceSize mappedSize;
#if __VK_ENABLETS
    /* ts related information. */
    __vkTileStatus *ts;
#endif
#if (ANDROID_SDK_VERSION >= 26)
    struct AHardwareBuffer  *ahwBuffer;
#endif
} __vkDeviceMemory;

typedef struct __vkBufferRec
{
    __vkObject obj; /* Must be the first field */
    __vkDevContext *devCtx;

    VkAllocationCallbacks memCb;

    /* Buffer specific fields */
    VkBufferCreateInfo createInfo;
    VkMemoryRequirements memReq;

    __vkDeviceMemory *memory;
    VkDeviceSize memOffset;

    __vkDeviceMemory *splitMemory;
    VkDeviceSize splitMemOffset;

} __vkBuffer;

typedef struct __vkBufferViewRec
{
    __vkObject obj; /* Must be the first field */
    __vkDevContext *devCtx;

    VkAllocationCallbacks memCb;
    /* BufferView specific fields */
    VkBufferViewCreateInfo createInfo;

    __vkFormatInfo formatInfo;

    void *chipPriv;

} __vkBufferView;

typedef struct __vkImageLevelRec
{
    /* Surface size. */
    gctUINT                 requestW;   /* client request width  */
    gctUINT                 requestH;   /* client request height */
    gctUINT                 requestD;   /* client request depth  */
    gctUINT                 allocedW;   /* allocated width  applied msaa */
    gctUINT                 allocedH;   /* allocated height applied msaa */
    gctUINT                 alignedW;   /* aligned allocated width  */
    gctUINT                 alignedH;   /* aligned allocated height */

    uint32_t                partCount;
    VkDeviceSize            stride;
    VkDeviceSize            sliceSize;
    VkDeviceSize            partSize;
    VkDeviceSize            size;

    VkDeviceSize            offset;
} __vkImageLevel;

typedef struct __vkImageRec
{
    __vkObject obj; /* Must be the first field */
    __vkDevContext *devCtx;

    VkAllocationCallbacks memCb;
    /* Image specific fields */
    VkImageCreateInfo createInfo;
    VkMemoryRequirements memReq;
    __vkFormatInfo formatInfo;
    gceTILING halTiling;
    gctUINT32 hAlignment;
    gcsSAMPLES sampleInfo;

    __vkImageLevel *pImgLevels;

    /*
     * 'resident' means memory belongs to the image, but not attached by
     * BindImageMemory. Need free resident memory when destroy the image.
     */
    VkBool32 residentMemory;
    __vkDeviceMemory *memory;
    VkDeviceSize memOffset;
    VkImage shadowImage;
} __vkImage;

enum
{
    __VK_IMAGE_VIEW_TYPE_1D_BIT         = 1 << VK_IMAGE_VIEW_TYPE_1D,
    __VK_IMAGE_VIEW_TYPE_2D_BIT         = 1 << VK_IMAGE_VIEW_TYPE_2D,
    __VK_IMAGE_VIEW_TYPE_3D_BIT         = 1 << VK_IMAGE_VIEW_TYPE_3D,
    __VK_IMAGE_VIEW_TYPE_CUBE_BIT       = 1 << VK_IMAGE_VIEW_TYPE_CUBE,
    __VK_IMAGE_VIEW_TYPE_1D_ARRAY_BIT   = 1 << VK_IMAGE_VIEW_TYPE_1D_ARRAY,
    __VK_IMAGE_VIEW_TYPE_2D_ARRAY_BIT   = 1 << VK_IMAGE_VIEW_TYPE_2D_ARRAY,
    __VK_IMAGE_VIEW_TYPE_CUBE_ARRAY_BIT = 1 << VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
};

typedef struct __vkSamplerYcbcrConversionRec
{
    __vkObject obj; /* Must be the first field */
    VkSamplerYcbcrConversionCreateInfo  createInfo;
    VkAllocationCallbacks memCb;
}__vkSamplerYcbcrConversion;

typedef struct __vkImageViewRec
{
    __vkObject obj; /* Must be the first field */
    __vkDevContext *devCtx;

    /* ImageView specific fields */
    VkImageViewCreateInfo createInfo;
    __vkSamplerYcbcrConversion conversion;
    VkAllocationCallbacks memCb;

    const __vkFormatInfo *formatInfo;

    void *chipPriv;

} __vkImageView;


typedef struct __vkSamplerRec
{
    __vkObject obj; /* Must be the first field */

    /* Sampler specific fields */
    VkSamplerCreateInfo createInfo;

    VkAllocationCallbacks memCb;

    void *chipPriv;

} __vkSampler;

struct __vkBlitResRec
{
    VkBool32 isImage;

    union {
        struct {
            __vkImage *pImage;
            VkImageSubresource subRes;
            VkOffset3D offset;
            VkExtent3D extent;
        } img;

        struct {
            __vkBuffer *pBuffer;
            VkDeviceSize offset;
            uint32_t rowLength;
            uint32_t imgHeight;
            uint32_t sliceSize;
        } buf;
    } u;

};


enum
{
    __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT = VK_FORMAT_END_RANGE + 1,
    __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
    __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
    __VK_FORMAT_R16G16B16A16_SFLOAT_2_R16G16_SFLOAT,
    __VK_FORMAT_R16G16B16A16_SINT_2_R16G16_SINT,
    __VK_FORMAT_R16G16B16A16_UINT_2_R16G16_UINT,
    __VK_FORMAT_R32G32_SFLOAT_2_R32_SFLOAT,
    __VK_FORMAT_R32G32_SINT_2_R32_SINT,
    __VK_FORMAT_R32G32_UINT_2_R32_UINT,
    __VK_FORMAT_A4R4G4B4_UNORM_PACK16,
    __VK_FORMAT_R8_1_X8R8G8B8,
    __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32,
    __VK_FORMAT_D24_UNORM_X8_PACKED32,

    __VK_FORMAT_G8B8G8R8_422_RGB_IDENTITY_UNORM,
    __VK_FORMAT_B8G8R8G8_422_RGB_IDENTITY_UNORM,
};

enum
{
    __VK_SAMPLER_MIPMAP_MODE_NONE = VK_SAMPLER_MIPMAP_MODE_END_RANGE + 1,
};

#define __VK_MAX_IMAGE_LAYERS 512
#define __VK_MAX_PARTS        2

__vkFormatInfo * __vk_GetVkFormatInfo(
    VkFormat vkFormat
    );

typedef enum __VkMemoryImportType
{
    __VK_MEMORY_IMPORT_TYPE_USER_MEMORY,
    __VK_MEMORY_IMPORT_TYPE_VIDEO_NODE,
    __VK_MEMORY_IMPORT_TYPE_LINUX_DMA_BUF,
} __VkMemoryImportType;

typedef struct __VkMemoryImportInfoRec
{
    __VkMemoryImportType    type;

    union
    {
        /*
         * For user memory. Set physical to ~0UL if to wrap virtual memory,
         * For physical contiguous memory, virtual address is still required
         * currently.
         */
        /* This is risky. */
        struct
        {
            unsigned long   physical;
            void *          virtAddress;
        } usermemory;

        /* For video memory node. */
        struct
        {
            uint32_t        nodeName;
            gcePOOL         nodePool;
        } videoMemNode;

        /* For linux dma buf (includes ion). */
        struct
        {
            int             dmaBufFd;
        } dmaBuf;
    } u;
} __VkMemoryImportInfo;

VKAPI_ATTR VkResult VKAPI_CALL __vk_ImportMemory(
    VkDevice device,
    const VkMemoryAllocateInfo* pAllocateInfo,
    const __VkMemoryImportInfo* pImportInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDeviceMemory* pMemory
    );


#endif /* __gc_vk_resource_h__ */


