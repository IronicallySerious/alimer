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

#if defined(_WIN32) || defined(_WIN64)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#endif

#include <wrl/client.h>

#if defined(NTDDI_WIN10_RS2)
#    include <dxgi1_6.h>
#elif defined(NTDDI_WIN10_RS2)
#    include <dxgi1_5.h>
#else
#   include <dxgi1_4.h>
#endif
#include <d3d12.h>

#include "../Types.h"
#include "../PixelFormat.h"
#include "../../Core/Debug.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#ifndef D3D12_DEBUG
#   ifndef NDEBUG
#       define D3D12_DEBUG 1
#   else
#       define D3D12_DEBUG 0
#   endif
#endif

#ifdef D3D12_DEBUG
#    include <dxgidebug.h>
#endif

namespace alimer
{
    class GraphicsDeviceD3D12;

    static constexpr uint32_t RenderLatency = 2u;

    struct D3D12Fence
    {
        ID3D12Fence* D3DFence = nullptr;
        HANDLE FenceEvent = INVALID_HANDLE_VALUE;

        ~D3D12Fence();

        void Initialize(ID3D12Device* device, uint64_t initialValue = 0);
        void Shutdown();

        void Signal(ID3D12CommandQueue* queue, uint64_t fenceValue);
        void Wait(uint64_t fenceValue);
        bool Signaled(uint64_t fenceValue);
        void Clear(uint64_t fenceValue);
    };

    class D3D12Resource
    {
    public:
        ID3D12Resource * operator->() { return _resource.Get(); }
        const ID3D12Resource* operator->() const { return _resource.Get(); }

        ID3D12Resource* GetResource() { return _resource.Get(); }
        const ID3D12Resource* GetResource() const { return _resource.Get(); }

        D3D12_RESOURCE_STATES GetUsageState() const { return _usageState; }
        void SetUsageState(D3D12_RESOURCE_STATES state) { _usageState = state; }

        D3D12_RESOURCE_STATES GetTransitioningState() const { return _transitioningState; }
        void SetTransitioningState(D3D12_RESOURCE_STATES state) { _transitioningState = state; }

        D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return _gpuVirtualAddress; }

    protected:
        Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
        D3D12_RESOURCE_STATES _usageState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_STATES _transitioningState = (D3D12_RESOURCE_STATES)-1;
        D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        uint8_t* _cpuAddress = nullptr;
    };

    static inline D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(CompareOp op)
    {
        switch (op)
        {
        case CompareOp::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LessEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GreaterEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            ALIMER_UNREACHABLE();
            return (D3D12_COMPARISON_FUNC)0;
        }
    }
}
