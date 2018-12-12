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

#pragma once

#include "../Graphics/GraphicsResource.h"
#include "../Resource/Resource.h"
#include "../Base/Vector.h"

namespace Alimer
{
    /// Defines shader stage
    enum class ShaderStage : uint32_t
    {
        Vertex = 0,
        TessControl = 1,
        TessEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5,
        Count = 6
    };

    class ShaderModuleImpl;

    /// Defines a shader module resource.
    class ALIMER_API ShaderModule final : public Resource
    {
        friend class Graphics;
        ALIMER_OBJECT(ShaderModule, Resource);

    public:
        /// Constructor.
        ShaderModule();

        /// Destructor.
        ~ShaderModule() override;

        /// Unconditionally destroy the GPU resource.
        void Destroy();

        bool Define(ShaderStage stage, const String& shaderSource, const String& entryPoint = "main");
        bool Define(ShaderStage stage, const Vector<uint8_t>& bytecode);

    private:
        bool BeginLoad(Stream& source) override;
        bool EndLoad() override;

        /// Process include statements in the shader source code recursively. Return true if successful.
        bool ProcessIncludes(String& code, Stream& source);

        /// Register object factory.
        static void RegisterObject();

        ShaderModuleImpl* _impl;
        Vector<PipelineResource> _resources;
        /// Shader stage.
        ShaderStage _stage = ShaderStage::Count;
        /// Shader source code.
        String _sourceCode;
    };

    /// Defines a shader resource.
    class ALIMER_API Shader final : public Resource
    {
        friend class Graphics;
        ALIMER_OBJECT(Shader, Resource);

    public:
        /// Constructor.
        Shader();

        /// Destructor.
        ~Shader() override;

        /// Unconditionally destroy the GPU resource.
        void Destroy();

        bool Define(ShaderModule* vertex, ShaderModule* fragment);

        inline const ShaderModule* GetShader(ShaderStage stage) const
        {
            return _shaders[static_cast<unsigned>(stage)].Get();
        }

    private:
        bool BeginLoad(Stream& source) override;
        bool EndLoad() override;

        /// Register object factory.
        static void RegisterObject();

        SharedPtr<ShaderModule> _shaders[static_cast<unsigned>(ShaderStage::Count)] = {};
    };

    ALIMER_API const char* EnumToString(ShaderStage stage);
}
