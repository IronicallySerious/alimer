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

#include "core/Log.h"
#if defined(__ANDROID__)
#	include <android/log.h>
#elif defined(_WIN32) || defined(_WIN64)
#   include <windows.h>
#endif

namespace alimer
{
    Logger::Logger()
    {
    }

    Logger::~Logger()
    {
    }

    Logger &Logger::GetDefault()
    {
        static Logger defaultLogger;
        return defaultLogger;
    }

    void Logger::Log(LogLevel level, const std::string& message)
    {
        Log(level, "", message);
    }

    void Logger::Log(LogLevel level, const std::string& tag, const std::string& message)
    {
        if (!_isEnabled || static_cast<uint8_t>(level) < static_cast<uint8_t>(_level))
        {
            return;
        }

#if defined(__ANDROID__)
        android_LogPriority priority = ANDROID_LOG_DEFAULT;
        switch (level)
        {
        case LogLevel::Trace:
            priority = ANDROID_LOG_VERBOSE;
            break;
        case LogLevel::Debug:
            priority = ANDROID_LOG_DEBUG;
            break;
        case LogLevel::Info:
            priority = ANDROID_LOG_INFO;
            break;
        case LogLevel::Warn:
            priority = ANDROID_LOG_WARN;
            break;
        case LogLevel::Error:
            priority = ANDROID_LOG_ERROR;
            break;
        case LogLevel::Critical:
            priority = ANDROID_LOG_FATAL;
            break;
        default:
            priority = ANDROID_LOG_DEFAULT;
            break;
        }

        const char *tag_output = tag.empty() ? "alimer" : tag.c_str();
        const char *msg_output = message.c_str();
        // See system/core/liblog/logger_write.c for explanation of return value
        int ret = __android_log_write(priority, tag_output, msg_output);
        if (ret < 0)
        {
            ALIMER_ASSERT_FAIL("__android_log_write() failed, error: %s", ret);
        }

        __android_log_write(priority, tag_.c_str(),
#else
        const char* priority = "INFO";
        switch (level)
        {
        case LogLevel::Trace:
            priority = "TRACE";
            break;
        case LogLevel::Debug:
            priority = "DEBUG";
            break;
        case LogLevel::Info:
            priority = "INFO";
            break;
        case LogLevel::Warn:
            priority = "WARNING";
            break;
        case LogLevel::Error:
            priority = "ERROR";
            break;
        case LogLevel::Critical:
            priority = "CRITICAL";
            break;
        default:
            priority = "INFO";
            break;
        }
        fprintf(stdout, "%s [%s] : ", tag.c_str(), priority);
        fprintf(stdout, message.c_str());
        fprintf(stdout, "\n");

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
        OutputDebugStringA(message.c_str());
        OutputDebugStringA("\n");
#endif

#endif
    }
}
