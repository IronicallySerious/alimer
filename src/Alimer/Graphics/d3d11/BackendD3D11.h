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

#ifdef _WIN32
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#endif

#include "../Backend.h"
#include "../D3D/D3DPrerequisites.h"

#define D3D11_NO_HELPERS
#include <d3d11_1.h>

#if defined(NTDDI_WIN10_RS2)
#   include <dxgi1_6.h>
#else
#   include <dxgi1_5.h>
#endif

#pragma warning(push)
#pragma warning(disable : 4467 5038)
#include <wrl.h>
#pragma warning(pop)

#ifdef _DEBUG
#   include <dxgidebug.h>
#endif

#if defined(ALIMER_DEV) && (!defined(_XBOX_ONE) || !defined(_TITLE))
#   pragma comment(lib,"dxguid.lib")
#endif

namespace alimer
{
    class GraphicsDeviceD3D11;

    // Helper sets a D3D resource name string (used by PIX and debug layer leak reporting).
    inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char* name, size_t length)
    {
#if defined(ALIMER_DEV)
#   if defined(_XBOX_ONE) && defined(_TITLE)
        wchar_t wname[MAX_PATH];
        int result = MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(length), wname, MAX_PATH);
        if (result > 0)
        {
            resource->SetName(wname);
        }
#   else
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(length - 1), name);
#   endif
#else
        UNREFERENCED_PARAMETER(resource);
        UNREFERENCED_PARAMETER(name);
#endif
    }
}
