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

namespace vkts
{

SmartPointerVector<IImageDataSP> VKTS_APIENTRY imageDataCubemap(const IImageDataSP& sourceImage, const uint32_t length, const std::string& name)
{
    if (name.size() == 0 || !sourceImage.get() || length == 0)
    {
        return SmartPointerVector<IImageDataSP>();
    }

    std::string sourceImageFilename = name;

    auto dotIndex = sourceImageFilename.rfind(".");

    if (dotIndex == sourceImageFilename.npos)
    {
        return SmartPointerVector<IImageDataSP>();
    }

    auto sourceImageName = sourceImageFilename.substr(0, dotIndex);
    auto sourceImageExtension = sourceImageFilename.substr(dotIndex);

    SmartPointerVector<IImageDataSP> result;

    for (uint32_t side = 0; side < 6; side++)
    {
    	auto currentImageData = imageDataCreate(sourceImageName + "_LAYER" + std::to_string(side) + sourceImageExtension, length, length, 1, sourceImage->getImageType(), sourceImage->getFormat());

    	if (!currentImageData.get())
    	{
    		return SmartPointerVector<IImageDataSP>();
    	}

    	result.append(currentImageData);
    }

	//

    // 0.5 as step goes form -1.0 to 1.0 and not just 0.0 to 1.0
	float step = 2.0f / (float)length;
	float offset = step * 0.5f;

	//

	glm::vec3 scanVector;

	glm::vec2 sampleLocation;

	glm::vec4 texel;

	for (uint32_t y = 0; y < length; y++)
	{
		for (uint32_t x = 0; x < length; x++)
		{
			for (uint32_t i = 0; i < 6; i++)
			{
				switch (i)
				{
					case 0:

						// Positive X
						scanVector = glm::vec3(1.0f, 1.0f - offset - step * (float)y, 1.0f - offset - step * (float)x);

						break;
					case 1:

						// Negative X
						scanVector = glm::vec3(-1.0f, 1.0f - offset - step * (float)y, -1.0f + offset + step * (float)x);

						break;
					case 2:

						// Positive Y
						scanVector = glm::vec3(-1.0f + offset + step * (float)x, 1.0f, -1.0f + offset + step * (float)y);

						break;
					case 3:

						// Negative Y
						scanVector = glm::vec3(-1.0f + offset + step * (float)x, -1.0f, 1.0f - offset - step * (float)y);

						break;
					case 4:

						// Positive Z
						scanVector = glm::vec3(-1.0f + offset + step * (float)x, 1.0f - offset - step * (float)y, 1.0f);

						break;
					case 5:

						// Negative Z
						scanVector = glm::vec3(1.0f - offset - step * (float)x, 1.0f - offset - step * (float)y, -1.0f);

						break;
				}

				scanVector = glm::normalize(scanVector);

				//

				sampleLocation.s = 0.5f + 0.5f * atan2f(scanVector.x, -scanVector.z) / VKTS_PI;
				sampleLocation.t = 1.0f - acosf(scanVector.y) / VKTS_PI;

				//

				texel = sourceImage->getSample(sampleLocation.s, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, sampleLocation.t, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, 0.5f, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0, 0);

				result[i]->setTexel(texel, x, y, 0, 0, 0);
			}
		}
	}

    return result;
}

}
