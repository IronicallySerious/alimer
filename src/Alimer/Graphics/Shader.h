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

#include "../Graphics/GPUResource.h"
#include "../Base/Vector.h"

namespace alimer
{
    class Stream;
    struct GPUShader;

    /// Defines a shader resource.
    class ALIMER_API Shader final : public GPUResource
    {
        friend class GPUDevice;

    public:
        /// Constructor.
        Shader();

        /// Destructor.
        ~Shader() override;

        /// Unconditionally destroy the GPU resource.
        void Destroy();

        bool Define(const PODVector<uint8_t>& compute);
        bool Define(const PODVector<uint8_t>& vertex, const PODVector<uint8_t>& fragment);

        /// Get if shader is compute.
        bool IsCompute() const { return _compute; }

        /// Get the shader stage.
        ShaderStages GetStages() const { return _stage; }

        /// Get the GPUShader.
        GPUShader* GetGPUShader() const { return _module; }

    private:
        /// Register object factory.
        static void RegisterObject();

        void Reflect(const PODVector<uint8_t>& bytecode);

        GPUShader* _module = nullptr;

        //Vector<PipelineResource> _resources;
        bool _compute = false;
        /// Shader stage.
        ShaderStages _stage = ShaderStages::None;
    };
}
