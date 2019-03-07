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

#include "../Graphics/Pipeline.h"
#include "../Graphics/Backend.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"
#include <map>
#include <mutex>

// This requires SSE4.2 which is present on Intel Nehalem (Nov. 2008)
// and AMD Bulldozer (Oct. 2011) processors.  I could put a runtime
// check for this, but I'm just going to assume people playing with
// DirectX 12 on Windows 10 have fairly recent machines.
#ifdef _M_X64
#   define ENABLE_SSE_CRC32 1
#else
#   define ENABLE_SSE_CRC32 0
#endif

#if ENABLE_SSE_CRC32
#   pragma intrinsic(_mm_crc32_u32)
#   pragma intrinsic(_mm_crc32_u64)
#endif

namespace alimer
{
    Pipeline::Pipeline()
        : GPUResource(Type::Pipeline)
    {
        memset(&_descriptor, 0, sizeof(_descriptor));
    }

    Pipeline::~Pipeline()
    {
        SafeDelete(_handle);
    }

    static std::map<size_t, PipelineHandle*> s_renderPipelineHashMap;

    template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T> __forceinline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T> __forceinline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    inline size_t HashRange(const uint32_t* const Begin, const uint32_t* const End, size_t Hash)
    {
#if ENABLE_SSE_CRC32
        const uint64_t* Iter64 = (const uint64_t*)AlignUp(Begin, 8);
        const uint64_t* const End64 = (const uint64_t* const)AlignDown(End, 8);

        // If not 64-bit aligned, start with a single u32
        if ((uint32_t*)Iter64 > Begin)
            Hash = _mm_crc32_u32((uint32_t)Hash, *Begin);

        // Iterate over consecutive u64 values
        while (Iter64 < End64)
            Hash = _mm_crc32_u64((uint64_t)Hash, *Iter64++);

        // If there is a 32-bit remainder, accumulate that
        if ((uint32_t*)Iter64 < End)
            Hash = _mm_crc32_u32((uint32_t)Hash, *(uint32_t*)Iter64);
#else
        // An inexpensive hash for CPUs lacking SSE4.2
        for (const uint32_t* Iter = Begin; Iter < End; ++Iter)
            Hash = 16777619U * Hash ^ *Iter;
#endif

        return Hash;
    }

    template <typename T> inline size_t HashState(const T* StateDesc, size_t Count = 1, size_t Hash = 2166136261U)
    {
        static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
        return HashRange((uint32_t*)StateDesc, (uint32_t*)(StateDesc + Count), Hash);
    }

    PipelineHandle* Pipeline::GetHandle() const
    {
        if (!_device || !_device->IsInitialized()) {
            return nullptr;
        }

        size_t hashCode = HashState(&_descriptor);

        PipelineHandle* handle = nullptr;
        bool firstCompile = false;
        {
            static std::mutex s_HashMapMutex;
            std::lock_guard<std::mutex> lockGuard(s_HashMapMutex);
            auto iter = s_renderPipelineHashMap.find(hashCode);

            // Reserve space so the next inquiry will find that someone got here first.
            if (iter == s_renderPipelineHashMap.end())
            {
                firstCompile = true;
                handle = s_renderPipelineHashMap[hashCode];
            }
            else {
                handle = iter->second;
            }
        }

        if (firstCompile)
        {
            _handle = _device->CreateRenderPipeline(&_descriptor);
            s_renderPipelineHashMap[hashCode] = _handle;
        }
        else
        {
            _handle = handle;
        }

        return _handle;
    }
}
