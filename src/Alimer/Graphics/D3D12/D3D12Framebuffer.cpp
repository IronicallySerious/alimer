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

#include "D3D12Framebuffer.h"
#include "D3D12Graphics.h"
#include "D3D12Texture.h"
#include "../../Debug/Log.h"

namespace Alimer
{
    using namespace Microsoft::WRL;

    D3D12Framebuffer::D3D12Framebuffer(D3D12Graphics* graphics, const Vector<FramebufferAttachment>& colorAttachments)
        : _graphics(graphics)
    {
        if (colorAttachments.Size() > 0)
        {
            _rtvHandle = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, colorAttachments.Size());

            for (uint32_t i = 0, count = colorAttachments.Size(); i < count; ++i)
            {
                D3D12Texture* d3dTexture = static_cast<D3D12Texture*>(colorAttachments[i].texture->GetImpl());

                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHandle.GetCpuHandle(i);
                graphics->GetD3DDevice()->CreateRenderTargetView(
                    d3dTexture->GetResource(),
                    nullptr,
                    rtvHandle);
            }
        }
    }

    D3D12Framebuffer::~D3D12Framebuffer()
    {
        
    }
}
