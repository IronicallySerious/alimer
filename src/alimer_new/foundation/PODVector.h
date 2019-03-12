//
// Copyright (c) 2008-2019 the Urho3D project.
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

#include "foundation/Allocator.h"
#include "foundation/Assert.h"
#include <initializer_list>
#include <algorithm>
#include <utility>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:6293)
#endif

namespace alimer
{
    /// %Vector template class for POD types. Does not call constructors or destructors and uses block move. Is intentionally (for performance reasons) unsafe for self-insertion.
    template <class T, typename ALLOCATOR = ContainerAllocator>
    class PODVector
    {
    public:
        using ValueType = T;
        using Iterator = ValueType*;
        using ConstIterator = const ValueType*;

        /// Construct empty.
        PODVector(const ALLOCATOR& allocator = ALLOCATOR()) noexcept
            : allocator_(allocator)
        {
        }

        /// Construct with initial size.
        explicit PODVector(uint32_t size, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            Resize(size);
        }

        /// Construct with initial size and default value.
        PODVector(uint32_t size, const T& value, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            Resize(size);
            for (uint32_t i = 0; i < size; ++i) {
                At(i) = value;
            }
        }

        /// Construct with initial data.
        PODVector(const T* data, uint32_t size, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            Resize(size);
            CopyElements(data_, data, size);
        }

        /// Construct from another vector.
        PODVector(const PODVector<T>& other)
            : allocator_(other.allocator_)
        {
            *this = other;
        }

        /// Aggregate initialization constructor.
        PODVector(const std::initializer_list<T>& list, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            for (auto it = list.begin(); it != list.end(); it++)
            {
                Push(*it);
            }
        }
        /// Destruct.
        ~PODVector()
        {
            allocator_.Deallocate(data_);
        }

        /// Swap with another vector.
        void Swap(PODVector& other)
        {
            std::swap(allocator_, other.allocator_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            std::swap(data_, other.data_);
        }

        /// Assign from another vector.
        PODVector<T>& operator =(const PODVector<T>& other)
        {
            // In case of self-assignment do nothing
            if (&other != this)
            {
                allocator_ = other.allocator_;
                Resize(other.size_);
                CopyElements(data_, other.data_, other.size_);
            }

            return *this;
        }

        /// Add-assign an element.
        PODVector<T>& operator +=(const T& rhs)
        {
            Push(rhs);
            return *this;
        }

        /// Add-assign another vector.
        PODVector<T>& operator +=(const PODVector<T>& rhs)
        {
            Push(rhs);
            return *this;
        }

        /// Add an element.
        PODVector<T> operator +(const T& rhs) const
        {
            PODVector<T> ret(*this);
            ret.Push(rhs);
            return ret;
        }

        /// Add another vector.
        PODVector<T> operator +(const PODVector<T>& rhs) const
        {
            PODVector<T> ret(*this);
            ret.Push(rhs);
            return ret;
        }

        /// Test for equality with another vector.
        bool operator ==(const PODVector<T>& rhs) const
        {
            if (rhs.size_ != size_)
                return false;

            for (uint32_t i = 0; i < size_; ++i)
            {
                if (data_[i] != rhs.data_[i])
                    return false;
            }

            return true;
        }

        /// Test for inequality with another vector.
        bool operator !=(const PODVector<T>& rhs) const
        {
            if (rhs.size_ != size_)
                return true;

            for (uint32_t i = 0; i < size_; ++i)
            {
                if (data_[i] != rhs.data_[i])
                    return true;
            }

            return false;
        }

        /// Return element at index.
        T& operator [](uint32_t index)
        {
            ALIMER_VERIFY(index < size_);
            return data_[index];
        }

        /// Return const element at index.
        const T& operator [](uint32_t index) const
        {
            ALIMER_VERIFY(index < size_);
            return data_[index];
        }

        /// Return element at index.
        T& At(uint32_t index)
        {
            ALIMER_VERIFY(index < size_);
            return data_[index];
        }

        /// Return const element at index.
        const T& At(uint32_t index) const
        {
            ALIMER_VERIFY(index < size_);
            return data_[index];
        }

        /// Add an element at the end.
        void Push(const T& value)
        {
            if (size_ < capacity_) {
                ++size_;
            }
            else {
                Resize(size_ + 1);
            }
            Back() = value;
        }

        /// Add another vector at the end.
        void Push(const PODVector<T>& vector)
        {
            uint32_t oldSize = size_;
            Resize(size_ + vector.size_);
            CopyElements(data_ + oldSize, vector.data_, vector.size_);
        }

        /// Remove the last element.
        void Pop()
        {
            if (size_)
                Resize(size_ - 1);
        }

        /// Insert an element at position.
        void Insert(uint32_t pos, const T& value)
        {
            if (pos > size_) {
                pos = size_;
            }

            uint32_t oldSize = size_;
            Resize(size_ + 1);
            MoveRange(pos + 1, pos, oldSize - pos);
            data_[pos] = value;
        }

        /// Insert another vector at position.
        void Insert(uint32_t pos, const PODVector<T>& vector)
        {
            if (pos > size_)
                pos = size_;

            uint32_t oldSize = size_;
            Resize(size_ + vector.size_);
            MoveRange(pos + vector.size_, pos, oldSize - pos);
            CopyElements(data_ + pos, vector.data_, vector.size_);
        }

        /// Insert an element by iterator.
        Iterator Insert(const Iterator& dest, const T& value)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > size_)
                pos = size_;
            Insert(pos, value);

