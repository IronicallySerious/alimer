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

#include <fstream>
using namespace std;

#if !defined(ALIMER_SHADER_COMPILER)
namespace Alimer
{
    namespace ShaderCompiler
    {
        vector<uint8_t> Compile(const std::string& filePath, std::string& errorLog)
        {
            return {};
        }

        vector<uint8_t> Compile(const std::string& shaderSource, const std::string& filePath, ShaderStage stage, std::string& errorLog)
        {
            return {};
        }
    }
}
#else
#include "../Resource/ResourceManager.h"
#include "../IO/Path.h"
#include "../Core/Log.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

namespace Alimer
{
    inline static void InitResources(TBuiltInResource & resources)
    {
        // These numbers come from the OpenGL 4.4 core profile specification Chapter 23
        // unless otherwise specified.
        resources.maxLights = 8;
        resources.maxClipPlanes = 6;
        resources.maxTextureUnits = 2;
        resources.maxTextureCoords = 8;
        resources.maxVertexAttribs = 16;
        resources.maxVertexUniformComponents = 4096;
        resources.maxVaryingFloats = 60;
        resources.maxVertexTextureImageUnits = 16;
        resources.maxCombinedTextureImageUnits = 80;
        resources.maxTextureImageUnits = 16;
        resources.maxFragmentUniformComponents = 1024;
        resources.maxDrawBuffers = 8;
        resources.maxVertexUniformVectors = 256;
        resources.maxVaryingVectors = 15;
        resources.maxFragmentUniformVectors = 256;
        resources.maxVertexOutputVectors = 16;
        resources.maxFragmentInputVectors = 15;
        resources.minProgramTexelOffset = -8;
        resources.maxProgramTexelOffset = 7;
        resources.maxClipDistances = 8;
        resources.maxComputeWorkGroupCountX = 65535;
        resources.maxComputeWorkGroupCountY = 65535;
        resources.maxComputeWorkGroupCountZ = 65535;
        resources.maxComputeWorkGroupSizeX = 1024;
        resources.maxComputeWorkGroupSizeY = 1024;
        resources.maxComputeWorkGroupSizeZ = 64;
        resources.maxComputeUniformComponents = 512;
        resources.maxComputeTextureImageUnits = 16;
        resources.maxComputeImageUniforms = 8;
        resources.maxComputeAtomicCounters = 8;
        resources.maxComputeAtomicCounterBuffers = 1;
        resources.maxVaryingComponents = 60;
        resources.maxVertexOutputComponents = 64;
        resources.maxGeometryInputComponents = 64;
        resources.maxGeometryOutputComponents = 128;
        resources.maxFragmentInputComponents = 128;
        resources.maxImageUnits = 8;
        resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        resources.maxCombinedShaderOutputResources = 8;
        resources.maxImageSamples = 0;
        resources.maxVertexImageUniforms = 0;
        resources.maxTessControlImageUniforms = 0;
        resources.maxTessEvaluationImageUniforms = 0;
        resources.maxGeometryImageUniforms = 0;
        resources.maxFragmentImageUniforms = 8;
        resources.maxCombinedImageUniforms = 8;
        resources.maxGeometryTextureImageUnits = 16;
        resources.maxGeometryOutputVertices = 256;
        resources.maxGeometryTotalOutputComponents = 1024;
        resources.maxGeometryUniformComponents = 512;
        resources.maxGeometryVaryingComponents = 60;
        resources.maxTessControlInputComponents = 128;
        resources.maxTessControlOutputComponents = 128;
        resources.maxTessControlTextureImageUnits = 16;
        resources.maxTessControlUniformComponents = 1024;
        resources.maxTessControlTotalOutputComponents = 4096;
        resources.maxTessEvaluationInputComponents = 128;
        resources.maxTessEvaluationOutputComponents = 128;
        resources.maxTessEvaluationTextureImageUnits = 16;
        resources.maxTessEvaluationUniformComponents = 1024;
        resources.maxTessPatchComponents = 120;
        resources.maxPatchVertices = 32;
        resources.maxTessGenLevel = 64;
        resources.maxViewports = 16;
        resources.maxVertexAtomicCounters = 0;
        resources.maxTessControlAtomicCounters = 0;
        resources.maxTessEvaluationAtomicCounters = 0;
        resources.maxGeometryAtomicCounters = 0;
        resources.maxFragmentAtomicCounters = 8;
        resources.maxCombinedAtomicCounters = 8;
        resources.maxAtomicCounterBindings = 1;
        resources.maxVertexAtomicCounterBuffers = 0;
        resources.maxTessControlAtomicCounterBuffers = 0;
        resources.maxTessEvaluationAtomicCounterBuffers = 0;
        resources.maxGeometryAtomicCounterBuffers = 0;
        resources.maxFragmentAtomicCounterBuffers = 1;
        resources.maxCombinedAtomicCounterBuffers = 1;
        resources.maxAtomicCounterBufferSize = 32;
        resources.maxTransformFeedbackBuffers = 4;
        resources.maxTransformFeedbackInterleavedComponents = 64;
        resources.maxCullDistances = 8;
        resources.maxCombinedClipAndCullDistances = 8;
        resources.maxSamples = 4;
        resources.limits.nonInductiveForLoops = 1;
        resources.limits.whileLoops = 1;
        resources.limits.doWhileLoops = 1;
        resources.limits.generalUniformIndexing = 1;
        resources.limits.generalAttributeMatrixVectorIndexing = 1;
        resources.limits.generalVaryingIndexing = 1;
        resources.limits.generalSamplerIndexing = 1;
        resources.limits.generalVariableIndexing = 1;
        resources.limits.generalConstantMatrixVectorIndexing = 1;
    }

