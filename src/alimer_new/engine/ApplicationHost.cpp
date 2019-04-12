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

#include "engine/ApplicationHost.h"
#include "engine/Application.h"

#if defined(_WIN64) || defined(_WIN32)
#   include <Windows.h>
#endif

namespace alimer
{
    ApplicationHost::ApplicationHost(Application* application)
        : _application(application)
    {
       
    }

    void ApplicationHost::ErrorDialog(const std::string& title, const std::string& message)
    {
#if defined(_WIN32)
        MessageBoxA(NULL, message.c_str(), title.c_str(), MB_ICONERROR | MB_OK);
#else
        ALIMER_UNUSED(title);
        ALIMER_UNUSED(message);
#endif
    }

    void ApplicationHost::RequestExit()
    {
        _exitRequested = true;
    }

	void ApplicationHost::InitializeApplication()
	{
        _application->InitializeBeforeRun();
	}

    const std::vector<std::string> &ApplicationHost::GetArguments()
    {
        return _arguments;
    }
}
