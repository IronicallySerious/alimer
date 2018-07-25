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

    /// Defines shader stage usage.
    enum class ShaderStage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        TessControl = 1 << 1,
        TessEvaluation = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        Compute = 1 << 5,
        AllTessellation = (TessControl | TessEvaluation),
        AllGraphics = (Vertex | AllTessellation | Geometry | Fragment),
        All = (AllGraphics | Compute),
    };

    using ShaderStageFlags = Flags<ShaderStage>;
    ALIMER_FORCE_INLINE ShaderStageFlags operator|(ShaderStage bit0, ShaderStage bit1)
    {
        return ShaderStageFlags(bit0) | bit1;
    }

    ALIMER_FORCE_INLINE ShaderStageFlags operator~(ShaderStage bits)
    {
        return ~(ShaderStageFlags(bits));
    }

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
    class Shader : public Resource, public GpuResource
    {
        ALIMER_OBJECT(Shader, Resource);

    protected:
        /// Constructor.
        Shader(Graphics* graphics, const void *pCode, size_t codeSize);

        /// Constructor.
        Shader(Graphics* graphics,
            const void *pVertexCode, size_t vertexCodeSize,
            const void *pFragmentCode, size_t fragmentCodeSize);

    public:
        /// Destructor.
        virtual ~Shader();

        bool IsCompute() const { return _isCompute; }
        const ResourceLayout& GetResourceLayout() const { return _layout; }

    protected:
        void Reflect(ShaderStage stage, const void *pCode, size_t codeSize);

        bool _isCompute;
        ResourceLayout _layout;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Shader);
    };
}
