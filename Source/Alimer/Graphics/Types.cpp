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
#include "../Graphics/Graphics.h"

namespace Alimer
{
    const char* EnumToString(GraphicsBackend backend)
    {
        switch (backend)
        {
        case GraphicsBackend::Default: 
            return "default";
        case GraphicsBackend::Empty: 
            return "empty";
        case GraphicsBackend::Vulkan: 
            return "vulkan";
        case GraphicsBackend::D3D11: 
            return "d3d11";
        case GraphicsBackend::D3D12: 
            return "d3d12";
        case GraphicsBackend::Metal: 
            return "metal";
        case GraphicsBackend::OpenGL: 
            return "opengl";
        default: 
            return nullptr;
        }
    }

    const char* EnumToString(ResourceUsage usage)
    {
        switch (usage)
        {
        case ResourceUsage::Default: return "default";
        case ResourceUsage::Immutable: return "immutable";
        case ResourceUsage::Dynamic: return "dynamic";
        case ResourceUsage::Staging: return "staging";
        default: return nullptr;
        }
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

    uint32_t GetVertexElementSize(VertexElementFormat format)
    {
        switch (format)
        {
        case VertexElementFormat::Float:
        case VertexElementFormat::Byte4:
        case VertexElementFormat::Byte4N:
        case VertexElementFormat::UByte4:
        case VertexElementFormat::UByte4N:
        case VertexElementFormat::Short2:
        case VertexElementFormat::Short2N:
            return 4;

        case VertexElementFormat::Float2:
        case VertexElementFormat::Short4:
        case VertexElementFormat::Short4N:
            return 8;

        case VertexElementFormat::Float3:
            return 12;
        case VertexElementFormat::Float4:
            return 16;

        default:
            ALIMER_UNREACHABLE();
        }
    }
}
