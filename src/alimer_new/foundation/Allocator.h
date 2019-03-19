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

#include "foundation/Preprocessor.h"
#include <new>
#include <utility>

namespace alimer
{
    /**
    * Allocator interface.
    * Used to implement custom allocators.
    */
    class ALIMER_API Allocator
    {
    public:
        Allocator() {}
        virtual ~Allocator() {}
        Allocator(const Allocator&) = delete;
        Allocator& operator=(const Allocator&) = delete;

        virtual void* Allocate(size_t size, size_t align) = 0;
        virtual void Deallocate(void* ptr) = 0;
    };

    ///General purpose allocator.
    ALIMER_API Allocator& DefaultAllocator();
}

#if defined(__ANDROID_API__) && (__ANDROID_API__ < 16)
#include <cstdlib>
void *aligned_alloc(size_t alignment, size_t size)
{
    // alignment must be >= sizeof(void*)
    if (alignment < sizeof(void*))
    {
        alignment = sizeof(void*);
    }

    return memalign(alignment, size);
}
#elif defined(__APPLE__) || defined(__ANDROID__)
#include <cstdlib>
void *aligned_alloc(size_t alignment, size_t size)
{
    // alignment must be >= sizeof(void*)
    if (alignment < sizeof(void*))
    {
        alignment = sizeof(void*);
    }

    void *pointer;
    if (posix_memalign(&pointer, alignment, size) == 0)
    {
        return pointer;
    }

    return nullptr;
}
#endif

#ifndef ALIMER_ALIGNED_MALLOC
#   if defined(_WIN32)
#       define ALIMER_ALIGNED_MALLOC(size, alignment)   (_aligned_malloc((size), (alignment)))
#   else
#       define ALIMER_ALIGNED_MALLOC(size, alignment)   (aligned_alloc((alignment), (size) ))
#   endif
#endif

#ifndef ALIMER_ALIGNED_FREE
#   if defined(_WIN32)
#       define ALIMER_ALIGNED_FREE(ptr)   _aligned_free(ptr)
#   else
#       define ALIMER_ALIGNED_FREE(ptr)   free(ptr)
#   endif
#endif
