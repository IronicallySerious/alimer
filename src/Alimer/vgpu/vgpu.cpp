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

#include "vgpu.h"
#include <assert.h>

VgpuBackend vgpuGetBackend()
{
#if defined(VGPU_D3D12) || defined(ALIMER_D3D12)
    return VGPU_BACKEND_D3D12;
#elif defined(VGPU_D3D11) || defined(ALIMER_D3D11)
    return VGPU_BACKEND_D3D11;
#elif defined(VGPU_VK) || defined(ALIMER_VULKAN)
    return VGPU_BACKEND_VULKAN;
#elif defined(VGPU_GL) || defined(ALIMER_OPENGL)
    return VGPU_BACKEND_OPENGL;
#else
    return VGPU_BACKEND_INVALID;
#endif
}

/* Pixel Format */

typedef struct VgpuPixelFormatDesc
{
    VgpuPixelFormat         format;
    const char*             name;
    VgpuPixelFormatType     type;
    uint8_t                 bitsPerPixel;
    struct
    {
        uint8_t             blockWidth;
        uint8_t             blockHeight;
        uint8_t             blockSize;
        uint8_t             minBlockX;
        uint8_t             minBlockY;
    } compression;
    
    struct
    {
        uint8_t             depth;
        uint8_t             stencil;
        uint8_t             red;
        uint8_t             green;
        uint8_t             blue;
        uint8_t             alpha;
    } bits;
} VgpuPixelFormatDesc;

