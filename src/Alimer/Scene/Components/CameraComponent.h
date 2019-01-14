//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#pragma once

#include "../Entity.h"
#include "../../Renderer/Camera.h"
#include "../../Math/Transform.h"

#if TODO_ENTITY
namespace alimer
{
    /// Defines a Camera Component class.
    class ALIMER_API CameraComponent final : public Component<CameraComponent>
    {
        // ALIMER_OBJECT(CameraComponent, Component);

    public:
        CameraComponent() = default;
        virtual ~CameraComponent();

        void Update(const Transform& transform);

        mat4 GetView() const;
        mat4 GetProjection() const;

    public:
        // Field of view (in degrees)
        float fovy = 60.0f;
        float aspect = 16.0f / 9.0f;
        float znear = 1.0f;
        float zfar = 1000.0f;

    private:
        Camera _camera;

        // Calculated values.
        mat4 _view;
        mat4 _projection;
    };
}

#endif // TODO_ENTITY
