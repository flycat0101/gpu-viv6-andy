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


#include "gc_vk_precomp.h"

static const uint32_t s_crc32Table[256] =
{
    0x00000000,0x77073096,0xee0e612c,0x990951ba,
    0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
    0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,
    0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
    0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,
    0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
    0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,
    0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
    0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,
    0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
    0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,
    0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
    0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,
    0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
    0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
    0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
    0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,
    0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
    0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,
    0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
    0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
    0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
    0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,
    0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
    0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,
    0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
    0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,
    0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
    0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,
    0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
    0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,
    0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
    0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,
    0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
    0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,
    0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
    0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,
    0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
    0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,
    0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
    0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,
    0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
    0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,
    0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
    0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,
    0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
    0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,
    0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
    0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,
    0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
    0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,
    0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
    0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,
    0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
    0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,
    0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
    0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,
    0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
    0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,
    0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
    0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,
    0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
    0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
    0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d
};

uint32_t __vk_utils_evalCrc32(
    uint32_t initial,
    const void *pData,
    uint32_t bytes
    )
{
    uint32_t crc = ~initial;
    const uint8_t *start = (uint8_t*)pData;
    const uint8_t *end = start + bytes;

    while (start < end)
    {
        uint32_t data = *start++;
        data &= 0xFF;
        crc = s_crc32Table[(crc & 255) ^ data] ^ (crc >> 8);
    }

    return ~crc;
}

uint32_t __vk_utils_evalCrc32_masked_array(
    void **ppData,
    void **ppMask,
    uint32_t *pDataSizeInByte,
    uint32_t arraySize
    )
{
    uint32_t crc;
    uint8_t *start, *startMask;
    uint8_t *end;
    __VK_DEBUG_ONLY(uint8_t *endMask;)
    uint32_t i;
    uint32_t crcArray[8];

    __VK_ASSERT(arraySize < 8);
    for (i = 0; i < arraySize; i++)
    {
        crc = 0xFFFFFFFF;
        start = (uint8_t*)ppData[i];
        end = start + pDataSizeInByte[i];

        startMask = (uint8_t*)ppMask[i];
        __VK_DEBUG_ONLY(endMask = startMask + pDataSizeInByte[i]);

        while (start < end)
        {
            uint32_t data = (*start++) & (*startMask++);
            data &= 0xFF;
            crc = s_crc32Table[(crc & 255) ^ data] ^ (crc >> 8);
        }
        __VK_ASSERT(startMask == endMask);
        crcArray[i] = ~crc;
    }

    start = (uint8_t *)&crcArray[0];
    end = start + (arraySize * sizeof(uint32_t));
    crc = 0xFFFFFFFF;
    while (start < end)
    {
        uint32_t data = *start ++;
        data &= 0xFF;
        crc = s_crc32Table[(crc & 255) ^ data] ^ (crc >> 8);
    }

    return ~crc;
}

void __vk_utils_region_set(
    __vkDescriptorResourceRegion *region,
    uint32_t resource,
    uint32_t sampler
    )
{
    region->resource = resource;
    region->sampler = sampler;
}

void __vk_utils_region_mad(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *base,
    uint32_t scale,
    __vkDescriptorResourceRegion * offset
    )
{
    result->resource = base->resource * scale + offset->resource;
    result->sampler = base->sampler * scale + offset->sampler;
}

void __vk_utils_region_copy(
    __vkDescriptorResourceRegion *dstRegion,
    __vkDescriptorResourceRegion *srcRegion
    )
{
    __VK_MEMCOPY(dstRegion, srcRegion, sizeof(__vkDescriptorResourceRegion));
}

void __vk_utils_region_minus(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    )
{
    result->resource = lOperand->resource - rOperand->resource;
    result->sampler = lOperand->sampler - rOperand->sampler;
}

void __vk_utils_region_add(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    )
{
    result->resource = lOperand->resource + rOperand->resource;
    result->sampler = lOperand->sampler + rOperand->sampler;
}

void __vk_utils_region_mul(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *base,
    uint32_t scale
    )
{
    result->resource = base->resource * scale;
    result->sampler = base->sampler * scale;
}

VkBool32 __vk_utils_region_equal(
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    )
{
    return (lOperand->resource == rOperand->resource) && (lOperand->sampler == rOperand->sampler);
}

VkBool32 __vk_utils_region_gequal(
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    )
{
    return (lOperand->resource >= rOperand->resource) && (lOperand->sampler >= rOperand->sampler);
}

void __vk_utils_objAddRef(
    __vkUtilsHashObject *pObj
    )
{
    pObj->refCount++;
}

void __vk_utils_objReleaseRef(
    __vkUtilsHashObject *pObj
    )
{
    pObj->refCount--;

    __VK_ASSERT(pObj->refCount >= 0);
}

