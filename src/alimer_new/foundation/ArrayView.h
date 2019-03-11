//
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "foundation/Assert.h"

namespace alimer
{
    template<typename T>
    class ArrayView
    {
    public:
        using size_type = uint32_t;
        using value_type = T;
        using iterator = value_type * ;
        using const_iterator = const value_type*;

        ArrayView() = default;

        ArrayView(const ArrayView& other)
            : begin_(other.begin_)
            , end_(other.end_)
        {
        }

        template<typename OTHER>
        ArrayView(const ArrayView<OTHER>& other)
            : begin_(other.begin())
            , end_(other.end())
        {
        }

        ArrayView(value_type& value)
            : begin_(&value)
            , end_(begin_ + 1)
        {
            ALIMER_VERIFY(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(value_type* begin, value_type* end)
            : begin_(begin)
            , end_(end)
        {
            ALIMER_VERIFY(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(value_type* data, size_type size)
            : begin_(data)
            , end_(data + size)
        {
        }

        template<uint32_t N>
        ArrayView(value_type(&arr)[N])
            : begin_(&arr[0])
            , end_(&arr[0] + N)

        {
        }

        ~ArrayView() = default;

        constexpr iterator begin() noexcept { return begin_; }
        constexpr const_iterator begin() const noexcept { return begin_; }
        constexpr const_iterator cbegin() const noexcept { return begin_; }

        constexpr iterator end() noexcept { return end_; }
        constexpr const_iterator end() const noexcept { return end_; }
        constexpr const_iterator cend() const noexcept { return end_; }

        constexpr bool empty() const noexcept { return size() == 0; }
        constexpr size_type size() const { return (size_type)(end_ - begin_); }

        constexpr T* data() noexcept { return begin_; }
        constexpr const T* data() const noexcept { return begin_; }

        explicit operator bool() const { return !!begin_; }

        value_type& operator[](size_type index)
        {
            ALIMER_VERIFY(index >= 0 && index < size());
            return begin_[index];
        }

        const value_type& operator[](size_type index) const
        {
            ALIMER_VERIFY(index >= 0 && index < size());
            return begin_[index];
        }

    private:
        value_type* begin_ = nullptr;
        value_type* end_ = nullptr;
    };
} // namespace vortice
