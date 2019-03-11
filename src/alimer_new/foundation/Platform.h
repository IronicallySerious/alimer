//
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

// Platforms
#define ALIMER_PLATFORM_ANDROID 0
#define ALIMER_PLATFORM_EMSCRIPTEN 0
#define ALIMER_PLATFORM_BSD 0
#define ALIMER_PLATFORM_LINUX 0
#define ALIMER_PLATFORM_LINUX_RASPBERRYPI 0
#define ALIMER_PLATFORM_IOS 0
#define ALIMER_PLATFORM_TVOS 0
#define ALIMER_PLATFORM_WATCHOS 0
#define ALIMER_PLATFORM_MACOS 0
#define ALIMER_PLATFORM_WINDOWS 0
#define ALIMER_PLATFORM_XBOXONE 0
#define ALIMER_PLATFORM_UWP 0

// Platform traits and groups
#define ALIMER_PLATFORM_APPLE 0
#define ALIMER_PLATFORM_POSIX 0

#define ALIMER_PLATFORM_FAMILY_MOBILE 0
#define ALIMER_PLATFORM_FAMILY_DESKTOP 0
#define ALIMER_PLATFORM_FAMILY_CONSOLE 0

// Architectures
#define ALIMER_ARCH_ARM 0
#define ALIMER_ARCH_ARM5 0
#define ALIMER_ARCH_ARM6 0
#define ALIMER_ARCH_ARM7 0
#define ALIMER_ARCH_ARM8 0
#define ALIMER_ARCH_ARM_64 0
#define ALIMER_ARCH_ARM8_64 0
#define ALIMER_ARCH_X86 0
#define ALIMER_ARCH_X86_64 0
#define ALIMER_ARCH_PPC 0
#define ALIMER_ARCH_PPC_64 0
#define ALIMER_ARCH_IA64 0
#define ALIMER_ARCH_MIPS 0
#define ALIMER_ARCH_MIPS_64 0
#define ALIMER_ARCH_GENERIC 0

// Architecture
#define ALIMER_ARCH_SSE2 0
#define ALIMER_ARCH_SSE3 0
#define ALIMER_ARCH_SSE4 0
#define ALIMER_ARCH_SSE4_FMA3 0
#define ALIMER_ARCH_NEON 0
#define ALIMER_ARCH_THUMB 0

#define ALIMER_ARCH_ENDIAN_LITTLE 0
#define ALIMER_ARCH_ENDIAN_BIG 0

// Compilers
#define ALIMER_COMPILER_CLANG 0
#define ALIMER_COMPILER_GCC 0
#define ALIMER_COMPILER_MSVC 0
#define ALIMER_COMPILER_INTEL 0

// Android
#if defined(__ANDROID__)
#   undef  ALIMER_PLATFORM_ANDROID
#   define ALIMER_PLATFORM_ANDROID 1
#   undef  ALIMER_PLATFORM_POSIX
#   define ALIMER_PLATFORM_POSIX 1
#   define ALIMER_PLATFORM_NAME "Android"

// Architecture and detailed description
#   if defined( __arm__ )
#       undef  ALIMER_ARCH_ARM
#       define ALIMER_ARCH_ARM 1
#       ifdef __ARM_ARCH_7A__
#           undef  ALIMER_ARCH_ARM7
#           define ALIMER_ARCH_ARM7 1
#           define ALIMER_PLATFORM_DESCRIPTION "Android ARMv7"
#    elif defined(__ARM_ARCH_5TE__)
#           undef  ALIMER_ARCH_ARM5
#           define ALIMER_ARCH_ARM5 1
#           define ALIMER_PLATFORM_DESCRIPTION "Android ARMv5"
#    else
#       error Unsupported ARM architecture
#    endif
#   elif defined( __aarch64__ )
#       undef  ALIMER_ARCH_ARM
#       define ALIMER_ARCH_ARM 1
#       undef  ALIMER_ARCH_ARM_64
#       define ALIMER_ARCH_ARM_64 1
//Assume ARMv8 for now
//#     if defined( __ARM_ARCH ) && ( __ARM_ARCH == 8 )
#           undef  ALIMER_ARCH_ARM8_64
#           define ALIMER_ARCH_ARM8_64 1
#           define ALIMER_PLATFORM_DESCRIPTION "Android ARM64v8"
//#     else
//#         error Unrecognized AArch64 architecture
//#     endif
#   elif defined( __i386__ )
#       undef  ALIMER_ARCH_X86
#       define ALIMER_ARCH_X86 1
#       define ALIMER_PLATFORM_DESCRIPTION "Android x86"
#   elif defined( __x86_64__ )
#       undef  ALIMER_ARCH_X86_64
#       define ALIMER_ARCH_X86_64 1
#       define ALIMER_PLATFORM_DESCRIPTION "Android x86-64"
#   elif ( defined( __mips__ ) && defined( __mips64 ) )
#       undef  ALIMER_ARCH_MIPS
#       define ALIMER_ARCH_MIPS 1
#       undef  ALIMER_ARCH_MIPS_64
#       define ALIMER_ARCH_MIPS_64 1
#       define ALIMER_PLATFORM_DESCRIPTION "Android MIPS64"
#       ifndef _MIPS_ISA
#           define _MIPS_ISA 7 /*_MIPS_ISA_MIPS64*/
#       endif
#   elif defined( __mips__ )
#       undef  ALIMER_ARCH_MIPS
#       define ALIMER_ARCH_MIPS 1
#       define ALIMER_PLATFORM_DESCRIPTION "Android MIPS"
#       ifndef _MIPS_ISA
#           define _MIPS_ISA 6 /*_MIPS_ISA_MIPS32*/
#       endif
#   else
#       error Unknown architecture
#  endif

