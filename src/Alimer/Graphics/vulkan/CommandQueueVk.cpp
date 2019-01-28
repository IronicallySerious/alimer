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

#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    CommandQueueVk::CommandQueueVk(GPUDeviceVk* device, VkQueue queue, uint32_t queueFamilyIndex)
        : _device(device)
        , _queue(queue)
    {
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        if (vkCreateCommandPool(device->GetVkDevice(), &createInfo, nullptr, &_commandPool) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create command pool");
        }
    }

    CommandQueueVk::~CommandQueueVk()
    {
        if (_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_device->GetVkDevice(), _commandPool, nullptr);
            _commandPool = VK_NULL_HANDLE;
        }
    }

    void CommandQueueVk::ExecuteCommandBuffer(CommandBufferVk* commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
    {
        commandBuffer->End();

        const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkCommandBuffer vkCommandBuffer = commandBuffer->GetVkCommandBuffer();
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;
        vkThrowIfFailed(vkQueueSubmit(_queue, 1, &submitInfo, fence));
    }
}
