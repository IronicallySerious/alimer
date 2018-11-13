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

#include "../Base/String.h"

/**
* Identifiers the running platform type.
*/
enum PlatformType
{
    /**
    * Unknown platform.
    */
    PLATFORM_TYPE_UNKNOWN = 0,
    /**
    * Windows platform.
    */
    PLATFORM_TYPE_WINDOWS = 1,
    /**
    * Windows universal platform.
    */
    PLATFORM_TYPE_UWP = 2,
    /**
    * Xbox One platform.
    */
    PLATFORM_TYPE_XBOX_ONE = 3,
    /**
    * Linux platform.
    */
    PLATFORM_TYPE_LINUX = 4,
    /**
    * Apple OSX platform.
    */
    PLATFORM_TYPE_MACOS = 5,
    /**
    * Android platform.
    */
    PLATFORM_TYPE_ANDROID = 6,
    /**
    * Apple iOS platform.
    */
    PLATFORM_TYPE_IOS = 7,
    /**
    * Apple TV platform.
    */
    PLATFORM_TYPE_APPLE_TV = 8,
    /**
    * Web (Emscripten/WASM) platform.
    */
    PLATFORM_TYPE_WEB = 9,
};

/**
* Identifiers the running platform family.
*/
enum PlatformFamily
{
    /**
    * Unknown platform family.
    */
    PLATFORM_FAMILY_UNKNOWN = 0,
    /**
    * Desktop family.
    */
    PLATFORM_FAMILY_DESKTOP = 1,
    /**
    * Mobile family.
    */
    PLATFORM_FAMILY_MOBILE = 2,
    /**
    * Console family.
    */
    PLATFORM_FAMILY_
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
ALIMER_API void* GetSymbol(void* handle, const char* name);

/// Try to set the current thread name.
ALIMER_API void SetCurrentThreadName(const char* name);

/// Suspends execution for given milliseconds.
ALIMER_API void Sleep(uint32_t milliseconds);

namespace Alimer
{
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
    ALIMER_API String GetWin32ErrorString(unsigned long errorCode);
    ALIMER_API String GetDXErrorString(long hr);
#endif
}
