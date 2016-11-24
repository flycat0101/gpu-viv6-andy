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

#define VKTS_UNIFORM_RANDOM_BIAS 0.0000001f

namespace vkts
{

void VKTS_APIENTRY randomSetSeed(const uint32_t seed)
{
	srand(seed);
}

float VKTS_APIENTRY randomUniform(const float start, const float end)
{
	return ((float)rand() / (float)(RAND_MAX - 1)) * (end - start) + start;
}

float VKTS_APIENTRY randomNormal(const float mean, const float standardDeviation)
{
	float x1, x2;

	// Avoid logf(0.0f) and logf(1.0f)
	x1 = randomUniform(VKTS_UNIFORM_RANDOM_BIAS, 1.0f - VKTS_UNIFORM_RANDOM_BIAS);
	x2 = randomUniform(0.0f, 1.0f);

	return mean + standardDeviation * (sqrtf(-2.0f * logf(x1)) * cosf(2.0f * VKTS_PI * x2));
}

glm::vec2 VKTS_APIENTRY randomHammersley(const uint32_t sample, const uint32_t m)
{
	uint32_t revertSample;

	// Check, if m is in the allowed range, as only 32bit unsigned integer is supported.
	if (m == 0 || m > 32)
	{
		throw std::out_of_range("0 < m < 32");
	}

	// If not all bits are used: Check, if sample is out of bounds.
	if (m < 32 && sample >= (uint32_t)(1 << m))
	{
		throw std::out_of_range("Sample out of bounds");
	}

	// Revert bits by swapping blockwise. Lower bits are moved up and higher bits down.
	revertSample = (sample << 16u) | (sample >> 16u);
	revertSample = ((revertSample & 0x00ff00ffu) << 8u) | ((revertSample & 0xff00ff00u) >> 8u);
	revertSample = ((revertSample & 0x0f0f0f0fu) << 4u) | ((revertSample & 0xf0f0f0f0u) >> 4u);
	revertSample = ((revertSample & 0x33333333u) << 2u) | ((revertSample & 0xccccccccu) >> 2u);
	revertSample = ((revertSample & 0x55555555u) << 1u) | ((revertSample & 0xaaaaaaaau) >> 1u);

	// Shift back, as only m bits are used.
	revertSample = revertSample >> (32 - m);

	float binaryFractionFactor = 1.0f / (powf(2.0f, (float)m) - 1.0f);

	return glm::vec2((float)revertSample * binaryFractionFactor, (float)sample * binaryFractionFactor);
}

}
