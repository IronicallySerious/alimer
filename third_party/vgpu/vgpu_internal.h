//
// Copyright (c) 2019 Amer Koleci.
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

#include "vgpu/vgpu.h"

class Hasher
{
public:
    Hasher(uint64_t h_)
        : h(h_)
    {
    }

    Hasher() = default;

    template <typename T>
    inline void data(const T *data, size_t size)
    {
        size /= sizeof(*data);
        for (size_t i = 0; i < size; i++)
            h = (h * 0x100000001b3ull) ^ data[i];
    }

    inline void u32(uint32_t value)
    {
        h = (h * 0x100000001b3ull) ^ value;
    }

    inline void s32(int32_t value)
    {
        u32(uint32_t(value));
    }

    inline void f32(float value)
    {
        union
        {
            float f32;
            uint32_t u32;
        } u;
        u.f32 = value;
        u32(u.u32);
    }

    inline void u64(uint64_t value)
    {
        u32(value & 0xffffffffu);
        u32(value >> 32);
    }

    template <typename T>
    inline void pointer(T *ptr)
    {
        u64(reinterpret_cast<uintptr_t>(ptr));
    }

    inline void string(const char *str)
    {
        char c;
        u32(0xff);
        while ((c = *str++) != '\0')
            u32(uint8_t(c));
    }

    /*inline void string(const std::string &str)
    {
        u32(0xff);
        for (auto &c : str)
            u32(uint8_t(c));
    }*/

    inline uint64_t get() const
    {
        return h;
    }

private:
    uint64_t h = 0xcbf29ce484222325ull;
};