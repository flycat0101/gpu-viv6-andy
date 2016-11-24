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

#ifndef DEFAULTGRAPHICSPIPELINE_HPP_
#define DEFAULTGRAPHICSPIPELINE_HPP_

#include <vkts/vkts.hpp>

#define VKTS_MAX_VERTEX_BINDING_DESCRIPTION_COUNT	16
#define VKTS_MAX_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT	16
#define VKTS_MAX_VIEWPORTS				  			16
#define VKTS_MAX_SCISSORS				  			16
#define VKTS_MAX_BLEND_ATTACHMENTS		  			16

namespace vkts {

class defaultGraphicsPipeline
{

private:

	VkVertexInputBindingDescription 		vertexInputBindingDescription[VKTS_MAX_VERTEX_BINDING_DESCRIPTION_COUNT];
	VkVertexInputAttributeDescription 		vertexInputAttributeDescription[VKTS_MAX_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT];

    VkViewport                     			viewports[VKTS_MAX_VIEWPORTS];
    VkRect2D                       			scissors[VKTS_MAX_SCISSORS];

    VkPipelineColorBlendAttachmentState		pipelineColorBlendAttachmentState[VKTS_MAX_BLEND_ATTACHMENTS];

    VkDynamicState							dynamicState[VK_DYNAMIC_STATE_RANGE_SIZE];

    VkPipelineShaderStageCreateInfo			pipelineShaderStageCreateInfo[5];
    VkPipelineVertexInputStateCreateInfo	pipelineVertexInputStateCreateInfo;
    VkPipelineInputAssemblyStateCreateInfo	pipelineInputAssemblyStateCreateInfo;
    VkPipelineTessellationStateCreateInfo	pipelineTessellationStateCreateInfo;
    VkPipelineViewportStateCreateInfo		pipelineViewportStateCreateInfo;
    VkPipelineRasterizationStateCreateInfo	pipelineRasterizationStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo	pipelineMultisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo	pipelineDepthStencilStateCreateInfo;
    VkPipelineColorBlendStateCreateInfo		pipelineColorBlendStateCreateInfo;
    VkPipelineDynamicStateCreateInfo		pipelineDynamicStateCreateInfo;

	VkGraphicsPipelineCreateInfo 			graphicsPipelineCreateInfo;

public:

	defaultGraphicsPipeline();

	~defaultGraphicsPipeline();

	void reset();

	VkVertexInputBindingDescription& 		getVertexInputBindingDescription(const uint32_t index);
	VkVertexInputAttributeDescription& 		getVertexInputAttributeDescription(const uint32_t index);

    VkViewport&                     		getViewports(const uint32_t index);
    VkRect2D&                       		getScissors(const uint32_t index);

    VkPipelineColorBlendAttachmentState&	getPipelineColorBlendAttachmentState(const uint32_t index);

    VkDynamicState&							getDynamicState(const uint32_t index);

	//

    VkPipelineShaderStageCreateInfo&		getPipelineShaderStageCreateInfo(const uint32_t index);
    VkPipelineVertexInputStateCreateInfo&	getPipelineVertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo&	getPipelineInputAssemblyStateCreateInfo();
    VkPipelineTessellationStateCreateInfo&	getPipelineTessellationStateCreateInfo();
    VkPipelineViewportStateCreateInfo&		getPipelineViewportStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo&	getPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo&	getPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo&	getPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo&	getPipelineColorBlendStateCreateInfo();
    VkPipelineDynamicStateCreateInfo&		getPipelineDynamicStateCreateInfo();

    //

	VkGraphicsPipelineCreateInfo& 			getGraphicsPipelineCreateInfo();

};

} /* namespace vkts */

#endif /* DEFAULTGRAPHICSPIPELINE_HPP_ */
