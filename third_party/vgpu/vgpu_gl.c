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

#if defined(VGPU_GL) || defined(VGPU_GLES) || defined(VGPU_WEBGL)
#include "vgpu.h"
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
#   include <malloc.h>
#   undef    alloca
#   define   alloca _malloca
#   define   freea  _freea
#else
#   include <alloca.h>
#endif

#define _VGPU_ALLOC(type)           ((type*) malloc(sizeof(type)))
#define _VGPU_ALLOCN(type, n)       ((type*) malloc(sizeof(type) * n))
#define _VGPU_FREE(ptr)             (free((void*)(ptr)))
#define _VGPU_ALLOC_HANDLE(type)    ((type) calloc(1, sizeof(type##_T)))

#ifndef _VGPU_ASSERT
#   include <assert.h>
#   define _VGPU_ASSERT(c) assert(c)
#endif

#ifndef _VGPU_UNREACHABLE
#   define _VGPU_UNREACHABLE _VGPU_ASSERT(false)
#endif

#if defined(VGPU_WEBGL)
#   include <GLES3/gl3.h>
#   include <GLES2/gl2ext.h>
#   include <GL/gl.h>
#   include <GL/glext.h>
#else 
#   include "glad.h"
#endif

#define _VGPU_CHECK_ERROR() { _VGPU_ASSERT(glGetError() == GL_NO_ERROR); }

#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif

#ifndef GL_ZERO_TO_ONE
#define GL_ZERO_TO_ONE 0x935F
#endif

#ifndef GL_MAX_SHADER_STORAGE_BLOCK_SIZE
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#endif

#ifndef GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#endif

#ifndef GL_PATCHES
#define GL_PATCHES 0x000E
#endif

/* GL only types */
#define _VGPU_GL_MAX_TEXTURES (16u)
//#define _VGPU_GL_SHADER_POSITION 0
//#define _VGPU_GL_SHADER_NORMAL 1
//#define _VGPU_GL_SHADER_TEX_COORD 2
//#define _VGPU_GL_SHADER_VERTEX_COLOR 3

typedef enum _VGpuGLBufferType {
    _VGPU_GL_BUFFER_VERTEX,
    _VGPU_GL_BUFFER_INDEX,
    _VGPU_GL_BUFFER_UNIFORM,
    _VGPU_GL_BUFFER_SHADER_STORAGE,
    _VGPU_GL_BUFFER_TYPE_COUNT
} _VGpuGLBufferType;

/* Handle declaration */
typedef struct VGpuTexture_T {
    VGpuTextureType         textureType;
    VGpuPixelFormat         pixelFormat;
    VGpuExtent3D            size;
    uint32_t                mipLevels;
    uint32_t                arrayLayers;
    VgpuSampleCount         samples;
    VGpuTextureUsageFlags   usage;
    bool                    external_handle;
    GLenum                  gl_target;
    GLuint                  gl_handle;
} VGpuTexture_T;

typedef struct VGpuBuffer_T {
    uint64_t            size;
    VGpuBufferUsage     usage;
    VGpuBufferUsage     resourceUsage;
    bool                external_handle;
    _VGpuGLBufferType   gl_type;
    GLenum              gl_target;
    GLuint              gl_handle;
    void*               gl_data;
} VGpuBuffer_T;

typedef struct VGpuShader_T {
    GLuint          gl_handle;
} VGpuShader_T;

typedef struct _VGpuGLVertexAttribute {
    int8_t vb_index;        /* -1 if attr is not enabled */
    int8_t divisor;         /* -1 if not initialized */
    uint8_t stride;
    uint8_t size;
    uint8_t normalized;
    int offset;
    GLenum type;
    bool integer;
} _VGpuGLVertexAttribute;

typedef struct VGpuPipeline_T {
    VGpuShader              shader;
    GLenum                  topology;
    bool                    vertex_layout_valid[VGPU_MAX_VERTEX_BUFFER_BINDINGS];
    _VGpuGLVertexAttribute  gl_attrs[VGPU_MAX_VERTEX_ATTRIBUTES];
} VGpuPipeline_T;

typedef struct _vgpu_gl_features {
    bool    independentBlend;
    bool    compute;
    bool    geometry;
    bool    tessellation;
    bool    singlepass;
    bool    texture3D;
    bool    texture2DArray;
    bool    textureCubeSeamless;
    bool    textureCubeArray;
    bool    anisotropic;
    bool    multiSampleTexture;
    bool    textureCompressionBC;
    bool    textureCompressionETC2;
    /* Pixel formats */
    bool    textureCompressionBC1RGB;
    bool    textureCompressionBC1RGBA;
    bool    textureCompressionBC2;
    bool    textureCompressionBC3;
    /* Extra features */
    bool    storageBuffers;
    bool    textureStorage;             /* glTexStorage2D = 4.2 or GL_ARB_texture_storage*/
    bool    textureStorageMultisample;  /* glTexStorage2DMultisample = 4.3 or GL_ARB_texture_storage_multisample*/
    bool    bufferStorage;              /* glBufferStorage = 4.4 or GL_ARB_buffer_storage*/
    bool    clipControl;
} _vgpu_gl_features;

typedef struct _vgpu_gl_cache {
    /* rasterizer state */
    uint32_t                primitiveRestart;

    /* depth stencil state*/
    VGpuCompareFunction     depthCompareFunction;
    bool                    depthWriteEnabled;
    bool                    stencilEnabled;

    /* blend state */
    bool                    alphaToCoverage;

    /* program */
    GLuint                  program;
    VGpuPipeline            currentPipeline;

    /* Texture */
    GLuint                  activeTexture;
    VGpuTexture             textures[_VGPU_GL_MAX_TEXTURES];

    /* Buffer */
    uint32_t                buffers[_VGPU_GL_BUFFER_TYPE_COUNT];
} _vgpu_gl_cache;

