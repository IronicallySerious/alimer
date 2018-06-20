/* Copyright (c) 2017 Hans-Kristian Arntzen
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Adopted from Granite: https://github.com/Themaister/Granite
// Licensed under MIT: https://github.com/Themaister/Granite/blob/master/LICENSE

#pragma once
#include <stddef.h>
#include <utility>
#include <memory>

namespace Alimer
{
    template <typename T, typename Deleter = std::default_delete<T>>
    class IntrusivePtrEnabled
    {
    public:
        void AddRef()
        {
            ++_refCount;
        }

        void Release()
        {
            size_t count = --_refCount;
            if (count == 0)
                Deleter()(static_cast<T *>(this));
        }

        IntrusivePtrEnabled() = default;
        IntrusivePtrEnabled(const IntrusivePtrEnabled &) = delete;
        void operator=(const IntrusivePtrEnabled &) = delete;

    private:
        size_t _refCount = 1;
    };

    template <typename T, typename Deleter = std::default_delete<T>>
    class IntrusivePtr
    {
    public:
        IntrusivePtr() = default;

        IntrusivePtr(T *handle) noexcept
            : ptr_(handle)
        {
        }

        T &operator*()
        {
            return *ptr_;
        }

        const T &operator*() const
        {
            return *ptr_;
        }

        T *operator->()
        {
            return ptr_;
        }

        const T *operator->() const
        {
            return ptr_;
        }

        explicit operator bool() const
        {
            return ptr_ != nullptr;
        }

        bool operator==(const IntrusivePtr &other) const
        {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const IntrusivePtr &other) const
        {
            return ptr_ != other.ptr_;
        }

        T *Get()
        {
            return ptr_;
        }

        const T *Get() const
        {
            return ptr_;
        }

        void Reset()
        {
            // Static up-cast here to avoid potential issues with multiple intrusive inheritance.
            // Also makes sure that the pointer type actually inherits from this type.
            if (ptr_)
                static_cast<IntrusivePtrEnabled<T, Deleter> *>(ptr_)->Release();
            ptr_ = nullptr;
        }

        IntrusivePtr &operator=(const IntrusivePtr &other)
        {
            if (this != &other)
            {
                Reset();
                ptr_ = other.ptr_;

                // Static up-cast here to avoid potential issues with multiple intrusive inheritance.
                // Also makes sure that the pointer type actually inherits from this type.
                if (ptr_)
                    static_cast<IntrusivePtrEnabled<T, Deleter> *>(ptr_)->AddRef();
            }
            return *this;
        }

        IntrusivePtr(const IntrusivePtr &other)
        {
            *this = other;
        }

        ~IntrusivePtr()
        {
            Reset();
        }

        IntrusivePtr &operator=(IntrusivePtr &&other)
        {
            if (this != &other)
            {
                Reset();
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        IntrusivePtr(IntrusivePtr &&other)
        {
            *this = std::move(other);
        }

    private:
        T* ptr_ = nullptr;
    };

    template <typename T, typename... P>
    IntrusivePtr<T> MakeHandle(P &&... p)
    {
        return IntrusivePtr<T>(new T(std::forward<P>(p)...));
    }

    template <typename Base, typename Derived, typename... P>
    IntrusivePtr<Base> MakeAbstractHandle(P &&... p)
    {
        return IntrusivePtr<Base>(new Derived(std::forward<P>(p)...));
    }
}
