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

#include "foundation/ArrayView.h"
#include "foundation/Assert.h"

namespace alimer
{
    /// Fixed size array.
    template<typename T, uint32_t N = 1>
    class Array
    {
    public:
        using size_type = uint32_t;
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator = value_type * ;
        using const_iterator = const value_type*;

        operator ArrayView<T>() { return ArrayView<T>(data_, N); }
        operator ArrayView<const T>() const { return ArrayView<const T>(data_, N); }

        constexpr reference operator[](size_type i)
        {
            ALIMER_VERIFY(i >= 0 && i < N);
            return data_[i];
        }

        constexpr const_reference operator[](size_type i) const
        {
            ALIMER_VERIFY(index >= 0 && index < N);
            return data_[index];
        }

        constexpr const_reference at(size_type i) const
        {
            ALIMER_VERIFY(index >= 0 && index < N);
            return data_[index];
        }

        constexpr reference at(size_type i)
        {
            ALIMER_VERIFY(index >= 0 && index < N);
            return data_[index];
        }

        void fill(const value_type& Val)
        {
            for (size_type i = 0; i < N; ++i) {
                data_[i] = Val;
            }
        }

        constexpr reference front() { return data_[0]; }
        constexpr const_reference front() const { return data_[0]; }
        constexpr reference back() { return data_[N - 1]; }
        constexpr const_reference back() const { return data_[N - 1]; }

        constexpr iterator begin() noexcept { return &data_[0]; }
        constexpr const_iterator begin() const noexcept { return &data_[0]; }
        constexpr const_iterator cbegin() const noexcept { return &data_[0]; }

        constexpr iterator end() noexcept { return &data_[N]; }
        constexpr const_iterator end() const noexcept { return &data_[N]; }
        constexpr const_iterator cend() const noexcept { return &data_[N]; }

        constexpr T* data() noexcept { return &data_[0]; }
        constexpr const T* data() const noexcept { return &data_[0]; }

        constexpr bool empty() const noexcept { return N == 0; }
        constexpr size_type size() const noexcept { return (size_type)N; }
        constexpr size_type max_size() const noexcept { return (size_type)N; }

        value_type data_[N ? N : 1];
    };
}

