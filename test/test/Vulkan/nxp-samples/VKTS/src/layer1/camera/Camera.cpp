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

#include "Camera.hpp"

namespace vkts
{

void Camera::updateViewMatrix()
{
    viewMatrix = lookAtMat4(glm::vec4(translate, 1.0f), glm::vec4(translate + forward, 1.0f), up);
}

void Camera::update()
{
    updateViewMatrix();
}

Camera::Camera() :
    ICamera()
{
    updateViewMatrix();
}

Camera::Camera(const glm::vec4& eye, const glm::vec3& rotation) :
    ICamera()
{
    setTranslateRotate(glm::vec3(eye), rotation);
}

Camera::Camera(const glm::vec4& eye, const quat& rotationZ, const quat& rotationY, const quat& rotationX) :
    ICamera()
{
    setTranslateRotate(glm::vec3(eye), rotationZ, rotationY, rotationX);
}

Camera::Camera(const Camera& other) :
    ICamera(), viewMatrix(other.viewMatrix)
{
}

Camera::~Camera()
{
}

Camera& Camera::operator =(const Camera& other)
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

    this->viewMatrix = other.viewMatrix;

    return *this;
}

const glm::mat4& Camera::getViewMatrix() const
{
    return viewMatrix;
}

//
// ICloneable
//

ICameraSP Camera::clone() const
{
    return ICameraSP(new Camera(*this));
}

} /* namespace vkts */
