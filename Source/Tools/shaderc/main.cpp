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
#include <iostream>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4819)
#endif
#include <cxxopts.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

int main(int argc, char* argv[])
{
    cxxopts::Options cmd_options("AlimerShaderCompiler", "A tool for compiling HLSL to many shader languages.");

    // clang-format off
    cmd_options.add_options()
        ("I,input", "Input file name.", cxxopts::value<std::string>())
        ("O,output", "Output file name", cxxopts::value<std::string>())
        ("V,vert", "Entry point of the vertex shader.", cxxopts::value<std::string>()->default_value("VSMain"))
        ("P,pixel", "Entry point of the pixel shader.", cxxopts::value<std::string>()->default_value("PSMain"))
        ("C,comp", "Entry point of the compute shader.", cxxopts::value<std::string>()->default_value(""))
        //("T,target", "Target shading language: dxil, spirv, hlsl, glsl, essl, msl", cxxopts::value<std::string>()->default_value("dxil"))
        //("V,version", "The version of target shading language", cxxopts::value<std::string>()->default_value(""))
    ;
    // clang-format on

    auto opts = cmd_options.parse(argc, argv);

    if ((opts.count("input") == 0))
    {
        std::cerr << "COULDN'T find <input> in command line parameters." << std::endl;
        std::cerr << cmd_options.help() << std::endl;
        return 1;
    }

    using namespace ShaderCompiler;

    Compiler::Options options;
    options.fileName = opts["input"].as<std::string>();
    //const auto targetName = opts["target"].as<std::string>();
    //options.version = opts["version"].as<std::string>();

    const auto vertex = opts["vert"].as<std::string>();
    const auto pixel = opts["pixel"].as<std::string>();
    const auto comp = opts["comp"].as<std::string>();

    if (!vertex.empty())
    {
        options.shaders.push_back({ ShaderStage::Vertex, vertex });
    }

    if (!pixel.empty())
    {
        options.shaders.push_back({ ShaderStage::Pixel, pixel });
    }

    if (!comp.empty())
    {
        options.shaders.push_back({ ShaderStage::Compute, comp });
    }

    options.targetLanguage = ShadingLanguage::DXC;

    std::string outputName;
    if (opts.count("output") == 0)
    {
        static const std::string extMap[] = { "dxil", "spv", "cso", "hlsl", "glsl", "essl", "msl" };
        static_assert(sizeof(extMap) / sizeof(extMap[0]) == static_cast<uint32_t>(ShadingLanguage::COUNT),
            "extMap doesn't match with the number of shading languages.");
        outputName = options.fileName + "." + extMap[static_cast<uint32_t>(options.targetLanguage)];
    }
    else
    {
        outputName = opts["output"].as<std::string>();
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
