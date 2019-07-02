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


#ifndef __gc_vk_utils_h__
#define __gc_vk_utils_h__

void __vk_utils_region_set(
    __vkDescriptorResourceRegion *region,
    uint32_t resource,
    uint32_t sampler
    );

void __vk_utils_region_mad(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *base,
    uint32_t scale,
    __vkDescriptorResourceRegion * offset
    );

void __vk_utils_region_copy(
    __vkDescriptorResourceRegion *dstRegion,
    __vkDescriptorResourceRegion *srcRegion
    );

void __vk_utils_region_minus(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    );

void __vk_utils_region_add(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    );

void __vk_utils_region_mul(
    __vkDescriptorResourceRegion *result,
    __vkDescriptorResourceRegion *base,
    uint32_t scale
    );

VkBool32 __vk_utils_region_equal(
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    );

VkBool32 __vk_utils_region_gequal(
    __vkDescriptorResourceRegion *lOperand,
    __vkDescriptorResourceRegion *rOperand
    );


/* A general object who may be used in chip layer. It is a wrapper
   of user data. This object can be used in any data structure, such
   as array, link, hash, etc */
typedef struct __vkUtilsHashObjectRec
{
    /* Pointer to user defined data */
    void                            *pUserData;

    uint32_t                        entryId;

    /* A key uniquely indicating this object */
    void                            *pKey;

    /* How many references are this object being referenced currently?
       This is used to saftely delete object if object does not need to
       be maintained anymore */
    int32_t                         refCount;

    /* Recent referenced year of this object for LRU evict */
    uint32_t                        year;

    VkBool32                        perpetual;

    struct __vkUtilsHashObjectRec   *next;
} __vkUtilsHashObject;

typedef void (*__vkUtilsDeleteUserDataFunc)(VkAllocationCallbacks *pAllocator, void *pUserData);
typedef VkResult (*__vkUtilsProcessUserDataFunc)(void *pContext, void *pUserData);

/* Hash definition */
struct __vkUtilsHashRec
{
    __vkUtilsHashObject             **ppHashTable;

    /* How many objects of each entry */
    uint32_t                        *pEntryCounts;

    /* Hash table entry number must be power of 2 (32, 64, etc.), so that
       we can use a simple bitmask (entryNum-1) to get the hash entry */
    uint32_t                        tbEntryNum;

    /* Max objects count that each hash table entry can hold. For memory
       footprint consideration, we don't hope too many objects are hold in
       each entry. */
    uint32_t                        maxEntryObjs;

    /* Number of byte of each hash key */
    uint32_t                        keyBytes;

    uint32_t                        year;

    __vkUtilsDeleteUserDataFunc     pfnDeleteUserData;
};

uint32_t __vk_utils_evalCrc32(
    uint32_t initial,
    const void *pData,
    uint32_t bytes
    );

uint32_t __vk_utils_evalCrc32_masked_array(
    void **ppData,
    void **ppMask,
    uint32_t *pDataSizeInByte,
    uint32_t arraySize
    );

void __vk_utils_objAddRef(
    __vkUtilsHashObject *pObj
    );

void __vk_utils_objReleaseRef(
    __vkUtilsHashObject *pObj
    );

__vkUtilsHash* __vk_utils_hashCreate(
    VkAllocationCallbacks *pAllocator,
    uint32_t keyBytes,
    uint32_t tbEntryNum,
    uint32_t maxEntryObjs,
    __vkUtilsDeleteUserDataFunc pfnDeleteUserData
    );

void __vk_utils_hashDestory(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash
    );

void __vk_utils_hashDeleteObj(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash,
    __vkUtilsHashObject *pObj
    );

void __vk_utils_hashDeleteAllObjs(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash
    );

__vkUtilsHashObject* __vk_utils_hashAddObj(
    VkAllocationCallbacks *pAllocator,
    __vkUtilsHash *pHash,
    void *pUserData,
    const void *pKey,
    VkBool32 bPerpetual
    );

__vkUtilsHashObject* __vk_utils_hashFindObjByKey(
    __vkUtilsHash *pHash,
    const void *pKey
    );

VkResult __vk_utils_hashTraverse(
    __vkUtilsHash *pHash,
    void *pContext,
    __vkUtilsProcessUserDataFunc pfnProcessUserData
    );

VkResult __vk_utils_readBinary(
    VkAllocationCallbacks *pAllocator,
    gctCONST_STRING fileName,
    gctPOINTER *ppData,
    gctSIZE_T *pSize
    );

typedef struct __vkCmdResourceRec {
    VkBool32 isImage;
    char tag[gcdMAX_PATH];

    union {
        struct {
            __vkImage *pImage;
            VkImageSubresourceRange subResRange;
        } img;

        struct {
            __vkBuffer *pBuffer;
            VkDeviceSize offset;
            VkDeviceSize range;
        } buf;
    } u;

} __vkCmdResource;

struct __vkCmdResNodeRec {
    __vkCmdResource res;

    struct __vkCmdResNodeRec *next;
};

#if __VK_RESOURCE_INFO

void __vk_utils_insertDrawCmdRes(
    VkCommandBuffer commandBuffer,
    VkBuffer indirectBuf
    );

void __vk_utils_insertComputeCmdRes(
    VkCommandBuffer commandBuffer,
    VkBuffer indirectBuf
    );

void __vk_utils_insertCopyCmdRes(
    VkCommandBuffer commandBuffer,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes
    );

void __vk_utils_mergeCmdRes(
    __vkCommandBuffer *primary,
    __vkCommandBuffer *secondary
    );

void __vk_utils_dumpCmdInputRes(
    __vkCommandBuffer *cmdBuf
    );

void __vk_utils_dumpCmdOutputRes(
    __vkCommandBuffer *cmdBuf
    );

void __vk_utils_freeCmdRes(
    __vkCommandBuffer *cmdBuf
    );

#endif

void __vk_utils_reverseBytes(
    const char *source,
    char *dest,
    uint32_t destSize
    );

VkBool32 __vk_utils_reverseMatch(
    const char *source,
    const char *dest
    );

/*
* Compute the floor of the log base 2 of a unsigned integer (used mostly
*  for computing log2(2^n)).
*/
__VK_INLINE uint32_t __vkFloorLog2(uint32_t n)
{
    uint32_t i = 1;

    while ((n >> i) > 0)
    {
        i++;
    }

    return (i-1);
}

#endif


