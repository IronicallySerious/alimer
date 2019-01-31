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

#include "../foundation/Swap.h"
#include "../Base/Iterator.h"
#include "../Debug/Debug.h"

#include <cstring>
#include <algorithm>
#include <initializer_list>
#include <new>
#include <utility>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:6293)
#endif

namespace alimer
{
    /// Vector base class.
    /** Note that to prevent extra memory use due to vtable pointer, %VectorBase intentionally does not declare a virtual destructor
        and therefore %VectorBase pointers should never be used.
    */
    class ALIMER_API VectorBase
    {
    public:
        /// Construct.
        VectorBase() noexcept = default;

        /// Swap with another vector.
        void Swap(VectorBase& rhs)
        {
            alimer::Swap(_size, rhs._size);
            alimer::Swap(_capacity, rhs._capacity);
            alimer::Swap(_buffer, rhs._buffer);
        }

        /// Return number of elements in the vector.
        uint32_t Size() const { return _size; }
        /// Return element capacity of the vector.
        uint32_t Capacity() const { return _capacity; }
        /// Return whether has no elements.
        bool IsEmpty() const { return _size == 0; }

    protected:
        static uint8_t* AllocateBuffer(size_t size);
        static void FreeBuffer(const uint8_t* buffer);

        /// Size of vector.
        uint32_t _size = 0;
        /// Buffer capacity.
        uint32_t _capacity = 0;
        /// Buffer.
        uint8_t* _buffer = nullptr;
    };

