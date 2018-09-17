//
// Alimer is based on the Turso3D codebase.
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

#include <stddef.h>
#include <stdint.h>

/**
* Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
*/
#define ALIMER_MSVC 0
#define ALIMER_CLANG 0
#define ALIMER_GCC 0

#ifdef _MSC_VER
#	undef ALIMER_MSVC
#	if _MSC_VER >= 1910		// Visual Studio 2017
#		define ALIMER_MSVC 15
#	elif _MSC_VER >= 1900	// Visual Studio 2015
#		define ALIMER_MSVC 14
#	elif _MSC_VER >= 1800	// Visual Studio 2013
#		define ALIMER_MSVC 12
#   elif _MSC_VER >= 1700	// Visual Studio 2012
#       define ALIMER_MSVC 11
#else
#	error "Unknown VC version"
#	endif
#elif defined(__clang__)
#	undef ALIMER_CLANG
#	define ALIMER_CLANG 1
#elif defined(__GNUC__) // note: __clang__ imply __GNUC__
#	undef ALIMER_GCC
#	define ALIMER_GCC 1
#else
#	error "Unknown compiler"
#endif

#define ALIMER_PLATFORM_UWP 0
#define ALIMER_PLATFORM_WINDOWS 0
#define ALIMER_PLATFORM_ANDROID 0
#define ALIMER_PLATFORM_LINUX 0
#define ALIMER_PLATFORM_WEB 0
#define ALIMER_PLATFORM_APPLE 0
#define ALIMER_PLATFORM_APPLE_TV 0
#define ALIMER_PLATFORM_APPLE_IOS 0
#define ALIMER_PLATFORM_APPLE_OSX 0

/**
* Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#	undef ALIMER_PLATFORM_UWP
#	define ALIMER_PLATFORM_UWP 1 // Universal Windows platform
#elif defined(_WIN64) 
#	undef ALIMER_PLATFORM_WINDOWS
#	define ALIMER_PLATFORM_WINDOWS 1
#elif defined(_WIN32) // note: _M_PPC implies _WIN32
#	undef ALIMER_PLATFORM_WINDOWS
#	define ALIMER_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
#	undef ALIMER_PLATFORM_ANDROID
#	define ALIMER_PLATFORM_ANDROID 1
#elif defined(__linux__) || defined (__EMSCRIPTEN__) // note: __ANDROID__ implies __linux__
#	undef ALIMER_PLATFORM_LINUX
#	define ALIMER_PLATFORM_LINUX 1
#	if defined(__EMSCRIPTEN__)
#		undef ALIMER_PLATFORM_WEB
#		define ALIMER_PLATFORM_WEB 1
#	endif
#elif defined(__APPLE__)
#	include <TargetConditionals.h>
#	undef ALIMER_PLATFORM_APPLE
#	define ALIMER_PLATFORM_APPLE 1
#	if (TARGET_OS_TV)
#		undef ALIMER_PLATFORM_APPLE_TV
#		define ALIMER_PLATFORM_APPLE_TV 1
#	elif (TARGET_OS_IPHONE)
#		undef ALIMER_PLATFORM_APPLE_IOS
#		define ALIMER_PLATFORM_APPLE_IOS 1
#	elif (TARGET_OS_MAC)
#		undef ALIMER_PLATFORM_APPLE_OSX
#		define ALIMER_PLATFORM_APPLE_OSX 1
#	else
#		error Unknown Apple platform
#	endif
#endif

/**
* Architecture defines, see http://sourceforge.net/p/predef/wiki/Architectures/
*/
#define ALIMER_X64 0
#define ALIMER_X86 0
#define ALIMER_A64 0
#define ALIMER_ARM 0
#define ALIMER_SPU 0
#define ALIMER_PPC 0

