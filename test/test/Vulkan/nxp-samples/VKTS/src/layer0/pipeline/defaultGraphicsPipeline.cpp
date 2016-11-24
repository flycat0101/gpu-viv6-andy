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

namespace vkts {

defaultGraphicsPipeline::defaultGraphicsPipeline()
{
	reset();
}

defaultGraphicsPipeline::~defaultGraphicsPipeline()
{
}

void defaultGraphicsPipeline::reset()
{
    memset(pipelineShaderStageCreateInfo, 0, sizeof(pipelineShaderStageCreateInfo));

    for (uint32_t i = 0; i < 5; i++)
    {
    	pipelineShaderStageCreateInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    	pipelineShaderStageCreateInfo[i].pName = "main";
    }

    //

	memset(vertexInputBindingDescription, 0, sizeof(vertexInputBindingDescription));
	memset(vertexInputAttributeDescription, 0, sizeof(vertexInputAttributeDescription));

    memset(&pipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription;
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

    //

    memset(&pipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

    pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    //

    memset(&pipelineTessellationStateCreateInfo, 0, sizeof(VkPipelineTessellationStateCreateInfo));

    pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    //

    memset(viewports, 0, sizeof(viewports));
    memset(scissors, 0, sizeof(scissors));

    memset(&pipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

    pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    pipelineViewportStateCreateInfo.pViewports = viewports;
    pipelineViewportStateCreateInfo.pScissors = scissors;

    //

    memset(&pipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    //

    memset(&pipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //

    memset(&pipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    //

    memset(pipelineColorBlendAttachmentState, 0, sizeof(pipelineColorBlendAttachmentState));


    memset(&pipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    //

    memset(dynamicState, 0, sizeof(dynamicState));

    memset(&pipelineDynamicStateCreateInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));

    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicState;

    //
    //
    //

	memset(&graphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfo;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = nullptr;
	graphicsPipelineCreateInfo.pViewportState = nullptr;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = nullptr;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.pColorBlendState = nullptr;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;

}

//

VkVertexInputBindingDescription& defaultGraphicsPipeline::getVertexInputBindingDescription(const uint32_t index)
{
	if (index >= VKTS_MAX_VERTEX_BINDING_DESCRIPTION_COUNT)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VKTS_MAX_VERTEX_BINDING_DESCRIPTION_COUNT));
	}

	auto& current = getPipelineVertexInputStateCreateInfo();

	if (!current.pVertexBindingDescriptions)
	{
		current.pVertexBindingDescriptions = vertexInputBindingDescription;
	}

	current.vertexBindingDescriptionCount = glm::max(current.vertexBindingDescriptionCount, index + 1);

	return vertexInputBindingDescription[index];
}

VkVertexInputAttributeDescription& defaultGraphicsPipeline::getVertexInputAttributeDescription(const uint32_t index)
{
	if (index >= VKTS_MAX_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VKTS_MAX_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT));
	}

	auto& current = getPipelineVertexInputStateCreateInfo();

	if (!current.pVertexAttributeDescriptions)
	{
		current.pVertexAttributeDescriptions = vertexInputAttributeDescription;
	}

	current.vertexAttributeDescriptionCount = glm::max(current.vertexAttributeDescriptionCount, index + 1);

	return vertexInputAttributeDescription[index];
}

VkViewport& defaultGraphicsPipeline::getViewports(const uint32_t index)
{
	if (index >= VKTS_MAX_VIEWPORTS)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VKTS_MAX_VIEWPORTS));
	}

	auto& current = getPipelineViewportStateCreateInfo();

	if (!current.pViewports)
	{
		current.pViewports = viewports;
	}

	current.viewportCount = glm::max(current.viewportCount, index + 1);

	return viewports[index];
}

VkRect2D& defaultGraphicsPipeline::getScissors(const uint32_t index)
{
	if (index >= VKTS_MAX_SCISSORS)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VKTS_MAX_SCISSORS));
	}

	auto& current = getPipelineViewportStateCreateInfo();

	if (!current.pScissors)
	{
		current.pScissors = scissors;
	}

	current.scissorCount = glm::max(current.scissorCount, index + 1);

	return scissors[index];
}