// Traits
#   if ALIMER_ARCH_MIPS
#       if defined( __MIPSEL__ ) || defined( __MIPSEL ) || defined( _MIPSEL )
#           undef  ALIMER_ARCH_ENDIAN_LITTLE
#           define ALIMER_ARCH_ENDIAN_LITTLE 1
#       else
#           undef  ALIMER_ARCH_ENDIAN_BIG
#           define ALIMER_ARCH_ENDIAN_BIG 1
#       endif
#   elif defined( __AARCH64EB__ ) || defined( __ARMEB__ )
#       undef  ALIMER_ARCH_ENDIAN_BIG
#       define ALIMER_ARCH_ENDIAN_BIG 1
#   else
#       undef  ALIMER_ARCH_ENDIAN_LITTLE
#       define ALIMER_ARCH_ENDIAN_LITTLE 1
#   endif

#   undef ALIMER_PLATFORM_FAMILY_MOBILE
#   define ALIMER_PLATFORM_FAMILY_MOBILE 1
#   undef ALIMER_PLATFORM_FAMILY_CONSOLE
#   define ALIMER_PLATFORM_FAMILY_CONSOLE 1

// Emscripten
#elif defined(__EMSCRIPTEN__)
#   undef  ALIMER_PLATFORM_EMSCRIPTEN
#   define ALIMER_PLATFORM_EMSCRIPTEN 1
#   undef  ALIMER_PLATFORM_POSIX
#   define ALIMER_PLATFORM_POSIX 1
#   define ALIMER_PLATFORM_NAME "Emscripten"
#   undef  ALIMER_ARCH_ENDIAN_LITTLE
#   define ALIMER_ARCH_ENDIAN_LITTLE 1
#   undef ALIMER_PLATFORM_FAMILY_MOBILE
#   define ALIMER_PLATFORM_FAMILY_MOBILE 1
#   undef ALIMER_PLATFORM_FAMILY_CONSOLE
#   define ALIMER_PLATFORM_FAMILY_CONSOLE 1

// macOS and iOS
#elif defined(__APPLE__) 
#   undef  ALIMER_PLATFORM_APPLE
#   define ALIMER_PLATFORM_APPLE 1
#   undef  ALIMER_PLATFORM_POSIX
#   define ALIMER_PLATFORM_POSIX 1
#   include <TargetConditionals.h>

#   if TARGET_OS_IOS  || TARGET_OS_TV || TARGET_OS_WATCH 

#       if TARGET_OS_IOS
#       undef  ALIMER_PLATFORM_IOS
#       define ALIMER_PLATFORM_IOS 1
#       define ALIMER_PLATFORM_NAME "iOS"
#       elif TARGET_OS_TV
#       undef  ALIMER_PLATFORM_TVOS
#       define ALIMER_PLATFORM_TVOS 1
#       define ALIMER_PLATFORM_NAME "tvOS"
#       elif TARGET_OS_WATCH
#       undef  ALIMER_PLATFORM_WATCHOS
#       define ALIMER_PLATFORM_WATCHOS 1
#       define ALIMER_PLATFORM_NAME "watchOS"
#       endif

#       if defined( __arm__ )
#           undef  ALIMER_ARCH_ARM
#           define ALIMER_ARCH_ARM 1
#           if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__)
#               undef  ALIMER_ARCH_ARM7
#               define ALIMER_ARCH_ARM7 1
#               define ALIMER_PLATFORM_DESCRIPTION "iOS ARMv7"
#               ifndef __ARM_NEON__
#               error Missing ARM NEON support
#               endif
#           elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6__)
#               undef  ALIMER_ARCH_ARM6
#               define ALIMER_ARCH_ARM6 1
#               define ALIMER_PLATFORM_DESCRIPTION "iOS ARMv6"
#           else
#               error Unrecognized ARM architecture
#           endif
#       elif defined( __arm64__ )
#           undef  ALIMER_ARCH_ARM
#           define ALIMER_ARCH_ARM 1
#           undef  ALIMER_ARCH_ARM_64
#           define ALIMER_ARCH_ARM_64 1
#           if defined( __ARM64_ARCH_8__ )
#               undef  ALIMER_ARCH_ARM8_64
#               define ALIMER_ARCH_ARM8_64 1
#               define ALIMER_PLATFORM_DESCRIPTION "iOS ARM64v8"
#           else
#               error Unrecognized ARM architecture
#           endif
#       elif defined( __i386__ )
#           undef  ALIMER_ARCH_X86
#           define ALIMER_ARCH_X86 1
#           define ALIMER_PLATFORM_DESCRIPTION "iOS x86 (simulator)"
#       elif defined( __x86_64__ )
#           undef  ALIMER_ARCH_X86_64
#           define ALIMER_ARCH_X86_64 1
#           define ALIMER_PLATFORM_DESCRIPTION "iOS x86_64 (simulator)"
#       else
#           error Unknown architecture
#       endif