__vkUtilsHash* __vk_utils_hashCreate(
    VkAllocationCallbacks *pAllocator,
    uint32_t keyBytes,
    uint32_t tbEntryNum,
    uint32_t maxEntryObjs,
    __vkUtilsDeleteUserDataFunc pfnDeleteUserData
    )
{
    __vkUtilsHash *pHash = gcvNULL;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(pAllocator);

    pHash = (__vkUtilsHash*)__VK_ALLOC(sizeof(__vkUtilsHash), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(pHash ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(pHash, sizeof(__vkUtilsHash));

    pHash->keyBytes = keyBytes;
    pHash->tbEntryNum = tbEntryNum;
    pHash->maxEntryObjs = maxEntryObjs;
    pHash->year = 0;
    pHash->pfnDeleteUserData = pfnDeleteUserData;

    pHash->ppHashTable =
        (__vkUtilsHashObject**)__VK_ALLOC(sizeof(__vkUtilsHashObject *) * tbEntryNum, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(pHash->ppHashTable ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_MEMZERO(pHash->ppHashTable, sizeof(__vkUtilsHashObject *) * tbEntryNum);

    pHash->pEntryCounts =  (uint32_t *)__VK_ALLOC(sizeof(uint32_t) * tbEntryNum, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(pHash->pEntryCounts ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_MEMZERO(pHash->pEntryCounts, sizeof(uint32_t) * tbEntryNum);

    return pHash;

OnError:
    if (pHash)
    {
        if (pHash->ppHashTable)
        {
            __VK_FREE(pHash->ppHashTable);
        }
        if (pHash->pEntryCounts)
        {
            __VK_FREE(pHash->pEntryCounts);
        }
        __VK_FREE(pHash);
    }
    return VK_NULL_HANDLE;
}

void __vk_utils_hashDestory(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash
    )
{
    __VK_SET_ALLOCATIONCB(pAllocator);

    __vk_utils_hashDeleteAllObjs(pAllocator, pHash);
    __VK_FREE(pHash->pEntryCounts);
    __VK_FREE(pHash->ppHashTable);
    __VK_FREE(pHash);
}

void __vk_utils_hashDeleteObj(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash,
    __vkUtilsHashObject *pObj
    )
{
    __vkUtilsHashObject *pCurObj = pHash->ppHashTable[pObj->entryId];
    __vkUtilsHashObject *pPreObj = gcvNULL;
    __VK_SET_ALLOCATIONCB(pAllocator);

    while (pCurObj)
    {
        if (pCurObj == pObj)
        {
            break;
        }

        pPreObj = pCurObj;
        pCurObj = pCurObj->next;
    }

    /* Is this object being used? */
    __VK_ASSERT(pObj->refCount == 0);

    if (pPreObj == gcvNULL)
    {
        pHash->ppHashTable[pObj->entryId] = pCurObj->next;
    }
    else
    {
        pPreObj->next = pCurObj->next;
    }

    --pHash->pEntryCounts[pObj->entryId];
    pHash->pfnDeleteUserData(pAllocator, pCurObj->pUserData);
    __VK_FREE(pCurObj->pKey);
    __VK_FREE(pCurObj);
}

void __vk_utils_hashDeleteAllObjs(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash
    )
{
    uint32_t entryId;

    for (entryId = 0; entryId < pHash->tbEntryNum; ++entryId)
    {
        while (pHash->ppHashTable[entryId])
        {
            __vk_utils_hashDeleteObj(pAllocator, pHash, pHash->ppHashTable[entryId]);
        }
    }
}

__vkUtilsHashObject* __vk_utils_hashAddObj(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash,
    void *pUserData,
    const void *pKey,
    VkBool32 bPerpetual
    )
{
    __vkUtilsHashObject *pNewObj = gcvNULL;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(pAllocator);

    pNewObj = (__vkUtilsHashObject*)__VK_ALLOC(sizeof(__vkUtilsHashObject), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(pNewObj ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(pNewObj, sizeof(__vkUtilsHashObject));
    pNewObj->pUserData = pUserData;
    pNewObj->refCount = 0;
    pNewObj->year = pHash->year++;
    pNewObj->perpetual = bPerpetual;

    pNewObj->pKey = __VK_ALLOC(pHash->keyBytes, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(pNewObj->pKey ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_MEMCOPY(pNewObj->pKey, pKey, pHash->keyBytes);

    pNewObj->entryId = __vk_utils_evalCrc32(0, pKey, pHash->keyBytes) & (pHash->tbEntryNum - 1);

    /* Check if objects of this hash slot exceeds */
    if (++pHash->pEntryCounts[pNewObj->entryId] > pHash->maxEntryObjs)
    {
        uint32_t earliestYear = 0xFFFFFFFF;
        __vkUtilsHashObject *pOldestObj = gcvNULL;
        __vkUtilsHashObject *pObj = pHash->ppHashTable[pNewObj->entryId];
        while (pObj)
        {
            /* Found oldest object except perpetual ones */
            if (!pObj->perpetual && earliestYear > pObj->year)
            {
                earliestYear = pObj->year;
                pOldestObj = pObj;
            }

            pObj = pObj->next;
        }
        __vk_utils_hashDeleteObj(pAllocator, pHash, pOldestObj);
    }

    pNewObj->next = pHash->ppHashTable[pNewObj->entryId];
    pHash->ppHashTable[pNewObj->entryId] = pNewObj;

OnError:
    if (result != VK_SUCCESS && pNewObj)
    {
        if (pNewObj->pKey)
        {
            __VK_FREE(pNewObj->pKey);
        }

        __VK_FREE(pNewObj);
        pNewObj = gcvNULL;
    }

    return pNewObj;
}

__vkUtilsHashObject* __vk_utils_hashFindObjByKey(
    __vkUtilsHash *pHash,
    const void *pKey
    )
{
    uint32_t entryId = __vk_utils_evalCrc32(0, pKey, pHash->keyBytes) & (pHash->tbEntryNum - 1);
    __vkUtilsHashObject *pObj = pHash->ppHashTable[entryId];
    __vkUtilsHashObject *retObj = gcvNULL;

    while (pObj)
    {
        if (__VK_MEMCMP(pObj->pKey, pKey, pHash->keyBytes) == 0)
        {
            retObj = pObj;
            break;
        }

        pObj = pObj->next;
    }

    /* Update year to be recent one */
    if (retObj)
    {
        retObj->year = pHash->year++;
    }
    return retObj;
}

VkResult __vk_utils_hashTraverse(
    __vkUtilsHash *pHash,
    void *pContext,
    __vkUtilsProcessUserDataFunc pfnProcessUserData
    )
{
    uint32_t entryId = 0;
    VkResult result = VK_SUCCESS;

    for (entryId = 0; entryId < pHash->tbEntryNum; ++entryId)
    {
        __vkUtilsHashObject *pObj = pHash->ppHashTable[entryId];

        while (pObj)
        {
            __VK_ONERROR(pfnProcessUserData(pContext, pObj->pUserData));
            pObj = pObj->next;
        }
    }

OnError:
    return result;
}

VkResult __vk_utils_readBinary(
    VkAllocationCallbacks *pAllocator,
    gctCONST_STRING fileName,
    gctPOINTER *ppData,
    gctSIZE_T *pSize
    )
{
    gctUINT bytes = 0;
    gctFILE pFile = gcvNULL;
    gctPOINTER pData = gcvNULL;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(pAllocator);

    __VK_ONERROR(gcoOS_Open(gcvNULL, fileName, gcvFILE_READ, &pFile));
    __VK_ONERROR(gcoOS_Seek(gcvNULL, pFile, 0, gcvFILE_SEEK_END));
    __VK_ONERROR(gcoOS_GetPos(gcvNULL, pFile, &bytes));

    pData = __VK_ALLOC(bytes, 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    __VK_ONERROR(pData ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_ONERROR(gcoOS_Seek(gcvNULL, pFile, 0, gcvFILE_SEEK_SET));
    __VK_ONERROR(gcoOS_Read(gcvNULL, pFile, bytes, pData, gcvNULL));

    if (ppData)
    {
        *ppData = pData;
    }

    if (pSize)
    {
        *pSize = bytes;
    }

OnError:
    if (!__VK_IS_SUCCESS(result) && pData)
    {
        __VK_FREE(pData);
    }
    gcoOS_Close(gcvNULL, pFile);

    return result;
}

#if __VK_RESOURCE_INFO

#if __VK_RESOURCE_SAVE_TGA
static VkResult __vk_utils_saveResFile(
    __vkCommandBuffer *cmdBuf,
    __vkCmdResource *res
    )
{
    VkDevice device = VK_NULL_HANDLE;
    __vkDevContext *devCtx = gcvNULL;
    __vkImage      *pSrcImage = gcvNULL;
    __vkImageLevel *pSrcLevel = gcvNULL;
    VkImage  dstImage = VK_NULL_HANDLE;
    VkDeviceMemory dstMemory = VK_NULL_HANDLE;
    char suffix[6] = "+.tga";
    uint32_t width, height;
    void *pBaseARGB = gcvNULL;
    void *pBaseBGR  = gcvNULL;
    VkResult result = VK_SUCCESS;

    if (!res->isImage)
    {
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    pSrcImage = res->u.img.pImage;
    pSrcLevel = &pSrcImage->pImgLevels[res->u.img.subResRange.baseMipLevel];
    devCtx = pSrcImage->devCtx;
    device = (VkDevice)devCtx;

    width  = pSrcLevel->requestW;
    height = pSrcLevel->requestH;

    /* Currently NOT support 2DArray/3D output dump. */
    if (pSrcLevel->requestD > 1)
    {
        __VK_ASSERT(0);
        gcoOS_Print("Vulkan: NOT support >2D image dump\n");
    }

    do
    {
        VkImageCreateInfo imgCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            gcvNULL,
            0,
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_B8G8R8A8_UNORM,
            {width, height, 1},
            1,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_TILING_LINEAR,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            gcvNULL,
            VK_IMAGE_LAYOUT_UNDEFINED
        };

        VkImageSubresource imgSubRes = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};

        VkMemoryAllocateInfo memAllocInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            gcvNULL,
            0,
            0
        };

        VkCommandBufferBeginInfo cmdBeginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            gcvNULL,
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            gcvNULL
        };

        VkImageBlit blitRegion = {
            {VK_IMAGE_ASPECT_COLOR_BIT, res->u.img.subResRange.baseMipLevel, res->u.img.subResRange.baseArrayLayer, 1},
            {{0, 0, 0}, {width, height, 1}},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            {{0, 0, 0}, {width, height, 1}}
        };

        VkSubmitInfo submitInfo = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            gcvNULL,
            0,
            gcvNULL,
            gcvNULL,
            1,
            gcvNULL,
            0,
            gcvNULL
        };

        VkMemoryRequirements imgMemReq;
        VkSubresourceLayout imgLayout;

        uint8_t *pBGR;
        uint8_t *pARGB;
        uint32_t x, y;

        if (!devCtx->auxCmdPool)
        {
            VkCommandPoolCreateInfo poolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                gcvNULL,
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                0
            };
            VkCommandBufferAllocateInfo cmdAllocInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                gcvNULL,
                VK_NULL_HANDLE,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1
            };

            __vk_GetDeviceQueue(device, 0, 0, &devCtx->auxQueue);

            __VK_ERR_BREAK(__vk_CreateCommandPool(device, &poolCreateInfo, gcvNULL, &devCtx->auxCmdPool));

            cmdAllocInfo.commandPool = devCtx->auxCmdPool;
            __VK_ERR_BREAK(__vk_AllocateCommandBuffers(device, &cmdAllocInfo, &devCtx->auxCmdBuf));
            ((__vkCommandBuffer*)(devCtx->auxCmdBuf))->skipSaveTGA = VK_TRUE;
        }

        __VK_ERR_BREAK(__vk_CreateImage(device, &imgCreateInfo, gcvNULL, &dstImage));
        __vk_GetImageMemoryRequirements(device, dstImage, &imgMemReq);

        memAllocInfo.allocationSize = imgMemReq.size;
        __VK_ERR_BREAK(__vk_AllocateMemory(device, &memAllocInfo, gcvNULL, &dstMemory));
        __VK_ERR_BREAK(__vk_BindImageMemory(device, dstImage, dstMemory, 0));

        __VK_ERR_BREAK(__vk_BeginCommandBuffer(devCtx->auxCmdBuf, &cmdBeginInfo));
        __vk_CmdBlitImage(devCtx->auxCmdBuf,
                          (VkImage)pSrcImage, VK_IMAGE_LAYOUT_GENERAL,
                          dstImage, VK_IMAGE_LAYOUT_GENERAL,
                          1, &blitRegion, VK_FILTER_NEAREST);
        __VK_ERR_BREAK(__vk_EndCommandBuffer(devCtx->auxCmdBuf));

        submitInfo.pCommandBuffers = &devCtx->auxCmdBuf;
        __VK_ERR_BREAK(__vk_QueueSubmit(devCtx->auxQueue, 1, &submitInfo, VK_NULL_HANDLE));
        __VK_ERR_BREAK(__vk_QueueWaitIdle(devCtx->auxQueue));

        __VK_ERR_BREAK(__vk_MapMemory(device, dstMemory, 0, imgMemReq.size, 0, &pBaseARGB));
        __vk_GetImageSubresourceLayout(device, dstImage, &imgSubRes, &imgLayout);

        gcoOS_Allocate(gcvNULL, width * height * 3, &pBaseBGR);

        /* Color format conversion: ARGB to RGB. */
        pBGR = (gctUINT8_PTR)pBaseBGR;

        suffix[0] = '-';

        for (y = 0; y < height; ++y)
        {
            pARGB = (uint8_t*)pBaseARGB + y * imgLayout.rowPitch;

            for (x = 0; x < width; ++x)
            {
                pBGR[0] = pARGB[0];
                pBGR[1] = pARGB[1];
                pBGR[2] = pARGB[2];

                pARGB += 4;
                pBGR  += 3;
            }
        }
    }
    while (VK_FALSE);

    {
        gctFILE file = gcvNULL;
        char fName[gcdMAX_PATH] = {0};
        gctUINT8 tgaHeader[18];
        static uint32_t sFileSeq = 0;

        /* Prepare tga file header. */
        __VK_MEMZERO(tgaHeader, sizeof(tgaHeader));
        tgaHeader[ 2] = 2;
        tgaHeader[12] = (width  & 0x00ff);
        tgaHeader[13] = (width  & 0xff00) >> 8;
        tgaHeader[14] = (height & 0x00ff);
        tgaHeader[15] = (height & 0xff00) >> 8;
        tgaHeader[16] = 24;
        tgaHeader[17] = (0x01 << 5);

        __VK_VERIFY_OK(gcoOS_PrintStrSafe(fName, gcdMAX_PATH, gcvNULL, "f%04u_%s%s",
                                          sFileSeq++, res->pTag, suffix));

        /* Open tga file for write. */
        __VK_VERIFY_OK(gcoOS_Open(gcvNULL, fName, gcvFILE_CREATE, &file));

        /* Write tga file header. */
        __VK_VERIFY_OK(gcoOS_Write(gcvNULL, file, 18, tgaHeader));

        if (pBaseBGR)
        {
            /* Write pixel data. */
            __VK_VERIFY_OK(gcoOS_Write(gcvNULL, file, width * height * 3, pBaseBGR));
        }

        if (gcvNULL != file)
        {
            /* Close tga file. */
            __VK_VERIFY_OK(gcoOS_Close(gcvNULL, file));
        }
    }

    if (pBaseBGR)
    {
        gcoOS_Free(gcvNULL, pBaseBGR);
    }

    if (dstImage)
    {
        __vk_DestroyImage(device, dstImage, &devCtx->memCb);
    }

    if (dstMemory)
    {
        if (pBaseARGB)
        {
            __vk_UnmapMemory(device, dstMemory);
        }
        __vk_FreeMemory(device, dstMemory, &devCtx->memCb);
    }

    return result;
}
#endif

void __vk_utils_insertCmdRes(
    __vkCmdResNode **ppResLists,
    __vkCmdResource *res,
    VkAllocationCallbacks *pAllocator
    )
{
    VkBool32 found = VK_FALSE;
    __vkCmdResNode *pResNode = *ppResLists;

    if (!res || !pAllocator)
    {
        return;
    }

    if (res->isImage ? !res->u.img.pImage : !res->u.buf.pBuffer)
    {
        return;
    }

    while (pResNode)
    {
        if (0 == __VK_MEMCMP(&pResNode->res, res, sizeof(__vkCmdResource)))
        {
            found = VK_TRUE;
        }

        pResNode = pResNode->next;
    }

    if (!found)
    {
        __VK_SET_ALLOCATIONCB(pAllocator);

        pResNode = (__vkCmdResNode*)__VK_ALLOC(sizeof(__vkCmdResNode), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        __VK_MEMCOPY(&pResNode->res, res, sizeof(__vkCmdResource));
        gcoOS_StrCopySafe(pResNode->tag, gcdMAX_PATH, res->pTag);
        pResNode->res.pTag = pResNode->tag;
        pResNode->next = (*ppResLists);
        *ppResLists = pResNode;
    }
}


void __vk_utils_insertDescSetRes(
    __vkCommandBuffer *cmdBuf,
    __vkCmdBindDescSetInfo *descSetInfo,
    const char *op
)
{
    uint32_t setIdx;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);
    __vkCmdResource cmdRes;
    char tag[gcdMAX_PATH];

    for (setIdx = 0; setIdx < __VK_MAX_DESCRIPTOR_SETS; ++setIdx)
        {
            __vkDescriptorSet *descSet = descSetInfo->descSets[setIdx];
            uint32_t *dynamicOffsets = descSetInfo->dynamicOffsets[setIdx];
            uint32_t dynamicOffsetIdx = 0;

            if (descSet)
            {
                uint32_t bindIdx, arrayIdx;
                __vkDescriptorSetLayout *descSetLayout = descSet->descSetLayout;

                for (bindIdx = 0; bindIdx < descSetLayout->bindingCount; ++bindIdx)
                {
                    __vkDescriptorResourceInfo *resInfo;
                    __vkDescriptorResourceRegion curDescRegion;
                    __vkDescriptorSetLayoutBinding *binding = &descSetLayout->binding[bindIdx];

                    switch (binding->std.descriptorType)
                    {
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = resInfo->u.bufferInfo.buffer;
                                cmdRes.u.buf.offset  = resInfo->u.bufferInfo.offset;
                                cmdRes.u.buf.range   = resInfo->u.bufferInfo.range;

                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = resInfo->u.bufferInfo.buffer;
                                cmdRes.u.buf.offset  = resInfo->u.bufferInfo.offset + dynamicOffsets[dynamicOffsetIdx++];
                                cmdRes.u.buf.range   = resInfo->u.bufferInfo.range;

                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformBufDynamic=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = resInfo->u.bufferInfo.buffer;
                                cmdRes.u.buf.offset  = resInfo->u.bufferInfo.offset;
                                cmdRes.u.buf.range   = resInfo->u.bufferInfo.range;

                                /* Storage buffer can used as either input or output */
                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_storageBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = resInfo->u.bufferInfo.buffer;
                                cmdRes.u.buf.offset  = resInfo->u.bufferInfo.offset + dynamicOffsets[dynamicOffsetIdx++];
                                cmdRes.u.buf.range   = resInfo->u.bufferInfo.range;

                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageBufDynamic=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vkBufferView *bufView;
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);
                            bufView = resInfo->u.bufferView;

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bufView->createInfo.buffer);
                                cmdRes.u.buf.offset  = bufView->createInfo.offset;
                                cmdRes.u.buf.range   = bufView->createInfo.range;

                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformTexBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vkBufferView *bufView;
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);
                            bufView = resInfo->u.bufferView;

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_FALSE;
                                cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bufView->createInfo.buffer);
                                cmdRes.u.buf.offset  = bufView->createInfo.offset;
                                cmdRes.u.buf.range   = bufView->createInfo.range;

                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageTexBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
                        cmdRes.pTag = tag;
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; ++arrayIdx)
                        {
                            __vkImageView *imgView;
                            __vk_utils_region_mad(&curDescRegion, &binding->perElementSize, arrayIdx, &binding->offset);
                            resInfo = (__vkDescriptorResourceInfo*)((uint8_t*)descSet->resInfos + curDescRegion.resource);
                            imgView = resInfo->u.imageInfo.imageView;

                            if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                cmdRes.isImage = VK_TRUE;
                                cmdRes.u.img.pImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgView->createInfo.image);
                                cmdRes.u.img.subResRange = imgView->createInfo.subresourceRange;
                                cmdRes.u.img.subResRange.aspectMask = 0;

                                /* Storage image can used as either input or output */
                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_storageImg=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.img.pImage->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                                gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageImg=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.img.pImage->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                        break;

                    default:
                        break;
                    }
                }
            }
    }
}

void __vk_utils_insertDrawCmdRes(
    VkCommandBuffer commandBuffer,
    VkBuffer indirectBuf
    )
{
    uint32_t i;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer*)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);
    __vkCmdBindInfo *bindInfo = &cmdBuf->bindInfo;
    __vkRenderSubPassInfo *subPass = bindInfo->renderPass.subPass;
    __vkFramebuffer *fb = bindInfo->renderPass.fb;
    __vkCmdResource cmdRes;
    char tag[gcdMAX_PATH];
    const char *op = "draw";

    /* Dump Input Res */
    __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
    cmdRes.pTag = tag;
    cmdRes.isImage = VK_FALSE;
    cmdRes.u.buf.offset = 0;
    cmdRes.u.buf.range = VK_WHOLE_SIZE;

    if (indirectBuf)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, indirectBuf);
        gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indirectBuf=%d",
                           cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    for (i = bindInfo->vertexBuffers.firstBinding;
         i < bindInfo->vertexBuffers.firstBinding + bindInfo->vertexBuffers.bindingCount;
         ++i)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bindInfo->vertexBuffers.buffers[i]);
        gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_vertexBuf%u=%d",
                           cmdBuf->obj.id, op, i, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    if (bindInfo->indexBuffer.buffer)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bindInfo->indexBuffer.buffer);
        gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indexBuf=%d",
                           cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    __vk_utils_insertDescSetRes(cmdBuf, &bindInfo->bindDescSet.graphics, op);

    __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
    cmdRes.pTag = tag;
    cmdRes.isImage = VK_TRUE;

    for (i = 0; i < subPass->colorCount; ++i)
    {
        if (subPass->color_attachment_index[i] != VK_ATTACHMENT_UNUSED)
        {
            __vkImageView *colorView = fb->imageViews[subPass->color_attachment_index[i]];

            cmdRes.u.img.pImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, colorView->createInfo.image);
            cmdRes.u.img.subResRange = colorView->createInfo.subresourceRange;
            cmdRes.u.img.subResRange.aspectMask = 0;

            gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_color%u=%d",
                               cmdBuf->obj.id, op, i, cmdRes.u.img.pImage->obj.id);
            __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
        }
    }

    if (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        __vkImageView *dsView = fb->imageViews[subPass->dsAttachIndex];

        cmdRes.u.img.pImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dsView->createInfo.image);
        cmdRes.u.img.subResRange = dsView->createInfo.subresourceRange;
        cmdRes.u.img.subResRange.aspectMask = 0;

        gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_depth=%d",
                           cmdBuf->obj.id, op, cmdRes.u.img.pImage->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
    }
}

