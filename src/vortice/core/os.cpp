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
#if defined(_WIN32) || defined(_WIN64)
#   include <Windows.h>

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
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
#endif
#endif

namespace vortice {
    namespace os {

        void sleep(uint32_t milliseconds) {
#if defined(_WIN32) || defined(_WIN64)
            Sleep(milliseconds);
#endif
        }

        /// Suspends execution for given seconds.
        void sleep_seconds(double seconds) {
#if defined(_WIN32) || defined(_WIN64)
            Sleep((DWORD)(seconds * 1000));
#endif
        }

        /// Opens the library at given path.
        void* library_open(const char* path) {
#if defined(_WIN32) || defined(_WIN64)
            return (void*)LoadLibraryA(path);
#else
#endif
        }

        /// Closes a previously loaded library with library_open.
        void library_close(void* library) {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary((HMODULE)library);
#else
#endif
        }

        /// Returns a pointer to the symbol with name from given library.
        void* library_symbol(void* library, const char* name) {
#if defined(_WIN32) || defined(_WIN64)
            return (void*)GetProcAddress((HMODULE)library, name);
#else
#endif
        }

        /// Return the number of CPU cores available.
        uint32_t get_logical_cores_count(void) {
#if defined(_WIN32) || defined(_WIN64)
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

#else
#endif
        }

    } // namespace os
} // namespace vortice
