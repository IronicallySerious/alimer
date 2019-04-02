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

#include "CommandBufferD3D12.h"
#include "GraphicsDeviceD3D12.h"

namespace alimer
{
    CommandBufferD3D12::CommandBufferD3D12(GraphicsDeviceD3D12* device)
        : CommandBuffer(device)
    {
        // Create the command list.
        ThrowIfFailed(device->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
        ThrowIfFailed(device->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_PPV_ARGS(&_commandList)));
        //ThrowIfFailed(_commandList->Close());
    }

    CommandBufferD3D12::~CommandBufferD3D12()
    {
        ALIMER_VERIFY(_commandAllocator->Release() == 0);
        ALIMER_VERIFY(_commandList->Release() == 0);
    }
}
