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

typedef struct AgpuSwapchain_T {
    uint32_t            backBufferIndex;
    uint32_t            backbufferCount;

#if AGPU_D3D12
    IDXGISwapChain3*            d3d12SwapChain;
    ID3D12Resource*             d3d12RenderTargets[AGPU_MAX_BACK_BUFFER_COUNT];
    ID3D12Resource*             d3d12DepthStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE d3d12RTV[AGPU_MAX_BACK_BUFFER_COUNT] = { };
    D3D12_CPU_DESCRIPTOR_HANDLE d3d12DSV = { };
#endif
} AgpuSwapchain_T;

typedef struct AgpuBuffer_T {
#if AGPU_D3D11
    ID3D11Buffer*       d3d11Buffer;
#endif

#if AGPU_D3D12
    ID3D12Resource*     d3d12Buffer;
#endif

} AgpuBuffer_T;

struct AGpuRendererI
{
    virtual ~AGpuRendererI() = 0;

    virtual void Shutdown() = 0;
    virtual uint64_t Frame() = 0;
};

inline AGpuRendererI::~AGpuRendererI()
{
}

#if AGPU_D3D11
AgpuBool32 agpuIsD3D11Supported();
AgpuResult agpuCreateD3D11Backend(const AgpuDescriptor* descriptor, AGpuRendererI** pRenderer);
#endif /* AGPU_D3D11 */

#ifdef AGPU_D3D12
AgpuBool32 agpuIsD3D12Supported();
AgpuResult agpuCreateD3D12Backend(const AgpuDescriptor* descriptor, AGpuRendererI** pRenderer);
#endif /* AGPU_D3D12 */
