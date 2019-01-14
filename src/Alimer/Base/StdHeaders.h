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

#ifdef __BORLANDC__
#define __STD_ALGORITHM
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include <memory>

// STL containers
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <set>
#include <list>
#include <forward_list>
#include <deque>
#include <queue>
#include <bitset>
#include <array>

#include <unordered_map>
#include <unordered_set>

// STL algorithms & functions
#include <algorithm>
#include <functional>
#include <limits>
#include <iterator>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" 
{
#   include <sys/types.h>
#   include <sys/stat.h>
}

#if defined(_WIN32)
#   undef min
#   undef max
#	if !defined(NOMINMAX) && defined(_MSC_VER)
#	    define NOMINMAX // required to stop windows.h messing up std::min
#	endif
#   if defined( __MINGW32__ )
#       include <unistd.h>
#   endif
#elif defined(__APPLE__)
extern "C"
{
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
}
#elif defined(__linux__)
extern "C" 
{
#   include <unistd.h>
#   include <dlfcn.h>
}
#endif

namespace alimer
{
    /**
    * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
    *
    * Until C++14, std::hash<T> is not defined if T is a enum (see
    * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
    * even with C++14, as of october 2016, std::hash for enums is not widely
    * implemented by compilers, so here when T is a enum, we use EnumClassHash
    * instead of std::hash. (For instance, in alimer::HashCombine(), or
    * alimer::UnorderedMap.)
    */
    struct EnumClassHash
    {
        template <typename T>
        constexpr std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    /** Hasher that handles custom enums automatically. */
    template <typename Key>
    using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

    /** Fixed sized array that stores element contigously. */
    template <typename T, size_t Size>
    using Array = std::array<T, Size>;

    /** Dynamically sized array that stores element contigously. */
    template <typename T>
    using Vector = std::vector<T>;

    template <typename T>
    using List = std::list<T>;

    /** First-in, last-out data structure. */
    template <typename T>
    using Stack = std::stack<T, std::deque<T>>;

    /** First-in, first-out data structure. */
    template <typename T>
    using Queue = std::queue<T, std::deque<T>>;

    /** An associative container containing an ordered set of elements. */
    template <typename T, typename P = std::less<T>>
    using Set = std::set<T, P>;

    /** An associative container containing an ordered set of key-value pairs. */
    template <typename K, typename V, typename P = std::less<K>>
    using Map = std::map<K, V, P>;

    /** An associative container containing an unordered set of elements. Usually faster than Set for larger data sets. */
    template <typename T, typename H = HashType<T>, typename C = std::equal_to<T>>
    using UnorderedSet = std::unordered_set<T, H, C>;

    /** An associative container containing an ordered set of key-value pairs. Usually faster than Map for larger data sets. */
    template <typename K, typename V, typename H = HashType<K>, typename C = std::equal_to<K>>
    using UnorderedMap = std::unordered_map<K, V, H, C>;

    /// UniquePtr
    template <typename T>
    using UniquePtr = std::unique_ptr<T>;

    /// Construct UniquePtr.
    template <class T, class ... Args> UniquePtr<T> MakeUnique(Args && ... args)
    {
        static_assert(!std::is_array<T>::value, "arrays not supported");
        return UniquePtr<T>(new T(std::forward<Args>(args)...));
    }
}
