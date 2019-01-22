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

#include "AlimerConfig.h"

namespace alimer
{
    /// Cross platform Timer class with high precision.
    class ALIMER_API Timer
    {
    public:
        /// Constructor.
        Timer();
        virtual ~Timer() = default;

        /// Reset the timer
        void Reset();

        /// Tick one frame.
        double Frame();

        /// Enter in idle (pause) state.
        void EnterIdle();

        /// Exits from idle (pause) state.
        void LeaveIdle();

        /// Get elapsed time.
        double GetElapsed() const;

        /// Get Frame time.
        double GetFrameTime() const;

    private:
        int64_t GetTime();

        int64_t _start;
        int64_t _last;
        int64_t _lastPeriod;
        int64_t _idleStart;
        int64_t _idleTime;

    private:
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
    };
}
