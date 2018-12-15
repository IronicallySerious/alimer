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

#include "Editor.h"

namespace Alimer
{
    Editor::Editor()
    {
        _settings.preferredGraphicsBackend = GraphicsBackend::D3D11;
        // _settings.preferredGraphicsBackend = GraphicsBackend::Vulkan;
    }

    Editor::~Editor()
    {
        //ui::ShutdownDock();
    }

    void Editor::Initialize()
    {
        _mainWindow->SetTitle("Alimer Studio 2018");
    }

    void Editor::OnRenderFrame(double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        if (_input.IsMouseButtonHeld(MouseButton::Left))
        {
            ALIMER_LOGINFO("Mouse left button is held");
        }
    }
}

int main(int argc, char** argv)
{
    using namespace Alimer;

    Editor app;
    int returnCode = app.Run(argc, argv);
    return returnCode;
}

#if ALIMER_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
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

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ALIMER_UNUSED(hInstance);
    ALIMER_UNUSED(hPrevInstance);
    ALIMER_UNUSED(lpCmdLine);
    ALIMER_UNUSED(nCmdShow);
    int returnCode = 0;
    __try
    {
        int argc = __argc;
        char** argv = __argv;
        returnCode = main(argc, argv);
    }
    __except (GenerateDump(GetExceptionInformation()))
    {
        
    }

    return returnCode;
}
#endif

