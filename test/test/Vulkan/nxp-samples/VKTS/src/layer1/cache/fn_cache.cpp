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

#define VKTS_CACHE_DIRECTORY "cache"

namespace vkts
{

static VkBool32 g_cacheEnabled = VKTS_CACHE_ENABLED;


static std::string VKTS_APIENTRY cacheGetDirectory(const char* filename)
{
	if (!filename)
	{
		return std::string(VKTS_CACHE_DIRECTORY);
	}

	auto search = std::string(filename);

	auto lastSlash = search.rfind('/');

	if (lastSlash == search.npos)
	{
		return std::string(VKTS_CACHE_DIRECTORY);
	}

	return std::string(VKTS_CACHE_DIRECTORY) + "/" + search.substr(0, lastSlash);
}

static std::string cacheGetFilename(const char* filename)
{
	if (!filename)
	{
		return std::string(VKTS_CACHE_DIRECTORY);
	}

	return std::string(VKTS_CACHE_DIRECTORY) + "/" + std::string(filename);
}

VkBool32 VKTS_APIENTRY cacheGetEnabled()
{
	return g_cacheEnabled;
}

void VKTS_APIENTRY cacheSetEnabled(const VkBool32 enabled)
{
	g_cacheEnabled = enabled;
}

VkBool32 VKTS_APIENTRY cacheSaveImageData(const IImageDataSP& imageData)
{
	if (!imageData.get())
	{
		return VK_FALSE;
	}

	//

	if (!fileCreateDirectory(cacheGetDirectory(imageData->getName().c_str()).c_str()))
	{
		return VK_FALSE;
	}

	//

	std::string cacheFilename = cacheGetFilename(imageData->getName().c_str());

	if (!imageDataSave(cacheFilename.c_str(), imageData))
	{
		return VK_FALSE;
	}

	return VK_TRUE;
}

IImageDataSP VKTS_APIENTRY cacheLoadImageData(const char* filename)
{
	if (!filename)
	{
		return IImageDataSP();
	}

	std::string cacheFilename = cacheGetFilename(filename);

	return imageDataLoad(cacheFilename.c_str());
}

IImageDataSP VKTS_APIENTRY cacheLoadRawImageData(const char* filename, const uint32_t width, const uint32_t height, const VkFormat format)
{
	if (!filename || width == 0 || height == 0)
	{
		return IImageDataSP();
	}

	std::string cacheFilename = cacheGetFilename(filename);

	return imageDataLoadRaw(cacheFilename.c_str(), width, height, format);
}

}
