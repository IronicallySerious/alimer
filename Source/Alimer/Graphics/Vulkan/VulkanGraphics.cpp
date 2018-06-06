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

#include "../../Core/Log.h"
#include "../../Core/Window.h"
#include "VulkanGraphics.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTexture.h"
#include "VulkanConvert.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#	include "../../Core/Windows/WindowWindows.h"
#endif
using namespace std;

namespace Alimer
{
    VulkanGraphics::VulkanGraphics(bool validation, const string& applicationName)
        : Graphics(validation, applicationName)
    {
        
    }

    VulkanGraphics::~VulkanGraphics()
    {
        Finalize();
    }

    void VulkanGraphics::Finalize()
    {
        WaitIdle();

        // Release all GPU objects
        Graphics::Finalize();

        // Destroy main swap chain.
        _swapchain.Reset();
        _commandBuffers.clear();

        for (auto& it : _renderPassCache)
        {
            vkDestroyRenderPass(_logicalDevice, it.second, nullptr);
        }
        _renderPassCache.clear();

        for (auto& it : _framebufferCache)
        {
            vkDestroyFramebuffer(_logicalDevice, it.second->framebuffer, nullptr);
            SafeDelete(it.second);
        }
        _renderPassCache.clear();

        //vkDestroyPipelineCache(_logicalDevice, pipelineCache, nullptr);
        vkDestroyCommandPool(_logicalDevice, _graphicsCommandPool, nullptr);
        vkDestroySemaphore(_logicalDevice, _imageAcquiredSemaphore, nullptr);
        vkDestroySemaphore(_logicalDevice, _renderCompleteSemaphore, nullptr);

        for (auto& fence : _waitFences) {
            vkDestroyFence(_logicalDevice, fence, nullptr);
        }
        _waitFences.clear();

        // Destroy logical device.
        if (_logicalDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(_logicalDevice, nullptr);
            _logicalDevice = VK_NULL_HANDLE;
        }

        if (_debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_instance, _debugCallback, nullptr);
            _debugCallback = VK_NULL_HANDLE;
        }

        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }

    bool VulkanGraphics::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_logicalDevice);
        if (result < VK_SUCCESS)
            return false;

        return true;
    }

    bool VulkanGraphics::Initialize(const SharedPtr<Window>& window)
    {
        VkResult result = VK_SUCCESS;

        // Enumerate physical devices.
        uint32_t gpuCount = 0;
        // Get number of available physical devices
        vkThrowIfFailed(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
        if (gpuCount > 0)
        {
            // Enumerate physical devices.
            std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
            result = vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());

            //for (uint32_t i = 0; i < gpuCount; ++i)
            //{
            //	_physicalDevice.Push(new VulkanPhysicalDevice(physicalDevices[i]));
            //}

            _vkPhysicalDevice = physicalDevices.front();
        }

        // Store Properties features, limits and properties of the physical device for later use
        // Device properties also contain limits and sparse properties
        vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &_deviceProperties);
        // Features should be checked by the examples before using them
        vkGetPhysicalDeviceFeatures(_vkPhysicalDevice, &_deviceFeatures);
        // Memory properties are used regularly for creating all kinds of buffers
        vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_deviceMemoryProperties);
        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, nullptr);
        assert(queueFamilyCount > 0);
        _queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

        // Now create logical device.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        const float defaultQueuePriority(0.0f);

        // Graphics queue.
        const VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
            _queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
            VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueInfo.queueFamilyIndex = _queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
        else {
            _queueFamilyIndices.graphics = static_cast<uint32_t>(-1);
        }

        // Dedicated compute queue
        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
            _queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
            if (_queueFamilyIndices.compute != _queueFamilyIndices.graphics) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                queueInfo.queueFamilyIndex = _queueFamilyIndices.compute;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {
            // Else we use the same queue
            _queueFamilyIndices.compute = _queueFamilyIndices.graphics;
        }

        // Create the logical device representation
        std::vector<const char*> deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDeviceFeatures enabledFeatures{};
        if (_deviceFeatures.samplerAnisotropy) {
            enabledFeatures.samplerAnisotropy = VK_TRUE;
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        if (deviceExtensions.size() > 0) {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        result = vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice);
        vkThrowIfFailed(result);

        // Get queue's.
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.graphics, 0, &_graphicsQueue);
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.compute, 0, &_computeQueue);

