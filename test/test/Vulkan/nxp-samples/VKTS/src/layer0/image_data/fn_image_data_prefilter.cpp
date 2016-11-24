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

// TODO: Merge code and make generic.

SmartPointerVector<IImageDataSP> VKTS_APIENTRY imageDataPrefilterCookTorrance(const IImageDataSP& sourceImage, const uint32_t m, const std::string& name)
{
    if (name.size() == 0 || !sourceImage.get() || sourceImage->getArrayLayers() != 6 || sourceImage->getDepth() != 1 || sourceImage->getWidth() != sourceImage->getHeight() || m == 0 || m > 32 || sourceImage->getWidth() < 2)
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

    IImageDataSP currentTargetImage;
    std::string targetImageFilename;

    SmartPointerVector<IImageDataSP> result;

    // Create mip maps for all six layers.
    for (uint32_t layer = 0; layer < 6; layer++)
    {
        int32_t level = 0;

        int32_t width = sourceImage->getWidth();
        int32_t height = sourceImage->getHeight();

		while (width > 0 || height > 0)
		{
			targetImageFilename = sourceImageName + "_LEVEL" + std::to_string(level++) + "_LAYER" + std::to_string(layer) + "_COOKTORRANCE" + sourceImageExtension;

			currentTargetImage = imageDataCreate(targetImageFilename, glm::max(width, 1), glm::max(height, 1), 1, 0.0f, 0.0f, 0.0f, 0.0f, sourceImage->getImageType(), sourceImage->getFormat());

			if (!currentTargetImage.get())
			{
				return SmartPointerVector<IImageDataSP>();
			}

			result.append(currentTargetImage);

			//

			width = width / 2;
			height = height / 2;
		}
    }

    uint32_t roughnessSamples = (uint32_t)(result.size() / 6);

    //
    // Cook torrance specular.
    //

    uint32_t samples = 1 << m;

	for (uint32_t roughnessSampleIndex = 0; roughnessSampleIndex < roughnessSamples; roughnessSampleIndex++)
	{
		uint32_t length = (uint32_t)sourceImage->getWidth() / (1 << roughnessSampleIndex);

		// 0.5 as step goes form -1.0 to 1.0 and not just 0.0 to 1.0
		float step = 2.0f / (float)length;
		float offset = step * 0.5f;

		//

		float roughness = (float)roughnessSampleIndex / (float)(roughnessSamples - 1);

		//

		glm::vec3 scanVector;

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

					glm::mat3 basis = renderGetBasis(scanVector);

					glm::vec3 colorCookTorrance = glm::vec3(0.0f, 0.0f, 0.0f);

					float sampleDivisior = 0.0f;

					for (uint32_t sampleIndex = 0; sampleIndex < samples; sampleIndex++)
					{
						glm::vec2 randomPoint = randomHammersley(sampleIndex, m);

						// N = V
						auto currentColorCookTorrance = renderCookTorrance(sourceImage, VK_FILTER_LINEAR, 0, randomPoint, basis, scanVector, scanVector, roughness);

						if (!std::isnan(currentColorCookTorrance.x) && !std::isnan(currentColorCookTorrance.y) && !std::isnan(currentColorCookTorrance.z))
						{
							colorCookTorrance += currentColorCookTorrance;

							sampleDivisior += 1.0f;
						}
					}

					//

					if (sampleDivisior > 0.0f)
					{
						colorCookTorrance = colorCookTorrance / sampleDivisior;
					}

					result[i * roughnessSamples + roughnessSampleIndex]->setTexel(glm::vec4(colorCookTorrance, 1.0f), x, y, 0, 0, 0);
				}
			}
		}
	}

    return result;
}

SmartPointerVector<IImageDataSP> VKTS_APIENTRY imageDataPrefilterOrenNayar(const IImageDataSP& sourceImage, const uint32_t m, const std::string& name)
{
    if (name.size() == 0 || !sourceImage.get() || sourceImage->getArrayLayers() != 6 || sourceImage->getDepth() != 1 || sourceImage->getWidth() != sourceImage->getHeight() || m == 0 || m > 32 || sourceImage->getWidth() < 2)
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

    IImageDataSP currentTargetImage;
    std::string targetImageFilename;

    SmartPointerVector<IImageDataSP> result;

    // Create mip maps for all six layers.
    for (uint32_t layer = 0; layer < 6; layer++)
    {
        int32_t level = 0;

        int32_t width = sourceImage->getWidth();
        int32_t height = sourceImage->getHeight();

		while (width > 1 || height > 1)
		{
			targetImageFilename = sourceImageName + "_LEVEL" + std::to_string(level++) + "_LAYER" + std::to_string(layer) + "_ORENNAYAR" + sourceImageExtension;

			currentTargetImage = imageDataCreate(targetImageFilename, width, height, 1, 0.0f, 0.0f, 0.0f, 0.0f, sourceImage->getImageType(), sourceImage->getFormat());

			if (!currentTargetImage.get())
			{
				return SmartPointerVector<IImageDataSP>();
			}

			result.append(currentTargetImage);

			//

			width = glm::max(width / 2, 1);
			height = glm::max(height / 2, 1);
		}
    }

    uint32_t roughnessSamples = (uint32_t)(result.size() / 6);

    //
    // Oren-Nayar diffuse.
    //

    uint32_t samples = 1 << m;

    uint32_t length = (uint32_t)sourceImage->getWidth();

    // 0.5 as step goes form -1.0 to 1.0 and not just 0.0 to 1.0
	float step = 2.0f / (float)length;
	float offset = step * 0.5f;

	//

	glm::vec3 scanVector;

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

				glm::mat3 basis = renderGetBasis(scanVector);

				for (uint32_t roughnessSampleIndex = 0; roughnessSampleIndex < roughnessSamples; roughnessSampleIndex++)
				{
					glm::vec3 colorOrenNayar = glm::vec3(0.0f, 0.0f, 0.0f);

					float roughness = (float)roughnessSampleIndex / (float)(roughnessSamples - 1);

					for (uint32_t sampleIndex = 0; sampleIndex < samples; sampleIndex++)
					{
						glm::vec2 randomPoint = randomHammersley(sampleIndex, m);

						// N = V
						colorOrenNayar += renderOrenNayar(sourceImage, VK_FILTER_LINEAR, 0, randomPoint, basis, scanVector, scanVector, roughness);
					}

					//

					result[i * roughnessSamples + roughnessSampleIndex]->setTexel(glm::vec4(colorOrenNayar / (float) samples, 1.0f), x, y, 0, 0, 0);
				}
			}
		}
	}

    return result;
}