#       undef  ALIMER_ARCH_ENDIAN_LITTLE
#       define ALIMER_ARCH_ENDIAN_LITTLE 1

#       undef  ALIMER_PLATFORM_FAMILY_MOBILE
#       define ALIMER_PLATFORM_FAMILY_MOBILE 1

#       undef  ALIMER_PLATFORM_FAMILY_CONSOLE
#       define ALIMER_PLATFORM_FAMILY_CONSOLE 1

#   elif TARGET_OS_MAC 

#       undef  ALIMER_PLATFORM_MACOS
#       define ALIMER_PLATFORM_MACOS 1

#       define ALIMER_PLATFORM_NAME "macOS"

#       if defined( __x86_64__ ) ||  defined( __x86_64 ) || defined( __amd64 )
#           undef  ALIMER_ARCH_X86_64
#           define ALIMER_ARCH_X86_64 1
#           undef  ALIMER_ARCH_ENDIAN_LITTLE
#           define ALIMER_ARCH_ENDIAN_LITTLE 1
#           define ALIMER_PLATFORM_DESCRIPTION "macOS x86-64"
#       elif defined( __i386__ ) || defined( __intel__ )
#           undef  ALIMER_ARCH_X86
#           define ALIMER_ARCH_X86 1
#           undef  ALIMER_ARCH_ENDIAN_LITTLE
#           define ALIMER_ARCH_ENDIAN_LITTLE 1
#           define ALIMER_PLATFORM_DESCRIPTION "macOS x86"
#       elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#           undef  ALIMER_ARCH_PPC_64
#           define ALIMER_ARCH_PPC_64 1
#           undef  ALIMER_ARCH_ENDIAN_BIG
#           define ALIMER_ARCH_ENDIAN_BIG 1
#           define ALIMER_PLATFORM_DESCRIPTION "macOS PPC64"
#       elif defined( __powerpc__ ) || defined( __POWERPC__ )
#           undef  ALIMER_ARCH_PPC
#           define ALIMER_ARCH_PPC 1
#           undef  ALIMER_ARCH_ENDIAN_BIG
#           define ALIMER_ARCH_ENDIAN_BIG 1
#           define ALIMER_PLATFORM_DESCRIPTION "macOS PPC"
#       else
#           error Unknown architecture
#       endif

#       undef  ALIMER_PLATFORM_FAMILY_DESKTOP
#       define ALIMER_PLATFORM_FAMILY_DESKTOP 1

#   else
#       error Unknown Apple Platform
#   endif

// Linux
#elif defined(__linux__) || defined(__linux)
#   undef  ALIMER_PLATFORM_LINUX
#   define ALIMER_PLATFORM_LINUX 1
#   undef  ALIMER_PLATFORM_POSIX
#   define ALIMER_PLATFORM_POSIX 1
#   define ALIMER_PLATFORM_NAME "Linux"

#   if defined( __x86_64__ ) || defined( __x86_64 ) || defined( __amd64 )
#       undef  ALIMER_ARCH_X86_64
#       define ALIMER_ARCH_X86_64 1
#       undef  ALIMER_ARCH_ENDIAN_LITTLE
#       define ALIMER_ARCH_ENDIAN_LITTLE 1
#       define ALIMER_PLATFORM_DESCRIPTION "Linux x86-64"
#   elif defined( __i386__ ) || defined( __intel__ ) || defined( _M_IX86 )
#       undef  ALIMER_ARCH_X86
#       define ALIMER_ARCH_X86 1
#       undef  ALIMER_ARCH_ENDIAN_LITTLE
#       define ALIMER_ARCH_ENDIAN_LITTLE 1
#       define ALIMER_PLATFORM_DESCRIPTION "Linux x86"
#   elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#       undef  ALIMER_ARCH_PPC_64
#       define ALIMER_ARCH_PPC_64 1
#       undef  ALIMER_ARCH_ENDIAN_BIG
#       define ALIMER_ARCH_ENDIAN_BIG 1
#       define ALIMER_PLATFORM_DESCRIPTION "Linux PPC64"
#   elif defined( __powerpc__ ) || defined( __POWERPC__ )
#       undef  ALIMER_ARCH_PPC
#       define ALIMER_ARCH_PPC 1
#       undef  ALIMER_ARCH_ENDIAN_BIG
#       define ALIMER_ARCH_ENDIAN_BIG 1
#       define ALIMER_PLATFORM_DESCRIPTION "Linux PPC"
#   elif defined( __arm__ )
#       undef  ALIMER_ARCH_ARM
#       define ALIMER_ARCH_ARM 1
#       if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__)
#           undef  ALIMER_ARCH_ARM7
#           define ALIMER_ARCH_ARM7 1
#           define ALIMER_PLATFORM_DESCRIPTION "Linux ARMv7"
#           ifndef __ARM_NEON__
#               error Missing ARM NEON support
#           endif
#       elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6ZK__)
#           undef  ALIMER_ARCH_ARM6
#           define ALIMER_ARCH_ARM6 1
#           define ALIMER_PLATFORM_DESCRIPTION "Linux ARMv6"
#       else
#           error Unrecognized ARM architecture
#       endif

