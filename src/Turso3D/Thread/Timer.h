//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "Turso3DConfig.h"
#include "../Base/String.h"

#pragma once

namespace Turso3D
{
    /// Low-resolution operating system timer.
    class TURSO3D_API Timer
    {
    public:
        /// Construct. Get the starting clock value.
        Timer();

        /// Return elapsed milliseconds.
        unsigned ElapsedMSec();
        /// Reset the timer.
        void Reset();

    private:
        /// Starting clock value in milliseconds.
        unsigned startTime;
    };

    /// High-resolution operating system timer used in profiling.
    class TURSO3D_API HiresTimer
    {
    public:
        /// Construct. Get the starting high-resolution clock value.
        HiresTimer();

        /// Return elapsed microseconds.
        long long ElapsedUSec();
        /// Reset the timer.
        void Reset();

        /// Perform one-time initialization to check support and frequency. Is called automatically at program start.
        static void Initialize();
        /// Return if high-resolution timer is supported.
        static bool IsSupported() { return supported; }
        /// Return high-resolution timer frequency if supported.
        static long long Frequency() { return frequency; }

    private:
        /// Starting clock value in CPU ticks.
        long long startTime;

        /// High-resolution timer support flag.
        static bool supported;
        /// High-resolution timer frequency.
        static long long frequency;
    };

    /// Return a date/time stamp as a string.
    TURSO3D_API String TimeStamp();
    /// Return current time as seconds since epoch.
    TURSO3D_API unsigned CurrentTime();
}