struct {
    bool                    initialized;
    uint32_t                frameIndex;
    _vgpu_gl_features       features;
    VGpuLimits              limits;
    _vgpu_gl_cache          state;
    bool                    srgb;
    GLsizei                 width;
    GLsizei                 height;
    const char*             vendor;
    const char*             renderer;
    const char*             version;
    const char*             glslVersion;
    GLint                   version_major;
    GLint                   version_minor;
    GLuint                  default_framebuffer;
    GLuint                  default_vao;
} _gl = { 0 };

extern void _vgpu_log(vgpu_log_type type, const char *message);

static int32_t _vgpuGLGetInt(GLenum param) {
    GLint attr = 0;
    glGetIntegerv(param, &attr);
    return attr;
}

static uint32_t _vgpuGLGetUInt(GLenum param) {
    return (uint32_t)_vgpuGLGetInt(param);
};

static float _vgpuGLGetFloat(GLenum param) {
    GLfloat attr = 0.0f;
    glGetFloatv(param, &attr);
    return attr;
}

void _vgpu_gl_check_extension(const char* ext)
{
    if (strstr(ext, "ARB_draw_buffers_blend"))
    {
        _gl.features.independentBlend = true;
    }
    else if (strstr(ext, "EXT_texture_compression_s3tc")
        || strstr(ext, "MOZ_WEBGL_compressed_texture_s3tc")
        || strstr(ext, "WEBGL_compressed_texture_s3tc")
        || strstr(ext, "WEBKIT_WEBGL_compressed_texture_s3tc"))
    {
        _gl.features.textureCompressionBC = true;
        return;
    }
    else if (strstr(ext, "ARB_seamless_cube_map"))
    {
        _gl.features.textureCubeSeamless = true;
    }
    else if (strstr(ext, "ARB_texture_cube_map_array")
        || strstr(ext, "EXT_texture_cube_map_array"))
    {
        _gl.features.textureCubeArray = true;
    }
    else if (strstr(ext, "EXT_texture_filter_anisotropic"))
    {
        _gl.features.anisotropic = true;
    }
    else if (strstr(ext, "GL_ARB_clip_control"))
    {
        _gl.features.clipControl = true;
    }
    else if (strstr(ext, "ARB_texture_multisample")
        || strstr(ext, "ANGLE_framebuffer_multisample"))
    {
        _gl.features.multiSampleTexture = true;
    }
    else if (strstr(ext, "ARB_shader_storage_buffer_object"))
    {
        _gl.features.storageBuffers = true;
    }
    else if (strstr(ext, "ARB_compute_shader"))
    {
        _gl.features.compute = true;
    }
    else if (strstr(ext, "ARB_tessellation_shader"))
    {
        _gl.features.tessellation = true;
    }
    else if (strstr(ext, "ARB_geometry_shader4"))
    {
        _gl.features.geometry = true;
    }
    else if (strstr(ext, "ARB_texture_storage"))
    {
        _gl.features.textureStorage = true;
    }
    else if (strstr(ext, "ARB_texture_storage_multisample"))
    {
        _gl.features.textureStorageMultisample = true;
    }
    else if (strstr(ext, "ARB_buffer_storage"))
    {
        _gl.features.bufferStorage = true;
    }
}

static GLenum _vgpuGLConvertResourceUsage(VGpuResourceUsage usage) {
    switch (usage) {
    case VGPU_RESOURCE_USAGE_STATIC:
    case VGPU_RESOURCE_USAGE_IMMUTABLE:
        return GL_STATIC_DRAW;

    case VGPU_RESOURCE_USAGE_DYNAMIC:
        return GL_DYNAMIC_DRAW;

    case VGPU_RESOURCE_USAGE_STREAM:
        return GL_STREAM_DRAW;
    default: _VGPU_UNREACHABLE; return 0;
    }
}

static GLenum _vgpuConvertCompareFunction(VGpuCompareFunction func)
{
    switch (func)
    {
    case VGPU_COMPARE_FUNCTION_NEVER:
        return GL_NEVER;
    case VGPU_COMPARE_FUNCTION_LESS:
        return GL_LESS;
    case VGPU_COMPARE_FUNCTION_EQUAL:
        return GL_EQUAL;
    case VGPU_COMPARE_FUNCTION_LESS_EQUAL:
        return GL_LEQUAL;
    case VGPU_COMPARE_FUNCTION_GREATER:
        return GL_GREATER;
    case VGPU_COMPARE_FUNCTION_NOT_EQUAL:
        return GL_NOTEQUAL;
    case VGPU_COMPARE_FUNCTION_GREATER_EQUAL:
        return GL_GEQUAL;
    case VGPU_COMPARE_FUNCTION_ALWAYS:
        return GL_ALWAYS;
    }

    return GL_ALWAYS;
}

static GLenum _vgpuGLConvertTextureType(VGpuTextureType type, bool is_array) {
    switch (type)
    {
    case VGPU_TEXTURE_TYPE_2D:
        return is_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case VGPU_TEXTURE_TYPE_3D:
        return GL_TEXTURE_3D;
    case VGPU_TEXTURE_TYPE_CUBE:
    {
        if (is_array
            && _gl.features.textureCubeArray) {
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        }

        return GL_TEXTURE_CUBE_MAP;
    }
    default: _VGPU_UNREACHABLE; return 0;
    }
}

