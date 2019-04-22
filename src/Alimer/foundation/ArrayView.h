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

#include "../Core/Debug.h"
#include <type_traits>

namespace Alimer
{
    template<typename T>
    class ArrayView
    {
    public:
        using ConstT = const typename std::remove_const<T>::type;

        ArrayView() = default;

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

        ArrayView(T& value)
            : begin_(&value)
            , end_(begin_ + 1)
        {
            ALIMER_ASSERT(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(T* begin, T* end)
            : begin_(begin)
            , end_(end)
        {
            ALIMER_ASSERT(begin_ == nullptr && end_ == nullptr || (begin_ != nullptr && end_ != nullptr));
        }

        ArrayView(T* data, size_t size)
            : begin_(data)
            , end_(data + size)
        {
        }

        template<size_t SIZE>
        ArrayView(T(&arr)[SIZE])
            : begin_(&arr[0])
            , end_(&arr[0] + SIZE)

        {
        }

        ~ArrayView() = default;

        T *begin() { return begin_; }
        T *end() { return end_; }
        ConstT *begin() const { return begin_; }
        ConstT *end() const { return end_; }
        size_t size() const { return (size_t)(end_ - begin_); }
        T *data() { return begin_; }
        ConstT *data() const { return begin_; }

        explicit operator bool() const { return !!begin_; }

        T &operator[](size_t index)
        {
            ALIMER_ASSERT(index >= 0 && index < size());
            return begin_[index];
        }

        ConstT &operator[](size_t index) const
        {
            ALIMER_ASSERT(index >= 0 && index < size());
            return begin_[index];
        }

        bool empty() const
        {
            return size() == 0;
        }

        void reset()
        {
            begin_ = nullptr;
            end_ = nullptr;
        }

    private:
        T* begin_ = nullptr;
        T* end_ = nullptr;
    };
} 
