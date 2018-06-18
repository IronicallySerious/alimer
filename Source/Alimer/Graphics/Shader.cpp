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

#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include <spirv-cross/spirv_cross.hpp>
#include <vector>
using namespace std;

namespace Alimer
{
    Shader::Shader()
    {
    }

    Shader::Shader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
    {
        Reflect(ShaderStage::Vertex, vertex.code.data(), vertex.code.size() * sizeof(uint32_t));
        Reflect(ShaderStage::Fragment, fragment.code.data(), fragment.code.size() * sizeof(uint32_t));
    }

    Shader::~Shader()
    {
    }

    void Shader::Reflect(ShaderStage stage, const uint32_t *data, size_t size)
    {
        vector<uint32_t> code(data, data + size / sizeof(uint32_t));
        spirv_cross::Compiler compiler(move(code));
        auto resources = compiler.get_shader_resources();

        if (stage == ShaderStage::Vertex)
        {
            for (auto &attrib : resources.stage_inputs)
            {
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);

                if (location >= MaxVertexAttributes)
                {
                    ALIMER_LOGERROR("SPIRV attribute location higher than allowed one");
                    return;
                }

                _layout.attributeMask |= 1u << location;
            }
        }
        else if (stage == ShaderStage::Fragment)
        {
            for (auto &attrib : resources.stage_outputs)
            {
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);
                _layout.renderTargetMask |= 1u << location;
            }
        }
    }
}
