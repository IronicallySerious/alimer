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

#ifndef AGPU_IMPLEMENTATION
#error "Cannot include AGpuInternal.h without implementation"
#endif

#include "agpu.h"

#ifdef AGPU_D3D11
#   include <d3d11_1.h>
#   include <dxgi1_2.h>
#endif

#ifdef AGPU_D3D12
#   include <d3d12.h>
#   if defined(NTDDI_WIN10_RS2)
#       include <dxgi1_6.h>
#   else
#       include <dxgi1_5.h>
#   endif
#endif

#if (defined(AGPU_D3D11) || defined(AGPU_D3D12)) && defined(_DEBUG)
#   include <dxgidebug.h>
#endif

typedef struct AgpuFence_T {
#if AGPU_D3D12
    ID3D12Fence*        d3d12Fence = nullptr;
    HANDLE              d3d12FenceEvent = INVALID_HANDLE_VALUE;
#endif
} AgpuFence_T;

typedef struct AgpuBuffer_T {
#if AGPU_D3D11
    ID3D11Buffer*       d3d11Buffer = nullptr;
#endif

#if AGPU_D3D12
    ID3D12Resource*     d3d12Buffer = nullptr;
#endif

} AgpuBuffer_T;

struct AGpuRenderer;

#if AGPU_D3D11 || AGPU_D3D12
#include <wrl/client.h>
#include <wrl/event.h>

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define AGPU_D3D_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define AGPU_D3D_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define AGPU_D3D_DYNAMIC_LIB 1
#endif

typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY1)(REFIID riid, _COM_Outptr_ void **ppFactory);
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, void** _factory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE)(REFIID riid, _COM_Outptr_ void** pDebug);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE1)(UINT Flags, REFIID riid, _COM_Outptr_ void** pDebug);

typedef struct AGpuD3DDynamicLib
{
#ifdef AGPU_D3D_DYNAMIC_LIB
    HMODULE dxgiLib;
    HMODULE d3d11Lib;
    HMODULE d3d12Lib;
#endif

    PFN_CREATE_DXGI_FACTORY1            CreateDXGIFactory1;
    PFN_CREATE_DXGI_FACTORY2            CreateDXGIFactory2;
    PFN_GET_DXGI_DEBUG_INTERFACE        DXGIGetDebugInterface;
    PFN_GET_DXGI_DEBUG_INTERFACE1       DXGIGetDebugInterface1;

#if AGPU_D3D11
    PFN_D3D11_CREATE_DEVICE             D3D11CreateDevice;
#endif

#if AGPU_D3D12
    PFN_D3D12_GET_DEBUG_INTERFACE       D3D12GetDebugInterface;
    PFN_D3D12_CREATE_DEVICE             D3D12CreateDevice;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE  D3D12SerializeRootSignature;
#endif

} AGpuD3DDynamicLib;

#endif /* AGPU_D3D11 || AGPU_D3D12 */

#if AGPU_D3D11

typedef struct AGpuD3D11Context
{
#if AGPU_D3D_DYNAMIC_LIB
    AGpuD3DDynamicLib dynLib;
#endif

} AGpuD3D11Context;

AgpuBool32 agpuIsD3D11Supported(AGpuRenderer* renderer);
#endif /* AGPU_D3D11 */

#ifdef AGPU_D3D12
AgpuBool32 agpuIsD3D12Supported(AGpuRenderer* renderer);
AgpuResult agpuSetupD3D12Backend(AGpuRenderer* renderer, const AgpuDescriptor* descriptor);
#endif /* AGPU_D3D12 */

typedef struct AGpuRenderer {
    AgpuBool32              valid;
    AgpuBackend             backend;

    // backend callbacks
    void(*shutdown)(AGpuRenderer*);
    AgpuResult(*beginFrame)(AGpuRenderer*);
    uint64_t(*endFrame)(AGpuRenderer*);

#if AGPU_D3D_DYNAMIC_LIB
    AGpuD3DDynamicLib dynLib;
#endif

#if AGPU_D3D12
    static const size_t MAX_BACK_BUFFER_COUNT = 3;

    struct {
        DWORD                                               factoryFlags;
        Microsoft::WRL::ComPtr<IDXGIFactory4>               factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1>               adapter;
        Microsoft::WRL::ComPtr<ID3D12Device>                device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>          commandQueue;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   commandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      commandAllocators[MAX_BACK_BUFFER_COUNT];
        D3D_FEATURE_LEVEL                                   featureLevel;

        AgpuFence_T*                                        frameFence;

        UINT                                                backBufferIndex;
    } d3d12;
#endif
} AGpuRenderer;
