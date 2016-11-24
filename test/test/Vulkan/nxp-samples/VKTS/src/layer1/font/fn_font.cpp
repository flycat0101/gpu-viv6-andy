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

#include <vkts/vkts.hpp>

#include "../../fn_internal.hpp"

#include "../../layer0/binary_buffer/BinaryBuffer.hpp"

#include "../../layer0/text_buffer/TextBuffer.hpp"

#include "../../layer0/image_view/ImageView.hpp"

#include "../../layer0/sampler/Sampler.hpp"

#include "../buffer_object/BufferObject.hpp"

#include "../texture/Texture.hpp"

#include "Char.hpp"

#include "Font.hpp"

#define VKTS_BINDING_VERTEX_BUFFER 0

#define VKTS_FONT_VERTEX_SHADER_NAME 	"shader/SPIR/V/font.vert.spv"
#define VKTS_FONT_FRAGMENT_SHADER_NAME 	"shader/SPIR/V/font.frag.spv"

namespace vkts
{

static VkBool32 fontGetDirectory(char* directory, const char* filename)
{
    if (!directory || !filename)
    {
        return VK_FALSE;
    }

    auto position = strrchr(filename, '/');

    if (position)
    {
        strncpy(directory, filename, position - filename + 1);
        directory[position - filename + 1] = '\0';

        return VK_TRUE;
    }

    position = strrchr(filename, '\\');

    if (position)
    {
        strncpy(directory, filename, position - filename + 1);
        directory[position - filename + 1] = '\0';

        return VK_TRUE;
    }

    return VK_FALSE;
}

static VkBool32 fontIsToken(const char* buffer, const char* token)
{
    if (!buffer || !token)
    {
        return VK_FALSE;
    }

    if (!(buffer[strlen(token)] == ' '))
    {
        return VK_FALSE;
    }

    if (strncmp(buffer, token, strlen(token)) == 0)
    {
        return VK_TRUE;
    }

    return VK_FALSE;
}

static VkBool32 fontExtractValue(const char* buffer, const char* parameter, char* value)
{
    if (!buffer || !parameter || !value)
    {
        return VK_FALSE;
    }

    const char* start = strstr(buffer, parameter);

    if (!start)
    {
    	return VK_FALSE;
    }

    // Skip parameter and '='
    start += 1 + strlen(parameter);

    const char* end = strstr(start, " ");

    if (!end)
    {
    	end = strstr(start, "\t");

    	if (!end)
    	{
    		end = strstr(start, "\r");

			if (!end)
			{
				end = strstr(start, "\n");

				if (!end)
				{
					return VK_FALSE;
				}
			}
    	}
    }

    strncpy(value, start, end - start);
    value[end - start] = '\0';

    return VK_TRUE;
}

static VkBool32 fontExtractIntValue(const char* buffer, const char* parameter, int& value)
{
    if (!buffer || !parameter)
    {
        return VK_FALSE;
    }

    char temp[VKTS_MAX_TOKEN_CHARS + 1];

    if (!fontExtractValue(buffer, parameter, temp))
    {
    	return VK_FALSE;
    }

    value = atoi(temp);

    return VK_TRUE;
}

static VkBool32 fontExtractStringValue(const char* buffer, const char* parameter, char* value)
{
    if (!buffer || !parameter || !value)
    {
        return VK_FALSE;
    }

    char temp[VKTS_MAX_TOKEN_CHARS + 1];

    if (!fontExtractValue(buffer, parameter, temp))
    {
    	return VK_FALSE;
    }

    char* start = strstr(temp, "\"");
    if (!start)
    {
    	return VK_FALSE;
    }
    start++;

    char* end = strstr(start, "\"");
    if (!end)
    {
    	return VK_FALSE;
    }

    strncpy(value, start, end - start);
    value[end - start] = '\0';

    return VK_TRUE;
}

IFontSP VKTS_APIENTRY fontCreate(const char* filename, const IInitialResourcesSP& initialResources, const ICommandBuffersSP& commandBuffer, const IRenderPassSP& renderPass, SmartPointerVector<vkts::IImageSP>& allStageImages, vkts::SmartPointerVector<vkts::IBufferSP>& allStageBuffers, vkts::SmartPointerVector<vkts::IDeviceMemorySP>& allStageDeviceMemories)
{
    if (!filename || !initialResources.get() || !commandBuffer.get())
    {
        return IFontSP();
    }

    auto textBuffer = fileLoadText(filename);

    if (!textBuffer.get())
    {
        return IFontSP();
    }

    char directory[VKTS_MAX_BUFFER_CHARS];

    fontGetDirectory(directory, filename);

    auto font = new Font();

    auto interfaceFont = IFontSP(font);

    if (!interfaceFont.get())
    {
        return IFontSP();
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    int idata;

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (fontIsToken(buffer, "info"))
        {
        	if (!fontExtractStringValue(buffer, "face", sdata))
        	{
            	return IFontSP();
        	}

        	font->setFace(sdata);

        	if (!fontExtractIntValue(buffer, "size", idata))
        	{
            	return IFontSP();
        	}

        	font->setSize((float)idata);
        }
        else if (fontIsToken(buffer, "common"))
        {
        	if (!fontExtractIntValue(buffer, "lineHeight", idata))
        	{
            	return IFontSP();
        	}

        	font->setLineHeight((float)idata);

        	if (!fontExtractIntValue(buffer, "base", idata))
        	{
            	return IFontSP();
        	}

        	font->setBase((float)idata);

        	if (!fontExtractIntValue(buffer, "scaleW", idata))
        	{
            	return IFontSP();
        	}

        	font->setScaleWidth((float)idata);

        	if (!fontExtractIntValue(buffer, "scaleH", idata))
        	{
            	return IFontSP();
        	}

        	font->setScaleHeight((float)idata);
        }
        else if (fontIsToken(buffer, "page"))
        {
        	if (!fontExtractStringValue(buffer, "file", sdata))
        	{
            	return IFontSP();
        	}

        	std::string finalImageDataFilename = std::string(directory) + std::string(sdata);

        	auto imageData = imageDataLoad(finalImageDataFilename.c_str());

        	if (!imageData.get())
        	{
        		return IFontSP();
        	}

        	//

            VkImageTiling imageTiling;
            VkMemoryPropertyFlags memoryPropertyFlags;

            if (!initialResources->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, imageData->getFormat(), imageData->getImageType(), 0, imageData->getExtent3D(), imageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, imageData->getSize()))
            {
            	return IFontSP();
            }

            //

            VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            VkAccessFlags srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

            if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
            {
            	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            	srcAccessMask = 0;
            }

            //

            VkImageCreateInfo imageCreateInfo;

            memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

            imageCreateInfo.flags = 0;
            imageCreateInfo.imageType = imageData->getImageType();
            imageCreateInfo.format = imageData->getFormat();
            imageCreateInfo.extent = imageData->getExtent3D();
            imageCreateInfo.mipLevels = imageData->getMipLevels();
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = imageTiling;
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.queueFamilyIndexCount = 0;
            imageCreateInfo.pQueueFamilyIndices = nullptr;
            imageCreateInfo.initialLayout = initialLayout;

            VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, imageData->getMipLevels(), 0, 1};

            IDeviceMemorySP stageDeviceMemory;
            IBufferSP stageBuffer;
            IImageSP stageImage;

            auto memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, initialResources, commandBuffer, font->getFace(), imageData, imageCreateInfo, srcAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, memoryPropertyFlags);

