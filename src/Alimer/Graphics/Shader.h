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

#pragma once

#include <vector>
#include "../Resource/Resource.h"
#include "../Graphics/GPUResource.h"

namespace alimer
{
    class ShaderHandle;

    /// Defines a shader resource.
    class ALIMER_API Shader : public Resource, public GPUResource
    {
        friend class GraphicsDevice;

        ALIMER_OBJECT(Shader, Resource);

    public:
        /// Constructor.
        Shader();

        /// Destructor.
        ~Shader() override;

        void Destroy() override;

        /// Define shader stage and bytecode. All existing variations are destroyed.
        void Define(ShaderStage stage, const std::vector<uint8_t>& byteCode);

        /// Define shader stage and source code. All existing variations are destroyed.
        void Define(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main");

        /// Get the shader stage.
        ShaderStage GetStage() const { return _stage; }

        /// Get the backend handle.
        ShaderHandle* GetHandle() const { return _handle; }

    private:
        /// Register object factory.
        static void RegisterObject();

    protected:
        void Reflect(const std::vector<uint8_t>& bytecode);

        ShaderStage _stage = ShaderStage::Count;
        std::vector<uint8_t> _byteCode;
        std::string _sourceCode;

        ShaderHandle* _handle = nullptr;
    };
}
