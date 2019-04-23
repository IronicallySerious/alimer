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

#include "Math/math.h"

namespace alimer
{
    enum class ProjectionMode : std::uint32_t
    {
        Perspective = 0,
        Orthographic = 1
    };

	/// Defines a Camera.
    class ALIMER_API Camera final
	{
    public:
        Camera();
        virtual ~Camera() = default;

        void SetProjectionMode(ProjectionMode mode);

        inline ProjectionMode GetProjectionMode() const
        {
            return _projectionMode;
        }

    private:
        ProjectionMode _projectionMode = ProjectionMode::Perspective;

        // Field of view (in degrees)
        float _fovy = 60.0f;
        float _aspect = 16.0f / 9.0f;
        float _znear = 1.0f;
        float _zfar = 1000.0f;

        // Calculated values.
        mat4 _view;
        mat4 _projection;

        /// Camera dirty flags.
        mutable int _dirtyFlags;
	};
}
