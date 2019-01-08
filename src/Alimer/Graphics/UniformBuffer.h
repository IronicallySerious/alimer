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

#include "../Graphics/Buffer.h"

namespace alimer
{
    enum class UniformType
    {
        Float,
        Float2,
        Float3,
        Float4,
        Matrix3x3,
        Matrix3x4,
        Matrix4x4
    };

    struct Uniform
    {
        /// Data type of constant.
        UniformType type;
        /// Name of constant.
        String name;
        /// Number of elements. Default 1.
        uint32_t numElements = 1;
        /// Element size. Filled by UniformBuffer.
        uint32_t elementSize = 0;
        /// Offset from the beginning of the buffer. Filled by UniformBuffer.
        uint32_t offset = 0;
    };

    /// Defines a uniform buffer.
    class UniformBuffer final : public Buffer
    {
    public:
        /// Constructor.
        UniformBuffer();

        bool Define(const PODVector<Uniform>& uniforms);
        bool Define(const Uniform* uniforms, uint32_t count);

        /// Return number of indices.
        const PODVector<Uniform>& GetUniforms() const { return _uniforms; }

    private:
        /// Number of indices.
        PODVector<Uniform> _uniforms;
        /// Type of index.
        IndexType _indexType = IndexType::UInt16;
    };

    ALIMER_API uint32_t GetUniformTypeSize(UniformType type);
}
