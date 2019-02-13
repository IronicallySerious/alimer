//
// Copyright (c) 2019 Amer Koleci.
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

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) && defined(VAUDIO_BUILD_SHARED_LIBRARY)
#   define VAUDIO_API __declspec(dllexport)
#elif defined(_WIN32) && defined(VAUDIO_USE_SHARED_LIBRARY)
#   define VAUDIO_API __declspec(dllimport)
#elif defined(__GNUC__) && defined(VAUDIO_BUILD_SHARED_LIBRARY)
#   define VAUDIO_API __attribute__((visibility("default")))
#else
#   define VAUDIO_API
#endif

#ifndef VAUDIO_DEFINE_HANDLE
#   define VAUDIO_DEFINE_HANDLE(object) typedef struct object##_T* object
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define VAUDIO_VERSION_MAJOR                  0
#define VAUDIO_VERSION_MINOR                  1

    typedef enum VAudioLogLevel {
        VAUDIO_LOG_LEVEL_INFO = 0,
        VAUDIO_LOG_LEVEL_WARN,
        VAUDIO_LOG_LEVEL_DEBUG,
        VAUDIO_LOG_LEVEL_ERROR
    } VAudioLogLevel;

    typedef enum VAudioResult {
        VAUDIO_SUCCESS = 0,
        VAUDIO_NOT_READY = 1,
        VAUDIO_TIMEOUT = 2,
        VAUDIO_INCOMPLETE = 3,
        VAUDIO_ALREADY_INITIALIZED = 4,
        VAUDIO_ERROR_GENERIC = -1,
        VAUDIO_ERROR_INITIALIZATION_FAILED = -2,
        VAUDIO_ERROR_TOO_MANY_OBJECTS = -3,
    } VAudioResult;

    typedef enum VAudioChannels {
        VAUDIO_CHANNELS_MONO = 0,
        VAUDIO_CHANNELS_STEREO,
    } VAudioChannels;

    typedef enum VAudioFormat {
        VAUDIO_FORMAT_UNKNOWN = 0,
        VAUDIO_FORMAT_UINT8,
        VAUDIO_FORMAT_SINT16, 
        VAUDIO_FORMAT_SINT24,
        VAUDIO_FORMAT_SINT32,
        VAUDIO_FORMAT_FLOAT32,
        VAUDIO_FORMAT_COUNT
    } VAudioFormat;

    typedef enum VAudioBackend {
        VAUDIO_BACKEND_INVALID = 0,
        VAUDIO_BACKEND_NULL,
        VAUDIO_BACKEND_WASAPI,
        VAUDIO_BACKEND_COUNT
    } VAudioBackend;

    /* Callbacks */
    typedef void(*vaudio_log_fn)(VAudioLogLevel level, const char* context, const char* message);

    typedef struct VAudioDescriptor {
        uint32_t        sampleRate;
        VAudioChannels  channels;
    } VAudioDescriptor;

    VAUDIO_API VAudioBackend vaudioGetBackend();
    VAUDIO_API VAudioResult vaudioInitialize(VAudioDescriptor* descriptor);
    VAUDIO_API void vaudioShutdown();

#ifdef __cplusplus
}
#endif // __cplusplus
