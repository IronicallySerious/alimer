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

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

namespace alimer
{
    namespace hash
    {
        /**
        * Hash for enum types, to be used instead of std::hash<T> when T is an enum.

        * Until C++14, std::hash<T> is not defined if T is a enum (see
        * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
        * even with C++14, as of october 2016, std::hash for enums is not widely
        * implemented by compilers, so here when T is a enum, we use EnumClassHash
        * instead of std::hash.
        */
        struct EnumClassHash
        {
            template <typename T>
            constexpr size_t operator()(T t) const
            {
                return static_cast<size_t>(t);
            }
        };

        /** Generates a hash for the provided type. Type must have a std::hash specialization. */
        template <class T>
        size_t hash(const T& v)
        {
            using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

            HashType hasher;
            return hasher(v);
        }

        /** Generates a new hash for the provided type using the default standard hasher and combines it with a previous hash. */
        template <class T>
        void combine(size_t& seed, const T& v)
        {
            using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

            HashType hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }


        inline uint32_t murmur3(const uint32_t* key, size_t wordCount, uint32_t seed) {
            uint32_t h = seed;
            size_t i = wordCount;
            do {
                uint32_t k = *key++;
                k *= 0xcc9e2d51;
                k = (k << 15) | (k >> 17);
                k *= 0x1b873593;
                h ^= k;
                h = (h << 13) | (h >> 19);
                h = (h * 5) + 0xe6546b64;
            } while (--i);
            h ^= wordCount;
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }

        template<typename T>
        struct MurmurHashFn {
            uint32_t operator()(const T& key) const {
                static_assert(0 == (sizeof(key) & 3), "Hashing requires a size that is a multiple of 4.");
                return murmur3((const uint32_t*)&key, sizeof(key) / 4, 0);
            }
        };
    } 
} 
