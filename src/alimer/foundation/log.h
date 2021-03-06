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

#include "foundation/platform.h"
#include <string>
#include <vector>

namespace alimer
{
    /// Enum describing level of logging.
    enum class LogLevel : uint32_t
    {
        /// Trace log level.
        Trace = 0,
        /// Debug log level.
        Debug,
        /// Information log level.
        Info,
        /// Warning log level.
        Warn,
        /// Error log level.
        Error,
        /// Critical/Fatal log level.
        Critical
    };


    /// Defines class for loging capabilities
    class ALIMER_API Logger final
    {
    public:
        /// Constructor.
        Logger();

        /// Destructor.
        virtual ~Logger();


        static Logger &GetDefault();

        void Log(LogLevel level, const std::string& message);
        void Log(LogLevel level, const std::string& tag, const std::string& message);

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

#define ALIMER_TAG "alimer"
#define ALIMER_LOGTRACE(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Trace, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
#define ALIMER_LOGDEBUG(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Debug, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
#define ALIMER_LOGINFO(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Info, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
#define ALIMER_LOGWARN(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Warn, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
#define ALIMER_LOGERROR(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Error, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
#define ALIMER_LOGCRITICAL(...) alimer::Logger::GetDefault().Log(alimer::LogLevel::Critical, ALIMER_TAG, alimer::str::Format(__VA_ARGS__))
