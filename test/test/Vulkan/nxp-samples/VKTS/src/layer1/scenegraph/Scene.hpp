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

#ifndef VKTS_SCENE_HPP_
#define VKTS_SCENE_HPP_

#include <vkts/vkts.hpp>

namespace vkts
{

class Scene: public IScene
{

private:

    std::string name;

    SmartPointerVector<IObjectSP> allObjects;

    ITextureSP environment;

    ITextureSP diffuseEnvironment;

    ITextureSP specularEnvironment;

    ITextureSP lut;

public:

    Scene();
    Scene(const Scene& other);
    Scene(Scene&& other) = delete;
    virtual ~Scene();

    Scene& operator =(const Scene& other) = delete;

    Scene& operator =(Scene && other) = delete;

    //
    // IScene
    //

    virtual const std::string& getName() const override;

    virtual void setName(const std::string& name) override;

    virtual void addObject(const IObjectSP& object) override;

    virtual VkBool32 removeObject(const IObjectSP& object) override;

    virtual IObjectSP findObject(const std::string& name) const override;

    virtual size_t getNumberObjects() const override;

    virtual const SmartPointerVector<IObjectSP>& getObjects() const override;

    virtual void setEnvironment(const ITextureSP& environment) override;

    virtual ITextureSP getEnvironment() const override;

    virtual void setDiffuseEnvironment(const ITextureSP& diffuseEnvironment) override;

    virtual ITextureSP getDiffuseEnvironment() const override;

    virtual void setSpecularEnvironment(const ITextureSP& cookTorranceEnvironment) override;

    virtual ITextureSP getSpecularEnvironment() const override;

    virtual void setLut(const ITextureSP& environment) override;

    virtual ITextureSP getLut() const override;

    virtual void updateDescriptorSetsRecursive(const uint32_t allWriteDescriptorSetsCount, VkWriteDescriptorSet* allWriteDescriptorSets) override;

    virtual void bindDrawIndexedRecursive(const ICommandBuffersSP& cmdBuffer, const SmartPointerVector<IGraphicsPipelineSP>& allGraphicsPipelines, const overwrite* renderOverwrite = nullptr, const uint32_t bufferIndex = 0, const uint32_t objectOffset = 0, const uint32_t objectStep = 1, const size_t objectLimit = SIZE_MAX) const override;

    virtual void updateRecursive(const IUpdateThreadContext& updateContext, const uint32_t objectOffset = 0, const uint32_t objectStep = 1, const size_t objectLimit = SIZE_MAX) override;

    //
    // ICloneable
    //

    virtual ISceneSP clone() const override;

    //
    // IDestroyable
    //

    virtual void destroy() override;

};

} /* namespace vkts */

#endif /* VKTS_SCENE_HPP_ */
