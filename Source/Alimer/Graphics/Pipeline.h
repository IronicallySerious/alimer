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

#include "../Graphics/Shader.h"

namespace Alimer
{
    struct VertexAttributeDescriptor
    {
        VertexElementFormat             format = VertexElementFormat::Unknown;
        uint32_t                        offset = 0;
        uint32_t                        bufferIndex = 0;
    };

    struct VertexBufferLayoutDescriptor
    {
        uint32_t                        stride = 0;
        VertexInputRate                 inputRate = VertexInputRate::Vertex;
    };

    struct VertexDescriptor
    {
        VertexBufferLayoutDescriptor    layouts[AGPU_MAX_VERTEX_BUFFER_BINDINGS];
        VertexAttributeDescriptor       attributes[AGPU_MAX_VERTEX_ATTRIBUTES];
    };

    struct RenderPipelineDescriptor
    {
        SharedPtr<Shader>               shader;

        VertexDescriptor                vertexDescriptor;
        PrimitiveTopology               primitiveTopology = PrimitiveTopology::TriangleList;
    };

    /// Defines a Pipeline class.
    class ALIMER_API Pipeline final : public GraphicsResource
    {
    public:
        /// Constructor.
        Pipeline();

        ~Pipeline() override;
        void Destroy() override;

        bool Define(const RenderPipelineDescriptor* descriptor);

        AgpuPipeline GetHandle() const { return _handle; }

        /// Get whether pipeline is compute.
        bool IsCompute() const { return _isCompute; }

    private:
        bool _isCompute;
        AgpuPipeline _handle;
    };
}