static _VGpuGLBufferType _vgpuGLConvertBufferUsage(VGpuBufferUsage usage) {
    if (usage & VGPU_BUFFER_USAGE_UNIFORM) {
        return _VGPU_GL_BUFFER_UNIFORM;
    }

    if ((usage & VGPU_BUFFER_USAGE_STORAGE_READ)
        || (usage & VGPU_BUFFER_USAGE_STORAGE_READ)) {
        return _VGPU_GL_BUFFER_SHADER_STORAGE;
    }

    if (usage & VGPU_BUFFER_USAGE_VERTEX) {
        return _VGPU_GL_BUFFER_VERTEX;
    }

    if (usage & VGPU_BUFFER_USAGE_INDEX) {
        return _VGPU_GL_BUFFER_UNIFORM;
    }

    return 0;
}

static GLenum _vgpuGLConvertBufferType(_VGpuGLBufferType type) {
    switch (type) {
    case _VGPU_GL_BUFFER_VERTEX: return GL_ARRAY_BUFFER;
    case _VGPU_GL_BUFFER_INDEX: return GL_ELEMENT_ARRAY_BUFFER;
    case _VGPU_GL_BUFFER_UNIFORM: return GL_UNIFORM_BUFFER;
    case _VGPU_GL_BUFFER_SHADER_STORAGE: return GL_SHADER_STORAGE_BUFFER;
        //case _VGPU_GL_BUFFER_GENERIC: return GL_COPY_WRITE_BUFFER;
    default: _VGPU_UNREACHABLE; return 0;
    }
}

static GLenum _vgpuGLConvertPrimitiveTopology(VGpuPrimitiveTopology topology) {
    switch (topology) {
    case VGPU_PRIMITIVE_TOPOLOGY_POINT_LIST: return GL_POINTS;
    case VGPU_PRIMITIVE_TOPOLOGY_LINE_LIST: return GL_LINES;
    case VGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP: return GL_LINE_STRIP;
    case VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return GL_TRIANGLES;
    case VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case VGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST: return GL_PATCHES;
    default: _VGPU_UNREACHABLE; return 0;
    }
}

static uint8_t _vgpuGLGetVertexFormatSize(VGpuVertexFormat format) {
    switch (format) {
    case VGPU_VERTEX_FORMAT_FLOAT:     return 1;
    case VGPU_VERTEX_FORMAT_FLOAT2:    return 2;
    case VGPU_VERTEX_FORMAT_FLOAT3:    return 3;
    case VGPU_VERTEX_FORMAT_FLOAT4:    return 4;
    case VGPU_VERTEX_FORMAT_BYTE4:     return 4;
    case VGPU_VERTEX_FORMAT_BYTE4N:    return 4;
    case VGPU_VERTEX_FORMAT_UBYTE4:    return 4;
    case VGPU_VERTEX_FORMAT_UBYTE4N:   return 4;
    case VGPU_VERTEX_FORMAT_SHORT2:    return 2;
    case VGPU_VERTEX_FORMAT_SHORT2N:   return 2;
    case VGPU_VERTEX_FORMAT_SHORT4:    return 4;
    case VGPU_VERTEX_FORMAT_SHORT4N:   return 4;
    case VGPU_VERTEX_FORMAT_UINT10_N2: return 4;
    default: _VGPU_UNREACHABLE; return 0;
    }
}

static GLenum _vgpuGLGetVertexFormatType(VGpuVertexFormat format) {
    switch (format) {
    case VGPU_VERTEX_FORMAT_FLOAT:
    case VGPU_VERTEX_FORMAT_FLOAT2:
    case VGPU_VERTEX_FORMAT_FLOAT3:
    case VGPU_VERTEX_FORMAT_FLOAT4:
        return GL_FLOAT;
    case VGPU_VERTEX_FORMAT_BYTE4:
    case VGPU_VERTEX_FORMAT_BYTE4N:
        return GL_BYTE;
    case VGPU_VERTEX_FORMAT_UBYTE4:
    case VGPU_VERTEX_FORMAT_UBYTE4N:
        return GL_UNSIGNED_BYTE;
    case VGPU_VERTEX_FORMAT_SHORT2:
    case VGPU_VERTEX_FORMAT_SHORT2N:
    case VGPU_VERTEX_FORMAT_SHORT4:
    case VGPU_VERTEX_FORMAT_SHORT4N:
        return GL_SHORT;
    case VGPU_VERTEX_FORMAT_UINT10_N2:
        return GL_UNSIGNED_INT_2_10_10_10_REV;
    default:
        _VGPU_UNREACHABLE; return 0;
    }
}

static GLboolean _vgpuGLConvertVertexFormatNormalized(VGpuVertexFormat format) {
    switch (format) {
    case VGPU_VERTEX_FORMAT_BYTE4:
    case VGPU_VERTEX_FORMAT_UBYTE4:
    case VGPU_VERTEX_FORMAT_SHORT2N:
    case VGPU_VERTEX_FORMAT_SHORT4N:
    case VGPU_VERTEX_FORMAT_UINT10_N2:
        return GL_TRUE;
    default:
        return GL_FALSE;
    }
}