#if TODO
        // Create default command pool.
        VkCommandPoolCreateInfo cmdPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            _queueFamilyIndices.graphics
        };
        vkThrowIfFailed(vkCreateCommandPool(_logicalDevice, &cmdPoolInfo, nullptr, &_graphicsCommandPool));

        // Create the main swap chain.
        _swapchain.Reset(new VulkanSwapchain(this, window));

        // Allocate vulkan command buffers.
        const uint32_t imageCount = _swapchain->GetImageCount();
        std::vector<VkCommandBuffer> vkCommandBuffers(imageCount);
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocateInfo.pNext = nullptr;
        cmdBufAllocateInfo.commandPool = _graphicsCommandPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = imageCount;
        vkThrowIfFailed(vkAllocateCommandBuffers(_logicalDevice, &cmdBufAllocateInfo, vkCommandBuffers.data()));
        _commandBuffers.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            _commandBuffers[i] = MakeShared<VulkanCommandBuffer>(this, _graphicsCommandPool, vkCommandBuffers[i]);
        }

        // Create sync primitives.
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_imageAcquiredSemaphore));
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphore));

        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        // Create in signaled state so we don't wait on first render of each command buffer
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        _waitFences.resize(_swapchain->GetImageCount());
        for (auto& fence : _waitFences)
        {
            vkThrowIfFailed(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence));
        }
