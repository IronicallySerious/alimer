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

#include "core/log.h"
#include "core/platform.h"
#include <stdio.h>

#if defined(__ANDROID__)
#   include <android/log.h>
#elif defined(_WIN32)
#   include <Windows.h>
#   include <strsafe.h>
#elif defined(__EMSCRIPTEN__)
#   include <emscripten/emscripten.h>
#endif

static const char *s_vortice_log_level_prefixes[VORTICE_LOG_LEVEL_OFF] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRITICAL"
};


#ifdef _DEBUG
static VorticeLogLevel s_log_level = VORTICE_LOG_LEVEL_DEBUG;
#else
static VorticeLogLevel s_log_level = VORTICE_LOG_LEVEL_INFO;
#endif

static void vortice_default_log_function(void *userData, VorticeLogLevel level, const char *message);

static vortice_log_function s_log_function = vortice_default_log_function;
static void* s_log_userData = NULL;

void vortice_log_set_level(VorticeLogLevel level) {
    s_log_level = level;
}

VorticeLogLevel vortice_log_get_level() {
    return s_log_level;
}

void vortice_log_get_function(vortice_log_function *callback, void **userData) {
    if (callback) {
        *callback = s_log_function;
    }
    if (userData) {
        *userData = s_log_userData;
    }
}

void vortice_log_set_function(vortice_log_function callback, void *userData) {
    s_log_function = callback;
    s_log_userData = userData;
}

void vortice_log_print(VorticeLogLevel level, const char *fmt, ...) {
    if (level == VORTICE_LOG_LEVEL_OFF
        || s_log_level > level)
    {
        VORTICE_UNUSED(fmt);
        return;
    }

    va_list args;
    va_start(args, fmt);
    vortice_log_vprint(level, fmt, args);
    va_end(args);
}

void vortice_log_vprint(VorticeLogLevel level, const char *fmt, va_list args) {
    if (level == VORTICE_LOG_LEVEL_OFF
        || s_log_level > level)
    {
        VORTICE_UNUSED(fmt);
        VORTICE_UNUSED(args);
        return;
    }

    char message[4096];
    vsnprintf(message, 4096, fmt, args);
    vortice_log_write(level, message);
}

void vortice_log_write(VorticeLogLevel level, const char *message) {
    if (level == VORTICE_LOG_LEVEL_OFF
        || s_log_level > level)
    {
        VORTICE_UNUSED(message);
        return;
    }

    if (s_log_function) {
        s_log_function(s_log_userData, level, message);
    }
}

void vortice_default_log_function(void *userData, VorticeLogLevel level, const char *message) {
#if defined(__ANDROID__)
    int priority;
    switch (level)
    {
    case VORTICE_LOG_LEVEL_TRACE: priority = ANDROID_LOG_VERBOSE; break;
    case VORTICE_LOG_LEVEL_DEBUG: priority = ANDROID_LOG_DEBUG; break;
    case VORTICE_LOG_LEVEL_INFO: priority = ANDROID_LOG_INFO; break;
    case VORTICE_LOG_LEVEL_WARN: priority = ANDROID_LOG_WARN; break;
    case VORTICE_LOG_LEVEL_ERROR: priority = ANDROID_LOG_ERROR; break;
    case VORTICE_LOG_LEVEL_CRITICAL: priority = ANDROID_LOG_FATAL; break;
    default: priority = ANDROID_LOG_DEFAULT; break;
    }
    __android_log_write(priority, "vortice", message);
#elif TARGET_OS_IOS || TARGET_OS_TV
    int priority = 0;
    switch (level)
    {
    case VORTICE_LOG_LEVEL_ERROR:
        priority = LOG_ERR;
        break;
    case VORTICE_LOG_LEVEL_WARN:
        priority = LOG_WARNING;
        break;
    case VORTICE_LOG_LEVEL_TRACE:
    case VORTICE_LOG_LEVEL_INFO:
        priority = LOG_INFO;
        break;
    case VORTICE_LOG_LEVEL_DEBUG:
        priority = LOG_DEBUG;
        break;
    case VORTICE_LOG_LEVEL_CRITICAL:
        priority = LOG_CRIT;
        break;
    default:
        break;
    }
    syslog(priority, "%s", message);
#elif TARGET_OS_MAC || defined(__linux__)
    int fd = 0;
    switch (level)
    {
    case VORTICE_LOG_LEVEL_TRACE:
    case VORTICE_LOG_LEVEL_DEBUG:
    case VORTICE_LOG_LEVEL_INFO:
        fd = STDOUT_FILENO;
        break;
    case VORTICE_LOG_LEVEL_WARN:
    case VORTICE_LOG_LEVEL_ERROR:
    case VORTICE_LOG_LEVEL_CRITICAL:
        fd = STDERR_FILENO;
        break;
    default: break;
    }

    vortice::Vector<char> output(message, strlen(message));
    output.push_back('\n');

    size_t offset = 0;
    while (offset < output.size())
    {
        ssize_t written = write(fd, output.data() + offset, output.size() - offset);
        if (written == -1)
            return;

        offset += static_cast<size_t>(written);
    }
#elif defined(_WIN32)
    char buffer[4096];
    sprintf(buffer, "[%s]: %s", s_vortice_log_level_prefixes[level], message);

    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, 0, 0);
    if (bufferSize == 0) {
        return;
    }

    ++bufferSize; // for the newline
    WCHAR* wBuffer = (WCHAR*)malloc(sizeof(WCHAR) * bufferSize);
    if (MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wBuffer, bufferSize) == 0) {
        return;
    }

    StringCchCatW(wBuffer, bufferSize, L"\n");
    OutputDebugStringW(wBuffer);

#if defined(_DEBUG) && VORTICE_PLATFORM_WINDOWS
    HANDLE handle = 0;
    switch (level)
    {
    case VORTICE_LOG_LEVEL_WARN:
    case VORTICE_LOG_LEVEL_ERROR:
    case VORTICE_LOG_LEVEL_CRITICAL:
        handle = GetStdHandle(STD_ERROR_HANDLE);
        break;
    default:
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
        break;
    }

    if (handle)
    {
        DWORD bytesWritten;
        WriteConsoleW(handle, wBuffer, (DWORD)wcslen(wBuffer), &bytesWritten, 0);
    }

    free(wBuffer);
#endif

#elif defined(__EMSCRIPTEN__)
    int flags = EM_LOG_NO_PATHS;
    switch (level)
    {
    case VORTICE_LOG_LEVEL_TRACE:
    case VORTICE_LOG_LEVEL_DEBUG:
    case VORTICE_LOG_LEVEL_INFO:
        flags |= EM_LOG_CONSOLE;
        break;

    case VORTICE_LOG_LEVEL_WARN:
        flags |= EM_LOG_CONSOLE | EM_LOG_WARN;
        break;

    case VORTICE_LOG_LEVEL_ERROR:
        flags |= EM_LOG_CONSOLE | EM_LOG_ERROR;
        break;

    case VORTICE_LOG_LEVEL_CRITICAL:
        flags |= EM_LOG_CONSOLE | EM_LOG_ERROR | EM_LOG_C_STACK | EM_LOG_JS_STACK | EM_LOG_DEMANGLE;
        break;

    default: break;
}

    emscripten_log(flags, "%s", message);
#endif
}