void _vgpu_gl_reset_state_cache()
{
    _gl.state.currentPipeline = 0;
    _gl.state.program = 0;
    glUseProgram(0);
    _VGPU_CHECK_ERROR();

    for (uint32_t i = 0; i < _VGPU_GL_BUFFER_TYPE_COUNT; ++i) {
        _gl.state.buffers[i] = 0;
        glBindBuffer(_vgpuGLConvertBufferType(i), 0);
    }
    _VGPU_CHECK_ERROR();

    for (uint32_t i = 0; i < VGPU_MAX_VERTEX_ATTRIBUTES; i++) {
        glDisableVertexAttribArray(i);
        _VGPU_CHECK_ERROR();
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifndef VGPU_WEBGL
    //glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_PROGRAM_POINT_SIZE);
    if (_gl.srgb) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
#endif

    /* depth-stencil state */
    _gl.state.depthWriteEnabled = true;
    _gl.state.depthCompareFunction = VGPU_COMPARE_FUNCTION_LESS_EQUAL;
    if (_gl.state.depthCompareFunction == VGPU_COMPARE_FUNCTION_ALWAYS
        && !_gl.state.depthWriteEnabled)
    {
        glDisable(GL_DEPTH_TEST);

    }
    else
    {
        glEnable(GL_DEPTH_TEST);
    }

    if (_gl.state.depthWriteEnabled) {
        glDepthMask(GL_TRUE);
    }
    else {
        glDepthMask(GL_FALSE);
    }

    glDepthFunc(_vgpuConvertCompareFunction(_gl.state.depthCompareFunction));

    _gl.state.stencilEnabled = false;
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0);

    /* blend state */
    glDisable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
    _gl.state.alphaToCoverage = false;
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);


    /* rasterizer state */
    glPolygonOffset(0.0f, 0.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    //glLineWidth(1.0f);
#ifdef VGPU_GLES
    //glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#elif defined(VGPU_GL)
    //glEnable(GL_PRIMITIVE_RESTART);
    //_gl.state.primitiveRestart = 0xffffffff;
    //glPrimitiveRestartIndex(_gl.state.primitiveRestart);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

static void _vgpuGLBindTexture(VGpuTexture texture, uint32_t slot)
{
    _VGPU_ASSERT(slot >= 0 && slot < _VGPU_GL_MAX_TEXTURES);
    //texture = texture ? texture : _vgpuGLGetDefaultTexture();

    if (texture != _gl.state.textures[slot]) {
        _gl.state.textures[slot] = texture;
        if (_gl.state.activeTexture != slot) {
            glActiveTexture(GL_TEXTURE0 + slot);
            _gl.state.activeTexture = slot;
        }
        glBindTexture(texture->gl_target, texture->gl_handle);
    }
    _VGPU_CHECK_ERROR();
}


static void _vgpuGLBindBuffer(VGpuBuffer buffer) {
    if (_gl.state.buffers[buffer->gl_type] != buffer->gl_handle) {
        _gl.state.buffers[buffer->gl_type] = buffer->gl_handle;
        glBindBuffer(buffer->gl_target, buffer->gl_handle);
        _VGPU_CHECK_ERROR();
    }
}

static void _vgpuGLUseProgram(uint32_t program) {
    if (_gl.state.program != program) {
        _gl.state.program = program;
        glUseProgram(program);
        _VGPU_CHECK_ERROR();
        /* TODO: Increate shader switches stats */
    }
}

static void _vgpuGLSetupTexture(VGpuTexture texture, const VGpuTextureDescriptor* descriptor)
{
    texture->textureType = descriptor->textureType;
    texture->pixelFormat = descriptor->pixelFormat;
    texture->size = descriptor->size;
    texture->mipLevels = descriptor->mipLevels;
    texture->arrayLayers = descriptor->arrayLayers;
    texture->samples = descriptor->samples;
    texture->usage = descriptor->usage;
    texture->gl_target = _vgpuGLConvertTextureType(descriptor->textureType, descriptor->arrayLayers > 1);
}

VGpuBackend vgpuGetBackend() {
    return VGPU_BACKEND_OPENGL;
}

bool vgpuInitialize(const char* appName, const VGpuRendererSettings* settings)
{
    if (_gl.initialized) {
        _vgpu_log(vgpu_log_type_error, "vgpu already initialized");
        return true;
    }

    _gl.vendor = (const char*)glGetString(GL_VENDOR);
    _gl.renderer = (const char*)glGetString(GL_RENDERER);
    _gl.version = (const char*)glGetString(GL_VERSION);
    _gl.glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    glGetIntegerv(GL_MAJOR_VERSION, &_gl.version_major);

    if (glGetError() == GL_INVALID_ENUM)
    {
        char *parse_version = (char *)glGetString(GL_VERSION);
        _gl.version_minor = atoi(parse_version);
        parse_version = strchr(parse_version, '.') + 1;
        _gl.version_minor = atoi(parse_version);
    }
    else
    {
        glGetIntegerv(GL_MINOR_VERSION, &_gl.version_minor);
    }

#if defined(VGPU_GLES) || defined(VGPU_WEBGL)
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
#else
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (GLint index = 0; index < numExtensions; ++index)
    {
        const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, index);
        _vgpu_gl_check_extension(ext);
    }
#endif

    GLint numCompressedTexFormats = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTexFormats);
    GLint* compressedTexFormats = (GLint*)alloca(sizeof(GLint)*numCompressedTexFormats);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTexFormats);

    for (GLint i = 0; i < numCompressedTexFormats; ++i)
    {
        GLint format = compressedTexFormats[i];
        switch (format)
        {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
            _gl.features.textureCompressionBC1RGB = true;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            _gl.features.textureCompressionBC1RGBA = true;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            _gl.features.textureCompressionBC2 = true;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            _gl.features.textureCompressionBC3 = true;
            break;

        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            _gl.features.textureCompressionETC2 = true;
            break;
        default:
            break;
        }
    }

