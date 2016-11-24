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

namespace vkts
{

void IMoveable::updateVectors()
{
    quat rotation = rotateZ * rotateY * rotateX;

    forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    left = rotation * glm::vec3(-1.0f, 0.0f, 0.0f);
    up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 IMoveable::getAngle(const glm::vec3& forward, const glm::vec3& up) const
{
	glm::vec3 rotate(0.0f, 0.0f, 0.0f);

	//

	auto newForward = glm::normalize(forward);
	auto newUp = glm::normalize(up);

	auto side = glm::cross(newForward, newUp);

	newUp = glm::cross(side, newForward);

	//

	auto newForwardZ = glm::normalize(glm::vec3(newForward.x, 0.0f, newForward.z));

	//

	rotate.x = 90.0f - glm::degrees(acosf(glm::dot(newForward, glm::vec3(0.0f, 1.0f, 0.0f))));

	rotate.y = glm::degrees(acosf(glm::dot(newForwardZ, glm::vec3(0.0f, 0.0f, -1.0f))));

	if (glm::dot(newUp, glm::vec3(0.0f, 1.0f, 0.0f)) < 0.0f)
	{
		rotate.x = 180.0f - rotate.x;
		rotate.y = rotate.y + 180.0f;
	}

	if (glm::dot(newForwardZ, glm::vec3(-1.0f, 0.0f, 0.0f)) < 0.0f)
	{
		rotate.y = -rotate.y;
	}

	rotate.z = 0.0f;

	return rotate;
}

IMoveable::IMoveable() :
    IUpdateable(), translate(0.0f, 0.0f, 0.0f), rotateZ(), rotateY(), rotateX(), targetTranslate(0.0f, 0.0f, 0.0f), targetRotateZ(), targetRotateY(), targetRotateX(), forward(0.0f, 0.0f, -1.0f), left(-1.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f)
{
}

IMoveable::IMoveable(const glm::vec3& translate, const glm::vec3& rotate) :
    IUpdateable(), translate(), rotateZ(), rotateY(), rotateX(), targetTranslate(), targetRotateZ(), targetRotateY(), targetRotateX(), forward(0.0f, 0.0f, -1.0f), left(-1.0f, 0.0f, 0.0f), up(0.0f, 1.0f,0.0f)
{
    setTranslateRotate(translate, rotate);
}

IMoveable::IMoveable(const glm::vec3& translate, const quat& rotateZ, const quat& rotateY, const quat& rotateX) :
    IUpdateable(), translate(), rotateZ(), rotateY(), rotateX(), targetTranslate(), targetRotateZ(), targetRotateY(), targetRotateX(), forward(0.0f, 0.0f, -1.0f), left(-1.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f)
{
    setTranslateRotate(translate, rotateZ, rotateY, rotateX);
}

IMoveable::IMoveable(const IMoveable& other) :
    IUpdateable(), translate(other.translate), rotateZ(other.rotateZ), rotateY(other.rotateY), rotateX(other.rotateX), targetTranslate(other.targetTranslate), targetRotateZ(other.targetRotateZ), targetRotateY(other.targetRotateY), targetRotateX(other.targetRotateX), forward(other.forward), left(other.left), up(other.up)
{
}

IMoveable::~IMoveable()
{
}

IMoveable& IMoveable::operator =(const IMoveable& other)
{
    this->translate = other.translate;
    this->rotateZ = other.rotateZ;
    this->rotateY = other.rotateY;
    this->rotateX = other.rotateX;

    this->targetTranslate = other.targetTranslate;
    this->targetRotateZ = other.targetRotateZ;
    this->targetRotateY = other.targetRotateY;
    this->targetRotateX = other.targetRotateX;

    this->forward = other.forward;
    this->left = other.left;
    this->up = other.up;

    return *this;
}

//
// IMoveable
//

void IMoveable::setTranslateRotate(const glm::vec3& translate, const glm::vec3& rotate)
{
    this->translate = translate;
    this->rotateZ = rotateRz(rotate.z);
    this->rotateY = rotateRy(rotate.y);
    this->rotateX = rotateRx(rotate.x);

    this->targetTranslate = this->translate;
    this->targetRotateZ = this->rotateZ;
    this->targetRotateY = this->rotateY;
    this->targetRotateX = this->rotateX;

    updateVectors();
    update();
}

void IMoveable::setTranslateRotate(const glm::vec3& translate, const quat& rotateZ, const quat& rotateY, const quat& rotateX)
{
    this->translate = translate;
    this->rotateZ = rotateZ;
    this->rotateY = rotateY;
    this->rotateX = rotateX;

    this->targetTranslate = this->translate;
    this->targetRotateZ = this->rotateZ;
    this->targetRotateY = this->rotateY;
    this->targetRotateX = this->rotateX;

    updateVectors();
    update();
}

void IMoveable::setTranslate(const glm::vec3& translate)
{
    this->translate = translate;

    this->targetTranslate = this->translate;

    update();
}

const glm::vec3& IMoveable::getTranslate() const
{
	return translate;
}

void IMoveable::setRotate(const glm::vec3& rotate)
{
    this->rotateZ = rotateRz(rotate.z);
    this->rotateY = rotateRy(rotate.y);
    this->rotateX = rotateRx(rotate.x);

    this->targetRotateZ = this->rotateZ;
    this->targetRotateY = this->rotateY;
    this->targetRotateX = this->rotateX;

    updateVectors();
    update();
}

void IMoveable::setRotate(const quat& rotateZ, const quat& rotateY, const quat& rotateX)
{
    this->rotateZ = rotateZ;
    this->rotateY = rotateY;
    this->rotateX = rotateX;

    this->targetRotateZ = this->rotateZ;
    this->targetRotateY = this->rotateY;
    this->targetRotateX = this->rotateX;

    updateVectors();
    update();
}

const quat& IMoveable::getRotateX() const
{
	return rotateX;
}

const quat& IMoveable::getRotateY() const
{
	return rotateY;
}

const quat& IMoveable::getRotateZ() const
{
	return rotateZ;
}

glm::vec3 IMoveable::getRotate() const
{
	return glm::vec3(glm::degrees(2.0f * acosf(rotateX.w)), glm::degrees(2.0f * acosf(rotateY.w)), glm::degrees(2.0f * acosf(rotateZ.w)));
}

void IMoveable::setForwardUp(const glm::vec3& forward, const glm::vec3& up)
{
	setRotate(getAngle(forward, up));
}

const glm::vec3& IMoveable::getForward() const
{
	return forward;
}

const glm::vec3& IMoveable::getLeft() const
{
	return left;
}

const glm::vec3& IMoveable::getUp() const
{
	return up;
}

void IMoveable::moveDirection(const glm::vec3& forward, const glm::vec3& up)
{
	glm::vec3 rotate = getAngle(forward, up);

	this->targetRotateZ = rotateRz(rotate.z);
    this->targetRotateY = rotateRy(rotate.y);
    this->targetRotateX = rotateRx(rotate.x);
}

void IMoveable::moveTranslateDirection(const glm::vec3& translate, const glm::vec3& forward, const glm::vec3& up)
{
	this->targetTranslate = translate;

	glm::vec3 rotate = getAngle(forward, up);

	this->targetRotateZ = rotateRz(rotate.z);
    this->targetRotateY = rotateRy(rotate.y);
    this->targetRotateX = rotateRx(rotate.x);
}

void IMoveable::moveTranslateRotate(const glm::vec3& translate, const glm::vec3& rotate)
{
    this->targetTranslate = translate;
    this->targetRotateZ = targetRotateZ * rotateRz(rotate.z);
    this->targetRotateY = targetRotateY * rotateRy(rotate.y);
    this->targetRotateX = targetRotateX * rotateRx(rotate.x);
}

void IMoveable::moveTranslateRotate(const glm::vec3& translate, const quat& rotateZ, const quat& rotateY, const quat& rotateX)
{
    this->targetTranslate = translate;
    this->targetRotateZ = targetRotateZ * rotateZ;
    this->targetRotateY = targetRotateY * rotateY;
    this->targetRotateX = targetRotateX * rotateX;
}

void IMoveable::moveTranslateRotate(const float forwardFactor, const float strafeFactor, const float upFactor, const glm::vec3& rotate)
{
    this->targetTranslate += forward * forwardFactor + left * strafeFactor + up * upFactor;
    this->targetRotateZ = targetRotateZ * rotateRz(rotate.z);
    this->targetRotateY = targetRotateY * rotateRy(rotate.y);
    this->targetRotateX = targetRotateX * rotateRx(rotate.x);
}

void IMoveable::moveTranslateRotate(const float forwardFactor, const float strafeFactor, const float upFactor, const quat& rotateZ, const quat& rotateY, const quat& rotateX)
{
    this->targetTranslate += forward * forwardFactor + left * strafeFactor + up * upFactor;
    this->targetRotateZ = targetRotateZ * rotateZ;
    this->targetRotateY = targetRotateY * rotateY;
    this->targetRotateX = targetRotateX * rotateX;
}

//
// IUpdateable
//

VkBool32 IMoveable::update(const double deltaTime, const uint64_t deltaTicks)
{
    if (targetTranslate != translate || targetRotateZ != rotateZ || targetRotateY != rotateY || targetRotateX != rotateX)
    {
        glm::vec3 currentTranslate = (targetTranslate + translate) * 0.5f;

        if (dot(targetRotateZ, rotateZ) < 0.0f)
        {
			targetRotateZ = -targetRotateZ;
        }
        if (dot(targetRotateY, rotateY) < 0.0f)
        {
			targetRotateY = -targetRotateY;
        }
        if (dot(targetRotateX, rotateX) < 0.0f)
        {
			targetRotateX = -targetRotateX;
        }

		quat currentRotateZ = slerp(targetRotateZ, rotateZ, 0.5f);
		quat currentRotateY = slerp(targetRotateY, rotateY, 0.5f);
		quat currentRotateX = slerp(targetRotateX, rotateX, 0.5f);

        this->translate = currentTranslate;
        this->rotateZ = currentRotateZ;
        this->rotateY = currentRotateY;
        this->rotateX = currentRotateX;

        updateVectors();
        update();
    }

    return VK_TRUE;
}

} /* namespace vkts */
