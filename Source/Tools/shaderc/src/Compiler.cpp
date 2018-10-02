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

#include "Compiler.h"
#include <fstream>
#include <streambuf>
#include <iostream>

#include "glslang/Include/ShHandle.h"
#include "glslang/Include/revision.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
using namespace std;

namespace Alimer
{
    static inline EShLanguage MapShaderStage(CompilerShaderStage stage)
    {
        switch (stage)
        {
        case CompilerShaderStage::Vertex: return EShLangVertex;
        case CompilerShaderStage::TessControl: return EShLangTessControl;
        case CompilerShaderStage::TessEvaluation: return EShLangTessEvaluation;
        case CompilerShaderStage::Geometry: return EShLangGeometry;
        case CompilerShaderStage::Fragment: return EShLangFragment;
        case CompilerShaderStage::Compute: return EShLangCompute;
        default: return EShLangVertex;
        }
    }

    enum TOptions {
        EOptionNone = 0,
        EOptionIntermediate = (1 << 0),
        EOptionSuppressInfolog = (1 << 1),
        EOptionMemoryLeakMode = (1 << 2),
        EOptionRelaxedErrors = (1 << 3),
        EOptionGiveWarnings = (1 << 4),
        EOptionLinkProgram = (1 << 5),
        EOptionMultiThreaded = (1 << 6),
        EOptionDumpConfig = (1 << 7),
        EOptionDumpReflection = (1 << 8),
        EOptionSuppressWarnings = (1 << 9),
        EOptionDumpVersions = (1 << 10),
        EOptionSpv = (1 << 11),
        EOptionHumanReadableSpv = (1 << 12),
        EOptionVulkanRules = (1 << 13),
        EOptionDefaultDesktop = (1 << 14),
        EOptionOutputPreprocessed = (1 << 15),
        EOptionOutputHexadecimal = (1 << 16),
        EOptionReadHlsl = (1 << 17),
        EOptionCascadingErrors = (1 << 18),
        EOptionAutoMapBindings = (1 << 19),
        EOptionFlattenUniformArrays = (1 << 20),
        EOptionNoStorageFormat = (1 << 21),
        EOptionKeepUncalled = (1 << 22),
        EOptionHlslOffsets = (1 << 23),
        EOptionHlslIoMapping = (1 << 24),
        EOptionAutoMapLocations = (1 << 25),
        EOptionDebug = (1 << 26),
        EOptionStdin = (1 << 27),
        EOptionOptimizeDisable = (1 << 28),
        EOptionOptimizeSize = (1 << 29),
    };

    static inline void SetMessageOptions(int options, EShMessages& messages)
    {
        if (options & EOptionRelaxedErrors)
            messages = (EShMessages)(messages | EShMsgRelaxedErrors);
        if (options & EOptionIntermediate)
            messages = (EShMessages)(messages | EShMsgAST);
        if (options & EOptionSuppressWarnings)
            messages = (EShMessages)(messages | EShMsgSuppressWarnings);
        if (options & EOptionSpv)
            messages = (EShMessages)(messages | EShMsgSpvRules);
        if (options & EOptionVulkanRules)
            messages = (EShMessages)(messages | EShMsgVulkanRules);
        if (options & EOptionOutputPreprocessed)
            messages = (EShMessages)(messages | EShMsgOnlyPreprocessor);
        if (options & EOptionReadHlsl)
            messages = (EShMessages)(messages | EShMsgReadHlsl);
        if (options & EOptionCascadingErrors)
            messages = (EShMessages)(messages | EShMsgCascadingErrors);
        if (options & EOptionKeepUncalled)
            messages = (EShMessages)(messages | EShMsgKeepUncalled);
    }

