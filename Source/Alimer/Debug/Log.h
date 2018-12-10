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

#include "../Base/Ptr.h"
#include "../Core/Object.h"
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
        Count
    };

    class FileStream;

    /// Listener interface for Log events.
    class ALIMER_API LogListener
    {
    public:
        virtual ~LogListener() { }

        /// Called when message is being logged.
        virtual void MessageLogged(LogLevel level, const String& message) = 0;
    };

    /// Class for logging functionalities.
    class ALIMER_API Logger final : public Object
    {
        ALIMER_OBJECT(Logger, Object);

    public:
        /// Construcor.
        Logger();

        /// Destructor.
        ~Logger();

        /// Open the log file.
        void Open(const String& fileName);
        /// Close the log file.
        void Close();

        /// Set logging level.
        void SetLevel(LogLevel newLevel);

        /// Return logging level.
        LogLevel GetLevel() const { return _level; }

        void Log(LogLevel level, const String& tag, const String& message);
        void Trace(const String& tag, const String& message);
        void Debug(const String& tag, const String& message);
        void Info(const String& tag, const String& message);
        void Warn(const String& tag, const String& message);
        void Error(const String& tag, const String& message);

        /// Adds a log listener.
        void AddListener(LogListener* listener);

        /// Removes a log listener.
        void RemoveListener(LogListener* listener);

        /// Return instance of opened log file.
        const FileStream* GetLogFile() const { return _logFile; }

    private:
        void OnLog(LogLevel level, const String& tag, const String& message);

    private:
        LogLevel _level;

        /// Log file.
        FileStream* _logFile;

        /// List of Listener's on the Log.
        std::vector<LogListener*> _listeners;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Logger);
    };

    /// Access to Logger module.
    ALIMER_API Logger& gLog();
}

#ifdef ALIMER_LOGGING
#define ALIMER_LOGTRACE(tag, message) Alimer::gLog().Log(Alimer::LogLevel::Trace, tag, message)
#define ALIMER_LOGDEBUG(tag, message) Alimer::gLog().Log(Alimer::LogLevel::Debug, tag, message)
#define ALIMER_LOGINFO(tag, message) Alimer::gLog().Log(Alimer::LogLevel::Info, tag, message)
#define ALIMER_LOGWARN(tag, message) Alimer::gLog().Log(Alimer::LogLevel::Warn, tag, message)
#define ALIMER_LOGERROR(tag, message) Alimer::gLog().Log(Alimer::LogLevel::Error, tag, message)
#define ALIMER_LOGCRITICAL(tag, message) do { \
	Alimer::gLog().Log(Alimer::LogLevel::Critical, tag, message); \
	ALIMER_BREAKPOINT(); \
	ALIMER_UNREACHABLE(); \
} while (0)

#define ALIMER_LOGTRACEF(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Trace, "alimer", Alimer::String::Format(format, __VA_ARGS__))
#define ALIMER_LOGDEBUGF(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Debug, "alimer", Alimer::String::Format(format, __VA_ARGS__))
#define ALIMER_LOGINFOF(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Info, "alimer", Alimer::String::Format(format, __VA_ARGS__))
#define ALIMER_LOGWARNF(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Warn, "alimer", Alimer::String::Format(format, __VA_ARGS__))
#define ALIMER_LOGERRORF(format, ...) Alimer::gLog().Log(Alimer::LogLevel::Error, "alimer", Alimer::String::Format(format, __VA_ARGS__))
#define ALIMER_LOGCRITICALF(format, ...) do { \
	Alimer::gLog().Log(Alimer::LogLevel::Critical, "alimer", Alimer::String::Format(format, __VA_ARGS__)); \
	ALIMER_BREAKPOINT(); \
	ALIMER_UNREACHABLE(); \
} while (0)

#else
#define ALIMER_LOGTRACE(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#define ALIMER_LOGDEBUG(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#define ALIMER_LOGINFO(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#define ALIMER_LOGWARN(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#define ALIMER_LOGERROR(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#define ALIMER_LOGCRITICAL(tag, message) do { ALIMER_UNUSED(tag); ALIMER_UNUSED(message); } while(0)
#endif
