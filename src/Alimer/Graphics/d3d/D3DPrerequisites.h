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

#include <foundation/windows.h>

#include <dxgi.h>
#include <d3dcompiler.h>

#define D3D11_NO_HELPERS
#include <d3d11_1.h>

#pragma warning(push)
#pragma warning(disable : 4467 5038)
#   include <wrl.h>
#pragma warning(pop)

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define ALIMER_D3D_DYNAMIC_LIB 1
#endif

#include "../../Core/Platform.h"
#include "../Types.h"
#include "../../Core/Log.h"

namespace alimer
{
#if !defined(NDEBUG)
#define ThrowIfFailed(hr) \
    do \
    { \
        ALIMER_ASSERT_MSG(SUCCEEDED(hr), alimer::GetDXErrorStringAnsi(hr).c_str()); \
    } \
    while(0)
#else
    // Throws a DXException on failing HRESULT
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICAL("DirectX Error: {}", alimer::GetDXErrorStringAnsi(hr).c_str());
        }
    }
#endif

    struct D3DResourceViewInfo
    {
        D3DResourceViewInfo() = default;
        D3DResourceViewInfo(uint32_t mostDetailedMip_, uint32_t mipCount_, uint32_t firstArraySlice_, uint32_t arraySize_) : mostDetailedMip(mostDetailedMip_), mipCount(mipCount_), firstArraySlice(firstArraySlice_), arraySize(arraySize_) {}
        uint32_t mostDetailedMip = 0;
        uint32_t mipCount = RemainingMipLevels;
        uint32_t firstArraySlice = 0;
        uint32_t arraySize = RemainingArrayLayers;

        bool operator==(const D3DResourceViewInfo& other) const
        {
            return (firstArraySlice == other.firstArraySlice) && (arraySize == other.arraySize) && (mipCount == other.mipCount) && (mostDetailedMip == other.mostDetailedMip);
        }
    };
}