void __vk_utils_insertComputeCmdRes(
    VkCommandBuffer commandBuffer,
    VkBuffer indirectBuf
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer*)commandBuffer;
    const char *op = "compute";

    if (indirectBuf)
    {
        __vkCmdResource cmdRes;
        char tag[gcdMAX_PATH];
        __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);

        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
        cmdRes.pTag = tag;
        cmdRes.isImage = VK_FALSE;
        cmdRes.u.buf.offset = 0;
        cmdRes.u.buf.range = VK_WHOLE_SIZE;
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, indirectBuf);

        gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indirectBuf=%d",
                           cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    __vk_utils_insertDescSetRes(cmdBuf, &cmdBuf->bindInfo.bindDescSet.compute, op);
}

void __vk_utils_insertCopyCmdRes(
    VkCommandBuffer commandBuffer,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer*)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);
    __vkCmdResource cmdRes;
    char tag[gcdMAX_PATH];

    if (srcRes)
    {
        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));

        cmdRes.pTag = tag;
        cmdRes.isImage = srcRes->isImage;
        if (cmdRes.isImage)
        {
            cmdRes.u.img.pImage = srcRes->u.img.pImage;
            cmdRes.u.img.subResRange.aspectMask     = 0;
            cmdRes.u.img.subResRange.baseMipLevel   = srcRes->u.img.subRes.mipLevel;
            cmdRes.u.img.subResRange.levelCount     = 1;
            cmdRes.u.img.subResRange.baseArrayLayer = srcRes->u.img.subRes.arrayLayer;
            cmdRes.u.img.subResRange.layerCount     = 1;

            gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_input_image=%d",
                               cmdBuf->obj.id, cmdRes.u.img.pImage->obj.id);
        }
        else
        {
            cmdRes.u.buf.pBuffer = srcRes->u.buf.pBuffer;
            cmdRes.u.buf.offset  = srcRes->u.buf.offset;
            cmdRes.u.buf.range   = VK_WHOLE_SIZE;

            gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_input_buffer=%d",
                               cmdBuf->obj.id, cmdRes.u.buf.pBuffer->obj.id);
        }

        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    if (dstRes)
    {
        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));

        cmdRes.pTag = tag;
        cmdRes.isImage = dstRes->isImage;
        if (cmdRes.isImage)
        {
            cmdRes.u.img.pImage = dstRes->u.img.pImage;
            cmdRes.u.img.subResRange.aspectMask     = 0;
            cmdRes.u.img.subResRange.baseMipLevel   = dstRes->u.img.subRes.mipLevel;
            cmdRes.u.img.subResRange.levelCount     = 1;
            cmdRes.u.img.subResRange.baseArrayLayer = dstRes->u.img.subRes.arrayLayer;
            cmdRes.u.img.subResRange.layerCount     = 1;

            gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_output_image=%d",
                               cmdBuf->obj.id, cmdRes.u.img.pImage->obj.id);
        }
        else
        {
            cmdRes.u.buf.pBuffer = dstRes->u.buf.pBuffer;
            cmdRes.u.buf.offset  = dstRes->u.buf.offset;
            cmdRes.u.buf.range   = VK_WHOLE_SIZE;

            gcoOS_PrintStrSafe(tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_output_buffer=%d",
                               cmdBuf->obj.id, cmdRes.u.buf.pBuffer->obj.id);
        }

        __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
    }
}

