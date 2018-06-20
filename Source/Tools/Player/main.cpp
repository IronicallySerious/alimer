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
        ~RuntimeApplication() override;

    private:
        void Initialize() override;
        void OnRender(const SharedPtr<Texture>& frameTexture) override;

    private:
       
        SharedPtr<GpuBuffer> _uboBuffer;
        SharedPtr<PipelineState> _renderPipeline;

        
    };

    class Settings
    {
    public:
        void Serialize(Serializer& serializer)
        {
            serializer.Serialize("width", 800);
            serializer.Serialize("height", 600);
            serializer.Serialize("fullscreen", false);
        }
    };

    RuntimeApplication::RuntimeApplication()
    {
        std::vector<char> c = { 'c','i','a','o' };
        auto s = str::ToString(ShaderStage::Geometry);
        ShaderStage stage = str::FromString<ShaderStage>(s);
        std::map<std::string, ShaderStage> map;
        map["CIAO"] = ShaderStage::Vertex;
        map["CIAO2"] = ShaderStage::Compute;
        Color color;
        Settings settings;

        {
            auto stream = OpenStream("Test.json", StreamMode::WriteOnly);
            JsonSerializer serializer(*stream.Get());

            serializer.Serialize("color", Color::Green);
            serializer.Serialize("str", "Hello World");
            serializer.Serialize("stage", ShaderStage::Compute);
            serializer.Serialize("settings", settings);

            serializer.BeginObject("testArray", true);
            serializer.Serialize(nullptr, 450);
            serializer.Serialize(nullptr, 420);
            serializer.EndObject();

            serializer.Serialize("vector", c);
            serializer.Serialize("map", map);
        }
    }

    RuntimeApplication::~RuntimeApplication()
    {
        _vertexBuffer.Reset();
        _renderPipeline.Reset();
    }

    void RuntimeApplication::Initialize()
    {
        const float aspectRatio = _window->GetAspectRatio();
        _camera.viewMatrix = glm::mat4(1.0f);
        _camera.projectionMatrix = glm::mat4(1.0f);

        GpuBufferDescription uboBufferDesc = {};
        uboBufferDesc.usage = BufferUsage::Uniform;
        uboBufferDesc.elementCount = 3;
        uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
        _uboBuffer = _graphics->CreateBuffer(uboBufferDesc, &_camera);

        RenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.shader = _graphics->CreateShader("color.vert", "color.frag");
        renderPipelineDescriptor.vertexDescriptor.attributes[0].format = VertexFormat::Float3;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].format = VertexFormat::Float4;
        renderPipelineDescriptor.vertexDescriptor.attributes[1].offset = 12;
        renderPipelineDescriptor.vertexDescriptor.layouts[0].stride = _vertexBuffer->GetElementSize();
        _renderPipeline = _graphics->CreateRenderPipelineState(renderPipelineDescriptor);

        // Create scene
        auto triangleEntity = _scene->CreateEntity();
        triangleEntity->AddComponent<TransformComponent>();
        triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }

    void RuntimeApplication::OnRender(const SharedPtr<Texture>& frameTexture)
    {
        /*SharedPtr<CommandBuffer> commandBuffer = _graphics->GetCommandQueue()->CreateCommandBuffer();

        RenderPassDescriptor passDescriptor = {};
        passDescriptor.colorAttachments[0].texture = frameTexture;
        passDescriptor.colorAttachments[0].clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        commandBuffer->BeginRenderPass(passDescriptor);
        commandBuffer->SetVertexBuffer(_vertexBuffer.Get(), 0);
        commandBuffer->SetPipeline(_renderPipeline);
        commandBuffer->SetUniformBuffer(0, 0, _uboBuffer.Get());
        commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
        commandBuffer->EndRenderPass();

        commandBuffer->Commit();*/
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
