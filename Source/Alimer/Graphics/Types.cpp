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

#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDevice.h"

namespace Alimer
{
    const char* EnumToString(ResourceUsage usage)
    {

#define CASE_STRING(ENUM_VALUE) case ResourceUsage::##ENUM_VALUE : return #ENUM_VALUE

        switch (usage)
        {
            CASE_STRING(Default);
            CASE_STRING(Immutable);
            CASE_STRING(Dynamic);
            CASE_STRING(Staging);
        }

#undef CASE_STRING
        return nullptr;
    }

    const char* EnumToString(BufferUsage usage)
    {
        if (any(usage & BufferUsage::Vertex))
            return "Vertex";

        if (any(usage & BufferUsage::Index))
            return "Index";

        if (any(usage & BufferUsage::Uniform))
            return "Uniform";

        if (any(usage & BufferUsage::Storage))
            return "Storage";

        if (any(usage & BufferUsage::Indirect))
            return "Indirect";

        return "None";
    }

    const char* EnumToString(ShaderStage stage)
    {
#define CASE_STRING(ENUM_VALUE) case ShaderStage::##ENUM_VALUE : return #ENUM_VALUE

        switch (stage)
        {
            CASE_STRING(Vertex);
            CASE_STRING(TessControl);
            CASE_STRING(TessEvaluation);
            CASE_STRING(Geometry);
            CASE_STRING(Fragment);
            CASE_STRING(Compute);
        }

#undef CASE_STRING
        return nullptr;
    }

    const char* EnumToString(VertexElementSemantic semantic)
    {
        switch (semantic)
        {
        case VertexElementSemantic::Position:       return "POSITION";
        case VertexElementSemantic::Normal:         return "NORMAL";
        case VertexElementSemantic::Binormal:       return "BINORMAL";
        case VertexElementSemantic::Tangent:        return "TANGENT";
        case VertexElementSemantic::BlendWeight:    return "BLENDWEIGHT";
        case VertexElementSemantic::BlendIndices:   return "BLENDINDICES";
        case VertexElementSemantic::Color0:         return "COLOR0";
        case VertexElementSemantic::Color1:         return "COLOR1";
        case VertexElementSemantic::Color2:         return "COLOR2";
        case VertexElementSemantic::Color3:         return "COLOR3";
        case VertexElementSemantic::Texcoord0:      return "TEXCOORD0";
        case VertexElementSemantic::Texcoord1:      return "TEXCOORD1";
        case VertexElementSemantic::Texcoord2:      return "TEXCOORD2";
        case VertexElementSemantic::Texcoord3:      return "TEXCOORD3";
        case VertexElementSemantic::Texcoord4:      return "TEXCOORD4";
        case VertexElementSemantic::Texcoord5:      return "TEXCOORD5";
        case VertexElementSemantic::Texcoord6:      return "TEXCOORD6";
        case VertexElementSemantic::Texcoord7:      return "TEXCOORD7";
        default: return nullptr;
        }
    }

    uint32_t GetVertexFormatSize(VertexFormat format)
    {
        switch (format)
        {
        case VertexFormat::Float:
        case VertexFormat::Byte4:
        case VertexFormat::Byte4N:
        case VertexFormat::UByte4:
        case VertexFormat::UByte4N:
        case VertexFormat::Short2:
        case VertexFormat::Short2N:
            return 4;
        case VertexFormat::Float2:
        case VertexFormat::Short4:
        case VertexFormat::Short4N:
            return 8;
        case VertexFormat::Float3:
            return 12;
        case VertexFormat::Float4:
            return 16;

        default:
            return static_cast<uint32_t>(-1);
        }
    }
}