#ifndef VGPU_WEBGL
    _gl.features.singlepass = GLAD_GL_ARB_viewport_array && GLAD_GL_AMD_vertex_shader_viewport_index && GLAD_GL_ARB_fragment_layer_viewport;
    // Core since version 3.0
    _gl.features.texture3D = true;
    _gl.features.texture2DArray = true;

    // Core since 3.2+
    if (_gl.version_major >= 3
        && _gl.version_minor >= 2)
    {
        _gl.features.geometry = true;
        _gl.features.multiSampleTexture = true;
        _gl.features.textureCubeSeamless = true;
    }

    // Version 4.0+
    if (_gl.version_major >= 4)
    {
        _gl.features.independentBlend = true;
        _gl.features.textureCubeArray = true;
        // https://www.khronos.org/opengl/wiki/Tessellation
        _gl.features.tessellation = true;

        // Core in version 4.2+
        if (_gl.version_minor >= 2)
        {
            _gl.features.textureStorage = true;
        }

        // Core in version 4.3+
        if (_gl.version_minor >= 3)
        {
            // https://www.khronos.org/opengl/wiki/Compute_Shader
            _gl.features.compute = true;
            _gl.features.storageBuffers = true;
            _gl.features.textureStorageMultisample = true;
        }

        // Core in version 4.4+
        if (_gl.version_minor >= 4)
        {
            _gl.features.bufferStorage = true;
        }

        // Core in version 4.5+
        if (_gl.version_minor >= 5)
        {
            _gl.features.clipControl = true;
        }
    }
#endif

#if defined(VGPU_GLES) || defined(VGPU_WEBGL)
    // WebGL 2.0 or GLES 3.0.
    _gl.features.texture3D = _gl.features.texture2DArray = _gl.version_major >= 3;

    // GLES 3.1
    _gl.features.compute = _gl.version_major >= 3 && _gl.version_minor >= 1;
    _gl.features.storageBuffers = _gl.version_major >= 3 && _gl.version_minor >= 1;
#else
    if (_gl.features.textureCubeSeamless)
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    if (_gl.features.clipControl)
    {
        //glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
    }
#endif

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_gl.limits.maxTextureDimension2D);
    if (_gl.features.texture3D) {
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &_gl.limits.maxTextureDimension3D);
    }
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &_gl.limits.maxTextureDimensionCube);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &_gl.limits.maxTextureArrayLayers);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &_gl.limits.maxColorAttachments);
    //glGetIntegerv(GL_MAX_SAMPLES, &_gl.limits.maxSamples);

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_gl.limits.maxUniformBufferSize);
    _gl.limits.minUniformBufferOffsetAlignment = _vgpuGLGetUInt(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
    if (_gl.features.storageBuffers)
    {
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &_gl.limits.maxStorageBufferSize);
        _gl.limits.minStorageBufferOffsetAlignment = _vgpuGLGetUInt(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
    }

    _gl.limits.maxSamplerAnisotropy = (uint32_t)_vgpuGLGetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);

    // Viewport
    glGetIntegerv(GL_MAX_VIEWPORTS, &_gl.limits.maxViewports);
    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    _gl.limits.maxViewportDimensions[0] = (uint32_t)maxViewportDims[0];
    _gl.limits.maxViewportDimensions[1] = (uint32_t)maxViewportDims[1];

#if defined(VGPU_GL)
    glGetIntegerv(GL_MAX_PATCH_VERTICES, &_gl.limits.maxPatchVertices);
    glGetFloatv(GL_POINT_SIZE_RANGE, _gl.limits.pointSizeRange);
    glGetFloatv(GL_LINE_WIDTH_RANGE, _gl.limits.lineWidthRange);

    // Compute
    if (_gl.features.compute)
    {
        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &_gl.limits.maxComputeSharedMemorySize);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &_gl.limits.maxComputeWorkGroupCount[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &_gl.limits.maxComputeWorkGroupCount[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &_gl.limits.maxComputeWorkGroupCount[2]);
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &_gl.limits.maxComputeWorkGroupInvocations);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &_gl.limits.maxComputeWorkGroupSize[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &_gl.limits.maxComputeWorkGroupSize[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &_gl.limits.maxComputeWorkGroupSize[2]);
    }

#elif defined(VGPU_WEBGL)
    // WebGL
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, _gl.limits.pointSizeRange);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, _gl.limits.lineWidthRange);
#else
    // GLES
    glGetFloatv(GL_POINT_SIZE_RANGE, _gl.limits.pointSizeRange);
    glGetFloatv(GL_LINE_WIDTH_RANGE, _gl.limits.lineWidthRange);
#endif

    // Get default FBO binding.
    _VGPU_CHECK_ERROR();
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&_gl.default_framebuffer);

    // Generate default VAO.
    glGenVertexArrays(1, &_gl.default_vao);
    glBindVertexArray(_gl.default_vao);
    _VGPU_CHECK_ERROR();

    _gl.srgb = settings->swapchain.srgb;
    _gl.width = settings->width;
    _gl.height = settings->height;
    _vgpu_gl_reset_state_cache();

    _vgpu_log(vgpu_log_type_debug, "vgpu initialized with success");
    _gl.frameIndex = true;
    _gl.initialized = true;
    return true;
}

void vgpuShutdown()
{
    if (!_gl.initialized) {
        return;
    }

    // Delete default VAO.
    glDeleteVertexArrays(1, &_gl.default_vao);
    _VGPU_CHECK_ERROR();

    _gl.initialized = false;
    _vgpu_log(vgpu_log_type_debug, "vgpu shutdown with success");
}

