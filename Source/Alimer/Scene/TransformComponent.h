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

#pragma once

#include "../Math/MathUtil.h"
#include "../Scene/Entity.h"

namespace Alimer
{
    struct Transform
    {
        //glm::vec3 scale = glm::vec3(1.0f);
        //glm::vec3 translation = glm::vec3(0.0f);
        //glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

	/// Defines a Transform Component.
    class ALIMER_API TransformComponent : public Component
	{
    public:
        TransformComponent();
        virtual ~TransformComponent() = default;

        Transform transform;

        //glm::mat4 worldTransform;

        uint32_t lastTimestamp = ~0u;
        const uint32_t *currentTimestamp = nullptr;
	};
}
