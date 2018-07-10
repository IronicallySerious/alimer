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

#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphics.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"
using namespace std;

namespace Alimer
{
    VulkanCommandQueue::VulkanCommandQueue(VulkanGraphics* graphics, VkQueue vkQueue, uint32_t queueFamilyIndex)
        : _graphics(graphics)
        , _vkQueue(vkQueue)
        , _queueFamilyIndex(queueFamilyIndex)
        , _logicalDevice(graphics->GetLogicalDevice())
    {
        VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        //createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(_logicalDevice, &createInfo, nullptr, &_vkHandle) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan - failed to create command pool");
        }
    }

    VulkanCommandQueue::~VulkanCommandQueue()
    {
        // Finalize.
        /*for (size_t i = 0; i < MaxCommandBuffersPerQueue; i++)
        {
            if (_recycleCommandBuffers[i])
            {
                SafeDelete(_recycleCommandBuffers[i]);
            }
        }*/

        // Destroy command pool.
        vkDestroyCommandPool(_logicalDevice, _vkHandle, nullptr);
    }

    CommandBuffer* VulkanCommandQueue::CreateCommandBuffer()
    {
       /* VulkanCommandBuffer* commandBuffer = GetCommandBuffer();

        if (commandBuffer == nullptr)
        {
            commandBuffer = new VulkanCommandBuffer(_graphics, this);
        }

        // Begin implicitly reset the command buffer.
        commandBuffer->Begin();
        */
        return nullptr;
    }

    void VulkanCommandQueue::Enqueue(VkCommandBuffer vkCommandBuffer)
    {
        _queuedCommandBuffers.push_back(vkCommandBuffer);
    }

    void VulkanCommandQueue::Commit(VulkanCommandBuffer* commandBuffer)
    {
        // Recycle command buffer.
        RecycleCommandBuffer(commandBuffer);
    }

    void VulkanCommandQueue::Submit(const std::vector<VkSemaphore>& waitSemaphores)
    {
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };

        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence));

        vector<VkPipelineStageFlags> waitDstStageMasks(waitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pNext = nullptr;
        submitInfo.pWaitDstStageMask = waitDstStageMasks.data();
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = _queuedCommandBuffers.data();
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
        vkThrowIfFailed(vkQueueSubmit(_vkQueue, 1, &submitInfo, fence));

        vkThrowIfFailed(vkWaitForFences(_logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(_logicalDevice, fence, nullptr);
    }

    VulkanCommandBuffer* VulkanCommandQueue::GetCommandBuffer()
    {
        lock_guard<mutex> lock(_commandBufferMutex);

        if (_commandBufferId == 0)
            return nullptr;

        return _recycleCommandBuffers.at(--_commandBufferId);
    }

    void VulkanCommandQueue::RecycleCommandBuffer(VulkanCommandBuffer* commandBuffer)
    {
        lock_guard<mutex> lock(_commandBufferMutex);

        if (_commandBufferId < 16)
        {
            _recycleCommandBuffers.at(_commandBufferId++) = commandBuffer;
        }
    }
}
