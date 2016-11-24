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

#ifndef VKTS_MATERIAL_HPP_
#define VKTS_MATERIAL_HPP_

#include <vkts/vkts.hpp>

namespace vkts
{

class Material
{

protected:

    IDescriptorPoolSP descriptorPool;
    IDescriptorSetsSP descriptorSets;
    VkDescriptorImageInfo descriptorImageInfos[VKTS_BINDING_UNIFORM_MATERIAL_TOTAL_BINDING_COUNT];
    VkWriteDescriptorSet writeDescriptorSets[VKTS_BINDING_UNIFORM_MATERIAL_TOTAL_BINDING_COUNT];
    std::string nodeName;

    SmartPointerMap<std::string, IDescriptorPoolSP> allDescriptorPools;
    SmartPointerMap<std::string, IDescriptorSetsSP> allDescriptorSets;

    IDescriptorSetsSP createDescriptorSetsByName(const std::string& nodeName);
    IDescriptorSetsSP getDescriptorSetsByName(const std::string& nodeName) const;

    void updateDescriptorImageInfo(const uint32_t colorIndex, const uint32_t dstBindingOffset, const VkSampler sampler, const VkImageView imageView, const VkImageLayout imageLayout);

public:

    Material();
    Material(const Material& other);
    Material(Material&& other) = delete;
    virtual ~Material();

    Material& operator =(const Material& other) = delete;

    Material& operator =(Material && other) = delete;

    virtual void destroy();

};

} /* namespace vkts */

#endif /* VKTS_MATERIAL_HPP_ */