#       if defined( __ARMEB__ )
#           undef  ALIMER_ARCH_ENDIAN_BIG
#           define ALIMER_ARCH_ENDIAN_BIG 1
#       else
#           undef  ALIMER_ARCH_ENDIAN_LITTLE
#           define ALIMER_ARCH_ENDIAN_LITTLE 1
#       endif

#   elif defined( __arm64__ ) || defined( __aarch64__ )
#       undef  ALIMER_ARCH_ARM
#       define ALIMER_ARCH_ARM 1
#       undef  ALIMER_ARCH_ARM_64
#       define ALIMER_ARCH_ARM_64 1
#       undef  ALIMER_ARCH_ENDIAN_LITTLE
#       define _ARCH_ENDIAN_LITTLE 1
#       if defined( __ARM64_ARCH_8__ )
#           undef  ALIMER_ARCH_ARM8_64
#           define ALIMER_ARCH_ARM8_64 1
#           define ALIMER_PLATFORM_DESCRIPTION "Linux ARM64v8"
#       else       ALIMER
#           error UALIMERnrecognized ARM architecture
#       endif

#       if defined( __AARCH64EB__ )
#           undef  ALIMER_ARCH_ENDIAN_BIG
#           define ALIMER_ARCH_ENDIAN_BIG 1
#       else
#           undef  ALIMER_ARCH_ENDIAN_LITTLE
#           define ALIMER_ARCH_ENDIAN_LITTLE 1
#       endif

#       else
#           error Unknown architecture
#   endif

#   if defined( __raspberrypi__ )
#       undef  ALIMER_PLATFORM_LINUX_RASPBERRYPI
#       define ALIMER_PLATFORM_LINUX_RASPBERRYPI 1
#   endif

#   undef  ALIMER_PLATFORM_FAMILY_DESKTOP
#   define ALIMER_PLATFORM_FAMILY_DESKTOP 1

// BSD family
#elif defined(__BSD__) || defined(__FreeBSD__)
#   undef  ALIMER_PLATFORM_BSD
#   define ALIMER_PLATFORM_BSD 1
#   undef  ALIMER_PLATFORM_POSIX
#   define ALIMER_PLATFORM_POSIX 1
#   define ALIMER_PLATFORM_NAME "BSD"

#  if defined( __x86_64__ ) || defined( __x86_64 ) || defined( __amd64 )
#    undef  ALIMER_ARCH_X86_64
#    define ALIMER_ARCH_X86_64 1
#    undef  ALIMER_ARCH_ENDIAN_LITTLE
#    define ALIMER_ARCH_ENDIAN_LITTLE 1
#    define ALIMER_PLATFORM_DESCRIPTION "BSD x86-64"
#  elif defined( __i386__ ) || defined( __intel__ ) || defined( _M_IX86 )
#    undef  ALIMER_ARCH_X86
#    define ALIMER_ARCH_X86 1
#    undef  ALIMER_ARCH_ENDIAN_LITTLE
#    define ALIMER_ARCH_ENDIAN_LITTLE 1
#    define ALIMER_PLATFORM_DESCRIPTION "BSD x86"
#  elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#    undef  ALIMER_ARCH_PPC_64
#    define ALIMER_ARCH_PPC_64 1
#    undef  ALIMER_ARCH_ENDIAN_BIG
#    define ALIMER_ARCH_ENDIAN_BIG 1
#    define ALIMER_PLATFORM_DESCRIPTION "BSD PPC64"
#  elif defined( __powerpc__ ) || defined( __POWERPC__ )
#    undef  ALIMER_ARCH_PPC
#    define ALIMER_ARCH_PPC 1
#    undef  ALIMER_ARCH_ENDIAN_BIG
#    define ALIMER_ARCH_ENDIAN_BIG 1
#    define ALIMER_PLATFORM_DESCRIPTION "BSD PPC"
#  else
#    error Unknown architecture
#  endif

#   undef  ALIMER_PLATFORM_FAMILY_DESKTOP
#   define ALIMER_PLATFORM_FAMILY_DESKTOP 1

