//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Scene/CameraComponent.h"
#include "../Scene/TransformComponent.h"

namespace Alimer
{
    CameraComponent::CameraComponent()
        : _fovy(0.5f * glm::half_pi<float>())
        , _aspect(16.0f / 9.0f)
        , _znear(1.0f)
        , _zfar(1000.0f)
    {
        
    }

    void CameraComponent::Update(const glm::mat4& worldTransform)
    {
        _view = glm::inverse(worldTransform);
        _projection = glm::perspective(_fovy, _aspect, _znear, _zfar);
    }

    glm::mat4 CameraComponent::GetView() const
    {
        return _view;
    }

    glm::mat4 CameraComponent::GetProjection() const
    {
        return _projection;
    }
}