            if (stageDeviceMemory.get())
            {
            	allStageDeviceMemories.append(stageDeviceMemory);
            }
            if (stageBuffer.get())
            {
            	allStageBuffers.append(stageBuffer);
            }
            if (stageImage.get())
            {
            	allStageImages.append(stageImage);
            }

            if (!memoryImage.get())
            {
            	return VK_FALSE;
            }

            //

        	VkSamplerCreateInfo samplerCreateInfo;

        	memset(&samplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

        	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        	samplerCreateInfo.flags = 0;
        	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        	samplerCreateInfo.mipLodBias = 0.0f;
        	samplerCreateInfo.maxAnisotropy = 1.0f;
        	samplerCreateInfo.compareEnable = VK_FALSE;
        	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        	samplerCreateInfo.minLod = 0.0f;
        	samplerCreateInfo.maxLod = 0.0f;
        	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        	VkImageViewCreateInfo imageViewCreateInfo;

        	memset(&imageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

        	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        	imageViewCreateInfo.flags = 0;
        	imageViewCreateInfo.image = VK_NULL_HANDLE;				// Defined later.
        	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        	imageViewCreateInfo.format = VK_FORMAT_UNDEFINED;		// Defined later.
        	imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        	imageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            //

            auto texture = textureCreate(initialResources, font->getFace(), VK_FALSE, VK_FALSE, memoryImage, samplerCreateInfo, imageViewCreateInfo);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            font->setTexture(texture);
        }
        else if (fontIsToken(buffer, "chars"))
        {
        	// Do nothing.
        }
        else if (fontIsToken(buffer, "char"))
        {
        	Char character;

        	if (!fontExtractIntValue(buffer, "id", idata))
        	{
            	return IFontSP();
        	}

        	character.setId(idata);

        	if (!fontExtractIntValue(buffer, "x", idata))
        	{
            	return IFontSP();
        	}

        	character.setX((float)idata);

        	if (!fontExtractIntValue(buffer, "y", idata))
        	{
            	return IFontSP();
        	}

        	character.setY((float)idata);

        	if (!fontExtractIntValue(buffer, "width", idata))
        	{
            	return IFontSP();
        	}

        	character.setWidth((float)idata);

        	if (!fontExtractIntValue(buffer, "height", idata))
        	{
            	return IFontSP();
        	}

        	character.setHeight((float)idata);

        	if (!fontExtractIntValue(buffer, "xoffset", idata))
        	{
            	return IFontSP();
        	}

        	character.setXoffset((float)idata);

        	if (!fontExtractIntValue(buffer, "yoffset", idata))
        	{
            	return IFontSP();
        	}

        	character.setYoffset((float)idata);

        	if (!fontExtractIntValue(buffer, "xadvance", idata))
        	{
            	return IFontSP();
        	}

        	character.setXadvance((float)idata);

        	//

        	font->setChar(character);
        }
        else if (fontIsToken(buffer, "kernings"))
        {
        	// Do nothing.
        }
        else if (fontIsToken(buffer, "kerning"))
        {
        	if (!fontExtractIntValue(buffer, "first", idata))
        	{
            	return IFontSP();
        	}

        	int32_t characterId = idata;

        	if (!fontExtractIntValue(buffer, "second", idata))
        	{
            	return IFontSP();
        	}

        	int32_t nextCharacterId = idata;

        	if (!fontExtractIntValue(buffer, "amount", idata))
        	{
            	return IFontSP();
        	}

        	float amount = (float)idata;

        	//

        	font->setKerning(characterId, nextCharacterId, amount);
        }
    }