    template<typename T> class Vector : public VectorBase
    {
        struct CopyTag {};
        struct MoveTag {};

    public:
        using IndexType = uint32_t;
        using ValueType = T;
        using Iterator = RandomAccessIterator<T>;
        using ConstIterator = RandomAccessConstIterator<T>;

        Vector() noexcept = default;

        /// Construct with initial size.
        explicit Vector(uint32_t size)
        {
            Resize(size);
        }

        /// Construct with initial size and default value.
        Vector(uint32_t size, const T& value)
        {
            Resize(size);
            for (uint32_t i = 0; i < size; ++i)
            {
                At(i) = value;
            }
        }

        /// Construct with initial data.
        Vector(const T* data, uint32_t size)
        {
            DoInsertElements(0, data, data + size, CopyTag{});
        }

        /// Copy-construct from another vector.
        Vector(const Vector<T>& vector)
        {
            DoInsertElements(0, vector.Begin(), vector.End(), CopyTag{});
        }

        /// Move-construct from another vector.
        Vector(Vector<T> && vector)
        {
            Swap(vector);
        }

        /// Aggregate initialization constructor.
        Vector(const std::initializer_list<T>& list) : Vector()
        {
            for (auto it = list.begin(); it != list.end(); it++)
            {
                Push(*it);
            }
        }

        ~Vector()
        {
            DestructElements(Buffer(), _size);
            FreeBuffer(_buffer);
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
            ALIMER_ASSERT(&rhs != this);
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
            if (rhs._size != _size)
                return false;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (uint32_t i = 0; i < _size; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
                    return false;
            }

            return true;
        }

        /// Test for inequality with another vector.
        bool operator !=(const Vector<T>& rhs) const
        {
            if (rhs._size != _size)
                return true;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (uint32_t i = 0; i < _size; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
                    return true;
            }

            return false;
        }

        void Fill(T value)
        {
            for (IndexType i = 0; i < _size; ++i)
            {
                _buffer[i] = value;
            }
        }

        /// Return element at index.
        T& operator [](IndexType index)
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return const element at index.
        const T& operator [](IndexType index) const
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return element at index.
        T& At(IndexType index)
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return const element at index.
        const T& At(IndexType index) const
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Create an element at the end.
        template <class... Args> T& EmplaceBack(Args&&... args)
        {
            if (_size < _capacity)
            {
                // Optimize common case
                ++_size;
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
        void Push(const T& value)
        {
            if (_size < _capacity)
            {
                // Optimize common case
                ++_size;
                new (&Back()) T(value);
            }
            else {
                DoInsertElements(_size, &value, &value + 1, CopyTag{});
            }
        }

        /// Move-add an element at the end.
        void Push(T && value)
        {
            if (_size < _capacity)
            {
                // Optimize common case
                ++_size;
                new (&Back()) T(std::move(value));
            }
            else
                DoInsertElements(_size, &value, &value + 1, MoveTag{});
        }

        /// Add another vector at the end.
        void Push(const Vector<T>& vector) { DoInsertElements(_size, vector.Begin(), vector.End(), CopyTag{}); }

        /// Remove the last element.
        void Pop()
        {
            if (_size) {
                Resize(_size - 1);
            }
        }

        /// Insert an element at position.
        void Insert(uint32_t pos, const T& value)
        {
            DoInsertElements(pos, &value, &value + 1, CopyTag{});
        }

        /// Insert an element at position.
        void Insert(uint32_t pos, T && value)
        {
            DoInsertElements(pos, &value, &value + 1, MoveTag{});
        }

        /// Insert another vector at position.
        void Insert(uint32_t pos, const Vector<T>& vector)
        {
            DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
        }

        /// Insert an element by iterator.
        Iterator Insert(const Iterator& dest, const T& value)
        {
            auto pos = (uint32_t)(dest - Begin());
            return DoInsertElements(pos, &value, &value + 1, CopyTag{});
        }

        /// Move-insert an element by iterator.
        Iterator Insert(const Iterator& dest, T && value)
        {
            auto pos = (uint32_t)(dest - Begin());
            return DoInsertElements(pos, &value, &value + 1, MoveTag{});
        }

        /// Insert a vector by iterator.
        Iterator Insert(const Iterator& dest, const Vector<T>& vector)
        {
            auto pos = (uint32_t)(dest - Begin());
            return DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
        }

        /// Insert a vector partially by iterators.
        Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
        {
            auto pos = (uint32_t)(dest - Begin());
            return DoInsertElements(pos, start, end, CopyTag{});
        }

        /// Insert elements.
        Iterator Insert(const Iterator& dest, const T* start, const T* end)
        {
            auto pos = (uint32_t)(dest - Begin());
            return DoInsertElements(pos, start, end, CopyTag{});
        }

        /// Erase a range of elements.
        void Erase(uint32_t pos, uint32_t length = 1)
        {
            // Return if the range is illegal
            if (pos + length > _size || !length)
                return;

            DoEraseElements(pos, length);
        }

        /// Erase a range of elements by swapping elements from the end of the array.
        void EraseSwap(uint32_t pos, uint32_t length = 1)
        {
            uint32_t shiftStartIndex = pos + length;
            // Return if the range is illegal
            if (shiftStartIndex > _size || !length)
                return;

            uint32_t newSize = _size - length;
            uint32_t trailingCount = _size - shiftStartIndex;
            if (trailingCount <= length)
            {
                // We're removing more elements from the array than exist past the end of the range being removed, so perform a normal shift and destroy
                DoEraseElements(pos, length);
            }
            else
            {
                // Swap elements from the end of the array into the empty space
                T* buffer = Buffer();
                std::move(buffer + newSize, buffer + _size, buffer + pos);
                Resize(newSize);
            }
        }

        /// Erase an element by iterator. Return iterator to the next element.
        Iterator Erase(const Iterator& it)
        {
            auto pos = (uint32_t)(it - Begin());
            if (pos >= _size)
            {
                return End();
            }

            Erase(pos);

            return Begin() + pos;
        }

        /// Erase a range by iterators. Return iterator to the next element.
        Iterator Erase(const Iterator& start, const Iterator& end)
        {
            auto pos = (uint32_t)(start - Begin());
            if (pos >= _size)
                return End();
            auto length = (uint32_t)(end - start);
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

        void Clear()
        {
            Resize(0);
        }

        /// Resize the vector.
        void Resize(uint32_t newSize) { DoResize(newSize); }

        /// Resize the vector and fill new elements with default value.
        void Resize(uint32_t newSize, const T& value)
        {
            uint32_t oldSize = Size();
            DoResize(newSize);
            for (uint32_t i = oldSize; i < newSize; ++i)
            {
                At(i) = value;
            }
        }

        /// Set new capacity.
        void Reserve(uint32_t newCapacity)
        {
            if (newCapacity < _size)
                newCapacity = _size;

            if (newCapacity != _capacity)
            {
                T* newBuffer = nullptr;
                _capacity = newCapacity;

                if (_capacity)
                {
                    newBuffer = reinterpret_cast<T*>(AllocateBuffer(_capacity * sizeof(T)));
                    // Move the data into the new buffer
                    ConstructElements(newBuffer, Begin(), End(), MoveTag{});
                }

                // Delete the old buffer
                DestructElements(Buffer(), _size);
                FreeBuffer(_buffer);
                _buffer = reinterpret_cast<uint8_t*>(newBuffer);
            }
        }

        /// Reallocate so that no extra memory is used.
        void Compact() { Reserve(_size); }

        /// Return iterator to value, or to the end if not found.
        Iterator Find(const T& value)
        {
            Iterator it = Begin();
            while (it != End() && *it != value)
            {
                ++it;
            }

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
        uint32_t IndexOf(const T& value) const
        {
            return Find(value) - Begin();
        }

        /// Return whether contains a specific value.
        bool Contains(const T& value) const { return Find(value) != End(); }

        /// Return iterator to the beginning.
        Iterator Begin() { return Iterator(Buffer()); }

        /// Return const iterator to the beginning.
        ConstIterator Begin() const { return ConstIterator(Buffer()); }

        /// Return iterator to the end.
        Iterator End() { return Iterator(Buffer() + _size); }

        /// Return const iterator to the end.
        ConstIterator End() const { return ConstIterator(Buffer() + _size); }

        /// Return first element.
        T& Front()
        {
            ALIMER_ASSERT(_size);
            return Buffer()[0];
        }

        /// Return const first element.
        const T& Front() const
        {
            ALIMER_ASSERT(_size);
            return Buffer()[0];
        }

        /// Return last element.
        T& Back()
        {
            ALIMER_ASSERT(_size);
            return Buffer()[_size - 1];
        }

        /// Return const last element.
        const T& Back() const
        {
            ALIMER_ASSERT(_size);
            return Buffer()[_size - 1];
        }
        
        uint64_t ElementSize() const { return sizeof(T);}
        uint64_t MemorySize() const { return _size * sizeof(T); }

        /// Return the buffer with right type.
        T* Buffer() const { return reinterpret_cast<T*>(_buffer); }
        const T* Data() const { return reinterpret_cast<T*>(_buffer); }
        T* Data() { return reinterpret_cast<T*>(_buffer); }

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

            while (capacity < size)
            {
                capacity += (capacity + 1) >> 1;
            }
            return capacity;
        }

        /// Resize the vector and create/remove new elements as necessary.
        void DoResize(uint32_t newSize)
        {
            // If size shrinks, destruct the removed elements
            if (newSize < _size)
            {
                DestructElements(Buffer() + newSize, _size - newSize);
            }
            else
            {
                // Allocate new buffer if necessary and copy the current elements
                if (newSize > _capacity)
                {
                    T* src = Buffer();

                    // Reallocate vector
                    Vector<T> newVector;
                    newVector.Reserve(CalculateCapacity(newSize, _capacity));
                    newVector._size = _size;
                    T* dest = newVector.Buffer();

                    // Move old elements
                    ConstructElements(dest, src, src + _size, MoveTag{});

                    Swap(newVector);
                }

                // Initialize the new elements
                ConstructElements(Buffer() + _size, newSize - _size);
            }

            _size = newSize;
        }

        /// Insert elements into the vector using copy or move constructor.
        template <class Tag, class RandomIteratorT>
        Iterator DoInsertElements(uint32_t pos, RandomIteratorT start, RandomIteratorT end, Tag)
        {
            if (pos > _size)
                pos = _size;

            const uint32_t numElements = (uint32_t)(end - start);
            if (_size + numElements > _capacity)
            {
                T* src = Buffer();

                // Reallocate vector
                Vector<T> newVector;
                newVector.Reserve(CalculateCapacity(_size + numElements, _capacity));
                newVector._size = _size + numElements;
                T* dest = newVector.Buffer();

                // Copy or move new elements
                ConstructElements(dest + pos, start, end, Tag{});

                // Move old elements
                if (pos > 0)
                    ConstructElements(dest, src, src + pos, MoveTag{});
                if (pos < _size)
                    ConstructElements(dest + pos + numElements, src + pos, src + _size, MoveTag{});

                Swap(newVector);
            }
            else if (numElements > 0)
            {
                T* buffer = Buffer();

                // Copy or move new elements
                ConstructElements(buffer + _size, start, end, Tag{});

                // Rotate buffer
                if (pos < _size)
                {
                    std::rotate(buffer + pos, buffer + _size, buffer + _size + numElements);
                }

                // Update size
                _size += numElements;
            }

            return Begin() + pos;
        }

        /// Erase elements from the vector.
        Iterator DoEraseElements(uint32_t pos, uint32_t count)
        {
            ALIMER_ASSERT(count > 0);
            ALIMER_ASSERT(pos + count <= _size);
            T* buffer = Buffer();
            std::move(buffer + pos + count, buffer + _size, buffer + pos);
            Resize(_size - count);
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
    };

    /// %Vector template class for POD types. Does not call constructors or destructors and uses block move. Is intentionally (for performance reasons) unsafe for self-insertion.
    template <class T> class PODVector : public VectorBase
    {
    public:
        using ValueType = T;
        using Iterator = RandomAccessIterator<T>;
        using ConstIterator = RandomAccessConstIterator<T>;

        /// Construct empty.
        PODVector() noexcept = default;

        /// Construct with initial size.
        explicit PODVector(uint32_t size)
        {
            Resize(size);
        }

        /// Construct with initial size and default value.
        PODVector(uint32_t size, const T& value)
        {
            Resize(size);
            for (uint32_t i = 0; i < size; ++i)
            {
                At(i) = value;
            }
        }

        /// Construct with initial data.
        PODVector(const T* data, uint32_t size)
        {
            Resize(size);
            CopyElements(Buffer(), data, size);
        }

        /// Construct from another vector.
        PODVector(const PODVector<T>& vector)
        {
            *this = vector;
        }

        /// Aggregate initialization constructor.
        PODVector(const std::initializer_list<T>& list) : PODVector()
        {
            for (auto it = list.begin(); it != list.end(); it++)
            {
                Push(*it);
            }
        }

        /// Destruct.
        ~PODVector()
        {
            FreeBuffer(_buffer);
        }

        /// Assign from another vector.
        PODVector<T>& operator =(const PODVector<T>& rhs)
        {
            // In case of self-assignment do nothing
            if (&rhs != this)
            {
                Resize(rhs._size);
                CopyElements(Buffer(), rhs.Buffer(), rhs._size);
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
            if (rhs._size != _size)
                return false;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (uint32_t i = 0; i < _size; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
                    return false;
            }

            return true;
        }

        /// Test for inequality with another vector.
        bool operator !=(const PODVector<T>& rhs) const
        {
            if (rhs._size != _size)
                return true;

            T* buffer = Buffer();
            T* rhsBuffer = rhs.Buffer();
            for (uint32_t i = 0; i < _size; ++i)
            {
                if (buffer[i] != rhsBuffer[i])
                    return true;
            }

            return false;
        }

        /// Return element at index.
        T& operator [](uint32_t index)
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return const element at index.
        const T& operator [](uint32_t index) const
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return element at index.
        T& At(uint32_t index)
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Return const element at index.
        const T& At(uint32_t index) const
        {
            ALIMER_ASSERT(index < _size);
            return Buffer()[index];
        }

        /// Add an element at the end.
        void Push(const T& value)
        {
            if (_size < _capacity)
                ++_size;
            else
                Resize(_size + 1);
            Back() = value;
        }

        /// Add another vector at the end.
        void Push(const PODVector<T>& vector)
        {
            uint32_t oldSize = _size;
            Resize(_size + vector._size);
            CopyElements(Buffer() + oldSize, vector.Buffer(), vector._size);
        }

        /// Remove the last element.
        void Pop()
        {
            if (_size)
                Resize(_size - 1);
        }

        /// Insert an element at position.
        void Insert(uint32_t pos, const T& value)
        {
            if (pos > _size)
                pos = _size;

            uint32_t oldSize = _size;
            Resize(_size + 1);
            MoveRange(pos + 1, pos, oldSize - pos);
            Buffer()[pos] = value;
        }

        /// Insert another vector at position.
        void Insert(uint32_t pos, const PODVector<T>& vector)
        {
            if (pos > _size)
                pos = _size;

            uint32_t oldSize = _size;
            Resize(_size + vector._size);
            MoveRange(pos + vector._size, pos, oldSize - pos);
            CopyElements(Buffer() + pos, vector.Buffer(), vector._size);
        }

        /// Insert an element by iterator.
        Iterator Insert(const Iterator& dest, const T& value)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > _size)
                pos = _size;
            Insert(pos, value);

            return Begin() + pos;
        }

        /// Insert a vector by iterator.
        Iterator Insert(const Iterator& dest, const PODVector<T>& vector)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > _size)
                pos = _size;
            Insert(pos, vector);

            return Begin() + pos;
        }

        /// Insert a vector partially by iterators.
        Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > _size)
                pos = _size;
            auto length = (uint32_t)(end - start);
            Resize(_size + length);
            MoveRange(pos + length, pos, _size - pos - length);
            CopyElements(Buffer() + pos, &(*start), length);

            return Begin() + pos;
        }

