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

#include "../Base/Cache.h"
#include "../Graphics/GraphicsResource.h"
#include "../Resource/Resource.h"
#include <string>
#include <vector>

namespace Alimer
{
    ALIMER_API void SPIRVReflectResources(const uint32_t* pCode, size_t size, ShaderReflection* reflection);

    struct ShaderBlob
    {
        uint64_t size;
        uint8_t *data;
    };

    struct ShaderDescriptor
    {
        ShaderBlob stages[static_cast<unsigned>(ShaderStage::Count)];
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

        bool Define(ShaderStage stage, const String& shaderSource, const String& entryPoint = "main");

        bool Define(const  std::vector<uint8_t>& bytecode);

        bool Finalize();

        /// Get the gpu handle.
        inline AgpuShader GetHandle() const { return _handle; }
    private:
        /// Register object factory.
        static void RegisterObject();

        //uint64_t _hash;
        AgpuShaderDescriptor _shaderDescriptor = {};
        AgpuShader _handle = nullptr;
    };
}
