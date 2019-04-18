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

#include "Turso3D.h"
using namespace Turso3D;
//using namespace alimer;

class HelloWorldApp : public Application
{
public:
    HelloWorldApp(/*const ApplicationConfiguration& config*/)
        : Application(/*config*/)
    {
    }
};

int main()
{
    /*ApplicationConfiguration config = {};
    config.title = "Sample 01 - Hello World";
    HelloWorldApp app(config);
    return app.Run();*/

#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    HelloWorldApp app/*(config)*/;
    return app.Run();

    /*Log log;
    Profiler profiler;
    ResourceCache cache;

    Image* image = nullptr;

    {
        profiler.BeginFrame();
        cache.AddResourceDir(ExecutableDir() + "Data");
        image = cache.LoadResource<Image>("Test.png");
        profiler.EndFrame();
    }

    if (image)
    {
        //printf("Image loaded successfully, size %dx%d pixel byte size %d\n", image->Width(), image->Height(), (int)image->PixelByteSize());
        image->SavePNG("Test_Save.png");
    }

    TURSO3D_LOGRAW(profiler.OutputResults(false, false, 16));*/
}
