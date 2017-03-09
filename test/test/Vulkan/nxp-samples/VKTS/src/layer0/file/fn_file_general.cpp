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

#include "fn_file_internal.hpp"

namespace vkts
{

VkBool32 VKTS_APIENTRY _fileInit()
{
	std::string testFilename = "vk_layer_settings.txt";

	auto testFile = fopen(testFilename.c_str(), "rb");

	if (testFile)
	{
		fclose(testFile);

		return VK_TRUE;
	}

	std::string directory = "../VKTS_Binaries/";

	for (uint32_t i = 0; i < 3; i++)
	{
		testFile = fopen((directory + testFilename).c_str(), "rb");

		if (testFile)
		{
			_fileSetBaseDirectory(directory.c_str());

			fclose(testFile);

			return VK_TRUE;
		}

		directory = "../" + directory;
	}

	logPrint(VKTS_LOG_WARNING, "File: Could not find base directory");

	return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _filePrepareLoadBinary(const char* filename)
{
	// Do nothing.

	return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _filePrepareSaveBinary(const char* filename)
{
	// Do nothing.

	return VK_TRUE;
}

}
