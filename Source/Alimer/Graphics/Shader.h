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

#include "../Core/Flags.h"
#include "../Graphics/GpuResource.h"
#include "../Resource/Resource.h"
#include <string>
#include <vector>
#include <map>

namespace Alimer
{
    class Graphics;

    ALIMER_API void SPIRVReflectResources(const std::vector<uint32_t>& spirv, ShaderStage& stage, std::vector<PipelineResource>& shaderResources);

    /// Defines a shader module class.
    class ALIMER_API ShaderModule : public Resource, public GpuResource
    {
        ALIMER_OBJECT(ShaderModule, Resource);

    public:
        /// Constructor.
        ShaderModule(Graphics* graphics, const std::vector<uint32_t>& spirv);

        /// Destructor.
        virtual ~ShaderModule() = default;

        ShaderStage GetStage() const { return _stage; }
        std::vector<uint32_t> AcquireBytecode();
        const std::vector<PipelineResource>& GetResources() const { return _resources; }

    protected:
        std::vector<uint32_t> _spirv;
        ShaderStage _stage = ShaderStage::Count;
        std::vector<PipelineResource> _resources;
    };

    /// Defines a shader program class.
    class ALIMER_API ShaderProgram : public GpuResource, public RefCounted
    {
    protected:
        /// Constructor.
        ShaderProgram(Graphics* graphics, const ShaderProgramDescriptor* descriptor);

    public:
        /// Destructor.
        virtual ~ShaderProgram() = default;

        inline const ShaderModule *GetShader(ShaderStage stage) const
        {
            return _shaders[static_cast<uint32_t>(stage)];
        }

        /// Gets if this program is compute program.
        bool IsCompute() const { return _isCompute; }

    protected:
        bool _isCompute;
        ShaderModule* _shaders[static_cast<uint32_t>(ShaderStage::Count)] = {};
    };
}
