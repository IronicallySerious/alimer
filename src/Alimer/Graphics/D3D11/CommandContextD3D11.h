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

#include "Graphics/DeviceBackend.h"
#include "D3D11Prerequisites.h"

namespace alimer
{
    class ShaderD3D11;
    class FramebufferD3D11;
    class DeviceD3D11;

    class CommandContextD3D11 final : public GPUCommandBuffer
    {
    public:
        CommandContextD3D11(DeviceD3D11* device);
        ~CommandContextD3D11() override;

    private:
        uint64_t Flush(bool waitForCompletion) override;

        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const char* name) override;

        void BeginRenderPass(const RenderPassDescriptor* descriptor) override;
        void EndRenderPass() override;

        /*void SetViewport(const RectangleF& viewport) override;
        void SetViewport(uint32_t viewportCount, const RectangleF* viewports) override;
        void SetScissor(const Rectangle& scissor)  override;
        void SetScissor(uint32_t scissorCount, const Rectangle* scissors) override;
        void SetBlendColor(float r, float g, float b, float a) override;

        void SetShaderImpl(Shader* shader) override;

        void SetVertexBufferImpl(uint32_t binding, Buffer* buffer, uint32_t offset, uint32_t stride, VertexInputRate inputRate) override;
        void SetIndexBufferImpl(Buffer* buffer, uint32_t offset, IndexType indexType) override;

        void DrawInstancedImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
    
        void DispatchImpl(uint32_t x, uint32_t y, uint32_t z) override;*/

        void BeginContext();
        void FlushRenderState();
        void FlushDescriptorSet(uint32_t set);
        void FlushDescriptorSets();

    private:
        ID3D11DeviceContext*        _context;
        ID3D11DeviceContext1*       _context1;
        ID3DUserDefinedAnnotation*  _annotation;
        bool                        _immediate;
        bool                        _needWorkaround = false;
        uint64_t                    _fenceValue;

        PrimitiveTopology           _currentTopology;
        const ShaderD3D11*          _graphicsShader = nullptr;
        const ShaderD3D11*          _computeShader = nullptr;
        ID3D11RasterizerState1*     _currentRasterizerState;
        ID3D11DepthStencilState*    _currentDepthStencilState;
        ID3D11BlendState1*          _currentBlendState;

        ID3D11VertexShader*         _currentVertexShader = nullptr;
        ID3D11PixelShader*          _currentPixelShader = nullptr;
        ID3D11ComputeShader*        _currentComputeShader = nullptr;
        ID3D11InputLayout*          _currentInputLayout = nullptr;

        struct VertexBindingState
        {
            ID3D11Buffer*               buffers[MaxVertexBufferBindings];
            const VertexDeclaration*    formats[MaxVertexBufferBindings];
            UINT                        offsets[MaxVertexBufferBindings];
            UINT                        strides[MaxVertexBufferBindings];
            VertexInputRate             inputRates[MaxVertexBufferBindings];
        } _vbo = {};

        // RenderTargetViews
        ID3D11RenderTargetView*     _renderTargetsViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        uint32_t                    _renderTargetsViewsCount;
        ID3D11DepthStencilView*     _depthStencilView = nullptr;

        // Viewports
        bool                        _viewportsDirty;
        D3D11_VIEWPORT              _viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _viewportsCount;

        // Scissors
        bool                        _scissorsDirty;
        D3D11_RECT                  _scissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
        uint32_t                    _scissorsCount;

        bool                        _vertexAttributesDirty;
        D3D11_INPUT_ELEMENT_DESC    _vertexAttributes[MaxVertexAttributes] = {};
        uint32_t                    _vertexAttributesCount;

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
