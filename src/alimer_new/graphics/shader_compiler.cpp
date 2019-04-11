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

#include "alimer_config.h"
#if defined(ALIMER_VULKAN)
#include "graphics/shader_compiler.h"
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/OSDependent/osinclude.h>
#include <StandAlone/ResourceLimits.h>

namespace alimer
{
    namespace
    {
        inline EShLanguage GetEShLanguage(ShaderStage stage)
        {
            switch (stage)
            {
            case ShaderStage::Vertex:
                return EShLangVertex;

            case ShaderStage::TessellationControl:
                return EShLangTessControl;

            case ShaderStage::TessellationEvaluation:
                return EShLangTessEvaluation;

            case ShaderStage::Geometry:
                return EShLangGeometry;

            case ShaderStage::Fragment:
                return EShLangFragment;

            case ShaderStage::Compute:
                return EShLangCompute;

            default:
                return EShLangVertex;
            }
        }
    }        // namespace

    bool compile_to_spirv(ShaderStage stage, const std::string& shaderSource, const std::string& entryPoint,
        std::vector<uint32_t> &spirv, std::string& infoLog)
    {
        // Initialize glslang library.
        glslang::InitializeProcess();

        EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

        EShLanguage language = GetEShLanguage(stage);

        const char *file_name_list[1] = { "" };
        const char *shader_source = reinterpret_cast<const char *>(shaderSource.data());

        glslang::TShader shader(language);
        shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
        shader.setEntryPoint(entryPoint.c_str());
        shader.setSourceEntryPoint(entryPoint.c_str());

        if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
        {
            infoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
            return false;
        }

        // Add shader to new program object.
        glslang::TProgram program;
        program.addShader(&shader);

        // Link program.
        if (!program.link(messages))
        {
            infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
            return false;
        }

        // Save any info log that was generated.
        if (shader.getInfoLog())
        {
            infoLog += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
        }

        if (program.getInfoLog())
        {
            infoLog += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        }

        glslang::TIntermediate *intermediate = program.getIntermediate(language);

        // Translate to SPIRV.
        if (!intermediate)
        {
            infoLog += "Failed to get shared intermediate code.\n";
            return false;
        }

        spv::SpvBuildLogger logger;

        glslang::GlslangToSpv(*intermediate, spirv, &logger);

        infoLog += logger.getAllMessages() + "\n";

        // Shutdown glslang library.
        glslang::FinalizeProcess();

        return true;
    }
}
#endif 
