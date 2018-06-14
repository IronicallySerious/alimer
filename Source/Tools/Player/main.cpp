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
        void OnRender() override;

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _uboBuffer;
        SharedPtr<PipelineState> _renderPipeline;

        struct PerCameraCBuffer
        {
            Matrix4x4 viewMatrix;
            Matrix4x4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
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
        std::map<String, ShaderStage> map;
        map["CIAO"] = ShaderStage::Vertex;
        map["CIAO2"] = ShaderStage::Compute;
        Color color;
        Settings settings;

        {
            auto stream = OpenStream("Test.json", StreamMode::WriteOnly);
            JsonSerializer serializer(*stream.Get());
            settings.Serialize(serializer);

            //serializer.Serialize("color", Color::Green);
           // serializer.Serialize("str", "Hello World");
           // serializer.Serialize("stage", ShaderStage::Compute);
           // serializer.Serialize("settings", settings);

            serializer.BeginObject("testArray", true);
            serializer.Serialize(nullptr, 450);
            serializer.Serialize(nullptr, 420);
            serializer.EndObject();

            //serializer.Serialize("vector", c);
            //serializer.Serialize("map", map);
        }
    }

    RuntimeApplication::~RuntimeApplication()
    {
        _vertexBuffer.Reset();
        _renderPipeline.Reset();
    }

    void RuntimeApplication::Initialize()
    {
        struct Vertex
        {
            Vector3 position;
            Color color;
        };

        const float aspectRatio = _window->GetAspectRatio();

        Vertex triangleVertices[] =
        {
            { Vector3(0.0f, 0.25f * aspectRatio, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
            { Vector3(0.25f, -0.25f * aspectRatio, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
            { Vector3(-0.25f, -0.25f * aspectRatio, 0.0f),Color(0.0f, 0.0f, 1.0f, 1.0f) }
        };

        _vertexBuffer = _graphics->CreateBuffer(BufferUsage::Vertex, 3, sizeof(Vertex), triangleVertices);

        _camera.viewMatrix = Matrix4x4::Identity;
        _camera.projectionMatrix = Matrix4x4::Identity;
        _uboBuffer = _graphics->CreateBuffer(BufferUsage::Uniform, 1, sizeof(PerCameraCBuffer), &_camera);

        RenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.shader = _graphics->CreateShader("color.vert", "color.frag");
        renderPipelineDescriptor.vertexElements[0].format = VertexFormat::Float3;
        renderPipelineDescriptor.vertexElements[1].format = VertexFormat::Float4;
        renderPipelineDescriptor.vertexElements[1].offset = 12;
        _renderPipeline = _graphics->CreateRenderPipelineState(renderPipelineDescriptor);
    }

    void RuntimeApplication::OnRender()
    {
        SharedPtr<CommandBuffer> commandBuffer = _graphics->GetCommandQueue()->CreateCommandBuffer();
        commandBuffer->Commit();
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);

#if TODO
void AlimerMain(const std::vector<std::string>& args)
{
    
}

void AlimerShutdown()
{
    vertexBuffer.Reset();
    renderPipeline.Reset();
}

void AlimerRender(const SharedPtr<Texture>& frameTexture)
{
    
    RenderPassDescriptor passDescriptor;
    passDescriptor.colorAttachments[0].texture = frameTexture;
    passDescriptor.colorAttachments[0].clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandBuffer->BeginRenderPass(passDescriptor);
    commandBuffer->SetVertexBuffer(vertexBuffer.Get(), 0);
    commandBuffer->SetPipeline(renderPipeline);
    commandBuffer->SetUniformBuffer(0, 0, uboBuffer.Get());
    commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
    commandBuffer->EndRenderPass();
    //commandBuffer->Commit();
}
#endif // TODO

