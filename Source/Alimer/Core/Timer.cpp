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


#include "../Core/Timer.h"
#include <chrono>

namespace Alimer
{
    Timer::Timer()
        : _idleTime(0)
    {
        Reset();
    }

    void Timer::Reset()
    {
        _start = GetTime();
        _last = _start;
        _lastPeriod = 0;
    }

    double Timer::Frame()
    {
        int64_t newTime = GetTime() - _idleTime;
        _lastPeriod = newTime - _last;
        _last = newTime;
        return double(newTime) * 1e-9;
    }

    void Timer::EnterIdle()
    {
        _idleStart = GetTime();
    }

    void Timer::LeaveIdle()
    {
        int64_t idleEnd = GetTime();
        _idleTime += idleEnd - _idleStart;
    }

    double Timer::GetElapsed() const
    {
        return double(_last - _start) * 1e-9;
    }

    double Timer::GetFrameTime() const
    {
        return double(_lastPeriod) * 1e-9;
    }

    int64_t Timer::GetTime()
    {
        auto current = std::chrono::steady_clock::now().time_since_epoch();
        auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(current);
        return nsecs.count();
    }
}