#endif // TODO


        return Graphics::Initialize(window);
    }

    SharedPtr<Texture> VulkanGraphics::AcquireNextImage()
    {
        // Acquire the next image from the swap chain
        SharedPtr<Texture> texture = _swapchain->AcquireNextImage(_imageAcquiredSemaphore, &_swapchainImageIndex);

        // Wait for frame fence.
        vkThrowIfFailed(vkWaitForFences(_logicalDevice, 1, &_waitFences[_swapchainImageIndex], VK_TRUE, UINT64_MAX));
        vkThrowIfFailed(vkResetFences(_logicalDevice, 1, &_waitFences[_swapchainImageIndex]));

        //DestroyPendingResources();

        return texture;
    }

    bool VulkanGraphics::Present()
    {
        // Submit frame command buffer.
        _commandBuffers[_swapchainImageIndex]->End();
        SubmitCommandBuffer(_commandBuffers[_swapchainImageIndex]);

        VkResult result = _swapchain->QueuePresent(
            _graphicsQueue,
            _swapchainImageIndex,
            _renderCompleteSemaphore);
        if (result < VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

    SharedPtr<CommandBuffer> VulkanGraphics::GetCommandBuffer()
    {
        // Init current command buffer.
        _commandBuffers[_swapchainImageIndex]->Begin();

        return _commandBuffers[_swapchainImageIndex];
    }

    GpuBufferPtr VulkanGraphics::CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
    {
        return nullptr;
    }

    PipelineLayoutPtr VulkanGraphics::CreatePipelineLayout()
    {
        return nullptr;
    }

    SharedPtr<Shader> VulkanGraphics::CreateComputeShader(const ShaderStageDescription& desc)
    {
        return nullptr;
    }

    SharedPtr<Shader> VulkanGraphics::CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
    {
        return nullptr;
    }

    PipelineStatePtr VulkanGraphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
    {
        return nullptr;
    }

    bool VulkanGraphics::PrepareDraw(PrimitiveTopology topology)
    {
        return true;
    }

    VkCommandBuffer VulkanGraphics::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _graphicsCommandPool;
        info.level = level;
        info.commandBufferCount = 1;

        VkCommandBuffer vkCommandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(_logicalDevice, &info, &vkCommandBuffer));

        // If requested, also start recording for the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkThrowIfFailed(vkBeginCommandBuffer(vkCommandBuffer, &cmdBufferBeginInfo));
        }

        return vkCommandBuffer;
    }

    void VulkanGraphics::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void VulkanGraphics::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE)
            return;

        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };

        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing.
        vkThrowIfFailed(vkWaitForFences(_logicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkDestroyFence(_logicalDevice, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(_logicalDevice, _graphicsCommandPool, 1, &commandBuffer);
        }
    }

    void VulkanGraphics::ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue)
    {
        // Transition to destination layout.
        vk::SetImageLayout(commandBuffer, image, aspect, sourceLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Clear the image
        range.aspectMask = aspect;
        vkCmdClearColorImage(
            commandBuffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            clearValue,
            1,
            &range);

        // Transition back to source layout.
        vk::SetImageLayout(commandBuffer, image, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, destLayout);
    }

    void VulkanGraphics::SubmitCommandBuffer(VulkanCommandBuffer* commandBuffer)
    {
        auto vkCommandBuffer = commandBuffer->GetVkCommandBuffer();
        const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_imageAcquiredSemaphore;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &_renderCompleteSemaphore;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _waitFences[_swapchainImageIndex]));
    }

    VkRenderPass VulkanGraphics::GetVkRenderPass(const RenderPassDescriptor& descriptor, uint64_t hash)
    {
        auto it = _renderPassCache.find(hash);
        if (it != end(_renderPassCache))
            return it->second;

        uint32_t attachmentCount = 0;
        std::array<VkAttachmentDescription, MaxColorAttachments + 1> attachments = {};
        std::vector<VkAttachmentReference> colorReferences;
        VkAttachmentReference depthReference = {};
        bool hasDepth = false;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassColorAttachmentDescriptor& attachment = descriptor.colorAttachments[i];
            Texture* texture = attachment.texture;
            if (!texture)
                continue;

            attachments[attachmentCount].format = vk::Convert(texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(attachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(attachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorReferences.push_back({ attachmentCount, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

            attachmentCount++;
        }

        if (descriptor.depthAttachment.texture
            || descriptor.stencilAttachment.texture)
        {
            attachments[attachmentCount].format = vk::Convert(descriptor.depthAttachment.texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(descriptor.depthAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(descriptor.depthAttachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = vk::Convert(descriptor.stencilAttachment.loadAction);
            attachments[attachmentCount].stencilStoreOp = vk::Convert(descriptor.stencilAttachment.storeAction);
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthReference.attachment = attachmentCount;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachmentCount++;

            hasDepth = true;
        }

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescription.pColorAttachments = colorReferences.data();
        if (hasDepth)
        {
            subpassDescription.pDepthStencilAttachment = &depthReference;
        }
        else
        {
            subpassDescription.pDepthStencilAttachment = nullptr;
        }

        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDescription;
        createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        createInfo.pDependencies = dependencies.data();

        VkRenderPass renderPass;
        VkResult result = vkCreateRenderPass(_logicalDevice, &createInfo, nullptr, &renderPass);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create render pass.");
            return VK_NULL_HANDLE;
        }

        _renderPassCache[hash] = renderPass;
        return renderPass;
    }

    VulkanFramebuffer* VulkanGraphics::GetFramebuffer(VkRenderPass renderPass, const RenderPassDescriptor& descriptor, uint64_t hash)
    {
        auto it = _framebufferCache.find(hash);
        if (it != end(_framebufferCache))
            return it->second;

        VkImageView views[MaxColorAttachments + 1];
        uint32_t numViews = 0;
        uint32_t width = UINT32_MAX;
        uint32_t height = UINT32_MAX;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassColorAttachmentDescriptor& attachment = descriptor.colorAttachments[i];
            Texture* texture = attachment.texture;
            if (!texture)
                continue;

            width = std::min(width, texture->GetLevelWidth(attachment.level));
            height = std::min(height, texture->GetLevelHeight(attachment.level));
            views[numViews++] = static_cast<VulkanTexture*>(texture)->GetDefaultImageView();
        }

        if (descriptor.depthAttachment.texture
            || descriptor.stencilAttachment.texture)
        {
        }

        VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = numViews;
        createInfo.pAttachments = views;
        createInfo.width = width;
        createInfo.height = height;
        createInfo.layers = 1;

        VkFramebuffer framebuffer;
        VkResult result = vkCreateFramebuffer(_logicalDevice, &createInfo, nullptr, &framebuffer);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create framebuffer.");
            return nullptr;
        }

        VulkanFramebuffer* fbo = new VulkanFramebuffer();
        fbo->renderPass = renderPass;
        fbo->framebuffer = framebuffer;
        fbo->size = { width, height };
        _framebufferCache[hash] = fbo;
        return fbo;
    }
}
