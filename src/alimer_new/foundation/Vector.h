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
    class Vector
    {
        struct CopyTag {};
        struct MoveTag {};
    public:
        using ValueType = T;
        using Iterator = ValueType * ;
        using ConstIterator = const ValueType*;

        /// Construct empty.
        Vector(const ALLOCATOR& allocator = ALLOCATOR()) noexcept
            : allocator_(allocator)
        {

        }

        /// Construct with initial size.
        explicit Vector(uint32_t size, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            Resize(size);
        }

        /// Construct with initial size and default value.
        Vector(uint32_t size, const T& value, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            Resize(size);
            for (uint32_t i = 0; i < size; ++i)
            {
                At(i) = value;
            }
        }

        /// Construct with initial data.
        Vector(const T* data, uint32_t size, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            DoInsertElements(0, data, data + size, CopyTag{});
        }

        /// Copy-construct from another vector.
        Vector(const Vector<T>& other)
            : allocator_(other.allocator_)
        {
            DoInsertElements(0, other.Begin(), other.End(), CopyTag{});
        }

        /// Move-construct from another vector.
        Vector(Vector<T> && vector)
        {
            Swap(vector);
        }

        /// Aggregate initialization constructor.
        Vector(const std::initializer_list<T>& list, const ALLOCATOR& allocator = ALLOCATOR())
            : allocator_(allocator)
        {
            for (auto it = list.begin(); it != list.end(); it++)
            {
                Push(*it);
            }
        }

        /// Destruct.
        ~Vector()
        {
            DestructElements(data_, size_);
            allocator_.Deallocate(data_);
        }

        /// Swap with another vector.
        void Swap(Vector& other)
        {
            std::swap(allocator_, other.allocator_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            std::swap(data_, other.data_);
        }

        /// Assign from another vector.
        Vector<T>& operator =(const Vector<T>& rhs)
        {
            // In case of self-assignment do nothing
            if (&rhs != this)
            {
                Vector<T> copy(rhs);
                Swap(copy);
            }
            return *this;
        }

        /// Move-assign from another vector.
        Vector<T>& operator =(Vector<T> && rhs)
        {
            ALIMER_VERIFY(&rhs != this);
            Swap(rhs);
            return *this;
        }

        /// Add-assign an element.
        Vector<T>& operator +=(const T& rhs)
        {
            Push(rhs);
            return *this;
        }

        /// Add-assign another vector.
        Vector<T>& operator +=(const Vector<T>& rhs)
        {
            Push(rhs);
            return *this;
        }

        /// Add an element.
        Vector<T> operator +(const T& rhs) const
        {
            Vector<T> ret(*this);
            ret.Push(rhs);
            return ret;
        }

        /// Add another vector.
        Vector<T> operator +(const Vector<T>& rhs) const
        {
            Vector<T> ret(*this);
            ret.Push(rhs);
            return ret;
        }

        /// Test for equality with another vector.
        bool operator ==(const Vector<T>& rhs) const
        {
            if (rhs.size_ != size_)
                return false;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (unsigned i = 0; i < size_; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
                    return false;
            }

            return true;
        }

        /// Test for inequality with another vector.
        bool operator !=(const Vector<T>& rhs) const
        {
            if (rhs.size_ != size_)
                return true;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (uint32_t i = 0; i < size_; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
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

        /// Create an element at the end.
        template <class... Args> T& EmplaceBack(Args&&... args)
        {
            if (size_ < capacity_)
            {
                // Optimize common case
                ++size_;
                new (&Back()) T(std::forward<Args>(args)...);
            }
            else
            {
                T value(std::forward<Args>(args)...);
                Push(std::move(value));
            }
            return Back();
        }

        /// Add an element at the end.
#ifndef COVERITY_SCAN_MODEL
        void Push(const T& value)
        {
            if (size_ < capacity_)
            {
                // Optimize common case
                ++size_;
                new (&Back()) T(value);
            }
            else
                DoInsertElements(size_, &value, &value + 1, CopyTag{});
        }

        /// Move-add an element at the end.
        void Push(T && value)
        {
            if (size_ < capacity_)
            {
                // Optimize common case
                ++size_;
                new (&Back()) T(std::move(value));
            }
            else
                DoInsertElements(size_, &value, &value + 1, MoveTag{});
        }
#else
    // FIXME: Attempt had been made to use this model in the Coverity-Scan model file without any success
    // Probably because the model had generated a different mangled name than the one used by static analyzer
        void Push(const T& value)
        {
            T array[] = { value };
            DoInsertElements(size_, array, array + 1, CopyTag{});
        }
#endif

        /// Add another vector at the end.
        void Push(const Vector<T>& vector) { DoInsertElements(size_, vector.Begin(), vector.End(), CopyTag{}); }

        /// Remove the last element.
        void Pop()
        {
            if (size_)
                Resize(size_ - 1);
        }

        /// Insert an element at position.
        void Insert(unsigned pos, const T& value)
        {
            DoInsertElements(pos, &value, &value + 1, CopyTag{});
        }

        /// Insert an element at position.
        void Insert(unsigned pos, T && value)
        {
            DoInsertElements(pos, &value, &value + 1, MoveTag{});
        }

        /// Insert another vector at position.
        void Insert(unsigned pos, const Vector<T>& vector)
        {
            DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
        }

        /// Insert an element by iterator.
        Iterator Insert(const Iterator& dest, const T& value)
        {
            auto pos = (unsigned)(dest - Begin());
            return DoInsertElements(pos, &value, &value + 1, CopyTag{});
        }

        /// Move-insert an element by iterator.
        Iterator Insert(const Iterator& dest, T && value)
        {
            auto pos = (unsigned)(dest - Begin());
            return DoInsertElements(pos, &value, &value + 1, MoveTag{});
        }

        /// Insert a vector by iterator.
        Iterator Insert(const Iterator& dest, const Vector<T>& vector)
        {
            auto pos = (unsigned)(dest - Begin());
            return DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
        }

        /// Insert a vector partially by iterators.
        Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
        {
            auto pos = (unsigned)(dest - Begin());
            return DoInsertElements(pos, start, end, CopyTag{});
        }

        /// Insert elements.
        Iterator Insert(const Iterator& dest, const T* start, const T* end)
        {
            auto pos = (unsigned)(dest - Begin());
            return DoInsertElements(pos, start, end, CopyTag{});
        }

        /// Erase a range of elements.
        void Erase(unsigned pos, unsigned length = 1)
        {
            // Return if the range is illegal
            if (pos + length > size_ || !length)
                return;

            DoEraseElements(pos, length);
        }

        /// Erase a range of elements by swapping elements from the end of the array.
        void EraseSwap(unsigned pos, unsigned length = 1)
        {
            unsigned shiftStartIndex = pos + length;
            // Return if the range is illegal
            if (shiftStartIndex > size_ || !length)
                return;

            unsigned newSize = size_ - length;
            unsigned trailingCount = size_ - shiftStartIndex;
            if (trailingCount <= length)
            {
                // We're removing more elements from the array than exist past the end of the range being removed, so perform a normal shift and destroy
                DoEraseElements(pos, length);
            }
            else
            {
                // Swap elements from the end of the array into the empty space
                T* buffer = Buffer();
                std::move(buffer + newSize, buffer + size_, buffer + pos);
                Resize(newSize);
            }
        }

        /// Erase an element by iterator. Return iterator to the next element.
        Iterator Erase(const Iterator& it)
        {
            auto pos = (unsigned)(it - Begin());
            if (pos >= size_)
                return End();
            Erase(pos);

            return Begin() + pos;
        }

        /// Erase a range by iterators. Return iterator to the next element.
        Iterator Erase(const Iterator& start, const Iterator& end)
        {
            auto pos = (unsigned)(start - Begin());
            if (pos >= size_)
                return End();
            auto length = (unsigned)(end - start);
            Erase(pos, length);

            return Begin() + pos;
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
            else
                return false;
        }

        /// Clear the vector.
        void Clear() { Resize(0); }

        /// Resize the vector.
        void Resize(unsigned newSize) { DoResize(newSize); }

        /// Resize the vector and fill new elements with default value.
        void Resize(unsigned newSize, const T& value)
        {
            unsigned oldSize = Size();
            DoResize(newSize);
            for (unsigned i = oldSize; i < newSize; ++i)
                At(i) = value;
        }

        /// Set new capacity.
        void Reserve(unsigned newCapacity)
        {
            if (newCapacity < size_)
                newCapacity = size_;

            if (newCapacity != capacity_)
            {
                T* newBuffer = nullptr;
                capacity_ = newCapacity;

                if (capacity_)
                {
                    newBuffer = reinterpret_cast<T*>(allocator_.Allocate(capacity_ * sizeof(T), alignof(T)));
                    // Move the data into the new buffer
                    ConstructElements(newBuffer, Begin(), End(), MoveTag{});
                }

                // Delete the old buffer
                DestructElements(data_, size_);
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
            while (it != End() && *it != value)
                ++it;
            return it;
        }

        /// Return const iterator to value, or to the end if not found.
        ConstIterator Find(const T& value) const
        {
            ConstIterator it = Begin();
            while (it != End() && *it != value)
                ++it;
            return it;
        }

        /// Return index of value in vector, or size if not found.
        unsigned IndexOf(const T& value) const
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
        T& Front()
        {
            ALIMER_VERIFY(size_);
            return data_[0];
        }

        /// Return const first element.
        const T& Front() const
        {
            ALIMER_VERIFY(size_);
            return data_[0];
        }

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

        /// Return size of vector.
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
        /// Construct elements using default ctor.
        static void ConstructElements(T* dest, uint32_t count)
        {
            for (uint32_t i = 0; i < count; ++i)
            {
                new(dest + i) T();
            }
        }

        /// Copy-construct elements.
        template <class RandomIteratorT>
        static void ConstructElements(T* dest, RandomIteratorT start, RandomIteratorT end, CopyTag)
        {
            const uint32_t count = (uint32_t)(end - start);
            for (uint32_t i = 0; i < count; ++i)
            {
                new(dest + i) T(*(start + i));
            }
        }

        /// Move-construct elements.
        template <class RandomIteratorT>
        static void ConstructElements(T* dest, RandomIteratorT start, RandomIteratorT end, MoveTag)
        {
            const uint32_t count = (uint32_t)(end - start);
            for (uint32_t i = 0; i < count; ++i)
            {
                new(dest + i) T(std::move(*(start + i)));
            }
        }

        /// Calculate new vector capacity.
        static uint32_t CalculateCapacity(uint32_t size, uint32_t capacity)
        {
            if (!capacity)
                return size;

            while (capacity < size) {
                capacity += (capacity + 1) >> 1;
            }

            return capacity;
        }

        /// Resize the vector and create/remove new elements as necessary.
        void DoResize(uint32_t newSize)
        {
            // If size shrinks, destruct the removed elements
            if (newSize < size_)
            {
                DestructElements(Buffer() + newSize, size_ - newSize);
            }
            else
            {
                // Allocate new buffer if necessary and copy the current elements
                if (newSize > capacity_)
                {
                    T* src = Buffer();

                    // Reallocate vector
                    Vector<T> newVector;
                    newVector.Reserve(CalculateCapacity(newSize, capacity_));
                    newVector.size_ = size_;
                    T* dest = newVector.Buffer();

                    // Move old elements
                    ConstructElements(dest, src, src + size_, MoveTag{});

                    Swap(newVector);
                }

                // Initialize the new elements
                ConstructElements(Buffer() + size_, newSize - size_);
            }

            size_ = newSize;
        }

        /// Insert elements into the vector using copy or move constructor.
        template <class Tag, class RandomIteratorT>
        Iterator DoInsertElements(uint32_t pos, RandomIteratorT start, RandomIteratorT end, Tag)
        {
            if (pos > size_)
                pos = size_;

            const uint32_t numElements = (uint32_t)(end - start);
            if (size_ + numElements > capacity_)
            {
                T* src = data_;

                // Reallocate vector
                Vector<T> newVector;
                newVector.Reserve(CalculateCapacity(size_ + numElements, capacity_));
                newVector.size_ = size_ + numElements;
                T* dest = newVector.data_;

                // Copy or move new elements
                ConstructElements(dest + pos, start, end, Tag{});

                // Move old elements
                if (pos > 0)
                    ConstructElements(dest, src, src + pos, MoveTag{});
                if (pos < size_)
                    ConstructElements(dest + pos + numElements, src + pos, src + size_, MoveTag{});

                Swap(newVector);
            }
            else if (numElements > 0)
            {
                T* buffer = data_;

                // Copy or move new elements
                ConstructElements(buffer + size_, start, end, Tag{});

                // Rotate buffer
                if (pos < size_)
                {
                    std::rotate(buffer + pos, buffer + size_, buffer + size_ + numElements);
                }

                // Update size
                size_ += numElements;
            }

            return Begin() + pos;
        }

        /// Erase elements from the vector.
        Iterator DoEraseElements(uint32_t pos, uint32_t count)
        {
            ALIMER_VERIFY(count > 0);
            ALIMER_VERIFY(pos + count <= size_);
            T* buffer = Buffer();
            std::move(buffer + pos + count, buffer + size_, buffer + pos);
            Resize(size_ - count);
            return Begin() + pos;
        }

        /// Call the elements' destructors.
        static void DestructElements(T* dest, uint32_t count)
        {
            while (count--)
            {
                dest->~T();
                ++dest;
            }
        }

        /// Buffer.
        T* data_ = nullptr;
        /// Size of vector.
        uint32_t size_ = 0;
        /// Buffer capacity.
        uint32_t capacity_ = 0;
        /// Allocator
        ALLOCATOR allocator_;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
