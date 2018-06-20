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
#include <memory>
#include <vector>
#include <stdlib.h>

namespace Alimer
{
    template <typename T>
    class ObjectPool
    {
    public:
        template <typename... P>
        T *Allocate(P &&... p)
        {
            if (_vacants.empty())
            {
                size_t num_objects = 64u << _memory.size();
                T *ptr = static_cast<T *>(malloc(num_objects * sizeof(T)));
                if (!ptr)
                    return nullptr;

                for (size_t i = 0; i < num_objects; i++)
                {
                    _vacants.push_back(&ptr[i]);
                }

                _memory.emplace_back(ptr);
            }

            T *ptr = _vacants.back();
            _vacants.pop_back();
            new (ptr) T(std::forward<P>(p)...);
            return ptr;
        }

        void Free(T *ptr)
        {
            ptr->~T();
            _vacants.push_back(ptr);
        }

        void Clear()
        {
            _vacants.clear();
            _memory.clear();
        }

    private:
        struct MallocDeleter
        {
            void operator()(T *ptr)
            {
                ::free(ptr);
            }
        };

        std::vector<T *> _vacants;
        std::vector<std::unique_ptr<T, MallocDeleter>> _memory;
    };
}
