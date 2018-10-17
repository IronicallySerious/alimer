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
    const char* EnumToString(IndexType type)
    {
#define CASE_STRING(ENUM_VALUE) case IndexType::##ENUM_VALUE : return #ENUM_VALUE

        switch (type)
        {
            CASE_STRING(UInt16);
            CASE_STRING(UInt32);
        }

#undef CASE_STRING
        return nullptr;
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
