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

#include "../Graphics/Shader.h"
#include "../Graphics/GraphicsDevice.h"
#include "../IO/Path.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Log.h"
#include <algorithm>
#include <unordered_map>
//#include <spirv-cross/spirv_glsl.hpp>

namespace alimer
{
    /*class CustomCompiler : public spirv_cross::CompilerGLSL
    {
    public:
        CustomCompiler(const std::vector<uint8_t>& bytecode)
            : spirv_cross::CompilerGLSL(reinterpret_cast<const uint32_t*>(bytecode.Data()), bytecode.Size() / 4)
        {

        }

        /*VkAccessFlags GetAccessFlags(const spirv_cross::SPIRType& type)
        {
            // SPIRV-Cross hack to get the correct readonly and writeonly attributes on ssbos.
            // This isn't working correctly via Compiler::get_decoration(id, spv::DecorationNonReadable) for example.
            // So the code below is extracted from private methods within spirv_cross.cpp.
            // The spirv_cross executable correctly outputs the attributes when converting spirv back to GLSL,
            // but it's own reflection code does not :-(
            auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
            for (auto i = 0U; i < type.member_types.size(); ++i)
                all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

            auto base_flags = meta[type.self].decoration.decoration_flags;
            base_flags.merge_or(spirv_cross::Bitset(all_members_flag_mask));

            VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            if (base_flags.get(spv::DecorationNonReadable))
                access = VK_ACCESS_SHADER_WRITE_BIT;
            else if (base_flags.get(spv::DecorationNonWritable))
                access = VK_ACCESS_SHADER_READ_BIT;

            return access;
        }
    };*/

    Shader::Shader()
        : GPUResource(Type::Shader)
    {
    }

    Shader::~Shader()
    {
        Destroy();
    }

    void Shader::Destroy()
    {
        //SafeDelete(_handle);
    }

    void Shader::RegisterObject()
    {
        RegisterFactory<Shader>();
    }

    void Shader::Define(ShaderStage stage, const std::vector<uint8_t>& byteCode)
    {
        _stage = stage;
        _byteCode = std::move(byteCode);
    }

    void Shader::Define(ShaderStage stage, const std::string& code, const std::string& entryPoint)
    {
        _stage = stage;
        _sourceCode = code;

        if (_device
            && _device->IsInitialized())
        {
            //_handle = _device->CreateShader(_stage, code, entryPoint);
        }
    }
    
    void Shader::Reflect(const std::vector<uint8_t>& bytecode)
    {
        // Parse SPIRV binary.
        /*CustomCompiler compiler(bytecode);
        spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
        opts.enable_420pack_extension = true;
        compiler.set_common_options(opts);

        // Reflect on all resource bindings.
        auto resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model())
        {
        case spv::ExecutionModelVertex:
            _stage = ShaderStages::Vertex;
            break;
        case spv::ExecutionModelTessellationControl:
            _stage = ShaderStages::TessellationControl;
            break;
        case spv::ExecutionModelTessellationEvaluation:
            _stage = ShaderStages::TessellationEvaluation;
            break;
        case spv::ExecutionModelGeometry:
            _stage = ShaderStages::Geometry;
            break;
        case spv::ExecutionModelFragment:
            _stage = ShaderStages::Fragment;
            break;
        case spv::ExecutionModelGLCompute:
            _stage = ShaderStages::Compute;
            break;
        default:
            ALIMER_LOGCRITICAL("Invalid shader execution model");
        }

        // Extract per stage inputs.
        for (auto& resource : resources.stage_inputs)
        {
        }*/
    }
}
