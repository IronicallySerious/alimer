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


#include "core.h"

#ifdef _WIN32
#   include <Windows.h>
#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>

static BOOL alimer_is_wow64() {
    BOOL wow64 = FALSE;

    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (fnIsWow64Process) {
        if (!fnIsWow64Process(GetCurrentProcess(), &wow64)) {
            wow64 = FALSE;
        }
    }

    return wow64;
}

#elif defined(__EMSCRIPTEN__)
#   include <emscripten/emscripten.h>
#   include <emscripten/trace.h>
#   include <emscripten/html5.h>
#   if defined(__EMSCRIPTEN_PTHREADS__)
#       include <emscripten/threading.h>
#   endif
#elif defined(__ANDROID__)
#   include <cpu-features.h>
#   include <dlfcn.h>           // dlopen, dlclose, dlsym
#   include <unistd.h>
#else
#   include <dlfcn.h>           // dlopen, dlclose, dlsym
#   include <unistd.h>
#endif

uint32_t alimer_get_logical_cores_count(void) {

#ifdef _WIN32
#   if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    SYSTEM_INFO sysinfo;
    if (alimer_is_wow64())
    {
        GetNativeSystemInfo(&sysinfo);
    }
    else
#endif /* VORTICE_PLATFORM_WINDOWS */
    {
        GetSystemInfo(&sysinfo);
    }

    return sysinfo.dwNumberOfProcessors;
#elif defined(__EMSCRIPTEN__)
#   if defined(__EMSCRIPTEN_PTHREADS__)
    return emscripten_num_logical_cores();
#   else
    return 1;
#   endif
#elif defined(__ANDROID__)
    return android_getCpuCount();
#else
    return 1;
#endif

}

void alimer_sleep(uint32_t milliseconds) {
#if defined(_WIN32)
    Sleep(milliseconds);
#elif defined(__EMSCRIPTEN__)
    emscripten_sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}


void alimer_sleep_seconds(double seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#elif defined(__EMSCRIPTEN__)
    emscripten_sleep((unsigned int)(seconds * 1000));
#else
    usleep((unsigned int)(seconds * 1000000));
#endif
}

/* Platform setup */
#ifdef _WIN32
static void alimer_glfw_error(int code, const char* description) {
    //alimer_throw(description);
}

bool alimer_platform_init(void)
{
    glfwSetErrorCallback(alimer_glfw_error);
    return glfwInit();
}

void alimer_platform_shutdown()
{
    glfwTerminate();
}
#endif
