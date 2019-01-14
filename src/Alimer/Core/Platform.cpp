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

#include "../Core/Platform.h"
#include "../Base/String.h"
#include "../Core/Log.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

// ntdll.dll function pointer typedefs
typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI* PFN_RtlGetVersion)(PRTL_OSVERSIONINFOEXW);
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

    OSVERSIONINFOEXW osvi = { sizeof(osvi), wMajorVersion, wMinorVersion, 0, 0, {0}, wServicePackMajor };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return RtlVerifyVersionInfo_(&osvi, mask, cond) == 0;
}


#elif __EMSCRIPTEN__
#   include <emscripten/emscripten.h>
#	include <dlfcn.h>
#elif defined(__APPLE__) 
#   include <TargetConditionals.h>
#   include <sys/time.h>
#   include <unistd.h>
#   include <dlfcn.h>
#   include <pthread.h>
#else
#   include <sys/time.h>
#   include <unistd.h>
#   include <dlfcn.h>
#   include <pthread.h>
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

    const char* GetPlatformName()
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

    const char* GetOSDescription()
    {
        static String version;
        if (!version.IsEmpty())
            return version.CString();

#if ALIMER_PLATFORM_WINDOWS
        RTL_OSVERSIONINFOEXW osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        version = "Microsoft Windows";

        static PFN_RtlGetVersion RtlGetVersion_ = nullptr;

        if (RtlGetVersion_ == nullptr)
        {
            RtlGetVersion_ = reinterpret_cast<PFN_RtlGetVersion>(GetProcAddress(s_ntdllHandle, "RtlGetVersion"));
        }

        if (RtlGetVersion_(&osvi) == 0)
        {
            if (osvi.szCSDVersion[0] != '\0')
            {
                version.AppendWithFormat(" %d.%d.%d %s",
                    osvi.dwMajorVersion,
                    osvi.dwMinorVersion,
                    osvi.dwBuildNumber,
                    alimer::String(osvi.szCSDVersion).CString()
                );
            }
            else
            {
                version.AppendWithFormat(" %d.%d.%d",
                    osvi.dwMajorVersion,
                    osvi.dwMinorVersion,
                    osvi.dwBuildNumber
                );
            }
            }

#elif ALIMER_PLATFORM_UWP
        return "Microsoft Windows";
#endif

        return version.CString();
        }

    void* LoadNativeLibrary(const char* name)
    {
#if ALIMER_PLATFORM_WINDOWS
        std::string nameStr(name);
        std::wstring wideName = std::wstring(nameStr.begin(), nameStr.end());
        HMODULE handle = LoadLibraryW(wideName.c_str());
        return handle;
#elif ALIMER_PLATFORM_UWP
        std::string nameStr(name);
        std::wstring wideName = std::wstring(nameStr.begin(), nameStr.end());
        HMODULE handle = LoadPackagedLibrary(wideName.c_str(), 0);
        return handle;
#elif ALIMER_PLATFORM_WEB
        ALIMER_UNUSED(name);
        return nullptr;
#else
        return ::dlopen(name, RTLD_LOCAL | RTLD_LAZY);
#endif
    }

    void UnloadNativeLibrary(void* handle)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        ::FreeLibrary((HMODULE)handle);
#elif ALIMER_PLATFORM_WEB
        ALIMER_UNUSED(handle);
#else
        ::dlclose(handle);
#endif
    }

    void* GetLibrarySymbol(void* handle, const char* name)
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
        return (void*)::GetProcAddress((HMODULE)handle, name);
#elif ALIMER_PLATFORM_WEB
        ALIMER_UNUSED(handle);
        ALIMER_UNUSED(name);
        return nullptr;
#else
        return ::dlsym(handle, name);
#endif
}

    void SetCurrentThreadName(const char* name)
    {
#if defined(_MSC_VER)
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
#else
#  ifdef __APPLE__
        if (pthread_setname_np(name) != 0)
        {
            ALIMER_LOGERROR("Failed to set thread name");
        }
#  elif defined(__linux__) || defined(__ANDROID__)
        if (pthread_setname_np(pthread_self(), name) != 0)
        {
            ALIMER_LOGERROR("Failed to set thread name");
        }
#  endif
#endif
        }

    void Sleep(uint32_t milliseconds)
    {
#if defined(_MSC_VER)
        ::Sleep(static_cast<DWORD>(milliseconds));
#else
        timespec time{ static_cast<time_t>(milliseconds / 1000), static_cast<long>((milliseconds % 1000) * 1000000) };
        nanosleep(&time, nullptr);
#endif
    }

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
    std::wstring GetWin32ErrorString(unsigned long errorCode)
    {
        wchar_t errorString[MAX_PATH];
        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
            0,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorString,
            MAX_PATH,
            NULL);

        std::wstring message = L"Win32 Error: ";
        message += errorString;
        return message;
    }

    void DXGetErrorDescriptionW(_In_ HRESULT hr, _Out_cap_(count) wchar_t* desc, _In_ size_t count)
    {
        if (!count)
            return;

        *desc = 0;

        // First try to see if FormatMessage knows this hr
        LPWSTR errorText = nullptr;

        DWORD result = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorText, 0, nullptr);

        if (result > 0 && errorText)
        {
            wcscpy_s(desc, count, errorText);

            if (errorText)
                LocalFree(errorText);

            return;
        }

        if (errorText)
            LocalFree(errorText);

