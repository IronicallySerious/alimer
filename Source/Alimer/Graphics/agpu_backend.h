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
    uint32_t                    backBufferIndex;
    uint32_t                    backbufferCount;
    AgpuPixelFormat             backBufferFormat;
    AgpuTexture                 backBufferTexture[AGPU_MAX_BACK_BUFFER_COUNT];
    AgpuFramebuffer             backBufferFramebuffers[AGPU_MAX_BACK_BUFFER_COUNT];

#if AGPU_D3D11 || AGPU_D3D12
    DXGI_FORMAT                 dxgiBackBufferFormat;
#endif

#if AGPU_D3D12
    IDXGISwapChain3*            d3d12SwapChain;
    ID3D12Resource*             d3d12DepthStencil;
#endif
} AgpuSwapchain_T;

typedef struct AgpuTexture_T {
#if AGPU_D3D11 || AGPU_D3D12
    DXGI_FORMAT                     dxgiFormat;
#endif

#if AGPU_D3D11
    union {
        ID3D11Texture1D*            d3d11Texture1D;
        ID3D11Texture2D*            d3d11Texture2D;
        ID3D11Texture3D*            d3d11Texture3D;
    };
#endif

#if AGPU_D3D12
    D3D12_RESOURCE_STATES           d3d12ResourceState;
    ID3D12Resource*                 d3d12Resource;
#endif

} AgpuTexture_T;

typedef struct AgpuFramebuffer_T {
    AgpuFramebufferAttachment       colorAttachments[AGPU_MAX_COLOR_ATTACHMENTS];
    AgpuFramebufferAttachment       depthStencilAttachment;

#if AGPU_D3D12
    uint32_t                        numRTVs;
    D3D12_CPU_DESCRIPTOR_HANDLE     d3d12RTVs[AGPU_MAX_COLOR_ATTACHMENTS];
    D3D12_CPU_DESCRIPTOR_HANDLE     d3d12DSV;
#endif
} AgpuFramebuffer_T;

typedef struct AgpuBuffer_T {
#if AGPU_D3D11
    ID3D11Buffer*                   d3d11Buffer;
#endif

#if AGPU_D3D12
    ID3D12Resource*                 d3d12Buffer;
#endif

} AgpuBuffer_T;

typedef struct AgpuShader_T {
#if AGPU_D3D12
    D3D12_SHADER_BYTECODE           d3d12Bytecode;
#endif
} AgpuShader_T;

typedef struct AgpuPipeline_T {
    AgpuBool32                      isCompute;

#if AGPU_D3D12
    ID3D12PipelineState*            d3d12PipelineState;
#endif

} AgpuPipeline_T;

typedef struct AgpuCommandBuffer_T {
#if AGPU_D3D12
    ID3D12GraphicsCommandList*      d3d12CommandList;
#endif
} AgpuCommandBuffer_T;

struct AGpuRendererI
{
    virtual ~AGpuRendererI() = 0;

    virtual void Shutdown() = 0;
    virtual uint64_t Frame() = 0;

    virtual AgpuTexture CreateTexture(const AgpuTextureDescriptor* descriptor, void* externalHandle) = 0;
    virtual void DestroyTexture(AgpuTexture texture) = 0;

    virtual AgpuFramebuffer CreateFramebuffer(const AgpuFramebufferDescriptor* descriptor) = 0;
    virtual void DestroyFramebuffer(AgpuFramebuffer framebuffer) = 0;

    virtual AgpuShader CreateShader(const AgpuShaderDescriptor* descriptor) = 0;
    virtual void DestroyShader(AgpuShader shader) = 0;

    virtual AgpuPipeline CreateRenderPipeline(const AgpuRenderPipelineDescriptor* descriptor) = 0;
    virtual AgpuPipeline CreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor) = 0;
    virtual void DestroyPipeline(AgpuPipeline pipeline) = 0;
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
