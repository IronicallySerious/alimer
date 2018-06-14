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

#include "../Core/Ptr.h"
#include <mutex>
#include <array>

namespace Alimer
{
    /// Thread-safe object recycle array.
    template<typename T, size_t N>
    class RecycleArray
    {
    public:
        SharedPtr<T> Get() {
            std::lock_guard<std::mutex> lock(_mutex);

            if (_id == 0)
                return nullptr;

            return _objects.at(--_id);
        }

        void Store(const SharedPtr<T>& object) {
            std::lock_guard<std::mutex> lock(_mutex);

            if (_id < N)
                _objects.at(_id++) = object;
        }

    private:
        std::mutex _mutex;
        std::array<SharedPtr<T>, N> _objects;
        size_t _id = 0;
    };
}