    //
    // Create a quad out of a triangle strip.
    //

    const float vertexData[4*4] = {
		0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
    };

    auto binaryBuffer = binaryBufferCreate((const uint8_t*)vertexData, sizeof(vertexData));

    if (!binaryBuffer.get())
    {
    	return IFontSP();
    }

    //

    VkBufferCreateInfo bufferCreateInfo;

    memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferCreateInfo.size = sizeof(vertexData);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices = nullptr;

    IDeviceMemorySP stageDeviceMemory;
    IBufferSP stageBuffer;

    auto vertexBuffer = bufferObjectCreate(stageBuffer, stageDeviceMemory, initialResources, commandBuffer, binaryBuffer, bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    if (stageDeviceMemory.get())
    {
    	allStageDeviceMemories.append(stageDeviceMemory);
    }
    if (stageBuffer.get())
    {
    	allStageBuffers.append(stageBuffer);
    }

    if (!vertexBuffer.get())
    {
    	return IFontSP();
    }

    font->setVertexBuffer(vertexBuffer);

    //
    // Shader modules.
    //

	auto vertexShaderBinary = fileLoadBinary(VKTS_FONT_VERTEX_SHADER_NAME);

	if (!vertexShaderBinary.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not load vertex shader: '%s'", VKTS_FONT_VERTEX_SHADER_NAME);

		return IFontSP();
	}

	auto fragmentShaderBinary = fileLoadBinary(VKTS_FONT_FRAGMENT_SHADER_NAME);

	if (!fragmentShaderBinary.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not load fragment shader: '%s'", VKTS_FONT_FRAGMENT_SHADER_NAME);

		return IFontSP();
	}

	//

	auto vertexShaderModule = shaderModuleCreate(VKTS_FONT_VERTEX_SHADER_NAME, initialResources->getDevice()->getDevice(), 0, vertexShaderBinary->getSize(), (uint32_t*)vertexShaderBinary->getData());

	if (!vertexShaderModule.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not create vertex shader module.");

		return IFontSP();
	}

	// No need to store.

	auto fragmentShaderModule = shaderModuleCreate(VKTS_FONT_FRAGMENT_SHADER_NAME, initialResources->getDevice()->getDevice(), 0, fragmentShaderBinary->getSize(), (uint32_t*)fragmentShaderBinary->getData());

	if (!fragmentShaderModule.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not create fragment shader module.");

		return VK_FALSE;
	}

	// No need to store.

	//
	// Descriptor set layout.
	//

	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[1];

	memset(&descriptorSetLayoutBinding, 0, sizeof(descriptorSetLayoutBinding));

	descriptorSetLayoutBinding[0].binding = 0;
	descriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBinding[0].descriptorCount = 1;
	descriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding[0].pImmutableSamplers = nullptr;

    //

    auto descriptorSetLayout = descriptorSetLayoutCreate(initialResources->getDevice()->getDevice(), 0, 1, descriptorSetLayoutBinding);

	if (!descriptorSetLayout.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not create descriptor set layout.");

		return IFontSP();
	}

	font->setDescriptorSetLayout(descriptorSetLayout);

	//

    VkDescriptorPoolSize descriptorPoolSize[1];

    memset(&descriptorPoolSize, 0, sizeof(descriptorPoolSize));

    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize[0].descriptorCount = 1;

    auto descriptorPool = descriptorPoolCreate(initialResources->getDevice()->getDevice(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1, 1, descriptorPoolSize);

    if (!descriptorPool.get())
    {
    	return IFontSP();
    }

    font->setDescriptorPool(descriptorPool);

	//

    auto allDescriptorSetLayouts = descriptorSetLayout->getDescriptorSetLayout();

	auto descriptorSets = descriptorSetsCreate(initialResources->getDevice()->getDevice(), descriptorPool->getDescriptorPool(), 1, &allDescriptorSetLayouts);

    if (!descriptorSets.get())
    {
    	return IFontSP();
    }

    font->setDescriptorSets(descriptorSets);

	//

	VkDescriptorImageInfo descriptorImageInfo;

	memset(&descriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));

	descriptorImageInfo.sampler = font->getTexture()->getSampler()->getSampler();
	descriptorImageInfo.imageView = font->getTexture()->getImageView()->getImageView();
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet writeDescriptorSet;

	memset(&writeDescriptorSet, 0, sizeof(writeDescriptorSet));

	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

	writeDescriptorSet.dstSet = descriptorSets->getDescriptorSets()[0];
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(initialResources->getDevice()->getDevice(), 1, &writeDescriptorSet, 0, nullptr);

	//
	// Pipeline layout.
	//

    // Using push constants for displacement.

    VkPushConstantRange pushConstantRange[1];

    pushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange[0].offset = 0;
    pushConstantRange[0].size = 16 * sizeof(float) + 12 * sizeof(float) + 4 * sizeof(float);

	VkDescriptorSetLayout setLayouts[1];

	setLayouts[0] = descriptorSetLayout->getDescriptorSetLayout();

	auto pipelineLayout = vkts::pipelineCreateLayout(initialResources->getDevice()->getDevice(), 0, 1, setLayouts, 1, pushConstantRange);

	if (!pipelineLayout.get())
	{
		logPrint(VKTS_LOG_ERROR, "Font: Could not create pipeline layout.");

		return IFontSP();
	}

	font->setPipelineLayout(pipelineLayout);

    //
	// Graphics pipeline.
	//

    defaultGraphicsPipeline gp;

    gp.getPipelineShaderStageCreateInfo(0).stage = VK_SHADER_STAGE_VERTEX_BIT;
    gp.getPipelineShaderStageCreateInfo(0).module = vertexShaderModule->getShaderModule();

    gp.getPipelineShaderStageCreateInfo(1).stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    gp.getPipelineShaderStageCreateInfo(1).module = fragmentShaderModule->getShaderModule();


	VkTsVertexBufferType vertexBufferType = VKTS_VERTEX_BUFFER_TYPE_VERTEX;

    gp.getVertexInputBindingDescription(0).binding = VKTS_BINDING_VERTEX_BUFFER;
    gp.getVertexInputBindingDescription(0).stride = commonGetStrideInBytes(vertexBufferType);
    gp.getVertexInputBindingDescription(0).inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gp.getVertexInputAttributeDescription(0).location = 0;
    gp.getVertexInputAttributeDescription(0).binding = VKTS_BINDING_VERTEX_BUFFER;
    gp.getVertexInputAttributeDescription(0).format = VK_FORMAT_R32G32B32A32_SFLOAT;
    gp.getVertexInputAttributeDescription(0).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_VERTEX, vertexBufferType);


