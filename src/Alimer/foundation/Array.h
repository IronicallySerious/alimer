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

#include "../foundation/ArrayView.h"
#include "../Core/Debug.h"
#include <type_traits>

namespace Alimer
{
    /// Fixed size array.
    template<typename T, size_t SIZE>
    class Array
    {
    public:
        using ConstT = const typename std::remove_const<T>::type;

        operator ArrayView<T>() { return ArrayView<T>(data_, SIZE); }
        operator ArrayView<const T>() const { return ArrayView<const T>(data_, SIZE); }

        T& operator[](size_t index)
        {
            ALIMER_ASSERT(index >= 0 && index < SIZE);
            return data_[index];
        }

        ConstT &operator[](size_t index) const
        {
            ALIMER_ASSERT(index >= 0 && index < SIZE);
            return data_[index];
        }

        void fill(const T& value)
        {
            for (size_t index = 0; index < SIZE; ++index) {
                data_[index] = value;
            }
        }

        T& front() { return data_[0]; }
        const T& front() const { return data_[0]; }
        T& back() { return data_[SIZE - 1]; }
        const T& back() const { return data_[SIZE - 1]; }

        T *begin() { return data_; }
        ConstT *begin() const { return data_; }
        T *end() { return data_ + SIZE; }
        ConstT *end() const { return data_ + SIZE; }

        T* data() { return &data_[0]; }
        const T* data() const { return &data_[0]; }
        size_t size() const { return SIZE; }

        T data_[SIZE];
    };
} 
