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

#include "../Core/Log.h"
#include "../IO/FileStream.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <vector>
#include <ctime>

#if VORTEX_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
#include <sys/syslog.h>
#endif

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <strsafe.h>
#endif

namespace Alimer
{
    static const char *LogLevelPrefix[static_cast<unsigned>(LogLevel::Count)] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "CRITICAL"
    };

#if ALIMER_DEV && !ALIMER_PLATFORM_UWP 
    WORD SetConsoleAttribs(HANDLE consoleHandle, WORD attribs)
    {
        CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
        GetConsoleScreenBufferInfo(consoleHandle, &orig_buffer_info);
        WORD back_color = orig_buffer_info.wAttributes;
        // retrieve the current background color
        back_color &= static_cast<WORD>(~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
        // keep the background color unchanged
        SetConsoleTextAttribute(consoleHandle, attribs | back_color);
        return orig_buffer_info.wAttributes; // return orig attribs
    }
#endif

    String GetTimeStamp()
    {
        char dateTime[20];
        time_t sysTime;
        time(&sysTime);
        tm* timeInfo = localtime(&sysTime);
        strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", timeInfo);
        return String(dateTime);
    }

    static Alimer::Logger* __logInstance = nullptr;

    Logger::Logger()
        : _logFile(nullptr)
    {
#ifdef _DEBUG
        SetLevel(LogLevel::Debug);
#else
        SetLevel(LogLevel::Info);
#endif

#if ALIMER_DEV
        AllocConsole();
#endif
        AddSubsystem(this);
        __logInstance = this;
    }

    Logger::~Logger()
    {
        Close();
        RemoveSubsystem(this);
        __logInstance = nullptr;
    }

    void Logger::Open(const String& fileName)
    {
        if (fileName.IsEmpty())
            return;

        if (_logFile && _logFile->IsOpen())
        {
            if (_logFile->GetName() == fileName)
            {
                return;
            }

            Close();
        }

        _logFile = new FileStream();
        if (_logFile->Open(fileName, FileAccess::WriteOnly))
        {
            Log(LogLevel::Info, String::Format("Opened log file '%s'", fileName.CString()));
        }
        else
        {
            SafeDelete(_logFile);
            Log(LogLevel::Error, String::Format("Failed to create log file '%s'", fileName.CString()));
        }
    }

    void Logger::Close()
    {
        if (_logFile
            && _logFile->IsOpen())
        {
            SafeDelete(_logFile);
        }
    }

    void Logger::SetLevel(LogLevel newLevel)
    {
        _level = newLevel;
    }

    void Logger::Log(LogLevel level, const String& message)
    {
        if (_level > level)
            return;

        // Log to the default output
        OnLog(level, message);
    }

    void Logger::Trace(const String& message)
    {
        if (_level > LogLevel::Trace)
            return;

        OnLog(LogLevel::Trace, message);
    }

    void Logger::Debug(const String& message)
    {
        if (_level > LogLevel::Debug)
            return;

        OnLog(LogLevel::Debug, message);
    }

    void Logger::Info(const String& message)
    {
        if (_level > LogLevel::Info)
            return;

        OnLog(LogLevel::Info, message);
    }

    void Logger::Warn(const String& message)
    {
        if (_level > LogLevel::Warn)
            return;

        OnLog(LogLevel::Warn, message);
    }

    void Logger::Error(const String& message)
    {
        if (_level > LogLevel::Error)
            return;

        OnLog(LogLevel::Error, message);
    }

    void Logger::AddListener(LogListener* listener)
    {
        ALIMER_ASSERT(listener);

        _listeners.push_back(listener);
    }

    void Logger::RemoveListener(LogListener* listener)
    {
        ALIMER_ASSERT(listener);


        for (auto it = _listeners.begin(); it != _listeners.end(); ++it)
        {
            if ((*it) == listener)
            {
                _listeners.erase(it);
                break;
            }
        }
    }

    void Logger::OnLog(LogLevel level, const String& message)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        size_t length = strlen(LogLevelPrefix[static_cast<unsigned>(level)]) + 2 + message.Length() + 1 + 1 + 1;
        char* output = new char[length];
        snprintf(output, length, "%s: %s\r\n", LogLevelPrefix[static_cast<unsigned>(level)], message.CString());

        std::vector<wchar_t> szBuffer(length);
        if (MultiByteToWideChar(CP_UTF8, 0, output, -1, szBuffer.data(), static_cast<int>(szBuffer.size())) == 0)
            return;

        OutputDebugStringW(szBuffer.data());

#if ALIMER_DEV && !ALIMER_PLATFORM_UWP 
        HANDLE handle = nullptr;
        switch (level)
        {
        case LogLevel::Warn:
        case LogLevel::Error:
        case LogLevel::Critical:
            handle = GetStdHandle(STD_ERROR_HANDLE);
            break;
        case LogLevel::Trace:
        case LogLevel::Info:
        case LogLevel::Debug:
            handle = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        default: break;
        }

        if (handle)
        {
            DWORD bytesWritten;

            String timeStamp = GetTimeStamp();
            timeStamp += " [";

            WriteConsoleA(handle,
                timeStamp.CString(),
                static_cast<DWORD>(timeStamp.Length()),
                &bytesWritten,
                nullptr);

            const WORD BOLD = FOREGROUND_INTENSITY;
            const WORD RED = FOREGROUND_RED;
            const WORD GREEN = FOREGROUND_GREEN;
            const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
            const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

            DWORD attribs = 0;
            switch (level)
            {
            case LogLevel::Trace:
                attribs = WHITE;
                break;

            case LogLevel::Debug:
                // Cyan 
                attribs = CYAN;
                break;

            case LogLevel::Info:
                attribs = GREEN;
                break;

            case LogLevel::Warn:
                attribs = YELLOW | BOLD;
                break;

            case LogLevel::Error:
                attribs = RED | BOLD;
                break;

            case LogLevel::Critical:
                attribs = BACKGROUND_RED | WHITE | BOLD; // white bold on red background
                break;
            }

            const char* levelMessage = LogLevelPrefix[static_cast<unsigned>(level)];

            DWORD origAttribs = SetConsoleAttribs(handle, attribs);
            WriteConsoleA(handle,
                levelMessage,
                static_cast<DWORD>(strlen(levelMessage)),
                &bytesWritten,
                nullptr);
            ::SetConsoleTextAttribute(handle, origAttribs); // Reset to original colors

            String printMessage = "] ";
            printMessage += message;
            printMessage += "\r\n";

            WriteConsoleA(handle,
                printMessage.CString(),
                static_cast<DWORD>(printMessage.Length()),
                &bytesWritten,
                nullptr);
        }
#endif 
#endif

        if (_logFile)
        {
            String formattedMessage = LogLevelPrefix[static_cast<uint32_t>(level)];
            formattedMessage += ": " + message;

            _logFile->WriteLine(formattedMessage);
            _logFile->Flush();
        }

        // Log listeners.
        for (auto listener : _listeners)
        {
            listener->MessageLogged(level, message);
        }
    }

    Logger& gLog()
    {
        return *__logInstance;
    }
}