    enum vertex_attribs
    {
        VERTEX_POSITION = 0,
        VERTEX_NORMAL,
        VERTEX_TEXCOORD0,
        VERTEX_TEXCOORD1,
        VERTEX_TEXCOORD2,
        VERTEX_TEXCOORD3,
        VERTEX_TEXCOORD4,
        VERTEX_TEXCOORD5,
        VERTEX_TEXCOORD6,
        VERTEX_TEXCOORD7,
        VERTEX_COLOR0,
        VERTEX_COLOR1,
        VERTEX_COLOR2,
        VERTEX_COLOR3,
        VERTEX_TANGENT,
        VERTEX_BITANGENT,
        VERTEX_INDICES,
        VERTEX_WEIGHTS,
        VERTEX_ATTRIB_COUNT
    };

    static const char* k_attrib_names[VERTEX_ATTRIB_COUNT] = {
        "POSITION",
        "NORMAL",
        "TEXCOORD0",
        "TEXCOORD1",
        "TEXCOORD2",
        "TEXCOORD3",
        "TEXCOORD4",
        "TEXCOORD5",
        "TEXCOORD6",
        "TEXCOORD7",
        "COLOR0",
        "COLOR1",
        "COLOR2",
        "COLOR3",
        "TANGENT",
        "BINORMAL",
        "BLENDINDICES",
        "BLENDWEIGHT"
    };

    static int k_attrib_sem_indices[VERTEX_ATTRIB_COUNT] = {
        0,
        0,
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        0,
        1,
        2,
        3,
        0,
        0,
        0,
        0
    };

    Compiler::Compiler()
    {
        glslang::InitializeProcess();
    }

    Compiler::~Compiler()
    {
        glslang::FinalizeProcess();
    }

    bool Compiler::Compile(const CompilerOptions& options)
    {
        // construct semantics mapping defines
        // to be used in layout(location = SEMANTIC) inside GLSL
        std::string semantics_def;
        for (int i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            char sem_line[128];
            snprintf(sem_line, sizeof(sem_line), "#define %s %d\n", k_attrib_names[i], i);
            semantics_def += std::string(sem_line);
        }

        // Add SV_Target semantics for more HLSL compatibility
        for (int i = 0; i < 8; i++)
        {
            char sv_target_line[128];
            snprintf(sv_target_line, sizeof(sv_target_line), "#define SV_Target%d %d\n", i, i);
            semantics_def += std::string(sv_target_line);
        }

        // Set message options.
        int cOptions = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
        EShMessages messages = EShMsgDefault;
        SetMessageOptions(cOptions, messages);
        int defaultVersion = 100;

        glslang::TProgram program;
        std::string def("#extension GL_GOOGLE_include_directive : require\n");
        def += semantics_def;

        std::vector<std::string> processes;
        for (auto define : options.defines)
        {
            def += "#define " + define.first;
            if (!define.second.empty()) {
                def += std::string(" ") + define.second;
            }
            else
            {
                def += std::string(" 1");
            }
            def += std::string("\n");

            char process[256];
            snprintf(process, sizeof(process), "D%s", define.first.c_str());
            processes.push_back(process);
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(CompilerShaderStage::Count); i++)
        {
            CompilerStageOptions stageOptions = options.shaders[i];
            if (stageOptions.file.empty())
                continue;

            std::ifstream stream(stageOptions.file);
            std::string shaderSource((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            
            CompilerShaderStage stage = static_cast<CompilerShaderStage>(i);
            EShLanguage language = MapShaderStage(stage);
            glslang::TShader shader(language);
            const char* shaderStrings = shaderSource.c_str();
            const int shaderLengths = static_cast<int>(shaderSource.length());
            const char* stringNames = stageOptions.file.c_str();
            shader.setStringsWithLengthsAndNames( &shaderStrings, &shaderLengths, &stringNames, 1);
            shader.setInvertY(options.invertY ? true : false);
            shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientOpenGL, defaultVersion);
            shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
            shader.setEntryPoint(stageOptions.entryPoint.c_str());
            shader.setShiftSamplerBinding(0);
            shader.setShiftTextureBinding(0);
            shader.setShiftImageBinding(0);
            shader.setShiftUboBinding(0);
            shader.setShiftSsboBinding(0);
            shader.setFlattenUniformArrays(false);
            shader.setNoStorageFormat(false);
            shader.setPreamble(def.c_str());
            shader.addProcesses(processes);
        }

        return true;
    }
}
