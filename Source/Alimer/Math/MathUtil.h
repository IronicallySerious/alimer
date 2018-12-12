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

#include "../Debug/Debug.h"

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable:4244) // Conversion from 'double' to 'float'
#   pragma warning(disable:4702) // unreachable code
#endif

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <limits>

#ifdef _MSC_VER
#   include <intrin.h>
#endif

namespace Alimer
{
#ifdef M_PI
#   undef M_PI
#endif

    constexpr float M_PI = 3.141592654f;
    constexpr float M_2PI = 6.283185307f;
    constexpr float M_1DIVPI = 0.318309886f;
    constexpr float M_1DIV2PI = 0.159154943f;
    constexpr float M_PIDIV2 = 1.570796327f;
    constexpr float M_PIDIV4 = 0.785398163f;
    constexpr float M_DEGTORAD = M_PI / 180.0f;
    constexpr float M_RADTODEG = 180.0f / M_PI;

    constexpr float M_EPSILON = 0.000001f;
    constexpr float M_LARGE_EPSILON = 0.00005f;

    /// Check whether two floating point values are equal within accuracy.
    template <class T>
    inline bool Equals(T lhs, T rhs) { return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs; }

    /// Return the smaller of two values.
    template <typename T>
    inline constexpr T Min(T a, T b) noexcept { return b < a ? b : a; }

    template <typename T>
    inline constexpr T Max(T a, T b) noexcept { return a < b ? b : a; }

    template <typename T>
    inline constexpr T Clamp(T v, T lo, T hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    /// Return the sign of a float (-1, 0 or 1.)
    template <typename T>
    inline constexpr T Sign(T v) noexcept { return v < T(0) ? T(-1) : (v > T(0) ? T(1) : T(0)); }

    /// Return absolute value of a value
    template <class T>
    inline constexpr T Abs(T v) noexcept  { return std::abs(v); }

    template<typename T>
    inline constexpr T Saturate(T v) noexcept { return T(Min(T(1), Max(T(0), v))); }

    template<typename T>
    inline constexpr T Mix(T x, T y, T a) noexcept { return x * (T(1) - a) + y * a; }

    /// Linear interpolation between two values.
    template<typename T>
    inline constexpr T Lerp(T x, T y, T a) noexcept { return mix(x, y, a); }

    /// Smoothly damp between values.
    template <class T>
    inline constexpr T SmoothStep(T lhs, T rhs, T t) noexcept
    {
        t = Clamp((t - lhs) / (rhs - lhs), T(0.0), T(1.0)); // Saturate t
        return t * t * (3.0 - 2.0 * t);
    }

    template <typename T> inline T Sin(T v) { return std::sin(v); }
    template <typename T> inline T Cos(T v) { return std::cos(v); }
    template <typename T> inline T Tan(T v) { return std::tan(v); }
    template <typename T> inline T Asin(T v) { return std::asin(v); }
    template <typename T> inline T Acos(T v) { return std::acos(v); }
    template <typename T> inline T Atan(T v) { return std::atan(v); }
    template <typename T> inline T Atan2(T y, T x) { return std::atan2(y, x); }
    template <typename T> inline T Log2(T v) { return std::log2(v); }
    template <typename T> inline T Log10(T v) { return std::log10(v); }
    template <typename T> inline T Log(T v) { return std::log(v); }
    template <typename T> inline T Exp2(T v) { return std::exp2(v); }
    template <typename T> inline T Exp(T v) { return std::exp(v); }

    /// Return X in power Y.
    template <typename T> T inline Pow(T x, T y) { return std::pow(x, y); }

    /// Return square root of X.
    inline float Sqrt(float x) { return std::sqrt(x); }
    inline double Sqrt(double x) { return std::sqrt(x); }

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

    /// Converts from degrees to radians.
    inline constexpr float ToRadians(float degrees) { return degrees * M_DEGTORAD; }

    /// Converts from radians to degrees.
    inline constexpr float ToDegrees(float radians) { return radians * M_RADTODEG; }

    /// Return a representation of the specified floating-point value as a single format bit layout.
    inline uint32_t FloatToRawIntBits(float value)
    {
        uint32_t u = *((uint32_t*)&value);
        return u;
    }

    /// Check whether a floating point value is NaN.
    template <class T> inline bool IsNaN(T value) { return std::isnan(value); }

    /// Check whether an unsigned integer is a power of two.
    template <typename T> ALIMER_FORCE_INLINE bool IsPowerOfTwo(T value)
    {
        return 0 == (value & (value - 1));
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

    /// Update a hash with the given 8-bit value using the SDBM algorithm.
    inline constexpr unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }

    inline uint32_t ScanForward(uint32_t bits)
    {
        ALIMER_VERIFY(bits != 0);
#if defined(_MSC_VER)
        unsigned long firstBitIndex = 0ul;
        uint8_t ret = _BitScanForward(&firstBitIndex, bits);
        ALIMER_VERIFY(ret != 0);
        return firstBitIndex;
#else
        return static_cast<uint32_t>(__builtin_ctz(bits));
#endif
    }

    inline uint32_t AlignTo(uint32_t value, uint32_t alignment)
    {
        ALIMER_ASSERT(alignment > 0);
        return ((value + alignment - 1) / alignment) * alignment;
    }

    inline uint64_t AlignTo(uint64_t value, uint64_t alignment)
    {
        ALIMER_ASSERT(alignment > 0);
        return ((value + alignment - 1) / alignment) * alignment;
    }

    template <typename T> ALIMER_FORCE_INLINE T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }

    ALIMER_API bool IsZero(float value);
    ALIMER_API bool IsOne(float value);

    /// Determines if two scalars are nearly equal with given epsilon value
    ALIMER_API bool ScalarNearEqual(float scalar1, float scalar2, float epsilon = std::numeric_limits<float>::epsilon());

    /// Calculate both sine and cosine, with angle in radians.
    ALIMER_API void ScalarSinCos(float angle, float* sin, float* cos);


#ifdef __GNUC__
#	define leading_zeroes(x) ((x) == 0 ? 32 : __builtin_clz(x))
#	define trailing_zeroes(x) ((x) == 0 ? 32 : __builtin_ctz(x))
#	define trailing_ones(x) __builtin_ctz(~(x))
#elif defined(_MSC_VER)
    namespace Internal
    {
        static inline uint32_t clz(uint32_t x)
        {
            unsigned long result;
            if (_BitScanReverse(&result, x))
                return 31 - result;
            else
                return 32;
        }

        static inline uint32_t ctz(uint32_t x)
        {
            unsigned long result;
            if (_BitScanForward(&result, x))
                return result;
            else
                return 32;
        }
    }

#define leading_zeroes(x) ::Alimer::Internal::clz(x)
#define trailing_zeroes(x) ::Alimer::Internal::ctz(x)
#define trailing_ones(x) ::Alimer::Internal::ctz(~(x))
#endif

    template<typename T>
    inline void ForEachBit(uint32_t value, const T &func)
    {
        while (value)
        {
            uint32_t bit = trailing_zeroes(value);
            func(bit);
            value &= ~(1u << bit);
        }
    }

    template<typename T>
    inline void ForEachBitRange(uint32_t value, const T &func)
    {
        while (value)
        {
            uint32_t bit = trailing_zeroes(value);
            uint32_t range = trailing_ones(value >> bit);
            func(bit, range);
            value &= ~((1u << (bit + range)) - 1);
        }
    }

    //void ComputeTransform(glm::vec3 translation, glm::quat rotation, glm::vec3 scale, glm::mat4 &world, const glm::mat4 &parent);
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif
