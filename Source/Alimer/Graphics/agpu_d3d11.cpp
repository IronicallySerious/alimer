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
#if AGPU_D3D11
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
static AGpuD3D11Context s_d3d11data;

AgpuBool32 agpuIsD3D11Supported(AGpuRenderer* renderer)
{
    static AgpuBool32 availableCheck = AGPU_FALSE;
    static AgpuBool32 isAvailable = AGPU_FALSE;

    if (availableCheck)
        return isAvailable;

    memset(&s_d3d11data, 0, sizeof(s_d3d11data));
    availableCheck = AGPU_TRUE;
#if AGPU_D3D_DYNAMIC_LIB
    // DXGI
    s_d3d11data.dynLib.dxgiLib = LoadLibraryW(L"dxgi.dll");
    if (!s_d3d11data.dynLib.dxgiLib)
    {
        OutputDebugStringW(L"Failed to load dxgi.dll");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    s_d3d11data.dynLib.CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(s_d3d11data.dynLib.dxgiLib, "CreateDXGIFactory1");
    s_d3d11data.dynLib.CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(s_d3d11data.dynLib.dxgiLib, "CreateDXGIFactory2");
    s_d3d11data.dynLib.DXGIGetDebugInterface = (PFN_GET_DXGI_DEBUG_INTERFACE)GetProcAddress(s_d3d11data.dynLib.dxgiLib, "DXGIGetDebugInterface");
    s_d3d11data.dynLib.DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(s_d3d11data.dynLib.dxgiLib, "DXGIGetDebugInterface1");
    if (!s_d3d11data.dynLib.CreateDXGIFactory1)
    {
        OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    s_d3d11data.dynLib.d3d11Lib = LoadLibraryW(L"d3d11.dll");
    if (!s_d3d11data.dynLib.d3d11Lib)
    {
        OutputDebugStringW(L"Failed to load d3d11.dll");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    s_d3d11data.dynLib.D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(s_d3d11data.dynLib.d3d11Lib, "D3D11CreateDevice");
    if (!s_d3d11data.dynLib.D3D11CreateDevice)
    {
        OutputDebugStringW(L"Cannot find D3D11CreateDevice entry point.");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }
#else
    s_d3d11data.dynLib.CreateDXGIFactory1 = CreateDXGIFactory1;
    s_d3d11data.dynLib.CreateDXGIFactory2 = CreateDXGIFactory2;
    s_d3d11data.dynLib.DXGIGetDebugInterface = DXGIGetDebugInterface;
    s_d3d11data.dynLib.DXGIGetDebugInterface1 = DXGIGetDebugInterface1;

    // D3D11
    s_d3d11data.dynLib.D3D11CreateDevice = D3D11CreateDevice;
#endif

    isAvailable = AGPU_TRUE;
    return isAvailable;
}
#endif /* AGPU_D3D12 */
