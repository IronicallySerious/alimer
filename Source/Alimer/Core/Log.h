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

#include "../AlimerConfig.h"

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

		void Log(LogLevel level, const char* message, ...);

	private:
		LogLevel _level;

		DISALLOW_COPY_MOVE_AND_ASSIGN(Logger);
	};

    /// Access to Logger module.
    ALIMER_API Logger& gLog();
}

#ifndef ALIMER_DISABLE_LOGGING

#if defined(_MSC_VER)
extern void __cdecl __debugbreak(void);
#define ALIMER_DEBUG_BREAK() __debugbreak();
#elif defined(__MINGW32__)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#define ALIMER_DEBUG_BREAK() DebugBreak;
#else
#define ALIMER_DEBUG_BREAK()
#endif

#	define ALIMER_LOGTRACE(...) Alimer::gLog().Log(Alimer::LogLevel::Trace, __VA_ARGS__)
#	define ALIMER_LOGDEBUG(...) Alimer::gLog().Log(Alimer::LogLevel::Debug, __VA_ARGS__)
#	define ALIMER_LOGINFO(...) Alimer::gLog().Log(Alimer::LogLevel::Info, __VA_ARGS__)
#	define ALIMER_LOGWARN(...) Alimer::gLog().Log(Alimer::LogLevel::Warn, __VA_ARGS__)
#	define ALIMER_LOGERROR(...) Alimer::gLog().Log(Alimer::LogLevel::Error, __VA_ARGS__)
#	define ALIMER_LOGCRITICAL(...) do \
{ \
	Alimer::gLog().Log(Alimer::LogLevel::Critical, __VA_ARGS__); \
	ALIMER_DEBUG_BREAK(); \
	ALIMER_ASSERT(0); \
	std::exit(-1); \
} while (0)

#else

#	define ALIMER_LOGTRACE(...) ((void)0)
#	define ALIMER_LOGDEBUG(...) ((void)0)
#	define ALIMER_LOGINFO(...) ((void)0)
#	define ALIMER_LOGWARN(...) ((void)0)
#	define ALIMER_LOGERROR(...) ((void)0)
#	define ALIMER_LOGCRITICAL(...) ((void)0)
#endif