bool vgpuQueryFeature(VGpuFeature feature) {
    assert(_gl.initialized);

    switch (feature)
    {
    case VGPU_FEATURE_INDEPENDENT_BLEND:
        return _gl.features.independentBlend; // _d3d11.feature_level >= D3D_FEATURE_LEVEL_10_0;

    case VGPU_FEATURE_COMPUTE_SHADER:
        return _gl.features.compute;

    case VGPU_FEATURE_GEOMETRY_SHADER:
        return  false; //_d3d11.feature_level >= D3D_FEATURE_LEVEL_11_0;

    case VGPU_FEATURE_TESSELLATION_SHADER:
        return  false; //_d3d11.feature_level >= D3D_FEATURE_LEVEL_11_0;

    case VGPU_FEATURE_STORAGE_BUFFERS:
        return  _gl.features.storageBuffers;

    case VGPU_FEATURE_MULTI_VIEWPORT:
        return true;

    case VGPU_FEATURE_INDEX_UINT32:
        return true;

    case VGPU_FEATURE_DRAW_INDIRECT:
        return true;

    case VGPU_FEATURE_FILL_MODE_NON_SOLID:
        return true;

    case VGPU_FEATURE_SAMPLER_ANISOTROPY:
        return true;

    case VGPU_FEATURE_TEXTURE_COMPRESSION_BC:
        return _gl.features.textureCompressionBC;

    case VGPU_FEATURE_TEXTURE_COMPRESSION_PVRTC:
        return false;

    case VGPU_FEATURE_TEXTURE_COMPRESSION_ETC2:
        return _gl.features.textureCompressionETC2;

    case VGPU_FEATURE_TEXTURE_COMPRESSION_ATC:
    case VGPU_FEATURE_TEXTURE_COMPRESSION_ASTC:
        return false;

    case VGPU_FEATURE_TEXTURE_3D:
        return _gl.features.texture3D;

    case VGPU_FEATURE_TEXTURE_2D_ARRAY:
        return _gl.features.texture2DArray;

    case VGPU_FEATURE_TEXTURE_CUBE_ARRAY:
        return false; // (_d3d11.feature_level >= D3D_FEATURE_LEVEL_10_1);

    case VGPU_FEATURE_RAYTRACING:
        return false;

    default:
        return false;
    }

    return false;
}

void vgpuQueryLimits(VGpuLimits* pLimits) {
    assert(_gl.initialized);
    assert(pLimits);

    memcpy(pLimits, &_gl.limits, sizeof(VGpuLimits));
}

uint32_t vgpuFrame() {
    return  _gl.frameIndex++;
}

/* Texture */
VGpuTexture vgpuCreateTexture(const VGpuTextureDescriptor* descriptor) {
    VGpuTexture texture = _VGPU_ALLOC_HANDLE(VGpuTexture);
    texture->external_handle = false;
    _vgpuGLSetupTexture(texture, descriptor);

    glGenTextures(1, &texture->gl_handle);
    _vgpuGLBindTexture(texture, 0);
    _VGPU_CHECK_ERROR();
    return texture;
}

VGpuTexture vgpuCreateExternalTexture(const VGpuTextureDescriptor* descriptor, void* handle) {
    VGpuTexture texture = _VGPU_ALLOC_HANDLE(VGpuTexture);
    _vgpuGLSetupTexture(texture, descriptor);
    texture->gl_handle = *(GLuint*)handle;
    texture->external_handle = true;
    return texture;
}

void vgpuDestroyTexture(VGpuTexture texture) {
    if (!texture || texture->external_handle) {
        return;
    }

    _VGPU_CHECK_ERROR();
    if (texture->gl_handle) {
        glDeleteTextures(1, &texture->gl_handle);
    }
    _VGPU_FREE(texture);
    _VGPU_CHECK_ERROR();
}

/* Buffer */
VGpuBuffer vgpuCreateBuffer(uint64_t size, VGpuBufferUsage usage, VGpuResourceUsage resourceUsage, const void* data) {
    _VGPU_CHECK_ERROR();
    VGpuBuffer buffer = _VGPU_ALLOC_HANDLE(VGpuBuffer);
    buffer->size = size;
    buffer->usage = usage;
    buffer->resourceUsage = resourceUsage;
    buffer->external_handle = false;
    buffer->gl_type = _vgpuGLConvertBufferUsage(usage);
    buffer->gl_target = _vgpuGLConvertBufferType(buffer->gl_type);
    glGenBuffers(1, &buffer->gl_handle);
    _vgpuGLBindBuffer(buffer);

#if defined(VGPU_WEBGL)
    buffer->gl_data = malloc(size);
    _VGPU_ASSERT(buffer->gl_data);
    glBufferData(buffer->gl_target, size, data, _vgpuGLConvertResourceUsage(usage));

    if (data) {
        memcpy(buffer->gl_data, data, size);
    }
#else
    if (_gl.features.bufferStorage) {
        const bool readable = false;
        const bool dynamic = usage == VGPU_RESOURCE_USAGE_DYNAMIC || usage == VGPU_RESOURCE_USAGE_STREAM;
        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
        if (dynamic) {
            flags |= GL_DYNAMIC_STORAGE_BIT;
        }
        if (readable) {
            flags |= GL_MAP_READ_BIT;
        }
        glBufferStorage(buffer->gl_target, size, data, flags);
        buffer->gl_data = glMapBufferRange(buffer->gl_target, 0, size, flags | GL_MAP_FLUSH_EXPLICIT_BIT);
    }
    else {
        glBufferData(buffer->gl_target, size, data, _vgpuGLConvertResourceUsage(usage));
    }
#endif
    _VGPU_CHECK_ERROR();

    return buffer;
}

void vgpuDestroyBuffer(VGpuBuffer buffer) {
    if (!buffer || buffer->external_handle) {
        return;
    }

    _VGPU_CHECK_ERROR();
    if (buffer->gl_handle) {
        glDeleteBuffers(1, &buffer->gl_handle);

#ifndef VGPU_WEBGL
        if (!_gl.features.bufferStorage) {
#endif
            free(buffer->gl_data);
#ifndef VGPU_WEBGL
        }
#endif

    }
    _VGPU_FREE(buffer);
    _VGPU_CHECK_ERROR();
}