// Windows
#elif defined(_XBOX_ONE) \
        || (defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_APP) \
        || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)

#   if defined(_XBOX_ONE)
#       undef ALIMER_PLATFORM_XBOXONE
#       define ALIMER_PLATFORM_XBOXONE 1
#       undef  ALIMER_PLATFORM_FAMILY_CONSOLE
#       define ALIMER_PLATFORM_FAMILY_CONSOLE 1
#       define ALIMER_PLATFORM_NAME "XboxOne"
#       define ALIMER_PLATFORM_DESCRIPTION "XboxOne"
#   elif defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_APP
#       undef  ALIMER_PLATFORM_UWP
#       define ALIMER_PLATFORM_UWP 1
#       undef  ALIMER_PLATFORM_FAMILY_DESKTOP
#       define ALIMER_PLATFORM_FAMILY_DESKTOP 1
#       define ALIMER_PLATFORM_NAME "UWP"
#       define ALIMER_PLATFORM_DESCRIPTION "UWP"
#   elif defined(_WIN64) || defined(_WIN32)
#       undef  ALIMER_PLATFORM_WINDOWS
#       define ALIMER_PLATFORM_WINDOWS 1
#       undef  ALIMER_PLATFORM_FAMILY_DESKTOP
#       define ALIMER_PLATFORM_FAMILY_DESKTOP 1
#       define ALIMER_PLATFORM_NAME "Windows"
#   endif

#   if defined(__x86_64__) || defined(_M_AMD64) || defined(_AMD64_)
#       undef  ALIMER_ARCH_X86_64
#       define ALIMER_ARCH_X86_64 1
#       if ALIMER_PLATFORM_WINDOWS
#           define ALIMER_PLATFORM_DESCRIPTION "Windows x86-64"
#       endif
#   elif defined( __x86__ ) || defined( _M_IX86 ) || defined( _X86_ )
#       undef  ALIMER_ARCH_X86
#       define ALIMER_ARCH_X86 1
#       if ALIMER_PLATFORM_WINDOWS
#           define ALIMER_PLATFORM_DESCRIPTION "Windows x86"
#       endif
#   elif defined( __ia64__ ) || defined( _M_IA64 ) || defined( _IA64_ )
#       undef  ALIMER_ARCH_IA64
#       define ALIMER_ARCH_IA64 1
#       if ALIMER_PLATFORM_WINDOWS
#           define ALIMER_PLATFORM_DESCRIPTION "Windows IA-64"
#       endif
#   else
#       error Unknown architecture
#   endif

#   undef  ALIMER_ARCH_ENDIAN_LITTLE
#   define ALIMER_ARCH_ENDIAN_LITTLE 1

#else
#   error Unknown platform
#endif

// Export/Import attribute
#if defined(__CYGWIN32__)
#   define ALIMER_INTERFACE_EXPORT  __declspec(dllexport)
#   define ALIMER_INTERFACE_IMPORT __declspec(dllimport)
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
#   define ALIMER_INTERFACE_EXPORT __declspec(dllexport)
#   define ALIMER_INTERFACE_IMPORT __declspec(dllimport)
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
#   define ALIMER_INTERFACE_EXPORT __attribute__((visibility("default")))
#   define ALIMER_INTERFACE_IMPORT
#else
#   define ALIMER_INTERFACE_EXPORT
#   define ALIMER_INTERFACE_IMPORT
#endif

#if defined(ALIMER_BUILD_SHARED_LIBRARY)
#   define ALIMER_API ALIMER_INTERFACE_EXPORT
#elif defined(ALIMER_USE_SHARED_LIBRARY)
#   define ALIMER_API ALIMER_INTERFACE_IMPORT
#else
#   define ALIMER_API 
#endif

// Utility macros
#define ALIMER_STRINGIZE_(X) #X
#define ALIMER_STRINGIZE(X) ALIMER_STRINGIZE_(X)

#define ALIMER_CONCAT_HELPER(X, Y) X##Y
#define ALIMER_CONCAT(X, Y) ALIMER_CONCAT_HELPER(X, Y)

#define ALIMER_MAKE_FOURCC(_a, _b, _c, _d) \
    (((uint32_t)(_a) | ((uint32_t)(_b) << 8) | ((uint32_t)(_c) << 16) | ((uint32_t)(_d) << 24)))

// Architecture details
#if defined(__SSE2__) || ALIMER_ARCH_X86_64
#   undef  ALIMER_ARCH_SSE2
#   define ALIMER_ARCH_SSE2 1
#endif

#ifdef __SSE3__
#   undef  ALIMER_ARCH_SSE3
#   define ALIMER_ARCH_SSE3 1
#endif

#ifdef __SSE4_1__
#   undef  ALIMER_ARCH_SSE4
#   define ALIMER_ARCH_SSE4 1
#endif

#ifdef __ARM_NEON__
#   undef  ALIMER_ARCH_NEON
#   define ALIMER_ARCH_NEON 1
#endif

