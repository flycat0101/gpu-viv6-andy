/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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

#if defined(ANDROID)
    {
        enum DUMP_FLAG
        {
            DUMP_UNINITIALIZED,
            DUMP_TRUE,
            DUMP_FALSE,
        };
        static gctINT dump = DUMP_UNINITIALIZED;

        if (DUMP_UNINITIALIZED == dump)
        {
            if (gcvSTATUS_TRUE == gcoOS_DetectProcessByName(gcdDUMP_KEY))
            {
                dump = DUMP_TRUE;
            }
            else
            {
                dump = DUMP_FALSE;
            }
        }

        if (DUMP_TRUE != dump)
        {
            return VK_SUCCESS;
        }
    }
#endif

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

        __VK_ONERROR(gcoOS_Allocate(gcvNULL, width * height * 3, &pBaseBGR));

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

        __VK_VERIFY_OK(gcoOS_PrintStrSafe(fName, gcdMAX_PATH, gcvNULL, "%sf%04u_%s%s",
                                          gcdDUMP_PATH ,sFileSeq++, res->tag, suffix));

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

OnError:
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
    uint32_t dirtyMask;
    uint32_t setIdx = 0;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);
    __vkCmdResource cmdRes;

    dirtyMask = descSetInfo->dirtyMask;

    while (dirtyMask)
    {
        if (dirtyMask & (1 << setIdx))
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

                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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

                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformBufDynamic=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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
                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_storageBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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

                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageBufDynamic=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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

                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_uniformTexBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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

                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageTexBuf=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->outputs, &cmdRes, &cdp->memCb);
                            }
                        }
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
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
                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_storageImg=%d",
                                                   cmdBuf->obj.id, op, cmdRes.u.img.pImage->obj.id);
                                __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
                                gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_storageImg=%d",
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
            dirtyMask &= ~(1 << setIdx);
        }
        setIdx++;
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
    const char *op = "draw";

    /* Dump Input Res */
    __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
    cmdRes.isImage = VK_FALSE;
    cmdRes.u.buf.offset = 0;
    cmdRes.u.buf.range = VK_WHOLE_SIZE;

    if (indirectBuf)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, indirectBuf);
        gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indirectBuf=%d",
                           cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    for (i = bindInfo->vertexBuffers.firstBinding;
         i < bindInfo->vertexBuffers.firstBinding + bindInfo->vertexBuffers.bindingCount;
         ++i)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bindInfo->vertexBuffers.buffers[i]);
        gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_vertexBuf%u=%d",
                           cmdBuf->obj.id, op, i, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    if (bindInfo->indexBuffer.buffer)
    {
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bindInfo->indexBuffer.buffer);
        gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indexBuf=%d",
                           cmdBuf->obj.id, op, cmdRes.u.buf.pBuffer->obj.id);
        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    __vk_utils_insertDescSetRes(cmdBuf, &bindInfo->bindDescSet.graphics, op);

    __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
    cmdRes.isImage = VK_TRUE;

    for (i = 0; i < subPass->colorCount; ++i)
    {
        if (subPass->color_attachment_index[i] != VK_ATTACHMENT_UNUSED)
        {
            __vkImageView *colorView = fb->imageViews[subPass->color_attachment_index[i]];

            cmdRes.u.img.pImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, colorView->createInfo.image);
            cmdRes.u.img.subResRange = colorView->createInfo.subresourceRange;
            cmdRes.u.img.subResRange.aspectMask = 0;

            gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_color%u=%d",
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

        gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_output_depth=%d",
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
        __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool*, cmdBuf->commandPool);

        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));
        cmdRes.isImage = VK_FALSE;
        cmdRes.u.buf.offset = 0;
        cmdRes.u.buf.range = VK_WHOLE_SIZE;
        cmdRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, indirectBuf);

        gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_%s_input_indirectBuf=%d",
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

    if (srcRes)
    {
        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));

        cmdRes.isImage = srcRes->isImage;
        if (cmdRes.isImage)
        {
            cmdRes.u.img.pImage = srcRes->u.img.pImage;
            cmdRes.u.img.subResRange.aspectMask     = 0;
            cmdRes.u.img.subResRange.baseMipLevel   = srcRes->u.img.subRes.mipLevel;
            cmdRes.u.img.subResRange.levelCount     = 1;
            cmdRes.u.img.subResRange.baseArrayLayer = srcRes->u.img.subRes.arrayLayer;
            cmdRes.u.img.subResRange.layerCount     = 1;

            gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_input_image=%d",
                               cmdBuf->obj.id, cmdRes.u.img.pImage->obj.id);
        }
        else
        {
            cmdRes.u.buf.pBuffer = srcRes->u.buf.pBuffer;
            cmdRes.u.buf.offset  = srcRes->u.buf.offset;
            cmdRes.u.buf.range   = srcRes->u.buf.sliceSize;

            gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_input_buffer=%d",
                               cmdBuf->obj.id, cmdRes.u.buf.pBuffer->obj.id);
        }

        __vk_utils_insertCmdRes(&cmdBuf->inputs, &cmdRes, &cdp->memCb);
    }

    if (dstRes)
    {
        __VK_MEMZERO(&cmdRes, sizeof(__vkCmdResource));

        cmdRes.isImage = dstRes->isImage;
        if (cmdRes.isImage)
        {
            cmdRes.u.img.pImage = dstRes->u.img.pImage;
            cmdRes.u.img.subResRange.aspectMask     = 0;
            cmdRes.u.img.subResRange.baseMipLevel   = dstRes->u.img.subRes.mipLevel;
            cmdRes.u.img.subResRange.levelCount     = 1;
            cmdRes.u.img.subResRange.baseArrayLayer = dstRes->u.img.subRes.arrayLayer;
            cmdRes.u.img.subResRange.layerCount     = 1;

            gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_output_image=%d",
                               cmdBuf->obj.id, cmdRes.u.img.pImage->obj.id);
        }
        else
        {
            cmdRes.u.buf.pBuffer = dstRes->u.buf.pBuffer;
            cmdRes.u.buf.offset  = dstRes->u.buf.offset;
            cmdRes.u.buf.range   = dstRes->u.buf.sliceSize;

            gcoOS_PrintStrSafe(cmdRes.tag, gcdMAX_PATH, gcvNULL, "cmdBuf=%d_blit_output_buffer=%d",
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
                gcmDUMP(gcvNULL, "#[info: update memory: %s]", pResNode->res.tag);
                gcmDUMP_BUFFER(gcvNULL,
                               gcvDUMP_BUFFER_MEMORY,
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

                gcmDUMP(gcvNULL, "#[info: verify memory: %s]", pResNode->res.tag);
                gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_VERIFY, physical, logical, offset, size);
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

            gcmDUMP(gcvNULL, "#[info: verify memory: %s]", pResNode->res.tag);
            gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_VERIFY, physical, logical, offset, size);
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

VkBool32 __vk_utils_reverseMatch(
    const char *source,
    const char *dest
    )
{
    char tempBuf[__VK_MAX_NAME_LENGTH];
    char *pos;

    __vk_utils_reverseBytes(dest, tempBuf, __VK_MAX_NAME_LENGTH);
    gcoOS_StrStr(source, tempBuf, &pos);

    return (pos ? VK_TRUE : VK_FALSE);

}





