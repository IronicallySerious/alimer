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

#include "../Shader.h"
#include "D3D11Prerequisites.h"
#include <array>

namespace Alimer
{
	class DeviceD3D11;


    /*class D3D11Shader final : public Shader
    {
    public:
        D3D11Shader(DeviceD3D11* device, const ShaderDescriptor* descriptor);
        ~D3D11Shader() override;
        void Destroy();

        void Bind(ID3D11DeviceContext* context);

        using BindingIndexInfo = std::array<std::array<uint32_t, MaxBindingsPerSet>, MaxDescriptorSets>;
        const BindingIndexInfo& GetBindingIndexInfo() const { return _indexInfo; }

    private:
        ID3D11VertexShader* _vertexShader = nullptr;
        ID3D11PixelShader* _pixelShader = nullptr;
        ID3D11ComputeShader* _computeShader = nullptr;
        BindingIndexInfo _indexInfo;
    };
    */
}
