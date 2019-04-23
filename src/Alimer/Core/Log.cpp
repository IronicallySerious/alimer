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

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif

#if defined(__ANDROID__)
#	include <android/log.h>
#elif TARGET_OS_IOS || TARGET_OS_TV
#   include <sys/syslog.h>
#elif TARGET_OS_MAC || defined(__linux__)
#   include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#   include <windows.h>
#elif defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#endif

#include "../Core/Log.h"

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
        if (!_isEnabled
            || static_cast<uint8_t>(level) < static_cast<uint8_t>(_level))
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

        __android_log_write(priority, tag_output, msg_output);
#elif TARGET_OS_IOS || TARGET_OS_TV
        int priority = 0;
        switch (level)
        {
        case LogLevel::Trace:
        case LogLevel::Info:
            priority = LOG_INFO;
            break;
        case LogLevel::Debug:
            priority = LOG_DEBUG;
            break;

        case LogLevel::Warn:
            priority = LOG_WARNING;
            break;

        case LogLevel::Error:
            priority = LOG_ERR;
            break;

        case LogLevel::Critical:
            priority = LOG_CRIT;
            break;

        default: break;
        }
        syslog(priority, "%s : %s", tag.c_str(), message.c_str());
#elif TARGET_OS_MAC || defined(__linux__)
#elif defined(_WIN32) || defined(_WIN64)
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

        std::string messageToWrite = StringUtils::Format("%s [%s] : %s\n", tag.c_str(), priority, message.c_str());
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, messageToWrite.c_str(), -1, nullptr, 0);
        if (bufferSize == 0) {
            return;
        }

        std::vector<WCHAR> buffer(bufferSize);
        if (MultiByteToWideChar(CP_UTF8, 0, messageToWrite.c_str(), -1, buffer.data(), bufferSize) == 0) {
            return;
        }

        OutputDebugStringW(buffer.data());

#   if defined(_DEBUG)
        HANDLE handle = 0;
        switch (level)
        {
        case LogLevel::Warn:
        case LogLevel::Error:
        case LogLevel::Critical:
            handle = GetStdHandle(STD_ERROR_HANDLE);
            break;

        default:
            handle = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        }

        if (handle)
        {
            DWORD bytesWritten;
            WriteConsoleW(handle, buffer.data(), static_cast<DWORD>(wcslen(buffer.data())), &bytesWritten, nullptr);
        }
#   endif

#elif defined(__EMSCRIPTEN__)
        int flags = EM_LOG_NO_PATHS;
        switch (level)
        {
        case LogLevel::Trace:
        case LogLevel::Debug:
        case LogLevel::Info:
            flags |= EM_LOG_CONSOLE;
            break;

        case LogLevel::Warn:
            flags |= EM_LOG_CONSOLE | EM_LOG_WARN;
            break;

        case LogLevel::Error:
            flags |= EM_LOG_CONSOLE | EM_LOG_ERROR;
            break;

        case LogLevel::Critical:
            flags |= EM_LOG_CONSOLE | EM_LOG_ERROR | EM_LOG_C_STACK | EM_LOG_JS_STACK | EM_LOG_DEMANGLE;
            break;

        default: break;
        }

        emscripten_log(flags, "%s : %s", tag.c_str(), message.c_str());
#endif
}
}
