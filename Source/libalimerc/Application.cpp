//
// Alimer is based on the Turso3D codebase.
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

#include "alimer_platform.h"
#include "Application.h"

ApplicationProxy::ApplicationProxy(
    ApplicationCallback_T initialize,
    ApplicationCallback_T exit)
    : _initializeCallback(initialize)
    , _exitCallback(exit)
{

}

void ApplicationProxy::Initialize()
{
    _initializeCallback();
}

void ApplicationProxy::OnExiting()
{
    _exitCallback();
}

extern "C"
{
    EXPORT_API ApplicationProxy* Application_new(
        ApplicationCallback_T initialize,
        ApplicationCallback_T exit)
    {
        return new ApplicationProxy(initialize, exit);
    }

    EXPORT_API int Application_Run(ApplicationProxy* _this)
    {
        return _this->Run();
    }

    EXPORT_API void Application_RunFrame(ApplicationProxy* _this)
    {
        _this->RunFrame();
    }

    EXPORT_API void Application_Exit(ApplicationProxy* _this)
    {
        _this->Exit();
    }
}

