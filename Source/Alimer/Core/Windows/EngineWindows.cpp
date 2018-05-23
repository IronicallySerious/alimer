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

#include "EngineWindows.h"
#include "WindowWindows.h"
#include "../../Core/Log.h"
#include "../../Input/Windows/InputWindows.h"
#include "../../Audio/WASAPI/AudioWASAPI.h"
#include <shellapi.h>
#include <Ole2.h>
#include <oleidl.h>

static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

HINSTANCE alimerHinstance = nullptr;

namespace Alimer
{
	bool Win32PlatformInitialize()
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (hr == RPC_E_CHANGED_MODE)
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

		if (hr == S_FALSE || FAILED(hr))
		{
			//ALIMER_LOGDEBUG("Failed to initialize COM, error: {}", hr);
			return false;
		}

		// Enable high DPI as SDL not support.
		if (HMODULE shCoreLibrary = ::LoadLibraryW(L"Shcore.dll"))
		{
			typedef HRESULT(WINAPI*SetProcessDpiAwarenessType)(size_t value);
			if (auto fn = GetProcAddress(shCoreLibrary, "SetProcessDpiAwareness"))
				((SetProcessDpiAwarenessType)fn)(2);    // PROCESS_PER_MONITOR_DPI_AWARE

			FreeLibrary(shCoreLibrary);
		}
		else
		{
			if (HMODULE user32Library = ::LoadLibraryW(L"user32.dll"))
			{
				typedef BOOL(WINAPI * PFN_SetProcessDPIAware)(void);

				if (auto fn = GetProcAddress(user32Library, "SetProcessDPIAware"))
					((PFN_SetProcessDPIAware)fn)();

				FreeLibrary(user32Library);
			}
		}

		return true;
	}

	EngineWindows::EngineWindows(LPWSTR commandLine)
	{
		int argc;
		LPWSTR* argv = CommandLineToArgvW(commandLine, &argc);

		if (argc)
		{
			// Skip first one as its executable path.
			char temporaryCString[256];
			for (int i = 1; i < argc; i++)
			{
				WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, temporaryCString, sizeof(temporaryCString), nullptr, nullptr);

				_args.push_back(temporaryCString);
			}

			LocalFree(argv);
		}
	}

	EngineWindows::~EngineWindows()
	{

	}

	int EngineWindows::Run()
	{
		if (!Win32PlatformInitialize())
		{
			//ALIMER_LOGERROR("[Win32] - Failed to setup");
			return EXIT_FAILURE;
		}

		if (!Initialize())
		{
			return EXIT_FAILURE;
		}

		MSG msg;
		while (_running)
		{
			if (!_paused)
			{
				if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);

					if (msg.message == WM_QUIT)
					{
						Exit();
						break;
					}
				}

				// Tick handles pause state.
				Tick();
			}
			else
			{
				BOOL ret = GetMessageW(&msg, nullptr, 0, 0);
				if (ret == 0)
				{
					Exit();
					break;
				}
				else if (ret == -1)
				{
					Exit();
					ALIMER_LOGERROR("[Win32] - Failed to get message");
					return EXIT_FAILURE;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}

				// Tick handles pause state.
				Tick();
			}
		}

		return EXIT_SUCCESS;
	}

	std::shared_ptr<Window> EngineWindows::CreateWindow()
	{
		return std::make_shared<WindowWindows>(alimerHinstance);
	}

	Input* EngineWindows::CreateInput()
	{
		return new InputWindows();
	}

	Audio* EngineWindows::CreateAudio()
	{
		return new AudioWASAPI();
	}

	bool Engine::SetCurrentThreadName(const std::string& name)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name.c_str();
		info.dwThreadID = static_cast<DWORD>(-1);
		info.dwFlags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return true;
	}
}

#if !defined(ALIMER_WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd)
{
	alimerHinstance = hInstance;

#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	int result = EXIT_SUCCESS;
	{
		Alimer::EngineWindows application(GetCommandLineW());
		result = application.Run();
	}

	return result;
}
#else
int main(int argc, char** argv)
{

}

#endif
