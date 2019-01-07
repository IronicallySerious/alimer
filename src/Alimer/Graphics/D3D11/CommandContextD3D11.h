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

#include "../GPUDeviceImpl.h"
#include "D3D11Prerequisites.h"

namespace alimer
{
    class D3D11Pipeline;
    class FramebufferD3D11;
    class DeviceD3D11;

    class CommandBufferD3D11 final : public GPUCommandBuffer
    {
    public:
        CommandBufferD3D11(DeviceD3D11* device);
        ~CommandBufferD3D11() override;

        uint64_t Flush(bool waitForCompletion) override;
        void BeginRenderPass(GPUFramebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) override;
        void EndRenderPass() override;

        void SetViewport(const RectangleF& viewport) override;
        void SetViewport(uint32_t viewportCount, const RectangleF* viewports) override;
        void SetScissor(const Rectangle& scissor)  override;
        void SetScissor(uint32_t scissorCount, const Rectangle* scissors) override;
        void SetBlendConstants(const float blendConstants[4]) override;

        void SetVertexBuffer(uint32_t binding, GPUBuffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate) override;
        void SetIndexBuffer(GPUBuffer* buffer, uint32_t offset, IndexType indexType) override;

        /*
        void DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        void DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

       */

    private:
        void BeginContext();
        void FlushRenderState();
        void FlushDescriptorSet(uint32_t set);
        void FlushDescriptorSets();

        //void SetPrimitiveTopologyCore(PrimitiveTopology topology) override;
        //void DrawInstancedCore(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        //void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    private:
        ID3D11DeviceContext*        _context;
        ID3D11DeviceContext1*       _context1;
        ID3DUserDefinedAnnotation*  _annotation;
        bool                        _immediate;
        bool                        _needWorkaround = false;
        uint64_t                    _fenceValue;

        const FramebufferD3D11*     _currentFramebuffer = nullptr;
        uint32_t                    _currentColorAttachmentsBound;
        PrimitiveTopology           _currentTopology;
        const D3D11Pipeline*        _renderPipeline = nullptr;
        const D3D11Pipeline*        _computePipeline = nullptr;
        ID3D11RasterizerState1*     _currentRasterizerState;
        ID3D11DepthStencilState*    _currentDepthStencilState;
        ID3D11BlendState1*          _currentBlendState;

        ID3D11VertexShader*         _vertexShader = nullptr;
        ID3D11PixelShader*          _pixelShader = nullptr;
        ID3D11ComputeShader*        _computeShader = nullptr;
        ID3D11InputLayout*          _inputLayout = nullptr;

        struct VertexBindingState
        {
            ID3D11Buffer*   buffers[MaxVertexBufferBindings];
            UINT            offsets[MaxVertexBufferBindings];
            UINT            strides[MaxVertexBufferBindings];
            VertexInputRate inputRates[MaxVertexBufferBindings];
        } _vbo = {};

        // Viewports
        bool                        _viewportsDirty;
        D3D11_VIEWPORT              _viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _viewportsCount;

        // Scissors
        bool                        _scissorsDirty;
        D3D11_RECT                  _scissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _scissorsCount;

        /*struct BufferBindingInfo {
            Buffer*  buffer;
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
        };*/

        
        
        bool            _inputLayoutDirty;

        //ResourceBindings _bindings;
        uint32_t        _dirtySets = 0;
        uint32_t        _dirtyVbos = 0;
    };
}
