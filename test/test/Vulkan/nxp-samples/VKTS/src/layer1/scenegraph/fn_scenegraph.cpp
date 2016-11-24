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

#include "../../fn_internal.hpp"

#include "../../layer0/binary_buffer/BinaryBuffer.hpp"

#include "../../layer0/text_buffer/TextBuffer.hpp"

#include "../../layer0/image_view/ImageView.hpp"

#include "../../layer0/sampler/Sampler.hpp"

#include "../buffer_object/BufferObject.hpp"

#include "../texture/Texture.hpp"

#include "Animation.hpp"
#include "BSDFMaterial.hpp"

#include "Channel.hpp"

#include "PhongMaterial.hpp"

#include "SubMesh.hpp"

#include "Mesh.hpp"

#include "Node.hpp"

#include "Object.hpp"

#include "Context.hpp"

#include "Scene.hpp"

namespace vkts
{

static VkBool32 scengraphGetDirectory(char* directory, const char* filename)
{
    if (!directory || !filename)
    {
        return VK_FALSE;
    }

    auto position = strrchr(filename, '/');

    if (position)
    {
        strncpy(directory, filename, (position - filename) + 1);

        return VK_TRUE;
    }

    position = strrchr(filename, '\\');

    if (position)
    {
        strncpy(directory, filename, (position - filename) + 1);

        return VK_TRUE;
    }

    return VK_FALSE;
}

static VkBool32 scenegraphSkipBuffer(const char* buffer)
{
    if (!buffer)
    {
        return VK_TRUE;
    }

    if (strncmp(buffer, "#", 1) == 0)
    {
        // Comment, just skip.

        return VK_TRUE;
    }
    else if (strncmp(buffer, " ", 1) == 0 || strncmp(buffer, "\t", 1) == 0 || strncmp(buffer, "\r", 1) == 0 || strncmp(buffer, "\n", 1) == 0 || strlen(buffer) == 0)
    {
        // Empty line, just skip.

        return VK_TRUE;
    }

    return VK_FALSE;
}

static VkBool32 scenegraphIsToken(const char* buffer, const char* token)
{
    if (!buffer || !token)
    {
        return VK_FALSE;
    }

    if (!(buffer[strlen(token)] == ' ' || buffer[strlen(token)] == '\t' || buffer[strlen(token)] == '\n' || buffer[strlen(token)] == '\r' || buffer[strlen(token)] == '\0'))
    {
        return VK_FALSE;
    }

    if (strncmp(buffer, token, strlen(token)) == 0)
    {
        return VK_TRUE;
    }

    return VK_FALSE;
}

static VkBool32 scenegraphParseString(const char* buffer, char* string)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %s", token, string) != 2)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseStringTuple(const char* buffer, char* string0, char* string1)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %s %s", token, string0, string1) != 3)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseBool(const char* buffer, VkBool32* scalar)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    char value[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %s", token, value) != 2)
    {
        return VK_FALSE;
    }

    if (strcmp(value, "true") == 0)
    {
    	*scalar = VK_TRUE;
    }
    else if (strcmp(value, "false") == 0)
    {
    	*scalar = VK_FALSE;
    }
    else
    {
    	return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseFloat(const char* buffer, float* scalar)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f", token, scalar) != 2)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseVec2(const char* buffer, float vec2[2])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f %f", token, &vec2[0], &vec2[1]) != 3)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseVec3(const char* buffer, float vec3[3])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f %f %f", token, &vec3[0], &vec3[1], &vec3[2]) != 4)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseVec4(const char* buffer, float vec4[4])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f %f %f %f", token, &vec4[0], &vec4[1], &vec4[2], &vec4[3]) != 5)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseVec6(const char* buffer, float vec6[6])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f %f %f %f %f %f", token, &vec6[0], &vec6[1], &vec6[2], &vec6[3], &vec6[4], &vec6[5]) != 7)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseVec8(const char* buffer, float vec8[8])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %f %f %f %f %f %f %f %f", token, &vec8[0], &vec8[1], &vec8[2], &vec8[3], &vec8[4], &vec8[5], &vec8[6], &vec8[7]) != 9)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseInt(const char* buffer, int32_t* scalar)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %d", token, scalar) != 2)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseUIntHex(const char* buffer, uint32_t* scalar)
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %x", token, scalar) != 2)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static VkBool32 scenegraphParseIVec3(const char* buffer, int32_t ivec3[3])
{
    if (!buffer)
    {
        return VK_FALSE;
    }

    char token[VKTS_MAX_TOKEN_CHARS + 1];

    if (sscanf(buffer, "%s %d %d %d", token, &ivec3[0], &ivec3[1], &ivec3[2]) != 4)
    {
        return VK_FALSE;
    }

    return VK_TRUE;
}

static void scenegraphUnknownBuffer(const char* buffer)
{
    if (!buffer)
    {
        return;
    }

    std::string unknown(buffer);

    if (unknown.length() > 0 && (unknown[unknown.length() - 1] == '\r' || unknown[unknown.length() - 1] == '\n'))
    {
        logPrint(VKTS_LOG_WARNING, "Scenegraph: Could not parse line '%s'", unknown.substr(0, unknown.length() - 1).c_str());
    }
    else
    {
        logPrint(VKTS_LOG_WARNING, "Scenegraph: Could not parse line '%s'", unknown.c_str());
    }
}

static ITextureSP scenegraphCreateTexture(const float red, const float green, const float blue, const VkFormat format, const IContextSP& context)
{
    char imageDataName[VKTS_MAX_BUFFER_CHARS + 1];
    char textureName[VKTS_MAX_BUFFER_CHARS + 1];

    //

    if (format == VK_FORMAT_R8_UNORM)
    {
        sprintf(imageDataName, "image_%1.3f_unorm.tga", red);

        sprintf(textureName, "texture_%1.3f_unorm", red);
    }
    else if (format == VK_FORMAT_R8G8_UNORM)
    {
        sprintf(imageDataName, "image_%1.3f_%1.3f_unorm.tga", red, green);

        sprintf(textureName, "texture_%1.3f_%1.3f_unorm", red, green);
    }
    else if (format == VK_FORMAT_R8G8B8_UNORM)
    {
        sprintf(imageDataName, "image_%1.3f_%1.3f_%1.3f_unorm.tga", red, green, blue);

        sprintf(textureName, "texture_%1.3f_%1.3f_%1.3f_unorm", red, green, blue);
    }
    else if (format == VK_FORMAT_R8G8B8A8_UNORM)
    {
        sprintf(imageDataName, "image_%1.3f_%1.3f_%1.3f_unorm.tga", red, green, blue);

        sprintf(textureName, "texture_%1.3f_%1.3f_%1.3f_unorm", red, green, blue);
    }
    else
    {
        return ITextureSP();
    }

    //

    auto imageData = context->useImageData(imageDataName);

    if (!imageData.get())
    {
        imageData = imageDataCreate(imageDataName, 1, 1, 1, red, green, blue, 0.0f, VK_IMAGE_TYPE_2D, format);

        logPrint(VKTS_LOG_DEBUG, "Scenegraph: Create image '%s'", imageDataName);

        if (!imageData.get())
        {
            return ITextureSP();
        }

        context->addImageData(imageData);
    }

    //

    VkImageTiling imageTiling;
    VkMemoryPropertyFlags memoryPropertyFlags;

    if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, imageData->getFormat(), imageData->getImageType(), 0, imageData->getExtent3D(), imageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, imageData->getSize()))
    {
        if (format == VK_FORMAT_R8G8B8_UNORM)
        {
            context->removeImageData(imageData);

            sprintf(imageDataName, "image_%1.3f_%1.3f_%1.3f_1.0_unorm.tga", red, green, blue);

            sprintf(textureName, "texture_%1.3f_%1.3f_%1.3f_1.0_unorm", red, green, blue);

            auto convertedImageData = context->useImageData(imageDataName);

            if (!convertedImageData.get())
            {
                convertedImageData = imageDataConvert(imageData, VK_FORMAT_R8G8B8A8_UNORM, imageDataName);

                if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, convertedImageData->getFormat(), convertedImageData->getImageType(), 0, convertedImageData->getExtent3D(), convertedImageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, convertedImageData->getSize()))
                {
                    logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                    return ITextureSP();
                }

                context->addImageData(convertedImageData);
            }
            else
            {
                if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, convertedImageData->getFormat(),convertedImageData->getImageType(), 0, convertedImageData->getExtent3D(), convertedImageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, convertedImageData->getSize()))
                {
                    logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                    return ITextureSP();
                }
            }

            imageData = convertedImageData;
        }
        else
        {
            return ITextureSP();
        }
    }

    //

    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    VkAccessFlags srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

    if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
    {
    	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    	srcAccessMask = 0;
    }

    //

    VkImageCreateInfo imageCreateInfo;

    memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imageCreateInfo.flags = 0;
    imageCreateInfo.imageType = imageData->getImageType();
    imageCreateInfo.format = imageData->getFormat();
    imageCreateInfo.extent = imageData->getExtent3D();
    imageCreateInfo.mipLevels = imageData->getMipLevels();
    imageCreateInfo.arrayLayers = imageData->getArrayLayers();
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = imageTiling;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = initialLayout;

    VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, imageData->getMipLevels(), 0, imageData->getArrayLayers()};

    IDeviceMemorySP stageDeviceMemory;
    IBufferSP stageBuffer;
    IImageSP stageImage;

    auto memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), textureName, imageData, imageCreateInfo, srcAccessMask, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, memoryPropertyFlags);

    context->addStageImage(stageImage);
    context->addStageBuffer(stageBuffer);
    context->addStageDeviceMemory(stageDeviceMemory);

    if (!memoryImage.get())
    {
        return ITextureSP();
    }

    context->addMemoryImage(memoryImage);

    //

    auto texture = textureCreate(context->getInitialResources(), textureName, VK_FALSE, VK_FALSE, memoryImage, context->getSamplerCreateInfo(), context->getImageViewCreateInfo());

    if (!texture.get())
    {
        return ITextureSP();
    }

    context->addTexture(texture);

    return texture;
}

