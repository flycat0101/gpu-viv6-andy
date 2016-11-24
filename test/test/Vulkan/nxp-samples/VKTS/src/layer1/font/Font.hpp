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

#ifndef VKTS_FONT_HPP_
#define VKTS_FONT_HPP_

#include <vkts/vkts.hpp>

#include "Char.hpp"

namespace vkts
{

class Font: public IFont
{

private:

	std::string face;

	float size;

	float lineHeight;

	float base;

	float scaleWidth;

	float scaleHeight;

	ITextureSP texture;

	std::map<int32_t, Char> allCharacters;

	IBufferObjectSP vertexBuffer;

	IDescriptorSetLayoutSP descriptorSetLayout;

	IDescriptorPoolSP descriptorPool;

	IDescriptorSetsSP descriptorSets;

	IPipelineLayoutSP pipelineLayout;

	IGraphicsPipelineSP graphicsPipeline;

public:

    Font();
    Font(const Font& other) = delete;
    Font(Font&& other) = delete;
    virtual ~Font();

    Font& operator =(const Font& other) = delete;
    Font& operator =(Font && other) = delete;

    void setFace(const std::string& face);

	void setSize(const float size);

	void setLineHeight(const float lineHeight);

	void setBase(const float base);

	void setScaleWidth(const float scaleWidth);

	void setScaleHeight(const float scaleHeight);

	void setTexture(const ITextureSP& texture);

	void setChar(const Char& character);

	void setKerning(const int32_t characterId, const int32_t nextCharacterId, const float amount);

	void setVertexBuffer(const IBufferObjectSP& vertexBuffer);

	void setDescriptorSetLayout(const IDescriptorSetLayoutSP& descriptorSetLayout);

	void setDescriptorPool(const IDescriptorPoolSP& descriptorPool);

	void setDescriptorSets(const IDescriptorSetsSP& descriptorSets);

	void setPipelineLayout(const IPipelineLayoutSP& pipelineLayout);

	void setGraphicsPipeline(const IGraphicsPipelineSP& graphicsPipeline);

    //
    // IFont
    //

	virtual const std::string& getFace() const override;

	virtual float getSize() const override;

	virtual float getLineHeight() const override;

	virtual float getLineHeight(const float fontSize) const override;

	virtual float getBase() const override;

	virtual float getScaleWidth() const override;

	virtual float getScaleHeight() const override;

	virtual const ITextureSP& getTexture() const override;


	virtual void drawText(const ICommandBuffersSP& cmdBuffer, const glm::mat4& viewProjection, const glm::vec2& translate, const std::string& text, const float fontSize, const glm::vec4& color) const override;

    //
    // IDestroyable
    //

    virtual void destroy() override;
};

} /* namespace vkts */

#endif /* VKTS_FONT_HPP_ */
