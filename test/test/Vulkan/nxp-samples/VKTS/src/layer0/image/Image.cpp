/**
 * VKTS - VulKan ToolS.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) since 2014 Norbert Nopper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Image.hpp"

namespace vkts
{

Image::Image(const VkDevice device, const VkImageCreateFlags flags, const VkImageType imageType, const VkFormat format, const VkExtent3D& extent, const uint32_t mipLevels, const uint32_t arrayLayers, const VkSampleCountFlagBits samples, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkSharingMode sharingMode, const uint32_t queueFamilyIndexCount, const uint32_t* queueFamilyIndices, const VkImageLayout initialLayout, const VkAccessFlags accessMask, const VkImage image) :
    IImage(), device(device), imageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, flags, imageType, format, extent, mipLevels, arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount, nullptr, initialLayout}, allQueueFamilyIndices(), imageLayout(mipLevels * arrayLayers), accessMask(mipLevels * arrayLayers), image(image)
{
    if (queueFamilyIndices)
    {
        for (uint32_t i = 0; i < queueFamilyIndexCount; i++)
        {
            allQueueFamilyIndices.push_back(queueFamilyIndices[i]);
        }

        if (queueFamilyIndexCount > 0)
        {
            imageCreateInfo.pQueueFamilyIndices = &allQueueFamilyIndices[0];
        }
    }

    for (uint32_t k = 0; k < arrayLayers; k++)
    {
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			this->imageLayout[k * mipLevels + i] = initialLayout;
			this->accessMask[k * mipLevels + i] = accessMask;
		}
    }
}

Image::~Image()
{
    destroy();
}

//
// IImage
//

const VkDevice Image::getDevice() const
{
    return device;
}

const VkImageCreateInfo& Image::getImageCreateInfo() const
{
    return imageCreateInfo;
}

VkImageCreateFlags Image::getFlags() const
{
    return imageCreateInfo.flags;
}

VkImageType Image::getImageType() const
{
    return imageCreateInfo.imageType;
}

const VkFormat& Image::getFormat() const
{
    return imageCreateInfo.format;
}

const VkExtent3D& Image::getExtent() const
{
    return imageCreateInfo.extent;
}

int32_t Image::getWidth() const
{
    return imageCreateInfo.extent.width;
}

int32_t Image::getHeight() const
{
    return imageCreateInfo.extent.height;
}

int32_t Image::getDepth() const
{
    return imageCreateInfo.extent.depth;
}

uint32_t Image::getMipLevels() const
{
    return imageCreateInfo.mipLevels;
}

uint32_t Image::getArrayLayers() const
{
    return imageCreateInfo.arrayLayers;
}

VkSampleCountFlagBits Image::getSamples() const
{
    return imageCreateInfo.samples;
}

VkImageTiling Image::getTiling() const
{
    return imageCreateInfo.tiling;
}

VkImageUsageFlags Image::getUsage() const
{
    return imageCreateInfo.usage;
}

VkSharingMode Image::getSharingMode() const
{
    return imageCreateInfo.sharingMode;
}

uint32_t Image::getQueueFamilyCount() const
{
    return imageCreateInfo.queueFamilyIndexCount;
}

const uint32_t* Image::getQueueFamilyIndices() const
{
    return imageCreateInfo.pQueueFamilyIndices;
}

VkImageLayout Image::getInitialLayout() const
{
    return imageCreateInfo.initialLayout;
}

const VkImage Image::getImage() const
{
    return image;
}

VkAccessFlags Image::getAccessMask(const uint32_t mipLevel, const uint32_t arrayLayer) const
{
	return accessMask[arrayLayer * getMipLevels() + mipLevel];
}

VkImageLayout Image::getImageLayout(const uint32_t mipLevel, const uint32_t arrayLayer) const
{
    return imageLayout[arrayLayer * getMipLevels() + mipLevel];
}

void Image::getImageMemoryRequirements(VkMemoryRequirements& memoryRequirements) const
{
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);
}

void Image::getImageSubresourceLayout(VkSubresourceLayout& subresourceLayout, const VkImageSubresource& imageSubresource) const
{
	// Reset for undefined cases.
	subresourceLayout.arrayPitch = 0;
	subresourceLayout.depthPitch = 0;

    vkGetImageSubresourceLayout(device, image, &imageSubresource, &subresourceLayout);
}

void Image::copyImage(const VkCommandBuffer cmdBuffer, IImageSP& targetImage, const VkImageCopy& imageCopy)
{
	if (!targetImage.get())
	{
		return;
	}

    VkImageLayout sourceImageLayout = getImageLayout(imageCopy.srcSubresource.mipLevel, imageCopy.srcSubresource.baseArrayLayer);
    VkAccessFlags sourceAccessMask = getAccessMask(imageCopy.srcSubresource.mipLevel, imageCopy.srcSubresource.baseArrayLayer);
    VkImageLayout targetImageLayout = targetImage->getImageLayout(imageCopy.dstSubresource.mipLevel, imageCopy.dstSubresource.baseArrayLayer);
    VkAccessFlags targetAccessMask = targetImage->getAccessMask(imageCopy.dstSubresource.mipLevel, imageCopy.dstSubresource.baseArrayLayer);

    // Prepare source image for copy.

    VkImageSubresourceRange srcImageSubresourceRange = {imageCopy.srcSubresource.aspectMask, imageCopy.srcSubresource.mipLevel, 1, imageCopy.srcSubresource.baseArrayLayer, imageCopy.srcSubresource.layerCount};

    cmdPipelineBarrier(cmdBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageSubresourceRange);

    // Prepare target image for copy.

    VkImageSubresourceRange dstImageSubresourceRange = {imageCopy.dstSubresource.aspectMask, imageCopy.dstSubresource.mipLevel, 1, imageCopy.dstSubresource.baseArrayLayer, imageCopy.dstSubresource.layerCount};

    targetImage->cmdPipelineBarrier(cmdBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageSubresourceRange);

    // Copy image by command.

    vkCmdCopyImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, targetImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

    // Revert back.

    targetImage->cmdPipelineBarrier(cmdBuffer, targetAccessMask, targetImageLayout, dstImageSubresourceRange);

    // Revert back.

    cmdPipelineBarrier(cmdBuffer, sourceAccessMask, sourceImageLayout, srcImageSubresourceRange);
}

void Image::copyImageToBuffer(const VkCommandBuffer cmdBuffer, IBufferSP& targetBuffer, const VkBufferImageCopy& bufferImageCopy)
{
	if (!targetBuffer.get())
	{
		return;
	}

    VkImageLayout sourceImageLayout = getImageLayout(bufferImageCopy.imageSubresource.mipLevel, bufferImageCopy.imageSubresource.baseArrayLayer);
    VkAccessFlags sourceAccessMask = getAccessMask(bufferImageCopy.imageSubresource.mipLevel, bufferImageCopy.imageSubresource.baseArrayLayer);
    VkAccessFlags targetAccessMask = targetBuffer->getAccessMask();

    // Prepare source image for copy.

    VkImageSubresourceRange imageSubresourceRange = {bufferImageCopy.imageSubresource.aspectMask, bufferImageCopy.imageSubresource.mipLevel, 1, bufferImageCopy.imageSubresource.baseArrayLayer, bufferImageCopy.imageSubresource.layerCount};

    cmdPipelineBarrier(cmdBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageSubresourceRange);

    // Prepare target buffer for copy.

    targetBuffer->cmdPipelineBarrier(cmdBuffer, VK_ACCESS_TRANSFER_WRITE_BIT);

	// Copy image by command.

	vkCmdCopyImageToBuffer(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, targetBuffer->getBuffer(), 1, &bufferImageCopy);

    // Revert back.

	targetBuffer->cmdPipelineBarrier(cmdBuffer, targetAccessMask);

    // Revert back.

    cmdPipelineBarrier(cmdBuffer, sourceAccessMask, sourceImageLayout, imageSubresourceRange);
}

void Image::cmdPipelineBarrier(const VkCommandBuffer cmdBuffer, const VkAccessFlags dstAccessMask, const VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange)
{
	if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED || newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		logPrint(VKTS_LOG_WARNING, "Image: New layout not allowed: %d", newLayout);

		return;
	}

	VkImageMemoryBarrier imageMemoryBarrier;

	memset(&imageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));

	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// It is allowed, that each mip level can have different layouts.
	for (uint32_t arrayLayer = subresourceRange.baseArrayLayer; arrayLayer < subresourceRange.baseArrayLayer + subresourceRange.layerCount; arrayLayer++)
	{
		for (uint32_t mipLevel = subresourceRange.baseMipLevel; mipLevel < subresourceRange.baseMipLevel + subresourceRange.levelCount; mipLevel++)
		{
			if (accessMask[arrayLayer * getMipLevels() + mipLevel] == dstAccessMask && imageLayout[arrayLayer * getMipLevels() + mipLevel] == newLayout)
			{
				continue;
			}

			imageMemoryBarrier.srcAccessMask = accessMask[arrayLayer * getMipLevels() + mipLevel];
			imageMemoryBarrier.oldLayout = imageLayout[arrayLayer * getMipLevels() + mipLevel];
			imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevel;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = arrayLayer;
			imageMemoryBarrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			accessMask[arrayLayer * getMipLevels() + mipLevel] = dstAccessMask;
			imageLayout[arrayLayer * getMipLevels() + mipLevel] = newLayout;
		}
	}
}

//
// IDestroyable
//

void Image::destroy()
{
    if (image)
    {
        vkDestroyImage(device, image, nullptr);

        image = VK_NULL_HANDLE;
    }
}

} /* namespace vkts */
