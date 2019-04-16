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

#include "types.h"

#ifdef __ANDROID__
#   include <android/log.h>
#   define alimer_log_verbose(...) __android_log_vprint(ANDROID_LOG_VERBOSE, "ALIMER", __VA_ARGS__)
#   define alimer_log_debug(...) __android_log_vprint(ANDROID_LOG_DEBUG, "ALIMER", __VA_ARGS__)
#   define alimer_log_info(...) __android_log_print(ANDROID_LOG_INFO, "ALIMER", __VA_ARGS__)
#   define alimer_log_warn(...) __android_log_print(ANDROID_LOG_WARN, "ALIMER", __VA_ARGS__)
#   define alimer_log_error(...) __android_log_print(ANDROID_LOG_ERROR, "ALIMER", __VA_ARGS__)
#else
#   include <stdio.h>
#   define alimer_log_verbose(...) printf(__VA_ARGS__)
#   define alimer_log_debug(...) printf(__VA_ARGS__)
#   define alimer_log_info(...) printf(__VA_ARGS__)
#   define alimer_log_warn(...) fprintf(stderr, __VA_ARGS__)
#   define alimer_log_error(...) fprintf(stderr, __VA_ARGS__)
#endif

/// Return the number of CPU cores available.
ALIMER_API uint32_t alimer_get_logical_cores_count(void);

/// Suspends execution for given  milliseconds.
ALIMER_API void alimer_sleep(uint32_t milliseconds);

/// Suspends execution for given seconds.
ALIMER_API void alimer_sleep_seconds(double seconds);

ALIMER_API bool alimer_platform_init(void);
ALIMER_API void alimer_platform_shutdown();