#define  CHK_ERRA(hrchk) \
        case hrchk: \
             wcscpy_s( desc, count, L#hrchk );

#define  CHK_ERR(hrchk, strOut) \
        case hrchk: \
             wcscpy_s( desc, count, L##strOut );

        switch (hr)
        {
            // Commmented out codes are actually alises for other codes

            // -------------------------------------------------------------
            // dxgi.h error codes
            // -------------------------------------------------------------
            CHK_ERR(DXGI_STATUS_OCCLUDED, "The target window or output has been occluded. The application should suspend rendering operations if possible.");
            CHK_ERR(DXGI_STATUS_CLIPPED, "Target window is clipped.");
            CHK_ERR(DXGI_STATUS_NO_REDIRECTION, "");
            CHK_ERR(DXGI_STATUS_NO_DESKTOP_ACCESS, "No access to desktop.");
            CHK_ERR(DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE, "");
            CHK_ERR(DXGI_STATUS_MODE_CHANGED, "Display mode has changed");
            CHK_ERR(DXGI_STATUS_MODE_CHANGE_IN_PROGRESS, "Display mode is changing");
            CHK_ERR(DXGI_ERROR_INVALID_CALL, "The application has made an erroneous API call that it had enough information to avoid. This error is intended to denote that the application should be altered to avoid the error. Use of the debug version of the DXGI.DLL will provide run-time debug output with further information.");
            CHK_ERR(DXGI_ERROR_NOT_FOUND, "The item requested was not found. For GetPrivateData calls, this means that the specified GUID had not been previously associated with the object.");
            CHK_ERR(DXGI_ERROR_MORE_DATA, "The specified size of the destination buffer is too small to hold the requested data.");
            CHK_ERR(DXGI_ERROR_UNSUPPORTED, "Unsupported.");
            CHK_ERR(DXGI_ERROR_DEVICE_REMOVED, "Hardware device removed.");
            CHK_ERR(DXGI_ERROR_DEVICE_HUNG, "Device hung due to badly formed commands.");
            CHK_ERR(DXGI_ERROR_DEVICE_RESET, "Device reset due to a badly formed commant.");
            CHK_ERR(DXGI_ERROR_WAS_STILL_DRAWING, "Was still drawing.");
            CHK_ERR(DXGI_ERROR_FRAME_STATISTICS_DISJOINT, "The requested functionality is not supported by the device or the driver.");
            CHK_ERR(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, "The requested functionality is not supported by the device or the driver.");
            CHK_ERR(DXGI_ERROR_DRIVER_INTERNAL_ERROR, "An internal driver error occurred.");
            CHK_ERR(DXGI_ERROR_NONEXCLUSIVE, "The application attempted to perform an operation on an DXGI output that is only legal after the output has been claimed for exclusive owenership.");
            CHK_ERR(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, "The requested functionality is not supported by the device or the driver.");
            CHK_ERR(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, "Remote desktop client disconnected.");
            CHK_ERR(DXGI_ERROR_REMOTE_OUTOFMEMORY, "Remote desktop client is out of memory.");

            // -------------------------------------------------------------
            // d3d11.h error codes
            // -------------------------------------------------------------
            CHK_ERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS, "There are too many unique state objects.");
            CHK_ERR(D3D11_ERROR_FILE_NOT_FOUND, "File not found");
            CHK_ERR(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS, "Therea are too many unique view objects.");
            CHK_ERR(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD, "Deferred context requires Map-Discard usage pattern");

            // -------------------------------------------------------------
            // WIC error codes
            // -------------------------------------------------------------
            CHK_ERR(WINCODEC_ERR_WRONGSTATE, "WIC object in incorrect state.");
            CHK_ERR(WINCODEC_ERR_VALUEOUTOFRANGE, "WIC Value out of range.");
            CHK_ERR(WINCODEC_ERR_UNKNOWNIMAGEFORMAT, "Encountered unexpected value or setting in WIC image format.");
            CHK_ERR(WINCODEC_ERR_UNSUPPORTEDVERSION, "Unsupported WINCODEC_SD_VERSION passed to WIC factory.");
            CHK_ERR(WINCODEC_ERR_NOTINITIALIZED, "WIC component not initialized.");
            CHK_ERR(WINCODEC_ERR_ALREADYLOCKED, "WIC bitmap object already locked.");
            CHK_ERR(WINCODEC_ERR_PROPERTYNOTFOUND, "WIC property not found.");
            CHK_ERR(WINCODEC_ERR_PROPERTYNOTSUPPORTED, "WIC property not supported.");
            CHK_ERR(WINCODEC_ERR_PROPERTYSIZE, "Invalid property size");
            CHK_ERRA(WINCODEC_ERR_CODECPRESENT); // not currently used by WIC
            CHK_ERRA(WINCODEC_ERR_CODECNOTHUMBNAIL); // not currently used by WIC
            CHK_ERR(WINCODEC_ERR_PALETTEUNAVAILABLE, "Required palette data not available.");
            CHK_ERR(WINCODEC_ERR_CODECTOOMANYSCANLINES, "More scanlines requested than are available in WIC bitmap.");
            CHK_ERR(WINCODEC_ERR_INTERNALERROR, "Unexpected internal error in WIC.");
            CHK_ERR(WINCODEC_ERR_SOURCERECTDOESNOTMATCHDIMENSIONS, "Source WIC rectangle does not match bitmap dimensions.");
            CHK_ERR(WINCODEC_ERR_COMPONENTNOTFOUND, "WIC component not found.");
            CHK_ERR(WINCODEC_ERR_IMAGESIZEOUTOFRANGE, "Image size beyond expected boundaries for WIC codec.");
            CHK_ERR(WINCODEC_ERR_TOOMUCHMETADATA, "Image metadata size beyond expected boundaries for WIC codec.");
            CHK_ERR(WINCODEC_ERR_BADIMAGE, "WIC image is corrupted.");
            CHK_ERR(WINCODEC_ERR_BADHEADER, "Invalid header found in WIC image.");
            CHK_ERR(WINCODEC_ERR_FRAMEMISSING, "Expected bitmap frame data not found in WIC image.");
            CHK_ERR(WINCODEC_ERR_BADMETADATAHEADER, "Invalid metadata header found in WIC image.");
            CHK_ERR(WINCODEC_ERR_BADSTREAMDATA, "Invalid stream data found in WIC image.");
            CHK_ERR(WINCODEC_ERR_STREAMWRITE, "WIC operation on write stream failed.");
            CHK_ERR(WINCODEC_ERR_STREAMREAD, "WIC operation on read stream failed.");
            CHK_ERR(WINCODEC_ERR_STREAMNOTAVAILABLE, "Required stream is not available.");
            //        CHK_ERRA(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT)
            CHK_ERR(WINCODEC_ERR_UNSUPPORTEDOPERATION, "This operation is not supported by WIC.");
            CHK_ERR(WINCODEC_ERR_INVALIDREGISTRATION, "Error occurred reading WIC codec registry keys.");
            CHK_ERR(WINCODEC_ERR_COMPONENTINITIALIZEFAILURE, "Failed initializing WIC codec.");
            CHK_ERR(WINCODEC_ERR_INSUFFICIENTBUFFER, "Not enough buffer space available for WIC operation.");
            CHK_ERR(WINCODEC_ERR_DUPLICATEMETADATAPRESENT, "Duplicate metadata detected in WIC image.");
            CHK_ERR(WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE, "Unexpected property type in WIC image.");
            CHK_ERR(WINCODEC_ERR_UNEXPECTEDSIZE, "Unexpected value size in WIC metadata.");
            CHK_ERR(WINCODEC_ERR_INVALIDQUERYREQUEST, "Invalid metadata query.");
            CHK_ERR(WINCODEC_ERR_UNEXPECTEDMETADATATYPE, "Unexpected metadata type encountered in WIC image.");
            CHK_ERR(WINCODEC_ERR_REQUESTONLYVALIDATMETADATAROOT, "Operation only valid on meatadata root.");
            CHK_ERR(WINCODEC_ERR_INVALIDQUERYCHARACTER, "Invalid character in WIC metadata query.");
            CHK_ERR(WINCODEC_ERR_WIN32ERROR, "General Win32 error encountered during WIC operation.");
            CHK_ERR(WINCODEC_ERR_INVALIDPROGRESSIVELEVEL, "Invalid level for progressive WIC image decode.");
        }

#undef CHK_ERR
#undef CHK_ERRA
    }

    std::wstring GetDXErrorString(long hr)
    {
        const uint32_t errStringSize = 1024;
        wchar_t errorString[errStringSize];
        DXGetErrorDescriptionW(hr, errorString, errStringSize);

        std::wstring message = L"DirectX Error: ";
        message += errorString;
        return message;
    }

    std::string GetDXErrorStringAnsi(long hr)
    {
        std::wstring errorString = GetDXErrorString(hr);

        std::string message;
        for (size_t i = 0; i < errorString.length(); ++i)
            message.append(1, static_cast<char>(errorString[i]));
        return message;
    }
#endif
}
