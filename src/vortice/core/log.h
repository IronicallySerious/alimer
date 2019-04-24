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

#include "core/platform.h"

#include <stdarg.h>     // va_list
#include <stdint.h>     // uint32_t, int64_t, etc.

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum VorticeLogLevel {
        VORTICE_LOG_LEVEL_TRACE = 0,
        VORTICE_LOG_LEVEL_DEBUG = 1,
        VORTICE_LOG_LEVEL_INFO = 2,
        VORTICE_LOG_LEVEL_WARN = 3,
        VORTICE_LOG_LEVEL_ERROR = 4,
        VORTICE_LOG_LEVEL_CRITICAL = 5,
        VORTICE_LOG_LEVEL_OFF = 6
    } VorticeLogLevel;

    typedef void(*vortice_log_function)(void *userData, VorticeLogLevel level, const char *message);

    VORTICE_API void vortice_log_set_level(VorticeLogLevel level);
    VORTICE_API VorticeLogLevel vortice_log_get_level();

    /// Get the current log output function.
    VORTICE_API void vortice_log_get_function(vortice_log_function *callback, void **userData);
    /// Set the current log output function.
    VORTICE_API void vortice_log_set_function(vortice_log_function callback, void *userData);

    VORTICE_API void vortice_log_print(VorticeLogLevel level, const char *fmt, ...);
    VORTICE_API void vortice_log_vprint(VorticeLogLevel level, const char *fmt, va_list args);
    VORTICE_API void vortice_log_write(VorticeLogLevel level, const char *message);

#ifdef __cplusplus
}
#endif
