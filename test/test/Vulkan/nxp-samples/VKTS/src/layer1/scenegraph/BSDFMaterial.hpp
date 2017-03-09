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

#ifndef VKTS_BSDFMATERIAL_HPP_
#define VKTS_BSDFMATERIAL_HPP_

#include <vkts/vkts.hpp>

#include "Material.hpp"

namespace vkts
{

class BSDFMaterial : public IBSDFMaterial, Material
{

protected:

    std::string name;

    IShaderModuleSP fragmentShader;

    VkTsVertexBufferType attributes;

    SmartPointerVector<ITextureSP> allTextures;

public:

    BSDFMaterial();
    BSDFMaterial(const BSDFMaterial& other);
    BSDFMaterial(BSDFMaterial&& other) = delete;
    virtual ~BSDFMaterial();

    BSDFMaterial& operator =(const BSDFMaterial& other) = delete;
    BSDFMaterial& operator =(BSDFMaterial && other) = delete;

    //
    // IBSDFMaterial
    //

    virtual const std::string& getName() const override;

    virtual void setName(const std::string& name) override;

    virtual IShaderModuleSP getFragmentShader() const override;

	virtual void setFragmentShader(const IShaderModuleSP& fragmentShader) override;

    virtual VkTsVertexBufferType getAttributes() const override;

	virtual void setAttributes(const VkTsVertexBufferType attributes) override;

    virtual void addTexture(const ITextureSP& texture) override;

    virtual VkBool32 removeTexture(const ITextureSP& texture) override;

    virtual size_t getNumberTextures() const override;

    virtual const SmartPointerVector<ITextureSP>& getTextures() const override;

    virtual const IDescriptorPoolSP& getDescriptorPool() const override;

    virtual void setDescriptorPool(const IDescriptorPoolSP& descriptorPool) override;

    virtual IDescriptorSetsSP getDescriptorSets() const override;

    virtual void setDescriptorSets(const IDescriptorSetsSP& descriptorSets) override;

    virtual void updateDescriptorSetsRecursive(const std::string& nodeName, const uint32_t allWriteDescriptorSetsCount, VkWriteDescriptorSet* allWriteDescriptorSets) override;

    virtual void bindDescriptorSets(const std::string& nodeName, const ICommandBuffersSP& cmdBuffer, const VkPipelineLayout layout, const uint32_t bufferIndex = 0) const override;

    virtual void bindDrawIndexedRecursive(const std::string& nodeName, const ICommandBuffersSP& cmdBuffer, const IGraphicsPipelineSP& graphicsPipeline, const overwrite* renderOverwrite = nullptr, const uint32_t bufferIndex = 0) const override;

    //
    // ICloneable
    //

    virtual IBSDFMaterialSP clone() const override;

    //
    // IDestroyable
    //

    virtual void destroy() override;
};

} /* namespace vkts */

#endif /* VKTS_BSDFMATERIAL_HPP_ */
