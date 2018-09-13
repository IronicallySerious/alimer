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

#include "../Base/String.h"
#include <memory>
#include <vector>

namespace Alimer
{
    enum class LogLevel : uint8_t
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Off = 6
    };

    /// Listener interface for Log events.
    class ALIMER_API LogListener
    {
    public:
        virtual ~LogListener() { }

        /// Called when message is being logged.
        virtual void MessageLogged(LogLevel level, const String& message) = 0;
    };

    /// Class for logging functionalities.
    class ALIMER_API Logger final
    {
    public:
        /// Construcor.
        Logger();

        /// Destructor.
        ~Logger();

        /// Set logging level.
        void SetLevel(LogLevel newLevel);

        /// Return logging level.
        LogLevel GetLevel() const { return _level; }

        void Log(LogLevel level, const String& message);
        void Trace(const String& message);
        void Debug(const String& message);
        void Info(const String& message);
        void Warn(const String& message);
        void Error(const String& message);

        /// Adds a log listener.
        void AddListener(LogListener* listener);

        /// Removes a log listener.
        void RemoveListener(LogListener* listener);

    private:
        void OnLog(LogLevel level, const String& message);

    private:
        LogLevel _level;
        /// List of Listener's on the Log.
        std::vector<LogListener*> _listeners;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Logger);
    };

    /// Access to Logger module.
    ALIMER_API Logger& gLog();
}

#ifndef ALIMER_DISABLE_LOGGING

#	define ALIMER_LOGTRACE(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Trace, Alimer::String::Format(format, __VA_ARGS__))
#	define ALIMER_LOGDEBUG(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Debug, Alimer::String::Format(format, __VA_ARGS__))
#	define ALIMER_LOGINFO(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Info, Alimer::String::Format(format, __VA_ARGS__))
#	define ALIMER_LOGWARN(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Warn, Alimer::String::Format(format, __VA_ARGS__))
#	define ALIMER_LOGERROR(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Error, Alimer::String::Format(format, __VA_ARGS__))
#	define ALIMER_LOGCRITICAL(format, ...) do \
{ \
	Alimer::gLog().Log(Alimer::LogLevel::Critical, Alimer::String::Format(format, __VA_ARGS__)); \
	ALIMER_BREAKPOINT(); \
	ALIMER_UNREACHABLE(); \
} while (0)

#else

#	define ALIMER_LOGTRACE(...) ((void)0)
#	define ALIMER_LOGDEBUG(...) ((void)0)
#	define ALIMER_LOGINFO(...) ((void)0)
#	define ALIMER_LOGWARN(...) ((void)0)
#	define ALIMER_LOGERROR(...) ((void)0)
#	define ALIMER_LOGCRITICAL(...) ((void)0)
#endif
