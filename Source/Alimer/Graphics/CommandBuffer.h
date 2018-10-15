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

#pragma once

#include "../Graphics/Types.h"
#include "../Graphics/BackendTypes.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexFormat.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
#if ALIMER_VULKAN
    class VulkanFramebuffer;
    class VulkanRenderPass;
#endif

    class Graphics;

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer final : public RefCounted
    {
        friend class Graphics;

    public:
        enum class Type
        {
            Generic,
            AsyncGraphics,
            AsyncCompute,
            AsyncTransfer,
            Count
        };

    public:
        CommandBuffer(Type type, bool secondary);

        /// Destructor.
        ~CommandBuffer();

        /// Destroy the command buffer.
        void Destroy();

        void BeginRenderPass(const RenderPassDescriptor* descriptor);
        void EndRenderPass();

        // Compute
        void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

#if ALIMER_VULKAN
        VkCommandBuffer GetVkCommandBuffer() const { return _vkCommandBuffer; }
        VkFence GetVkFence() const { return _vkFence; }
#endif

        inline Type GetCommandBufferType() const { return _type; }

    private:
        void BeginCompute();
        void BeginGraphics();
        virtual void BeginContext();
        void FlushComputeState();

        // Backend methods.
        void BeginRenderPassImpl(const RenderPassDescriptor* descriptor);
        void EndRenderPassImpl();
        void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    protected:
        /// Graphics subsystem.
        WeakPtr<Graphics> _graphics;
        Type _type;
        bool _isCompute;

#if ALIMER_VULKAN
        VkCommandPool _vkCommandPool = 0;
        VkCommandBuffer _vkCommandBuffer = nullptr;
        VkFence _vkFence = 0;
        const VulkanFramebuffer* _framebuffer = nullptr;
        const VulkanRenderPass* _renderPass = nullptr;
#endif

    private:
        bool Create(bool secondary);

        inline bool IsInsideRenderPass() const
        {
            return _state == State::InRenderPass;
        }

        enum class State
        {
            None,
            Recording,
            InRenderPass,
            Committed
        };

        State _state = State::None;
    };
}
