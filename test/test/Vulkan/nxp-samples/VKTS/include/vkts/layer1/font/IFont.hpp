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

#ifndef VKTS_IFONT_HPP_
#define VKTS_IFONT_HPP_

#include <vkts/vkts.hpp>

namespace vkts
{

class overwrite;

class IFont : public IDestroyable
{

public:

    IFont() :
        IDestroyable()
    {
    }

    virtual ~IFont()
    {
    }

	virtual const std::string& getFace() const = 0;

	virtual float getSize() const = 0;

	virtual float getLineHeight() const = 0;

	virtual float getLineHeight(const float fontSize) const = 0;

	virtual float getBase() const = 0;

	virtual float getScaleWidth() const = 0;

	virtual float getScaleHeight() const = 0;

	virtual const ITextureSP& getTexture() const = 0;


	virtual void drawText(const ICommandBuffersSP& cmdBuffer, const glm::mat4& viewProjection, const glm::vec2& translate, const std::string& text, const float fontSize, const glm::vec4& color) const = 0;

};

typedef std::shared_ptr<IFont> IFontSP;

} /* namespace vkts */

#endif /* VKTS_IFONT_HPP_ */
