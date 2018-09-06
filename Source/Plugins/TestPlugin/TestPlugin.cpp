/*
** Alimer - Copyright (C) 2016-2018 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#define SDL_MAIN_HANDLED
#include <SDL.h>

extern "C" __declspec(dllexport) int AlimerPluginLoad()
{
    SDL_SetMainReady();
    int result = SDL_Init(
        SDL_INIT_VIDEO
        | SDL_INIT_GAMECONTROLLER
        | SDL_INIT_HAPTIC
        | SDL_INIT_TIMER);
    return result;
}

namespace alimer
{
    class __declspec(dllexport) TestClass
    {
    public:
        TestClass();
    };

    TestClass::TestClass() {

    }
}

/*
#include "Alimer.h"
using namespace Alimer;

const std::string sPluginName = "TestPlugin";

class TestPlugin final : public Alimer::Plugin
{
public:
    const std::string& GetName() const { return sPluginName; }

    void Install() override {

    }

    void Uninstall()  override {

    }

    void Initialize() override {

    }

    void Shutdown() override {

    }
};

#if !defined(ALIMER_STATIC_PLUGIN)
extern "C" ALIMER_INTERFACE_EXPORT Alimer::Plugin* AlimerPluginLoad()
{
    return new TestPlugin();
}

extern "C" ALIMER_INTERFACE_EXPORT void AlimerPluginUnload(Alimer::Plugin* plugin)
{

}
#endif
*/
