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

#include "Material.hpp"

namespace vkts
{

IDescriptorSetsSP Material::createDescriptorSetsByName(const std::string& nodeName)
{
	if (!descriptorPool.get() || !descriptorSets.get())
	{
		return IDescriptorSetsSP();
	}

	if (allDescriptorSets.contains(nodeName))
	{
		return allDescriptorSets[nodeName];
	}

	//

	if (this->nodeName == "")
	{
		this->nodeName = nodeName;

		allDescriptorPools[nodeName] = descriptorPool;

		allDescriptorSets[nodeName] = descriptorSets;

		return descriptorSets;
	}

	//

	auto currentDescriptorPool = descriptorPoolCreate(descriptorPool->getDevice(), descriptorPool->getFlags(), descriptorPool->getMaxSets(), descriptorPool->getPoolSizeCount(), descriptorPool->getPoolSizes());

    if (!currentDescriptorPool.get())
    {
        return IDescriptorSetsSP();
    }

	auto currentDescriptorSets = descriptorSetsCreate(currentDescriptorPool->getDevice(), currentDescriptorPool->getDescriptorPool(), descriptorSets->getDescriptorSetCount(), descriptorSets->getSetLayouts());

    if (!currentDescriptorSets.get())
    {
        return IDescriptorSetsSP();
    }

    allDescriptorPools[nodeName] = currentDescriptorPool;

    allDescriptorSets[nodeName] = currentDescriptorSets;

    //

    return allDescriptorSets[nodeName];
}

IDescriptorSetsSP Material::getDescriptorSetsByName(const std::string& nodeName) const
{
	if (allDescriptorSets.contains(nodeName))
	{
		return allDescriptorSets[nodeName];
	}

	return IDescriptorSetsSP();
}

void Material::updateDescriptorImageInfo(const uint32_t colorIndex, const uint32_t dstBindingOffset, const VkSampler sampler, const VkImageView imageView, const VkImageLayout imageLayout)
{
    if (colorIndex >= VKTS_BINDING_UNIFORM_MATERIAL_TOTAL_BINDING_COUNT)
    {
        return;
    }

    memset(&descriptorImageInfos[colorIndex], 0, sizeof(VkDescriptorImageInfo));

    descriptorImageInfos[colorIndex].sampler = sampler;
    descriptorImageInfos[colorIndex].imageView = imageView;
    descriptorImageInfos[colorIndex].imageLayout = imageLayout;

    memset(&writeDescriptorSets[colorIndex], 0, sizeof(VkWriteDescriptorSet));

    writeDescriptorSets[colorIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSets[colorIndex].dstSet = VK_NULL_HANDLE; // Defined later.
    writeDescriptorSets[colorIndex].dstBinding = dstBindingOffset + colorIndex;
    writeDescriptorSets[colorIndex].dstArrayElement = 0;
    writeDescriptorSets[colorIndex].descriptorCount = 1;
    writeDescriptorSets[colorIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSets[colorIndex].pImageInfo = &descriptorImageInfos[colorIndex];
    writeDescriptorSets[colorIndex].pBufferInfo = nullptr;
    writeDescriptorSets[colorIndex].pTexelBufferView = nullptr;
}

Material::Material() :
    descriptorPool(), descriptorSets(), nodeName(), allDescriptorPools(), allDescriptorSets()
{
	memset(descriptorImageInfos, 0, sizeof(descriptorImageInfos));
	memset(writeDescriptorSets, 0, sizeof(writeDescriptorSets));
}

Material::Material(const Material& other) :
    descriptorPool(), descriptorSets(), nodeName(), allDescriptorPools(), allDescriptorSets()
{
	memset(descriptorImageInfos, 0, sizeof(descriptorImageInfos));
	memset(writeDescriptorSets, 0, sizeof(writeDescriptorSets));
}

Material::~Material()
{
    destroy();
}

void Material::destroy()
{
    descriptorSets = IDescriptorSetsSP();

    descriptorPool = IDescriptorPoolSP();

    memset(&descriptorImageInfos, 0, sizeof(descriptorImageInfos));
    memset(&writeDescriptorSets, 0, sizeof(writeDescriptorSets));

    for (size_t i = 0; i < allDescriptorSets.size(); i++)
    {
        allDescriptorSets.valueAt(i)->destroy();
    }
    allDescriptorSets.clear();

    for (size_t i = 0; i < allDescriptorPools.size(); i++)
    {
    	allDescriptorPools.valueAt(i)->destroy();
    }
    allDescriptorPools.clear();
}

} /* namespace vkts */
