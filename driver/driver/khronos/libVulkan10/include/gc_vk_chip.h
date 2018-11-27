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


#ifndef __gc_vk_chip_h__
#define __gc_vk_chip_h__

typedef struct __vkChipFuncTableRec
{
    VkResult (*InitializeChipModule)(VkDevice device);
    VkResult (*FinializeChipModule)(VkDevice device);
    VkResult (*CreateGraphicsPipeline)(VkDevice device, const VkGraphicsPipelineCreateInfo *info, VkPipeline pipeline);
    VkResult (*CreateComputePipeline)(VkDevice device, const VkComputePipelineCreateInfo *info, VkPipeline pipeline);
    VkResult (*DestroyPipeline)(VkDevice device, VkPipeline pipeline);
    VkResult (*ClearImage)(VkCommandBuffer cmdBuf, VkImage image, VkImageSubresource *subResource, VkClearValue *clearValue, VkRect2D *rect);
    VkResult (*CopyImage)(VkCommandBuffer cmdBuf, __vkBlitRes *srcRes, __vkBlitRes *dstRes, VkBool32 rawCopy);
    VkResult (*FillBuffer)(VkCommandBuffer cmdBuf, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, uint32_t data);
    VkResult (*CopyBuffer)(VkCommandBuffer cmdBuf, __vkBlitRes *srcRes, __vkBlitRes *dstRes, uint64_t copySize);
    VkResult (*UpdateBuffer)(VkCommandBuffer cmdBuf, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, const uint32_t* pData);
    VkResult (*ComputeBlit)(VkCommandBuffer cmdBuf, __vkBlitRes *srcRes, __vkBlitRes *dstRes, VkBool32 rawCopy, VkBool3D *reverse, VkFilter filter);
    VkResult (*Draw)(VkCommandBuffer cmdBuf, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    VkResult (*DrawIndexed)(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    VkResult (*DrawIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride);
    VkResult (*DrawIndexedIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride);
    VkResult (*Dispatch)(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    VkResult (*DispatchIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    VkResult (*CreateSampler)(VkDevice device, VkSampler sampler);
    VkResult (*CreateImageView)(VkDevice device, VkImageView imageView);
    VkResult (*DestroyImageView)(VkDevice device, VkImageView imageView);
    VkResult (*CreateBufferView)(VkDevice device, VkBufferView bufferView);
    VkResult (*DestroyBufferView)(VkDevice device, VkBufferView bufferView);
    VkResult (*BeginCommandBuffer)(VkCommandBuffer commandBuffer);
    VkResult (*EndCommandBuffer)(VkCommandBuffer commandBuffer);
    VkResult (*AllocDescriptorSet)(VkDevice device, VkDescriptorSet descriptorSet);
    VkResult (*FreeDescriptorSet)(VkDevice device, VkDescriptorSet descriptorSet);
    VkResult (*UpdateDescriptorSet)(VkDevice device, VkDescriptorSet descriptorSet);
    VkResult (*ProcessQueryRequest)(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkBool32 beginOQ);
    VkResult (*BindDescritptors)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, uint32_t firstSet, uint32_t setCount);
    void (*PipelineBarrier)(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
 VkPipelineStageFlags destStageMask, VkDependencyFlags dependencyFlags,
 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
 const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
 const VkImageMemoryBarrier* pImageMemoryBarriers);
    void (*SetEvent)(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask, VkBool32 signal);
    void (*WaitEvents)(VkCommandBuffer commandBuffer, uint32_t eventCount,
 const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask,
 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
 const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
 const VkImageMemoryBarrier* pImageMemoryBarriers);
    VkResult (*AllocateCommandBuffer)(VkDevice device, VkCommandBuffer commandBuffer);
    VkResult (*FreeCommandBuffer)(VkDevice device, VkCommandBuffer commandBuffer);
    void (*BindPipeline)(VkCommandBuffer commandBuffer, VkPipeline oldPipeline, VkPipeline newPipeline);
    VkResult (*setMultiGpuSync)(VkDevice device, uint32_t **commandBuffer, uint32_t *sizeInUint);
    VkResult (*flushCache)(VkDevice device, uint32_t **commandBuffer, uint32_t *sizeInUint, int32_t cacheMask);
    VkBool32 (*tweakCopy)(VkCommandBuffer cmdBuf, VkBuffer destBuffer);
} __vkChipFuncTable;

#endif /* __gc_vk_chip_h__ */


