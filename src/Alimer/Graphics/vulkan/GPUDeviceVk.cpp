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

#define VMA_STATS_STRING_ENABLED 0
#define VMA_IMPLEMENTATION
#include "GPUDeviceVk.h"
#include "SwapChainVk.h"
#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "TextureVk.h"
#include "SamplerVk.h"
#include "BufferVk.h"

namespace alimer
{
    bool GPUDeviceVk::IsSupported()
    {
        return true;
    }

    GPUDeviceVk::GPUDeviceVk(bool validation, bool headless)
    {
    }

    GPUDeviceVk::~GPUDeviceVk()
    {
        
    }

    /*bool GPUDeviceVk::SetMode(const SwapChainHandle* handle, const SwapChainDescriptor* descriptor)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!_headless)
        {
            surface = CreateSurface(handle);
        }

        
        // Create command queue's.
        _graphicsCommandQueue = std::make_unique<CommandQueueVk>(this, _graphicsQueue, _graphicsQueueFamily);
        _computeCommandQueue = std::make_unique<CommandQueueVk>(this, _computeQueue, _computeQueueFamily);
        _copyCommandQueue = std::make_unique<CommandQueueVk>(this, _transferQueue, _transferQueueFamily);

        // Create main swap chain.
        if (!_headless)
        {
            _mainSwapChain = new SwapChainVk(this, surface, descriptor);
        }

        _waitFences.resize(RenderLatency);
        _presentCompleteSemaphores.resize(RenderLatency);
        _renderCompleteSemaphores.resize(RenderLatency);
        _perFrame.clear();
        for (uint32_t i = 0; i < RenderLatency; ++i)
        {
            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_waitFences[i]));

            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentCompleteSemaphores[i]));
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphores[i]));

            auto frame = std::unique_ptr<PerFrame>(new PerFrame(this));
            _perFrame.emplace_back(std::move(frame));
        }

        // Frame command buffers.
        {
            std::vector<VkCommandBuffer> vkCommandBuffers(_mainSwapChain->GetTextureCount());
            VkCommandBufferAllocateInfo commandBufferAllocateInfo;
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.pNext = nullptr;
            commandBufferAllocateInfo.commandPool = _graphicsCommandQueue->GetVkCommandPool();
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = _mainSwapChain->GetTextureCount();
            vkThrowIfFailed(vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, vkCommandBuffers.data()));

            _commandBuffers.resize(_mainSwapChain->GetTextureCount());
            for (uint32_t i = 0; i < _mainSwapChain->GetTextureCount(); i++)
            {
                _commandBuffers[i] = std::make_unique<CommandBufferVk>(this, _graphicsCommandQueue.get(), vkCommandBuffers[i]);
            }
        }

        return true;
    }*/

    /*bool GPUDeviceVk::BeginFrame()
    {
        vkThrowIfFailed(vkWaitForFences(_device, 1, &_waitFences[_frameIndex], VK_TRUE, UINT64_MAX));
        vkThrowIfFailed(vkResetFences(_device, 1, &_waitFences[_frameIndex]));

        VkResult result = _mainSwapChain->AcquireNextImage(_presentCompleteSemaphores[_frameIndex], &_swapchainImageIndex);
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
        {
            _mainSwapChain->Resize();
        }
        else {
            vkThrowIfFailed(result);
        }

        _commandBuffers[_swapchainImageIndex]->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        return true;
    }

    void GPUDeviceVk::EndFrame()
    {
        _graphicsCommandQueue->ExecuteCommandBuffer(
            _commandBuffers[_swapchainImageIndex].get(),
            _presentCompleteSemaphores[_frameIndex],
            _renderCompleteSemaphores[_frameIndex],
            _waitFences[_frameIndex]
        );

        VkResult result = _mainSwapChain->QueuePresent(_graphicsQueue, _swapchainImageIndex, _renderCompleteSemaphores[_frameIndex]);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                _mainSwapChain->Resize();
                return;
            }
            else {
                vkThrowIfFailed(result);
            }
        }

        _frameIndex += 1;
        _frameIndex %= RenderLatency;
    }

    GPUTexture* GPUDeviceVk::CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData)
    {
        return new TextureVk(this, descriptor, nativeTexture, pInitData);
    }

    GPUSampler* GPUDeviceVk::CreateSampler(const SamplerDescriptor* descriptor)
    {
        return new SamplerVk(this, descriptor);
    }

    GPUBuffer* GPUDeviceVk::CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData)
    {
        return new BufferVk(this, descriptor, pInitData);
    }*/

    void GPUDeviceVk::DestroySampler(VkSampler sampler)
    {
#if !defined(NDEBUG)
        ALIMER_ASSERT(std::find(frame().destroyedSamplers.begin(), frame().destroyedSamplers.end(), sampler) == frame().destroyedSamplers.end());
#endif
        frame().destroyedSamplers.push_back(sampler);
    }

    GPUDeviceVk::PerFrame::PerFrame(GPUDeviceVk* device_)
        : device(device_)
        , logicalDevice(device_->GetVkDevice())
    {

    }

    GPUDeviceVk::PerFrame::~PerFrame()
    {
        Begin();
    }

    void GPUDeviceVk::PerFrame::Begin()
    {
        for (auto &sampler : destroyedSamplers)
        {
            vkDestroySampler(logicalDevice, sampler, nullptr);
        }
    }
}
