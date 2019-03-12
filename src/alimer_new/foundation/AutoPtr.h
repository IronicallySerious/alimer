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
    /// Pointer which takes ownership of an object and deletes it when the pointer goes out of scope. Ownership can be transferred to another pointer, in which case the source pointer becomes null.
    template <class T> class AutoPtr
    {
    public:
        /// Construct a null pointer.
        AutoPtr() noexcept : ptr_(nullptr)
        {
        }

        /// Construct a null pointer.
        AutoPtr(std::nullptr_t) noexcept :     // NOLINT(google-explicit-constructor)
            ptr_(nullptr)
        {
        }

        /// Copy-construct. Ownership is transferred, making the source pointer null.
        AutoPtr(const AutoPtr<T>& other) noexcept :    // NOLINT(google-explicit-constructor)
            : ptr_(other.ptr_)
        {
            // Trick the compiler so that the AutoPtr can be copied to containers; the latest copy stays non-null
            const_cast<AutoPtr<T>&>(other).ptr_ = nullptr;
        }

        /// Construct with a raw pointer; take ownership of the object.
        AutoPtr(T* ptr) : ptr_(ptr)
        {
        }

        /// Destruct. Delete the object pointed to.
        ~AutoPtr()
        {
            delete ptr_;
        }

        /// Assign from a pointer. Existing object is deleted and ownership is transferred from the source pointer, which becomes null.
        AutoPtr<T>& operator = (const AutoPtr<T>& rhs)
        {
            delete ptr_;
            ptr_ = rhs.ptr_;
            const_cast<AutoPtr<T>&>(rhs).ptr_ = nullptr;
            return *this;
        }

        /// Assign a new object. Existing object is deleted.
        AutoPtr<T>& operator = (T* rhs)
        {
            delete ptr_;
            ptr_ = rhs;
            return *this;
        }

        /// Detach the object from the pointer without destroying it and return it. The pointer becomes null.
        T* Detach()
        {
            T* ret = ptr_;
            ptr_ = nullptr;
            return ret;
        }

        /// Reset to null. Destroys the object.
        void Reset()
        {
            *this = nullptr;
        }

        /// Point to the object.
        T* operator -> () const { ALIMER_VERIFY(ptr_); return ptr_; }
        /// Dereference the object.
        T& operator * () const { ALIMER_VERIFY(ptr_); return *ptr_; }
        /// Convert to the object.
        operator T* () const { return ptr_; }

        /// Return the object.
        T* Get() const { return ptr_; }
        /// Return whether is a null pointer.
        bool IsNull() const { return ptr_ == nullptr; }

    private:
        /// Object pointer.
        T* ptr_;
    };

    /// Pointer which takes ownership of an array allocated with new[] and deletes it when the pointer goes out of scope.
    template <class T> class AutoArrayPtr
    {
    public:
        /// Construct a null pointer.
        AutoArrayPtr() noexcept : ptr_(nullptr)
        {

        }

        /// Construct a null pointer.
        AutoArrayPtr(std::nullptr_t) noexcept :     // NOLINT(google-explicit-constructor)
            ptr_(nullptr)
        {
        }

        /// Copy-construct. Ownership is transferred, making the source pointer null.
        AutoArrayPtr(AutoArrayPtr<T>& other) : ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        /// Construct and take ownership of the array.
        AutoArrayPtr(T* ptr) : ptr_(ptr)
        {
        }

        /// Destruct. Delete the array pointed to.
        ~AutoArrayPtr()
        {
            delete[] ptr_;
        }

        /// Assign from a pointer. Existing array is deleted and ownership is transferred from the source pointer, which becomes null.
        AutoArrayPtr<T>& operator = (AutoArrayPtr<T>& rhs)
        {
            delete[] ptr_;
            ptr_ = rhs.ptr_;
            rhs.ptr_ = nullptr;
            return *this;
        }

        /// Assign a new array. Existing array is deleted.
        AutoArrayPtr<T>& operator = (T* rhs)
        {
            delete[] ptr_;
            ptr_ = rhs;
            return *this;
        }

        /// Detach the array from the pointer without destroying it and return it. The pointer becomes null.
        T* Detach()
        {
            T* ret = ptr_;
            ptr_ = nullptr;
            return ret;
        }

        /// Reset to null. Destroys the array.
        void Reset()
        {
            *this = nullptr;
        }

        /// Point to the array.
        T* operator -> () const { ALIMER_VERIFY(ptr_); return ptr_; }
        /// Dereference the array.
        T& operator * () const { ALIMER_VERIFY(ptr_); return *ptr_; }
        /// Index the array.
        T& operator [] (size_t index) { ALIMER_VERIFY(ptr_); return ptr_[index]; }
        /// Const-index the array.
        const T& operator [] (size_t index) const { ALIMER_VERIFY(ptr_); return ptr_[index]; }
        /// Convert to bool.
        operator bool() const { return ptr_ != nullptr; }

        /// Return the array.
        T* Get() const { return ptr_; }
        /// Return whether is a null pointer.
        bool IsNull() const { return ptr_ == nullptr; }

    private:
        /// Array pointer.
        T* ptr_;
    };

}
