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

#include "Alimer.h"
using namespace Alimer;

typedef struct alimer_app_t alimer_app_t;
typedef void(*alimer_app_initialize_delegate)(alimer_app_t* cmanagedApp);
typedef void(*alimer_app_shutdown_delegate)(alimer_app_t* cmanagedApp);

typedef void(*ApplicationCallback_T)(Application*);

static alimer_app_initialize_delegate gInitialize;
static alimer_app_shutdown_delegate gShutdown;

class ApplicationProxy final : public Application
{
public:
    ApplicationProxy();

    virtual ~ApplicationProxy() override;

private:
    void Initialize() override;
    void OnExiting() override;
};

static inline ApplicationProxy* AsManagedApplication(alimer_app_t* chandle) {
    return reinterpret_cast<ApplicationProxy*>(chandle);
}
static inline alimer_app_t* ToManagedApp(ApplicationProxy* _this) {
    return reinterpret_cast<alimer_app_t*>(_this);
}
static inline const alimer_app_t* ToManagedApp(const ApplicationProxy* _this) {
    return reinterpret_cast<const alimer_app_t*>(_this);
}

ApplicationProxy::ApplicationProxy()
{

}

ApplicationProxy::~ApplicationProxy()
{

}

void ApplicationProxy::Initialize()
{
    gInitialize(ToManagedApp(this));
}

void ApplicationProxy::OnExiting()
{
    gShutdown(ToManagedApp(this));
}

#if defined(_MSC_VER)
#   define ALIMER_EXPORT_API extern "C" __declspec(dllexport)
#else
#   define ALIMER_EXPORT_API extern "C"
#endif

ALIMER_EXPORT_API void alimer_app_set_delegates(
    const alimer_app_initialize_delegate pInitialize,
    const alimer_app_shutdown_delegate pShutdown)
{
    gInitialize = pInitialize;
    gShutdown = pShutdown;
}

ALIMER_EXPORT_API alimer_app_t* alimer_app_new()
{
    return ToManagedApp(new ApplicationProxy());
}

ALIMER_EXPORT_API int alimer_app_run(alimer_app_t* app)
{
    return AsManagedApplication(app)->Run();
}

ALIMER_EXPORT_API void alimer_app_run_frame(alimer_app_t* app)
{
    AsManagedApplication(app)->RunFrame();
}

ALIMER_EXPORT_API void alimer_app_run_exit(alimer_app_t* app)
{
    AsManagedApplication(app)->Exit();
}

