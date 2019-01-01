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

#include <stddef.h>
#include <stdint.h>

// CPU
#define ALIMER_CPU_ARM   0
#define ALIMER_CPU_JIT   0
#define ALIMER_CPU_MIPS  0
#define ALIMER_CPU_PPC   0
#define ALIMER_CPU_RISCV 0
#define ALIMER_CPU_X86   0

// Architecture
#define ALIMER_32BITS 0
#define ALIMER_64BITS 0

// Endianess
#define ALIMER_BIG_ENDIAN    0
#define ALIMER_LITTLE_ENDIAN 0

// Compiler
#define ALIMER_COMPILER_MSVC 0
#define ALIMER_COMPILER_CLANG 0
#define ALIMER_COMPILER_GCC 0

// SIMD defines
#define ALIMER_SSE2 0
#define ALIMER_NEON 0
#define ALIMER_VMX  0

/**
* Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
*/
#ifdef _MSC_VER
#	undef ALIMER_COMPILER_MSVC
#	if _MSC_VER >= 1910		// Visual Studio 2017
#		define ALIMER_COMPILER_MSVC 15
#	elif _MSC_VER >= 1900	// Visual Studio 2015
#		define ALIMER_COMPILER_MSVC 14
#	elif _MSC_VER >= 1800	// Visual Studio 2013
#		define ALIMER_COMPILER_MSVC 12
#   elif _MSC_VER >= 1700	// Visual Studio 2012
#       define ALIMER_COMPILER_MSVC 11
#else
#	error "Unknown VC version"
#	endif
#elif defined(__clang__)
#	undef ALIMER_COMPILER_CLANG
#	define ALIMER_COMPILER_CLANG 1
#elif defined(__GNUC__) // note: __clang__ imply __GNUC__
#	undef ALIMER_COMPILER_GCC
#	define ALIMER_COMPILER_GCC 1
#else
#	error "Unknown compiler"
#endif

#define ALIMER_PLATFORM_WINDOWS     0
#define ALIMER_PLATFORM_UWP         0
#define ALIMER_PLATFORM_XBOX_ONE    0
#define ALIMER_PLATFORM_IOS         0
#define ALIMER_PLATFORM_TVOS        0
#define ALIMER_PLATFORM_MACOS       0
#define ALIMER_PLATFORM_ANDROID     0
#define ALIMER_PLATFORM_LINUX       0
#define ALIMER_PLATFORM_WEB         0

/**
* Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  ALIMER_PLATFORM_XBOX_ONE
#	define ALIMER_PLATFORM_XBOX_ONE 1
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#	undef ALIMER_PLATFORM_UWP
#	define ALIMER_PLATFORM_UWP 1 // Universal Windows platform
#elif defined(_WIN64) || defined(_WIN32) // Windows
#	undef ALIMER_PLATFORM_WINDOWS
#	define ALIMER_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) // macOS, iOS, tvOS
#   include <TargetConditionals.h>
#   if TARGET_OS_IOS
#       undef ALIMER_PLATFORM_IOS
#       define ALIMER_PLATFORM_IOS 1
#   elif TARGET_OS_TV
#       undef ALIMER_PLATFORM_TVOS
#       define ALIMER_PLATFORM_TVOS 1
#   elif TARGET_OS_MAC
#       undef ALIMER_PLATFORM_MACOS 
#       define ALIMER_PLATFORM_MACOS 1
#   endif
#elif defined(__ANDROID__)
#	undef ALIMER_PLATFORM_ANDROID
#	define ALIMER_PLATFORM_ANDROID 1
#   define ALIMER_SUPPORTS_OPENGL 1
#   define ALIMER_SUPPORTS_OPENGLES 1
#   define ALIMER_OPENGL_INTERFACE_EGL 1
#elif defined(__linux__) 
#	undef ALIMER_PLATFORM_LINUX
#	define ALIMER_PLATFORM_LINUX 1
#   define ALIMER_SUPPORTS_OPENGL 1
#elif defined(__EMSCRIPTEN__) // Emscripten
#   undef ALIMER_PLATFORM_WEB
#   define ALIMER_PLATFORM_WEB 1
#   define ALIMER_SUPPORTS_OPENGL 1
#   define ALIMER_SUPPORTS_OPENGLES 1
#   define ALIMER_OPENGL_INTERFACE_EGL 1
#endif

/**
* Architecture defines, see http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
*/
#if defined(__arm__)     || \
	defined(__aarch64__) || \
	defined(_M_ARM)
#	undef  ALIMER_CPU_ARM
#	define ALIMER_CPU_ARM 1
#	define ALIMER_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     || \
	  defined(__mips_isa_rev) || \
	  defined(__mips64)
#	undef  ALIMER_CPU_MIPS
#	define ALIMER_CPU_MIPS 1
#	define ALIMER_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        || \
	  defined(__powerpc__)   || \
	  defined(__powerpc64__)
#	undef  ALIMER_CPU_PPC
#	define ALIMER_CPU_PPC 1
#	define ALIMER_CACHE_LINE_SIZE 128
#elif defined(__riscv)   || \
	  defined(__riscv__) || \
	  defined(RISCVEL)
#	undef  ALIMER_CPU_RISCV
#	define ALIMER_CPU_RISCV 1
#	define ALIMER_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    || \
	  defined(_M_X64)     || \
	  defined(__i386__)   || \
	  defined(__x86_64__)
#	undef  ALIMER_CPU_X86
#	define ALIMER_CPU_X86 1
#	define ALIMER_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  ALIMER_CPU_JIT
#	define ALIMER_CPU_JIT 1
#	define ALIMER_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    || \
	defined(_M_X64)        || \
	defined(__aarch64__)   || \
	defined(__64BIT__)     || \
	defined(__mips64)      || \
	defined(__powerpc64__) || \
	defined(__ppc64__)     || \
	defined(__LP64__)
#	undef  ALIMER_64BITS
#	define ALIMER_64BITS 1
#else
#	undef  ALIMER_32BITS
#	define ALIMER_32BITS 1
#endif

#if ALIMER_CPU_PPC
#	undef  ALIMER_BIG_ENDIAN
#	define ALIMER_BIG_ENDIAN 1
#else
#	undef  ALIMER_LITTLE_ENDIAN
#	define ALIMER_LITTLE_ENDIAN 1
#endif 

/**
* SIMD defines
*/
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

/**
* noinline macro
*/
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#	define ALIMER_NOINLINE __declspec(noinline)
#elif ALIMER_COMPILER_CLANG || ALIMER_COMPILER_GCC
#	define ALIMER_NOINLINE __attribute__((noinline))
#else
#	define ALIMER_NOINLINE
#endif

/**
* noalias macro
*/
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#	define ALIMER_NOALIAS __declspec(noalias)
#else
#	define ALIMER_NOALIAS
#endif

/**
* Inline macro
*/
#define ALIMER_INLINE inline
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
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

#ifndef ALIMER_UNUSED
#   define ALIMER_UNUSED(x) do { (void)sizeof(x); } while(0)
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

#ifndef _MSC_VER
// SAL annotations
#   define _In_reads_(size)
#endif

#ifdef __cplusplus

#include <type_traits>

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

namespace alimer
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
