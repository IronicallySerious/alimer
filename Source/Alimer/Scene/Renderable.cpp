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

#include "../Scene/Scene.h"
#include "../Scene/TransformComponent.h"
#include "../Scene/Renderable.h"
namespace Alimer
{
    TriangleRenderable::TriangleRenderable()
    {
        /*struct Vertex
        {
            glm::vec3 position;
            Color color;
        };

        const float aspectRatio = 800.0f / 600.0f;

        Vertex triangleVertices[] =
        {
            { glm::vec3(0.0f, 0.25f * aspectRatio, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
            { glm::vec3(0.25f, -0.25f * aspectRatio, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
            { glm::vec3(-0.25f, -0.25f * aspectRatio, 0.0f),Color(0.0f, 0.0f, 1.0f, 1.0f) }
        };

        GpuBufferDescription vertexBufferDesc = {};
        vertexBufferDesc.usage = BufferUsage::Vertex;
        vertexBufferDesc.elementCount = 3;
        vertexBufferDesc.elementSize = sizeof(Vertex);
        _vertexBuffer = gGraphics().CreateBuffer(vertexBufferDesc, triangleVertices);*/

        /*RenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.shader = gGraphics().CreateShader("color.vert", "color.frag");
        renderPipelineDescriptor.vertexDescriptor.attributes[0].format = VertexFormat::Float3;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].format = VertexFormat::Float4;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].offset = 12;
        renderPipelineDescriptor.vertexDescriptor.layouts[0].stride = _vertexBuffer->GetElementSize();
        _renderPipeline = gGraphics().CreateRenderPipelineState(renderPipelineDescriptor);*/
    }

    TriangleRenderable::~TriangleRenderable()
    {

    }

    void TriangleRenderable::Render(CommandBuffer* commandBuffer)
    {
        /*commandBuffer->SetVertexBuffer(_vertexBuffer.Get(), 0);
        commandBuffer->SetPipeline(_renderPipeline);
        commandBuffer->Draw(PrimitiveTopology::Triangles, 3);*/
    }
}
