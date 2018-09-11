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

#include "../Core/Platform.h"
#include "../Core/Log.h"
#include "../Base/String.h"

#if !ALIMER_WINDOWS_FAMILY
#   include <dlfcn.h>
#endif

#if ALIMER_PLATFORM_WINDOWS
// ntdll.dll function pointer typedefs
typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI* PFN_RtlGetVersion)(PRTL_OSVERSIONINFOW);
typedef LONG(WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);

static HMODULE s_ntdllHandle = LoadLibraryA("ntdll.dll");

bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    RTL_OSVERSIONINFOEXW verInfo = { 0 };
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);

    static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo_ = nullptr;

    if (RtlVerifyVersionInfo_ == nullptr)
    {
        RtlVerifyVersionInfo_ = reinterpret_cast<PFN_RtlVerifyVersionInfo>(GetProcAddress(s_ntdllHandle, "RtlVerifyVersionInfo"));
    }

    std::string version = "Microsoft Windows";

    OSVERSIONINFOEXW osvi = { sizeof(osvi), wMajorVersion, wMinorVersion, 0, 0, {0}, wServicePackMajor };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return RtlVerifyVersionInfo_(&osvi, mask, cond) == 0;
}

static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

#endif

namespace Alimer
{
    PlatformId GetPlatformId()
    {
#if __ANDROID__
        return PlatformId::Android;
#elif ALIMER_PLATFORM_APPLE_IOS
        return PlatformId::iOS;
#elif ALIMER_PLATFORM_APPLE_TV
        return PlatformId::AppleTV;
#elif ALIMER_PLATFORM_APPLE_OSX
        return PlatformId::MacOS;
#elif ALIMER_PLATFORM_WINDOWS
        return PlatformId::Windows;
#elif ALIMER_PLATFORM_UWP
        return PlatformId::WindowsUniversal;
#elif ALIMER_PLATFORM_WEB
        return PlatformId::Web;
#elif ALIMER_PLATFORM_LINUX
        return PlatformId::Linux;
#else
        return PlatformId::Unknown;
#endif
    }

    PlatformFamily GetPlatformFamily()
    {
#if ALIMER_PLATFORM_ANDROID || ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
        return PlatformFamily::Mobile;
#elif ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_APPLE_OSX
        return PlatformFamily::Desktop;
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

    const char* GetPlatformName()
    {
        PlatformId platform = GetPlatformId();
        switch (platform)
        {
        case PlatformId::Windows:
            return "Windows";
        case PlatformId::WindowsUniversal:
            return "UWP";
        case PlatformId::Linux:
            return "Linux";
        case PlatformId::MacOS:
            return "MacOS";
        case PlatformId::Android:
            return "Android";
        case PlatformId::iOS:
            return "iOS";
        case PlatformId::AppleTV:
            return "AppleTV";
        case PlatformId::Web:
            return "Web";
        default:
            return "Unknown";
        }
    }

    String GetOSDescription()
    {
#if ALIMER_PLATFORM_WINDOWS
        RTL_OSVERSIONINFOW osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        String version = "Microsoft Windows";

        static PFN_RtlGetVersion RtlGetVersion_ = nullptr;

        if (RtlGetVersion_ == nullptr)
        {
            RtlGetVersion_ = reinterpret_cast<PFN_RtlGetVersion>(GetProcAddress(s_ntdllHandle, "RtlGetVersion"));
        }

        if (RtlGetVersion_(&osvi) == 0)
        {
            version = String::Format("%s %d.%d.%d %s",
                version.CString(),
                osvi.dwMajorVersion,
                osvi.dwMinorVersion,
                osvi.dwBuildNumber,
                String(osvi.szCSDVersion).CString()
            );
        }

        return version;
#elif ALIMER_PLATFORM_UWP
        return "Microsoft Windows";
#endif
    }

    void* LoadNativeLibrary(const char* name)
    {
#if ALIMER_PLATFORM_WINDOWS
        auto wideName = WString(name);
        HMODULE handle = LoadLibraryW(wideName.CString());
        return handle;
#elif ALIMER_PLATFORM_UWP
        auto wideName = WString(name);
        HMODULE handle = LoadPackagedLibrary(wideName.CString(), 0);
        return handle;
#else
        return ::dlopen(name, RTLD_LOCAL | RTLD_LAZY);
#endif
    }

    void UnloadNativeLibrary(void* handle)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        ::FreeLibrary((HMODULE)handle);
#else
        ::dlclose(handle);
#endif
    }

    void* GetSymbol(void* handle, const char* name)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        return (void*)::GetProcAddress((HMODULE)handle, name);
#else
        return ::dlsym(handle, name);
#endif
    }

    bool SetCurrentThreadName(const char* name)
    {
#if ALIMER_PLATFORM_WINDOWS
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = static_cast<DWORD>(-1);
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
        return true;

#else
        return false;
#endif
    }
}