        /// Insert elements.
        Iterator Insert(const Iterator& dest, const T* start, const T* end)
        {
            auto pos = (uint32_t)(dest - Begin());
            if (pos > _size)
                pos = _size;
            auto length = (uint32_t)(end - start);
            Resize(_size + length);
            MoveRange(pos + length, pos, _size - pos - length);

            T* destPtr = Buffer() + pos;
            for (const T* i = start; i != end; ++i)
                *destPtr++ = *i;

            return Begin() + pos;
        }

        /// Erase a range of elements.
        void Erase(uint32_t pos, uint32_t length = 1)
        {
            // Return if the range is illegal
            if (!length || pos + length > _size)
                return;

            MoveRange(pos, pos + length, _size - pos - length);
            Resize(_size - length);
        }

        /// Erase an element by iterator. Return iterator to the next element.
        Iterator Erase(const Iterator& it)
        {
            auto pos = (uint32_t)(it - Begin());
            if (pos >= _size)
                return End();
            Erase(pos);

            return Begin() + pos;
        }

        /// Erase a range by iterators. Return iterator to the next element.
        Iterator Erase(const Iterator& start, const Iterator& end)
        {
            auto pos = (uint32_t)(start - Begin());
            if (pos >= _size)
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
            if (shiftStartIndex > _size || !length)
                return;

            uint32_t newSize = _size - length;
            uint32_t trailingCount = _size - shiftStartIndex;
            if (trailingCount <= length)
            {
                // We're removing more elements from the array than exist past the end of the range being removed, so perform a normal shift and destroy
                MoveRange(pos, shiftStartIndex, trailingCount);
            }
            else
            {
                // Swap elements from the end of the array into the empty space
                CopyElements(Buffer() + pos, Buffer() + newSize, length);
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
            if (newSize > _capacity)
            {
                if (!_capacity)
                {
                    _capacity = newSize;
                }
                else
                {
                    while (_capacity < newSize)
                        _capacity += (_capacity + 1) >> 1;
                }

                uint8_t* newBuffer = AllocateBuffer(_capacity * sizeof(T));
                // Move the data into the new buffer and delete the old
                if (_buffer)
                {
                    CopyElements(reinterpret_cast<T*>(newBuffer), Buffer(), _size);
                    FreeBuffer(_buffer);
                }
                _buffer = newBuffer;
            }

            _size = newSize;
        }

        /// Set new capacity.
        void Reserve(uint32_t newCapacity)
        {
            if (newCapacity < _size)
                newCapacity = _size;

            if (newCapacity != _capacity)
            {
                uint8_t* newBuffer = nullptr;
                _capacity = newCapacity;

                if (_capacity)
                {
                    newBuffer = AllocateBuffer(_capacity * sizeof(T));
                    // Move the data into the new buffer
                    CopyElements(reinterpret_cast<T*>(newBuffer), Buffer(), _size);
                }

                // Delete the old buffer
                FreeBuffer(_buffer);
                _buffer = newBuffer;
            }
        }

        /// Reallocate so that no extra memory is used.
        void Compact() { Reserve(_size); }

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
        uint32_t IndexOf(const T& value) const
        {
            return Find(value) - Begin();
        }

        /// Return whether contains a specific value.
        bool Contains(const T& value) const { return Find(value) != End(); }

        /// Return iterator to the beginning.
        Iterator Begin() { return Iterator(Buffer()); }

        /// Return const iterator to the beginning.
        ConstIterator Begin() const { return ConstIterator(Buffer()); }

        /// Return iterator to the end.
        Iterator End() { return Iterator(Buffer() + _size); }

        /// Return const iterator to the end.
        ConstIterator End() const { return ConstIterator(Buffer() + _size); }

        /// Return first element.
        T& Front() { return Buffer()[0]; }

        /// Return const first element.
        const T& Front() const { return Buffer()[0]; }

        /// Return last element.
        T& Back()
        {
            ALIMER_ASSERT(_size);
            return Buffer()[_size - 1];
        }

        /// Return const last element.
        const T& Back() const
        {
            ALIMER_ASSERT(_size);
            return Buffer()[_size - 1];
        }

        /// Return the buffer with right type.
        T* Buffer() const { return reinterpret_cast<T*>(_buffer); }
        const T* Data() const { return reinterpret_cast<T*>(_buffer); }
        T* Data() { return reinterpret_cast<T*>(_buffer); }

    private:
        /// Move a range of elements within the vector.
        void MoveRange(uint32_t dest, uint32_t src, uint32_t count)
        {
            if (count)
                memmove(Buffer() + dest, Buffer() + src, count * sizeof(T));
        }

        /// Copy elements from one buffer to another.
        static void CopyElements(T* dest, const T* src, uint32_t count)
        {
            if (count)
                memcpy(dest, src, count * sizeof(T));
        }
    };

    template <class T> typename alimer::Vector<T>::ConstIterator begin(const alimer::Vector<T>& v) { return v.Begin(); }

    template <class T> typename alimer::Vector<T>::ConstIterator end(const alimer::Vector<T>& v) { return v.End(); }

    template <class T> typename alimer::Vector<T>::Iterator begin(alimer::Vector<T>& v) { return v.Begin(); }

    template <class T> typename alimer::Vector<T>::Iterator end(alimer::Vector<T>& v) { return v.End(); }

    template <class T> typename alimer::PODVector<T>::ConstIterator begin(const alimer::PODVector<T>& v) { return v.Begin(); }

    template <class T> typename alimer::PODVector<T>::ConstIterator end(const alimer::PODVector<T>& v) { return v.End(); }

    template <class T> typename alimer::PODVector<T>::Iterator begin(alimer::PODVector<T>& v) { return v.Begin(); }

    template <class T> typename alimer::PODVector<T>::Iterator end(alimer::PODVector<T>& v) { return v.End(); }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
