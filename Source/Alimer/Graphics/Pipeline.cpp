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

#include "../Graphics/Pipeline.h"
#include "../Graphics/Graphics.h"
#include "../Core/Log.h"

namespace Alimer
{
    Pipeline::Pipeline(GPUDevice* device)
        : GraphicsResource(device)
        , _isCompute(true)
    {
    }

    Pipeline::~Pipeline()
    {
        Destroy();
    }

    void Pipeline::Destroy()
    {
    }

    bool Pipeline::Define(const RenderPipelineDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
#ifdef _DEBUG
        if (descriptor->shader == nullptr)
        {
            ALIMER_LOGERROR("RenderPipeline: Invalid vertex shader.");
        }

        /*if (descriptor->fragment == nullptr)
        {
            ALIMER_LOGERROR("RenderPipeline: Invalid fragment shader.");
        }*/
#endif

        Destroy();
        _isCompute = false;

        /*AgpuRenderPipelineDescriptor gpuPipelineDesc = {};
        gpuPipelineDesc.shader = descriptor->shader->GetHandle();

        for (uint32_t i = 0u; i < AGPU_MAX_VERTEX_BUFFER_BINDINGS; i++)
        {
            gpuPipelineDesc.vertexDescriptor.layouts[i].stride = descriptor->vertexDescriptor.layouts[i].stride;
            gpuPipelineDesc.vertexDescriptor.layouts[i].inputRate = static_cast<AgpuVertexInputRate>(descriptor->vertexDescriptor.layouts[i].inputRate);
        }

        for (uint32_t i = 0u; i < AGPU_MAX_VERTEX_ATTRIBUTES; i++)
        {
            gpuPipelineDesc.vertexDescriptor.attributes[i].format = static_cast<AgpuVertexFormat>(descriptor->vertexDescriptor.attributes[i].format);
            gpuPipelineDesc.vertexDescriptor.attributes[i].offset = descriptor->vertexDescriptor.attributes[i].offset;
            gpuPipelineDesc.vertexDescriptor.attributes[i].bufferIndex = descriptor->vertexDescriptor.attributes[i].bufferIndex;
        }

        gpuPipelineDesc.primitiveTopology = static_cast<AgpuPrimitiveTopology>(descriptor->primitiveTopology);
        _handle = agpuCreateRenderPipeline(&gpuPipelineDesc);*/

        return true;
    }
}
