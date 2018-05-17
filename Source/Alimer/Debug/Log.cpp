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

#include "../Debug/Log.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cassert>
#include <vector>

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif

#include <windows.h>
#endif


namespace Alimer
{
	static const char *LogLevelPrefix[static_cast<unsigned>(LogLevel::Off) + 1] = {
		"TRACE",
		"DEBUG",
		"INFO",
		"WARN",
		"ERROR",
		"CRITICAL",
		"OFF"
	};

	static void LogOutput(LogLevel level, const char* message)
	{
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
		size_t length = strlen(LogLevelPrefix[static_cast<unsigned>(level)]) + 2 + strlen(message) + 1 + 1 + 1;
		char* output = new char[length];
		snprintf(output, length, "%s: %s\r\n", LogLevelPrefix[static_cast<unsigned>(level)], message);

		std::vector<wchar_t> szBuffer(length);
		if (MultiByteToWideChar(CP_UTF8, 0, output, -1, szBuffer.data(), static_cast<int>(szBuffer.size())) == 0)
			return;

		OutputDebugStringW(szBuffer.data());

#if defined(ALIMER_DEV) && !ALIMER_PLATFORM_UWP 
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
			WriteConsoleW(handle, szBuffer.data(), static_cast<DWORD>(wcslen(szBuffer.data())), &bytesWritten, nullptr);
		}
#endif 
#endif
	}

	Alimer::Logger* log = new Logger();

	Logger::Logger()
	{
#ifdef _DEBUG
		SetLevel(LogLevel::Debug);
#else
		SetLevel(LogLevel::Info);
#endif

#ifdef ALIMER_DEV
		AllocConsole();
#endif

		log = this;
	}

	Logger::~Logger()
	{
		log = nullptr;
	}

	void Logger::SetLevel(LogLevel newLevel)
	{
		_level = newLevel;
	}

	void Logger::Log(LogLevel level, const char* message, ...)
	{
		if (level == LogLevel::Off || _level >= level)
			return;

		// Declare a moderately sized buffer on the stack that should be
		// large enough to accommodate most log requests.
		int size = 1024;
		char stackBuffer[1024];
		std::vector<char> dynamicBuffer;
		char* str = stackBuffer;
		for (; ; )
		{
			va_list args;
			va_start(args, message);

			// Pass one less than size to leave room for nullptr terminator
			int needed = vsnprintf(str, size - 1, message, args);

			// Some platforms return -1 when vsnprintf runs out of room, while others return
			// the number of characters actually needed to fill the buffer.
			if (needed >= 0 && needed < size)
			{
				// Successfully wrote buffer. Added a nullptr terminator in case it wasn't written.
				str[needed] = '\0';
				va_end(args);
				break;
			}

			size = needed > 0 ? (needed + 1) : (size * 2);
			dynamicBuffer.resize(size);
			str = &dynamicBuffer[0];

			va_end(args);
		}

		// Log to the default output
		LogOutput(level, str);
	}
}
