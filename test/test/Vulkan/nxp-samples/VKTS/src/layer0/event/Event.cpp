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

#include "Event.hpp"

namespace vkts
{

Event::Event(const VkDevice device, const VkEventCreateFlags flags, const VkEvent event) :
    IEvent(), device(device), eventCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, flags}, event(event)
{
}

Event::~Event()
{
    destroy();
}

//
// IEvent
//

VkResult Event::resetEvent() const
{
    return vkResetEvent(device, event);
}

VkResult Event::getStatus() const
{
	return vkGetEventStatus(device, event);
}

const VkDevice Event::getDevice() const
{
    return device;
}

const VkEventCreateInfo& Event::getEventCreateInfo() const
{
    return eventCreateInfo;
}

VkEventCreateFlags Event::getFlags() const
{
    return eventCreateInfo.flags;
}

const VkEvent Event::getEvent() const
{
    return event;
}

//
// IResetable
//

VkResult Event::reset()
{
    return resetEvent();
}

//
// IDestroyable
//

void Event::destroy()
{
    if (event)
    {
        vkDestroyEvent(device, event, nullptr);

        event = VK_NULL_HANDLE;
    }
}

} /* namespace vkts */
