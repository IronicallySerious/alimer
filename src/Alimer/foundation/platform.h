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
#       else       
#           error Unrecognized ARM architecture
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
#if defined(ALIMER_SHARED_LIBRARY) && ALIMER_PLATFORM_WINDOWS
#   define FOUNDATION_EXPORT_LINK __declspec(dllexport)
#   define FOUNDATION_IMPORT_LINK __declspec(dllimport)
#else
#   define FOUNDATION_EXPORT_LINK
#   define FOUNDATION_IMPORT_LINK
#endif

#if ALIMER_COMPILE
#   ifdef __cplusplus
#       define FOUNDATION_EXTERN extern "C" FOUNDATION_IMPORT_LINK
#       define FOUNDATION_API extern "C" FOUNDATION_EXPORT_LINK
#   else
#       define FOUNDATION_EXTERN extern FOUNDATION_IMPORT_LINK
#       define FOUNDATION_API extern FOUNDATION_EXPORT_LINK
#   endif
#   define ALIMER_API FOUNDATION_EXPORT_LINK
#else
#   ifdef __cplusplus
#       define FOUNDATION_EXTERN extern "C" FOUNDATION_IMPORT_LINK
#       define FOUNDATION_API extern "C" FOUNDATION_IMPORT_LINK
#   else
#       define FOUNDATION_EXTERN extern FOUNDATION_IMPORT_LINK
#       define FOUNDATION_API extern FOUNDATION_IMPORT_LINK
#   endif
#   define ALIMER_API FOUNDATION_IMPORT_LINK
#endif

// Utility macros
#define ALIMER_STRINGIZE_HELPER(X) #X
#define ALIMER_STRINGIZE(X) ALIMER_STRINGIZE_HELPER(X)

#define ALIMER_CONCAT_HELPER(X, Y) X##Y
#define ALIMER_CONCAT(X, Y) ALIMER_CONCAT_HELPER(X, Y)

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
#if defined(__clang__)

#   undef  ALIMER_COMPILER_CLANG
#   define ALIMER_COMPILER_CLANG 1

#   define ALIMER_COMPILER_NAME "clang"
#   define ALIMER_COMPILER_DESCRIPTION ALIMER_COMPILER_NAME " " ALIMER_STRINGIZE(__clang_major__) "." ALIMER_STRINGIZE(__clang_minor__)
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

#   if ALIMER_PLATFORM_WINDOWS
#       define STDCALL __stdcall
#       define va_copy(d,s) ((d)=(s))
#   endif

#   include <intrin.h>

// Microsoft
#elif defined(_MSC_VER)

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
#   define nullptr ((void*)0)
#endif

#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined)
typedef enum {
    false = 0,
    true = 1
} bool;
#endif

#if ALIMER_COMPILER_MSVC
typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;
#endif

#if ALIMER_COMPILER_CLANG
#   pragma clang diagnostic pop
#endif

typedef float  float32_t;
typedef double float64_t;

struct uint128_t {
    uint64_t word[2];
};
typedef struct uint128_t uint128_t;

struct uint256_t {
    uint64_t word[4];
};
typedef struct uint256_t uint256_t;

#define FLOAT32_C(x)   (x##f)
#define FLOAT64_C(x)   (x)

#define ALIMER_SIZE_REAL 4

#if ALIMER_SIZE_REAL == 8
typedef   float64_t    real;
#   define REAL_C(x)    FLOAT64_C(x)
#else
typedef   float32_t    real;
#   define REAL_C(x)    FLOAT32_C(x)
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

// Macro for determining size of arrays.
#if defined(_MSC_VER)
#   define ALIMER_ARRAYSIZE(arr) _countof(arr)
#else
#   define ALIMER_ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif


#if ALIMER_COMPILER_MSVC || defined(__STDC_NO_ATOMICS__)

//Atomic types
ALIMER_ALIGNED_STRUCT(atomic32_t, 4) {
    volatile int32_t nonatomic;
};
typedef struct atomic32_t atomic32_t;

ALIMER_ALIGNED_STRUCT(atomic64_t, 8) {
    volatile int64_t nonatomic;
};
typedef struct atomic64_t atomic64_t;

ALIMER_ALIGNED_STRUCT(atomicptr_t, ALIMER_SIZE_POINTER) {
    volatile void* nonatomic;
};
typedef struct atomicptr_t atomicptr_t;

#else

typedef volatile _Atomic(int32_t) atomic32_t;
typedef volatile _Atomic(int64_t) atomic64_t;
typedef volatile _Atomic(void*) atomicptr_t;

#endif

// Pointer arithmetic
#define pointer_offset(ptr, ofs) (void*)((char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_offset_const(ptr, ofs) (const void*)((const char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second) (ptrdiff_t)((const char*)(first) - (const char*)(second))

#include <string.h>

