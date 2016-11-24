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

#include "Surface.hpp"

namespace vkts
{

Surface::Surface(const VkInstance instance, const VKTS_NATIVE_DISPLAY nativeDisplay, const VKTS_NATIVE_WINDOW nativeWindow, const VkSurfaceKHR surface) :
    ISurface(), instance(instance), nativeDisplay(nativeDisplay), nativeWindow(nativeWindow), surface(surface), allExtents(), zeroExtent{0, 0}
{
}

Surface::~Surface()
{
    destroy();
}

//
// ISurface
//

const VkInstance Surface::getInstance() const
{
    return instance;
}

const VKTS_NATIVE_DISPLAY Surface::getNativeDisplay() const
{
    return nativeDisplay;
}

const VKTS_NATIVE_WINDOW Surface::getNativeWindow() const
{
    return nativeWindow;
}

const VkSurfaceKHR Surface::getSurface() const
{
    return surface;
}

VkResult Surface::getPhysicalDeviceSurfaceCapabilities(const VkPhysicalDevice physicalDevice, VkSurfaceCapabilitiesKHR& surfaceCapabilities) const
{
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
}

VkBool32 Surface::hasCurrentExtentChanged(const VkPhysicalDevice physicalDevice) const
{
	VkBool32 changed = VK_FALSE;

	//

	auto currentExtent = allExtents.find(physicalDevice);

	if (currentExtent == allExtents.end())
	{
		changed = VK_TRUE;
	}

	//

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	auto result = getPhysicalDeviceSurfaceCapabilities(physicalDevice, surfaceCapabilities);

	if (result == VK_SUCCESS)
	{
		if (!changed)
		{
			if (currentExtent->second.width != surfaceCapabilities.currentExtent.width || currentExtent->second.height != surfaceCapabilities.currentExtent.height)
			{
				changed = VK_TRUE;
			}
		}

		if (changed)
		{
			allExtents[physicalDevice] = surfaceCapabilities.currentExtent;
		}
	}
	else
	{
		if (changed)
		{
			allExtents.erase(currentExtent);
		}

		changed = VK_TRUE;
	}

	if (!changed)
	{
		if (allExtents[physicalDevice].width == 0 || allExtents[physicalDevice].height == 0)
		{
			changed = VK_TRUE;
		}
	}

	return changed;
}

const VkExtent2D& Surface::getCurrentExtent(const VkPhysicalDevice physicalDevice, const VkBool32 refresh) const
{
	if (refresh)
	{
		if (!hasCurrentExtentChanged(physicalDevice))
		{
			return zeroExtent;
		}
	}
	else
	{
		auto currentExtent = allExtents.find(physicalDevice);

		if (currentExtent == allExtents.end())
		{
			return zeroExtent;
		}
	}

	return allExtents[physicalDevice];
}

//
// IDestroyable
//

void Surface::destroy()
{
    if (surface)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);

        surface = VK_NULL_HANDLE;
    }
}

} /* namespace vkts */
