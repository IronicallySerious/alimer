/*
** Alimer - Copyright (C) 2017-2019 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#include "ShaderImporter.h"
using namespace alimer;

const String sPluginName = "ShaderImporterPlugin";

class ShaderImporterPlugin final : public alimer::Plugin
{
public:
    const String& GetName() const { return sPluginName; }

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
extern "C" ALIMER_INTERFACE_EXPORT alimer::Plugin* AlimerPluginLoad()
{
    return new ShaderImporterPlugin();
}

extern "C" ALIMER_INTERFACE_EXPORT void AlimerPluginUnload(alimer::Plugin* plugin)
{
    delete static_cast<ShaderImporterPlugin*>(plugin);
}
#endif