#ifdef __thumb__
#   undef  ALIMER_ARCH_THUMB
#   define ALIMER_ARCH_THUMB 1
#endif

// Compilers
#if defined( __clang__ )

#   undef  ALIMER_COMPILER_CLANG
#   define ALIMER_COMPILER_CLANG 1

#   define ALIMER_COMPILER_NAME "clang"
#   define ALIMER_COMPILER_DESCRIPTION ALIMER " " ALIMER_STRINGIZE(__clang_major__) "." ALIMER_STRINGIZE(__clang_minor__)
#   define ALIMER_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#   define ALIMER_RESTRICT __restrict
#   define ALIMER_THREADLOCAL _Thread_local

#   define ALIMER_ATTRIBUTE(x) __attribute__((__##x##__))
#   define ALIMER_ATTRIBUTE2(x,y) __attribute__((__##x##__(y)))
#   define ALIMER_ATTRIBUTE3(x,y,z) __attribute__((__##x##__(y,z)))
#   define ALIMER_ATTRIBUTE4(x,y,z,w) __attribute__((__##x##__(y,z,w)))

#   define ALIMER_DEPRECATED ALIMER_ATTRIBUTE(deprecated)
#   define ALIMER_FORCEINLINE inline ALIMER_ATTRIBUTE(always_inline)
#   define ALIMER_NOINLINE ALIMER_ATTRIBUTE(noinline)
#   define ALIMER_PURECALL ALIMER_ATTRIBUTE(pure)
#   define ALIMER_CONSTCALL ALIMER_ATTRIBUTE(const)
#   define ALIMER_PRINTFCALL(start, num) ALIMER_ATTRIBUTE4(format, printf, start, num)
#   define ALIMER_ALIGN(alignment) ALIMER_ATTRIBUTE2(aligned, alignment)
#   define ALIMER_ALIGNOF(type) __alignof__(type)
#   define ALIMER_ALIGNED_STRUCT(name, alignment) struct __attribute__((__aligned__(alignment))) name

#   define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#   define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#   define ALIMER_NORETURN __attribute__((noreturn))

#   if ALIMER_PLATFORM_WINDOWS
#       if (ALIMER_CLANG_VERSION < 30800)
#           error CLang 3.8 or later is required
#       endif
#       define STDCALL ALIMER_ATTRIBUTE(stdcall)
#       ifndef __USE_MINGW_ANSI_STDIO
#           define __USE_MINGW_ANSI_STDIO 1
#       endif
#       ifndef _CRT_SECURE_NO_WARNINGS
#           define _CRT_SECURE_NO_WARNINGS 1
#       endif
#       ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#           define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 0
#       endif
#       ifndef __MSVCRT_VERSION__
#           define __MSVCRT_VERSION__ 0x0800
#       endif
#       define USE_NO_MINGW_SETJMP_TWO_ARGS 1
#       if __has_warning("-Wunknown-pragmas")
#           pragma clang diagnostic ignored "-Wunknown-pragmas"
#       endif
#       if __has_warning("-Wformat-non-iso")
#           pragma clang diagnostic ignored "-Wformat-non-iso"
#       endif
#   endif

#   if __has_warning("-Wreserved-id-macro")
#       pragma clang diagnostic ignored "-Wreserved-id-macro"
#   endif
#   if __has_warning("-Wcovered-switch-default")
#       pragma clang diagnostic ignored "-Wcovered-switch-default"
#   endif
#   if __has_warning("-Wmissing-field-initializers")
#       pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   endif

#elif defined( __GNUC__ )

#   undef  ALIMER_COMPILER_GCC
#   define ALIMER_COMPILER_GCC 1

#   define ALIMER_COMPILER_NAME "gcc"
#   define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__GNUC__) "." ALIMER_STRINGIZE(__GNUC_MINOR__)

#   define ALIMER_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#   define ALIMER_RESTRICT __restrict
#   define ALIMER_THREADLOCAL __thread

#   define ALIMER_ATTRIBUTE(x) __attribute__((__##x##__))
#   define ALIMER_ATTRIBUTE2(x,y) __attribute__((__##x##__(y)))
#   define ALIMER_ATTRIBUTE3(x,y,z) __attribute__((__##x##__(y,z)))
#   define ALIMER_ATTRIBUTE4(x,y,z,w) __attribute__((__##x##__(y,z,w)))

#   define ALIMER_DEPRECATED ALIMER_ATTRIBUTE(deprecated)
#   define ALIMER_FORCEINLINE inline ALIMER_ATTRIBUTE(always_inline)
#   define ALIMER_NOINLINE ALIMER_ATTRIBUTE(noinline)
#   define ALIMER_PURECALL ALIMER_ATTRIBUTE(pure)
#   define ALIMER_CONSTCALL ALIMER_ATTRIBUTE(const)
#   define ALIMER_PRINTFCALL(start, num) ALIMER_ATTRIBUTE4(format, printf, start, num)
#   define ALIMER_ALIGN(alignment) ALIMER_ATTRIBUTE2(aligned, alignment)
#   define ALIMER_ALIGNOF(type) __alignof__(type)
#   define ALIMER_ALIGNED_STRUCT(name, alignment) struct ALIMER_ALIGN(alignment) name