            return Begin() + pos;
        }

        /// Insert a vector by iterator.
        Iterator Insert(const Iterator& dest, const PODVector<T>& vector)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > size_)
                pos = size_;
            Insert(pos, vector);

            return Begin() + pos;
        }

        /// Insert a vector partially by iterators.
        Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > size_)
                pos = size_;
            auto length = (uint32_t)(end - start);
            Resize(size_ + length);
            MoveRange(pos + length, pos, size_ - pos - length);
            CopyElements(data_ + pos, &(*start), length);

            return Begin() + pos;
        }

        /// Insert elements.
        Iterator Insert(const Iterator& dest, const T* start, const T* end)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > size_)
                pos = size_;
            auto length = (uint32_t)(end - start);
            Resize(size_ + length);
            MoveRange(pos + length, pos, size_ - pos - length);

            T* destPtr = data_ + pos;
            for (const T* i = start; i != end; ++i)
                *destPtr++ = *i;

            return Begin() + pos;
        }

        /// Erase a range of elements.
        void Erase(uint32_t pos, uint32_t length = 1)
        {
            // Return if the range is illegal
            if (!length || pos + length > size_)
                return;

            MoveRange(pos, pos + length, size_ - pos - length);
            Resize(size_ - length);
        }

        /// Erase an element by iterator. Return iterator to the next element.
        Iterator Erase(const Iterator& it)
        {
            auto pos = (uint32_t)(it - Begin());
            if (pos >= size_)
                return End();
            Erase(pos);

            return Begin() + pos;
        }

        /// Erase a range by iterators. Return iterator to the next element.
        Iterator Erase(const Iterator& start, const Iterator& end)
        {
            auto pos = (uint32_t)(start - Begin());
            if (pos >= size_)
                return End();
            auto length = (uint32_t)(end - start);
            Erase(pos, length);

            return Begin() + pos;
        }

        /// Erase a range of elements by swapping elements from the end of the array.
        void EraseSwap(uint32_t pos, uint32_t length = 1)
        {
            uint32_t shiftStartIndex = pos + length;
            // Return if the range is illegal
            if (shiftStartIndex > size_ || !length)
                return;

            uint32_t newSize = size_ - length;
            uint32_t trailingCount = size_ - shiftStartIndex;
            if (trailingCount <= length)
            {
                // We're removing more elements from the array than exist past the end of the range being removed, so perform a normal shift and destroy
                MoveRange(pos, shiftStartIndex, trailingCount);
            }
            else
            {
                // Swap elements from the end of the array into the empty space
                CopyElements(data_ + pos, data_ + newSize, length);
            }
            Resize(newSize);
        }

        /// Erase an element by value. Return true if was found and erased.
        bool Remove(const T& value)
        {
            Iterator i = Find(value);
            if (i != End())
            {
                Erase(i);
                return true;
            }
            else
                return false;
        }

        /// Erase an element by value by swapping with the last element. Return true if was found and erased.
        bool RemoveSwap(const T& value)
        {
            Iterator i = Find(value);
            if (i != End())
            {
                EraseSwap(i - Begin());
                return true;
            }
            return false;
        }

        /// Clear the vector.
        void Clear() { Resize(0); }

        /// Resize the vector.
        void Resize(uint32_t newSize)
        {
            if (newSize > capacity_)
            {
                if (!capacity_)
                    capacity_ = newSize;
                else
                {
                    while (capacity_ < newSize)
                        capacity_ += (capacity_ + 1) >> 1;
                }

                T* newBuffer = static_cast<T*>(allocator_.Allocate(capacity_ * sizeof(T), alignof(T)));
                // Move the data into the new buffer and delete the old
                if (data_)
                {
                    CopyElements(newBuffer, data_, size_);
                    allocator_.Deallocate(data_);
                }
                data_ = newBuffer;
            }

            size_ = newSize;
        }

        /// Set new capacity.
        void Reserve(uint32_t newCapacity)
        {
            if (newCapacity < size_)
                newCapacity = size_;

            if (newCapacity != capacity_)
            {
                T* newBuffer = nullptr;
                capacity_ = newCapacity;

                if (capacity_)
                {
                    newBuffer = static_cast<T*>(allocator_.Allocate(newCapacity * sizeof(T), alignof(T)));
                    // Move the data into the new buffer
                    CopyElements(newBuffer, data_, size_);
                }

                // Delete the old buffer
                allocator_.Deallocate(data_);
                data_ = newBuffer;
            }
        }

        /// Reallocate so that no extra memory is used.
        void Compact() { Reserve(size_); }

        /// Return iterator to value, or to the end if not found.
        Iterator Find(const T& value)
        {
            Iterator it = Begin();
            while (it != End() && *it != value) {
                ++it;
            }
            return it;
        }

        /// Return const iterator to value, or to the end if not found.
        ConstIterator Find(const T& value) const
        {
            ConstIterator it = Begin();
            while (it != End() && *it != value) {
                ++it;
            }
            return it;
        }

        /// Return index of value in vector, or size if not found.
        uint32_t IndexOf(const T& value) const
        {
            return Find(value) - Begin();
        }

        /// Return whether contains a specific value.
        bool Contains(const T& value) const { return Find(value) != End(); }

        /// Return iterator to the beginning.
        Iterator Begin() noexcept { return data_; }

        /// Return const iterator to the beginning.
        ConstIterator Begin() const noexcept { return data_; }

        /// Return iterator to the end.
        Iterator End() noexcept { return data_ + size_; }

        /// Return const iterator to the end.
        ConstIterator End() const noexcept { return data_ + size_; }

        /// Return first element.
        T& Front() { return data_[0]; }

        /// Return const first element.
        const T& Front() const { return data_[0]; }

        /// Return last element.
        T& Back()
        {
            ALIMER_VERIFY(size_);
            return data_[size_ - 1];
        }

        /// Return const last element.
        const T& Back() const
        {
            ALIMER_VERIFY(size_);
            return data_[size_ - 1];
        }

        /// Return number of elements.
        uint32_t Size() const { return size_; }

        /// Return capacity of vector.
        uint32_t Capacity() const { return capacity_; }

        /// Return whether vector is empty.
        bool Empty() const { return size_ == 0; }

        T* Data() noexcept { return &data_[0]; }
        const T* Data() const noexcept { return &data_[0]; }

        // STL compatible
        Iterator begin() noexcept { return data_; }
        ConstIterator begin() const noexcept { return data_; }
        Iterator end() noexcept { return data_ + size_; }
        ConstIterator end() const noexcept { return data_ + size_; }

        T* data() noexcept { return &data_[0]; }
        const T* data() const noexcept { return &data_[0]; }
        uint32_t size() const noexcept { return size_; }
        uint32_t capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return size_ == 0; }


    private:
        /// Move a range of elements within the vector.
        void MoveRange(uint32_t dest, uint32_t src, uint32_t count)
        {
            if (count) {
                memmove(data_ + dest, data_ + src, count * sizeof(T));
            }
        }

        /// Copy elements from one buffer to another.
        static void CopyElements(T* dest, const T* src, uint32_t count)
        {
            if (count)
                memcpy(dest, src, count * sizeof(T));
        }

        /// Data.
        T* data_ = nullptr;
        /// Size of vector.
        uint32_t size_ = 0;
        /// Data capacity.
        uint32_t capacity_ = 0;
        /// Allocator
        ALLOCATOR allocator_;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