const VgpuPixelFormatDesc FormatDesc[] =
{
    // format                               name,               type                                bpp         compression             bits
    { VGPU_PIXEL_FORMAT_UNKNOWN,            "Unknown",          VGPU_PIXEL_FORMAT_TYPE_UNKNOWN,     0,          {0, 0, 0, 0, 0},        {0, 0, 0, 0, 0, 0}},
    // 8-bit pixel formats
    { VGPU_PIXEL_FORMAT_A8_UNORM,           "A8",               VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {1, 1, 1, 1, 1},        {0, 0, 0, 0, 0, 8}},
    { VGPU_PIXEL_FORMAT_R8_UNORM,           "R8",               VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R8_SNORM,           "R8S",              VGPU_PIXEL_FORMAT_TYPE_SNORM,       8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R8_UINT,            "R8U",              VGPU_PIXEL_FORMAT_TYPE_UINT,        8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R8_SINT,            "R8I",              VGPU_PIXEL_FORMAT_TYPE_SINT,        8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},

    // 16-bit pixel formats
    { VGPU_PIXEL_FORMAT_R16_UNORM,          "R16",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R16_SNORM,          "R16S",             VGPU_PIXEL_FORMAT_TYPE_SNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R16_UINT,           "R16U",             VGPU_PIXEL_FORMAT_TYPE_UINT,        16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R16_SINT,           "R16I",             VGPU_PIXEL_FORMAT_TYPE_SINT,        16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R16_FLOAT,          "R16F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG8_UNORM,          "RG8",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG8_SNORM,          "RG8S",             VGPU_PIXEL_FORMAT_TYPE_SNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG8_UINT,           "RG8U",             VGPU_PIXEL_FORMAT_TYPE_UINT,        16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG8_SINT,           "RG8I",             VGPU_PIXEL_FORMAT_TYPE_SINT,        16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},

    // Packed 16-bit pixel formats
    { VGPU_PIXEL_FORMAT_R5G6B5_UNORM,       "R5G6B5",           VGPU_PIXEL_FORMAT_TYPE_UNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 5, 6, 5, 0}},
    { VGPU_PIXEL_FORMAT_RGBA4_UNORM,        "RGBA4",            VGPU_PIXEL_FORMAT_TYPE_UNORM,       16,         {1, 1, 2, 1, 1},        {0, 0, 4, 4, 4, 4}},

    // 32-bit pixel formats
    { VGPU_PIXEL_FORMAT_R32_UINT,           "R32U",             VGPU_PIXEL_FORMAT_TYPE_UINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R32_SINT,           "R32I",             VGPU_PIXEL_FORMAT_TYPE_SINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_R32_FLOAT,          "R32F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG16_UNORM,         "RG16",             VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG16_SNORM,         "RG16S",            VGPU_PIXEL_FORMAT_TYPE_SNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG16_UINT,          "RG16U",            VGPU_PIXEL_FORMAT_TYPE_UINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG16_SINT,          "RG16I",            VGPU_PIXEL_FORMAT_TYPE_SINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG16_FLOAT,         "RG16F",            VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
    { VGPU_PIXEL_FORMAT_RGBA8_UNORM,        "RGBA8",            VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
    { VGPU_PIXEL_FORMAT_RGBA8_SNORM,        "RGBA8S",           VGPU_PIXEL_FORMAT_TYPE_SNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
    { VGPU_PIXEL_FORMAT_RGBA8_UINT,         "RGBA8U",           VGPU_PIXEL_FORMAT_TYPE_UINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
    { VGPU_PIXEL_FORMAT_RGBA8_SINT,         "RGBA8I",           VGPU_PIXEL_FORMAT_TYPE_SINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
    { VGPU_PIXEL_FORMAT_BGRA8_UNORM,        "BGRA8",            VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},

    // Packed 32-Bit Pixel formats
    { VGPU_PIXEL_FORMAT_RGB10A2_UNORM,      "RGB10A2",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {0, 0, 10, 10, 10, 2}},
    { VGPU_PIXEL_FORMAT_RGB10A2_UINT,       "RGB10A2U",         VGPU_PIXEL_FORMAT_TYPE_UINT,        32,         {1, 1, 4, 1, 1},        {0, 0, 10, 10, 10, 2}},
    { VGPU_PIXEL_FORMAT_RG11B10_FLOAT,      "RG11B10F",         VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {0, 0, 11, 11, 10, 0}},
    { VGPU_PIXEL_FORMAT_RGB9E5_FLOAT,       "RGB9E5F",          VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {0, 0, 9, 9, 9, 5}},

    // 64-Bit Pixel Formats
    { VGPU_PIXEL_FORMAT_RG32_UINT,          "RG32U",            VGPU_PIXEL_FORMAT_TYPE_UINT,        64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG32_SINT,          "RG3I",             VGPU_PIXEL_FORMAT_TYPE_SINT,        64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
    { VGPU_PIXEL_FORMAT_RG32_FLOAT,         "RG3F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
    { VGPU_PIXEL_FORMAT_RGBA16_UNORM,       "RGBA16",           VGPU_PIXEL_FORMAT_TYPE_UNORM,       64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
    { VGPU_PIXEL_FORMAT_RGBA16_SNORM,       "RGBA16S",          VGPU_PIXEL_FORMAT_TYPE_SNORM,       64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
    { VGPU_PIXEL_FORMAT_RGBA16_UINT,        "RGBA16U",          VGPU_PIXEL_FORMAT_TYPE_UINT,        64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
    { VGPU_PIXEL_FORMAT_RGBA16_SINT,        "RGBA16S",          VGPU_PIXEL_FORMAT_TYPE_SINT,        64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
    { VGPU_PIXEL_FORMAT_RGBA16_FLOAT,       "RGBA16F",          VGPU_PIXEL_FORMAT_TYPE_FLOAT,       64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},

    // 128-Bit Pixel Formats
    { VGPU_PIXEL_FORMAT_RGBA32_UINT,        "RGBA32U",          VGPU_PIXEL_FORMAT_TYPE_UINT,        128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},
    { VGPU_PIXEL_FORMAT_RGBA32_SINT,        "RGBA32S",          VGPU_PIXEL_FORMAT_TYPE_SINT,        128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},
    { VGPU_PIXEL_FORMAT_RGBA32_FLOAT,       "RGBA32F",          VGPU_PIXEL_FORMAT_TYPE_FLOAT,       128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},

    // Depth-stencil
    { VGPU_PIXEL_FORMAT_D16,                "D16",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       16,         {1, 1, 2, 1, 1},        {16, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D24,                "D24",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       24,         {1, 1, 3, 1, 1},        {24, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D24S8,              "D24S8",            VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {24, 8, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D32,                "D32",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       32,         {1, 1, 4, 1, 1},        {32, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D16F,               "D16F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       16,         {1, 1, 2, 1, 1},        {16, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D24F,               "D24F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       24,         {1, 1, 3, 1, 1},        {24, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D32F,               "D32F",             VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {32, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D32FS8,             "D32FS8",           VGPU_PIXEL_FORMAT_TYPE_FLOAT,       32,         {1, 1, 4, 1, 1},        {32, 8, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_D0S8,               "D0S8",             VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {1, 1, 1, 1, 1},        {0, 8, 0, 0, 0, 0}},

    // Compressed formats
    { VGPU_PIXEL_FORMAT_BC1_UNORM,          "BC1",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC2_UNORM,          "BC2",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC3_UNORM,          "BC3",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC4_UNORM,          "BC4",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC4_SNORM,          "BC4S",             VGPU_PIXEL_FORMAT_TYPE_SNORM,       4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC5_UNORM,          "BC5",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC5_SNORM,          "BC5S",             VGPU_PIXEL_FORMAT_TYPE_SNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC6HS16,            "BC6HS16",          VGPU_PIXEL_FORMAT_TYPE_FLOAT,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC6HU16,            "BC6HU16",          VGPU_PIXEL_FORMAT_TYPE_FLOAT,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_BC7_UNORM,          "BC7",              VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},

    // Compressed PVRTC Pixel Formats
    { VGPU_PIXEL_FORMAT_PVRTC_RGB2,         "PVRTC_RGB2",       VGPU_PIXEL_FORMAT_TYPE_UNORM,       2,          {8, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_PVRTC_RGBA2,        "PVRTC_RGBA2",      VGPU_PIXEL_FORMAT_TYPE_UNORM,       2,          {8, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_PVRTC_RGB4,         "PVRTC_RGB4",       VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_PVRTC_RGBA4,        "PVRTC_RGBA4",      VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},

    // Compressed ETC Pixel Formats
    { VGPU_PIXEL_FORMAT_ETC2_RGB8,          "ETC2_RGB8",        VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_ETC2_RGB8A1,        "ETC2_RGB8A1",      VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},

    // Compressed ASTC Pixel Formats
    { VGPU_PIXEL_FORMAT_ASTC4x4,            "ASTC4x4",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_ASTC5x5,            "ASTC5x5",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       6,          {5, 5, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_ASTC6x6,            "ASTC6x6",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {6, 6, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
    { VGPU_PIXEL_FORMAT_ASTC8x5,            "ASTC8x5",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       4,          {8, 5, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
    { VGPU_PIXEL_FORMAT_ASTC8x6,            "ASTC8x6",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       3,          {8, 6, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
    { VGPU_PIXEL_FORMAT_ASTC8x8,            "ASTC8x8",          VGPU_PIXEL_FORMAT_TYPE_UNORM,       3,          {8, 8, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
    { VGPU_PIXEL_FORMAT_ASTC10x10,          "ASTC10x10",        VGPU_PIXEL_FORMAT_TYPE_UNORM,       3,          {10, 10, 16, 1, 1},     {0, 0, 0, 0, 0, 0} },
    { VGPU_PIXEL_FORMAT_ASTC12x12,          "ASTC12x12",        VGPU_PIXEL_FORMAT_TYPE_UNORM,       3,          {12, 12, 16, 1, 1},     {0, 0, 0, 0, 0, 0} },

};

uint32_t vgpuGetFormatBitsPerPixel(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].bitsPerPixel;
}

uint32_t vgpuGetFormatBlockSize(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].compression.blockSize;
}

uint32_t vgpuGetFormatBlockWidth(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].compression.blockWidth;
}

uint32_t vgpuGetFormatBlockHeight(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].compression.blockHeight;
}

VgpuPixelFormatType vgpuGetFormatType(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].type;
}

VgpuBool32 vgpuIsDepthFormat(VgpuPixelFormat format)
{
    assert(FormatDesc[format].format == format);
    return FormatDesc[format].bits.depth > 0;
}

VgpuBool32 vgpuIsStencilFormat(VgpuPixelFormat format)
{
    assert(FormatDesc[format].format == format);
    return FormatDesc[format].bits.stencil > 0;
}

VgpuBool32 vgpuIsDepthStencilFormat(VgpuPixelFormat format)
{
    return vgpuIsDepthFormat(format) || vgpuIsStencilFormat(format);
}

VgpuBool32 vgpuIsCompressedFormat(VgpuPixelFormat format)
{
    assert(FormatDesc[format].format == format);
    return format >= VGPU_PIXEL_FORMAT_BC1_UNORM && format <= VGPU_PIXEL_FORMAT_PVRTC_RGBA4;
}

const char* vgpuGetFormatName(VgpuPixelFormat format)
{
    assert(FormatDesc[(uint32_t)format].format == format);
    return FormatDesc[(uint32_t)format].name;
}

#ifdef TODO_D3D12
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "../Core/Log.h"
#include <vector>

static AGpuRendererI* s_renderer = nullptr;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
#endif

static void agpuBufferInitialize(AgpuBuffer buffer, const AgpuBufferDescriptor* descriptor)
{
    if (buffer != nullptr)
    {
        // Set properties
        uint64_t size = descriptor->size;
        //size = alimer::AlignTo(size, uint64_t(descriptor->stride));

        buffer->usage = descriptor->usage;
        buffer->size = size;
        buffer->stride = descriptor->stride;

#if defined(ALIMER_DEV)
        const char* usageStr = "";
        if (buffer->usage & AGPU_BUFFER_USAGE_VERTEX)
        {
            usageStr = "vertex";
        }
        else if (buffer->usage & AGPU_BUFFER_USAGE_INDEX)
        {
            usageStr = "index";
        }
        else if (buffer->usage & AGPU_BUFFER_USAGE_UNIFORM)
        {
            usageStr = "uniform";
        }

        ALIMER_LOGDEBUG("Created {} buffer [size: {}, stride: {}]",
            usageStr,
            buffer->size,
            buffer->stride
        );
#endif
    }
    else
    {
        ALIMER_LOGERROR("Failed to create buffer");
    }
}

AgpuBuffer agpuCreateBuffer(const AgpuBufferDescriptor* descriptor, const void* initialData)
{
    AgpuBuffer buffer = s_renderer->CreateBuffer(descriptor, initialData, NULL);
    agpuBufferInitialize(buffer, descriptor);
    return buffer;
}

AgpuBuffer agpuCreateExternalBuffer(const AgpuBufferDescriptor* descriptor, void* handle)
{
    AgpuBuffer buffer = s_renderer->CreateBuffer(descriptor, NULL, handle);
    agpuBufferInitialize(buffer, descriptor);
    return buffer;
}

void agpuDestroyBuffer(AgpuBuffer buffer)
{
    s_renderer->DestroyBuffer(buffer);
}

static void agpuTextureInitialize(AgpuTexture texture, const AgpuTextureDescriptor* descriptor)
{
    if (texture != nullptr)
    {
        // Set properties
        texture->width = descriptor->width;
        texture->height = descriptor->height;
        texture->depthOrArraySize = descriptor->depthOrArraySize;
        texture->mipLevels = descriptor->mipLevels;
    }
}

AgpuTexture agpuCreateTexture(const AgpuTextureDescriptor* descriptor)
{
    AgpuTexture texture = s_renderer->CreateTexture(descriptor, NULL);
    agpuTextureInitialize(texture, descriptor);
    return texture;
}

AgpuTexture agpuCreateExternalTexture(const AgpuTextureDescriptor* descriptor, void* handle)
{
    AgpuTexture texture = s_renderer->CreateTexture(descriptor, handle);
    agpuTextureInitialize(texture, descriptor);
    return texture;
}

void agpuDestroyTexture(AgpuTexture texture)
{
    s_renderer->DestroyTexture(texture);
}

AgpuFramebuffer agpuCreateFramebuffer(const AgpuFramebufferDescriptor* descriptor)
{
    return s_renderer->CreateFramebuffer(descriptor);
}

void agpuDestroyFramebuffer(AgpuFramebuffer framebuffer)
{
    s_renderer->DestroyFramebuffer(framebuffer);
}

AgpuShaderModule agpuCreateShaderModule(const AgpuShaderModuleDescriptor* descriptor)
{
    return s_renderer->CreateShaderModule(descriptor);
}

void agpuDestroyShaderModule(AgpuShaderModule shaderModule)
{
    s_renderer->DestroyShaderModule(shaderModule);
}

AgpuShader agpuCreateShader(const AgpuShaderDescriptor* descriptor)
{
    return s_renderer->CreateShader(descriptor);
}

void agpuDestroyShader(AgpuShader shader)
{
    s_renderer->DestroyShader(shader);
}

AgpuPipeline agpuCreateRenderPipeline(const AgpuRenderPipelineDescriptor* descriptor)
{
#if defined(ALIMER_DEV)
    ALIMER_ASSERT_MSG(descriptor, "Invalid render pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->shader, "Invalid shader.");
    //ALIMER_ASSERT_MSG(descriptor->fragment, "Invalid fragment shader.");

    for (int i = 0; i < AGPU_MAX_VERTEX_BUFFER_BINDINGS; i++)
    {
        const AgpuVertexBufferLayoutDescriptor* desc = &descriptor->vertexDescriptor.layouts[i];
        if (desc->stride == 0) {
            continue;
        }

        ALIMER_ASSERT_MSG((desc->stride & 3) == 0, "AgpuVertexDescriptor buffer stride must be multiple of 4");
    }

    ALIMER_ASSERT_MSG(descriptor->vertexDescriptor.attributes[0].format != AGPU_VERTEX_FORMAT_UNKNOWN, "AgpuVertexDescriptor attributes is empty or not continuous");
    bool attrsContinuous = true;

    for (int i = 0; i < AGPU_MAX_VERTEX_ATTRIBUTES; i++)
    {
        const AgpuVertexAttributeDescriptor* desc = &descriptor->vertexDescriptor.attributes[i];
        if (desc->format == AGPU_VERTEX_FORMAT_UNKNOWN) {
            attrsContinuous = false;
            continue;
        }
        ALIMER_ASSERT_MSG(attrsContinuous, "AgpuVertexDescriptor buffer stride must be multiple of 4");
        ALIMER_ASSERT(desc->bufferIndex < AGPU_MAX_VERTEX_BUFFER_BINDINGS);
    }
#endif

    return s_renderer->CreateRenderPipeline(descriptor);
}

AgpuPipeline agpuCreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor)
{
#if defined(ALIMER_DEV)
    ALIMER_ASSERT_MSG(descriptor, "Invalid compute pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->shader, "Invalid shader");
#endif

    return s_renderer->CreateComputePipeline(descriptor);
}

void agpuDestroyPipeline(AgpuPipeline pipeline)
{
    s_renderer->DestroyPipeline(pipeline);
}

void agpuBeginRenderPass(AgpuFramebuffer framebuffer)
{
    ALIMER_ASSERT_MSG(framebuffer, "Invalid framebuffer");
    s_renderer->CmdBeginRenderPass(framebuffer);
}

void agpuEndRenderPass()
{
    s_renderer->CmdEndRenderPass();
}

void agpuCmdSetShader(AgpuShader shader)
{
    ALIMER_ASSERT_MSG(shader, "Invalid shader");
    s_renderer->CmdSetShader(shader);
}

void agpuCmdSetVertexBuffer(uint32_t binding, AgpuBuffer buffer, uint64_t offset, AgpuVertexInputRate inputRate)
{
    ALIMER_ASSERT_MSG(buffer, "Invalid buffer");
    ALIMER_ASSERT(binding < AGPU_MAX_VERTEX_BUFFER_BINDINGS);
#ifdef ALIMER_DEV
    if ((buffer->usage & AGPU_BUFFER_USAGE_VERTEX) == 0)
    {
        //agpuNotifyValidationError("agpuCmdSetVertexBuffer need buffer with Vertex usage");
        return;
    }
#endif

    s_renderer->CmdSetVertexBuffer(binding, buffer, offset, buffer->stride, inputRate);
}

void agpuCmdSetIndexBuffer(AgpuBuffer buffer, uint64_t offset, AgpuIndexType indexType)
{
    ALIMER_ASSERT_MSG(buffer, "Invalid buffer");
#ifdef ALIMER_DEV
    if ((buffer->usage & AGPU_BUFFER_USAGE_INDEX) == 0)
    {
        //agpuNotifyValidationError("agpuCmdSetIndexBuffer need buffer with Index usage");
        return;
    }
#endif

    s_renderer->CmdSetIndexBuffer(buffer, offset, indexType);
}

void agpuCmdSetViewport(AgpuViewport viewport)
{
    s_renderer->CmdSetViewport(viewport);
}

void agpuCmdSetViewports(uint32_t viewportCount, const AgpuViewport* pViewports)
{
    s_renderer->CmdSetViewports(viewportCount, pViewports);
}

void agpuCmdSetScissor(AgpuRect2D scissor)
{
    s_renderer->CmdSetScissor(scissor);
}

void agpuCmdSetScissors(uint32_t scissorCount, const AgpuRect2D* pScissors)
{
    s_renderer->CmdSetScissors(scissorCount, pScissors);
}

void CmdSetPrimitiveTopology(AgpuPrimitiveTopology topology)
{
    s_renderer->CmdSetPrimitiveTopology(topology);
}

void agpuCmdDraw(uint32_t vertexCount, uint32_t firstVertex)
{
    s_renderer->CmdDraw(vertexCount, 1, firstVertex, 0);
}

void agpuCmdDrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset)
{
    ALIMER_ASSERT(indexCount > 1);
    s_renderer->CmdDrawIndexed(indexCount, 1, firstIndex, vertexOffset, 0);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif  
#endif // TODO_D3D12

