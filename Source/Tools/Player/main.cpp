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

#include "Alimer.h"
using namespace Alimer;

namespace Alimer
{
    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override = default;

    private:
        void Initialize() override;
        void OnRender(CommandBuffer* commandBuffer) override;

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<PipelineState> _renderPipeline;
    };

    RuntimeApplication::RuntimeApplication()
    {
        auto test = GetOSDescription();
    }

    void RuntimeApplication::Initialize()
    {
        struct Vertex
        {
            glm::vec3 position;
            Color color;
        };

        Vertex triangleVertices[] =
        {
            { glm::vec3(0.0f, 0.5f, 0.0f), Color(1.0f, 0.0f, 0.0f) },
            { glm::vec3(0.5f, -0.5f, 0.0f), Color(0.0f, 1.0f, 0.0f) },
            { glm::vec3(-0.5f, -0.5f, 0.0f),Color(0.0f, 0.0f, 1.0f) }
        };

        GpuBufferDescription vertexBufferDesc = {};
        vertexBufferDesc.usage = BufferUsage::Vertex;
        vertexBufferDesc.elementCount = 3;
        vertexBufferDesc.elementSize = sizeof(Vertex);
        _vertexBuffer = _graphics->CreateBuffer(vertexBufferDesc, triangleVertices);

        RenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.shader = _graphics->CreateShader("color.vert", "color.frag");
        renderPipelineDescriptor.vertexDescriptor.attributes[0].format = VertexFormat::Float3;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].format = VertexFormat::Float4;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].offset = 12;
        renderPipelineDescriptor.vertexDescriptor.layouts[0].stride = _vertexBuffer->GetElementSize();
        //_renderPipeline = _graphics->CreateRenderPipelineState(renderPipelineDescriptor);*/

        // Create scene
       // auto triangleEntity = _scene->CreateEntity();
       // triangleEntity->AddComponent<TransformComponent>();
       // triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }

    void RuntimeApplication::OnRender(CommandBuffer* commandBuffer)
    {
        commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
        commandBuffer->EndRenderPass();

        //commandBuffer->SetPipeline(_renderPipeline);
        //commandBuffer->SetVertexBuffer(_vertexBuffer.Get(), 0);
        //commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
