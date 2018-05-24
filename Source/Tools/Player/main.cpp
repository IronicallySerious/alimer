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

#include "Alimer/Alimer.h"
using namespace Alimer;

GpuBufferPtr vertexBuffer;
std::shared_ptr<PipelineLayout> renderPipelineLayout;
PipelineStatePtr renderPipeline;

void AlimerMain(const std::vector<std::string>& args)
{
	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct Vertex
	{
		Vector3 position;
		Vector4 color;
	};

	const float aspectRatio = static_cast<float>(engine->GetMainWindow()->GetWidth()) / engine->GetMainWindow()->GetHeight();

	Vertex triangleVertices[] =
	{
		{ { 0.0f, 0.25f * aspectRatio, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f * aspectRatio, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f * aspectRatio, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	vertexBuffer = graphics->CreateBuffer(BufferUsage::Vertex, 3, sizeof(Vertex), triangleVertices);

	renderPipelineLayout = graphics->CreatePipelineLayout();

	RenderPipelineDescriptor renderPipelineDescriptor;
	renderPipelineDescriptor.layout = renderPipelineLayout;
	renderPipelineDescriptor.vertexElements[0].format = VertexFormat::Float3;
	renderPipelineDescriptor.vertexElements[1].format = VertexFormat::Float4;
	renderPipelineDescriptor.vertexElements[1].offset = 12;
	renderPipeline = graphics->CreateRenderPipelineState(renderPipelineDescriptor);

	//auto shader = engine->GetResources()->Load<Shader>("assets://shaders/test.vert");
}

void AlimerShutdown()
{
	vertexBuffer.reset();
	renderPipeline.reset();
	renderPipelineLayout.reset();
}

void AlimerRender()
{
	auto frameTexture = graphics->AcquireNextImage();
	auto commandBuffer = graphics->CreateCommandBuffer();
	RenderPassDescriptor passDescriptor;
	passDescriptor.colorAttachments[0].texture = frameTexture.get();
	passDescriptor.colorAttachments[0].clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandBuffer->BeginRenderPass(passDescriptor);
	commandBuffer->SetVertexBuffer(vertexBuffer.get(), 0);
	commandBuffer->SetPipeline(renderPipeline);
	commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
	commandBuffer->EndRenderPass();
	commandBuffer->Commit();
}
