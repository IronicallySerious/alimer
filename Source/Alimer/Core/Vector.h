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

#include "../PlatformDef.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <initializer_list>
#include <new>
#include <utility>

namespace Alimer
{
    template <typename T>
    class Vector
	{
    public:
        using IndexType = uint32_t;
        using Iterator = T*;
        using ConstIterator = const Iterator*;
        static const IndexType MIN_SIZE = 16;

        Vector() = default;
        Vector(const Vector& other)
        {
            InternalResize(other.size_);
            size_ = other.size_;
            for (IndexType idx = 0; idx < other.size_; ++idx)
            {
                new(data_ + idx) T(other.data_[idx]);
            }
        }

        Vector(Vector&& other) { swap(other); }
        Vector(IndexType size, const T& default = T()) { resize(size, default); }
        /// Aggregate initialization constructor.
        Vector(const std::initializer_list<T>& list) : Vector()
        {
            for (auto it = list.begin(); it != list.end(); it++)
            {
                push_back(*it);
            }
        }

        ~Vector() { InternalResize(0); }

        Vector& operator=(const Vector& other)
        {
            if (other.size_ != size_)
                InternalResize(other.size_);

            // destruct
            Destruct(data_, data_ + size_);

            // reconstruct
            size_ = other.size_;
            for (IndexType idx = 0; idx < other.size_; ++idx)
            {
                new(data_ + idx) T(other.data_[idx]);
            }

            return *this;
        }

        Vector& operator=(Vector&& other)
        {
            swap(other);
            return *this;
        }

        void swap(Vector& other)
        {
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
        }

        Iterator push_back(const T& value)
        {
            if (capacity_ < (size_ + 1))
                InternalResize(GetGrowCapacity(capacity_));
            new(data_ + size_) T(value);
            return (data_ + size_++);
        }

        Iterator push_back(T&& value)
        {
            if (capacity_ < (size_ + 1))
                InternalResize(GetGrowCapacity(capacity_));
            new(data_ + size_) T(std::forward<T>(value));
            return (data_ + size_++);
        }

        template<class... ValueType>
        Iterator emplace_back(ValueType&&... value)
        {
            if (capacity_ < (size_ + 1))
                InternalResize(GetGrowCapacity(capacity_));
            new(data_ + size_) T(std::forward<ValueType>(value)...);
            return (data_ + size_++);
        }

        T* data() noexcept { return &data_[0]; }
        const T* data() const noexcept { return &data_[0]; }
        IndexType size() const noexcept { return size_; }
        IndexType capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return size_ == 0; }

        T& operator[](IndexType index)
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size_, "Index out of bounds");
            return data_[index];
        }

        const T& operator[](IndexType index) const
        {
            ALIMER_ASSERT_MSG(index >= 0 && index < size_, "Index out of bounds");
            return data_[index];
        }

    private:
        static uint8_t* AllocateBuffer(IndexType size)
        {
            return new uint8_t[size];
        }

        static void FreeBuffer(const void* buffer)
        {
            delete[] buffer;
        }

        static IndexType GetGrowCapacity(IndexType currCapacity)
        {
            return currCapacity >= MIN_SIZE ? (currCapacity + (currCapacity / 2)) : MIN_SIZE;
        }

        static Iterator UnitializedMove(Iterator first, Iterator last, Iterator dest)
        {
            for (; first != last; ++dest, ++first)
            {
                new(dest) T(std::move(*first));
            }

            return dest;
        }

        static void Destruct(Iterator first) { first->~T(); }

        static void Destruct(Iterator first, Iterator last)
        {
            for (; first != last; ++first)
                Destruct(first);
        }

        void InternalResize(IndexType newCapacity)
        {
            IndexType copySize = newCapacity < size_ ? newCapacity : size_;
            T* newData = nullptr;
            if (newCapacity > 0)
            {
                newData = reinterpret_cast<T*>(AllocateBuffer(newCapacity * sizeof(T)));
                ALIMER_ASSERT_MSG(newData, "Unable to allocate for resize.");
                UnitializedMove(data_, data_ + copySize, newData);
                Destruct(data_, data_ + copySize);
            }

            // Destruct trailing elements.
            if (copySize < size_)
                Destruct(data_ + copySize, data_ + size());

            FreeBuffer(data_);

            data_ = newData;
            size_ = copySize;
            capacity_ = newCapacity;
        }

        T* data_ = nullptr;
        IndexType size_ = 0;
        IndexType capacity_ = 0;
    };

}