void __vk_utils_mergeCmdRes(
    __vkCommandBuffer *primary,
    __vkCommandBuffer *secondary
    )
{
    __vkCmdResNode *pResNode = gcvNULL;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, primary->commandPool);

    pResNode = secondary->inputs;
    while (pResNode)
    {
        __vk_utils_insertCmdRes(&primary->inputs, &pResNode->res, &cdp->memCb);

        pResNode = pResNode->next;
    }

    pResNode = secondary->outputs;
    while (pResNode)
    {
        __vk_utils_insertCmdRes(&primary->outputs, &pResNode->res, &cdp->memCb);

        pResNode = pResNode->next;
    }

}

void __vk_utils_dumpCmdInputRes(
    __vkCommandBuffer *cmdBuf
    )
{
#if gcdDUMP
    typedef struct __vkMemNodeRec {
        __vkDeviceMemory *pMemory;
        struct __vkMemNodeRec *next;
    } __vkMemNode;

    __vkMemNode *pMemHead = gcvNULL;
    __vkMemNode *pMemNode = gcvNULL;
    __vkCmdResNode *pResNode = cmdBuf->inputs;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    while (pResNode)
    {
        VkBool32 found = VK_FALSE;
        __vkDeviceMemory *pMemory;

        if (pResNode->res.isImage)
        {
            __vkImage *pImage = pResNode->res.u.img.pImage;
            pMemory = pImage->memory;

            /* Theoretically, only linear image can be mapped by apps */
            __VK_ASSERT(!pMemory->mapped || pImage->createInfo.tiling == VK_IMAGE_TILING_LINEAR);
        }
        else
        {
            __vkBuffer *pBuffer = pResNode->res.u.buf.pBuffer;
            pMemory = pBuffer->memory;
        }

        pMemNode = pMemHead;
        while (pMemNode)
        {
            if (pMemNode->pMemory == pMemory)
            {
                found = VK_TRUE;
            }

            pMemNode = pMemNode->next;
        }

        if (!found)
        {
            pMemNode = (__vkMemNode*)__VK_ALLOC(sizeof(__vkMemNode), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
            pMemNode->pMemory = pMemory;
            pMemNode->next = pMemHead;
            pMemHead = pMemNode;

            /* Only dump the mapped range once */
            if (pMemory->mapped)
            {
                gcmDUMP(gcvNULL, "#[info: update memory: %s", pResNode->res.pTag);
                gcmDUMP_BUFFER(gcvNULL,
                               "memory",
                               pMemory->devAddr,
                               pMemory->hostAddr,
                               (gctUINT)pMemory->mappedOffset,
                               (gctUINT)pMemory->mappedSize);
            }
        }

        pResNode = pResNode->next;
    }

    pMemNode = pMemHead;
    while (pMemNode)
    {
        __vkMemNode *pNextNode = pMemNode->next;

        __VK_FREE(pMemNode);

        pMemNode = pNextNode;
    }
#endif
}

void __vk_utils_dumpCmdOutputRes(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkCmdResNode *pResNode = cmdBuf->outputs;

    while (pResNode)
    {
#if gcdDUMP
        gctUINT32 physical;
        gctPOINTER logical;
        gctSIZE_T offset;
        gctSIZE_T size;

        if (pResNode->res.isImage)
        {
            uint32_t partIdx;
            __vkImage *pImage = pResNode->res.u.img.pImage;
            VkImageSubresourceRange *pSubResRange = &pResNode->res.u.img.subResRange;
            __vkImageLevel *pLevel = &pImage->pImgLevels[pSubResRange->baseMipLevel];

            physical = pImage->memory->devAddr;
            logical = pImage->memory->hostAddr;
            size = (uint32_t)(pSubResRange->layerCount * pLevel->sliceSize);

            for (partIdx = 0; partIdx < pImage->formatInfo.partCount; ++partIdx)
            {
                offset = (uint32_t)(pImage->memOffset +
                                    pLevel->offset +
                                    pLevel->partSize * partIdx +
                                    pLevel->sliceSize * pSubResRange->baseArrayLayer);

                gcmDUMP(gcvNULL, "#[info: verify memory: %s", pResNode->res.pTag);
                gcmDUMP_BUFFER(gcvNULL, "verify", physical, logical, offset, size);
            }
        }
        else
        {
            __vkBuffer *pBuffer = pResNode->res.u.buf.pBuffer;

            physical = pBuffer->memory->devAddr;
            logical = pBuffer->memory->hostAddr;

            offset = (uint32_t)(pBuffer->memOffset + pResNode->res.u.buf.offset);
            size = (VK_WHOLE_SIZE == pResNode->res.u.buf.range)
                 ? (gctSIZE_T)(pBuffer->createInfo.size - pResNode->res.u.buf.offset)
                 : (gctSIZE_T)pResNode->res.u.buf.range;

            gcmDUMP(gcvNULL, "#[info: verify memory: %s", pResNode->res.pTag);
            gcmDUMP_BUFFER(gcvNULL, "verify", physical, logical, offset, size);
        }
#endif

#if __VK_RESOURCE_SAVE_TGA
        if (!cmdBuf->skipSaveTGA && pResNode->res.isImage)
        {
            __vk_utils_saveResFile(cmdBuf, &pResNode->res);
        }
#endif

        pResNode = pResNode->next;
    }
}


void __vk_utils_freeCmdRes(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkCmdResNode *pResNode = gcvNULL;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    pResNode = cmdBuf->inputs;
    while (pResNode)
    {
        __vkCmdResNode *pNextNode = pResNode->next;

        __VK_FREE(pResNode);

        pResNode = pNextNode;
    }
    cmdBuf->inputs = gcvNULL;

    pResNode = cmdBuf->outputs;
    while (pResNode)
    {
        __vkCmdResNode *pNextNode = pResNode->next;

        __VK_FREE(pResNode);

        pResNode = pNextNode;
    }
    cmdBuf->outputs = gcvNULL;
}

#endif

void __vk_utils_reverseBytes(
    const char *source,
    char *dest,
    uint32_t destSize
    )
{
    char *p;

    gcoOS_StrCopySafe(dest, destSize, source);
    p = dest;
    while (*p)
    {
        *p = ~(*p);
        p++;
    }
}


/*
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#ifndef __VK_BIG_ENDIAN
#define __vk_utils_swapUINT(data, uints)   /* Nothing */
#else
static void __vk_utils_swapUINT(uint8_t *data, size_t uints)
{
    do
    {
        uint32_t swapped = ((uint32_t)data[3]) << 24
                         | ((uint32_t)data[2]) << 16
                         | ((uint32_t)data[1]) << 8
                         | ((uint32_t)data[0])
        *(uint32_t*)data = swapped;

        data  += 4;
    } while (--uints);
}
#endif

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void __vk_utils_MD5Transform(
    __VkUtilsMD5Context *ctx,
    uint32_t data[16]
    )
{
    uint32_t *p = data;
    register uint32_t a, b, c, d;

    a = ctx->states[0];
    b = ctx->states[1];
    c = ctx->states[2];
    d = ctx->states[3];

#ifdef __VK_BIG_ENDIAN
    /* App data need to be copied to scratch before byte swap. */
    if (p != (uint32_t*)ctx->buffer)
    {
        static uint32_t scratch[16];
        __VK_MEMCOPY(scratch, data, sizeof(uint32_t) * 16);
        p = scratch;
    }
#endif
    __vk_utils_swapUINT((uint8_t*)p, 16);

#define __VK_MD5STEP(f, w, x, y, z, data, s) \
  (w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x )

#define __VK_F1(x, y, z) (z ^ (x & (y ^ z)))
    __VK_MD5STEP(__VK_F1, a, b, c, d, p[ 0] + 0xd76aa478, 7);
    __VK_MD5STEP(__VK_F1, d, a, b, c, p[ 1] + 0xe8c7b756, 12);
    __VK_MD5STEP(__VK_F1, c, d, a, b, p[ 2] + 0x242070db, 17);
    __VK_MD5STEP(__VK_F1, b, c, d, a, p[ 3] + 0xc1bdceee, 22);
    __VK_MD5STEP(__VK_F1, a, b, c, d, p[ 4] + 0xf57c0faf, 7);
    __VK_MD5STEP(__VK_F1, d, a, b, c, p[ 5] + 0x4787c62a, 12);
    __VK_MD5STEP(__VK_F1, c, d, a, b, p[ 6] + 0xa8304613, 17);
    __VK_MD5STEP(__VK_F1, b, c, d, a, p[ 7] + 0xfd469501, 22);
    __VK_MD5STEP(__VK_F1, a, b, c, d, p[ 8] + 0x698098d8, 7);
    __VK_MD5STEP(__VK_F1, d, a, b, c, p[ 9] + 0x8b44f7af, 12);
    __VK_MD5STEP(__VK_F1, c, d, a, b, p[10] + 0xffff5bb1, 17);
    __VK_MD5STEP(__VK_F1, b, c, d, a, p[11] + 0x895cd7be, 22);
    __VK_MD5STEP(__VK_F1, a, b, c, d, p[12] + 0x6b901122, 7);
    __VK_MD5STEP(__VK_F1, d, a, b, c, p[13] + 0xfd987193, 12);
    __VK_MD5STEP(__VK_F1, c, d, a, b, p[14] + 0xa679438e, 17);
    __VK_MD5STEP(__VK_F1, b, c, d, a, p[15] + 0x49b40821, 22);
#undef __VK_F1

#define __VK_F2(x, y, z) (y ^ (z & (x ^ y)))
    __VK_MD5STEP(__VK_F2, a, b, c, d, p[ 1] + 0xf61e2562, 5);
    __VK_MD5STEP(__VK_F2, d, a, b, c, p[ 6] + 0xc040b340, 9);
    __VK_MD5STEP(__VK_F2, c, d, a, b, p[11] + 0x265e5a51, 14);
    __VK_MD5STEP(__VK_F2, b, c, d, a, p[ 0] + 0xe9b6c7aa, 20);
    __VK_MD5STEP(__VK_F2, a, b, c, d, p[ 5] + 0xd62f105d, 5);
    __VK_MD5STEP(__VK_F2, d, a, b, c, p[10] + 0x02441453, 9);
    __VK_MD5STEP(__VK_F2, c, d, a, b, p[15] + 0xd8a1e681, 14);
    __VK_MD5STEP(__VK_F2, b, c, d, a, p[ 4] + 0xe7d3fbc8, 20);
    __VK_MD5STEP(__VK_F2, a, b, c, d, p[ 9] + 0x21e1cde6, 5);
    __VK_MD5STEP(__VK_F2, d, a, b, c, p[14] + 0xc33707d6, 9);
    __VK_MD5STEP(__VK_F2, c, d, a, b, p[ 3] + 0xf4d50d87, 14);
    __VK_MD5STEP(__VK_F2, b, c, d, a, p[ 8] + 0x455a14ed, 20);
    __VK_MD5STEP(__VK_F2, a, b, c, d, p[13] + 0xa9e3e905, 5);
    __VK_MD5STEP(__VK_F2, d, a, b, c, p[ 2] + 0xfcefa3f8, 9);
    __VK_MD5STEP(__VK_F2, c, d, a, b, p[ 7] + 0x676f02d9, 14);
    __VK_MD5STEP(__VK_F2, b, c, d, a, p[12] + 0x8d2a4c8a, 20);
#undef __VK_F2

#define __VK_F3(x, y, z) (x ^ y ^ z)
    __VK_MD5STEP(__VK_F3, a, b, c, d, p[ 5] + 0xfffa3942, 4);
    __VK_MD5STEP(__VK_F3, d, a, b, c, p[ 8] + 0x8771f681, 11);
    __VK_MD5STEP(__VK_F3, c, d, a, b, p[11] + 0x6d9d6122, 16);
    __VK_MD5STEP(__VK_F3, b, c, d, a, p[14] + 0xfde5380c, 23);
    __VK_MD5STEP(__VK_F3, a, b, c, d, p[ 1] + 0xa4beea44, 4);
    __VK_MD5STEP(__VK_F3, d, a, b, c, p[ 4] + 0x4bdecfa9, 11);
    __VK_MD5STEP(__VK_F3, c, d, a, b, p[ 7] + 0xf6bb4b60, 16);
    __VK_MD5STEP(__VK_F3, b, c, d, a, p[10] + 0xbebfbc70, 23);
    __VK_MD5STEP(__VK_F3, a, b, c, d, p[13] + 0x289b7ec6, 4);
    __VK_MD5STEP(__VK_F3, d, a, b, c, p[ 0] + 0xeaa127fa, 11);
    __VK_MD5STEP(__VK_F3, c, d, a, b, p[ 3] + 0xd4ef3085, 16);
    __VK_MD5STEP(__VK_F3, b, c, d, a, p[ 6] + 0x04881d05, 23);
    __VK_MD5STEP(__VK_F3, a, b, c, d, p[ 9] + 0xd9d4d039, 4);
    __VK_MD5STEP(__VK_F3, d, a, b, c, p[12] + 0xe6db99e5, 11);
    __VK_MD5STEP(__VK_F3, c, d, a, b, p[15] + 0x1fa27cf8, 16);
    __VK_MD5STEP(__VK_F3, b, c, d, a, p[ 2] + 0xc4ac5665, 23);
#undef __VK_F3

#define __VK_F4(x, y, z) (y ^ (x | ~z))
    __VK_MD5STEP(__VK_F4, a, b, c, d, p[ 0] + 0xf4292244, 6);
    __VK_MD5STEP(__VK_F4, d, a, b, c, p[ 7] + 0x432aff97, 10);
    __VK_MD5STEP(__VK_F4, c, d, a, b, p[14] + 0xab9423a7, 15);
    __VK_MD5STEP(__VK_F4, b, c, d, a, p[ 5] + 0xfc93a039, 21);
    __VK_MD5STEP(__VK_F4, a, b, c, d, p[12] + 0x655b59c3, 6);
    __VK_MD5STEP(__VK_F4, d, a, b, c, p[ 3] + 0x8f0ccc92, 10);
    __VK_MD5STEP(__VK_F4, c, d, a, b, p[10] + 0xffeff47d, 15);
    __VK_MD5STEP(__VK_F4, b, c, d, a, p[ 1] + 0x85845dd1, 21);
    __VK_MD5STEP(__VK_F4, a, b, c, d, p[ 8] + 0x6fa87e4f, 6);
    __VK_MD5STEP(__VK_F4, d, a, b, c, p[15] + 0xfe2ce6e0, 10);
    __VK_MD5STEP(__VK_F4, c, d, a, b, p[ 6] + 0xa3014314, 15);
    __VK_MD5STEP(__VK_F4, b, c, d, a, p[13] + 0x4e0811a1, 21);
    __VK_MD5STEP(__VK_F4, a, b, c, d, p[ 4] + 0xf7537e82, 6);
    __VK_MD5STEP(__VK_F4, d, a, b, c, p[11] + 0xbd3af235, 10);
    __VK_MD5STEP(__VK_F4, c, d, a, b, p[ 2] + 0x2ad7d2bb, 15);
    __VK_MD5STEP(__VK_F4, b, c, d, a, p[ 9] + 0xeb86d391, 21);
#undef __VK_F4

#undef __VK_MD5STEP

    ctx->states[0] += a;
    ctx->states[1] += b;
    ctx->states[2] += c;
    ctx->states[3] += d;
}


void __vk_uitls_MD5Init(
    __VkUtilsMD5Context *ctx
    )
{
    __VK_MEMZERO(ctx, sizeof(__VkUtilsMD5Context));

    ctx->bytes = 0;

    /* mysterious initial constants */
    ctx->states[0] = 0x67452301;
    ctx->states[1] = 0xefcdab89;
    ctx->states[2] = 0x98badcfe;
    ctx->states[3] = 0x10325476;
}

/* Update context to reflect the concatenation of another buffer full of bytes */
void __vk_utils_MD5Update(
    __VkUtilsMD5Context *ctx,
    const void *data,
    size_t bytes
    )
{
    const uint8_t *buf = (const uint8_t*)data;
    size_t left = ctx->bytes & 0x3F;
    size_t fill = 64 - left;

    ctx->bytes += bytes;

    /* Handle any leading odd-sized chunks */
    if (left > 0 && fill <= bytes)
    {
        __VK_MEMCOPY(&ctx->buffer[left], buf, fill);

        __vk_utils_MD5Transform(ctx, (uint32_t*)ctx->buffer);

        buf   += fill;
        bytes -= fill;
        left = 0;

    }

    /* Process data in 64-bytes chunks */
    while (bytes >= 64)
    {
        __vk_utils_MD5Transform(ctx, (uint32_t*)buf);
        buf   += 64;
        bytes -= 64;
    }

    /* Copy any remaining bytes for next process */
    if (bytes > 0)
    {
        __VK_MEMCOPY(&ctx->buffer[left], buf, bytes);
    }
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void __vk_utils_MD5Final(
    __VkUtilsMD5Context *ctx,
    uint8_t digest[16]
    )
{
    size_t left, fill;
    uint64_t bits = 0;

    left = ctx->bytes & 0x3F;
    ctx->buffer[left++] = 0x80;     /* padding 0x80*/
    fill = 64 - left;

    if (fill > 0)
    {
        __VK_MEMZERO(&ctx->buffer[left], fill);
    }

    if (fill < sizeof(bits))
    {
        __vk_utils_MD5Transform(ctx, (uint32_t*)ctx->buffer);
        __VK_MEMZERO(ctx->buffer, 64);
        left = 0;
    }

    /* Append length in bits and transform */
    bits = ctx->bytes << 3;
    __VK_MEMCOPY(&ctx->buffer[left], &bits, sizeof(bits));

    __vk_utils_MD5Transform(ctx, (uint32_t*)ctx->buffer);
    __vk_utils_swapUINT((uint8_t*)ctx->states, 4);
    __VK_MEMCOPY(digest, ctx->states, 16);
}