static VkBool32 scenegraphLoadImages(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    VkBool32 bdata;

    std::string memoryImageName;
    VkBool32 mipMap = VK_FALSE;
    VkBool32 environment = VK_FALSE;
    VkBool32 preFiltered = VK_FALSE;
    IImageDataSP imageData;

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }
        if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
            	logPrint(VKTS_LOG_ERROR, "Scenegraph: Invalid name");

                return VK_FALSE;
            }

            memoryImageName = sdata;

            mipMap = VK_FALSE;
            environment = VK_FALSE;
            preFiltered = VK_FALSE;
        }
        else if (scenegraphIsToken(buffer, "mipmap"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            mipMap = bdata;
        }
        else if (scenegraphIsToken(buffer, "environment"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            environment = bdata;
        }
        else if (scenegraphIsToken(buffer, "pre_filtered"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            preFiltered = bdata;
        }
        else if (scenegraphIsToken(buffer, "image_data"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (mipMap && environment)
            {
            	return VK_FALSE;
            }

            if (preFiltered && !environment)
            {
            	return VK_FALSE;
            }

            //

            std::string finalImageDataFilename = std::string(directory) + std::string(sdata);

            imageData = context->useImageData(finalImageDataFilename.c_str());

            if (!imageData.get())
            {
                imageData = context->useImageData(sdata);

                if (!imageData.get())
                {
                    imageData = imageDataLoad(finalImageDataFilename.c_str());

                    if (!imageData.get())
                    {
                        imageData = imageDataLoad(sdata);

                        if (!imageData.get())
                        {
                        	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not load image data '%s'", finalImageDataFilename.c_str());

                            return VK_FALSE;
                        }
                    }

                    //

                    VkImageTiling imageTiling;
                    VkMemoryPropertyFlags memoryPropertyFlags;

                    if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, imageData->getFormat(), imageData->getImageType(), 0, imageData->getExtent3D(), imageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, imageData->getSize()))
                    {
                        if (imageData->getFormat() == VK_FORMAT_R8G8B8_UNORM)
                        {
                            imageData = imageDataConvert(imageData, VK_FORMAT_R8G8B8A8_UNORM, imageData->getName());

                            if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, imageData->getFormat(), imageData->getImageType(), 0, imageData->getExtent3D(), imageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, imageData->getSize()))
                            {
                                logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                                return VK_FALSE;
                            }
                        }
                        else
                        {
                            logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                            return VK_FALSE;
                        }
                    }

                    if (mipMap && (imageData->getExtent3D().width > 1 || imageData->getExtent3D().height > 1 || imageData->getExtent3D().depth > 1))
                    {
                        // Mipmaping image creation.

                        auto dotIndex = finalImageDataFilename.rfind(".");

                        if (dotIndex == finalImageDataFilename.npos)
                        {
                        	logPrint(VKTS_LOG_ERROR, "Scenegraph: No valid image filename '%s'", finalImageDataFilename.c_str());

                            return VK_FALSE;
                        }

                        auto sourceImageName = finalImageDataFilename.substr(0, dotIndex);
                        auto sourceImageExtension = finalImageDataFilename.substr(dotIndex);

                        int32_t width = imageData->getWidth();
                        int32_t height = imageData->getHeight();
                        int32_t depth = imageData->getDepth();

                        SmartPointerVector<IImageDataSP> allMipMaps;

                        if (cacheGetEnabled())
                        {
                            allMipMaps.append(imageData);

                            int32_t level = 1;

                            while (width > 1 || height > 1 || depth > 1)
							{
								width = glm::max(width / 2, 1);
								height = glm::max(height / 2, 1);
								depth = glm::max(depth / 2, 1);

								auto targetImageFilename = sourceImageName + "_LEVEL" + std::to_string(level++) + sourceImageExtension;

								auto targetImage = cacheLoadImageData(targetImageFilename.c_str());

								if (!targetImage.get())
								{
									allMipMaps.clear();

									break;
								}

								allMipMaps.append(targetImage);

								//

								width = targetImage->getWidth();
								height = targetImage->getHeight();
								depth = targetImage->getDepth();
							}
                        }

                        //

                        if (allMipMaps.size() == 0)
                        {
                        	allMipMaps = imageDataMipmap(imageData, VK_FALSE, finalImageDataFilename);

                            if (allMipMaps.size() == 0)
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create mip maps for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            if (cacheGetEnabled())
                            {
								logPrint(VKTS_LOG_INFO, "Scenegraph: Storing cached data for '%s'", finalImageDataFilename.c_str());

								// Only cache mip maps sub levels.
								for (size_t i = 1; i < allMipMaps.size(); i++)
								{
									cacheSaveImageData(allMipMaps[i]);
								}
                            }
                        }
                        else
                        {
                        	logPrint(VKTS_LOG_INFO, "Scenegraph: Using cached data for '%s'", finalImageDataFilename.c_str());
                        }

                        imageData = imageDataMerge(allMipMaps, finalImageDataFilename, (uint32_t)allMipMaps.size(), 1);

                        if (!imageData.get())
                        {
                        	logPrint(VKTS_LOG_ERROR, "Scenegraph: No merged image for '%s'", finalImageDataFilename.c_str());

                            return VK_FALSE;
                        }
                    }
                    else if (environment)
                    {
                    	// Cube map image creation.

                        auto dotIndex = finalImageDataFilename.rfind(".");

                        if (dotIndex == finalImageDataFilename.npos)
                        {
                        	logPrint(VKTS_LOG_ERROR, "Scenegraph: No valid image filename '%s'", finalImageDataFilename.c_str());

                            return VK_FALSE;
                        }

                        auto sourceImageName = finalImageDataFilename.substr(0, dotIndex);
                        auto sourceImageExtension = finalImageDataFilename.substr(dotIndex);

                        SmartPointerVector<IImageDataSP> allCubeMaps;

                        if (cacheGetEnabled())
                        {
                            for (uint32_t layer = 0; layer < 6; layer++)
							{
								auto targetImageFilename = sourceImageName + "_LAYER" + std::to_string(layer) + sourceImageExtension;

								auto targetImage = cacheLoadImageData(targetImageFilename.c_str());

								if (!targetImage.get())
								{
									allCubeMaps.clear();

									break;
								}

								allCubeMaps.append(targetImage);
							}
                        }

                        //

                        if (allCubeMaps.size() == 0)
                        {
                        	allCubeMaps = imageDataCubemap(imageData, (uint32_t)imageData->getHeight() / 2, finalImageDataFilename);

                            if (allCubeMaps.size() == 0)
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create cube maps for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            if (cacheGetEnabled())
                            {
								logPrint(VKTS_LOG_INFO, "Scenegraph: Storing cached data for '%s'", finalImageDataFilename.c_str());

								for (size_t i = 0; i < allCubeMaps.size(); i++)
								{
									cacheSaveImageData(allCubeMaps[i]);
								}
                            }
                        }
                        else
                        {
                        	logPrint(VKTS_LOG_INFO, "Scenegraph: Using cached data for '%s'", finalImageDataFilename.c_str());
                        }

                        imageData = imageDataMerge(allCubeMaps, finalImageDataFilename, 1, (uint32_t)allCubeMaps.size());

                        if (!imageData.get())
                        {
                        	logPrint(VKTS_LOG_ERROR, "Scenegraph: No merged image for '%s'", finalImageDataFilename.c_str());

                            return VK_FALSE;
                        }

                        if (preFiltered)
                        {
                            if (memoryImageName == "")
                            {
                                return VK_FALSE;
                            }

                        	// Pre-filtered diffuse cube map.

                        	SmartPointerVector<IImageDataSP> allDiffuseCubeMaps;

                            if (cacheGetEnabled())
                            {
                                for (uint32_t layer = 0; layer < 6; layer++)
    							{
    								auto targetImageFilename = sourceImageName + "_LEVEL0_LAYER" + std::to_string(layer) + "_LAMBERT" + sourceImageExtension;

    								auto targetImage = cacheLoadImageData(targetImageFilename.c_str());

    								if (!targetImage.get())
    								{
    									allDiffuseCubeMaps.clear();

    									break;
    								}

    								allDiffuseCubeMaps.append(targetImage);
    							}
                            }

                            if (allDiffuseCubeMaps.size() == 0)
                            {
                            	allDiffuseCubeMaps = imageDataPrefilterLambert(imageData, VKTS_BSDF_M_CUBE_MAP, finalImageDataFilename);

                                if (allDiffuseCubeMaps.size() == 0)
                                {
                                	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create diffuse cube maps for '%s'", finalImageDataFilename.c_str());

                                    return VK_FALSE;
                                }

                                if (cacheGetEnabled())
                                {
    								logPrint(VKTS_LOG_INFO, "Scenegraph: Storing cached data for '%s'", finalImageDataFilename.c_str());

    								for (size_t i = 0; i < allDiffuseCubeMaps.size(); i++)
    								{
    									cacheSaveImageData(allDiffuseCubeMaps[i]);
    								}
                                }
                            }
                            else
                            {
                            	logPrint(VKTS_LOG_INFO, "Scenegraph: Using cached data for '%s'", finalImageDataFilename.c_str());
                            }

                            auto diffuseImageData = imageDataMerge(allDiffuseCubeMaps, finalImageDataFilename, 1, (uint32_t)allDiffuseCubeMaps.size());

                            if (!diffuseImageData.get())
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No merged image for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            context->addImageData(diffuseImageData);

                            //

                            VkImageTiling imageTiling;
                            VkMemoryPropertyFlags memoryPropertyFlags;

                            if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, diffuseImageData->getFormat(), diffuseImageData->getImageType(), 0, diffuseImageData->getExtent3D(), diffuseImageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, diffuseImageData->getSize()))
                            {
                                logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                                return VK_FALSE;
                            }


                            VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                            VkAccessFlags srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

                            if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
                            {
                            	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            	srcAccessMask = 0;
                            }

                            //

                            VkImageCreateInfo imageCreateInfo;

                            memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

                            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

                            imageCreateInfo.flags = environment ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
                            imageCreateInfo.imageType = diffuseImageData->getImageType();
                            imageCreateInfo.format = diffuseImageData->getFormat();
                            imageCreateInfo.extent = diffuseImageData->getExtent3D();
                            imageCreateInfo.mipLevels = diffuseImageData->getMipLevels();
                            imageCreateInfo.arrayLayers = diffuseImageData->getArrayLayers();
                            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                            imageCreateInfo.tiling = imageTiling;
                            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                            imageCreateInfo.queueFamilyIndexCount = 0;
                            imageCreateInfo.pQueueFamilyIndices = nullptr;
                            imageCreateInfo.initialLayout = initialLayout;

                            VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, diffuseImageData->getMipLevels(), 0, diffuseImageData->getArrayLayers()};

                            IDeviceMemorySP stageDeviceMemory;
                            IImageSP stageImage;
                            IBufferSP stageBuffer;

                            auto memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), memoryImageName + "_LAMBERT", diffuseImageData, imageCreateInfo, srcAccessMask, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, memoryPropertyFlags);

                            context->addStageImage(stageImage);
                            context->addStageBuffer(stageBuffer);
                            context->addStageDeviceMemory(stageDeviceMemory);

                            if (!memoryImage.get())
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No memory image for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            context->addMemoryImage(memoryImage);

                            //
                            //

                        	// Pre-filtered cook torrance cube map.

                        	SmartPointerVector<IImageDataSP> allCookTorranceCubeMaps;

                        	uint32_t levelCount = 1;

                            if (cacheGetEnabled())
                            {
                                for (uint32_t layer = 0; layer < 6; layer++)
    							{
                                    for (uint32_t level = 0; level < levelCount; level++)
        							{
										auto targetImageFilename = sourceImageName + "_LEVEL" + std::to_string(level) + "_LAYER" + std::to_string(layer) + "_COOKTORRANCE" + sourceImageExtension;

										auto targetImage = cacheLoadImageData(targetImageFilename.c_str());

										if (!targetImage.get())
										{
											allCookTorranceCubeMaps.clear();

											break;
										}

										// Gather level count iterations by first image.
										if (layer == 0 && level == 0)
										{
											uint32_t testWidth = targetImage->getWidth();

											while (testWidth > 1)
											{
												testWidth /= 2;

												levelCount++;
											}
										}

										allCookTorranceCubeMaps.append(targetImage);
    								}
    							}
                            }

                            if (allCookTorranceCubeMaps.size() == 0)
                            {
                            	allCookTorranceCubeMaps = imageDataPrefilterCookTorrance(imageData, VKTS_BSDF_M_CUBE_MAP, finalImageDataFilename);

                                if (allCookTorranceCubeMaps.size() == 0)
                                {
                                	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create cook torrance cube maps for '%s'", finalImageDataFilename.c_str());

                                    return VK_FALSE;
                                }

                                levelCount = (uint32_t)allCookTorranceCubeMaps.size() / 6;

                                if (cacheGetEnabled())
                                {
    								logPrint(VKTS_LOG_INFO, "Scenegraph: Storing cached data for '%s'", finalImageDataFilename.c_str());

    								for (size_t i = 0; i < allCookTorranceCubeMaps.size(); i++)
    								{
    									cacheSaveImageData(allCookTorranceCubeMaps[i]);
    								}
                                }
                            }
                            else
                            {
                            	logPrint(VKTS_LOG_INFO, "Scenegraph: Using cached data for '%s'", finalImageDataFilename.c_str());
                            }

                            auto cookTorranceImageData = imageDataMerge(allCookTorranceCubeMaps, finalImageDataFilename, levelCount, 6);

                            if (!cookTorranceImageData.get())
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No merged image for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            context->addImageData(cookTorranceImageData);

                            //

                            if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, cookTorranceImageData->getFormat(), cookTorranceImageData->getImageType(), 0, cookTorranceImageData->getExtent3D(), cookTorranceImageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, cookTorranceImageData->getSize()))
                            {
                                logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                                return VK_FALSE;
                            }


                            initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

                            if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
                            {
                            	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            	srcAccessMask = 0;
                            }

                            //

                            memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

                            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

                            imageCreateInfo.flags = environment ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
                            imageCreateInfo.imageType = cookTorranceImageData->getImageType();
                            imageCreateInfo.format = cookTorranceImageData->getFormat();
                            imageCreateInfo.extent = cookTorranceImageData->getExtent3D();
                            imageCreateInfo.mipLevels = cookTorranceImageData->getMipLevels();
                            imageCreateInfo.arrayLayers = cookTorranceImageData->getArrayLayers();
                            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                            imageCreateInfo.tiling = imageTiling;
                            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                            imageCreateInfo.queueFamilyIndexCount = 0;
                            imageCreateInfo.pQueueFamilyIndices = nullptr;
                            imageCreateInfo.initialLayout = initialLayout;

                            subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, cookTorranceImageData->getMipLevels(), 0, cookTorranceImageData->getArrayLayers()};

                            memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), memoryImageName + "_COOKTORRANCE", cookTorranceImageData, imageCreateInfo, srcAccessMask, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, memoryPropertyFlags);

                            context->addStageImage(stageImage);
                            context->addStageBuffer(stageBuffer);
                            context->addStageDeviceMemory(stageDeviceMemory);

                            if (!memoryImage.get())
                            {
                            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No memory image for '%s'", finalImageDataFilename.c_str());

                                return VK_FALSE;
                            }

                            context->addMemoryImage(memoryImage);

                            //
                            //

                        	// Generate BSDF environment look up table.

                            auto lutName = "BSDF_LUT_" + std::to_string(VKTS_BSDF_LENGTH) + "_" + std::to_string(VKTS_BSDF_M);

                        	auto targetImageFilename = "texture/" + lutName + ".data";

                        	IImageDataSP lut = imageDataLoadRaw(targetImageFilename.c_str(), VKTS_BSDF_LENGTH, VKTS_BSDF_LENGTH, VK_FORMAT_R32G32_SFLOAT);

                            if (!lut.get())
                            {
                            	lut = imageDataEnvironmentBRDF(VKTS_BSDF_LENGTH, VKTS_BSDF_M, "BSDF_LUT.data");

                                if (!lut.get())
                                {
                                	logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not generate BSDF lut");

                                    return VK_FALSE;
                                }

                            	if (!imageDataSave(targetImageFilename.c_str(), lut))
                            	{
									logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not save BSDF lut");

									return VK_FALSE;
                                }

                                context->addImageData(lut);
                            }

                            if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, lut->getFormat(), lut->getImageType(), 0, lut->getExtent3D(), lut->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, lut->getSize()))
                            {
                                logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                                return VK_FALSE;
                            }

                            initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

                            if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
                            {
                            	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            	srcAccessMask = 0;
                            }

                            //

                            memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

                            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

                            imageCreateInfo.flags = 0;
                            imageCreateInfo.imageType = lut->getImageType();
                            imageCreateInfo.format = lut->getFormat();
                            imageCreateInfo.extent = lut->getExtent3D();
                            imageCreateInfo.mipLevels = lut->getMipLevels();
                            imageCreateInfo.arrayLayers = lut->getArrayLayers();
                            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                            imageCreateInfo.tiling = imageTiling;
                            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                            imageCreateInfo.queueFamilyIndexCount = 0;
                            imageCreateInfo.pQueueFamilyIndices = nullptr;
                            imageCreateInfo.initialLayout = initialLayout;

                            subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, lut->getMipLevels(), 0, lut->getArrayLayers()};

                            memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), lutName, lut, imageCreateInfo, srcAccessMask, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, memoryPropertyFlags);

                            context->addStageImage(stageImage);
                            context->addStageBuffer(stageBuffer);
                            context->addStageDeviceMemory(stageDeviceMemory);

                            if (!memoryImage.get())
                            {
                                return VK_FALSE;
                            }

                            context->addMemoryImage(memoryImage);
                        }
                    }

                    //

                    context->addImageData(imageData);
                }
            }

            //
            // Texture creation.
            //

            if (memoryImageName == "" || !imageData.get())
            {
            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No memory image name or image data for '%s'", finalImageDataFilename.c_str());

                return VK_FALSE;
            }

            //

            VkImageTiling imageTiling;
            VkMemoryPropertyFlags memoryPropertyFlags;

            if (!context->getInitialResources()->getPhysicalDevice()->getGetImageTilingAndMemoryProperty(imageTiling, memoryPropertyFlags, imageData->getFormat(), imageData->getImageType(), 0, imageData->getExtent3D(), imageData->getMipLevels(), 1, VK_SAMPLE_COUNT_1_BIT, imageData->getSize()))
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: Format not supported.");

                return VK_FALSE;
            }


            VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            VkAccessFlags srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

            if (imageTiling == VK_IMAGE_TILING_OPTIMAL)
            {
            	initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            	srcAccessMask = 0;
            }

            //

            VkImageCreateInfo imageCreateInfo;

            memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));

            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

            imageCreateInfo.flags = environment ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
            imageCreateInfo.imageType = imageData->getImageType();
            imageCreateInfo.format = imageData->getFormat();
            imageCreateInfo.extent = imageData->getExtent3D();
            imageCreateInfo.mipLevels = imageData->getMipLevels();
            imageCreateInfo.arrayLayers = imageData->getArrayLayers();
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = imageTiling;
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.queueFamilyIndexCount = 0;
            imageCreateInfo.pQueueFamilyIndices = nullptr;
            imageCreateInfo.initialLayout = initialLayout;

            VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, imageData->getMipLevels(), 0, imageData->getArrayLayers()};

            IDeviceMemorySP stageDeviceMemory;
            IImageSP stageImage;
            IBufferSP stageBuffer;

            auto memoryImage = memoryImageCreate(stageImage, stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), memoryImageName, imageData, imageCreateInfo, srcAccessMask, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, memoryPropertyFlags);

            context->addStageImage(stageImage);
            context->addStageBuffer(stageBuffer);
            context->addStageDeviceMemory(stageDeviceMemory);

            if (!memoryImage.get())
            {
            	logPrint(VKTS_LOG_ERROR, "Scenegraph: No memory image for '%s'", finalImageDataFilename.c_str());

                return VK_FALSE;
            }

            context->addMemoryImage(memoryImage);
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadTextures(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    VkBool32 bdata;

    std::string textureName;
    VkBool32 mipMap = VK_FALSE;
    VkBool32 environment = VK_FALSE;
    VkBool32 preFiltered = VK_FALSE;
    IMemoryImageSP memoryImage;

    ITextureSP texture;

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "image_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadImages(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not load images: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            textureName = sdata;

            mipMap = VK_FALSE;
            environment = VK_FALSE;
            preFiltered = VK_FALSE;
        }
        else if (scenegraphIsToken(buffer, "mipmap"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            mipMap = bdata;
        }
        else if (scenegraphIsToken(buffer, "environment"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            environment = bdata;
        }
        else if (scenegraphIsToken(buffer, "pre_filtered"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            preFiltered = bdata;
        }
        else if (scenegraphIsToken(buffer, "image"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (mipMap && environment)
            {
            	return VK_FALSE;
            }

            if (!preFiltered && environment)
            {
            	return VK_FALSE;
            }

            //

            if (preFiltered && environment)
            {
                // The pre-filtered images are stored in the mip map layers.

            	mipMap = VK_TRUE;
            }

            memoryImage = context->useMemoryImage(sdata);

			if (!memoryImage.get())
			{
				logPrint(VKTS_LOG_ERROR, "Scenegraph: Memory image not found: '%s'", sdata);

				return VK_FALSE;
			}

            // Texture creation.

            if (textureName == "")
            {
                return VK_FALSE;
            }

            auto texture = textureCreate(context->getInitialResources(), textureName, mipMap, environment, memoryImage, context->getSamplerCreateInfo(), context->getImageViewCreateInfo());

            if (!texture.get())
            {
                return VK_FALSE;
            }

            context->addTexture(texture);

            //

            if (preFiltered)
            {
            	auto lambertName = std::string(sdata) + "_LAMBERT";

                memoryImage = context->useMemoryImage(lambertName);

    			if (!memoryImage.get())
    			{
    				logPrint(VKTS_LOG_ERROR, "Scenegraph: Memory image not found: '%s'", lambertName.c_str());

    				return VK_FALSE;
    			}

                texture = textureCreate(context->getInitialResources(), textureName + "_LAMBERT", mipMap, environment, memoryImage, context->getSamplerCreateInfo(), context->getImageViewCreateInfo());

                if (!texture.get())
                {
                    return VK_FALSE;
                }

                context->addTexture(texture);

                //

            	auto cookTorranceName = std::string(sdata) + "_COOKTORRANCE";

                memoryImage = context->useMemoryImage(cookTorranceName);

    			if (!memoryImage.get())
    			{
    				logPrint(VKTS_LOG_ERROR, "Scenegraph: Memory image not found: '%s'", cookTorranceName.c_str());

    				return VK_FALSE;
    			}

                texture = textureCreate(context->getInitialResources(), textureName + "_COOKTORRANCE", mipMap, environment, memoryImage, context->getSamplerCreateInfo(), context->getImageViewCreateInfo());

                if (!texture.get())
                {
                    return VK_FALSE;
                }

                context->addTexture(texture);

                //

            	auto lutName = "BSDF_LUT_" + std::to_string(VKTS_BSDF_LENGTH) + "_" + std::to_string(VKTS_BSDF_M);

                memoryImage = context->useMemoryImage(lutName);

    			if (!memoryImage.get())
    			{
    				logPrint(VKTS_LOG_ERROR, "Scenegraph: Memory image not found: '%s'", lutName.c_str());

    				return VK_FALSE;
    			}

    			//

    			VkSamplerCreateInfo lutSamplerCreateInfo = context->getSamplerCreateInfo();

    			lutSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    			lutSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    			lutSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    			lutSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

                texture = textureCreate(context->getInitialResources(), lutName, VK_FALSE, VK_FALSE, memoryImage, lutSamplerCreateInfo, context->getImageViewCreateInfo());

                if (!texture.get())
                {
                    return VK_FALSE;
                }

                context->addTexture(texture);
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadMaterials(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[3];
    uint32_t uidata;
    VkBool32 bdata;

    auto bsdfMaterial = IBSDFMaterialSP();
    auto phongMaterial = IPhongMaterialSP();

    auto texture = ITextureSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "texture_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadTextures(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not load textures: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "shading"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (strncmp(sdata, "BSDF", 4) == 0)
            {
                bsdfMaterial = IBSDFMaterialSP(new BSDFMaterial());
                phongMaterial = IPhongMaterialSP();
            }
            else if (strncmp(sdata, "Phong", 5) == 0)
            {
                bsdfMaterial = IBSDFMaterialSP();
                phongMaterial = IPhongMaterialSP(new PhongMaterial());

                // Create all possibilities, even when not used.

                VkDescriptorPoolSize descriptorPoolSize[2];

                memset(&descriptorPoolSize, 0, sizeof(descriptorPoolSize));

				descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorPoolSize[0].descriptorCount = 5;

				descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorPoolSize[1].descriptorCount = 9;

                auto descriptorPool = descriptorPoolCreate(context->getInitialResources()->getDevice()->getDevice(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1, 2, descriptorPoolSize);

                if (!descriptorPool.get())
                {
                    return VK_FALSE;
                }

                phongMaterial->setDescriptorPool(descriptorPool);

                //

                auto allDescriptorSetLayouts = context->getDescriptorSetLayout()->getDescriptorSetLayout();

                auto descriptorSets = descriptorSetsCreate(context->getInitialResources()->getDevice()->getDevice(), descriptorPool->getDescriptorPool(), 1, &allDescriptorSetLayouts);

                if (!descriptorSets.get())
                {
                    return VK_FALSE;
                }

                phongMaterial->setDescriptorSets(descriptorSets);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: Unknown shading: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (bsdfMaterial.get())
            {
            	bsdfMaterial->setName(sdata);

                context->addBSDFMaterial(bsdfMaterial);
            }
            else if (phongMaterial.get())
            {
                phongMaterial->setName(sdata);

                context->addPhongMaterial(phongMaterial);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "transparent"))
        {
            if (!scenegraphParseBool(buffer, &bdata))
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setTransparent(bdata);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "emissive_color"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], fdata[1], fdata[2], VK_FORMAT_R8G8B8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setEmissive(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "alpha_value"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], 0.0f, 0.0f, VK_FORMAT_R8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setAlpha(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "displacement_value"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], 0.0f, 0.0f, VK_FORMAT_R8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setDisplacement(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "normal_vector"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0] * 0.5f + 0.5f, fdata[1] * 0.5f + 0.5f, fdata[2] * 0.5f + 0.5f, VK_FORMAT_R8G8B8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setNormal(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "emissive_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setEmissive(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "alpha_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setAlpha(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "displacement_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setDisplacement(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "normal_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
            	phongMaterial->setNormal(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_ambient_color"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], fdata[1], fdata[2], VK_FORMAT_R8G8B8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setAmbient(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_diffuse_color"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], fdata[1], fdata[2], VK_FORMAT_R8G8B8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setDiffuse(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_specular_color"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], fdata[1], fdata[2], VK_FORMAT_R8G8B8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setSpecular(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_specular_shininess_value"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            texture = scenegraphCreateTexture(fdata[0], 0.0f, 0.0f, VK_FORMAT_R8_UNORM, context);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setSpecularShininess(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_ambient_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setAmbient(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_diffuse_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setDiffuse(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_specular_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setSpecular(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "phong_specular_shininess_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (phongMaterial.get())
            {
                phongMaterial->setSpecularShininess(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "fragment_shader"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            auto shaderModule = context->useFragmentShaderModule(sdata);

            if (!shaderModule.get())
            {
                std::string finalFilename = std::string(directory) + std::string(sdata);

                auto shaderBinary = fileLoadBinary(finalFilename.c_str());

				if (!shaderBinary.get())
				{
					shaderBinary = fileLoadBinary(filename);

					if (!shaderBinary.get())
					{
						logPrint(VKTS_LOG_ERROR, "Could not load fragment shader: '%s'", sdata);

						return VK_FALSE;
					}
				}

				//

				shaderModule = shaderModuleCreate(sdata, context->getInitialResources()->getDevice()->getDevice(), 0, shaderBinary->getSize(), (uint32_t*)shaderBinary->getData());

				if (!shaderModule.get())
				{
					logPrint(VKTS_LOG_ERROR, "Could not create fragment shader module.");

					return VK_FALSE;
				}

				context->addFragmentShaderModule(shaderModule);
            }

            if (bsdfMaterial.get())
            {
            	bsdfMaterial->setFragmentShader(shaderModule);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "attributes"))
        {
            if (!scenegraphParseUIntHex(buffer, &uidata))
            {
                return VK_FALSE;
            }

            if (bsdfMaterial.get())
            {
            	bsdfMaterial->setAttributes((VkTsVertexBufferType)uidata);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "add_texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return VK_FALSE;
            }

            if (bsdfMaterial.get())
            {
            	bsdfMaterial->addTexture(texture);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No material");

                return VK_FALSE;
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadSubMeshes(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[8];
    int32_t idata[3];

    auto subMesh = ISubMeshSP();

    std::vector<float> vertex;
    std::vector<float> normal;
    std::vector<float> bitangent;
    std::vector<float> tangent;
    std::vector<float> texcoord;

    std::vector<float> boneIndices0;
    std::vector<float> boneIndices1;
    std::vector<float> boneWeights0;
    std::vector<float> boneWeights1;
    std::vector<float> numberBones;

    std::vector<int32_t> indices;

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "material_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadMaterials(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load materials: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            subMesh = ISubMeshSP(new SubMesh());

            if (!subMesh.get())
            {
                logPrint(VKTS_LOG_ERROR, "Sub mesh not created: '%s'", sdata);

                return VK_FALSE;
            }

            subMesh->setName(sdata);
        }
        else if (scenegraphIsToken(buffer, "vertex"))
        {
            if (!scenegraphParseVec4(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 4; i++)
                {
                    vertex.push_back(fdata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "normal"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 3; i++)
                {
                    normal.push_back(fdata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "bitangent"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 3; i++)
                {
                    bitangent.push_back(fdata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "tangent"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 3; i++)
                {
                    tangent.push_back(fdata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "texcoord"))
        {
            if (!scenegraphParseVec2(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 2; i++)
                {
                    texcoord.push_back(fdata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "boneIndex"))
        {
            if (!scenegraphParseVec8(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 8; i++)
                {
                    if (i < 4)
                    {
                        boneIndices0.push_back(fdata[i]);
                    }
                    else
                    {
                        boneIndices1.push_back(fdata[i]);
                    }
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "boneWeight"))
        {
            if (!scenegraphParseVec8(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 8; i++)
                {
                    if (i < 4)
                    {
                        boneWeights0.push_back(fdata[i]);
                    }
                    else
                    {
                        boneWeights1.push_back(fdata[i]);
                    }
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "numberBones"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                numberBones.push_back(fdata[0]);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "face"))
        {
            if (!scenegraphParseIVec3(buffer, idata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                for (int32_t i = 0; i < 3; i++)
                {
                    indices.push_back(idata[i]);
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "material"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (subMesh.get())
            {
                const auto phongMaterial = context->usePhongMaterial(sdata);

                if (phongMaterial.get())
                {
                    subMesh->setPhongMaterial(phongMaterial);
                }
                else
                {
                    const auto bsdfMaterial = context->useBSDFMaterial(sdata);

                    if (bsdfMaterial.get())
                    {
                        subMesh->setBSDFMaterial(bsdfMaterial);
                    }
                    else
                    {
                        logPrint(VKTS_LOG_ERROR, "Material not found: '%s'", sdata);

                        return VK_FALSE;
                    }
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }

            //
            // Sub mesh creation.
            //

            if (subMesh.get())
            {
                subMesh->setNumberVertices((int32_t)(vertex.size() / 4));
                subMesh->setNumberIndices((int32_t) indices.size());

                int32_t totalSize = 0;
                uint32_t strideInBytes = 0;

                VkTsVertexBufferType vertexBufferType = 0;

                if (vertex.size() > 0)
                {
                    subMesh->setVertexOffset(strideInBytes);
                    strideInBytes += 4 * sizeof(float);

                    totalSize += 4 * sizeof(float) * subMesh->getNumberVertices();

                    vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_VERTEX;
                }

                if (normal.size() > 0)
                {
                    if (normal.size() / 3 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Normal has different size");

                        return VK_FALSE;
                    }

                    subMesh->setNormalOffset(strideInBytes);
                    strideInBytes += 3 * sizeof(float);

                    totalSize += 3 * sizeof(float) * subMesh->getNumberVertices();

                    vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_NORMAL;
                }

                if (normal.size() > 0 && bitangent.size() > 0 && tangent.size() > 0)
                {
                    if (bitangent.size() / 3 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Bitangent has different size");

                        return VK_FALSE;
                    }

                    subMesh->setBitangentOffset(strideInBytes);
                    strideInBytes += 3 * sizeof(float);

                    totalSize += 3 * sizeof(float) * subMesh->getNumberVertices();

                    //

                    if (tangent.size() / 3 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Tangent has different size");

                        return VK_FALSE;
                    }

                    subMesh->setTangentOffset(strideInBytes);
                    strideInBytes += 3 * sizeof(float);

                    totalSize += 3 * sizeof(float) * subMesh->getNumberVertices();

                    //

                    vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_TANGENTS;
                }

                if (texcoord.size() > 0)
                {
                    if (texcoord.size() / 2 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Texture coordinate has different size");

                        return VK_FALSE;
                    }

                    subMesh->setTexcoordOffset(strideInBytes);
                    strideInBytes += 2 * sizeof(float);

                    totalSize += 2 * sizeof(float) * subMesh->getNumberVertices();

                    vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_TEXCOORD;
                }

                if (boneIndices0.size() > 0 && boneIndices1.size() > 0 && boneWeights0.size() > 0 && boneWeights1.size() > 0 && numberBones.size() > 0)
                {
                    if (boneIndices0.size() / 4 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Bone indices0 has different size: %u != %u", boneIndices0.size(), vertex.size());

                        return VK_FALSE;
                    }

                    subMesh->setBoneIndices0Offset(strideInBytes);
                    strideInBytes += 4 * sizeof(float);

                    totalSize += 4 * sizeof(float) * subMesh->getNumberVertices();

                    if (boneIndices1.size() / 4 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Bone indices1 has different size %u != %u", boneIndices1.size(), vertex.size());

                        return VK_FALSE;
                    }

                    subMesh->setBoneIndices1Offset(strideInBytes);
                    strideInBytes += 4 * sizeof(float);

                    totalSize += 4 * sizeof(float) * subMesh->getNumberVertices();

                    if (boneWeights0.size() / 4 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Bone weights0 has different size");

                        return VK_FALSE;
                    }

                    subMesh->setBoneWeights0Offset(strideInBytes);
                    strideInBytes += 4 * sizeof(float);

                    totalSize += 4 * sizeof(float) * subMesh->getNumberVertices();

                    if (boneWeights1.size() / 4 != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Bone weights has different size");

                        return VK_FALSE;
                    }

                    subMesh->setBoneWeights1Offset(strideInBytes);
                    strideInBytes += 4 * sizeof(float);

                    totalSize += 4 * sizeof(float) * subMesh->getNumberVertices();

                    if (numberBones.size() != vertex.size() / 4)
                    {
                        logPrint(VKTS_LOG_ERROR, "Number bones has different size");

                        return VK_FALSE;
                    }

                    subMesh->setNumberBonesOffset(strideInBytes);
                    strideInBytes += 1 * sizeof(float);

                    totalSize += 1 * sizeof(float) * subMesh->getNumberVertices();

                    vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_BONES;
                }

                subMesh->setStrideInBytes(strideInBytes);

                if (totalSize > 0)
                {
                    auto vertexBinaryBuffer = IBinaryBufferSP(new BinaryBuffer((size_t)totalSize));

                    if (!vertexBinaryBuffer.get() || vertexBinaryBuffer->getSize() != (size_t)totalSize)
                    {
                        logPrint(VKTS_LOG_ERROR, "Could not create vertex binary buffer");

                        return VK_FALSE;
                    }

                    for (int32_t currentVertexElement = 0; currentVertexElement < subMesh->getNumberVertices(); currentVertexElement++)
                    {
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_VERTEX)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&vertex[currentVertexElement * 4]), 1, 4 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_NORMAL)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&normal[currentVertexElement * 3]), 1, 3 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BITANGENT)
                        {
                            vertexBinaryBuffer->write( reinterpret_cast<const uint8_t*>(&bitangent[currentVertexElement * 3]), 1, 3 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_TANGENT)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&tangent[currentVertexElement * 3]), 1, 3 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_TEXCOORD)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&texcoord[currentVertexElement * 2]), 1, 2 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONE_INDICES0)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&boneIndices0[currentVertexElement * 4]), 1, 4 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONE_INDICES1)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&boneIndices1[currentVertexElement * 4]), 1, 4 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONE_WEIGHTS0)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&boneWeights0[currentVertexElement * 4]), 1, 4 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONE_WEIGHTS1)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&boneWeights1[currentVertexElement * 4]), 1, 4 * sizeof(float));
                        }
                        if (vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONE_NUMBERS)
                        {
                            vertexBinaryBuffer->write(reinterpret_cast<const uint8_t*>(&numberBones[currentVertexElement * 1]), 1, 1 * sizeof(float));
                        }
                    }

                    //

                    VkBufferCreateInfo bufferCreateInfo;

                    memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

                    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

                    bufferCreateInfo.size = (size_t) totalSize;
                    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                    bufferCreateInfo.flags = 0;
                    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    bufferCreateInfo.queueFamilyIndexCount = 0;
                    bufferCreateInfo.pQueueFamilyIndices = nullptr;

                    IDeviceMemorySP stageDeviceMemory;
                    IBufferSP stageBuffer;

                    auto vertexBuffer = bufferObjectCreate(stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), vertexBinaryBuffer, bufferCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    context->addStageBuffer(stageBuffer);
                    context->addStageDeviceMemory(stageDeviceMemory);

                    if (!vertexBuffer.get())
                    {
                        logPrint(VKTS_LOG_ERROR, "Could not create vertex buffer");

                        return VK_FALSE;
                    }

                    //

                    subMesh->setVertexBuffer(vertexBuffer, vertexBufferType, aabb((const float*)vertexBinaryBuffer->getData(), subMesh->getNumberVertices(), subMesh->getStrideInBytes()));
                }
                else
                {
                    logPrint(VKTS_LOG_ERROR, "Sub mesh incomplete");

                    return VK_FALSE;
                }

                if (indices.size() > 0)
                {
                	size_t size = sizeof(int32_t) * subMesh->getNumberIndices();

                    auto indicesBinaryBuffer = IBinaryBufferSP(new BinaryBuffer(reinterpret_cast<const uint8_t*>(&indices[0]), size));

                    if (!indicesBinaryBuffer.get() || indicesBinaryBuffer->getSize() != size)
                    {
                        logPrint(VKTS_LOG_ERROR, "Could not create indices binary buffer");

                        return VK_FALSE;
                    }

                    //

                    VkBufferCreateInfo bufferCreateInfo;

                    memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

                    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bufferCreateInfo.size = indicesBinaryBuffer->getSize();
                    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                    bufferCreateInfo.flags = 0;

                    IDeviceMemorySP stageDeviceMemory;
                    IBufferSP stageBuffer;

                    auto indicesVertexBuffer = bufferObjectCreate(stageBuffer, stageDeviceMemory, context->getInitialResources(), context->getCommandBuffer(), indicesBinaryBuffer, bufferCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    context->addStageBuffer(stageBuffer);
                    context->addStageDeviceMemory(stageDeviceMemory);

                    if (!indicesVertexBuffer.get())
                    {
                        logPrint(VKTS_LOG_ERROR, "Could not create indices vertex buffer");

                        return VK_FALSE;
                    }

                    //

                    subMesh->setIndicesVertexBuffer(indicesVertexBuffer);
                }
                else
                {
                    logPrint(VKTS_LOG_ERROR, "Sub mesh incomplete");

                    return VK_FALSE;
                }

                //

                if (subMesh->getBSDFMaterial().get())
                {
                	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[VKTS_BINDING_UNIFORM_BSDF_TOTAL_BINDING_COUNT];

                	memset(&descriptorSetLayoutBinding, 0, sizeof(descriptorSetLayoutBinding));

                	uint32_t bindingCount = 0;

                	descriptorSetLayoutBinding[bindingCount].binding = VKTS_BINDING_UNIFORM_BUFFER_VIEWPROJECTION;
                	descriptorSetLayoutBinding[bindingCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                	descriptorSetLayoutBinding[bindingCount].descriptorCount = 1;
                	descriptorSetLayoutBinding[bindingCount].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                	descriptorSetLayoutBinding[bindingCount].pImmutableSamplers = nullptr;

                	bindingCount++;

                	descriptorSetLayoutBinding[bindingCount].binding = VKTS_BINDING_UNIFORM_BUFFER_TRANSFORM;
                	descriptorSetLayoutBinding[bindingCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                	descriptorSetLayoutBinding[bindingCount].descriptorCount = 1;
                	descriptorSetLayoutBinding[bindingCount].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                	descriptorSetLayoutBinding[bindingCount].pImmutableSamplers = nullptr;

                	bindingCount++;

                	if ((subMesh->getVertexBufferType() & VKTS_VERTEX_BUFFER_TYPE_BONES) == VKTS_VERTEX_BUFFER_TYPE_BONES)
                	{
                    	descriptorSetLayoutBinding[bindingCount].binding = VKTS_BINDING_UNIFORM_BUFFER_BONE_TRANSFORM;
                    	descriptorSetLayoutBinding[bindingCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    	descriptorSetLayoutBinding[bindingCount].descriptorCount = 1;
                    	descriptorSetLayoutBinding[bindingCount].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                    	descriptorSetLayoutBinding[bindingCount].pImmutableSamplers = nullptr;

                    	bindingCount++;
                	}

                	if (subMesh->getBSDFMaterial()->getNumberTextures() > VKTS_BINDING_UNIFORM_BSDF_BINDING_COUNT)
                	{
                		vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Too many textures.");

                		return VK_FALSE;
                	}

                    for (size_t i = 0; i < subMesh->getBSDFMaterial()->getNumberTextures(); i++)
                    {
                		descriptorSetLayoutBinding[bindingCount].binding = VKTS_BINDING_UNIFORM_SAMPLER_BSDF_FIRST + (uint32_t)i;
                		descriptorSetLayoutBinding[bindingCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                		descriptorSetLayoutBinding[bindingCount].descriptorCount = 1;
                		descriptorSetLayoutBinding[bindingCount].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                		descriptorSetLayoutBinding[bindingCount].pImmutableSamplers = nullptr;

                		bindingCount++;
                    }

                    //

                    auto descriptorSetLayout = descriptorSetLayoutCreate(context->getInitialResources()->getDevice()->getDevice(), 0, bindingCount, descriptorSetLayoutBinding);

                	if (!descriptorSetLayout.get())
                	{
                		vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create descriptor set layout.");

                		return VK_FALSE;
                	}

                	subMesh->setDescriptorSetLayout(descriptorSetLayout);

                    // Create all possibilities, even when not used.

                    VkDescriptorPoolSize descriptorPoolSize[2];

                    memset(&descriptorPoolSize, 0, sizeof(descriptorPoolSize));

    				descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    				descriptorPoolSize[0].descriptorCount = 5;

    				descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    				descriptorPoolSize[1].descriptorCount = 18;

                    auto descriptorPool = descriptorPoolCreate(context->getInitialResources()->getDevice()->getDevice(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1, 2, descriptorPoolSize);

                    if (!descriptorPool.get())
                    {
                		vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create descriptor pool.");

                        return VK_FALSE;
                    }

            		subMesh->getBSDFMaterial()->setDescriptorPool(descriptorPool);

                	//

                	const auto allDescriptorSetLayouts = descriptorSetLayout->getDescriptorSetLayout();

                    auto descriptorSets = descriptorSetsCreate(context->getInitialResources()->getDevice()->getDevice(), subMesh->getBSDFMaterial()->getDescriptorPool()->getDescriptorPool(), 1, &allDescriptorSetLayouts);

                    if (!descriptorSets.get())
                    {
                    	vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create descriptor sets.");

                        return VK_FALSE;
                    }

                    subMesh->getBSDFMaterial()->setDescriptorSets(descriptorSets);

                    //

                	VkDescriptorSetLayout setLayouts[1];

                	setLayouts[0] = descriptorSetLayout->getDescriptorSetLayout();

                	auto pipelineLayout = pipelineCreateLayout(context->getInitialResources()->getDevice()->getDevice(), 0, 1, setLayouts, 0, nullptr);

                	if (!pipelineLayout.get())
                	{
                		vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create pipeline layout.");

                		return VK_FALSE;
                	}

                	subMesh->setPipelineLayout(pipelineLayout);

					// Create graphics pipeline for this sub mesh.

                	auto currentRenderPass = context->getRenderPass();

					if (!currentRenderPass.get())
					{
	                    logPrint(VKTS_LOG_ERROR, "Scenegraph: No renderpass found");

	                    return VK_FALSE;
					}

					auto currentPipelineLayout = subMesh->getPipelineLayout();

					if (!currentPipelineLayout.get())
					{
	                    logPrint(VKTS_LOG_ERROR, "Scenegraph: No pipeline layout found");

	                    return VK_FALSE;
					}


					vertexBufferType = subMesh->getVertexBufferType() & subMesh->getBSDFMaterial()->getAttributes();

					if (vertexBufferType != subMesh->getBSDFMaterial()->getAttributes())
					{
	                    logPrint(VKTS_LOG_ERROR, "Scenegraph: Sub mesh vertex buffer type does not match with material");

	                    return VK_FALSE;
					}

					if ((subMesh->getVertexBufferType() & VKTS_VERTEX_BUFFER_TYPE_BONES) == VKTS_VERTEX_BUFFER_TYPE_BONES)
					{
						vertexBufferType |= VKTS_VERTEX_BUFFER_TYPE_BONES;
					}

					auto currentVertexShaderModule = context->useVertexShaderModule(vertexBufferType);

					if (!currentVertexShaderModule.get())
					{
	                    logPrint(VKTS_LOG_ERROR, "Scenegraph: No shader module found");

						return VK_FALSE;
					}

					defaultGraphicsPipeline gp;

					gp.getPipelineShaderStageCreateInfo(0).stage = VK_SHADER_STAGE_VERTEX_BIT;
					gp.getPipelineShaderStageCreateInfo(0).module = currentVertexShaderModule->getShaderModule();

					gp.getPipelineShaderStageCreateInfo(1).stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					gp.getPipelineShaderStageCreateInfo(1).module = subMesh->getBSDFMaterial()->getFragmentShader()->getShaderModule();


					gp.getVertexInputBindingDescription(0).binding = 0;
					gp.getVertexInputBindingDescription(0).stride = commonGetStrideInBytes(subMesh->getVertexBufferType());
					gp.getVertexInputBindingDescription(0).inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


					uint32_t location = 0;

					gp.getVertexInputAttributeDescription(location).location = location;
					gp.getVertexInputAttributeDescription(location).binding = 0;
					gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32A32_SFLOAT;
					gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_VERTEX, subMesh->getVertexBufferType());

					if ((vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_NORMAL) == VKTS_VERTEX_BUFFER_TYPE_NORMAL)
					{
						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_NORMAL, subMesh->getVertexBufferType());

						if ((vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_TANGENTS) == VKTS_VERTEX_BUFFER_TYPE_TANGENTS)
						{
							location++;

							gp.getVertexInputAttributeDescription(location).location = location;
							gp.getVertexInputAttributeDescription(location).binding = 0;
							gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32_SFLOAT;
							gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BITANGENT, subMesh->getVertexBufferType());

							location++;

							gp.getVertexInputAttributeDescription(location).location = location;
							gp.getVertexInputAttributeDescription(location).binding = 0;
							gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32_SFLOAT;
							gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_TANGENT, subMesh->getVertexBufferType());
						}
					}

					if ((vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_TEXCOORD) == VKTS_VERTEX_BUFFER_TYPE_TEXCOORD)
					{
						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_TEXCOORD, subMesh->getVertexBufferType());
					}


					if ((vertexBufferType & VKTS_VERTEX_BUFFER_TYPE_BONES) == VKTS_VERTEX_BUFFER_TYPE_BONES)
					{
						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32A32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BONE_INDICES0, subMesh->getVertexBufferType());

						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32A32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BONE_INDICES1, subMesh->getVertexBufferType());

						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32A32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BONE_WEIGHTS0, subMesh->getVertexBufferType());

						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32G32B32A32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BONE_WEIGHTS1, subMesh->getVertexBufferType());

						location++;

						gp.getVertexInputAttributeDescription(location).location = location;
						gp.getVertexInputAttributeDescription(location).binding = 0;
						gp.getVertexInputAttributeDescription(location).format = VK_FORMAT_R32_SFLOAT;
						gp.getVertexInputAttributeDescription(location).offset = commonGetOffsetInBytes(VKTS_VERTEX_BUFFER_TYPE_BONE_NUMBERS, subMesh->getVertexBufferType());
					}

					//

					gp.getPipelineInputAssemblyStateCreateInfo().topology = subMesh->getPrimitiveTopology();

					gp.getViewports(0).x = 0.0f;
					gp.getViewports(0).y = 0.0f;
					gp.getViewports(0).width = 1.0f;
					gp.getViewports(0).height = 1.0f;
					gp.getViewports(0).minDepth = 0.0f;
					gp.getViewports(0).maxDepth = 1.0f;


					gp.getScissors(0).offset.x = 0;
					gp.getScissors(0).offset.y = 0;
					gp.getScissors(0).extent = {1, 1};


					gp.getPipelineMultisampleStateCreateInfo();

					gp.getPipelineDepthStencilStateCreateInfo().depthTestEnable = VK_TRUE;
					gp.getPipelineDepthStencilStateCreateInfo().depthWriteEnable = VK_TRUE;
					gp.getPipelineDepthStencilStateCreateInfo().depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

					for (uint32_t i = 0; i < 5; i++)
					{
						gp.getPipelineColorBlendAttachmentState(i).blendEnable = VK_FALSE;
						gp.getPipelineColorBlendAttachmentState(i).colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
					}

					gp.getDynamicState(0) = VK_DYNAMIC_STATE_VIEWPORT;
					gp.getDynamicState(1) = VK_DYNAMIC_STATE_SCISSOR;


					gp.getGraphicsPipelineCreateInfo().layout = currentPipelineLayout->getPipelineLayout();
					gp.getGraphicsPipelineCreateInfo().renderPass = currentRenderPass->getRenderPass();


					auto pipeline = pipelineCreateGraphics(context->getInitialResources()->getDevice()->getDevice(), VK_NULL_HANDLE, gp.getGraphicsPipelineCreateInfo(), vertexBufferType);

					if (!pipeline.get())
					{
						vkts::logPrint(VKTS_LOG_ERROR, "Scenegraph: Could not create graphics pipeline.");

						return VK_FALSE;
					}

					subMesh->setGraphicsPipeline(pipeline);
                }

                //

                context->addSubMesh(subMesh);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No sub mesh");

                return VK_FALSE;
            }

            vertex.clear();
            normal.clear();
            bitangent.clear();
            tangent.clear();
            texcoord.clear();

            boneIndices0.clear();
            boneIndices1.clear();
            boneWeights0.clear();
            boneWeights1.clear();
            numberBones.clear();

            indices.clear();
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadMeshes(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[6];

    auto mesh = IMeshSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "submesh_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadSubMeshes(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load sub meshes: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            mesh = IMeshSP(new Mesh());

            if (!mesh.get())
            {
                logPrint(VKTS_LOG_ERROR, "Mesh not created: '%s'", sdata);

                return VK_FALSE;
            }

            mesh->setName(sdata);

            context->addMesh(mesh);
        }
        else if (scenegraphIsToken(buffer, "submesh"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (mesh.get())
            {
                const auto& subMesh = context->useSubMesh(sdata);

                if (!subMesh.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Sub mesh not found: '%s'", sdata);

                    return VK_FALSE;
                }

                mesh->addSubMesh(subMesh);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "displace"))
        {
            if (!scenegraphParseVec2(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (mesh.get())
            {
                mesh->setDisplace(glm::vec2(fdata[0], fdata[1]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No mesh");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "aabb"))
        {
            if (!scenegraphParseVec6(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (mesh.get())
            {
                mesh->setAABB(aabb(glm::vec3(fdata[0], fdata[1], fdata[2]), glm::vec3(fdata[3], fdata[4], fdata[5])));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No mesh");

                return VK_FALSE;
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadChannels(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[6];

    auto channel = IChannelSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            channel = IChannelSP(new Channel());

            if (!channel.get())
            {
                logPrint(VKTS_LOG_ERROR, "Channel not created: '%s'", sdata);

                return VK_FALSE;
            }

            channel->setName(sdata);

            context->addChannel(channel);
        }
        else if (scenegraphIsToken(buffer, "target_transform"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (channel.get())
            {
                if (strncmp(sdata, "TRANSLATE", 9) == 0)
                {
                    channel->setTargetTransform(VKTS_TARGET_TRANSFORM_TRANSLATE);
                }
                else if (strncmp(sdata, "ROTATE", 6) == 0)
                {
                    channel->setTargetTransform(VKTS_TARGET_TRANSFORM_ROTATE);
                }
                else if (strncmp(sdata, "QUATERNION_ROTATE", 17) == 0)
                {
                    channel->setTargetTransform(VKTS_TARGET_TRANSFORM_QUATERNION_ROTATE);
                }
                else if (strncmp(sdata, "SCALE", 5) == 0)
                {
                    channel->setTargetTransform(VKTS_TARGET_TRANSFORM_SCALE);
                }
                else
                {
                    logPrint(VKTS_LOG_ERROR, "Unknown target transform: '%s'", sdata);

                    return VK_FALSE;
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No channel");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "target_element"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (channel.get())
            {
                if (strncmp(sdata, "X", 1) == 0)
                {
                    channel->setTargetTransformElement(VKTS_TARGET_TRANSFORM_ELEMENT_X);
                }
                else if (strncmp(sdata, "Y", 1) == 0)
                {
                    channel->setTargetTransformElement(VKTS_TARGET_TRANSFORM_ELEMENT_Y);
                }
                else if (strncmp(sdata, "Z", 1) == 0)
                {
                    channel->setTargetTransformElement(VKTS_TARGET_TRANSFORM_ELEMENT_Z);
                }
                else if (strncmp(sdata, "W", 1) == 0)
                {
                    channel->setTargetTransformElement(VKTS_TARGET_TRANSFORM_ELEMENT_W);
                }
                else
                {
                    logPrint(VKTS_LOG_ERROR, "Unknown target transform element: '%s'", sdata);

                    return VK_FALSE;
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No channel");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "keyframe"))
        {
            char token[VKTS_MAX_TOKEN_CHARS + 1];

            auto numberItems = sscanf(buffer, "%s %f %f %s %f %f %f %f", token, &fdata[0], &fdata[1], sdata, &fdata[2], &fdata[3], &fdata[4], &fdata[5]);

            if (numberItems != 4 && numberItems != 8)
            {
                return VK_FALSE;
            }

            if (numberItems == 4)
            {
                fdata[2] = fdata[0] - 0.1f;
                fdata[3] = fdata[1];
                fdata[4] = fdata[0] + 0.1f;
                fdata[5] = fdata[1];
            }

            if (channel.get())
            {
                VkTsInterpolator interpolator = VKTS_INTERPOLATOR_CONSTANT;

                if (strncmp(sdata, "CONSTANT", 8) == 0)
                {
                    // Do nothing.
                }
                else if (strncmp(sdata, "LINEAR", 6) == 0)
                {
                    interpolator = VKTS_INTERPOLATOR_LINEAR;
                }
                else if (strncmp(sdata, "BEZIER", 6) == 0)
                {
                    interpolator = VKTS_INTERPOLATOR_BEZIER;
                }
                else
                {
                    logPrint(VKTS_LOG_ERROR, "Unknown interpolator: '%s'", sdata);

                    return VK_FALSE;
                }

                channel->addEntry(fdata[0], fdata[1], glm::vec4(fdata[2], fdata[3], fdata[4], fdata[5]), interpolator);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No channel");

                return VK_FALSE;
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadAnimations(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[1];

    auto animation = IAnimationSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "channel_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadChannels(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load channels: '%s'", sdata);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            animation = IAnimationSP(new Animation());

            if (!animation.get())
            {
                logPrint(VKTS_LOG_ERROR, "Animation not created: '%s'", sdata);

                return VK_FALSE;
            }

            animation->setName(sdata);

            context->addAnimation(animation);
        }
        else if (scenegraphIsToken(buffer, "start"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (animation.get())
            {
                animation->setStart(fdata[0]);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No animation");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "stop"))
        {
            if (!scenegraphParseFloat(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (animation.get())
            {
                animation->setStop(fdata[0]);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No animation");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "channel"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return VK_FALSE;
            }

            if (animation.get())
            {
                const auto& channel = context->useChannel(sdata);

                if (!channel.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Channel not found: '%s'", sdata);

                    return VK_FALSE;
                }

                animation->addChannel(channel);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No animation");

                return VK_FALSE;
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadParticleSystems(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];

    auto animation = IAnimationSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        // TODO: Load particle systems and its data.
    }

    return VK_TRUE;
}

static VkBool32 scenegraphLoadObjects(const char* directory, const char* filename, const IContextSP& context)
{
    if (!directory || !filename || !context.get())
    {
        return VK_FALSE;
    }

    std::string finalFilename = std::string(directory) + std::string(filename);

    auto textBuffer = fileLoadText(finalFilename.c_str());

    if (!textBuffer.get())
    {
        textBuffer = fileLoadText(filename);

        if (!textBuffer.get())
        {
            return VK_FALSE;
        }
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata0[VKTS_MAX_TOKEN_CHARS + 1];
    char sdata1[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[3];
    int32_t idata;
    uint32_t uidata;
    glm::mat4 mat4;

    auto object = IObjectSP();

    auto node = INodeSP();

    SmartPointerMap<std::string, INodeSP> allNodes;

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "mesh_library"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadMeshes(directory, sdata0, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load meshes: '%s'", sdata0);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "animation_library"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadAnimations(directory, sdata0, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load animations: '%s'", sdata0);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "particle_system_library"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (!scenegraphLoadParticleSystems(directory, sdata0, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load particles: '%s'", sdata0);

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            object = IObjectSP(new Object());

            if (!object.get())
            {
                logPrint(VKTS_LOG_ERROR, "Object not created: '%s'", sdata0);

                return VK_FALSE;
            }

            object->setName(sdata0);

            context->addObject(object);
        }
        else if (scenegraphIsToken(buffer, "node"))
        {
            if (!scenegraphParseStringTuple(buffer, sdata0, sdata1))
            {
                return VK_FALSE;
            }

            if (object.get())
            {
                node = INodeSP(new Node());

                if (!node.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Node not created: '%s'", sdata0);

                    return VK_FALSE;
                }

                node->setName(sdata0);

                //

                allNodes[sdata0] = node;

                if (strncmp(sdata1, "-", 1) == 0)
                {
                    object->setRootNode(node);
                }
                else
                {
                    auto parentNodeIndex = allNodes.find(sdata1);

                    if (parentNodeIndex != allNodes.size() && allNodes.valueAt(parentNodeIndex).get())
                    {
                        node->setParentNode(allNodes.valueAt(parentNodeIndex));

                        allNodes.valueAt(parentNodeIndex)->addChildNode(node);
                    }
                    else
                    {
                        logPrint(VKTS_LOG_ERROR, "Parent node not found: '%s'", sdata1);

                        return VK_FALSE;
                    }
                }
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No object");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "layers"))
        {
            if (!scenegraphParseUIntHex(buffer, &uidata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setLayers(uidata);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "particle_system"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                // TODO: Append particle system.
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "seed"))
        {
            if (!scenegraphParseInt(buffer, &idata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                // TODO: Add seed.
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "translate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setTranslate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "rotate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setRotate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "scale"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setScale(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "jointIndex"))
        {
            if (!scenegraphParseInt(buffer, &idata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setJointIndex(idata);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "joints"))
        {
            if (!scenegraphParseInt(buffer, &idata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                VkBufferCreateInfo bufferCreateInfo;

                memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

                bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                // mat3 in std140 consumes three vec4 columns.
                bufferCreateInfo.size = commonGetDeviceSize(16 * sizeof(float) * (VKTS_MAX_JOINTS + 1) + 12 * sizeof(float) * (VKTS_MAX_JOINTS + 1), 16);
                bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                bufferCreateInfo.flags = 0;

                auto jointsUniformBuffer = bufferObjectCreate(context->getInitialResources(), bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

                if (!jointsUniformBuffer.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Could not create joint uniform buffer for node: '%s'", sdata0);

                    return VK_FALSE;
                }

                node->setJointsUniformBuffer(idata, jointsUniformBuffer);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "bind_translate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setBindTranslate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "bind_rotate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setBindRotate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "bind_scale"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                node->setBindScale(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "mesh"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                VkBufferCreateInfo bufferCreateInfo;

                memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

                bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                // mat3 in std140 consumes three vec4 columns.
                bufferCreateInfo.size = commonGetDeviceSize(16 * sizeof(float) + 12 * sizeof(float), 16);
                bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                bufferCreateInfo.flags = 0;

                auto transformUniformBuffer = bufferObjectCreate(context->getInitialResources(), bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

                if (!transformUniformBuffer.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Could not create uniform buffer for node: '%s'", sdata0);

                    return VK_FALSE;
                }

                //

                node->setTransformUniformBuffer(transformUniformBuffer);

            	//

                const auto& mesh = context->useMesh(sdata0);

                if (!mesh.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Mesh not found: '%s'", sdata0);

                    return VK_FALSE;
                }

                node->addMesh(mesh);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else if (scenegraphIsToken(buffer, "animation"))
        {
            if (!scenegraphParseString(buffer, sdata0))
            {
                return VK_FALSE;
            }

            if (node.get())
            {
                const auto& animation = context->useAnimation(sdata0);

                if (!animation.get())
                {
                    logPrint(VKTS_LOG_ERROR, "Animation not found: '%s'", sdata0);

                    return VK_FALSE;
                }

                node->addAnimation(animation);
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No node");

                return VK_FALSE;
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return VK_TRUE;
}

ISceneSP VKTS_APIENTRY scenegraphLoadScene(const char* filename, const IContextSP& context)
{
    if (!filename || !context.get())
    {
        return ISceneSP();
    }

    auto textBuffer = fileLoadText(filename);

    if (!textBuffer.get())
    {
        return ISceneSP();
    }

    char directory[VKTS_MAX_BUFFER_CHARS] = "";

    scengraphGetDirectory(directory, filename);

    auto scene = ISceneSP(new Scene());

    if (!scene.get())
    {
        return ISceneSP();
    }

    char buffer[VKTS_MAX_BUFFER_CHARS + 1];
    char sdata[VKTS_MAX_TOKEN_CHARS + 1];
    float fdata[3];

    auto object = IObjectSP();

    while (textBuffer->gets(buffer, VKTS_MAX_BUFFER_CHARS))
    {
        if (scenegraphSkipBuffer(buffer))
        {
            continue;
        }

        if (scenegraphIsToken(buffer, "scene_name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            scene->setName(std::string(sdata));
        }
        else if (scenegraphIsToken(buffer, "object_library"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            if (!scenegraphLoadObjects(directory, sdata, context))
            {
                logPrint(VKTS_LOG_ERROR, "Could not load objects: '%s'", sdata);

                return ISceneSP();
            }
        }
        else if (scenegraphIsToken(buffer, "object"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            object = context->useObject(sdata);

            if (!object.get())
            {
                logPrint(VKTS_LOG_ERROR, "Object not found: '%s'", sdata);

                return ISceneSP();
            }

            scene->addObject(object);
        }
        else if (scenegraphIsToken(buffer, "name"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            if (object.get())
            {
                object->setName(std::string(sdata));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No object");

                return ISceneSP();
            }
        }
        else if (scenegraphIsToken(buffer, "translate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return ISceneSP();
            }

            if (object.get())
            {
                object->setTranslate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No object");

                return ISceneSP();
            }
        }
        else if (scenegraphIsToken(buffer, "rotate"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return ISceneSP();
            }

            if (object.get())
            {
                object->setRotate(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No object");

                return ISceneSP();
            }
        }
        else if (scenegraphIsToken(buffer, "scale"))
        {
            if (!scenegraphParseVec3(buffer, fdata))
            {
                return ISceneSP();
            }

            if (object.get())
            {
                object->setScale(glm::vec3(fdata[0], fdata[1], fdata[2]));
            }
            else
            {
                logPrint(VKTS_LOG_ERROR, "Scenegraph: No object");

                return ISceneSP();
            }
        }
        else if (scenegraphIsToken(buffer, "environment"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            // Nothing for now.
        }
        else if (scenegraphIsToken(buffer, "texture"))
        {
            if (!scenegraphParseString(buffer, sdata))
            {
                return ISceneSP();
            }

            auto texture = context->useTexture(sdata);

            if (!texture.get())
            {
                return ISceneSP();
            }

            if (texture->getImageView()->getViewType() != VK_IMAGE_VIEW_TYPE_CUBE || texture->getImageView()->getLayerCount() != 6)
            {
            	return ISceneSP();
            }

            scene->setEnvironment(texture);

            //

            texture = context->useTexture(std::string(sdata) + "_LAMBERT");

            if (texture.get())
            {
                scene->setDiffuseEnvironment(texture);

                //

                texture = context->useTexture(std::string(sdata) + "_COOKTORRANCE");

                if (!texture.get())
                {
                	return ISceneSP();
                }

                scene->setSpecularEnvironment(texture);

                //

                texture = context->useTexture("BSDF_LUT_" + std::to_string(VKTS_BSDF_LENGTH) + "_" + std::to_string(VKTS_BSDF_M));

                if (!texture.get())
                {
                	return ISceneSP();
                }

                scene->setLut(texture);
            }
        }
        else
        {
            scenegraphUnknownBuffer(buffer);
        }
    }

    return scene;
}

IContextSP VKTS_APIENTRY scenegraphCreateContext(const VkBool32 replace, const IInitialResourcesSP& initialResources, const ICommandBuffersSP& cmdBuffer, const VkSamplerCreateInfo& samplerCreateInfo, const VkImageViewCreateInfo& imageViewCreateInfo, const IDescriptorSetLayoutSP& descriptorSetLayout)
{
    if (!initialResources.get())
    {
        return IContextSP();
    }

    return IContextSP(new Context(replace, initialResources, cmdBuffer, samplerCreateInfo, imageViewCreateInfo, descriptorSetLayout));
}

}
