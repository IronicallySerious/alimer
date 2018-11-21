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

#include "agpu.h"
#ifdef ALIMER_D3D11
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include <wrl/client.h>
#include <wrl/event.h>
#include "../Core/Platform.h"
#include "../Core/Log.h"

#if defined(_DEBUG)
#   include <dxgidebug.h>
#endif

#if defined(_DEBUG) || defined(PROFILE)
#   if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#       pragma comment(lib,"dxguid.lib")
#   endif
#endif

namespace d3d11
{
    using Microsoft::WRL::ComPtr;

    AgpuBool32 isSupported()
    {
        static AgpuBool32 availableCheck = AGPU_FALSE;
        static AgpuBool32 isAvailable = AGPU_FALSE;

        if (availableCheck)
            return isAvailable;

        availableCheck = AGPU_TRUE;
#if TODO_D3D11
#if AGPU_D3D_DYNAMIC_LIB
        // DXGI
        s_dxgiLib = LoadLibraryW(L"dxgi.dll");
        if (!s_dxgiLib)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        renderer->dynLib.CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(renderer->dynLib.dxgiLib, "CreateDXGIFactory1");
        renderer->dynLib.CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(renderer->dynLib.dxgiLib, "CreateDXGIFactory2");
        renderer->dynLib.DXGIGetDebugInterface = (PFN_GET_DXGI_DEBUG_INTERFACE)GetProcAddress(renderer->dynLib.dxgiLib, "DXGIGetDebugInterface");
        if (!renderer->dynLib.CreateDXGIFactory1)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        renderer->dynLib.d3d11Lib = LoadLibraryW(L"d3d11.dll");
        if (!renderer->dynLib.d3d11Lib)
        {
            OutputDebugStringW(L"Failed to load d3d11.dll");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        renderer->dynLib.D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(renderer->dynLib.d3d11Lib, "D3D11CreateDevice");
        if (!renderer->dynLib.D3D11CreateDevice)
        {
            OutputDebugStringW(L"Cannot find D3D11CreateDevice entry point.");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }
#else
        renderer->dynLib.CreateDXGIFactory1 = CreateDXGIFactory1;
        renderer->dynLib.CreateDXGIFactory2 = CreateDXGIFactory2;
        renderer->dynLib.DXGIGetDebugInterface = DXGIGetDebugInterface;

        // D3D11
        renderer->dynLib.D3D11CreateDevice = D3D11CreateDevice;
#endif  
#endif // TODO_D3D11


        isAvailable = AGPU_TRUE;
        return isAvailable;
    }

    AGpuRendererI* createBackend(bool validation)
    {
        return nullptr;
    }
}

AgpuBool32 agpuIsD3D11Supported()
{
    return d3d11::isSupported();
}

AGpuRendererI* agpuCreateD3D11Backend(bool validation)
{
    return d3d11::createBackend(validation);
}

#endif /* ALIMER_D3D11 */
