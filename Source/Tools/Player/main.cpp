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
#include "Serialization/Serializer.h"
using namespace Alimer;

GpuBufferPtr vertexBuffer;
std::shared_ptr<PipelineLayout> renderPipelineLayout;
PipelineStatePtr renderPipeline;

void AlimerMain(const std::vector<std::string>& args)
{
	{
		auto stream = FileSystem::Get().Open("assets://Test.json", StreamMode::WriteOnly);
		Serializer serializer(*stream.get(), SerializerMode::Write, Serializer::Type::JSON);
		bool testVal = true;
		int16_t testInt16 = std::numeric_limits<int16_t>::min();
		uint16_t testUInt16 = std::numeric_limits<uint16_t>::max();
		float floatV = 3.145f;
		double floatD = 6.26;
		Vector3 vector3 = Vector3::UnitY;
		serializer.Serialize("bool", testVal);
		serializer.Serialize("int16_t", testInt16);
		serializer.Serialize("uint16_t", testUInt16);
		serializer.Serialize("float", floatV);
		serializer.Serialize("double", floatD);
		serializer.Serialize("vector3", vector3);
	}

	// Read now.
	{
		auto stream = FileSystem::Get().Open("assets://Test.json", StreamMode::ReadOnly);
		Serializer serializer(*stream.get(), SerializerMode::Read, Serializer::Type::JSON);
		bool testVal;
		int16_t testInt16;
		uint16_t testUInt16;
		float floatV;
		double floatD;
		Vector3 vector3;

		serializer.Serialize("bool", testVal);
		serializer.Serialize("int16_t", testInt16);
		serializer.Serialize("uint16_t", testUInt16);
		serializer.Serialize("float", floatV);
		serializer.Serialize("double", floatD);
		serializer.Serialize("vector3", vector3);
	}

	struct Vertex
	{
		Vector3 position;
		Color color;
	};

	const float aspectRatio = static_cast<float>(engine->GetMainWindow()->GetWidth()) / engine->GetMainWindow()->GetHeight();

	Vertex triangleVertices[] =
	{
		{ Vector3(0.0f, 0.25f * aspectRatio, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f)},
		{ Vector3(0.25f, -0.25f * aspectRatio, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(-0.25f, -0.25f * aspectRatio, 0.0f),Color(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	vertexBuffer = graphics->CreateBuffer(BufferUsage::Vertex, 3, sizeof(Vertex), triangleVertices);

	renderPipelineLayout = graphics->CreatePipelineLayout();

	RenderPipelineDescriptor renderPipelineDescriptor;
	renderPipelineDescriptor.vertex = graphics->CreateShader("color.vert");
	renderPipelineDescriptor.fragment = graphics->CreateShader("color.frag");
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