    gp.getPipelineInputAssemblyStateCreateInfo().topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;


    gp.getViewports(0).x = 0.0f;
    gp.getViewports(0).y = 0.0f;
    gp.getViewports(0).width = 1.0f;
    gp.getViewports(0).height = 1.0f;
    gp.getViewports(0).minDepth = 0.0f;
    gp.getViewports(0).maxDepth = 1.0f;


    gp.getScissors(0).offset.x = 0;
    gp.getScissors(0).offset.y = 0;
    gp.getScissors(0).extent = {1, 1};


    gp.getPipelineMultisampleStateCreateInfo();
    gp.getPipelineDepthStencilStateCreateInfo();


    gp.getPipelineColorBlendAttachmentState(0).blendEnable = VK_TRUE;
    gp.getPipelineColorBlendAttachmentState(0).srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    gp.getPipelineColorBlendAttachmentState(0).dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    gp.getPipelineColorBlendAttachmentState(0).colorBlendOp = VK_BLEND_OP_ADD;
    gp.getPipelineColorBlendAttachmentState(0).srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    gp.getPipelineColorBlendAttachmentState(0).dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    gp.getPipelineColorBlendAttachmentState(0).alphaBlendOp = VK_BLEND_OP_ADD;
    gp.getPipelineColorBlendAttachmentState(0).colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


    gp.getDynamicState(0) = VK_DYNAMIC_STATE_VIEWPORT;
    gp.getDynamicState(1) = VK_DYNAMIC_STATE_SCISSOR;


    gp.getGraphicsPipelineCreateInfo().layout = pipelineLayout->getPipelineLayout();
    gp.getGraphicsPipelineCreateInfo().renderPass = renderPass->getRenderPass();


    auto graphicsPipeline = pipelineCreateGraphics(initialResources->getDevice()->getDevice(), VK_NULL_HANDLE, gp.getGraphicsPipelineCreateInfo(), VKTS_VERTEX_BUFFER_TYPE_VERTEX);

    if (!graphicsPipeline.get())
    {
    	return IFontSP();
    }

    font->setGraphicsPipeline(graphicsPipeline);

    return interfaceFont;
}

}
