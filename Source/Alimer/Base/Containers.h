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

#include "../Base/Debug.h"

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
            : begin_(nullptr)
            , end_(nullptr)
        {
        }

        ArrayView(const ArrayView& other)
            : begin_(other.begin_)
            , end_(other.end_)
        {
        }

        template<typename U>
        ArrayView(const ArrayView<U>& other)
            : begin_(other.begin())
            , end_(other.end())
        {
        }

        ArrayView(ValueType& value)
            : begin_(&value)
            , end_(begin_ + 1)
        {
            ALIMER_ASSERT(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(ValueType* begin, ValueType* end)
            : begin_(begin)
            , end_(end)
        {
            ALIMER_ASSERT(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(ValueType* data, IndexType size)
            : begin_(data)
            , end_(data + size)
        {
        }

        template<uint32_t SIZE>
        ArrayView(ValueType(&arr)[SIZE])
            : begin_(&arr[0])
            , end_(&arr[0] + SIZE)

        {
        }

        ~ArrayView() = default;

        Iterator begin() { return begin_; }
        Iterator end() { return end_; }
        ConstIterator begin() const { return begin_; }
        ConstIterator end() const { return end_; }

        IndexType size() const { return (IndexType)(end_ - begin_); }
        ValueType* data() const { return begin_; }

        explicit operator bool() const { return !!begin_; }

        ValueType& operator[](IndexType index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size(), "Index out of bounds. (%u, size %u)", index, size());
            return begin_[index];
        }

        const ValueType& operator[](IndexType index) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size(), "Index out of bounds. (%u, size %u)", index, size());
            return begin_[index];
        }

    private:
        ValueType* begin_;
        ValueType* end_;
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
                data_[index] = value;
            }
        }

        inline constexpr T const& operator[](size_t i) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (%u, size %u)", index, N);
            return data_[index];
        }

        inline constexpr T& operator[](uint32_t index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (index %u, size %u)", index, N);
            return data_[index];
        }

        inline constexpr T const& at(size_t i) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (%u, size %u)", index, N);
            return data_[index];
        }

        inline constexpr T& at(uint32_t index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < N, "Index out of bounds. (index %u, size %u)", index, N);
            return data_[index];
        }

        operator ArrayView<T>() { return ArrayView<T>(data_, N); }
        operator ArrayView<const T>() const { return ArrayView<const T>(data_, N); }

        inline constexpr T& front() { return data_[0]; }
        inline constexpr const T& front() const { return data_[0]; }
        inline constexpr T& back() { return data_[N - 1]; }
        inline constexpr const T& back() const { return data_[N - 1]; }

        inline constexpr Iterator begin() noexcept { return &data_[0]; }
        inline constexpr ConstIterator begin() const noexcept { return &data_[0]; }
        inline constexpr ConstIterator cbegin() const noexcept { return &data_[0]; }

        inline constexpr Iterator end() noexcept { return &data_[N]; }
        inline constexpr ConstIterator end() const noexcept { return &data_[N]; }
        inline constexpr ConstIterator cend() const noexcept { return &data_[N]; }

        inline constexpr bool empty() const noexcept { return (N == 0); }
        inline constexpr uint32_t size() const noexcept { return N; }
        inline constexpr uint32_t max_size() const noexcept { return N; }

        inline constexpr T* data() noexcept { return &data_[0]; }
        inline constexpr const T* data() const noexcept { return &data_[0]; }

        ValueType data_[N ? N : 1];
    };

}
