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

#include "AlimerConfig.h"

namespace Alimer
{
    namespace Assert
    {
        enum FailBehavior
        {
            Halt,
            Continue,
        };

        typedef FailBehavior(*Handler)(const char* condition, const char* msg, const char* file, int line);

        Handler GetHandler();
        void SetHandler(Handler newHandler);

        FailBehavior ReportFailure(const char* condition,
            const char* file,
            int line,
            const char* msg, ...);
    }
}

#if defined(__GNUC__)

#if defined(__i386__) || defined(__x86_64__)
#   define ALIMER_BREAKPOINT() __asm__ __volatile__("int $3\n\t")
#else
#   define ALIMER_BREAKPOINT() ((void)0)
#endif

#   define ALIMER_UNREACHABLE() __builtin_unreachable()

#elif defined(_MSC_VER)

#define ALIMER_BREAKPOINT() __debugbreak()
#define ALIMER_UNREACHABLE() __assume(false)

#else

#define ALIMER_BREAKPOINT() ((void)0)
#define ALIMER_UNREACHABLE()((void)0)

#endif

#if !defined(NDEBUG)

    #define ALIMER_ASSERT(cond) \
        do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, 0) == \
					Alimer::Assert::Halt) \
					ALIMER_BREAKPOINT(); \
			} \
		} while(0)

    #define ALIMER_ASSERT_MSG(cond, msg, ...) \
        do \
		{ \
			if (!(cond)) \
			{ \
				if (Alimer::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
					Alimer::Assert::Halt) \
					ALIMER_BREAKPOINT(); \
			} \
		} while(0)

    #define ALIMER_ASSERT_FAIL(msg, ...) \
		do \
		{ \
			if (Alimer::Assert::ReportFailure(0, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
				Alimer::Assert::Halt) \
			    ALIMER_BREAKPOINT(); \
		} while(0)

    #define ALIMER_VERIFY(cond) ALIMER_ASSERT(cond)
    #define ALIMER_VERIFY_MSG(cond, msg, ...) ALIMER_ASSERT_MSG(cond, msg, ##__VA_ARGS__)

#else

#   define ALIMER_ASSERT(condition) \
		do { ALIMER_UNUSED(condition); } while(0)
#   define ALIMER_ASSERT_MSG(condition, msg, ...) \
		do { ALIMER_UNUSED(condition); ALIMER_UNUSED(msg); } while(0)
#   define ALIMER_ASSERT_FAIL(msg, ...) \
		do { ALIMER_UNUSED(msg); } while(0)
#   define ALIMER_VERIFY(cond) (void)(cond)
#   define ALIMER_VERIFY_MSG(cond, msg, ...) \
		do { (void)(cond); ALIMER_UNUSED(msg); } while(0)
#endif

#define ALIMER_STATIC_ASSERT(x) static_assert(x, #x);
#define ALIMER_STATIC_ASSERT_MSG(x, msg) static_assert(x, #x ", " msg);