#   define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#   define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#   define ALIMER_NORETURN __attribute__((noreturn))

#   if ALIMER_PLATFORM_WINDOWS
#       define STDCALL ALIMER_ATTRIBUTE(stdcall)
#       ifndef __USE_MINGW_ANSI_STDIO
#           define __USE_MINGW_ANSI_STDIO 1
#       endif
#       ifndef _CRT_SECURE_NO_WARNINGS
#           define _CRT_SECURE_NO_WARNINGS 1
#       endif
#       ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#           define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 0
#       endif
#       ifndef __MSVCRT_VERSION__
#           define __MSVCRT_VERSION__ 0x0800
#       endif
#       pragma GCC diagnostic ignored "-Wformat"
#       pragma GCC diagnostic ignored "-Wformat-extra-args"
#       pragma GCC diagnostic ignored "-Wpedantic"
#   endif

#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// Intel
#elif defined( __ICL ) || defined( __ICC ) || defined( __INTEL_COMPILER )

#   undef  ALIMER_COMPILER_INTEL
#   define ALIMER_COMPILER_INTEL 1

#   define ALIMER_COMPILER_NAME "intel"
#   if defined( __ICL )
#       define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__ICL)
#   elif defined( __ICC )
#       define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__ICC)
#   endif

#   define ALIMER_RESTRICT __restrict
#   define ALIMER_THREADLOCAL __declspec( thread )

#   define ALIMER_ATTRIBUTE(x)
#   define ALIMER_ATTRIBUTE2(x,y)
#   define ALIMER_ATTRIBUTE3(x,y,z)
#   define ALIMER_ATTRIBUTE4(x,y,z,w)

#   define ALIMER_DEPRECATED
#   define ALIMER_FORCEINLINE __forceinline
#   define ALIMER_NOINLINE __declspec( noinline )
#   define ALIMER_PURECALL
#   define ALIMER_CONSTCALL
#   define ALIMER_PRINTFCALL( start, num )
#   define ALIMER_ALIGN( alignment ) __declspec( align( alignment ) )
#   define ALIMER_ALIGNOF( type ) __alignof( type )
#   define ALIMER_ALIGNED_STRUCT( name, alignment ) ALIMER_ALIGN( alignment ) struct name

#   define ALIMER_LIKELY(x) __builtin_expect(!!(x), 1)
#   define ALIMER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#   define ALIMER_NORETURN  __declspec(noreturn)

#   if ALIMER_PLATFORM_WINDOWS
#       define STDCALL __stdcall
#       define va_copy(d,s) ((d)=(s))
#   endif

#   include <intrin.h>

// Microsoft
#elif defined( _MSC_VER )

#   undef  ALIMER_COMPILER_MSVC
#   define ALIMER_COMPILER_MSVC 1

#   define ALIMER_COMPILER_NAME "msvc"
#   define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(_MSC_VER)

#   define ALIMER_ATTRIBUTE(x)
#   define ALIMER_ATTRIBUTE2(x,y)
#   define ALIMER_ATTRIBUTE3(x,y,z)
#   define ALIMER_ATTRIBUTE4(x,y,z,w)

#   define ALIMER_RESTRICT __restrict
#   define ALIMER_THREADLOCAL __declspec(thread)

#   define ALIMER_DEPRECATED __declspec(deprecated)
#   define ALIMER_FORCEINLINE __forceinline
#   define ALIMER_NOINLINE __declspec(noinline)
#   define ALIMER_PURECALL __declspec(noalias)
#   define ALIMER_CONSTCALL __declspec(noalias)
#   define ALIMER_PRINTFCALL(start, num)
#   define ALIMER_ALIGN(alignment) __declspec(align(alignment))
#   define ALIMER_ALIGNOF(type) __alignof(type)
#   define ALIMER_ALIGNED_STRUCT(name, alignment) ALIMER_ALIGN(alignment) struct name

#   define ALIMER_LIKELY(x) (x)
#   define ALIMER_UNLIKELY(x) (x)
#   define ALIMER_NORETURN  __declspec(noreturn)

#   pragma warning(disable : 4054)
#   pragma warning(disable : 4055)
#   pragma warning(disable : 4127)
#   pragma warning(disable : 4132)
#   pragma warning(disable : 4200)
#   pragma warning(disable : 4204)
#   pragma warning(disable : 4702)
#   pragma warning(disable : 4706)
#   ifdef __cplusplus
#       pragma warning(disable : 4100)
#       pragma warning(disable : 4510)
#       pragma warning(disable : 4512)
#       pragma warning(disable : 4610)
#   endif

#   if ALIMER_PLATFORM_WINDOWS
#       define STDCALL __stdcall
#   endif

