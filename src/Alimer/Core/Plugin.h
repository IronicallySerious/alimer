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


#pragma once

#include "../Base/String.h"

#if defined(__CYGWIN32__)
#   define ALIMER_INTERFACE_EXPORT __declspec(dllexport)
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
#   define ALIMER_INTERFACE_EXPORT __declspec(dllexport)
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
#   define ALIMER_INTERFACE_EXPORT
#else
#   define ALIMER_INTERFACE_EXPORT
#endif

namespace alimer
{
    /// Class defining a generic Engine plugin.
    class ALIMER_API Plugin
    {
    public:
        /// Constructor.
        Plugin() = default;
        virtual ~Plugin() = default;

        /// Get the plugin name.
        virtual const String& GetName() const = 0;

        /// Perform the plugin initial installation sequence. 
        virtual void Install() = 0;

        /// Perform the final plugin uninstallation sequence. 
        virtual void Uninstall() = 0;

        /// Perform any tasks the plugin needs to perform on full system initialization.
        virtual void Initialize() {}

        /// Perform any tasks the plugin needs to perform when the system is shut down.
        virtual void Shutdown() {}

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Plugin);
    };

    // Plugins should implement thoose functions
    // "AlimerPluginLoad" and "AlimerPluginUnload" (extern "C")
    typedef Plugin* (*PluginLoadFunc)();
    typedef void (*PluginUnloadFunc)(Plugin*);

}
