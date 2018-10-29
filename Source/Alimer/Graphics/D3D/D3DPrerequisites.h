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

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0A00
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

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
#include <wrl/event.h>

#include <d3d12.h>

#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#if defined(NTDDI_WIN10_RS2)
#   include <dxgi1_6.h>
#else
#   include <dxgi1_5.h>
#endif

#pragma warning(push)
#pragma warning(disable : 4324)
#include "d3dx12.h"
#pragma warning(pop)

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

#include <stdio.h>
#include <pix.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "../Types.h"
#include "../../Core/Log.h"

namespace Alimer
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICALF("Failure with HRESULT of %08X", static_cast<unsigned int>(hr));
        }
    }

    template <typename T>
    void SafeRelease(T& resource, const char* typeName = nullptr)
    {
        if (resource)
        {
#if defined(_DEBUG)
            resource->AddRef();
            ULONG refCount = resource->Release();
            if (refCount > 1)
            {
                if (typeName)
                {
                    OutputDebugStringA(typeName);
                    OutputDebugStringA(" Leakage found\n");
                }
                else
                {
                    OutputDebugStringA("IUnknown leakage found\n");
                }
            }
#endif

            resource->Release();
            resource = nullptr;
        }
    }

    struct ResourceViewInfo
    {
        ResourceViewInfo() = default;
        ResourceViewInfo(uint32_t mostDetailedMip_, uint32_t mipCount_, uint32_t firstArraySlice_, uint32_t arraySize_) : mostDetailedMip(mostDetailedMip_), mipCount(mipCount_), firstArraySlice(firstArraySlice_), arraySize(arraySize_) {}
        uint32_t mostDetailedMip = 0;
        uint32_t mipCount = RemainingMipLevels;
        uint32_t firstArraySlice = 0;
        uint32_t arraySize = RemainingArrayLayers;

        bool operator==(const ResourceViewInfo& other) const
        {
            return (firstArraySlice == other.firstArraySlice) && (arraySize == other.arraySize) && (mipCount == other.mipCount) && (mostDetailedMip == other.mostDetailedMip);
        }
    };

}