/* Shader */
const char* _vgpuGLShaderVertexPrefix = ""
"in vec3 vgpuPosition; \n"
"in vec3 vgpuNormal; \n"
"in vec2 vgpuTexCoord; \n"
"in vec4 vgpuVertexColor; \n"
"#line 0 \n";

const char* _vgpuGLShaderComputeESHeader = ""
"#version 310 es \n"
"#line 0 \n";

const char* _vgpuGLShaderComputeHeader = ""
"#version 430 \n"
"#line 0 \n";

static GLuint _vgpuGLCompileShader(GLenum type, const char** sources, int count) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, count, sources, NULL);
    glCompileShader(shader);

    int isShaderCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled) {
        int logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            char* log = malloc(logLength);
            _VGPU_ASSERT(log);
            glGetShaderInfoLog(shader, logLength, &logLength, log);
            const char* name;
            switch (type) {
            case GL_VERTEX_SHADER: name = "vertex shader"; break;
            case GL_FRAGMENT_SHADER: name = "fragment shader"; break;
            case GL_COMPUTE_SHADER: name = "compute shader"; break;
            default: name = "shader"; break;
            }

            //_vgpu_log(vgpu_log_type_error, "Could not compile %s:\n%s", name, log);
            free(log);
        }
    }
    _VGPU_CHECK_ERROR();

    return shader;
}

static GLuint _vgpuGLLinkProgram(GLuint program) {
    glLinkProgram(program);

    int isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (!isLinked) {
        int logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        char* log = malloc(logLength);
        _VGPU_ASSERT(log);
        glGetProgramInfoLog(program, logLength, &logLength, log);
        //_vgpu_log(vgpu_log_type_error, "Could not link shader:\n%s", log);
    }

    return program;
}


VGpuShader vgpuCreateShader(const char* vertexSource, const char* fragmentSource) {
#if defined(VGPU_WEBGL) || defined(VGPU_GLES)
    const char* vertexHeader = "#version 300 es\nprecision mediump float;\nprecision mediump int;\n";
    const char* fragmentHeader = vertexHeader;
#else
    const char* vertexHeader = _gl.features.compute ? "#version 430\n" : "#version 330 core\n";
    const char* fragmentHeader = "#version 150\n";
#endif

    const char* vertexSources[] = { vertexHeader/*, _vgpuGLShaderVertexPrefix*/, vertexSource };
    GLuint vertexShader = _vgpuGLCompileShader(GL_VERTEX_SHADER, vertexSources, sizeof(vertexSources) / sizeof(vertexSources[0]));

    // Fragment
    const char* fragmentSources[] = { fragmentHeader, fragmentSource };
    GLuint fragmentShader = _vgpuGLCompileShader(GL_FRAGMENT_SHADER, fragmentSources, sizeof(fragmentSources) / sizeof(fragmentSources[0]));

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    //glBindAttribLocation(program, _VGPU_GL_SHADER_POSITION, "vgpuPosition");
    //glBindAttribLocation(program, _VGPU_GL_SHADER_NORMAL, "vgpuNormal");
    //glBindAttribLocation(program, _VGPU_GL_SHADER_TEX_COORD, "vgpuTexCoord");
    //glBindAttribLocation(program, _VGPU_GL_SHADER_VERTEX_COLOR, "vgpuVertexColor");
    _vgpuGLLinkProgram(program);
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);
    _VGPU_CHECK_ERROR();

    // Bind program and get reflection data.
    _vgpuGLUseProgram(program);

    VGpuShader shader = _VGPU_ALLOC_HANDLE(VGpuShader);
    shader->gl_handle = program;
    return shader;
}

VGpuShader vgpuCreateComputeShader(const char* source) {
#if defined(VGPU_WEBGL)
    _VGPU_THROW("Compute shaders are not supported on WebGL");
#else
    if (!_gl.features.compute) {
        //_VGPU_THROW("Compute shaders are not supported on this system");
    }

#if defined(VGPU_GL) 
    const char* sources[] = { _vgpuGLShaderComputeHeader, source };
#else
    const char* sources[] = { _vgpuGLShaderComputeESHeader, source };
#endif

    GLuint program = glCreateProgram();
    GLuint computeShader = _vgpuGLCompileShader(GL_COMPUTE_SHADER, sources, sizeof(sources) / sizeof(sources[0]));
    glAttachShader(program, computeShader);
    _vgpuGLLinkProgram(program);
    glDetachShader(program, computeShader);
    glDeleteShader(computeShader);
    _VGPU_CHECK_ERROR();

    VGpuShader shader = _VGPU_ALLOC_HANDLE(VGpuShader);
    shader->gl_handle = program;
    //shader->gl_type = VGPU_SHADER_TYPE_COMPUTE;
    /* TODO: Load uniforms, ubo etc*/
    return shader;
#endif
}

void vgpuDestroyShader(VGpuShader shader) {
    if (!shader) {
        return;
    }

    _VGPU_CHECK_ERROR();
    if (shader->gl_handle) {
        glDeleteProgram(shader->gl_handle);
    }
    _VGPU_FREE(shader);
    _VGPU_CHECK_ERROR();
}

