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

#include "../Math/MathUtil.h"

namespace alimer
{
    bool IsZero(float value)
    {
        return fabsf(value) < M_EPSILON;
    }

    bool IsOne(float value)
    {
        return IsZero(value - 1.0f);
    }

    bool ScalarNearEqual(float scalar1, float scalar2, float epsilon)
    {
        float delta = scalar1 - scalar2;
        return (fabsf(delta) <= epsilon);
    }

    void ScalarSinCos(float angle, float* sin, float* cos)
    {
#if defined(HAVE_SINCOSF)
        sincosf(angle, sin, &cos);
#elif defined(__MACOSX__) && !defined(__INTEL_COMPILER)
        __sincosf(angle, &sin, &cos);
#else
        *sin = sinf(angle);
        *cos = cosf(angle);
#endif
    }

    /*void ComputeTransform(vec3 translation, quat rotation, vec3 scale, mat4 &world, const mat4 &parent)
    {
        mat4 S = glm::scale(scale);
        mat4 R = glm::mat4_cast(rotation);
        mat4 T = glm::translate(translation);

        mat4 model = R * S;
        world = parent * T * model;
    }*/
}
