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

#include "Alimer.h"
using namespace Alimer;



namespace Alimer
{
    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override;

    private:
        void Initialize() override;
    };

    class Settings
    {
    public:
        void Serialize(Serializer& serializer)
        {
            serializer.Serialize("width", 800);
            serializer.Serialize("height", 600);
            serializer.Serialize("fullscreen", false);
        }
    };

    RuntimeApplication::RuntimeApplication()
    {
        std::vector<char> c = { 'c','i','a','o' };
        auto s = str::ToString(ShaderStage::Geometry);
        ShaderStage stage = str::FromString<ShaderStage>(s);
        std::map<std::string, ShaderStage> map;
        map["CIAO"] = ShaderStage::Vertex;
        map["CIAO2"] = ShaderStage::Compute;
        Color color;
        Settings settings;

        {
            auto stream = OpenStream("Test.json", StreamMode::WriteOnly);
            JsonSerializer serializer(*stream.get());

            serializer.Serialize("color", Color::Green);
            serializer.Serialize("str", "Hello World");
            serializer.Serialize("stage", ShaderStage::Compute);
            serializer.Serialize("settings", settings);

            serializer.BeginObject("testArray", true);
            serializer.Serialize(nullptr, 450);
            serializer.Serialize(nullptr, 420);
            serializer.EndObject();

            serializer.Serialize("vector", c);
            serializer.Serialize("map", map);
        }
    }

    RuntimeApplication::~RuntimeApplication()
    {
    }

    void RuntimeApplication::Initialize()
    {
        // Create scene
        auto triangleEntity = _scene->CreateEntity();
        triangleEntity->AddComponent<TransformComponent>();
        triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
