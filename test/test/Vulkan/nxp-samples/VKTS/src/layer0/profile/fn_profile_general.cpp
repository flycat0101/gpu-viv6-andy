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

#include <sys/times.h>

namespace vkts
{

typedef struct _CpuData {
	uint64_t user;
	uint64_t nice;
	uint64_t system;
	uint64_t idle;
} CpuData;

static CpuData g_lastCpuData[VKTS_MAX_QUERY_CPU];
static CpuData g_currentCpuData[VKTS_MAX_QUERY_CPU];

static clock_t g_lastTime;

static clock_t g_lastKernelTime;
static clock_t g_lastUserTime;

static double g_lastCheck;

static VkBool32 profileGetCpuData()
{
	FILE* file = fopen("/proc/stat", "r");

	if (!file)
	{
		return VK_FALSE;
	}

	int result;

	CpuData currentCpuData;

	for (uint32_t cpu = 0; cpu <= glm::min(processorGetNumber(), VKTS_MAX_QUERY_CPU); cpu++)
	{
		result = fscanf(file, "%*s %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %*u %*u %*u %*u %*u %*u", &currentCpuData.user, &currentCpuData.nice, &currentCpuData.system, &currentCpuData.idle);

		if (cpu == 0)
		{
			continue;
		}

		if (result == 4)
		{
			g_currentCpuData[cpu - 1] = currentCpuData;
		}
	}

	fclose(file);

	return VK_TRUE;
}

static VkBool32 profileGetRamData(uint64_t& ram)
{
	FILE* file = fopen("/proc/self/status", "r");

	if (!file)
	{
		return VK_FALSE;
	}
	
	char buffer[VKTS_MAX_RAM_STRING + 1];

	VkBool32 found = VK_FALSE;

	while (fgets(buffer, VKTS_MAX_RAM_STRING, file))
	{
		if (strncmp(buffer, "VmRSS:", 6) == 0)
		{
			if (sscanf(buffer, "%*s %" SCNu64 " %*s", &ram) != 1)
			{
				break;
			}

			found = VK_TRUE;

			break;
		}
	}

	fclose(file);

	if (!found)
	{
		return VK_FALSE;
	}

	return VK_TRUE;
}


VkBool32 VKTS_APIENTRY _profileInit()
{
	g_lastCheck = timeGetRaw();

	if (!profileGetCpuData())
	{
		return VK_FALSE;
	}

	for (uint32_t cpu = 0; cpu < glm::min(processorGetNumber(), VKTS_MAX_QUERY_CPU); cpu++)
	{
		g_lastCpuData[cpu] = g_currentCpuData[cpu];
	}
	
	//
	
	struct tms buffer;
	
	g_lastTime = times(&buffer);
	
	if (g_lastTime == (clock_t)-1)
	{
		return VK_FALSE;	
	}
	
	g_lastKernelTime = buffer.tms_stime;
	g_lastUserTime = buffer.tms_utime;

    return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _profileGetCpuUsage(float& usage, const uint32_t cpu)
{
	if (cpu >= processorGetNumber())
	{
		return VK_FALSE;
	}

	//

	// Data is always collected at once, so no need to query again.
	double currentCheck = timeGetRaw();

	if (currentCheck - g_lastCheck >= 0.1)
	{
		for (uint32_t cpu = 0; cpu < glm::min(processorGetNumber(), VKTS_MAX_QUERY_CPU); cpu++)
		{
			g_lastCpuData[cpu] = g_currentCpuData[cpu];
		}

		if (!profileGetCpuData())
		{
			return VK_FALSE;
		}

		g_lastCheck = currentCheck;
	}

	//

	if (g_currentCpuData[cpu].user < g_lastCpuData[cpu].user || g_currentCpuData[cpu].nice < g_lastCpuData[cpu].nice || g_currentCpuData[cpu].system < g_lastCpuData[cpu].system || g_currentCpuData[cpu].idle < g_lastCpuData[cpu].idle)
	{
		g_lastCpuData[cpu] = g_currentCpuData[cpu];

		return VK_FALSE;
	}

	uint64_t totalUsage = (g_currentCpuData[cpu].user - g_lastCpuData[cpu].user) + (g_currentCpuData[cpu].nice - g_lastCpuData[cpu].nice) + (g_currentCpuData[cpu].system - g_lastCpuData[cpu].system);

	uint64_t total = totalUsage + (g_currentCpuData[cpu].idle - g_lastCpuData[cpu].idle);

	if (total == 0)
	{
		usage = 0.0f;

		return VK_TRUE;
	}

	usage = (float)((double)totalUsage / (double)total) * 100.0f;

	return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _profileApplicationGetCpuUsage(float& usage)
{
	struct tms buffer;
	
	clock_t currentTime = times(&buffer);
	
	if (currentTime == (clock_t)-1)
	{
		return VK_FALSE;	
	}
	
	auto deltaTime = currentTime - g_lastTime;
	
	if (deltaTime <= 0)
	{
		return VK_FALSE;
	}
	
	//
	
	usage = (float)((buffer.tms_stime - g_lastKernelTime) + (buffer.tms_utime - g_lastUserTime) / deltaTime);
	
	//
	
	g_lastTime = currentTime;
	
	g_lastKernelTime = buffer.tms_stime;
	g_lastUserTime = buffer.tms_utime;

    return VK_TRUE;
}

VkBool32 VKTS_APIENTRY _profileApplicationGetRam(uint64_t& ram)
{
	return profileGetRamData(ram);
}


void VKTS_APIENTRY _profileTerminate()
{
	// Nothing for now.
}

}
