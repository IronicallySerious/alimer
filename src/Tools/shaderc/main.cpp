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

#include "Compiler.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>
#include <fmt/printf.h>

#define ALIMER_SHADER_COMPILER_VERSION std::string("0.9.0")

using namespace std;
bool verboseOutput = false;

int main(int argc, char* argv[])
{
    CLI::App app{ fmt::sprintf("AlimerShaderCompiler %s: A tool for compiling HLSL to many shader languages.", ALIMER_SHADER_COMPILER_VERSION), "AlimerShaderCompiler" };
    app.add_flag("-v,--verbose", verboseOutput, "Enable verbose mode.");

    app.add_flag_function("-V,--version", [&](size_t count) {
        fmt::printf(
            "AlimerShaderCompiler version %s\n2017-2019 Amer Koleci and contributors.\n",
            ALIMER_SHADER_COMPILER_VERSION);
        exit(0);
    });

    std::string inputPath;
    std::string outputPath;
    std::string vertexShaderEntryPoint;
    std::string pixelShaderEntryPoint;
    std::string computeShaderEntryPoint;

    app.add_option("-i,--input", inputPath, "Input shader file.")->check(CLI::ExistingFile);
    app.add_option("-o,--output", outputPath, "Output shader file.");
    app.add_option("--vertex", vertexShaderEntryPoint, "Entry point of the vertex shader.");
    app.add_option("-p,--pixel", pixelShaderEntryPoint, "Entry point of the pixel shader.");
    app.add_option("-c,--compute", computeShaderEntryPoint, "Entry point of the compute shader.");

    // //("T,target", "Target shading language: dxil, spirv, hlsl, glsl, essl, msl", cxxopts::value<std::string>()->default_value("dxil"))

    CLI11_PARSE(app, argc, argv);
    using namespace ShaderCompiler;

    Compiler::Options options;
    options.fileName = inputPath;
    //const auto targetName = opts["target"].as<std::string>();

    if (!vertexShaderEntryPoint.empty())
    {
        options.shaders.push_back({ ShaderStage::Vertex, vertexShaderEntryPoint });
    }

    if (!pixelShaderEntryPoint.empty())
    {
        options.shaders.push_back({ ShaderStage::Pixel, pixelShaderEntryPoint });
    }

    if (!computeShaderEntryPoint.empty())
    {
        options.shaders.push_back({ ShaderStage::Compute, computeShaderEntryPoint });
    }

    options.targetLanguage = ShadingLanguage::DXC;

    std::string outputName;
    if (outputPath.empty())
    {
        static const std::string extMap[] = { "dxil", "spv", "cso", "hlsl", "glsl", "essl", "msl" };
        static_assert(sizeof(extMap) / sizeof(extMap[0]) == static_cast<uint32_t>(ShadingLanguage::COUNT),
            "extMap doesn't match with the number of shading languages.");
        outputName = options.fileName + "." + extMap[static_cast<uint32_t>(options.targetLanguage)];
    }
    else
    {
        outputName = outputPath;
    }

    {
        std::ifstream inputFile(options.fileName, std::ios_base::binary);
        if (!inputFile)
        {
            std::cerr << "COULDN'T load the input file: " << options.fileName << std::endl;
            return 1;
        }

        inputFile.seekg(0, std::ios::end);
        options.source.resize(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg);
        inputFile.read(&options.source[0], options.source.size());
    }

    try
    {
        const auto result = Compiler::Compile(std::move(options));

        if (!result.errorWarningMsg.empty())
        {
            std::cerr << "Error or warning form shader compiler: " << std::endl << result.errorWarningMsg << std::endl;
        }

        if (!result.bytecode.empty())
        {
            std::ofstream outputFile(outputName, std::ios_base::binary);
            if (!outputFile)
            {
                std::cerr << "COULDN'T open the output file: " << outputName << std::endl;
                return 1;
            }

            outputFile.write(reinterpret_cast<const char*>(result.bytecode.data()), result.bytecode.size());

            std::cout << "The compiled file is saved to " << outputName << std::endl;
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
