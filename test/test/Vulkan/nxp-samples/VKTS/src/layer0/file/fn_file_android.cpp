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

#include "../visual/fn_visual_android_internal.hpp"

#include "fn_file_internal.hpp"

#include <sys/stat.h>

namespace vkts
{

VkBool32 VKTS_APIENTRY _fileInit()
{
	auto androidApp = _visualGetAndroidApp();

	if (!androidApp)
	{
		vkts::logPrint(VKTS_LOG_ERROR, "File: No android application.");

		return VK_FALSE;
	}

    _fileSetBaseDirectory(androidApp->activity->externalDataPath);

	return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _filePrepareLoadBinary(const char* filename)
{
	std::string targetDirectory = std::string(_fileGetBaseDirectory());

	std::string targetFile = targetDirectory + std::string(filename);

	//

	FILE* testAsset = fopen(targetFile.c_str(), "rb");

	if (testAsset)
	{
		fclose(testAsset);

		return VK_TRUE;
	}

	//

	auto androidApp = _visualGetAndroidApp();

	if (!androidApp)
	{
		vkts::logPrint(VKTS_LOG_ERROR, "File: No android application.");

		return VK_FALSE;
	}

	AAssetManager* assetManager = androidApp->activity->assetManager;

    if (!assetManager)
    {
		vkts::logPrint(VKTS_LOG_ERROR, "File: No asset manager.");

		return VK_FALSE;
    }

    AAsset* sourceAsset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);

    if (!sourceAsset)
    {
		// No output by purpose.
		
		return VK_FALSE;
    }

    const void* sourceData = AAsset_getBuffer(sourceAsset);

	const off_t sourceLength = AAsset_getLength(sourceAsset);

	//


	std::string foldersToCreate = std::string(filename);

	auto lastSlash = foldersToCreate.rfind('/');

	if (lastSlash != foldersToCreate.npos)
	{
		foldersToCreate = foldersToCreate.substr(0, lastSlash);
	}

	if (!_fileCreateDirectory(foldersToCreate.c_str()))
	{
		return VK_FALSE;
	}

	//

	FILE* targetAsset = fopen(targetFile.c_str(), "wb");

	if (!targetAsset)
	{
		vkts::logPrint(VKTS_LOG_ERROR, "File: No target asset.");

		AAsset_close(sourceAsset);

		return VK_FALSE;
	}

	auto targetLength = fwrite(sourceData, sizeof(char), (size_t)sourceLength, targetAsset);

	if (sourceLength != targetLength)
	{
		vkts::logPrint(VKTS_LOG_ERROR, "File: Could not copy '%s'.", filename);

		fclose(targetAsset);

		AAsset_close(sourceAsset);

		return VK_FALSE;
	}

	fclose(targetAsset);

	//

	AAsset_close(sourceAsset);

	return VK_TRUE;
}


VkBool32 VKTS_APIENTRY _filePrepareSaveBinary(const char* filename)
{
	auto androidApp = _visualGetAndroidApp();

	if (!androidApp)
	{
		vkts::logPrint(VKTS_LOG_ERROR, "File: No android application.");

		return VK_FALSE;
	}

	if (!filename)
	{
		return VK_FALSE;
	}

	std::string foldersToCreate = std::string(filename);

	auto lastSlash = foldersToCreate.rfind('/');

	if (lastSlash != foldersToCreate.npos)
	{
		foldersToCreate = foldersToCreate.substr(0, lastSlash);
	}

	return _fileCreateDirectory(foldersToCreate.c_str());
}

VkBool32 VKTS_APIENTRY _fileCreateDirectory(const char* directory)
{
	if (!directory)
	{
		return VK_FALSE;
	}

	//

	struct stat sb;

	if (stat(directory, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return VK_TRUE;
	}

	//

	std::string targetDirectory = std::string(_fileGetBaseDirectory());

	//

	std::string foldersToCreate = std::string(directory);

	while (foldersToCreate.length())
	{
		auto lastSlash = foldersToCreate.find('/');

		if (lastSlash != foldersToCreate.npos)
		{
			targetDirectory += foldersToCreate.substr(0, lastSlash) + "/";

			foldersToCreate = foldersToCreate.substr(lastSlash + 1);
		}
		else
		{
			targetDirectory += foldersToCreate + "/";

			foldersToCreate = "";
		}
		
		if (stat(targetDirectory.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
		{
			continue;
		}

		mkdir(targetDirectory.c_str(), S_IRWXU | S_IRWXG | (S_IROTH | S_IXOTH));
	}

	return VK_TRUE;
}

}
