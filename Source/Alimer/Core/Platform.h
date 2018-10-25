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

#include "../PlatformDef.h"
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#   define WIN32_LEAN_AND_MEAN

#   ifndef NOMINMAX
#       define NOMINMAX
#   endif

#   include <windows.h>
#   ifdef DrawText
#       undef DrawText
#   endif
#endif

#include "../AlimerConfig.h"
#include <string>

namespace Alimer
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
		Windows,
		/**
		* Windows universal platform.
		*/
        UWP,
		/**
		* Linux platform.
		*/
		Linux,
		/**
		* Apple OSX platform.
		*/
        macOS,
		/**
		* Android platform.
		*/
		Android,
		/**
		* Apple iOS platform.
		*/
		iOS,
		/**
		* Apple TV platform.
		*/
		AppleTV,
		/**
		* Web (Emscripten/WASM) platform.
		*/
		Web,
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
		Desktop,
		/**
		* Mobile family.
		*/
		Mobile,
		/**
		* Console family.
		*/
		Console
	};

	/// Get the running platform type.
	ALIMER_API PlatformType GetPlatformType();

	/// Get the running platform family.
    ALIMER_API PlatformFamily GetPlatformFamily();

	/// Get the running platform name.
    ALIMER_API const char* GetPlatformName();

    /// Get the operating system description.
    ALIMER_API std::string GetOSDescription();

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
}
