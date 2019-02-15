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

#include "FramebufferD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "TextureD3D12.h"
#include "../../Core/Log.h"

using namespace std;
using namespace Microsoft::WRL;

namespace alimer
{

    D3D12Framebuffer::D3D12Framebuffer(GraphicsDeviceD3D12* device, const vector<FramebufferAttachment>& colorAttachments)
       // : _device(device)
    {
        if (colorAttachments.size() > 0)
        {
            _rtvHandle = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, UINT(colorAttachments.size()));

            for (size_t i = 0, count = colorAttachments.size(); i < count; ++i)
            {
                auto textureD3D12 = static_cast<TextureD3D12*>(colorAttachments[i].texture);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHandle.GetCpuHandle(uint32_t(i));
                device->GetD3DDevice()->CreateRenderTargetView(
                    textureD3D12->GetResource(),
                    nullptr,
                    rtvHandle);
            }
        }
    }

    D3D12Framebuffer::~D3D12Framebuffer()
    {

    }
}
