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

#include <CLI11/CLI11.hpp>
#include <iostream>

#include "glslang/StandAlone/ResourceLimits.h"
#include "glslang/StandAlone/Worklist.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Include/revision.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

#define ALIMER_SHADERC_VERSION_MAJOR 0
#define ALIMER_SHADERC_VERSION_MINOR 9

using namespace std;

TBuiltInResource Resources;

std::string ReadFile(const std::string& path)
{
    std::stringstream content;
    std::string line;
    std::ifstream fileStream(path);
    if (fileStream.is_open()) {
        while (getline(fileStream, line)) {
            content << line << '\n';
        }
        fileStream.close();
    }

    return content.str();
}

class AlimerIncluder : public glslang::TShader::Includer
{
public:
    AlimerIncluder(const string& from)
    {
        _dir = from;
        for (int i = static_cast<int>(from.size()) - 1; i >= 0; --i)
        {
            if (_dir[i] == '/' || _dir[i] == '\\')
            {
                _dir = _dir.substr(0, i + 1);
                break;
            }
        }
    }

    IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
    {
        return includeLocal(headerName, includerName, inclusionDepth);
    }

    IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
    {
        //ALIMER_UNUSED(includerName);
        //ALIMER_UNUSED(inclusionDepth);

        string fullPath = _dir + headerName;
        stringstream content;
        string line;
        ifstream fileStream(fullPath);
        if (fileStream.is_open()) {
            while (getline(fileStream, line)) {
                content << line << '\n';
            }
            fileStream.close();
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
    string _dir;
};

int main(int argc, char* argv[])
{
    CLI::App app{ "shaderc, Alimer shader compiler tool, version 0.9.", "shaderc" };

    struct CommandLineOptions
    {
        std::string inputFile;
        std::vector<std::string> includeDirs;
        std::string outputFile;
    };

    CommandLineOptions options;

    app.add_option("-f,--file", options.inputFile, "Input file to compile")->required()->check(CLI::ExistingFile);
    app.add_option("-i", options.includeDirs, "Target include paths.");
    app.add_option("-o,--output", options.outputFile, "Path to output directory");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    size_t firstExtStart = options.inputFile.find_last_of(".");
    bool hasFirstExt = firstExtStart != string::npos;
    size_t secondExtStart = hasFirstExt ? options.inputFile.find_last_of(".", firstExtStart - 1) : string::npos;
    bool hasSecondExt = secondExtStart != string::npos;
    string firstExt = options.inputFile.substr(firstExtStart + 1, std::string::npos);
    bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");
    if (usesUnifiedExt && firstExt == "hlsl")
    {
        // Add HLSL to glslang
    }

    std::string stageName;
    if (hasFirstExt && !usesUnifiedExt)
        stageName = firstExt;
    else if (usesUnifiedExt && hasSecondExt)
        stageName = options.inputFile.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);

    EShLanguage stage = EShLangCount;
    if (stageName == "vert")
        stage = EShLangVertex;
    else if (stageName == "tesc")
        stage = EShLangTessControl;
    else if (stageName == "tese")
        stage = EShLangTessEvaluation;
    else if (stageName == "geom")
        stage = EShLangGeometry;
    else if (stageName == "frag")
        stage = EShLangFragment;
    else if (stageName == "comp")
        stage = EShLangCompute;

    Resources = glslang::DefaultTBuiltInResource;

    string errorLog;
    string shaderSource = ReadFile(options.inputFile);
    const string macroDefinitions = "";
    const string poundExtension =
        "#extension GL_GOOGLE_include_directive : enable\n";
    const string preamble = macroDefinitions + poundExtension;

    {
        glslang::InitializeProcess();

        glslang::TProgram program;
        glslang::TShader shader(stage);
        const char* shaderStrings = shaderSource.c_str();
        const int shaderLengths = static_cast<int>(shaderSource.length());
        const char* stringNames = options.inputFile.c_str();
        shader.setStringsWithLengthsAndNames(
            &shaderStrings,
            &shaderLengths,
            &stringNames, 1);
        shader.setPreamble(preamble.c_str());
        shader.setEntryPoint("main");

        EShMessages messages = static_cast<EShMessages>(EShMsgCascadingErrors | EShMsgSpvRules | EShMsgVulkanRules);
        AlimerIncluder includer(options.inputFile);

        bool success = shader.parse(
            &Resources,
            100,
            ENoProfile,
            false,
            false,
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

        std::vector<uint32_t> spirv;
        glslang::SpvOptions options;
        options.generateDebugInfo = false;
        options.disableOptimizer = true;
        options.optimizeSize = false;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &options);

        glslang::FinalizeProcess();
    }

    return 0;
}
