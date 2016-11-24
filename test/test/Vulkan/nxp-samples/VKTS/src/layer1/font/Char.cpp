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

#include "Char.hpp"

namespace vkts
{

Char::Char() :
	id(-1), x(0.0f), y(0.0f), width(0.0f), height(0.0f), xoffset(0.0f), yoffset(0.0f), xadvance(0.0f), allKernings()
{
}

Char::Char(const Char& other) :
	id(other.id), x(other.x), y(other.y), width(other.width), height(other.height), xoffset(other.xoffset), yoffset(other.yoffset), xadvance(other.xadvance), allKernings(other.allKernings)
{
}

Char::~Char()
{
}

Char& Char::operator =(const Char& other)
{
	id = other.id;

	x = other.x;
	y = other.y;

	width = other.width;
	height = other.height;

	xoffset = other.xoffset;
	yoffset = other.yoffset;

	xadvance = other.xadvance;

	allKernings = other.allKernings;

	return *this;
}

int32_t Char::getId() const
{
	return id;
}

void Char::setId(const int32_t id)
{
	this->id = id;
}

float Char::getX() const
{
	return x;
}

void Char::setX(const float x)
{
	this->x = x;
}

float Char::getY() const
{
	return y;
}

void Char::setY(const float y)
{
	this->y = y;
}

float Char::getWidth() const
{
	return width;
}

void Char::setWidth(const float width)
{
	this->width = width;
}

float Char::getHeight() const
{
	return height;
}

void Char::setHeight(const float height)
{
	this->height = height;
}

float Char::getXoffset() const
{
	return xoffset;
}

void Char::setXoffset(const float xoffset)
{
	this->xoffset = xoffset;
}

float Char::getYoffset() const
{
	return yoffset;
}

void Char::setYoffset(const float yoffset)
{
	this->yoffset = yoffset;
}

float Char::getXadvance() const
{
	return xadvance;
}

void Char::setXadvance(const float xadvance)
{
	this->xadvance = xadvance;
}

float Char::getKerning(const int32_t characterId) const
{
	auto currentKerning = allKernings.find(characterId);

	if (currentKerning == allKernings.end())
	{
		return 0.0f;
	}

	return currentKerning->second;
}

void Char::setKerning(const int32_t characterId, const float amount)
{
	allKernings[characterId] = amount;
}

} /* namespace vkts */