#   if defined(ALIMER_COMPILE) && ALIMER_COMPILE && !defined(_CRT_SECURE_NO_WARNINGS)
#       define _CRT_SECURE_NO_WARNINGS 1
#   endif

#   ifndef _LINT
#       define _Static_assert static_assert
#   endif

#   if _MSC_VER < 1800
#       define va_copy(d,s) ((d)=(s))
#   endif

#   include <intrin.h>

#else

#   warning Unknown compiler

#   define ALIMER_COMPILER_NAME "unknown"
#   define ALIMER_COMPILER_DESCRIPTION "unknown"

#   define ALIMER_RESTRICT
#   define ALIMER_THREADLOCAL

#   define ALIMER_DEPRECATED
#   define ALIMER_FORCEINLINE
#   define ALIMER_NOINLINE
#   define ALIMER_PURECALL
#   define ALIMER_CONSTCALL
#   define ALIMER_ALIGN
#   define ALIMER_ALIGNOF
#   define ALIMER_ALIGNED_STRUCT(name, alignment) struct name

#   define ALIMER_LIKELY(x) (x)
#   define ALIMER_UNLIKELY(x) (x)

#endif

#if ALIMER_PLATFORM_POSIX
#   ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#   endif
#endif

#if ALIMER_COMPILER_CLANG
#   pragma clang diagnostic push
#   if __has_warning("-Wundef")
#       pragma clang diagnostic ignored "-Wundef"
#   endif
#   if __has_warning("-Wsign-conversion")
#       pragma clang diagnostic ignored "-Wsign-conversion"
#   endif
#   if __has_warning("-Wunknown-attributes")
#       pragma clang diagnostic ignored "-Wunknown-attributes"
#   endif
#endif

// Base data types
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#if !ALIMER_PLATFORM_WINDOWS
#   include <wchar.h>
#endif
#if (ALIMER_PLATFORM_POSIX && !ALIMER_PLATFORM_APPLE)
#   include <sys/types.h>
#endif

#ifndef __cplusplus
#  define nullptr ((void*)0)
#endif

#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined)
typedef enum {
    false = 0,
    true = 1
} bool;
#endif

#if ALIMER_COMPILER_CLANG
#   pragma clang diagnostic pop
#endif

// Pointer size
#if ALIMER_ARCH_ARM_64 || ALIMER_ARCH_X86_64 || ALIMER_ARCH_PPC_64 || ALIMER_ARCH_IA64 || ALIMER_ARCH_MIPS_64
#   define ALIMER_SIZE_POINTER 8
#else
#   define ALIMER_SIZE_POINTER 4
#endif

// wchar_t size
#if ALIMER_PLATFORM_LINUX_RASPBERRYPI
#   define ALIMER_SIZE_WCHAR 4
#else
#   if WCHAR_MAX > 0xffff
#       define ALIMER_SIZE_WCHAR 4
#   else
#       define ALIMER_SIZE_WCHAR 2
#   endif
#endif

// Misc
#define ALIMER_UNUSED(x) (void)(true ? (void)0 : ((void)(x)))

#ifndef __cplusplus
// static_assert doesn't do anything in MSVC + C compiler, because we just don't have it !
#   ifndef static_assert
#       if ALIMER_COMPILER_MSVC
#           define static_assert(_e, _msg)
#       else
#           define static_assert(_e, _msg) _Static_assert(_e, _msg)
#       endif
#   endif
#endif

#if defined(_MSC_VER) 
#   include <sal.h>
#   define ALIMER_PRINTF_FORMAT_STRING _Printf_format_string_
#   define ALIMER_PRINTF_VARARG_FUNC( fmtargnumber )
#   define ALIMER_SCANF_VARARG_FUNC( fmtargnumber )
#elif defined(__GNUC__)
#   define ALIMER_PRINTF_FORMAT_STRING
#   define ALIMER_PRINTF_VARARG_FUNC( fmtargnumber ) __attribute__ (( format( __printf__, fmtargnumber, fmtargnumber+1 )))
#   define ALIMER_SCANF_VARARG_FUNC( fmtargnumber ) __attribute__ (( format( __scanf__, fmtargnumber, fmtargnumber+1 )))
#else
#   define ALIMER_PRINTF_FORMAT_STRING
#   define ALIMER_PRINTF_VARARG_FUNC( fmtargnumber )
#   define ALIMER_SCANF_VARARG_FUNC( fmtargnumber )
#endif

#if defined(__GNUC__)
#   if defined(__i386__) || defined(__x86_64__)
#       define ALIMER_BREAKPOINT() __asm__ __volatile__("int $3\n\t")
#   else
#       define ALIMER_BREAKPOINT() ((void)0)
#   endif
#   define ALIMER_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#   define ALIMER_BREAKPOINT() __debugbreak()
#   define ALIMER_UNREACHABLE() __assume(false)
#else
#   define ALIMER_BREAKPOINT() ((void)0)
#   define ALIMER_UNREACHABLE()((void)0)
#endif