    class AlimerIncluder : public glslang::TShader::Includer
    {
    public:
        AlimerIncluder(const string& from)
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

            string fullPath = Path::Join(_rootDirectory, headerName);
            stringstream content;
            string line;
            ifstream file(fullPath);
            if (file.is_open())
            {
                while (getline(file, line))
                {
                    content << line << '\n';
                }
                file.close();
            }

            string fileContent = content.str();
            char* heapContent = new char[fileContent.size() + 1];
            strcpy(heapContent, fileContent.c_str());
            return new IncludeResult(fullPath, heapContent, content.str().size(), heapContent);
        }

        void releaseInclude(IncludeResult* result) override {
            delete (char*)result->userData;
            delete result;
        }
    private:
        string _rootDirectory;
    };

    class GlslangInitializer {
    public:
        GlslangInitializer() { glslang::InitializeProcess(); }

        ~GlslangInitializer() { glslang::FinalizeProcess(); }
    };

    GlslangInitializer s_glslangInitializer;

    namespace ShaderCompiler
    {
        vector<uint8_t> Compile(
            const string& filePath,
            string& errorLog)
        {
            if (!FileExists(filePath))
            {
                ALIMER_LOGERROR("Shader file '{}' does not exists", filePath.c_str());
                return {};
            }

            auto stream = FileSystem::Get().Open(filePath);
            if (!stream)
            {
                ALIMER_LOGERROR("Cannot open shader file '{}'", filePath.c_str());
                return {};
            }

            string shaderSource = stream->ReadAllText();
            size_t firstExtStart = filePath.find_last_of(".");
            bool hasFirstExt = firstExtStart != string::npos;
            size_t secondExtStart = hasFirstExt ? filePath.find_last_of(".", firstExtStart - 1) : string::npos;
            bool hasSecondExt = secondExtStart != string::npos;
            string firstExt = filePath.substr(firstExtStart + 1, std::string::npos);
            bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");
            if (usesUnifiedExt && firstExt == "hlsl")
            {
                // Add HLSL to glslang
            }

            std::string stageName;
            if (hasFirstExt && !usesUnifiedExt)
                stageName = firstExt;
            else if (usesUnifiedExt && hasSecondExt)
                stageName = filePath.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);

            ShaderStage stage = ShaderStage::Count;
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

            return Compile(shaderSource, filePath, stage, errorLog);
        }

        vector<uint8_t> Compile(const string& shaderSource, const std::string& filePath, ShaderStage stage, string& errorLog)
        {
            TBuiltInResource resources;
            InitResources(resources);

            const string macroDefinitions = "";
            const string poundExtension =
                "#extension GL_GOOGLE_include_directive : enable\n";
            const string preamble = macroDefinitions + poundExtension;

            //
            EShLanguage language = EShLangCount;
            switch (stage)
            {
            case ShaderStage::Vertex:
                language = EShLangVertex;
                break;
            case ShaderStage::TessControl:
                language = EShLangTessControl;
                break;
            case ShaderStage::TessEvaluation:
                language = EShLangTessEvaluation;
                break;
            case ShaderStage::Geometry:
                language = EShLangGeometry;
                break;
            case ShaderStage::Fragment:
                language = EShLangFragment;
                break;
            case ShaderStage::Compute:
                language = EShLangCompute;
                break;
            default:
                break;
            }
            glslang::TProgram program;
            glslang::TShader shader(language);

            const char* shaderStrings = shaderSource.c_str();
            const int shaderLengths = static_cast<int>(shaderSource.length());
            const char* stringNames = filePath.c_str();
            shader.setStringsWithLengthsAndNames(
                &shaderStrings,
                &shaderLengths,
                &stringNames, 1);
            shader.setPreamble(preamble.c_str());
            shader.setEntryPoint("main");
            //shader.setInvertY(true);

            EShMessages messages = static_cast<EShMessages>(EShMsgCascadingErrors | EShMsgSpvRules | EShMsgVulkanRules);
            AlimerIncluder includer(GetPath(filePath));

            const EProfile DefaultProfile = ENoProfile;
            const bool ForceVersionProfile = false;
            const bool NotForwardCompatible = false;

            bool success = shader.parse(
                &resources,
                450,
                DefaultProfile,
                ForceVersionProfile,
                NotForwardCompatible,
                messages,
                includer);

            if (!success)
            {
                errorLog = shader.getInfoLog();
                return {};
            }

            program.addShader(&shader);
            success = program.link(EShMsgDefault) && program.mapIO();
            if (!success)
            {
                errorLog = program.getInfoLog();
                return {};
            }

            vector<uint32_t> spirv;
            glslang::SpvOptions options;
            options.generateDebugInfo = false;
            options.disableOptimizer = true;
            options.optimizeSize = false;
            glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &options);


            vector<uint8_t> byteCode(spirv.size() * sizeof(uint32_t));
            memcpy(byteCode.data(), spirv.data(), byteCode.size());
            return byteCode;
        }
    }
}

#endif
