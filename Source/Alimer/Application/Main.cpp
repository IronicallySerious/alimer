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

#include  "AlimerConfig.h"
#ifdef ALIMER_TOOLS
#include "../Editor/StudioApplication.h"
#else
#include "../Application/Application.h"
#endif
#include "../Debug/Log.h"

int main(int argc, char** argv)
{
    using namespace Alimer;

#ifdef ALIMER_TOOLS
    StudioApplication app(argc, argv);
#else
    Application app(argc, argv);
#endif
    return app.Run();
}

#if ALIMER_PLATFORM_WINDOWS
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>

#if defined(ALIMER_MINI_DUMPS)
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h>

int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
    BOOL bMiniDumpSuccessful;
    char* szPath = "crash_dumps";
    WCHAR szFileName[MAX_PATH];
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime(&stLocalTime);
    StringCchPrintfW(szFileName, MAX_PATH, L"%s", szPath);
    CreateDirectoryW(szFileName, NULL);

    StringCchPrintfW(szFileName, MAX_PATH,
        L"%s\\%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", szPath,
        stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
        GetCurrentProcessId(), GetCurrentThreadId());
    hDumpFile =
        CreateFileW(szFileName, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    ExpParam.ThreadId = GetCurrentThreadId();
    ExpParam.ExceptionPointers = pExceptionPointers;
    ExpParam.ClientPointers = TRUE;

    bMiniDumpSuccessful =
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
            MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

    //vorticeErrorDialog("Error", "Vortice crashed. A minidump has been generated in 'app/crash_dumps'...");
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif /* ALIMER_MINI_DUMPS */

HINSTANCE alimerHInstance = nullptr;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ALIMER_UNUSED(hPrevInstance);
    ALIMER_UNUSED(lpCmdLine);
    ALIMER_UNUSED(nCmdShow);

    alimerHInstance = hInstance;
    int argc = __argc;
    char** argv = __argv;

#if defined(ALIMER_MINI_DUMPS)
    __try
#endif
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (hr == RPC_E_CHANGED_MODE)
            hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        if (hr != S_OK && hr != S_FALSE)
            ALIMER_LOGERROR("Win32 - Failed to initialize COM");

#if defined(ALIMER_DEV)
        if (!AllocConsole())
            ALIMER_LOGDEBUG("Win32 - Attached to console");
#endif

        int result = main(argc, argv);
        CoUninitialize();
        return result;
    }
#if defined(ALIMER_MINI_DUMPS)
    __except (GenerateDump(GetExceptionInformation()))
    {
        return 1;
    }
#endif
}
#elif ALIMER_PLATFORM_UWP
[Platform::MTAThread] 
int WINAPIV main(Platform::Array<Platform::String^>^)
{
}
#elif ALIMER_PLATFORM_ANDROID
android_app* AlimerAndroidAppState = nullptr;

void android_main(struct android_app *app) 
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Don't strip glue code. Although this is deprecated, it's easier with complex CMake files.
    app_dummy();
#pragma clang diagnostic pop

    AlimerAndroidAppState = app;
    ALIMER_LOGTRACE("android_main");
}
#endif 
