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
    struct Vertex
    {
        Vector3 position;
        Color color;
    };

    class TriangleExample
    {
    public:
        void Initialize(Graphics* graphics)
        {
            Vertex triangleVertices[] =
            {
                { vec3(0.0f, 0.5f, 0.0f), Color::Red },
                { vec3(0.5f, -0.5f, 0.0f), Color::Lime },
                { vec3(-0.5f, -0.5f, 0.0f), Color::Blue }
            };

            GpuBufferDescription vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::Vertex;
            vertexBufferDesc.elementCount = 3;
            vertexBufferDesc.elementSize = sizeof(Vertex);
            vertexBufferDesc.resourceUsage = ResourceUsage::Immutable;
            _vertexBuffer = graphics->CreateBuffer(vertexBufferDesc, triangleVertices);

            RenderPipelineDescription renderPipelineDesc = {};
            renderPipelineDesc.shader = graphics->CreateShader("color.vert", "color.frag");
            renderPipelineDesc.vertexDescription.attributes[0].format = VertexFormat::Float3;
            renderPipelineDesc.vertexDescription.attributes[1].format = VertexFormat::Float4;
            renderPipelineDesc.vertexDescription.attributes[1].offset = 12;
            renderPipelineDesc.vertexDescription.layouts[0].stride = _vertexBuffer->GetElementSize();
            _renderPipeline = graphics->CreateRenderPipelineState(renderPipelineDesc);


            _camera.viewMatrix = Matrix4x4::Identity;
            _camera.projectionMatrix = Matrix4x4::Identity;
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetPipeline(_renderPipeline.Get());
            commandBuffer->SetVertexBuffer(_vertexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<PipelineState> _renderPipeline;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class QuadExample
    {
    public:
        void Initialize(Graphics* graphics)
        {
            Vertex triangleVertices[] =
            {
                { Vector3(-0.5f, 0.5f, 0.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },
                { Vector3(0.5f, 0.5f, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
                { Vector3(0.5f, -0.5f, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
                { Vector3(-0.5f, -0.5f, 0.0f), Color(1.0f, 1.0f, 0.0f, 1.0f) },
            };

            GpuBufferDescription vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::Vertex;
            vertexBufferDesc.elementCount = 4;
            vertexBufferDesc.elementSize = sizeof(Vertex);
            vertexBufferDesc.resourceUsage = ResourceUsage::Immutable;
            _vertexBuffer = graphics->CreateBuffer(vertexBufferDesc, triangleVertices);

            const uint16_t indices[] = {
                0, 1, 2, 0, 2, 3
            };

            GpuBufferDescription indexBufferDesc = {};
            indexBufferDesc.usage = BufferUsage::Index;
            indexBufferDesc.elementCount = 6;
            indexBufferDesc.elementSize = sizeof(uint16_t);
            indexBufferDesc.resourceUsage = ResourceUsage::Immutable;
            _indexBuffer = graphics->CreateBuffer(indexBufferDesc, indices);

            RenderPipelineDescription renderPipelineDesc = {};
            renderPipelineDesc.shader = graphics->CreateShader("color.vert", "color.frag");
            renderPipelineDesc.vertexDescription.attributes[0].format = VertexFormat::Float3;
            renderPipelineDesc.vertexDescription.attributes[1].format = VertexFormat::Float4;
            renderPipelineDesc.vertexDescription.attributes[1].offset = 12;
            renderPipelineDesc.vertexDescription.layouts[0].stride = _vertexBuffer->GetElementSize();
            _renderPipeline = graphics->CreateRenderPipelineState(renderPipelineDesc);


            _camera.viewMatrix = Matrix4x4::Identity;
            _camera.projectionMatrix = Matrix4x4::Identity;
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetPipeline(_renderPipeline.Get());
            commandBuffer->SetVertexBuffer(_vertexBuffer.Get());
            commandBuffer->SetIndexBuffer(_indexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->DrawIndexed(PrimitiveTopology::Triangles, 6);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<PipelineState> _renderPipeline;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override = default;

    private:
        void Initialize() override;
        void OnRender(CommandBuffer* commandBuffer) override;

    private:
        TriangleExample _triangleExample;
        QuadExample _quadExample;
    };

    RuntimeApplication::RuntimeApplication()
    {
    }

    void RuntimeApplication::Initialize()
    {
        // _triangleExample.Initialize(_graphics);
        _quadExample.Initialize(_graphics);
        // Create scene
       // auto triangleEntity = _scene->CreateEntity();
       // triangleEntity->AddComponent<TransformComponent>();
       // triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }

    void RuntimeApplication::OnRender(CommandBuffer* commandBuffer)
    {
        //_triangleExample.Render(commandBuffer);
        _quadExample.Render(commandBuffer);
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
