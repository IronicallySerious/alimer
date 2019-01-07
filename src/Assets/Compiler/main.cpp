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

#include "AssetCompiler.h"
#include <CLI/CLI.hpp>
#include <fmt/printf.h>

#define ALIMER_ASSET_COMPILER_VERSION std::string("0.9.0")

using namespace alimer;
using namespace std;
bool verboseOutput = false;

int main(int argc, char* argv[])
{
    CLI::App app{fmt::sprintf("AlimerAssetCompiler %s: A tool for asset processing.", ALIMER_ASSET_COMPILER_VERSION), "AlimerAssetCompiler"};

    app.add_flag("-v,--verbose", verboseOutput, "Enable verbose mode.");

    app.add_flag_function("-V,--version", [&](size_t count) {
        ALIMER_UNUSED(count);
        fmt::printf(
            "AlimerAssetCompiler version %s\n2017-2019 Amer Koleci and contributors.\n",
            ALIMER_ASSET_COMPILER_VERSION);
        exit(0);
    });

    std::string inputPath;
    std::string outputPath;
    std::string targetPlatform;

    app.add_option("-i,--input", inputPath, "Source assets directory.")->check(CLI::ExistingDirectory);
    app.add_option("-o,--output", outputPath, "Output compiled assets output directory.");
    app.add_option("-t,--target", targetPlatform, "Target platform.");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) 
    {
        return app.exit(e);
    }

    AssetCompiler::Options options;
    options.assetsDirectory = inputPath;
    
    if (!targetPlatform.empty())
    {
        if (targetPlatform == "windows")
        {
            options.targetPlatform = PlatformType::Windows;
        }
        else if (targetPlatform == "linux")
        {
            options.targetPlatform = PlatformType::Linux;
        }
    }

    if (outputPath.empty())
    {
        String output = Path::Join(options.assetsDirectory, "windows");
        options.buildDirectory = output.CString();
    }
    else
    {
        options.buildDirectory = outputPath;
    }

    AssetCompiler compiler;
    if (!compiler.Run(options))
    {
        //auto errorMessage = compiler.GetErrorMessage();
        //fprintf(stderr, "%s\n", errorMessage.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
