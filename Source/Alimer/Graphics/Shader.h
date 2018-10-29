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

    struct ShaderStageDescriptor
    {

    };

    struct ShaderDescriptor
    {
        ShaderBlob stages[static_cast<unsigned>(ShaderStage::Count)];
    };

    /// Defines a shader module class.
    class ALIMER_API ShaderModule : public GraphicsResource
    {
    protected:
        /// Constructor.
        ShaderModule(GraphicsDevice* device, uint64_t hash, const ShaderBlob& blob);

    public:
        uint64_t GetHash() const { return _hash; }
        ShaderStage GetStage() const { return _reflection.stage; }
        const ShaderReflection& GetReflection() const { return _reflection; }

    private:
        uint64_t _hash;
        String _source;
        String _infoLog;
        ShaderReflection _reflection;
    };

    /// Defines a shader class.
    class ALIMER_API Shader : public Resource
    {
        ALIMER_OBJECT(Shader, Resource);

    protected:
        /// Constructor.
        Shader(GraphicsDevice* device, const ShaderDescriptor* descriptor);

    public:
        /// Get whether shader is compute.
        bool IsCompute() const { return _isCompute; }

    private:
        bool _isCompute;
    };
}
