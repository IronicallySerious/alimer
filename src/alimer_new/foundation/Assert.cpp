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

#include "foundation/Assert.h"
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace alimer
{
    namespace assert
    {
        FailBehavior DefaultHandler(const char* condition,
            const char* msg,
            const char* file,
            const int line)
        {
            const uint64_t BufferSize = 2048;
            char buffer[BufferSize];
            snprintf(buffer, BufferSize, "%s(%d): Assert Failure: ", file, line);

            if (condition != nullptr) {
                snprintf(buffer, BufferSize, "%s'%s' ", buffer, condition);
            }

            if (msg != nullptr) {
                snprintf(buffer, BufferSize, "%s%s", buffer, msg);
            }

            snprintf(buffer, BufferSize, "%s\n", buffer);

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
            OutputDebugStringA(buffer);
#else
            fprintf(stderr, "%s", buffer);
#endif

            return assert::Halt;
        }

        assert::Handler& GetAssertHandlerInstance()
        {
            static assert::Handler s_handler = &DefaultHandler;
            return s_handler;
        }

        Handler GetHandler()
        {
            return GetAssertHandlerInstance();
        }

        void SetHandler(Handler newHandler)
        {
            GetAssertHandlerInstance() = newHandler;
        }

        FailBehavior ReportFailure(const char* condition, const char* file, const int line, const char* msg, ...)
        {
            const char* message = nullptr;
            if (msg != nullptr)
            {
                char messageBuffer[1024];
                {
                    va_list args;
                    va_start(args, msg);
                    vsnprintf(messageBuffer, 1024, msg, args);
                    va_end(args);
                }

                message = messageBuffer;
            }

            return GetAssertHandlerInstance()(condition, message, file, line);
        }
    }
}
