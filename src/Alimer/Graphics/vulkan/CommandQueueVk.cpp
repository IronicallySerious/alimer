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

#include "../CommandQueue.h"
//#include "CommandContextVk.h"
#include "GraphicsDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    void CommandQueue::Create()
    {
        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        switch (_queueType)
        {
        case QueueType::Direct:
        default:
            createInfo.queueFamilyIndex = _device->GetImpl()->GetGraphicsQueueFamily();
            break;

        case QueueType::Compute:
            createInfo.queueFamilyIndex = _device->GetImpl()->GetComputeQueueFamily();
            break;

        case QueueType::Copy:
            createInfo.queueFamilyIndex = _device->GetImpl()->GetTransferQueueFamily();
            break;
        }

        if (vkCreateCommandPool(_device->GetImpl()->GetVkDevice(), &createInfo, nullptr, &_handle) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create command pool");
        }

        _fence = _device->GetImpl()->RequestFence();
    }

    void CommandQueue::Destroy()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            _device->GetImpl()->RecycleFence(_fence);
            vkDestroyCommandPool(_device->GetImpl()->GetVkDevice(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    void CommandQueue::Submit(SharedPtr<CommandBuffer> commandBuffer, bool waitForCompletion)
    {
        ALIMER_ASSERT(commandBuffer->GetCommandQueue() == this);

        if (commandBuffer.IsNull()) {
            return;
        }

        if (commandBuffer->GetStatus() == CommandBufferStatus::Committed) {
            return;
        }

        // End command buffer recording.
        commandBuffer->End();

        VkCommandBuffer vkCommandBuffer = commandBuffer->GetHandle();
        VkDevice vkDevice = _device->GetImpl()->GetVkDevice();
        VkQueue vkQueue = _device->GetImpl()->GetQueue(_queueType);
        VkFence vkFence = waitForCompletion ? _fence : commandBuffer->GetFence();

        //const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //submitInfo.pWaitDstStageMask = &waitDstStageMask;
        //submitInfo.pWaitSemaphores = &_presentCompleteSemaphores[_frameIndex];
        //submitInfo.waitSemaphoreCount = 1u;
        submitInfo.pSignalSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        submitInfo.commandBufferCount = 1u;
        vkThrowIfFailed(vkQueueSubmit(vkQueue, 1, &submitInfo, vkFence));

        if (waitForCompletion)
        {
            // Wait for the fence to signal that command buffer has finished executing.
            vkThrowIfFailed(vkWaitForFences(vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX));
            vkThrowIfFailed(vkResetFences(vkDevice, 1, &vkFence));
            _availableCommandBuffers.Push(commandBuffer);
            commandBuffer->SetStatus(CommandBufferStatus::Completed);
        }
        else
        {
            commandBuffer->SetStatus(CommandBufferStatus::Committed);
            _submittedCommandBuffers.Push(commandBuffer);
        }
    }

    bool CommandQueue::IsCompletted(const SharedPtr<CommandBuffer>& commandBuffer)
    {
        VkDevice vkDevice = _device->GetImpl()->GetVkDevice();
        VkFence vkFence = commandBuffer->GetFence();
        if (vkGetFenceStatus(vkDevice, vkFence) == VK_SUCCESS)
        {
            vkThrowIfFailed(vkResetFences(vkDevice, 1, &vkFence));
            return true;
        }

        return false;
    }
}
