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

/** @file windows.h
@brief Safe inclusion of windows.h

Safe inclusion of windows.h without collisions with foundation library symbols.
*/

#include <foundation/platform.h>
#include <foundation/types.h>

#if ALIMER_PLATFORM_WINDOWS

#   define STREAM_SEEK_END _STREAM_SEEK_END
#   undef UUID_DEFINED
#   undef UUID

#   define UUID_DEFINED 1
#   define UUID uint128_t

#   define WIN32_LEAN_AND_MEAN

#   if ALIMER_COMPILER_MSVC
//Work around broken dbghlp.h header
#       pragma warning(disable : 4091)
#   elif ALIMER_COMPILER_CLANG
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#   endif

#   include <Windows.h>

#   include <WinSock2.h>
#   include <IPTypes.h>
#   include <WS2tcpip.h>
#   include <iphlpapi.h>
#   include <share.h>
#   include <io.h>
#   include <shellapi.h>
#   include <stdlib.h>
#   include <ShlObj.h>
#   include <DbgHelp.h>
#   include <crtdbg.h>

#   undef min
#   undef max
#   undef STREAM_SEEK_END
#   undef UUID

#   if ALIMER_COMPILER_CLANG
#       undef WINAPI
#       define WINAPI STDCALL
#       pragma clang diagnostic pop
#   endif

#endif
