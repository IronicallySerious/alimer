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

#include "core/Platform.h"
#include "foundation/Utils.h"
#if defined(_WIN64) || defined(_WIN32)
#   include <Windows.h>
#endif

namespace alimer
{
    PlatformType GetPlatformType()
    {
#if defined(_DURANGO) || defined(_XBOX_ONE)
        return PlatformType::XboxOne;
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
        return PlatformType::UWP;
#elif defined(_WIN64) || defined(_WIN32) 
        return PlatformType::Windows;
#elif defined(__APPLE__)
#   if TARGET_OS_IOS
        return PlatformType::iOS;
#   elif TARGET_OS_TV
        return PlatformType::AppleTV;
#   elif TARGET_OS_MAC
        return PlatformType::MacOS;
#   endif
#elif defined(__ANDROID__)
        return PlatformType::Android;
#elif defined(__linux__)
        return PlatformType::Linux;
#elif defined(__EMSCRIPTEN__)
        return PlatformType::Web;
#else
        return PlatformType::Unknown;
#endif
    }

    PlatformFamily GetPlatformFamily()
    {
#if ALIMER_PLATFORM_ANDROID || ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
        return PLATFORM_FAMILY_MOBILE;
#elif ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_MACOS
        return PlatformFamily::Desktop;
#elif ALIMER_PLATFORM_XBOXONE
        return PLATFORM_FAMILY_CONSOLE;
#elif ALIMER_PLATFORM_UWP
        Windows::System::Profile::AnalyticsVersionInfo^ versionInfo = Windows::System::Profile::AnalyticsInfo::VersionInfo;
        Platform::String^ DeviceFamily = versionInfo->DeviceFamily;
        if (DeviceFamily->Equals("Windows.Desktop"))
        {
            return PlatformFamily::Desktop;
        }
        else if (DeviceFamily->Equals("Windows.Mobile"))
        {
            return PlatformFamily::Mobile;
        }
        else if (DeviceFamily->Equals("Windows.Xbox"))
        {
            return PlatformFamily::Console;
        }

        return PlatformFamily::Unknown;
#elif ALIMER_PLATFORM_WEB
        return PlatformFamily::Console;
#else
        return PlatformFamily::Unknown;
#endif
    }

    std::string GetPlatformName()
    {
        PlatformType platform = GetPlatformType();
        switch (platform)
        {
        case PlatformType::Windows:
            return "Windows";
        case PlatformType::UWP:
            return "UWP";
        case PlatformType::XboxOne:
            return "XboxOne";
        case PlatformType::Linux:
            return "Linux";
        case PlatformType::MacOS:
            return "macOS";
        case PlatformType::Android:
            return "Android";
        case PlatformType::iOS:
            return "iOS";
        case PlatformType::AppleTV:
            return "AppleTV";
        case PlatformType::Web:
            return "Web";
        default:
            return "Unknown";
        }
    }

    std::string GetOSDescription()
    {
        static std::string version;
        if (!version.empty()) {
            return version.c_str();
        }

#if ALIMER_PLATFORM_WINDOWS
        HMODULE handle = GetModuleHandleW(L"ntdll.dll");
        if (handle)
        {
            version = "Microsoft Windows";

            typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);
            RtlGetVersionPtr getVersionPtr = (RtlGetVersionPtr)GetProcAddress(handle, "RtlGetVersion");

            RTL_OSVERSIONINFOEXW osvi = { 0 };
            osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
            if (getVersionPtr != nullptr
                && getVersionPtr(&osvi) == 0)
            {
                if (osvi.szCSDVersion[0] != '\0')
                {
                    /*version += fmt::sprintf(" %d.%d.%d %s",
                        osvi.dwMajorVersion,
                        osvi.dwMinorVersion,
                        osvi.dwBuildNumber,
                        ToUtf8(osvi.szCSDVersion)
                    );*/
                }
                else
                {
                    /*version += fmt::sprintf(" %d.%d.%d",
                        osvi.dwMajorVersion,
                        osvi.dwMinorVersion,
                        osvi.dwBuildNumber
                    );*/
                }
            }
        }

#elif ALIMER_PLATFORM_UWP
        return "Microsoft Windows";
#endif

        return version.c_str();
    }
}
