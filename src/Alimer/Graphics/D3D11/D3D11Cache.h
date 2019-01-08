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

#include "../../Base/HashMap.h"
#include "../Pipeline.h"
#include "D3D11Prerequisites.h"
#include <unordered_map>

namespace alimer
{
    class ShaderModule;
    class DeviceD3D11;

    /// D3D11 cache.
    class D3D11Cache final 
    {
    public:
        /// Constructor.
        D3D11Cache(DeviceD3D11* device);

        /// Destructor.
        ~D3D11Cache();

        void Clear();

        ID3D11InputLayout* GetInputLayout(ShaderModule* shader, const RenderPipelineDescriptor* descriptor);

    private:
        DeviceD3D11* _device;
        HashMap<std::vector<uint8_t>> _vsBytecodes;
        HashMap<Microsoft::WRL::ComPtr<ID3D11InputLayout>> _inputLayouts;
    };
}
