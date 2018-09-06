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

#include "D3DPrerequisites.h"

namespace Alimer
{
    class D3DPlatformFunctions final
    {
    public:
        D3DPlatformFunctions();
        ~D3DPlatformFunctions();

        bool LoadFunctions(bool loadD3D12);

        // DXGI
        using PFN_DXGI_GET_DEBUG_INTERFACE1 = HRESULT(WINAPI*)(UINT Flags, REFIID riid, _COM_Outptr_ void** pDebug);
        PFN_DXGI_GET_DEBUG_INTERFACE1 dxgiGetDebugInterface1 = nullptr;

        using PFN_CREATE_DXGI_FACTORY1 = HRESULT(WINAPI*)(REFIID riid, _COM_Outptr_ void** ppFactory);
        PFN_CREATE_DXGI_FACTORY1 createDxgiFactory1 = nullptr;

        using PFN_CREATE_DXGI_FACTORY2 = HRESULT(WINAPI*)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);
        PFN_CREATE_DXGI_FACTORY2 createDxgiFactory2 = nullptr;

        // D3D11
        PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = nullptr;

        // Functions from d3d3compiler.dll
        pD3DCompile d3dCompile = nullptr;

    private:
        HMODULE _dxgiLib = nullptr;
        HMODULE _d3dCompilerLib = nullptr;
        HMODULE _d3d11Lib = nullptr;
        HMODULE _d3d12Lib = nullptr;
    };
}
