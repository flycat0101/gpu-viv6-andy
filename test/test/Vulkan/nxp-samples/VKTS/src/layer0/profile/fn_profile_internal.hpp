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

#ifndef VKTS_FN_PROFILE_INTERNAL_HPP_
#define VKTS_FN_PROFILE_INTERNAL_HPP_

#include <vkts/vkts.hpp>

#define VKTS_MAX_QUERY_CPU	128u

#define VKTS_MAX_CPU_STRING 128

#define VKTS_MAX_RAM_STRING	128

namespace vkts
{

VKTS_APICALL VkBool32 VKTS_APIENTRY _profileInit();

VKTS_APICALL VkBool32 VKTS_APIENTRY _profileGetCpuUsage(float& usage, const uint32_t cpu);

VKTS_APICALL VkBool32 VKTS_APIENTRY _profileApplicationGetCpuUsage(float& usage);

VKTS_APICALL VkBool32 VKTS_APIENTRY _profileApplicationGetRam(uint64_t& ram);

VKTS_APICALL void VKTS_APIENTRY _profileTerminate();

}

#endif /* VKTS_FN_PROFILE_INTERNAL_HPP_ */
