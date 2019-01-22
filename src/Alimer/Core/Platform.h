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

#include "../Base/String.h"

namespace alimer
{
    /**
    * Identifiers the running platform type.
    */
    enum class PlatformType : uint32_t
    {
        /**
        * Unknown platform.
        */
        Unknown = 0,
        /**
        * Windows platform.
        */
        Windows = 1,
        /**
        * Windows universal platform.
        */
        UWP = 2,
        /**
        * Xbox One platform.
        */
        XboxOne = 3,
        /**
        * Linux platform.
        */
        Linux = 4,
        /**
        * Apple OSX platform.
        */
        MacOS = 5,
        /**
        * Android platform.
        */
        Android = 6,
        /**
        * Apple iOS platform.
        */
        iOS = 7,
        /**
        * Apple TV platform.
        */
        AppleTV = 8,
        /**
        * Web (Emscripten/WASM) platform.
        */
        Web = 9,
    };

    /**
    * Identifiers the running platform family.
    */
    enum class PlatformFamily : uint32_t
    {
        /**
        * Unknown platform family.
        */
        Unknown = 0,
        /**
        * Desktop family.
        */
        Desktop = 1,
        /**
        * Mobile family.
        */
        Mobile = 2,
        /**
        * Console family.
        */
        Console = 3
    };

    /// Get the running platform type.
    ALIMER_API PlatformType GetPlatformType();

    /// Get the running platform family.
    ALIMER_API PlatformFamily GetPlatformFamily();

    /// Get the running platform name.
    ALIMER_API const char* GetPlatformName();

    /// Get the operating system description.
    ALIMER_API const char* GetOSDescription();

    /// Load native library
    ALIMER_API void* LoadNativeLibrary(const char* name);

    /// Unload native library
    ALIMER_API void UnloadNativeLibrary(void* handle);

    /// Get native library symbol
    ALIMER_API void* GetLibrarySymbol(void* handle, const char* name);

    /// Try to set the current thread name.
    ALIMER_API void SetCurrentThreadName(const char* name);

    /// Suspends execution for given milliseconds.
    ALIMER_API void Sleep(uint32_t milliseconds);

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
    ALIMER_API String GetWin32ErrorString(unsigned long errorCode);
    ALIMER_API WString GetDXErrorString(long hr);
    ALIMER_API String GetDXErrorStringAnsi(long hr);
#endif
}
