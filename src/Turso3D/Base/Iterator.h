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
    /// Random access iterator.
    template <class T> struct RandomAccessIterator
    {
        /// Construct a null iterator.
        RandomAccessIterator() :
            ptr(nullptr)
        {
        }

        /// Construct from a raw pointer.
        explicit RandomAccessIterator(T* ptr_) :
            ptr(ptr_)
        {
        }

        /// Point to the object.
        T* operator -> () const { return ptr; }
        /// Dereference the object.
        T& operator * () const { return *ptr; }
        /// Preincrement the pointer.
        RandomAccessIterator<T>& operator ++ () { ++ptr; return *this; }
        /// Postincrement the pointer.
        RandomAccessIterator<T> operator ++ (int) { RandomAccessIterator<T> i = *this; ++ptr; return i; }
        /// Predecrement the pointer.
        RandomAccessIterator<T>& operator -- () { --ptr; return *this; }
        /// Postdecrement the pointer.
        RandomAccessIterator<T> operator -- (int) { RandomAccessIterator<T> i = *this; --ptr; return i; }
        /// Add an offset to the pointer.
        RandomAccessIterator<T>& operator += (int value) { ptr += value; return *this; }
        /// Subtract an offset from the pointer.
        RandomAccessIterator<T>& operator -= (int value) { ptr -= value; return *this; }
        /// Add an offset to the pointer.
        RandomAccessIterator<T>& operator += (size_t value) { ptr += value; return *this; }
        /// Subtract an offset from the pointer.
        RandomAccessIterator<T>& operator -= (size_t value) { ptr -= value; return *this; }
        /// Add an offset to the pointer.
        RandomAccessIterator<T> operator + (int value) const { return RandomAccessIterator<T>(ptr + value); }
        /// Subtract an offset from the pointer.
        RandomAccessIterator<T> operator - (int value) const { return RandomAccessIterator<T>(ptr - value); }
        /// Add an offset to the pointer.
        RandomAccessIterator<T> operator + (size_t value) const { return RandomAccessIterator<T>(ptr + value); }
        /// Subtract an offset from the pointer.
        RandomAccessIterator<T> operator - (size_t value) const { return RandomAccessIterator<T>(ptr - value); }
        /// Calculate offset to another iterator.
        int operator - (const RandomAccessIterator& rhs) const { return (int)(ptr - rhs.ptr); }
        /// Test for equality with another iterator.
        bool operator == (const RandomAccessIterator& rhs) const { return ptr == rhs.ptr; }
        /// Test for inequality with another iterator.
        bool operator != (const RandomAccessIterator& rhs) const { return ptr != rhs.ptr; }
        /// Test for less than with another iterator.
        bool operator < (const RandomAccessIterator& rhs) const { return ptr < rhs.ptr; }
        /// Test for greater than with another iterator.
        bool operator > (const RandomAccessIterator& rhs) const { return ptr > rhs.ptr; }
        /// Test for less than or equal with another iterator.
        bool operator <= (const RandomAccessIterator& rhs) const { return ptr <= rhs.ptr; }
        /// Test for greater than or equal with another iterator.
        bool operator >= (const RandomAccessIterator& rhs) const { return ptr >= rhs.ptr; }

        /// Pointer to the random-accessed object(s).
        T* ptr;
    };

    /// Random access const iterator.
    template <class T> struct RandomAccessConstIterator
    {
        /// Construct a null iterator.
        RandomAccessConstIterator()
            : ptr(nullptr)
        {
        }

        /// Construct from a raw pointer.
        explicit RandomAccessConstIterator(const T* ptr_)
            : ptr(ptr_)
        {
        }

        /// Construct from a non-const iterator.
        RandomAccessConstIterator(const RandomAccessIterator<T>& it) :
            ptr(it.ptr)
        {
        }

        /// Assign from a non-const iterator.
        RandomAccessConstIterator<T>& operator = (const RandomAccessIterator<T>& rhs) { ptr = rhs.ptr; return *this; }
        /// Point to the object.
        const T* operator -> () const { return ptr; }
        /// Dereference the object.
        const T& operator * () const { return *ptr; }
        /// Preincrement the pointer.
        RandomAccessConstIterator<T>& operator ++ () { ++ptr; return *this; }
        /// Postincrement the pointer.
        RandomAccessConstIterator<T> operator ++ (int) { RandomAccessConstIterator<T> i = *this; ++ptr; return i; }
        /// Predecrement the pointer.
        RandomAccessConstIterator<T>& operator -- () { --ptr; return *this; }
        /// Postdecrement the pointer.
        RandomAccessConstIterator<T> operator -- (int) { RandomAccessConstIterator<T> i = *this; --ptr; return i; }
        /// Add an offset to the pointer.
        RandomAccessConstIterator<T>& operator += (int value) { ptr += value; return *this; }
        /// Subtract an offset from the pointer.
        RandomAccessConstIterator<T>& operator -= (int value) { ptr -= value; return *this; }
        /// Add an offset to the pointer.
        RandomAccessConstIterator<T>& operator += (size_t value) { ptr += value; return *this; }
        /// Subtract an offset from the pointer.
        RandomAccessConstIterator<T>& operator -= (size_t value) { ptr -= value; return *this; }
        /// Add an offset to the pointer.
        RandomAccessConstIterator<T> operator + (int value) const { return RandomAccessConstIterator<T>(ptr + value); }
        /// Subtract an offset from the pointer.
        RandomAccessConstIterator<T> operator - (int value) const { return RandomAccessConstIterator<T>(ptr - value); }
        /// Add an offset to the pointer.
        RandomAccessConstIterator<T> operator + (size_t value) const { return RandomAccessConstIterator<T>(ptr + value); }
        /// Subtract an offset from the pointer.
        RandomAccessConstIterator<T> operator - (size_t value) const { return RandomAccessConstIterator<T>(ptr - value); }
        /// Calculate offset to another iterator.
        int operator - (const RandomAccessConstIterator& rhs) const { return (int)(ptr - rhs.ptr); }
        /// Test for equality with another iterator.
        bool operator == (const RandomAccessConstIterator& rhs) const { return ptr == rhs.ptr; }
        /// Test for inequality with another iterator.
        bool operator != (const RandomAccessConstIterator& rhs) const { return ptr != rhs.ptr; }
        /// Test for less than with another iterator.
        bool operator < (const RandomAccessConstIterator& rhs) const { return ptr < rhs.ptr; }
        /// Test for greater than with another iterator.
        bool operator > (const RandomAccessConstIterator& rhs) const { return ptr > rhs.ptr; }
        /// Test for less than or equal with another iterator.
        bool operator <= (const RandomAccessConstIterator& rhs) const { return ptr <= rhs.ptr; }
        /// Test for greater than or equal with another iterator.
        bool operator >= (const RandomAccessConstIterator& rhs) const { return ptr >= rhs.ptr; }

        /// Pointer to the random-accessed object(s).
        const T* ptr;
    };
}
