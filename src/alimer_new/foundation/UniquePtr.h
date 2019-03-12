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
#include <cassert>
#include <cstddef>

namespace alimer
{
    /// Delete object of type T. T must be complete. See boost::checked_delete.
    template<class T> inline void CheckedDelete(T* x)
    {
        // intentionally complex - simplification causes regressions
        using type_must_be_complete = char[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        delete x;
    }

    /// Unique pointer template class.
    template <class T> class UniquePtr
    {
    public:
        /// Construct empty.
        UniquePtr() : ptr_(nullptr) { }

        /// Construct from pointer.
        explicit UniquePtr(T* ptr) : ptr_(ptr) { }

        /// Prevent copy construction.
        UniquePtr(const UniquePtr&) = delete;
        /// Prevent assignment.
        UniquePtr& operator=(const UniquePtr&) = delete;

        /// Assign from pointer.
        UniquePtr& operator = (T* ptr)
        {
            Reset(ptr);
            return *this;
        }

        /// Construct empty.
        UniquePtr(std::nullptr_t) : ptr_(nullptr) { }   // NOLINT(google-explicit-constructor)

        /// Move-construct from UniquePtr.
        UniquePtr(UniquePtr&& up) noexcept : ptr_(up.Detach()) {}

        /// Move-assign from UniquePtr.
        UniquePtr& operator =(UniquePtr&& up) noexcept
        {
            Reset(up.Detach());
            return *this;
        }

        /// Point to the object.
        T* operator ->() const
        {
            ALIMER_VERIFY(ptr_);
            return ptr_;
        }

        /// Dereference the object.
        T& operator *() const
        {
            ALIMER_VERIFY(ptr_);
            return *ptr_;
        }

        /// Test for less than with another unique pointer.
        template <class U>
        bool operator <(const UniquePtr<U>& rhs) const { return ptr_ < rhs.ptr_; }

        /// Test for equality with another unique pointer.
        template <class U>
        bool operator ==(const UniquePtr<U>& rhs) const { return ptr_ == rhs.ptr_; }

        /// Test for inequality with another unique pointer.
        template <class U>
        bool operator !=(const UniquePtr<U>& rhs) const { return ptr_ != rhs.ptr_; }

        /// Cast pointer to bool.
        operator bool() const { return !!ptr_; }    // NOLINT(google-explicit-constructor)

        /// Swap with another UniquePtr.
        void Swap(UniquePtr& up) { Urho3D::Swap(ptr_, up.ptr_); }

        /// Detach pointer from UniquePtr without destroying.
        T* Detach()
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        /// Check if the pointer is null.
        bool Null() const { return ptr_ == 0; }

        /// Check if the pointer is not null.
        bool NotNull() const { return ptr_ != 0; }

        /// Return the raw pointer.
        T* Get() const { return ptr_; }

        /// Reset.
        void Reset(T* ptr = nullptr)
        {
            CheckedDelete(ptr_);
            ptr_ = ptr;
        }

        /// Return hash value for HashSet & HashMap.
        uint64_t ToHash() const { return (uint64_t)((size_t)ptr_ / sizeof(T)); }

        /// Destruct.
        ~UniquePtr()
        {
            Reset();
        }

    private:
        T* ptr_;

    };

    /// Swap two UniquePtr-s.
    template <class T> void Swap(UniquePtr<T>& first, UniquePtr<T>& second)
    {
        first.Swap(second);
    }
}
