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

#include "../Base/String.h"
#include "../Math/Math.h"
#include "../Math/Matrix4x4.h"

namespace alimer
{
    /// Defines a transform in space.
    class ALIMER_API Transform
    {
    public:
        Transform() = default;
        Transform(const Transform& t) = default;
        Transform(Transform&& t) = default;
        Transform& operator=(const Transform& m) = default;
        Transform& operator=(Transform&& m) = default;

        Transform(const mat4& m);

        const vec3& GetPosition() const { return _position; }
        void SetPosition(const vec3& position);

        const quat& GetRotation() const { return _rotation; }
        void SetRotation(const quat& rotation);

        const vec3&  GetScale() const { return _scale; }
        void SetScale(const vec3& scale);

        const mat4& GetMatrix() const
        {
            Update();
            return _matrix;
        }

        // Constants
        static const Transform Identity;

    private:
        void Decompose();
        void Update() const;

        mutable mat4 _matrix = mat4::identity();
        vec3 _position = vec3(0.0f, 0.0f, 0.0f);
        quat _rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
        vec3 _scale = vec3(1.0f);

        mutable bool _dirty = false;
    };
}

