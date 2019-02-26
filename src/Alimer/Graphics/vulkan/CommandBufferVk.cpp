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

#include "../CommandBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "FramebufferVk.h"
#include "GraphicsDeviceVk.h"
#include "../../Math/Math.h"
#include "../../Core/Log.h"

namespace alimer
{
    void CommandBuffer::Create()
    {
        VkDevice device = _commandQueue->GetGraphicsDevice()->GetImpl()->GetVkDevice();
        VkCommandBufferAllocateInfo cmdBufAllocateInfo;
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.pNext = nullptr;
        cmdBufAllocateInfo.commandPool = _commandQueue->GetVkCommandPool();
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;
        vkThrowIfFailed(vkAllocateCommandBuffers(
            device,
            &cmdBufAllocateInfo,
            &_handle));

        _fence = _commandQueue->GetGraphicsDevice()->GetImpl()->RequestFence();
        _supportsDebugUtils = _commandQueue->GetGraphicsDevice()->GetImpl()->GetFeaturesVk().supportsDebugUtils;
        _supportsDebugMarker = _commandQueue->GetGraphicsDevice()->GetImpl()->GetFeaturesVk().supportsDebugMarker;
    }

    void CommandBuffer::Destroy()
    {
        if (_handle != VK_NULL_HANDLE) {
            _commandQueue->GetGraphicsDevice()->GetImpl()->RecycleFence(_fence);

            VkDevice device = _commandQueue->GetGraphicsDevice()->GetImpl()->GetVkDevice();
            vkFreeCommandBuffers(device, _commandQueue->GetVkCommandPool(), 1, &_handle);
            _handle = VK_NULL_HANDLE;
        }
    }

    void CommandBuffer::Reset()
    {
        _status = CommandBufferStatus::Initial;
        vkResetCommandBuffer(_handle, 0);
    }

    void CommandBuffer::Begin()
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        vkThrowIfFailed(
            vkBeginCommandBuffer(_handle, &beginInfo)
        );
    }

    void CommandBuffer::End()
    {
        vkThrowIfFailed(vkEndCommandBuffer(_handle));
    }

    void CommandBuffer::PushDebugGroupImpl(const std::string& name, const Color4& color)
    {
        if (_supportsDebugUtils)
        {
            VkDebugUtilsLabelEXT markerInfo;
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            markerInfo.pNext = nullptr;
            markerInfo.pLabelName = name.c_str();
            memcpy(markerInfo.color, color.Data(), sizeof(float) * 4);
            if (vkCmdBeginDebugUtilsLabelEXT) {
                vkCmdBeginDebugUtilsLabelEXT(_handle, &markerInfo);
            }
        }
        else if (_supportsDebugMarker)
        {
            VkDebugMarkerMarkerInfoEXT markerInfo;
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            markerInfo.pNext = nullptr;
            markerInfo.pMarkerName = name.c_str();
            memcpy(markerInfo.color, color.Data(), sizeof(float) * 4);
            vkCmdDebugMarkerBeginEXT(_handle, &markerInfo);
        }
    }

    void CommandBuffer::PopDebugGroupImpl()
    {
        if (_supportsDebugUtils)
        {
            if (vkCmdEndDebugUtilsLabelEXT != nullptr)
            {
                vkCmdEndDebugUtilsLabelEXT(_handle);
            }
        }
        else if (_supportsDebugMarker)
        {
            vkCmdDebugMarkerEndEXT(_handle);
        }
    }

    void CommandBuffer::InsertDebugMarkerImpl(const std::string& name, const Color4& color)
    {
        if (_supportsDebugUtils)
        {
            if (vkCmdInsertDebugUtilsLabelEXT != nullptr)
            {
                VkDebugUtilsLabelEXT markerInfo;
                markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                markerInfo.pNext = nullptr;
                markerInfo.pLabelName = name.c_str();
                memcpy(markerInfo.color, color.Data(), sizeof(float) * 4);
                vkCmdInsertDebugUtilsLabelEXT(_handle, &markerInfo);
            }
        }
        else if (_supportsDebugMarker)
        {
            VkDebugMarkerMarkerInfoEXT markerInfo;
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            markerInfo.pNext = nullptr;
            markerInfo.pMarkerName = name.c_str();
            memcpy(markerInfo.color, color.Data(), sizeof(float) * 4);
            vkCmdDebugMarkerInsertEXT(_handle, &markerInfo);
        }
    }

    void CommandBuffer::BeginRenderPassImpl(const RenderPassDescriptor* descriptor)
    {

    }

    void CommandBuffer::EndRenderPassImpl()
    {
        vkCmdEndRenderPass(_handle);
    }
}