// String argument helpers
#define STRING_CONST(s) (s), (sizeof((s))-1)
#define STRING_ARGS(s) (s).str, (s).length
#define STRING_ARGS_CAPACITY(s) (s).str, (s).length, (s).length+1
#define STRING_FORMAT(s) (int)(s).length, (s).str

//Format specifiers for 64bit and pointers
#if ALIMER_COMPILER_MSVC
#   define PRId32       "Id"
#   define PRIi32       "Ii"
#   define PRIo32       "Io"
#   define PRIu32       "Iu"
#   define PRIx32       "Ix"
#   define PRIX32       "IX"
#   define PRId64       "I64d"
#   define PRIi64       "I64i"
#   define PRIo64       "I64o"
#   define PRIu64       "I64u"
#   define PRIx64       "I64x"
#   define PRIX64       "I64X"
#   define PRIdPTR      "Id"
#   define PRIiPTR      "Ii"
#   define PRIoPTR      "Io"
#   define PRIuPTR      "Iu"
#   define PRIxPTR      "Ix"
#   define PRIXPTR      "IX"
#   define PRIsize      "Iu"
#else
#   include <inttypes.h>
#   define PRIsize      "zu"
#endif

#define PRItick        PRIi64
#define PRIhash        PRIx64

#if ALIMER_SIZE_REAL == 8
#   define PRIreal      "lf"
#else
#   define PRIreal      "f"
#endif

#if ALIMER_COMPILER_MSVC
#   if ALIMER_SIZE_POINTER == 8
#       define PRIfixPTR  "016I64X"
#   else
#       define PRIfixPTR  "08IX"
#   endif
#else
#  if ALIMER_SIZE_POINTER == 8
#    define PRIfixPTR  "016" PRIXPTR
#  else
#    define PRIfixPTR  "08" PRIXPTR
#  endif
#endif

// Misc
#if defined(__COVERITY__)
#   define ALIMER_UNUSED(x) ((void)(x))
#elif ALIMER_COMPILER_GCC
#   define ALIMER_UNUSED(x) ((void)sizeof((x)))
#else
#   define ALIMER_UNUSED(x) (/*lint --e{505,550,818,866} */(void)sizeof((x), 0))
#endif

// Version
#define ALIMER_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define ALIMER_VERSION_GET_MAJOR(version) ((unsigned)(version) >> 22)
#define ALIMER_VERSION_GET_MINOR(version) (((unsigned)(version) >> 12) & 0x3ff)
#define ALIMER_VERSION_GET_PATCH(version) ((unsigned)(version) & 0xfff)

#define ALIMER_DECLARE_THREAD_LOCAL(type, name, init) \
static ALIMER_THREADLOCAL type _thread_##name = init; \
static ALIMER_FORCEINLINE void set_thread_##name(type val) { _thread_##name = val; } \
static ALIMER_FORCEINLINE type get_thread_##name(void) { return _thread_##name; }

#define ALIMER_DECLARE_THREAD_LOCAL_ARRAY(type, name, arrsize) \
static ALIMER_THREADLOCAL type _thread_##name [arrsize] = {0}; \
static ALIMER_FORCEINLINE type* get_thread_##name(void) { return _thread_##name; }

// Utility functions for large integer types
static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint128_t uint128_make(const uint64_t w0, const uint64_t w1);

static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint128_t uint128_null(void);

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint128_equal(const uint128_t u0, const uint128_t u1);

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint128_is_null(const uint128_t u0);

static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint256_t uint256_make(const uint64_t w0, const uint64_t w1, const uint64_t w2, const uint64_t w3);
static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint256_t uint256_null(void);
static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint256_equal(const uint256_t u0, const uint256_t u1);
static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint256_is_null(const uint256_t u0);

// Implementations
static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint128_t uint128_make(const uint64_t w0, const uint64_t w1) {
    uint128_t u = { { w0, w1 } };
    return u;
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint128_t uint128_null(void) {
    return uint128_make(0, 0);
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint128_equal(const uint128_t u0, const uint128_t u1) {
    return u0.word[0] == u1.word[0] && u0.word[1] == u1.word[1];
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint128_is_null(const uint128_t u0) {
    return !u0.word[0] && !u0.word[1];
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint256_t
uint256_make(const uint64_t w0, const uint64_t w1, const uint64_t w2, const uint64_t w3) {
    uint256_t u = { { w0, w1, w2, w3 } };
    return u;
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL uint256_t uint256_null(void) {
    return uint256_make(0, 0, 0, 0);
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint256_equal(const uint256_t u0, const uint256_t u1) {
    return u0.word[0] == u1.word[0] && u0.word[1] == u1.word[1] &&
        u0.word[2] == u1.word[2] && u0.word[3] == u1.word[3];
}

static ALIMER_FORCEINLINE ALIMER_CONSTCALL bool uint256_is_null(const uint256_t u0) {
    return !u0.word[0] && !u0.word[1] && !u0.word[2] && !u0.word[3];
}
