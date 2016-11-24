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

// see Specular BRDF Reference: http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html

#include <vkts/vkts.hpp>

namespace vkts
{

float VKTS_APIENTRY renderGetGeometricShadowingSchlick(const float NdotV, const float k)
{
	return NdotV / (NdotV * (1.0f - k) + k);
}

float VKTS_APIENTRY renderGetGeometricShadowingSmithSchlickBeckmann(const float NdotL, const float NdotV, const float roughness)
{
	float k = roughness * roughness * sqrtf(2.0f / VKTS_PI);

	return renderGetGeometricShadowingSchlick(NdotL, k) * renderGetGeometricShadowingSchlick(NdotV, k);
}

float VKTS_APIENTRY renderGetGeometricShadowingSmithSchlickGGX(float NdotL, float NdotV, float roughness)
{
	float k = roughness * roughness * 0.5f;

	return renderGetGeometricShadowingSchlick(NdotL, k) * renderGetGeometricShadowingSchlick(NdotV, k);
}

glm::vec3 VKTS_APIENTRY renderGetGGXWeightedVector(const glm::vec2& e, const float roughness)
{
	float alpha = roughness * roughness;

	float phi = 2.0f * VKTS_PI * e.y;
	float cosTheta = sqrtf((1.0f - e.x) / (1.0f + (alpha*alpha - 1.0f) * e.x));
	float sinTheta = sqrtf(1.0f - cosTheta*cosTheta);

	// Note: Polar Coordinates
	// x = sin(theta)*cos(phi)
	// y = sin(theta)*sin(phi)
	// z = cos(theta)

	float x = sinTheta * cosf(phi);
	float y = sinTheta * sinf(phi);
	float z = cosTheta;

	return glm::normalize(glm::vec3(x, y, z));
}

glm::vec3 VKTS_APIENTRY renderGetCosineWeightedVector(const glm::vec2& e)
{
	float x = sqrtf(1.0f - e.x) * cosf(2.0f * VKTS_PI * e.y);
	float y = sqrtf(1.0f - e.x) * sinf(2.0f * VKTS_PI * e.y);
	float z = sqrtf(e.x);

	return glm::normalize(glm::vec3(x, y, z));
}

glm::vec2 VKTS_APIENTRY renderIntegrateCookTorrance(const glm::vec2& randomPoint, const float NdotV, const glm::vec3& V, const float roughness)
{
	glm::vec3 H = renderGetGGXWeightedVector(randomPoint, roughness);

	// Note: reflect takes incident vector.
	glm::vec3 L = reflect(-V, H);

	// N is vec3(0.0, 0.0, 1.0)
	float NdotL = L.z;
	float NdotH = H.z;

	// Lighted and visible
	if (NdotL > 0.0 && NdotV != 0.0f && NdotH != 0.0f)
	{
		float VdotH = glm::dot(V, H);

		// Geometric Shadowing
		float G = renderGetGeometricShadowingSmithSchlickGGX(NdotL, NdotV, roughness);

		//
		// Lo = BRDF * L * NdotL / PDF
		//
		// BRDF is D * F * G / (4 * NdotL * NdotV).
		// PDF is D * NdotH / (4 * VdotH).
		// D and 4 * NdotL are canceled out, which results in this formula: Lo = color * L * F * G * VdotH / (NdotV * NdotH)
		//
		// Using the approximation as
		// seen on page 6 in Real Shading in Unreal Engine 4
		// L is stored in the cube map array, F is replaced and color is later used in the real-time BRDF shader.

		float colorFactor = G * VdotH / (NdotV * NdotH);

		float fresnelFactor = powf(1.0f - VdotH, 5.0f);

		// Scale to F0 (first sum)
		float scaleF0 = (1.0f - fresnelFactor) * colorFactor;

		// Bias to F0 (second sum)
		float biasF0 = fresnelFactor * colorFactor;

		return glm::vec2(scaleF0, biasF0);
	}

	return glm::vec2(0.0f, 0.0f);
}

glm::vec3 VKTS_APIENTRY renderCookTorrance(const IImageDataSP& cubeMap, const VkFilter filter, const uint32_t mipLevel, const glm::vec2& randomPoint, const glm::mat3& basis, const glm::vec3& N, const glm::vec3& V, const float roughness)
{
	if (!cubeMap.get() || cubeMap->getArrayLayers() != 6)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 HtangentSpace = renderGetGGXWeightedVector(randomPoint, roughness);

	// Transform H to world space.
	glm::vec3 H = basis * HtangentSpace;

	// Note: reflect takes incident vector.
	glm::vec3 L = glm::reflect(-V, H);

	return glm::vec3(cubeMap->getSampleCubeMap(L.x, L.y, L.z, filter, mipLevel));
}

glm::vec3 VKTS_APIENTRY renderOrenNayar(const IImageDataSP& cubeMap, const VkFilter filter, const uint32_t mipLevel, const glm::vec2& randomPoint, const glm::mat3& basis, const glm::vec3& N, const glm::vec3& V, const float roughness)
{
	if (!cubeMap.get() || cubeMap->getArrayLayers() != 6)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 LtangentSpace = renderGetCosineWeightedVector(randomPoint);

	// Transform light ray to world space.
	glm::vec3 L = basis * LtangentSpace;


    float NdotL = glm::dot(N, L);
    float NdotV = glm::dot(N, V);

    float angleVN = acosf(NdotV);
    float angleLN = acosf(NdotL);

    float alpha = glm::max(angleVN, angleLN);
    float beta = glm::min(angleVN, angleLN);
    float gamma = glm::dot(V - N * NdotV, L - N * NdotL);


    float roughnessSquared = roughness * roughness;


    float A = 1.0f - 0.5f * (roughnessSquared / (roughnessSquared + 0.57f));

    float B = 0.45f * (roughnessSquared / (roughnessSquared + 0.09f));

    float C = sinf(alpha) * tanf(beta);


    float Lr = glm::max(0.0f, NdotL) * (A + B * glm::max(0.0f, gamma) * C);


	return glm::vec3(cubeMap->getSampleCubeMap(L.x, L.y, L.z, filter, mipLevel)) * Lr;
}

glm::vec3 VKTS_APIENTRY renderLambert(const IImageDataSP& cubeMap, const VkFilter filter, const uint32_t mipLevel, const glm::vec2& randomPoint, const glm::mat3& basis)
{
	if (!cubeMap.get() || cubeMap->getArrayLayers() != 6)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 LtangentSpace = renderGetCosineWeightedVector(randomPoint);

	// Transform light ray to world space.
	glm::vec3 L = basis * LtangentSpace;

	return glm::vec3(cubeMap->getSampleCubeMap(L.x, L.y, L.z, filter, mipLevel));
}

glm::mat3 VKTS_APIENTRY renderGetBasis(const glm::vec3& normal)
{
	glm::vec3 bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

	float NdotB = glm::dot(normal, bitangent);

	if (NdotB == 1.0f)
	{
		bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
	}
	else if (NdotB == -1.0f)
	{
		bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	glm::vec3 tangent = glm::normalize(glm::cross(bitangent, normal));
	bitangent = glm::cross(normal, tangent);

	return glm::mat3(tangent, bitangent, normal);
}

}
