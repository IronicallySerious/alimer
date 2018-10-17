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

    /// Defines a shader module class.
    class ALIMER_API ShaderModule : public GraphicsResource
    {
    protected:
        /// Constructor.
        ShaderModule(GraphicsDevice* device, Util::Hash hash, const uint32_t* pCode, size_t size);

    public:
        Util::Hash GetHash() const { return _hash; }
        ShaderStage GetStage() const { return _reflection.stage; }
        const ShaderReflection& GetReflection() const { return _reflection; }

    private:
        Util::Hash _hash;
        String _source;
        String _infoLog;
        ShaderReflection _reflection;
    };

    /// Defines a shader program
    class ALIMER_API Program : public GraphicsResource
    {
    protected:
        /// Constructor.
        Program(GraphicsDevice* device, Util::Hash hash, const std::vector<ShaderModule*>& shaders);

    public:
        Util::Hash GetHash() const { return _hash; }

    private:
        Util::Hash _hash;
    };

    class ALIMER_API ShaderTemplate final
    {
    public:
        ShaderTemplate(const std::string& path);

        struct Variant
        {
            std::vector<uint32_t> spirv;
            std::vector<std::pair<std::string, int>> defines;
            uint32_t instance = 0;
        };

        const Variant* RegisterVariant(const std::vector<std::pair<std::string, int>> *defines = nullptr);

    private:
        std::string _path;
        Cache<Variant> _variants;
    };

    /// Defines a shader program class.
    class ALIMER_API ShaderProgram : public GraphicsResource
    {
    public:
        /// Constructor.
        ShaderProgram(GraphicsDevice* device);

        void SetStage(ShaderStage stage, ShaderTemplate* shader);
        uint32_t RegisterVariant(const std::vector<std::pair<std::string, int>> &defines);
        Program* GetVariant(uint32_t variant);

    private:
        struct Variant
        {
            const ShaderTemplate::Variant* stages[static_cast<unsigned>(ShaderStage::Count)] = {};
            uint32_t shaderInstance[static_cast<unsigned>(ShaderStage::Count)] = {};
            Program* program;
        };

        ShaderTemplate* _stages[static_cast<unsigned>(ShaderStage::Count)] = {};
        std::vector<Variant> _variants;
        std::vector<Util::Hash> _variantHashes;
    };
}
