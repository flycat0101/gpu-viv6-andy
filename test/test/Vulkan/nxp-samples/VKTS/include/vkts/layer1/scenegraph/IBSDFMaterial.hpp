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

#ifndef VKTS_IBSDFMATERIAL_HPP_
#define VKTS_IBSDFMATERIAL_HPP_

#include <vkts/vkts.hpp>

namespace vkts
{

class overwrite;

class IBSDFMaterial : public ICloneable<IBSDFMaterial>, public IDestroyable
{

public:

    IBSDFMaterial() :
        IDestroyable()
    {
    }

    virtual ~IBSDFMaterial()
    {
    }

    virtual const std::string& getName() const = 0;

    virtual void setName(const std::string& name) = 0;

    virtual IShaderModuleSP getFragmentShader() const = 0;

	virtual void setFragmentShader(const IShaderModuleSP& fragmentShader) = 0;

    virtual VkTsVertexBufferType getAttributes() const = 0;

	virtual void setAttributes(const VkTsVertexBufferType attributes) = 0;

    virtual void addTexture(const ITextureSP& texture) = 0;

    virtual VkBool32 removeTexture(const ITextureSP& texture) = 0;

    virtual size_t getNumberTextures() const = 0;

    virtual const SmartPointerVector<ITextureSP>& getTextures() const = 0;

    virtual const IDescriptorPoolSP& getDescriptorPool() const = 0;

    virtual void setDescriptorPool(const IDescriptorPoolSP& descriptorPool) = 0;

    virtual IDescriptorSetsSP getDescriptorSets() const = 0;

    virtual void setDescriptorSets(const IDescriptorSetsSP& descriptorSets) = 0;

    virtual void updateDescriptorSetsRecursive(const std::string& nodeName, const uint32_t allWriteDescriptorSetsCount, VkWriteDescriptorSet* allWriteDescriptorSets) = 0;

    virtual void bindDescriptorSets(const std::string& nodeName, const ICommandBuffersSP& cmdBuffer, const VkPipelineLayout layout, const uint32_t bufferIndex = 0) const = 0;

    virtual void bindDrawIndexedRecursive(const std::string& nodeName, const ICommandBuffersSP& cmdBuffer, const IGraphicsPipelineSP& graphicsPipeline, const overwrite* renderOverwrite = nullptr, const uint32_t bufferIndex = 0) const = 0;

};

typedef std::shared_ptr<IBSDFMaterial> IBSDFMaterialSP;

} /* namespace vkts */

#endif /* VKTS_IBSDFMATERIAL_HPP_ */
