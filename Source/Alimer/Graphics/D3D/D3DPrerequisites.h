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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define STRICT                          // Use strict declarations for Windows types

// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <process.h>
#include <wrl.h>

#include <dxgi.h>
#include <d3dcompiler.h>

#define D3D11_NO_HELPERS
#include <d3d11_1.h>


#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

#include <stdio.h>
#include <pix.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// Un-define min and max from the windows headers
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <wincodec.h>
#pragma warning(pop)

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define ALIMER_D3D_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define ALIMER_D3D_DYNAMIC_LIB 1
#endif

#include "../../Core/Platform.h"
#include "../../Debug/Log.h"

namespace Alimer
{
#if !defined(NDEBUG)
#define ThrowIfFailed(hr) \
    do \
    { \
        ALIMER_ASSERT_MSG(SUCCEEDED(hr), Alimer::GetDXErrorString(hr).CString()); \
    } \
    while(0)
#else
    // Throws a DXException on failing HRESULT
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICALF("DirectX Error: %s", Alimer::GetDXErrorString(hr).CString());
        }
    }
#endif

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

}
