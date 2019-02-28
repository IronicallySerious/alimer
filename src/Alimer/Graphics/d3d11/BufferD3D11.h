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

#pragma once

#include "../Buffer.h"
#include "BackendD3D11.h"

namespace alimer
{
	/// D3D11 GpuBuffer implementation.
	class BufferD3D11 final : public Buffer
	{
	public:
		/// Constructor.
        BufferD3D11(GraphicsDeviceD3D11* device, const BufferDescriptor* descriptor, const void* pInitData);

		/// Destructor.
		~BufferD3D11() override;

        void Destroy() override;

        bool SetSubDataImpl(uint64_t offset, uint64_t size, const void* pData) override;

        ID3D11Buffer* GetHandle() const { return _handle; }

	private:
        ID3D11DeviceContext* _deviceContext;
        ID3D11Buffer* _handle;
        bool _mappable;
	};
}
