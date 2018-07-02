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
#include "../Graphics/PipelineState.h"
#include "../Math/Color.h"

namespace Alimer
{
    class CommandBuffer;

	/// Defines a base command encoder.
	class ALIMER_API CommandEncoder 
	{
    public:
        /// Disallow direct command encoder delete.
        void operator delete(void *p) { Fail(); }
        void operator delete[](void *p) { Fail(); }

    protected:
        static void Fail() {
#if defined(_HAS_EXCEPTIONS) || defined(__EXCEPTIONS)
            throw std::bad_alloc();
#else
            std::abort();
#endif
        }

	public:
        virtual ~CommandEncoder() = default;

        // The command buffer the encoder is encoding into.
        CommandBuffer* GetCommandBuffer() const { return _commandBuffer; }

        /// Close command encoding
        virtual void Close() = 0;

    protected:
        explicit CommandEncoder(CommandBuffer* commandBuffer)
            : _commandBuffer(commandBuffer) {}

        CommandBuffer* _commandBuffer = nullptr;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(CommandEncoder);
	};

    /// An encoder that specifies graphics-rendering commands and executes graphics functions.
    class ALIMER_API RenderPassCommandEncoder : public CommandEncoder
    {
    protected:
        explicit RenderPassCommandEncoder(CommandBuffer* commandBuffer)
            : CommandEncoder(commandBuffer) {}
    };
}
