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

#include "../Math/Math.h"
#include "../Graphics/Types.h"
#include <vector>

namespace Alimer
{
    class RenderPass;

    struct Command
    {
        enum class Type
        {
            Invalid,
            BeginRenderPass,
            EndRenderPass,
            SetViewport,
        };

        Command(Type type_) : type(type_)
        {
        }

        const Type type;
    };

    struct BeginRenderPassCommand : public Command
    {
        BeginRenderPassCommand(
            RenderPass* renderPass_,
            const Color4* clearColors_, uint32_t numClearColors_,
            float clearDepth_, uint8_t clearStencil_)
            : Command(Command::Type::BeginRenderPass)
            , renderPass(renderPass_)
            , clearColors(clearColors_, clearColors_ + numClearColors_)
            , clearDepth(clearDepth_)
            , clearStencil(clearStencil_)
        {
        }

        RenderPass* renderPass;
        std::vector<Color4> clearColors;
        float clearDepth;
        uint8_t clearStencil;
    };

    struct EndRenderPassCommand : public Command
    {
        EndRenderPassCommand()
            : Command(Command::Type::EndRenderPass)
        {
        }
    };

    struct SetViewportCommand : public Command
    {
        SetViewportCommand(const Viewport& viewport_)
            : Command(Command::Type::SetViewport)
            , viewport(viewport_)
        {
        }

        const Viewport viewport;
    };
}