VGpuPipeline vgpuCreateRenderPipeline(const VGpuRenderPipelineDescriptor* descriptor) {
    VGpuPipeline pipeline = _VGPU_ALLOC_HANDLE(VGpuPipeline);
    pipeline->shader = descriptor->shader;
    pipeline->topology = _vgpuGLConvertPrimitiveTopology(descriptor->primitiveTopology);

    /* resolve vertex attributes */
    for (unsigned i = 0; i < VGPU_MAX_VERTEX_ATTRIBUTES; i++) {
        pipeline->gl_attrs[i].vb_index = -1;
    }

    for (GLint i = 0; i < VGPU_MAX_VERTEX_ATTRIBUTES; i++) {
        const VGpuVertexAttributeDescriptor* attr_desc = &descriptor->vertexDescriptor.attributes[i];
        if (attr_desc->format == VGPU_VERTEX_FORMAT_UNKNOWN) {
            break;
        }

        _VGPU_ASSERT((attr_desc->bufferIndex >= 0) && (attr_desc->bufferIndex < VGPU_MAX_VERTEX_BUFFER_BINDINGS));
        const VGpuVertexBufferLayoutDescriptor* buffer_layout_desc = &descriptor->vertexDescriptor.layouts[attr_desc->bufferIndex];
        const VGpuVertexInputRate inputRate = buffer_layout_desc->inputRate;
        GLint attr_loc = i;
        /*if (!_sg_strempty(&shd->attrs[attr_index].name)) {
            attr_loc = glGetAttribLocation(pip->shader->gl_prog, _sg_strptr(&shd->attrs[attr_index].name));
        }*/
        if (attr_loc != -1) {
            _VGpuGLVertexAttribute* gl_attribute = &pipeline->gl_attrs[attr_loc];
            _VGPU_ASSERT(gl_attribute->vb_index == -1);
            gl_attribute->vb_index = (int8_t)attr_desc->bufferIndex;
            if (inputRate == VGPU_VERTEX_INPUT_RATE_VERTEX) {
                gl_attribute->divisor = 0;
            }
            else {
                gl_attribute->divisor = 1;
            }
            _VGPU_ASSERT(buffer_layout_desc->stride > 0);
            gl_attribute->stride = (uint8_t)buffer_layout_desc->stride;
            gl_attribute->offset = attr_desc->offset;
            gl_attribute->size = _vgpuGLGetVertexFormatSize(attr_desc->format);
            gl_attribute->type = _vgpuGLGetVertexFormatType(attr_desc->format);
            gl_attribute->normalized = _vgpuGLConvertVertexFormatNormalized(attr_desc->format);
            pipeline->vertex_layout_valid[attr_desc->bufferIndex] = true;
        }
        else {
        }
    }

    return pipeline;
}

void vgpuDestroyPipeline(VGpuPipeline pipeline) {
    if (!pipeline) {
        return;
    }

    _VGPU_CHECK_ERROR();
    _VGPU_FREE(pipeline);
    _VGPU_CHECK_ERROR();
}

/* Commands */
void vgpuBeginDefaultRenderPass(VGpuColor clearColor, float clearDepth, uint8_t clearStencil) {
    glBindFramebuffer(GL_FRAMEBUFFER, _gl.default_framebuffer);
    glViewport(0, 0, _gl.width, _gl.height);
    glScissor(0, 0, _gl.width, _gl.height);
    _VGPU_CHECK_ERROR();

#if defined(VGPU_GL)
    glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#else
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    glClearBufferfv(GL_COLOR, 0, &clearColor.r);
    _VGPU_CHECK_ERROR();

    // Enable depth mask when clearing depth.
    glDepthMask(GL_TRUE);
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);
    _VGPU_CHECK_ERROR();
}

void vgpuBeginRenderPass(const VGpuRenderPassBeginDescriptor* descriptor) {
}

void vgpuEndRenderPass() {
}

void vgpuBindPipeline(VGpuPipeline pipeline) {
    if (_gl.state.currentPipeline != pipeline)
    {
        _gl.state.currentPipeline = pipeline;

        /* Bind program */
        _vgpuGLUseProgram(pipeline->shader->gl_handle);
    }
}

static void _vgpuGLPrepareDraw() {
    /* vertex attributes */
    uint32_t vb_offset = 0;
    for (uint32_t i = 0; i < VGPU_MAX_VERTEX_ATTRIBUTES; i++) {
        _VGpuGLVertexAttribute* gl_attr = &_gl.state.currentPipeline->gl_attrs[i];
        if (gl_attr->vb_index >= 0) {
            vb_offset = gl_attr->offset;

            if (gl_attr->integer) {
                glVertexAttribIPointer(i, gl_attr->size, gl_attr->type, gl_attr->stride, (const GLvoid*)(GLintptr)vb_offset);
            }
            else {
                glVertexAttribPointer(i, gl_attr->size, gl_attr->type, gl_attr->normalized, gl_attr->stride, (const GLvoid*)(GLintptr)vb_offset);
            }
            glVertexAttribDivisor(i, gl_attr->divisor);
            glEnableVertexAttribArray(i);
        }
    }
}

void vgpuDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) {
    GLenum primitive_type = _gl.state.currentPipeline->topology;

    _vgpuGLPrepareDraw();

    if (instanceCount > 1) {
        glDrawArraysInstanced(primitive_type, firstVertex, vertexCount, instanceCount);
    }
    else {
        glDrawArrays(primitive_type, firstVertex, vertexCount);
    }
    _VGPU_CHECK_ERROR();
}

void vgpuDispatch(VGpuShader computeShader, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
#if defined(VGPU_WEBGL)
    _VGPU_THROW("Compute shaders are not supported on WebGL");
#else
    if (!_gl.features.compute) {
        //_VGPU_THROW("Compute shaders are not supported on this system");
    }

    glUseProgram(computeShader->gl_handle);
    glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    _VGPU_CHECK_ERROR();
#endif
}

#endif /* defined(VGPU_GL) || defined(VGPU_GLES) || defined(VGPU_WEBGL) */
