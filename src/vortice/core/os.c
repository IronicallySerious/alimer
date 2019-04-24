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

#include "core/os.h"
#include "core/platform.h"

#if defined(__ANDROID__)
#   include <cpu-features.h>
#   include <stdio.h>           // fputs, fflush
#   include <dlfcn.h>           // dlopen, dlclose, dlsym
#   include <unistd.h>          // unlink, rmdir, getcwd, access
#   include <android/log.h>
#elif defined(_WIN32)
#   include <Windows.h>

#   if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
static inline BOOL win_is_wow64() {
    BOOL wow64 = FALSE;

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (fnIsWow64Process) {
        if (!fnIsWow64Process(GetCurrentProcess(), &wow64)) {
            wow64 = FALSE;
        }
    }

    return wow64;
}
#   endif
#elif defined(__EMSCRIPTEN__)
#   include <stdio.h>           // fputs, fflush
#   include <emscripten/emscripten.h>
#   include <emscripten/trace.h>
#   include <emscripten/html5.h>
#endif

void vortice_sleep(uint32_t milliseconds) {
#if defined(__ANDROID__)
    usleep(milliseconds * 1000);
#elif defined(_WIN32)
    Sleep(milliseconds);
#elif defined(__EMSCRIPTEN__)
    emscripten_sleep(milliseconds);
#endif
}

/// Opens the library at given path.
void* vortice_dlopen(const char* path) {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    return ::dlopen(path, RTLD_LAZY);
#elif defined(_WIN32)
    return (void*)LoadLibraryA(path);
#else
    return nullptr;
#endif
}

/// Closes a previously loaded library with library_open.
void vortice_dlclose(void* library) {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    dlclose(library);
#elif defined(_WIN32)
    FreeLibrary((HMODULE)library);
#else
#endif
}

/// Returns a pointer to the symbol with name from given library.
void* vortice_dlsym(void* library, const char* name) {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    return ::dlsym(library, name);
#elif defined(_WIN32)
    return (void*)GetProcAddress((HMODULE)library, name);
#else
    return nullptr;
#endif
}

/// Return the number of CPU cores available.
uint32_t vortice_get_cpu_count(void) {
#if defined(__ANDROID__)
    return android_getCpuCount();
#elif defined(_WIN32)
#   if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    SYSTEM_INFO sysinfo;
    if (win_is_wow64()) {
        GetNativeSystemInfo(&sysinfo);
    }
    else
#   endif 
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
#endif
}
