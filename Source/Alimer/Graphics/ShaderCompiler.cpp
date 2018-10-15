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

#include "../Graphics/ShaderCompiler.h"
#include "../Graphics/VertexFormat.h"
#include "../Resource/ResourceManager.h"
#include "../IO/Path.h"
#include "../Core/Log.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/StandAlone/ResourceLimits.h"
#include "SPIRV/GlslangToSpv.h"
#include <fstream>
#include <sstream>

namespace Alimer
{
    class AlimerIncluder : public glslang::TShader::Includer
    {
    public:
        AlimerIncluder(const String& from)
        {
            _rootDirectory = from;
        }

        IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
        {
            return includeLocal(headerName, includerName, inclusionDepth);
        }

        IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
        {
            ALIMER_UNUSED(includerName);
            ALIMER_UNUSED(inclusionDepth);

            String fullPath = Path::Join(_rootDirectory, headerName);
            std::stringstream content;
            std::string line;
            std::ifstream file(fullPath.CString());
            if (file.is_open())
            {
                while (getline(file, line))
                {
                    content << line << '\n';
                }
                file.close();
            }
            else
            {
                ALIMER_LOGCRITICALF("Cannot open include file '%s'", fullPath.CString());
                return nullptr;
            }

            String fileContent(content.str());
            char* heapContent = new char[fileContent.Length() + 1];
            strcpy(heapContent, fileContent.CString());
            return new IncludeResult(fullPath.CString(), heapContent, content.str().size(), heapContent);
        }

        void releaseInclude(IncludeResult* result) override {
            delete (char*)result->userData;
            delete result;
        }
    private:
        String _rootDirectory;
    };

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

    void SetMessageOptions(int options, EShMessages& messages)
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

    EShLanguage MapShaderStage(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:
            return EShLangVertex;

        case ShaderStage::TessControl:
            return EShLangTessControl;

        case ShaderStage::TessEvaluation:
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

    namespace ShaderCompiler
    {
        bool Compile(
            const String& filePath,
            const String& entryPoint,
            std::vector<uint32_t>& spirv,
            String& infoLog)
        {
            auto stream = FileSystem::Get().Open(filePath);
            if (!stream)
            {
                infoLog = String::Format("Shader file '%s' does not exists", filePath.CString());
                return false;
            }

            String shaderSource = stream->ReadAllText();
            uint32_t firstExtStart = filePath.FindLast(".");
            bool hasFirstExt = firstExtStart != String::NPOS;
            uint32_t secondExtStart = hasFirstExt ? filePath.FindLast(".", firstExtStart - 1) : String::NPOS;
            bool hasSecondExt = secondExtStart != String::NPOS;
            String firstExt = filePath.Substring(firstExtStart + 1);
            bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");
            if (usesUnifiedExt && firstExt == "hlsl")
            {
                // Add HLSL to glslang
            }

            String stageName;
            if (hasFirstExt && !usesUnifiedExt)
                stageName = firstExt;
            else if (usesUnifiedExt && hasSecondExt)
                stageName = filePath.Substring(secondExtStart + 1, firstExtStart - secondExtStart - 1);

            ShaderStage stage = ShaderStage::Vertex;
            if (stageName == "vert")
                stage = ShaderStage::Vertex;
            else if (stageName == "tesc")
                stage = ShaderStage::TessControl;
            else if (stageName == "tese")
                stage = ShaderStage::TessEvaluation;
            else if (stageName == "geom")
                stage = ShaderStage::Geometry;
            else if (stageName == "frag")
                stage = ShaderStage::Fragment;
            else if (stageName == "comp")
                stage = ShaderStage::Compute;

            return Compile(stage, shaderSource, entryPoint, spirv, stream->GetName(), infoLog);
        }

        bool Compile(
            ShaderStage stage,
            const String& source,
            const String& entryPoint,
            std::vector<uint32_t>& spirv,
            const String& filePath,
            String& infoLog)
        {
            // Get default built in resource limits.
            auto resourceLimits = glslang::DefaultTBuiltInResource;

            // construct semantics mapping defines
            // to be used in layout(location = SEMANTIC) inside GLSL
            String semanticsDef;
            for (uint32_t i = 0; i < static_cast<uint32_t>(VertexElementSemantic::Count); i++)
            {
                VertexElementSemantic semantic = static_cast<VertexElementSemantic>(i);
                semanticsDef += String::Format("#define %s %d\n", VertexElementSemanticToString(semantic), i);
            }

            // Add SV_Target semantics for more HLSL compatibility
            for (int i = 0; i < 8; i++)
            {
                semanticsDef += String::Format("#define SV_Target%d %d\n", i, i);
            }

            const String macroDefinitions = "";
            String poundExtension = "#extension GL_GOOGLE_include_directive : require\n";
            poundExtension += semanticsDef;
            const String preamble = macroDefinitions + poundExtension;

            EShLanguage language = MapShaderStage(stage);
            glslang::TShader shader(language);

            const char* shaderStrings = source.CString();
            const int shaderLengths = static_cast<int>(source.Length());
            const char* stringNames = filePath.CString();
            shader.setStringsWithLengthsAndNames(
                &shaderStrings,
                &shaderLengths,
                &stringNames, 1);
            shader.setPreamble(preamble.CString());
            shader.setEntryPoint(entryPoint.CString());
            shader.setSourceEntryPoint(entryPoint.CString());
            shader.setShiftSamplerBinding(0);
            shader.setShiftTextureBinding(0);
            shader.setShiftImageBinding(0);
            shader.setShiftUboBinding(0);
            shader.setShiftSsboBinding(0);
            shader.setFlattenUniformArrays(false);
            shader.setNoStorageFormat(false);

            // Set message options.
            int options = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
            EShMessages messages = EShMsgDefault;
            SetMessageOptions(options, messages);

            AlimerIncluder includer(GetPath(filePath));

            const EProfile DefaultProfile = ENoProfile;
            const bool ForceVersionProfile = false;
            const bool NotForwardCompatible = false;

            bool parseSuccess = shader.parse(
                &resourceLimits,
                100,
                DefaultProfile,
                ForceVersionProfile,
                NotForwardCompatible,
                messages,
                includer);

            if (!parseSuccess)
            {
                infoLog = shader.getInfoLog();

                return false;
            }

            glslang::TProgram program;
            program.addShader(&shader);
            if (!program.link(messages))
            {
                infoLog = program.getInfoLog();

                return false;
            }

            // Map IO for SPIRV generation.
            if (!program.mapIO())
            {
                infoLog = program.getInfoLog();

                return false;
            }

            // Save any info log that was generated.
            if (strlen(shader.getInfoLog()))
                infoLog += String(shader.getInfoLog()) + "\n" + String(shader.getInfoDebugLog()) + "\n";

            if (strlen(program.getInfoLog()))
                infoLog += String(program.getInfoLog()) + "\n" + String(program.getInfoDebugLog());

            // Translate to SPIRV.
            if (program.getIntermediate(language))
            {
                glslang::SpvOptions spvOptions;
                spvOptions.generateDebugInfo = false;
                spvOptions.disableOptimizer = true;
                spvOptions.optimizeSize = false;

                spv::SpvBuildLogger logger;

                glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &logger, &spvOptions);
                auto spvMessages = logger.getAllMessages();
                if (!spvMessages.empty())
                {
                    infoLog += spvMessages + "\n";
                }
            }

            return true;
        }
    }
}
