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

#include "../Graphics/Shader.h"

namespace alimer
{
    struct VertexBufferLayoutDescriptor {
        uint32_t                        stride;
        VertexInputRate                 inputRate;
    };

    struct VertexAttributeDescriptor {
        VertexFormat                    format;
        uint32_t                        offset;
        uint32_t                        bufferIndex;
    };

    struct VertexDescriptor {
        VertexBufferLayoutDescriptor    layouts[MaxVertexBufferBindings];
        VertexAttributeDescriptor       attributes[MaxVertexAttributes];
    };

    struct RenderPipelineDescriptor
    {
        Shader*                         shader;
        RasterizationStateDescriptor    rasterizationState;
        //BlendStateDescriptor            blendStates;
        //DepthStencilStateDescriptor     depthStencilState;
        VertexDescriptor                vertexDescriptor;
        PrimitiveTopology               primitiveTopology = PrimitiveTopology::TriangleList;
    };

    /// Defines a Pipeline class.
    class ALIMER_API Pipeline : public GPUResource
    {
    protected:
        /// Constructor.
        Pipeline(GraphicsDevice* device, const RenderPipelineDescriptor* descriptor);

    public:
        /// Get whether pipeline is compute.
        bool IsCompute() const { return _isCompute; }

    private:
        bool _isCompute;
    };
}