#if defined(__x86_64__) || defined(_M_X64) // ps4 compiler defines _M_X64 without value
#	undef ALIMER_X64
#	define ALIMER_X64 1
#elif defined(__i386__) || defined(_M_IX86)
#	undef ALIMER_X86
#	define ALIMER_X86 1
#elif defined(__arm64__) || defined(__aarch64__)
#	undef ALIMER_A64
#	define ALIMER_A64 1
#elif defined(__arm__) || defined(_M_ARM)
#	undef ALIMER_ARM
#	define ALIMER_ARM 1
#elif defined(__SPU__)
#	undef ALIMER_SPU
#	define ALIMER_SPU 1
#elif defined(__ppc__) || defined(_M_PPC) || defined(__CELLOS_LV2__)
#	undef ALIMER_PPC
#	define ALIMER_PPC 1
#else
#	error "Unknown architecture"
#endif

/**
* SIMD defines
*/
#define ALIMER_SSE2 0
#define ALIMER_NEON 0
#define ALIMER_VMX 0

#if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#	undef ALIMER_SSE2
#	define ALIMER_SSE2 1
#endif
#if defined(_M_ARM) || defined(__ARM_NEON__)
#	undef ALIMER_NEON
#	define ALIMER_NEON 1
#endif
#if defined(_M_PPC) || defined(__CELLOS_LV2__)
#	undef ALIMER_VMX
#	define ALIMER_VMX 1
#endif

// Compiler shortcut
#define ALIMER_GCC_FAMILY (ALIMER_CLANG || ALIMER_GCC)
// OS shortcut
#define ALIMER_WINDOWS_FAMILY (ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_WINDOWS)
#define ALIMER_LINUX_FAMILY (ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_ANDROID)
#define ALIMER_APPLE_FAMILY (ALIMER_PLATFORM_APPLE)
#define ALIMER_UNIX_FAMILY (ALIMER_LINUX_FAMILY || ALIMER_PLATFORM_APPLE)
// Architecture shortcut
#define ALIMER_INTEL_FAMILY (ALIMER_X64 || ALIMER_X86)
#define ALIMER_ARM_FAMILY (ALIMER_ARM || ALIMER_A64)
#define ALIMER_64BIT (ALIMER_X64 || ALIMER_A64)

/**
* noinline macro
*/
#if ALIMER_WINDOWS_FAMILY
#	define ALIMER_NOINLINE __declspec(noinline)
#elif ALIMER_GCC_FAMILY
#	define ALIMER_NOINLINE __attribute__((noinline))
#else
#	define ALIMER_NOINLINE
#endif

/**
* noalias macro
*/
#if ALIMER_WINDOWS_FAMILY
#	define ALIMER_NOALIAS __declspec(noalias)
#else
#	define ALIMER_NOALIAS
#endif

/**
* Inline macro
*/
#define ALIMER_INLINE inline
#if ALIMER_WINDOWS_FAMILY
#	pragma inline_depth( 255 )
#endif

/**
* Force inline macro
*/
#ifndef ALIMER_FORCE_INLINE
#	if defined(__clang___)
#		if __has_attribute(always_inline)
#			define ALIMER_FORCE_INLINE __attribute__((always_inline)) __inline__
#		else
#			define ALIMER_FORCE_INLINE inline
#		endif
#	elif defined(__GNUC__)
#		define ALIMER_FORCE_INLINE __attribute__((always_inline)) __inline__
#	elif defined(_MSC_VER)
#		define ALIMER_FORCE_INLINE __forceinline
#	else
#		define ALIMER_FORCE_INLINE inline
#	endif
#endif

/**
* C++17 features
* @see: https://infektor.net/posts/2017-01-19-using-cpp17-attributes-today.html
*/
#if defined(__GNUC__)
#ifndef __has_cpp_attribute
#   define __has_cpp_attribute(name) 0
#endif

#if __has_cpp_attribute(nodiscard)
#   define NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(gnu::warn_unused_result)
#   define NODISCARD [[gnu::warn_unused_result]]
#else
#   define NODISCARD
#endif

#if __has_cpp_attribute(fallthrough)
#   define FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#   define FALLTHROUGH [[clang::fallthrough]]
#else
#   define FALLTHROUGH
#endif

