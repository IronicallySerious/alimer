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

#ifndef NOMINMAX
// Use the C++ standard templated min/max
#   define NOMINMAX
#endif

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>

#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#if defined(NTDDI_WIN10_RS2)
#   include <dxgi1_6.h>
#else
#   include <dxgi1_5.h>
#endif

#ifdef _DEBUG
#   include <dxgidebug.h>
#endif

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define ALIMER_D3D_DYNAMIC_LIB 1
#endif

#include <algorithm>
#include <stdexcept>

namespace alimer
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}

namespace alimer
{
    using Microsoft::WRL::ComPtr;

    inline const char* D3DFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel)
    {
        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_12_1:    return "12.1";
        case D3D_FEATURE_LEVEL_12_0:    return "12.0";
        case D3D_FEATURE_LEVEL_11_1:    return "11.1";
        case D3D_FEATURE_LEVEL_11_0:    return "11.0";
        case D3D_FEATURE_LEVEL_10_1:    return "10.1";
        case D3D_FEATURE_LEVEL_10_0:    return "10.0";
        case D3D_FEATURE_LEVEL_9_3:     return "9.3";
        case D3D_FEATURE_LEVEL_9_2:     return "9.2";
        case D3D_FEATURE_LEVEL_9_1:     return "9.1";
        }

        return "";
    }

    inline static const char* D3DGetVendorByID(unsigned id)
    {
        switch (id)
        {
        case 0x1002: return "Advanced Micro Devices, Inc.";
        case 0x10de: return "NVIDIA Corporation";
        case 0x102b: return "Matrox Electronic Systems Ltd.";
        case 0x1414: return "Microsoft Corporation";
        case 0x5333: return "S3 Graphics Co., Ltd.";
        case 0x8086: return "Intel Corporation";
        case 0x80ee: return "Oracle Corporation";
        case 0x15ad: return "VMware Inc.";
        }
        return "";
    }
}
