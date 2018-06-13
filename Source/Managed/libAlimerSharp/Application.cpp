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


#include "Application.h"

ApplicationProxy::ApplicationProxy(
    ApplicationCallbackB_T setup,
    ApplicationCallback_T initialize,
    ApplicationCallback_T exit)
    : _setupCallback(setup)
    , _initializeCallback(initialize)
    , _exitCallback(exit)
{

}

bool ApplicationProxy::Setup()
{
    return _setupCallback() == 1;
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
    ALIMER_DLL_EXPORT ApplicationProxy* Application_new(
        ApplicationCallbackB_T setup,
        ApplicationCallback_T initialize,
        ApplicationCallback_T exit)
    {
        return new ApplicationProxy(setup, initialize, exit);
    }

    ALIMER_DLL_EXPORT int Application_Run(ApplicationProxy* _this)
    {
        return _this->Run();
    }

    ALIMER_DLL_EXPORT void Application_Tick(ApplicationProxy* _this)
    {
        _this->Tick();
    }

    ALIMER_DLL_EXPORT void Application_Exit(ApplicationProxy* _this)
    {
        _this->Exit();
    }
}

