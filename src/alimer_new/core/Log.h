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

#include <cstdio>
#include <cstdlib>
#include "alimer_config.h"
#include "core/Object.h"

namespace alimer
{
    /// Enum describing level of logging.
    enum class LogLevel : uint8_t
    {
        /// Trace log level.
        Trace = 0,

        /// Debug log level.
        Debug = 1,

        /// Information log level.
        Info = 2,

        /// Warning log level.
        Warn = 3,

        /// Error log level.
        Error = 4,

        /// Critical/Fatal log level.
        Critical = 5
    };

    /// Class for logging support.
    class ALIMER_API Logger final : public Object
    {
        ALIMER_OBJECT(Logger, Object);

    public:
        /// Constructor.
        Logger();

        /// Destructor.
        ~Logger() override;

        void Log(LogLevel level, const std::string& message);

        /// Get if logger is enabled.
        bool IsEnabled() const { return _isEnabled; }

        /// Set logger enabled state.
        void SetEnabled(bool value) { _isEnabled = value; }

        /// Get the log level.
        LogLevel GetLevel() const { return _level; }

        /// Set the log level.
        void SetLevel(LogLevel value) { _level = value; }

    private:
        bool _isEnabled = true;
#ifdef _DEBUG
        LogLevel _level = LogLevel::Debug;
#else
        LogLevel _level = LogLevel::Info;
#endif
        
    };
}

#ifdef ALIMER_LOGGING

#define ALIMER_TAG "alimer"

#if defined(__ANDROID__)
#	include <android/log.h>
#	define LOGI(...) __android_log_print(ANDROID_LOG_INFO, ALIMER_TAG, __VA_ARGS__)
#	define LOGW(...) __android_log_print(ANDROID_LOG_WARN, ALIMER_TAG, __VA_ARGS__)
#	define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ALIMER_TAG, __VA_ARGS__)
#else
#	define LOGI(...) \
		{ \
			fprintf(stdout, "%s [INFO] : ", ALIMER_TAG); \
			fprintf(stdout, __VA_ARGS__); \
			fprintf(stdout, "\n"); \
		}
#	define LOGW(...) \
		{ \
			fprintf(stdout, "%s [WARNING] : ", ALIMER_TAG); \
			fprintf(stdout, __VA_ARGS__); \
			fprintf(stdout, "\n"); \
		}
#	define LOGE(...) \
		{ \
			fprintf(stderr, "%s [ERROR] : ", ALIMER_TAG); \
			fprintf(stderr, __VA_ARGS__); \
			fprintf(stderr, "\n"); \
		}
#endif

#else
#   define LOGI(...) ((void)0)
#   define LOGW(...) ((void)0)
#   define LOGE(...) ((void)0)
#endif /* ALIMER_LOGGING */
