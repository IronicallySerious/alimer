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

namespace Alimer
{
    template<typename T>
    class ArrayView
    {
    public:
        using IndexType = uint32_t;
        using ValueType = T;
        using Iterator = ValueType*;
        using ConstIterator = const ValueType*;

        ArrayView()
            : _begin(nullptr)
            , _end(nullptr)
        {
        }

        ArrayView(const ArrayView& other)
            : _begin(other._begin)
            , _end(other._end)
        {
        }

        template<typename U>
        ArrayView(const ArrayView<U>& other)
            : _begin(other.begin())
            , _end(other.end())
        {
        }

        ArrayView(ValueType& value)
            : _begin(&value)
            , _end(_begin + 1)
        {
            ALIMER_ASSERT(_begin == nullptr && _end == nullptr || (_begin != nullptr && _end != nullptr));
        }

        ArrayView(ValueType* begin, ValueType* end)
            : _begin(begin)
            , _end(end)
        {
            ALIMER_ASSERT(_begin == nullptr && _end == nullptr || (_begin != nullptr && _end != nullptr));
        }

        ArrayView(ValueType* data, IndexType size)
            : _begin(data)
            , _end(data + size)
        {
        }

        template<uint32_t SIZE>
        ArrayView(ValueType(&arr)[SIZE])
            : _begin(&arr[0])
            , _end(&arr[0] + SIZE)

        {
        }

        ~ArrayView() = default;

        Iterator begin() { return _begin; }
        Iterator end() { return _end; }
        ConstIterator begin() const { return _begin; }
        ConstIterator end() const { return _end; }

        IndexType size() const { return (IndexType)(_end - _begin); }
        ValueType* data() const { return _begin; }

        explicit operator bool() const { return !!_begin; }

        ValueType& operator[](IndexType index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size(), "Index out of bounds. (%u, size %u)", index, size());
            return _begin[index];
        }

        const ValueType& operator[](IndexType index) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size(), "Index out of bounds. (%u, size %u)", index, size());
            return _begin[index];
        }

    private:
        ValueType* _begin;
        ValueType* _end;
    };

    /**
     * Fixed size array.
    */
    template<typename T, uint32_t N = 1>
    class Array
    {
    public:
        using IndexType = uint32_t;
        using ValueType = T;
        using Iterator = ValueType*;
        using ConstIterator = const ValueType*;

        void fill(const T& value)
        {
            for (uint32_t index = 0; index < N; ++index)
            {
                _data[index] = value;
            }
        }

        inline constexpr T const& operator[](size_t i) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (%u, size %u)", index, N);
            return _data[index];
        }

        inline constexpr T& operator[](uint32_t index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (index %u, size %u)", index, N);
            return _data[index];
        }

        inline constexpr T const& at(size_t i) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (%u, size %u)", index, N);
            return _data[index];
        }

        inline constexpr T& at(uint32_t index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (index %u, size %u)", index, N);
            return _data[index];
        }

        operator ArrayView<T>() { return ArrayView<T>(_data, N); }
        operator ArrayView<const T>() const { return ArrayView<const T>(_data, N); }

        inline constexpr T& front() { return _data[0]; }
        inline constexpr const T& front() const { return _data[0]; }
        inline constexpr T& back() { return _data[N - 1]; }
        inline constexpr const T& back() const { return _data[N - 1]; }

        inline constexpr Iterator begin() noexcept { return &_data[0]; }
        inline constexpr ConstIterator begin() const noexcept { return &_data[0]; }
        inline constexpr ConstIterator cbegin() const noexcept { return &_data[0]; }

        inline constexpr Iterator end() noexcept { return &_data[N]; }
        inline constexpr ConstIterator end() const noexcept { return &_data[N]; }
        inline constexpr ConstIterator cend() const noexcept { return &_data[N]; }

        inline constexpr bool empty() const noexcept { return (N == 0); }
        inline constexpr uint32_t size() const noexcept { return N; }
        inline constexpr uint32_t max_size() const noexcept { return N; }

        inline constexpr T* data() noexcept { return &_data[0]; }
        inline constexpr const T* data() const noexcept { return &_data[0]; }

    private:
        ValueType _data[N ? N : 1];
    };
}
