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

#include "../Base/HashMap.h"
#include "../Core/Ptr.h"
#include <memory>
#include <utility>

namespace Alimer
{
    template <typename T>
    class Cache
    {
    public:
        void Clear()
        {
            _hashMap.clear();
        }

        T* Find(uint64_t hash) const
        {
            auto itr = _hashMap.find(hash);
            auto *ret = itr != end(_hashMap) ? itr->second.Get() : nullptr;
            return ret;
        }

        T* Insert(uint64_t hash, UniquePtr<T> value)
        {
            auto &cache = _hashMap[hash];
            if (!cache)
                cache = std::move(value);

            auto *ret = cache.Get();
            return ret;
        }

        HashMap<UniquePtr<T>>& GetMap()
        {
            return _hashMap;
        }

        const HashMap<UniquePtr<T>> &GetMap() const
        {
            return _hashMap;
        }

    private:
        HashMap<UniquePtr<T>> _hashMap;
    };
}
