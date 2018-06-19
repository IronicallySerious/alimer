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

#include "../Graphics/Types.h"
#include "../Resource/Resource.h"
#include <string>
#include <vector>
#include <map>

namespace Alimer
{
    class Graphics;

    struct ShaderStageDescription
    {
        std::vector<uint32_t> code;
        const char* entryPoint;
    };

    enum class BindingType
    {
        UniformBuffer,
        SampledTexture,
        Sampler,
        StorageBuffer
    };

    /*struct ShaderResourceParameter
    {
        uint32_t set;
        uint32_t binding;
        BindingType type;
        std::string name;
    };*/

    struct DescriptorSetLayout
    {
        uint32_t uniformBufferMask = 0;
        uint32_t stages = 0;
    };

    struct ResourceLayout
    {
        uint32_t attributeMask = 0;
        uint32_t renderTargetMask = 0;
        DescriptorSetLayout sets[MaxDescriptorSets] = {};
        uint32_t descriptorSetMask = 0;
        //std::map<std::string, ShaderResourceParameter> resources;
    };

    /// Defines a shader (module/function) class.
    class Shader : public Resource
    {
    protected:
        /// Constructor.
        Shader();

        /// Constructor.
        Shader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment);

    public:
        /// Destructor.
        virtual ~Shader();

        const ResourceLayout &GetResourceLayout() const
        {
            return _layout;
        }

    protected:
        void Reflect(ShaderStage stage, const uint32_t *data, size_t size);

        ShaderStageDescription _shaders[static_cast<unsigned>(ShaderStage::Count)] = {};
        ResourceLayout _layout;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Shader);
    };
}
