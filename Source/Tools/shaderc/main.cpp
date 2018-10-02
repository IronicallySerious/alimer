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

#include "CLI11.hpp"
#include "Compiler.h"
#include <iostream>

#define ALIMER_SHADERC_VERSION_MAJOR 0
#define ALIMER_SHADERC_VERSION_MINOR 9

using namespace Alimer;
using namespace std;

int main(int argc, char* argv[])
{
    CLI::App app{ "shaderc, Alimer shader compiler tool, version 0.9.", "shaderc" };

    CompilerOptions options;

    app.add_option("-v,--vert", options.shaders[static_cast<uint32_t>(CompilerShaderStage::Vertex)].file, "Vertex shader source file")->check(CLI::ExistingFile);
    app.add_option("-f,--frag", options.shaders[static_cast<uint32_t>(CompilerShaderStage::Fragment)].file, "Fragment shader source file")->check(CLI::ExistingFile);
    app.add_option("-c,--compute", options.shaders[static_cast<uint32_t>(CompilerShaderStage::Compute)].file, "Compute shader source file")->check(CLI::ExistingFile);
    app.add_option("-i", options.includeDirs, "Target include paths.");
    app.add_option("-o,--output", options.outputFile, "Path to output directory")->required(true);

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    Compiler compiler;
    return compiler.Compile(options) ? EXIT_SUCCESS : EXIT_FAILURE;
}
