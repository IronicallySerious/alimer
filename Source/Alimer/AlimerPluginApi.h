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

// Alimer native plugin API
// Compatible with C99

#if defined(__CYGWIN32__)
	#define ALIMER_INTERFACE_EXPORT __declspec(dllexport)
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
	#define ALIMER_INTERFACE_EXPORT __declspec(dllexport)
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
	#define ALIMER_INTERFACE_EXPORT
#else
	#define ALIMER_INTERFACE_EXPORT
#endif


#include <stddef.h>
#include <stdint.h>

#define ALIMER_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))

#define ALIMER_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define ALIMER_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define ALIMER_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum PluginApiID
	{
		CORE_API_ID = 0,
		PLUGIN_API_ID = 1,
		GPU_API_ID = 2
	} PluginApiID;

    typedef enum PluginType
    {
        PLUGIN_TYP_GENERIC = 0,
    } PluginType;

	// Plugins should also implement this function
	// Should be named "AlimerGetPluginApi" (extern "C")
	typedef void* (*GetPluginApiFunc)(uint32_t api, uint32_t version);

    typedef struct PluginDesc
    {
        char name[32];
        char description[64];
        uint32_t version;
        PluginType type;
    } PluginDesc;

	typedef struct AlimerPluginApi
	{
		/* Called when plugin needs to is initialized. */
		void(*Initialize)(GetPluginApiFunc getApi);

		/* Called when plugin needs to be unloaded. */
		void(*Shutdown)();

        /* Returns the description of the plugin. */
        PluginDesc* (*GetDesc)();
	} AlimerPluginApi;

	// If exported by a plugin, this function will be called when the plugin is loaded.
	ALIMER_INTERFACE_EXPORT void* AlimerGetPluginApi(uint32_t api, uint32_t version);

#ifdef __cplusplus
}
#endif