VkPipelineColorBlendAttachmentState& defaultGraphicsPipeline::getPipelineColorBlendAttachmentState(const uint32_t index)
{
	if (index >= VKTS_MAX_BLEND_ATTACHMENTS)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VKTS_MAX_BLEND_ATTACHMENTS));
	}

	auto& current = getPipelineColorBlendStateCreateInfo();

	if (!current.pAttachments)
	{
		current.pAttachments = pipelineColorBlendAttachmentState;
	}

	current.attachmentCount = glm::max(current.attachmentCount, index + 1);

	return pipelineColorBlendAttachmentState[index];
}

VkDynamicState&	defaultGraphicsPipeline::getDynamicState(const uint32_t index)
{
	if (index >= VK_DYNAMIC_STATE_RANGE_SIZE)
	{
		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(VK_DYNAMIC_STATE_RANGE_SIZE));
	}

	auto& current = getPipelineDynamicStateCreateInfo();

	if (!current.pDynamicStates)
	{
		current.pDynamicStates = dynamicState;
	}

	current.dynamicStateCount = glm::max(current.dynamicStateCount, index + 1);

	return dynamicState[index];
}

//

VkPipelineShaderStageCreateInfo& defaultGraphicsPipeline::getPipelineShaderStageCreateInfo(const uint32_t index)
{
	if (index >= 5)
	{
		throw std::out_of_range(std::to_string(index) + " >= 5");
	}

	if (!graphicsPipelineCreateInfo.pStages)
	{
		graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfo;
	}

	graphicsPipelineCreateInfo.stageCount = glm::max(graphicsPipelineCreateInfo.stageCount, index + 1);

	return pipelineShaderStageCreateInfo[index];
}

VkPipelineVertexInputStateCreateInfo& defaultGraphicsPipeline::getPipelineVertexInputStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pVertexInputState)
	{
		graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	}

	return pipelineVertexInputStateCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo&	defaultGraphicsPipeline::getPipelineInputAssemblyStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pInputAssemblyState)
	{
		graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	}

	return pipelineInputAssemblyStateCreateInfo;
}

VkPipelineTessellationStateCreateInfo& defaultGraphicsPipeline::getPipelineTessellationStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pTessellationState)
	{
		graphicsPipelineCreateInfo.pTessellationState = &pipelineTessellationStateCreateInfo;
	}

	return pipelineTessellationStateCreateInfo;
}

VkPipelineViewportStateCreateInfo& defaultGraphicsPipeline::getPipelineViewportStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pViewportState)
	{
		graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	}

	return pipelineViewportStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo&	defaultGraphicsPipeline::getPipelineRasterizationStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pRasterizationState)
	{
		graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	}

	return pipelineRasterizationStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo& defaultGraphicsPipeline::getPipelineMultisampleStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pMultisampleState)
	{
		graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	}

	return pipelineMultisampleStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo& defaultGraphicsPipeline::getPipelineDepthStencilStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pDepthStencilState)
	{
		graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	}

	return pipelineDepthStencilStateCreateInfo;
}

VkPipelineColorBlendStateCreateInfo& defaultGraphicsPipeline::getPipelineColorBlendStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pColorBlendState)
	{
		graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	}

	return pipelineColorBlendStateCreateInfo;
}

VkPipelineDynamicStateCreateInfo& defaultGraphicsPipeline::getPipelineDynamicStateCreateInfo()
{
	if (!graphicsPipelineCreateInfo.pDynamicState)
	{
		graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	}

	return pipelineDynamicStateCreateInfo;
}

//

VkGraphicsPipelineCreateInfo& defaultGraphicsPipeline::getGraphicsPipelineCreateInfo()
{
	return graphicsPipelineCreateInfo;
}

} /* namespace vkts */
