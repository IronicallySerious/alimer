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

#include "AssetCompiler.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4819)
#endif
#include <cxxopts.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define ALIMER_SHADERC_VERSION_MAJOR 0
#define ALIMER_SHADERC_VERSION_MINOR 9

using namespace Alimer;
using namespace std;

int main(int argc, char* argv[])
{
    cxxopts::Options cmd_options("AlimerAssetCompiler", "A tool for asset processing.");

    // clang-format off
    cmd_options.add_options()
        ("I,input", "Source assets directory.", cxxopts::value<std::string>())
        ("O,output", "Output Compiled assets output directory", cxxopts::value<std::string>())
        ("T,target", "Target platform..", cxxopts::value<std::string>()->default_value(""))
        ;
    // clang-format on
    auto opts = cmd_options.parse(argc, argv);

    if ((opts.count("input") == 0))
    {
        std::cerr << "COULDN'T find <input> in command line parameters." << std::endl;
        std::cerr << cmd_options.help() << std::endl;
        return 1;
    }

    AssetCompiler::Options options;
    options.assetsDirectory = opts["input"].as<std::string>();
    
    const auto target = opts["target"].as<std::string>();

    if (!target.empty())
    {
        if (target == "windows")
        {
            options.targetPlatform = PLATFORM_TYPE_WINDOWS;
        }
        else if (target == "linux")
        {
            options.targetPlatform = PLATFORM_TYPE_LINUX;
        }
    }

    if (opts.count("output") == 0)
    {
        String output = Path::Join(options.assetsDirectory, "windows");
        options.buildDirectory = output.CString();
    }
    else
    {
        options.buildDirectory = opts["output"].as<std::string>();
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
