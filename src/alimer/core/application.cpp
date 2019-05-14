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

//#include "core/log.h"
#include "core/application.h"
#include <vgpu.h>

namespace alimer
{
    Application::Application()
    {
    }

    Application::~Application()
    {
        // Shutdown vgpu
        vgpuShutdown();
    }

    static VGpuBuffer vertex_buffer;
    static VGpuPipeline renderPipeline;

    void Application::initialize()
    {
        const float vertices[] = {
            // positions            colors 
             0.0f, 0.5f, 0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
        };

        vertex_buffer = vgpuCreateBuffer(sizeof(vertices), VGPU_BUFFER_USAGE_VERTEX, VGPU_RESOURCE_USAGE_IMMUTABLE, vertices);
        const char* vertexShaderSource = R"(
        layout (location = 0) in vec3 vgpuPosition;
        layout (location = 1) in vec4 vgpuVertexColor;

        out vec4 vertexColor;

        void main()
        {
            vertexColor = vgpuVertexColor;
            gl_Position = vec4(vgpuPosition, 1.0);
        })";

        const char* fragmentShaderSource = R"(
        in vec4 vertexColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = vertexColor;
        })";

        VGpuRenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.shader = vgpuCreateShader(vertexShaderSource, fragmentShaderSource);
        pipelineDesc.primitiveTopology = VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineDesc.depthStencil.depthCompareFunction = VGPU_COMPARE_FUNCTION_LESS_EQUAL;
        pipelineDesc.depthStencil.depthWriteEnabled = true;
        pipelineDesc.vertexDescriptor.layouts[0].stride = 28;
        pipelineDesc.vertexDescriptor.attributes[0].format = VGPU_VERTEX_FORMAT_FLOAT3;
        pipelineDesc.vertexDescriptor.attributes[1].offset = 12;
        pipelineDesc.vertexDescriptor.attributes[1].format = VGPU_VERTEX_FORMAT_FLOAT4;
        renderPipeline = vgpuCreateRenderPipeline(&pipelineDesc);
    }

    void Application::frame()
    {
        vgpuBeginDefaultRenderPass({ 0.2f, 0.3f, 0.3f, 1.0f }, 1.0f, 0);

        // Get frame command buffer for recording.
        vgpuBindPipeline(renderPipeline);
        vgpuDraw(3, 1, 0);

        vgpuEndRenderPass();

        /*VGpuCommandBuffer commandBuffer = vgpuGetCommandBuffer();
        vgpuBeginCommandBuffer(commandBuffer);
        vgpuCmdBeginDefaultRenderPass(commandBuffer, { 0.5f, 0.5f, 0.5f, 1.0f}, 1.0f, 0);
        vgpuCmdEndRenderPass(commandBuffer);
        vgpuEndCommandBuffer(commandBuffer);
        vgpuSubmitCommandBuffer(commandBuffer);*/

        // Submit GPU frame.
        vgpuFrame();
    }
}
