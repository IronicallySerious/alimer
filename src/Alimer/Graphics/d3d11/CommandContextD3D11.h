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

#include "../CommandContext.h"
#include "BackendD3D11.h"

namespace alimer
{
    class FramebufferD3D11;
    class PipelineD3D11;
    class DeviceD3D11;

    class CommandContextD3D11 final : public CommandContext
    {
    public:
        CommandContextD3D11(DeviceD3D11* device);
        ~CommandContextD3D11() override;

    private:
        uint64_t FlushImpl(bool waitForCompletion) override;

        void PushDebugGroupImpl(const String& name) override;
        void PopDebugGroupImpl() override;
        void InsertDebugMarkerImpl(const String& name) override;

        void BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) override;
        void EndRenderPassImpl() override;

        void SetViewport(const RectangleF& viewport) override;
        void SetViewport(uint32_t viewportCount, const RectangleF* viewports) override;
        void SetScissor(const Rectangle& scissor)  override;
        void SetScissor(uint32_t scissorCount, const Rectangle* scissors) override;
        void SetBlendColor(const Color4& color) override;
        void SetStencilReference(uint32_t reference) override;

        void SetPipelineImpl(Pipeline* pipeline);

        //void SetVertexBufferImpl(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate) override;
        //void SetIndexBufferImpl(Buffer* buffer, uint32_t offset, IndexType indexType) override;

        void DrawInstancedImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void BeginContext();
        void FlushRenderState();
        void FlushComputeState();
        void FlushDescriptorSet(uint32_t set);
        void FlushDescriptorSets();

    private:
        ID3D11DeviceContext*        _context;
        ID3D11DeviceContext1*       _context1;
        ID3DUserDefinedAnnotation*  _annotation;
        bool                        _immediate;
        bool                        _needWorkaround = false;
        uint64_t                    _fenceValue;

        const FramebufferD3D11*     _currentFramebuffer = nullptr;
        PrimitiveTopology           _currentTopology;
        const PipelineD3D11*        _graphicsPipeline = nullptr;
        const PipelineD3D11*        _computePipeline = nullptr;
        ID3D11BlendState*           _blendState;
        ID3D11DepthStencilState*    _depthStencilState;
        ID3D11RasterizerState*      _rasterizerState;

        ID3D11VertexShader*         _currentVertexShader = nullptr;
        ID3D11PixelShader*          _currentPixelShader = nullptr;
        ID3D11ComputeShader*        _currentComputeShader = nullptr;
        ID3D11InputLayout*          _currentInputLayout = nullptr;

        struct VertexBindingState
        {
            ID3D11Buffer*               buffers[MaxVertexBufferBindings];
            UINT                        offsets[MaxVertexBufferBindings];
            UINT                        strides[MaxVertexBufferBindings];
            VertexInputRate             inputRates[MaxVertexBufferBindings];
        } _vbo = {};

        // RenderTargetViews
        uint32_t                    _renderTargetsViewsCount;

        // Viewports
        bool                        _viewportsDirty;
        D3D11_VIEWPORT              _viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _viewportsCount;

        // Scissors
        bool                        _scissorsDirty;
        D3D11_RECT                  _scissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _scissorsCount;

        Color4                      _blendColor;
        uint32_t                    _stencilReference;

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

        //ResourceBindings _bindings;
        uint32_t        _dirtySets = 0;
        uint32_t        _dirtyVbos = 0;
    };
}
