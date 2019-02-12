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

#include "PipelineVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    PipelineVk::PipelineVk(GPUDeviceVk* device, const RenderPipelineDescriptor* descriptor)
        : Pipeline(device, descriptor)
    {
        VkGraphicsPipelineCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        if (vkCreateGraphicsPipelines(device->GetVkDevice(), VK_NULL_HANDLE, 1,
            &createInfo, nullptr, &_handle) != VK_SUCCESS) {
            ALIMER_LOGERROR("Vulkan: Failed to create render pipeline");
        }
    }

    PipelineVk::PipelineVk(GPUDeviceVk* device, const ComputePipelineDescriptor* descriptor)
        : Pipeline(device, descriptor)
    {
        VkComputePipelineCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage.pNext = nullptr;
        createInfo.stage.flags = 0;
        createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        //createInfo.stage.module = ToBackend(descriptor->module)->GetHandle();
        //createInfo.stage.pName = descriptor->entryPoint;
        //createInfo.stage.pSpecializationInfo = nullptr;

        //createInfo.layout = ToBackend(descriptor->layout)->GetHandle();
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        if (vkCreateComputePipelines(device->GetVkDevice(), VK_NULL_HANDLE, 1, &createInfo,
            nullptr, &_handle) != VK_SUCCESS) {
            ALIMER_LOGERROR("Vulkan: Failed to create render pipeline");
        }
    }

    PipelineVk::~PipelineVk()
    {
        Destroy();
    }

    void PipelineVk::Destroy()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            StaticCast<GPUDeviceVk>(_device)->DestroyPipeline(_handle);
            _handle = VK_NULL_HANDLE;
        }
    }
}
