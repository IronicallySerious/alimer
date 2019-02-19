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
#include "CommandContextVk.h"
#include "GraphicsDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    CommandQueueVk::CommandQueueVk(GraphicsDevice* device, VkQueue queue, uint32_t queueFamilyIndex)
        : _device(device)
        , _queue(queue)
    {
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        //createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        if (vkCreateCommandPool(device->GetImpl()->GetVkDevice(), &createInfo, nullptr, &_commandPool) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create command pool");
        }
    }

    CommandQueueVk::~CommandQueueVk()
    {
        if (_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_device->GetImpl()->GetVkDevice(), _commandPool, nullptr);
            _commandPool = VK_NULL_HANDLE;
        }
    }
}
