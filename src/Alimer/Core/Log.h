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

#include "../Core/Debug.h"

#if ALIMER_PLATFORM_WINDOWS
#if ALIMER_COMPILER_MSVC
#include "spdlog/sinks/msvc_sink.h"
namespace spdlog
{
    namespace sinks
    {
        using platform_sink_mt = msvc_sink_mt;
    }
}
#else
#include "spdlog/sinks/wincolor_sink.h"
namespace spdlog
{
    namespace sinks
    {
        using platform_sink_mt = wincolor_stdout_sink_mt;
    }
}
#endif
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_MACOS
#include "spdlog/sinks/ansicolor_sink.h"
namespace spdlog
{
    namespace sinks
    {
        using platform_sink_mt = ansicolor_stdout_sink_mt;
    }
}
#elif ALIMER_PLATFORM_ANDROID
#include "spdlog/sinks/android_sink.h"
namespace spdlog
{
    namespace sinks
    {
        using platform_sink_mt = android_sink_mt;
    }
}
#else
#include "spdlog/sinks/ansicolor_sink.h"
namespace spdlog
{
    namespace sinks
    {
        using platform_sink_mt = stdout_sink_mt;
    }
}
#endif
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define ALIMER_LOG "Alimer"
#define ALIMER_LOGTRACE(...) spdlog::get(ALIMER_LOG)->trace(__VA_ARGS__)
#define ALIMER_LOGDEBUG(...) spdlog::get(ALIMER_LOG)->debug(__VA_ARGS__)
#define ALIMER_LOGINFO(...) spdlog::get(ALIMER_LOG)->info(__VA_ARGS__)
#define ALIMER_LOGWARN(...) spdlog::get(ALIMER_LOG)->warn(__VA_ARGS__)
#define ALIMER_LOGERROR(...) spdlog::get(ALIMER_LOG)->error(__VA_ARGS__)
#define ALIMER_LOGCRITICAL(...) do { \
	spdlog::get(ALIMER_LOG)->critical(__VA_ARGS__); \
	ALIMER_BREAKPOINT(); \
	ALIMER_UNREACHABLE(); \
} while (0)
