/*
** Alimer - Copyright (C) 2016-2018 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

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
