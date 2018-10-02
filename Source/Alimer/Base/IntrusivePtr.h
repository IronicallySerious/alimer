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

#include <stddef.h>
#include <utility>
#include <memory>
#include <atomic>
#include <type_traits>

namespace Alimer
{
    class DefaultRefCounter
    {
    public:
        inline void AddReference()
        {
            _count++;
        }

        inline bool Release()
        {
            return --_count == 0;
        }

    private:
        uint32_t _count = 1;
    };

    class MTDefaultRefCounter
    {
    public:
        MTDefaultRefCounter()
        {
            _count.store(1, std::memory_order_relaxed);
        }

        inline void AddReference()
        {
            _count.fetch_add(1, std::memory_order_relaxed);
        }

        inline bool Release()
        {
            uint32_t result = _count.fetch_sub(1, std::memory_order_acq_rel);
            return result == 1;
        }

    private:
        std::atomic_uint32_t _count;
    };

    template <typename T>
    class IntrusivePtr;

    template <typename T, typename Deleter = std::default_delete<T>, typename ReferenceCounter = DefaultRefCounter>
    class IntrusivePtrEnabled
    {
    public:
        using IntrusivePtrType = IntrusivePtr<T>;
        using EnabledBase = T;
        using EnabledDeleter = Deleter;
        using EnabledReferenceOp = ReferenceCounter;

        void AddReference()
        {
            _referenceCount.AddReference();
        }

        void Release()
        {
            if (_referenceCount.Release())
                Deleter()(static_cast<T *>(this));
        }

        IntrusivePtrEnabled() = default;

        IntrusivePtrEnabled(const IntrusivePtrEnabled &) = delete;
        void operator=(const IntrusivePtrEnabled &) = delete;

    protected:
        IntrusivePtr<T> ReferenceFromThis();

    private:
        ReferenceCounter _referenceCount;
    };

    template <typename T>
    class IntrusivePtr
    {
    public:
        template <typename U>
        friend class IntrusivePtr;

        IntrusivePtr() = default;

        explicit IntrusivePtr(T *ptr) : _ptr(ptr)
        {
        }

        T &operator*() { return *_ptr; }
        const T &operator*() const { return *_ptr; }
        T* operator->() { return _ptr; }
        const T* operator->() const { return _ptr; }

        explicit operator bool() const { return _ptr != nullptr; }

        bool operator==(const IntrusivePtr &other) const
        {
            return _ptr == other._ptr;
        }

        bool operator!=(const IntrusivePtr &other) const
        {
            return _ptr != other._ptr;
        }

        T* Get() { return _ptr; }

        const T* Get() const { return _ptr; }

        void Reset()
        {
            using ReferenceBase = IntrusivePtrEnabled<
                typename T::EnabledBase,
                typename T::EnabledDeleter,
                typename T::EnabledReferenceOp>;

            // Static up-cast here to avoid potential issues with multiple intrusive inheritance.
            // Also makes sure that the pointer type actually inherits from this type.
            if (_ptr)
            {
                static_cast<ReferenceBase*>(_ptr)->Release();
            }

            _ptr = nullptr;
        }

        template <typename U>
        IntrusivePtr &operator=(const IntrusivePtr<U> &other)
        {
            static_assert(std::is_base_of<T, U>::value,
                "Cannot safely assign downcasted intrusive pointers.");

            using ReferenceBase = IntrusivePtrEnabled<
                typename T::EnabledBase,
                typename T::EnabledDeleter,
                typename T::EnabledReferenceOp>;

            Reset();
            _ptr = static_cast<T*>(other._ptr);

            // Static up-cast here to avoid potential issues with multiple intrusive inheritance.
            // Also makes sure that the pointer type actually inherits from this type.
            if (_ptr)
                static_cast<ReferenceBase *>(_ptr)->AddReference();
            return *this;
        }

        IntrusivePtr &operator=(const IntrusivePtr &other)
        {
            using ReferenceBase = IntrusivePtrEnabled<
                typename T::EnabledBase,
                typename T::EnabledDeleter,
                typename T::EnabledReferenceOp>;

            if (this != &other)
            {
                Reset();
                _ptr = other._ptr;
                if (_ptr)
                    static_cast<ReferenceBase *>(_ptr)->AddReference();
            }
            return *this;
        }

        template <typename U>
        IntrusivePtr(const IntrusivePtr<U> &other)
        {
            *this = other;
        }

        IntrusivePtr(const IntrusivePtr &other)
        {
            *this = other;
        }

        ~IntrusivePtr()
        {
            Reset();
        }

        template <typename U>
        IntrusivePtr &operator=(IntrusivePtr<U> &&other) noexcept
        {
            Reset();
            _ptr = other._ptr;
            other._ptr = nullptr;
            return *this;
        }

        IntrusivePtr &operator=(IntrusivePtr &&other) noexcept
        {
            if (this != &other)
            {
                Reset();
                _ptr = other._ptr;
                other._ptr = nullptr;
            }
            return *this;
        }

        template <typename U>
        IntrusivePtr(IntrusivePtr<U> &&other) noexcept
        {
            *this = std::move(other);
        }

        template <typename U>
        IntrusivePtr(IntrusivePtr &&other) noexcept
        {
            *this = std::move(other);
        }

    private:
        T* _ptr = nullptr;
    };

    template <typename T, typename Deleter, typename ReferenceOps>
    IntrusivePtr<T> IntrusivePtrEnabled<T, Deleter, ReferenceOps>::ReferenceFromThis()
    {
        AddReference();
        return IntrusivePtr<T>(static_cast<T*>(this));
    }

    template <typename Derived>
    using DerivedIntrusivePtrType = IntrusivePtr<Derived>;

    template <typename T, typename... P>
    DerivedIntrusivePtrType<T> MakeHandle(P &&... p)
    {
        return DerivedIntrusivePtrType<T>(new T(std::forward<P>(p)...));
    }

    template <typename Base, typename Derived, typename... P>
    typename Base::IntrusivePtrType MakeDerivedHandle(P &&... p)
    {
        return typename Base::IntrusivePtrType(new Derived(std::forward<P>(p)...));
    }

    template <typename T>
    using ThreadSafeIntrusivePtrEnabled = IntrusivePtrEnabled<T, std::default_delete<T>, MTDefaultRefCounter>;
}
