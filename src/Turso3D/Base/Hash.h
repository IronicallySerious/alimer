//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "Turso3DConfig.h"

#include <cstddef>

namespace Turso3D
{
    struct AllocatorBlock;

    /// Pointer hash function.
    template <class T> uint32_t MakeHash(T* value)
    {
        return ((uint32_t)(size_t)value) / sizeof(T);
    }

    /// Const pointer hash function.
    template <class T> uint32_t MakeHash(const T* value)
    {
        return ((uint32_t)(size_t)value) / sizeof(T);
    }

    /// Generic hash function.
    template <class T> uint32_t MakeHash(const T& value)
    {
        return value.ToHash();
    }

    /// Void pointer hash function.
    template<> inline uint32_t MakeHash(void* value)
    {
        return (uint32_t)(size_t)value;
    }

    /// Const void pointer hash function.
    template<> inline uint32_t MakeHash(const void* value)
    {
        return (uint32_t)(size_t)value;
    }

    /// Long long hash function.
    template<> inline uint32_t MakeHash(const long long& value)
    {
        return (value >> 32) | (value & 0xffffffff);
    }

    /// Unsigned long long hash function.
    template<> inline uint32_t MakeHash(const unsigned long long& value)
    {
        return (value >> 32) | (value & 0xffffffff);
    }

    /// Int hash function.
    template<> inline uint32_t MakeHash(const int& value)
    {
        return value;
    }

    /// Unsigned hash function.
    template<> inline uint32_t MakeHash(const uint32_t& value)
    {
        return value;
    }

    /// Short hash function.
    template<> inline uint32_t MakeHash(const int16_t& value)
    {
        return value;
    }

    /// Unsigned short hash function.
    template<> inline unsigned MakeHash(const unsigned short& value)
    {
        return value;
    }

    /// Char hash function.
    template<> inline unsigned MakeHash(const char& value)
    {
        return value;
    }

    /// Unsigned char hash function.
    template<> inline unsigned MakeHash(const unsigned char& value)
    {
        return value;
    }

    /// Hash set/map node base class.
    struct TURSO3D_API HashNodeBase
    {
        /// Construct.
        HashNodeBase() :
            down(nullptr),
            prev(nullptr),
            next(nullptr)
        {
        }

        /// Next node in the bucket.
        HashNodeBase* down;
        /// Previous node.
        HashNodeBase* prev;
        /// Next node.
        HashNodeBase* next;
    };

    /// Hash set/map iterator base class.
    struct TURSO3D_API HashIteratorBase
    {
        /// Construct.
        HashIteratorBase() :
            ptr(nullptr)
        {
        }

        /// Construct with a node pointer.
        explicit HashIteratorBase(HashNodeBase* ptr_) :
            ptr(ptr_)
        {
        }

        /// Test for equality with another iterator.
        bool operator == (const HashIteratorBase& rhs) const { return ptr == rhs.ptr; }
        /// Test for inequality with another iterator.
        bool operator != (const HashIteratorBase& rhs) const { return ptr != rhs.ptr; }

        /// Go to the next node.
        void GotoNext()
        {
            if (ptr)
                ptr = ptr->next;
        }

        /// Go to the previous node.
        void GotoPrev()
        {
            if (ptr)
                ptr = ptr->prev;
        }

        /// %Node pointer.
        HashNodeBase* ptr;
    };

    /// Hash set/map base class.
    class TURSO3D_API HashBase
    {
    public:
        /// Initial amount of buckets.
        static const size_t MIN_BUCKETS = 8;
        /// Maximum load factor.
        static const size_t MAX_LOAD_FACTOR = 4;

        /// Construct.
        HashBase() :
            ptrs(nullptr),
            allocator(nullptr)
        {
        }

        /// Destruct.
        ~HashBase()
        {
            delete[] ptrs;
        }

        /// Swap with another hash set or map.
        void Swap(HashBase& hash);

        /// Return number of elements.
        size_t Size() const { return ptrs ? (reinterpret_cast<size_t*>(ptrs))[0] : 0; }
        /// Return whether has no elements.
        bool IsEmpty() const { return Size() == 0; }

    protected:
        /// Allocate bucket head pointers + room for size and bucket count variables.
        void AllocateBuckets(size_t size, size_t numBuckets);
        /// Reset bucket head pointers.
        void ResetPtrs();
        /// Set new size.
        void SetSize(size_t size) { reinterpret_cast<size_t*>(ptrs)[0] = size; }
        /// Set new head node.
        void SetHead(HashNodeBase* head) { ptrs[2] = head; }
        /// Set new tail node.
        void SetTail(HashNodeBase* tail) { ptrs[3] = tail; }

        /// Return number of buckets.
        size_t NumBuckets() const { return ptrs ? (reinterpret_cast<size_t*>(ptrs))[1] : MIN_BUCKETS; }
        /// Return list head node.
        HashNodeBase* Head() const { return ptrs ? ptrs[2] : nullptr; }
        /// Return list tail node.
        HashNodeBase* Tail() const { return ptrs ? ptrs[3] : nullptr; }
        /// Return bucket head pointers.
        HashNodeBase** Ptrs() const { return ptrs ? ptrs + 4 : nullptr; }

        /// Bucket head pointers.
        HashNodeBase** ptrs;
        /// %Node allocator.
        AllocatorBlock* allocator;
    };
}
