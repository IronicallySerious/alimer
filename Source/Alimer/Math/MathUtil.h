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

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/packing.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../AlimerConfig.h"
#include <string>

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable:4244) // Conversion from 'double' to 'float'
#   pragma warning(disable:4702) // unreachable code
#endif

namespace Alimer
{
    /// Check whether two floating point values are equal within accuracy.
    template <class T>
    inline bool Equals(T lhs, T rhs) { return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs; }

    /// Linear interpolation between two values.
    template <class T, class U>
    inline T Lerp(T lhs, T rhs, U t) { return lhs * (1.0 - t) + rhs * t; }

    /// Return the smaller of two values.
    template <class T, class U>
    inline T Min(T lhs, U rhs) { return lhs < rhs ? lhs : rhs; }

    /// Return the larger of two values.
    template <class T, class U>
    inline T Max(T lhs, U rhs) { return lhs > rhs ? lhs : rhs; }

    /// Return absolute value of a value
    template <class T>
    inline T Abs(T value) { return value >= 0.0 ? value : -value; }

    /// Return the sign of a float (-1, 0 or 1.)
    template <class T>
    inline T Sign(T value) { return value > 0.0 ? 1.0 : (value < 0.0 ? -1.0 : 0.0); }

    /// Return a representation of the specified floating-point value as a single format bit layout.
    inline uint32_t FloatToRawIntBits(float value)
    {
        uint32_t u = *((uint32_t*)&value);
        return u;
    }

    /// Check whether a floating point value is NaN.
    template <class T> inline bool IsNaN(T value) { return std::isnan(value); }

    /// Check whether an unsigned integer is a power of two.
    inline bool IsPowerOfTwo(size_t value)
    {
        return !(value & (value - 1));
    }

    /// Round up to next power of two.
    inline uint32_t NextPowerOfTwo(uint32_t value)
    {
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        --value;
        value |= value >> 1u;
        value |= value >> 2u;
        value |= value >> 4u;
        value |= value >> 8u;
        value |= value >> 16u;
        return ++value;
    }

    inline uint32_t Align(uint32_t value, uint32_t alignment)
    {
        ALIMER_ASSERT(alignment <= UINT32_MAX);
        ALIMER_ASSERT(IsPowerOfTwo(alignment));
        ALIMER_ASSERT(alignment != 0);
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    inline void* AlignVoidPtr(void* ptr, size_t alignment)
    {
        ALIMER_ASSERT(IsPowerOfTwo(alignment));
        ALIMER_ASSERT(alignment != 0);
        return reinterpret_cast<void*>((reinterpret_cast<size_t>(ptr) + (alignment - 1)) &
            ~(alignment - 1));
    }

    template <typename T>
    T* AlignPtr(T* ptr, size_t alignment)
    {
        return reinterpret_cast<T*>(AlignVoidPtr(ptr, alignment));
    }

    template <typename T>
    const T* AlignPtr(const T* ptr, size_t alignment)
    {
        return reinterpret_cast<const T*>(AlignVoidPtr(const_cast<T*>(ptr), alignment));
    }

    inline uint32_t ScanForward(uint32_t bits)
    {
        ALIMER_ASSERT(bits != 0);
#if defined(_MSC_VER)
        unsigned long firstBitIndex = 0ul;
        uint8_t ret = _BitScanForward(&firstBitIndex, bits);
        ALIMER_ASSERT(ret != 0);
        return firstBitIndex;
#else
        return static_cast<uint32_t>(__builtin_ctz(bits));
#endif
    }

    inline bool IsAligned(uint32_t value, uint32_t alignment)
    {
        ALIMER_ASSERT(alignment <= UINT32_MAX);
        ALIMER_ASSERT(IsPowerOfTwo(alignment));
        ALIMER_ASSERT(alignment != 0);
        return (value & (alignment - 1)) == 0;
    }

    inline bool IsPtrAligned(const void* ptr, size_t alignment)
    {
        ALIMER_ASSERT(IsPowerOfTwo(alignment));
        ALIMER_ASSERT(alignment != 0);
        return (reinterpret_cast<size_t>(ptr) & (alignment - 1)) == 0;
    }

    /// Clamp a number to a range.
    template <class T>
    inline T Clamp(T value, T min, T max)
    {
        if (value < min)
            return min;
        else if (value > max)
            return max;
        else
            return value;
    }

    /// Smoothly damp between values.
    template <class T>
    inline T SmoothStep(T lhs, T rhs, T t)
    {
        t = Clamp((t - lhs) / (rhs - lhs), T(0.0), T(1.0)); // Saturate t
        return t * t * (3.0 - 2.0 * t);
    }

    /// Return X in power Y.
    template <class T> inline T Pow(T x, T y) { return pow(x, y); }

    /// Return natural logarithm of X.
    template <class T> inline T Ln(T x) { return log(x); }

    /// Return square root of X.
    template <class T> inline T Sqrt(T x) { return sqrt(x); }

    /// Return floating-point remainder of X/Y.
    template <class T> inline T Mod(T x, T y) { return fmod(x, y); }

    /// Return fractional part of passed value in range [0, 1).
    template <class T> inline T Fract(T value) { return value - floor(value); }

    /// Round value down.
    template <class T> inline T Floor(T x) { return floor(x); }

    /// Round value down. Returns integer value.
    template <class T> inline int FloorToInt(T x) { return static_cast<int>(floor(x)); }

    /// Round value to nearest integer.
    template <class T> inline T Round(T x) { return round(x); }

    /// Round value to nearest integer.
    template <class T> inline int RoundToInt(T x) { return static_cast<int>(round(x)); }

    /// Round value up.
    template <class T> inline T Ceil(T x) { return ceil(x); }

    /// Round value up.
    template <class T> inline int CeilToInt(T x) { return static_cast<int>(ceil(x)); }

    void ComputeTransform(glm::vec3 translation, glm::quat rotation, glm::vec3 scale, glm::mat4 &world, const glm::mat4 &parent);
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif
