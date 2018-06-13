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

#pragma once

#include "../Core/Application.h"


#if defined(_WIN32) && !defined(ALIMER_WIN32_CONSOLE)
#   include <windows.h>
#	ifdef _MSC_VER
#		include <crtdbg.h>
#	endif
#endif

#if defined(_MSC_VER) && defined(_DEBUG) && !defined(ALIMER_WIN32_CONSOLE) && !ALIMER_PLATFORM_UWP
#define ALIMER_APPLICATION(className) \
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) { \
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
	Alimer::SharedPtr<className> application(new className()); \
	return application->Run(); \
}

// MSVC release mode: write minidump on crash
#elif defined(_MSC_VER) && defined(ALIMER_MINI_DUMPS) && !defined(ALIMER_WIN32_CONSOLE) && !ALIMER_PLATFORM_UWP
#define ALIMER_APPLICATION(className) \
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) { \

	int exitCode; \
	__try \
	{ \
		Alimer::SharedPtr<className> application(new className()); \
		exitCode =  application->Run(); \
	} \
	__except(Alimer::WriteMiniDump("Alimer", GetExceptionInformation())) \
	{ \
	} \
	return exitCode; \
}
// Other Win32 or minidumps disabled: just execute the function
#elif defined(_WIN32) && !defined(ALIMER_WIN32_CONSOLE) && !ALIMER_PLATFORM_UWP
#define ALIMER_APPLICATION(className) \
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) { \
	Alimer::SharedPtr<className> application(new className()); \
	return application->Run(); \
}
#elif ALIMER_PLATFORM_UWP
#define ALIMER_APPLICATION(className) \
[Platform::MTAThread] \
int WINAPIV main(Platform::Array<Platform::String^>^) { \
	Alimer::SharedPtr<className> application(new className()); \
	return application->Run(); \
}
#elif ALIMER_PLATFORM_ANDROID
#define ALIMER_APPLICATION(className) \
android_app* AlimerAndroidAppState = nullptr; \
void android_main(struct android_app *app) { \
#pragma clang diagnostic push \
#pragma clang diagnostic ignored "-Wdeprecated-declarations" \
	app_dummy(); \
#pragma clang diagnostic pop \
	AlimerAndroidAppState = app; \
	Alimer::SharedPtr<className> application(new className()); \
	application->Run(); \
}
#else
#define ALIMER_APPLICATION(className) \
int main(int argc, char** argv) { \
	Alimer::SharedPtr<className> application(new className()); \
	return application->Run(); \
}
#endif