SmartPointerVector<IImageDataSP> VKTS_APIENTRY imageDataPrefilterLambert(const IImageDataSP& sourceImage, const uint32_t m, const std::string& name)
{
    if (name.size() == 0 || !sourceImage.get() || sourceImage->getArrayLayers() != 6 || sourceImage->getDepth() != 1 || sourceImage->getWidth() != sourceImage->getHeight() || m == 0 || m > 32)
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

    IImageDataSP currentTargetImage;
    std::string targetImageFilename;

    SmartPointerVector<IImageDataSP> result;

    // Create mip maps for all six layers.
    for (uint32_t layer = 0; layer < 6; layer++)
    {
		targetImageFilename = sourceImageName + "_LEVEL0_LAYER" + std::to_string(layer) + "_LAMBERT" + sourceImageExtension;

		currentTargetImage = imageDataCreate(targetImageFilename, sourceImage->getWidth(), sourceImage->getHeight(), 1, 0.0f, 0.0f, 0.0f, 0.0f, sourceImage->getImageType(), sourceImage->getFormat());

		if (!currentTargetImage.get())
		{
			return SmartPointerVector<IImageDataSP>();
		}

		result.append(currentTargetImage);
    }

    //
    // Lambert diffuse.
    //

    uint32_t samples = 1 << m;

    uint32_t length = (uint32_t)sourceImage->getWidth();

    // 0.5 as step goes form -1.0 to 1.0 and not just 0.0 to 1.0
	float step = 2.0f / (float)length;
	float offset = step * 0.5f;

	//

	glm::vec3 scanVector;

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

				glm::mat3 basis = renderGetBasis(scanVector);

				glm::vec3 colorLambert = glm::vec3(0.0f, 0.0f, 0.0f);

				for (uint32_t sampleIndex = 0; sampleIndex < samples; sampleIndex++)
				{
					glm::vec2 randomPoint = randomHammersley(sampleIndex, m);

					colorLambert += renderLambert(sourceImage, VK_FILTER_LINEAR, 0, randomPoint, basis);
				}

				result[i]->setTexel(glm::vec4(colorLambert / (float) samples, 1.0f), x, y, 0, 0, 0);
			}
		}
	}

    return result;
}

IImageDataSP VKTS_APIENTRY imageDataEnvironmentBRDF(const uint32_t length, const uint32_t m, const std::string& name)
{
	if (name.size() == 0 || length <= 1 || m == 0 || m > 32)
	{
		return IImageDataSP();
	}

    std::string sourceImageFilename = name;

    auto dotIndex = sourceImageFilename.rfind(".");

    if (dotIndex == sourceImageFilename.npos)
    {
    	return IImageDataSP();
    }

    auto sourceImageName = sourceImageFilename.substr(0, dotIndex);
    auto sourceImageExtension = sourceImageFilename.substr(dotIndex);

    std::string targetImageFilename = sourceImageName + "_" + std::to_string(length) + "_" + std::to_string(m) + sourceImageExtension;

    auto currentTargetImage = imageDataCreate(targetImageFilename, length, length, 1, 0.0f, 0.0f, 0.0f, 0.0f, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32_SFLOAT);

	if (!currentTargetImage.get())
	{
		return IImageDataSP();
	}

	//

	uint32_t samples = 1 << m;

	for (uint32_t y = 0; y < length; y++)
	{
		float roughness = float(y) / float(length - 1);

		for (uint32_t x = 0; x < length; x++)
		{
			float NdotV = float(x) / float(length - 1);

			glm::vec3 V = glm::vec3(sqrtf(1.0f - NdotV * NdotV), 0.0f, NdotV);

			glm::vec2 outputCookTorrance = glm::vec2(0.0f, 0.0f);

			for (uint32_t sampleIndex = 0; sampleIndex < samples; sampleIndex++)
			{
				glm::vec2 randomPoint = randomHammersley(sampleIndex, m);

				// Specular
				outputCookTorrance += renderIntegrateCookTorrance(randomPoint, NdotV, V, roughness);
			}

			currentTargetImage->setTexel(glm::vec4(outputCookTorrance / float(samples), 0.0f, 1.0f), x, y, 0, 0, 0);
		}
	}

	//

	return currentTargetImage;
}

}
