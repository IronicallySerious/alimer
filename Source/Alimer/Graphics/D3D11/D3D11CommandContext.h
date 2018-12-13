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

#include "../CommandContext.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11Pipeline;
    class D3D11Framebuffer;
    class D3D11Graphics;

    class D3D11CommandContext final : public CommandContext
    {
    public:
        D3D11CommandContext(D3D11Graphics* graphics);
        ~D3D11CommandContext() override;

        uint64_t FlushImpl(bool waitForCompletion) override;
        void BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) override;
        void EndRenderPassImpl() override;

        //void SetVertexBufferImpl(GpuBuffer* buffer, uint32_t offset) override;
        //void SetVertexBuffersImpl(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets) override;
        void SetIndexBufferImpl(GpuBuffer* buffer, uint32_t offset, IndexType indexType) override;

        /*void DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t startVertexLocation) override;
        void DrawInstancedImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
        void DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        void DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

        void SetViewport(const rect& viewport) override;
        void SetScissor(const irect& scissor) override;*/

    private:
        void Reset();
        void FlushRenderState(PrimitiveTopology topology);
        void FlushDescriptorSet(uint32_t set);
        void FlushDescriptorSets();

        //void SetPipelineImpl(Pipeline* pipeline) override;
        //void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    private:
        ID3D11DeviceContext*         _d3dContext;
        ID3D11DeviceContext1*        _d3dContext1;

        bool _immediate;
        bool _needWorkaround = false;
        uint64_t _fenceValue;

        const D3D11Framebuffer* _currentFramebuffer = nullptr;
        uint32_t _currentColorAttachmentsBound;
        PrimitiveTopology _currentTopology;
        const D3D11Pipeline* _renderPipeline = nullptr;
        const D3D11Pipeline* _computePipeline = nullptr;
        ID3D11RasterizerState1* _currentRasterizerState;
        ID3D11DepthStencilState* _currentDepthStencilState;
        ID3D11BlendState1* _currentBlendState;

        ID3D11VertexShader* _vertexShader = nullptr;
        ID3D11PixelShader* _pixelShader = nullptr;
        ID3D11ComputeShader* _computeShader = nullptr;
        ID3D11InputLayout* _inputLayout = nullptr;

        struct BufferBindingInfo {
            GpuBuffer*  buffer;
            uint32_t    offset;
            uint32_t    range;
        };

        struct ResourceBinding
        {
            union {
                BufferBindingInfo buffer;
            };
        };

        struct ResourceBindings
        {
            ResourceBinding bindings[MaxDescriptorSets][MaxBindingsPerSet];
            uint8_t push_constant_data[MaxDescriptorSets];
        };

        ID3D11Buffer* _currentVertexBuffers[MaxVertexBufferBindings];
        UINT _vboStrides[MaxVertexBufferBindings];
        UINT _vboOffsets[MaxVertexBufferBindings];

        bool _inputLayoutDirty;
        ResourceBindings _bindings;
        uint32_t _dirtySets = 0;
        uint32_t _dirtyVbos = 0;
    };
}
