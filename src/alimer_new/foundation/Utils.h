//
// Copyright (c) 2008-2019 the Urho3D project.
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

#include "foundation/Platform.h"
#include <string>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
#   include <windows.h>
#endif

namespace alimer
{
    /// Constant blank string, useful for returning by ref where local does not exist
    const std::string EMPTY_STRING;

    template <typename T, size_t N>
    constexpr inline size_t ArraySize(T(&)[N])
    {
        return N;
    }

    template <typename T, unsigned int N>
    void SafeRelease(T(&resourceBlock)[N])
    {
        for (unsigned int i = 0; i < N; i++)
        {
            SafeRelease(resourceBlock[i]);
        }
    }

    template <typename T>
    void SafeRelease(T &resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    template <typename T>
    void SafeDelete(T *&resource)
    {
        delete resource;
        resource = nullptr;
    }

    template <typename T>
    void SafeDeleteContainer(T &resource)
    {
        for (auto &element : resource)
        {
            SafeDelete(element);
        }
        resource.clear();
    }

    template <typename T>
    void SafeDeleteArray(T *&resource)
    {
        delete[] resource;
        resource = nullptr;
    }

    // Provide a less-than function for comparing structs
// Note: struct memory must be initialized to zero, because of packing gaps
    template <typename T>
    inline bool StructLessThan(const T &a, const T &b)
    {
        return (memcmp(&a, &b, sizeof(T)) < 0);
    }

    // Provide a less-than function for comparing structs
    // Note: struct memory must be initialized to zero, because of packing gaps
    template <typename T>
    inline bool StructEquals(const T &a, const T &b)
    {
        return (memcmp(&a, &b, sizeof(T)) == 0);
    }

    template <typename T>
    inline void StructZero(T *obj)
    {
        memset(obj, 0, sizeof(T));
    }

    template <typename T>
    inline bool IsMaskFlagSet(T mask, T flag)
    {
        // Handles multibit flags as well
        return (mask & flag) == flag;
    }

#if defined(_WIN32) || defined(_WIN64)
    inline std::wstring ToUtf16(const char* utf8, size_t len) {
        int bufferSize = ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(len), nullptr, 0);
        std::vector<WCHAR> buffer(bufferSize);
        ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(len), buffer.data(), bufferSize);
        return std::wstring(buffer.data(), bufferSize);
    }

    inline std::wstring ToUtf16(const std::string& str) {
        return ToUtf16(str.data(), str.length());
    }

    inline std::string ToUtf8(const wchar_t* wide, size_t len) {
        int bufferSize = ::WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(len),
            nullptr, 0, nullptr, nullptr);
        std::vector<char> buffer(bufferSize);
        ::WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(len), buffer.data(), bufferSize,
            nullptr, nullptr);
        return std::string(buffer.data(), bufferSize);
    }

    inline std::string ToUtf8(const wchar_t* wide) {
        return ToUtf8(wide, wcslen(wide));
    }

    inline std::string ToUtf8(const std::wstring& wstr) {
        return ToUtf8(wstr.data(), wstr.length());
    }
#endif  // defined(_WIN32) || defined(_WIN64)
}

