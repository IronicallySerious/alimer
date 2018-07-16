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

#include "../Math/Vector3.h"
#include "../Math/MathUtil.h"

namespace Alimer
{
    const Vector3 Vector3::Zero = { 0.0f, 0.0f, 0.0f };
    const Vector3 Vector3::One = { 1.0f, 1.0f, 1.0f };
    const Vector3 Vector3::UnitX = { 1.0f, 0.0f, 0.0f };
    const Vector3 Vector3::UnitY = { 0.0f, 1.0f, 0.0f };
    const Vector3 Vector3::UnitZ = { 0.0f, 0.0f, 1.0f };
    const Vector3 Vector3::Up = { 0.0f, 1.0f, 0.0f };
    const Vector3 Vector3::Down = { 0.0f, -1.0f, 0.0f };
    const Vector3 Vector3::Right = { 1.0f, 0.0f, 0.0f };
    const Vector3 Vector3::Left = { -1.0f, 0.0f, 0.0f };
    const Vector3 Vector3::Forward = { 0.0f, 0.0f, -1.0f };
    const Vector3 Vector3::Backward = { 0.0f, 0.0f, 1.0f };

    void Vector3::Add(const Vector3& left, const Vector3& right, Vector3* result)
    {
        ALIMER_ASSERT(result);
        result->x = left.x + right.x;
        result->y = left.y + right.y;
        result->z = left.z + right.z;
    }

    Vector3 Vector3::Add(const Vector3& left, const Vector3& right)
    {
        Vector3 result;
        Add(left, right, &result);
        return result;
    }

    void Vector3::Subtract(const Vector3& left, const Vector3& right, Vector3* result)
    {
        ALIMER_ASSERT(result);
        result->x = left.x - right.x;
        result->y = left.y - right.y;
        result->z = left.z - right.z;
    }

    Vector3 Vector3::Subtract(const Vector3& left, const Vector3& right)
    {
        Vector3 result;
        Subtract(left, right, &result);
        return result;
    }

    void Vector3::Multiply(const Vector3& left, const Vector3& right, Vector3* result)
    {
        ALIMER_ASSERT(result);
        result->x = left.x * right.x;
        result->y = left.y * right.y;
        result->z = left.z * right.z;
    }

    Vector3 Vector3::Multiply(const Vector3& left, const Vector3& right)
    {
        Vector3 result;
        Multiply(left, right, &result);
        return result;
    }

    void Vector3::Divide(const Vector3& left, const Vector3& right, Vector3* result)
    {
        ALIMER_ASSERT(result);
        result->x = left.x / right.x;
        result->y = left.y / right.y;
        result->z = left.z / right.z;
    }

    Vector3 Vector3::Divide(const Vector3& left, const Vector3& right)
    {
        Vector3 result;
        Divide(left, right, &result);
        return result;
    }

    void Vector3::Cross(const Vector3& left, const Vector3& right, Vector3* result)
    {
        ALIMER_ASSERT(result);
        result->x = (left.y * right.z) - (left.z * right.y);
        result->y = (left.z * right.x) - (left.x * right.z);
        result->z = (left.x * right.y) - (left.y * right.x);
    }

    Vector3 Vector3::Cross(const Vector3& left, const Vector3& right)
    {
        Vector3 result;
        Cross(left, right, &result);
        return result;
    }

    float Vector3::Dot(const Vector3& left, const Vector3& right)
    {
        return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
    }
}
