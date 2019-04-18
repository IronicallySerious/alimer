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

#pragma once

#include "Turso3DConfig.h"

#ifndef WIN32
#   include <pthread.h>
#endif

namespace Turso3D
{

#ifndef WIN32
    typedef pthread_t ThreadID;
#else
    typedef unsigned ThreadID;
#endif

    /// Operating system thread.
    class TURSO3D_API Thread
    {
    public:
        /// Construct. Does not start the thread yet.
        Thread();
        /// Destruct. If running, stop and wait for thread to finish.
        virtual ~Thread();

        /// The function to run in the thread.
        virtual void ThreadFunction() = 0;

        /// Start running the thread. Return true on success, or false if already running or if can not create the thread.
        bool Run();
        /// Set the running flag to false and wait for the thread to finish.
        void Stop();
        /// Set thread priority. The thread must have been started first.
        void SetPriority(int priority);

        /// Return whether thread exists.
        bool IsStarted() const { return handle != nullptr; }

        /// Sleep the current thread for the specified amount of milliseconds. 0 to just yield the timeslice.
        static void Sleep(unsigned mSec);
        /// Set the current thread as the main thread.
        static void SetMainThread();
        /// Return the current thread's ID.
        static ThreadID CurrentThreadID();
        /// Return whether is executing in the main thread.
        static bool IsMainThread();

    protected:
        /// Thread handle.
        void* handle;
        /// Running flag.
        volatile bool shouldRun;

        /// Main thread's thread ID.
        static ThreadID mainThreadID;
    };

}
