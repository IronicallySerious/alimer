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

#include "../Graphics/Types.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Resource/ResourceManager.h"
#include "../IO/Path.h"
#include "../Core/Log.h"
#if TODO_SHADER_COMPILER
#include "glslang/Public/ShaderLang.h"
#include "glslang/StandAlone/ResourceLimits.h"
#include "SPIRV/GlslangToSpv.h"
#include <fstream>
#include <sstream>

namespace alimer
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
                ALIMER_LOGCRITICAL("Cannot open include file '{}'", fullPath.CString());
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

    ShaderBlob ShaderCompiler::Compile(
        const char* filePath,
        const char* entryPoint,
        ShaderLanguage language,
        const std::vector<std::pair<std::string, int>> &defines)
    {
        auto stream = FileSystem::Get().Open(filePath);
        if (!stream)
        {
            _errorMessage = str::Format("Shader file '%s' does not exists", filePath);
            return {};
        }

        std::string filePathStr = filePath;
        String shaderSource = stream->ReadAllText();
        auto firstExtStart = filePathStr.find_last_of(".");
        bool hasFirstExt = firstExtStart != String::NPOS;
        auto secondExtStart = hasFirstExt ? filePathStr.find_last_of(".", firstExtStart - 1) : String::NPOS;
        bool hasSecondExt = secondExtStart != String::NPOS;
        String firstExt = filePathStr.substr(firstExtStart + 1);
        bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");
        if (usesUnifiedExt && firstExt == "hlsl")
        {
            // Add HLSL to glslang
        }

        String stageName;
        if (hasFirstExt && !usesUnifiedExt)
            stageName = firstExt;
        else if (usesUnifiedExt && hasSecondExt)
            stageName = filePathStr.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);

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

        return Compile(shaderSource.CString(), entryPoint, language, stage, filePath, defines);
    }

    ShaderBlob ShaderCompiler::Compile(
        const char* source,
        const char* entryPoint,
        ShaderLanguage language,
        ShaderStage stage,
        const char* filePath,
        const std::vector<std::pair<std::string, int>> &defines)
    {
        static bool glslInitialized = false;
        if (!glslInitialized)
        {
            // Initialize glslang library.
            glslang::InitializeProcess();

            glslInitialized = true;
        }

        // Get default built in resource limits.
        auto resourceLimits = glslang::DefaultTBuiltInResource;

        // construct semantics mapping defines
        // to be used in layout(location = SEMANTIC) inside GLSL
        String semanticsDef;
        for (uint32_t i = 0; i < static_cast<uint32_t>(VertexElementSemantic::Count); i++)
        {
            VertexElementSemantic semantic = static_cast<VertexElementSemantic>(i);
            semanticsDef += String::Format("#define %s %d\n", EnumToString(semantic), i);
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

        EShLanguage eshLanguage = MapShaderStage(stage);
        glslang::TShader shader(eshLanguage);

        const char* shaderStrings = source;
        const int shaderLengths = static_cast<int>(strlen(source));
        const char* stringNames = filePath && strlen(filePath) ? filePath : "unknown";
        shader.setStringsWithLengthsAndNames(
            &shaderStrings,
            &shaderLengths,
            &stringNames, 1);
        shader.setPreamble(preamble.CString());
        shader.setEntryPoint(entryPoint);
        shader.setSourceEntryPoint(entryPoint);
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

        AlimerIncluder includer(FileSystem::GetPath(filePath));

        const EProfile DefaultProfile = ENoProfile;
        const bool ForceVersionProfile = false;
        const bool NotForwardCompatible = false;

        bool parseSuccess = shader.parse(
            &resourceLimits,
            450,
            DefaultProfile,
            ForceVersionProfile,
            NotForwardCompatible,
            messages,
            includer);

        if (!parseSuccess)
        {
            _errorMessage = shader.getInfoLog();
            return {};
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
        {
            _errorMessage = program.getInfoLog();
            return {};
        }

        // Map IO for SPIRV generation.
        if (!program.mapIO())
        {
            _errorMessage = program.getInfoLog();
            return {};
        }

        // Save any info log that was generated.
        if (strlen(shader.getInfoLog()))
            _errorMessage += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";

        if (strlen(program.getInfoLog()))
            _errorMessage += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());

        // Translate to SPIRV.
        ShaderBlob blob = {};
        if (program.getIntermediate(eshLanguage))
        {
            glslang::SpvOptions spvOptions;
            spvOptions.generateDebugInfo = false;
            spvOptions.disableOptimizer = true;
            spvOptions.optimizeSize = false;

            spv::SpvBuildLogger logger;
            std::vector<uint32_t> spirv;
            glslang::GlslangToSpv(
                *program.getIntermediate(eshLanguage),
                spirv,
                &logger,
                &spvOptions);

            auto spvMessages = logger.getAllMessages();
            if (!spvMessages.empty())
            {
                _errorMessage += spvMessages + "\n";
            }
            else
            {
                blob.size = spirv.size() * sizeof(uint32_t);
                blob.data = new uint8_t[blob.size];
                memcpy(blob.data, spirv.data(), blob.size);
            }
        }

        return blob;
    }
}
#endif // TODO_SHADER_COMPILER
