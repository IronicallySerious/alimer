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

#include "../Renderer/Camera.h"

#define CAMERA_DIRTY_VIEW 1
#define CAMERA_DIRTY_PROJ 2
#define CAMERA_DIRTY_FLAGS_ALL (CAMERA_DIRTY_VIEW | CAMERA_DIRTY_PROJ)

namespace Alimer
{
    Camera::Camera()
        : _dirtyFlags(CAMERA_DIRTY_FLAGS_ALL)
    {

    }

    void Camera::SetProjectionMode(ProjectionMode mode)
    {
        if (_projectionMode == mode)
        {
            return;
        }

        // Set projection mode and change dirty flags.
        _projectionMode = mode;
        _dirtyFlags = CAMERA_DIRTY_FLAGS_ALL;
    }
}

/*
#include <rttr/registration>
using namespace rttr;

RTTR_REGISTRATION
{
    registration::enumeration<Alimer::ProjectionMode>("ProjectionMode")(
        rttr::value("Perspective", Alimer::ProjectionMode::Perspective),
        rttr::value("Orthographic", Alimer::ProjectionMode::Orthographic)
        );

    registration::class_<Alimer::Camera>("Camera")
        //.constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property("projection_mode", &Alimer::Camera::GetProjectionMode, &Alimer::Camera::SetProjectionMode)
       ;
};
*/