#if __has_cpp_attribute(maybe_unused)
#   define MAYBE_UNUSED [[maybe_unused]]
#elif __has_cpp_attribute(gnu::unused)
#   define MAYBE_UNUSED [[gnu::unused]]
#else
#   define MAYBE_UNUSED
#endif

#elif defined(_MSC_VER)
#if _MSC_VER >= 1911
#   define NODISCARD [[nodiscard]]
#   define FALLTHROUGH [[fallthrough]]
#   define MAYBE_UNUSED [[maybe_unused]]
#else
#   define NODISCARD
#   define FALLTHROUGH
#   define MAYBE_UNUSED
#endif

#else
#   define NODISCARD
#   define FALLTHROUGH
#   define MAYBE_UNUSED
#endif

#if defined(__GNUC__)

#if defined(__i386__) || defined(__x86_64__)
#   define ALIMER_BREAKPOINT() __asm__ __volatile__("int $3\n\t")
#else
#   define ALIMER_BREAKPOINT() ((void)0)
#endif

#   define ALIMER_UNREACHABLE() __builtin_unreachable()

#elif defined(_MSC_VER)

extern void __cdecl __debugbreak(void);
#define ALIMER_BREAKPOINT() __debugbreak()
#define ALIMER_UNREACHABLE() __assume(false)

#else

#define ALIMER_BREAKPOINT() ((void)0)
#define ALIMER_UNREACHABLE()((void)0)

#endif

#ifndef ALIMER_ASSERT
#	ifdef _DEBUG
#   include <assert.h>
#	define ALIMER_ASSERT(expression) assert(expression)
#	define ALIMER_ASSERT_MSG(expression, msg) assert(expression && msg)
#	else
#	define ALIMER_ASSERT(expression) ((void)0)
#	define ALIMER_ASSERT_MSG(expression, msg) ((void)0)
#	endif
#endif

#if defined (__GNUC__)
#	define ALIMER_UNIX_EXPORT __attribute__((visibility("default")))
#else
#	define ALIMER_UNIX_EXPORT
#endif

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#	define ALIMER_DLL_EXPORT __declspec(dllexport)
#	define ALIMER_DLL_IMPORT __declspec(dllimport)
#elif defined (__GNUC__)
#	define ALIMER_DLL_EXPORT __attribute__((visibility("default")))
#	define ALIMER_DLL_IMPORT
#else
#	define ALIMER_DLL_EXPORT
#	define ALIMER_DLL_IMPORT
#endif

#ifndef _MSC_VER
// SAL annotations
#   define _In_reads_(size)
#endif

#ifdef __cplusplus

#include <type_traits>

// avoid unreferenced parameter warning
// preferred solution: omit the parameter's name from the declaration
template <class T>
ALIMER_INLINE void ALIMER_UNUSED(T const&)
{
}

// Utility to enable bitmask operators on enum classes.
// To use define an enum class with valid bitmask values and an underlying type
// then use the macro to enable support:
//  enum class MyBitmask : uint32_t {
//    Foo = 1 << 0,
//    Bar = 1 << 1,
//  };
//  ALIMER_BITMASK(MyBitmask);
//  MyBitmask value = ~(MyBitmask::Foo | MyBitmask::Bar);
#define ALIMER_BITMASK(enum_class)                                       \
  inline enum_class operator|(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) |       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator|=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) |        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator&(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) &       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator&=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) &        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator^(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) ^       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator^=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) ^        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator~(enum_class lhs) {                        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(~static_cast<enum_type>(lhs));      \
  }                                                                    \
  inline bool any(enum_class lhs) {                                    \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_type>(lhs) != 0;                           \
  }

template <typename T>
constexpr typename std::underlying_type<T>::type ecast(T x)
{
    return static_cast<typename std::underlying_type<T>::type>(x);
}

namespace Alimer
{
    template <typename T>
    void SafeDelete(T *&resource)
    {
        delete resource;
        resource = nullptr;
    }
}

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete; \
	TypeName(const TypeName&&) = delete; \
	TypeName& operator=(const TypeName&&) = delete;

#endif
