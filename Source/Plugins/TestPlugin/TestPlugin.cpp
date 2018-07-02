/*
** Alimer - Copyright (C) 2016-2018 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#include "AlimerPluginApi.h"
#include <cstring>

void InitializePlugin(GetPluginApiFunc getApi)
{
}

void ShutdownPlugin()
{
}

PluginDesc* GetDescPlugin()
{
    static PluginDesc desc;
    strcpy(desc.name, "TestPlugin");
    strcpy(desc.description, "TestPlugin Desc");
    desc.type = PLUGIN_TYP_GENERIC;
    desc.version = ALIMER_MAKE_VERSION(1, 0, 0);
    return &desc;
}

#if !defined ALIMER_STATIC_PLUGIN
ALIMER_INTERFACE_EXPORT void* AlimerGetPluginApi(uint32_t api, uint32_t version)
{
	static AlimerPluginApi plugin;

    if (version == 0) {
        plugin.Initialize = InitializePlugin;
        plugin.Shutdown = ShutdownPlugin;
        plugin.GetDesc = GetDescPlugin;
        return &plugin;
    }
    else {
        return nullptr;
    }

	return &plugin;
}
#endif
