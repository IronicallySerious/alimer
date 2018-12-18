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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4458) 
#endif

#include "CLI11.hpp"
#include "AssetCompiler.h"

#define ALIMER_SHADERC_VERSION_MAJOR 0
#define ALIMER_SHADERC_VERSION_MINOR 9

using namespace Alimer;
using namespace std;

int main(int argc, char* argv[])
{
    CLI::App app{ "shaderc, Alimer shader compiler tool, version 0.9.", "shaderc" };

    AssetCompiler::Options options;
    std::string targetPlatform;
    app.add_option("-i,--input", options.assetsDirectory, "Source assets directory")->check(CLI::ExistingDirectory);
    app.add_option("-o,--output", options.buildDirectory, "Compiled assets output directory")->required(false);
    app.add_option("-t,--target", targetPlatform, "Target platform")->required(false);

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (!targetPlatform.empty())
    {
        if (targetPlatform == "windows")
        {
            options.targetPlatform = PLATFORM_TYPE_WINDOWS;
        }
        else if (targetPlatform == "linux")
        {
            options.targetPlatform = PLATFORM_TYPE_LINUX;
        }
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

#ifdef _MSC_VER
#   pragma warning(pop)
#endif
