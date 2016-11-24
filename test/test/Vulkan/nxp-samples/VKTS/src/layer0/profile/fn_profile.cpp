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

#include "fn_profile_internal.hpp"

namespace vkts
{

static double g_totalTime;
static uint32_t g_fps;

VkBool32 VKTS_APIENTRY profileInit()
{
	g_totalTime = 0.0f;
	g_fps = 0;

    return _profileInit();
}

VkBool32 VKTS_APIENTRY profileGetCpuUsage(float& usage, const uint32_t cpu)
{
	return _profileGetCpuUsage(usage, cpu);
}

VkBool32 VKTS_APIENTRY profileGetCpuUsage(float& usage)
{
	usage = 0.0f;

	float currentUsage = 0.0f;

	for (uint32_t cpu = 0; cpu < processorGetNumber(); cpu++)
	{
		if (!_profileGetCpuUsage(currentUsage, cpu))
		{
			return VK_FALSE;
		}

		usage += currentUsage;
	}

	return VK_TRUE;
}

VkBool32 VKTS_APIENTRY profileApplicationGetCpuUsage(float& usage)
{
	return _profileApplicationGetCpuUsage(usage);
}

VkBool32 VKTS_APIENTRY profileApplicationGetRam(uint64_t& ram)
{
	return _profileApplicationGetRam(ram);
}

VkBool32 VKTS_APIENTRY profileApplicationGetFps(uint32_t& fps, const double deltaTime)
{
	g_totalTime += deltaTime;

	g_fps++;

	if (g_totalTime >= 1.0f)
	{
		fps = g_fps;

		g_totalTime = g_totalTime - 1.0;
		g_fps = 0;

		return VK_TRUE;
	}

	return VK_FALSE;
}

void VKTS_APIENTRY profileTerminate()
{
    _profileTerminate();
}

}
