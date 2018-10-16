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
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexFormat.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
    class GraphicsDevice;

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer : public RefCounted
    {
        friend class GraphicsDevice;

    public:
        CommandBuffer(GraphicsDevice* device, bool secondary);

    public:
        /// Destructor.
        virtual ~CommandBuffer() = default;

        void BeginRenderPass(const RenderPassDescriptor* descriptor);
        void EndRenderPass();

        // Compute
        void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

    protected:
        void BeginCompute();
        void BeginGraphics();
        virtual void BeginContext();
        void FlushComputeState();

        // Backend methods.
        virtual void BeginRenderPassImpl(const RenderPassDescriptor* descriptor) = 0;
        virtual void EndRenderPassImpl() = 0;
        virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    protected:
        /// Graphics subsystem.
        GraphicsDevice* _device;
        bool _isCompute;

    private:
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
