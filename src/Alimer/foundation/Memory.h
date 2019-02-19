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

#include <cstddef>
#include <type_traits>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#   include <malloc.h>
#endif

namespace utils {
    inline void* aligned_alloc(size_t size, size_t align) noexcept {
        assert(align && !(align & align - 1));

        void* p = nullptr;

        // must be a power of two and >= sizeof(void*)
        while (align < sizeof(void*)) {
            align <<= 1;
        }

#if defined(WIN32)
        p = ::_aligned_malloc(size, align);
#else
        ::posix_memalign(&p, align, size);
#endif
        return p;
    }

    inline void aligned_free(void* p) noexcept {
#if defined(WIN32)
        ::_aligned_free(p);
#else
        ::free(p);
#endif
    }
}
