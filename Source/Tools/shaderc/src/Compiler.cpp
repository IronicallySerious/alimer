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
#include "StandAlone/ResourceLimits.h"

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

    static inline std::string GetFileName(std::string path)
    {
        int i = int(path.size()) - 1;
        for (; i > 0; --i) {
            if (path[i] == '/' || path[i] == '\\') {
                ++i;
                break;
            }
        }
        return path.substr(i, std::string::npos);
    }

    static inline std::string GetExtension(const std::string &path)
    {
        auto index = path.find_last_of('.');
        if (index == std::string::npos)
            return "";
        
        return path.substr(index + 1, std::string::npos);
    }

    static inline std::string RemoveExtension(std::string fileName)
    {
        int i = int(fileName.size()) - 1;
        for (; i > 0; --i) {
            if (fileName[i] == '.') {
                break;
            }
        }
        if (i == 0) 
            return fileName;
        
        return fileName.substr(0, i);
    }

    static std::string GetStageName(EShLanguage stage)
    {
        switch (stage) {
        case EShLangVertex:
            return "vs";
        case EShLangFragment:
            return "fs";
        case EShLangCompute:
            return "cs";
        default:
            return "";
        }
    }

    static inline void WriteSpirv(const std::string& fileName, std::vector<uint32_t>& words) 
    {
        std::ofstream out;
        out.open(fileName, std::ios::binary | std::ios::out);

        for (uint32_t i = 0; i < words.size(); ++i) {
            out.put(words[i] & 0xff);
            out.put((words[i] >> 8) & 0xff);
            out.put((words[i] >> 16) & 0xff);
            out.put((words[i] >> 24) & 0xff);
        }

        out.close();
    }

    static inline void WriteCHeader(const std::string& fileName, const std::string& name, std::vector<uint8_t>& bytecode)
    {
        std::ofstream out;
        out.open(fileName, std::ios::out);
        out << "// This file is automatically created by alimer shader compiler v" << COMPILER_VERSION_MAJOR  << "." << COMPILER_VERSION_MINOR << "." << COMPILER_VERSION_PATCH << std::endl;
        out << "// https://github.com/amerkoleci/alimer" << std::endl;
        out << std::endl;
        out << "#pragma once" << std::endl << std::endl;

        size_t size = bytecode.size();
        out << "static const unsigned char " << name << "[" << size << "] = {" << std::endl << "\t";

        char hexValue[32];
        int char_offset = 0;
        const int chars_per_line = 16;
        for (size_t i = 0; i < size; i++)
        {
            if (i != size - 1)
            {
                snprintf(hexValue, sizeof(hexValue), "0x%02x, ", bytecode[i]);
            }
            else {
                snprintf(hexValue, sizeof(hexValue), "0x%02x };\n", bytecode[i]);
            }

            out << hexValue;

            ++char_offset;
            if (char_offset == chars_per_line)
            {
                out << std::endl << "\t";
                char_offset = 0;
            }
        }

        out.close();
    }

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
        // Set message options.
        int cOptions = EOptionSpv | EOptionVulkanRules | EOptionLinkProgram;
        

        std::string def("#extension GL_GOOGLE_include_directive : require\n");

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

        auto firstExtStart = options.inputFile.find_last_of(".");
        bool hasFirstExt = firstExtStart != string::npos;
        auto secondExtStart = hasFirstExt ? options.inputFile.find_last_of(".", firstExtStart - 1) : string::npos;
        bool hasSecondExt = secondExtStart != string::npos;
        string firstExt = options.inputFile.substr(firstExtStart + 1);
        bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");

        glslang::EShClient client = glslang::EShClientVulkan;
        glslang::EShSource source = glslang::EShSourceGlsl;
        if (usesUnifiedExt && firstExt == "hlsl")
        {
            source = glslang::EShSourceHlsl;
            cOptions |= EOptionReadHlsl;
        }

        string stageName;
        if (hasFirstExt && !usesUnifiedExt)
        {
            stageName = firstExt;
        }
        else if (usesUnifiedExt && hasSecondExt)
        {
            stageName = options.inputFile.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);
        }

        std::ifstream stream(options.inputFile);
        std::string shaderSource((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        CompilerShaderStage stage = CompilerShaderStage::Count;
        if (stageName == "vert")
            stage = CompilerShaderStage::Vertex;
        else if (stageName == "tesc")
            stage = CompilerShaderStage::TessControl;
        else if (stageName == "tese")
            stage = CompilerShaderStage::TessEvaluation;
        else if (stageName == "geom")
            stage = CompilerShaderStage::Geometry;
        else if (stageName == "frag")
            stage = CompilerShaderStage::Fragment;
        else if (stageName == "comp")
            stage = CompilerShaderStage::Compute;

        EShMessages messages = EShMsgDefault;
        SetMessageOptions(cOptions, messages);
        int defaultVersion = 100;

        EShLanguage language = MapShaderStage(stage);
        glslang::TShader shader(language);
        const char* shaderStrings = shaderSource.c_str();
        const int shaderLengths = static_cast<int>(shaderSource.length());
        const char* stringNames = options.inputFile.c_str();
        shader.setStringsWithLengthsAndNames(&shaderStrings, &shaderLengths, &stringNames, 1);
        shader.setInvertY(options.invertY ? true : false);
        shader.setEnvInput(source, language, client, defaultVersion);
        shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
        //shader.setEntryPoint(options.entryPoint.c_str());
        //shader.setEnvTargetHlslFunctionality1();
        shader.setShiftSamplerBinding(0);
        shader.setShiftTextureBinding(0);
        shader.setShiftImageBinding(0);
        shader.setShiftUboBinding(0);
        shader.setShiftSsboBinding(0);
        shader.setFlattenUniformArrays(false);
        shader.setNoStorageFormat(false);
        shader.setPreamble(def.c_str());
        shader.addProcesses(processes);

        auto resourceLimits = glslang::DefaultTBuiltInResource;

        if (!shader.parse(&glslang::DefaultTBuiltInResource, defaultVersion, false, messages))
        {
            _errorMessage = shader.getInfoLog();

            return false;
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
        {
            _errorMessage = program.getInfoLog();
            return false;
        }

        // Map IO for SPIRV generation.
        if (!program.mapIO())
        {
            _errorMessage = program.getInfoLog();
            return false;
        }

        if (program.getIntermediate(language))
        {
            glslang::SpvOptions spvOptions;
            //spvOptions.generateDebugInfo = false;
            //spvOptions.disableOptimizer = true;
            //spvOptions.optimizeSize = false;
            spvOptions.validate = true;

            spv::SpvBuildLogger logger;
            std::vector<uint32_t> spirv;
            glslang::GlslangToSpv(
                *program.getIntermediate(language),
                spirv,
                &logger,
                &spvOptions);

            /* Write output spv first. */
            CompilerShaderFormat format = options.format;
            if (format == CompilerShaderFormat::DEFAULT)
            {
                auto fileExt = GetExtension(options.outputFile);
                if (fileExt == "h")
                {
                    format = CompilerShaderFormat::C_HEADER;
                }
                else
                {
                    format = CompilerShaderFormat::BLOB;
                }
            }

            switch (format)
            {
            case CompilerShaderFormat::DEFAULT:
            case CompilerShaderFormat::BLOB:
                WriteSpirv(options.outputFile, spirv);
                break;
            case Alimer::CompilerShaderFormat::C_HEADER:
            {
                std::vector<uint8_t> bytecode;
                bytecode.resize(spirv.size() * sizeof(uint32_t));
                memcpy(bytecode.data(), spirv.data(), bytecode.size());
                string fileName = RemoveExtension(GetFileName(options.inputFile));
                fileName += "_" + GetStageName(language);
                WriteCHeader(options.outputFile, fileName, bytecode);
            }
                break;
            default:
                break;
            }
        }

        return true;
    }
}
