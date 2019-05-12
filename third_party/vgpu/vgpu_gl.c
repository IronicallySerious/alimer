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
#include <assert.h>
#if defined(_WIN32) || defined(_WIN64)
#   include <malloc.h>
#   undef    alloca
#   define   alloca _malloca
#   define   freea  _freea
#else
#   include <alloca.h>
#endif

#if defined(VGPU_WEBGL)
#   include <GLES3/gl3.h>
#   include <GLES2/gl2ext.h>
#   include <GL/gl.h>
#   include <GL/glext.h>
#else 
#   include "glad.h"
#endif

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

typedef struct _vgpu_gl_features {
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
    bool    clipControl;
} _vgpu_gl_features;

typedef struct _vgpu_gl_cache {
    /* depth stencil state*/
    VGpuCompareFunction     depthCompareFunction;
    bool                    depthWriteEnabled;
    bool                    stencilEnabled;

    bool        alphaToCoverage;
    uint32_t    primitiveRestart;
} _vgpu_gl_cache;

struct {
    bool initialized;
    _vgpu_gl_features   features;
    VGpuLimits           limits;
    _vgpu_gl_cache      state;
    VGpuSwapchainDescriptor swapchain;
    const char*         vendor;
    const char*         renderer;
    const char*         version;
    const char*         glslVersion;
    GLint               version_major;
    GLint               version_minor;
} _gl = { 0 };

extern void _vgpu_log(vgpu_log_type type, const char *message);

void _vgpu_gl_check_extension(const char* ext)
{
    if (strstr(ext, "EXT_texture_compression_s3tc")
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

void _vgpu_gl_reset_state_cache()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifndef VGPU_WEBGL
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);
    if (_gl.swapchain.srgb) {
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
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
#ifdef VGPU_GLES
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#elif defined(VGPU_GL)
    glEnable(GL_PRIMITIVE_RESTART);
    _gl.state.primitiveRestart = 0xffffffff;
    glPrimitiveRestartIndex(_gl.state.primitiveRestart);
#endif

}

VGpuBackend vgpu_get_backend() {
    return VGPU_BACKEND_OPENGL;
}

bool vgpu_initialize(const char* app_name, const VGpuRendererSettings* settings)
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
        _gl.features.textureCubeArray = true;
        // https://www.khronos.org/opengl/wiki/Tessellation
        _gl.features.tessellation = true;

        // Core in version 4.3+
        if (_gl.version_minor >= 3)
        {
            // https://www.khronos.org/opengl/wiki/Compute_Shader
            _gl.features.compute = true;
            _gl.features.storageBuffers = true;
        }

        // Core in version 4.5+
        if (_gl.version_minor >= 5)
        {
            _gl.features.clipControl = true;
        }
    }

    // point size.
    glGetFloatv(GL_POINT_SIZE_RANGE, _gl.limits.pointSizeRange);
#else
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, _gl.limits.pointSizeRange);
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

    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    _gl.limits.maxSamplerAnisotropy = (uint32_t)max_anisotropy;

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_gl.limits.maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_gl.limits.minUniformBufferOffsetAlignment);
    if (_gl.features.storageBuffers)
    {
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &_gl.limits.maxStorageBufferSize);
        glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &_gl.limits.minStorageBufferOffsetAlignment);
    }

    _vgpu_gl_reset_state_cache();

    _vgpu_log(vgpu_log_type_debug, "vgpu initialized with success");
    _gl.initialized = true;
    return true;
        }

void vgpu_shutdown()
{
    if (!_gl.initialized) {
        return;
    }

    _gl.initialized = false;
    _vgpu_log(vgpu_log_type_debug, "vgpu shutdown with success");
}

bool vgpu_query_feature(VGpuFeature feature) {
    assert(_gl.initialized);

    switch (feature)
    {
    case VGPU_FEATURE_BLEND_INDEPENDENT:
        return false; // _d3d11.feature_level >= D3D_FEATURE_LEVEL_10_0;

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

void vgpu_query_limits(VGpuLimits* pLimits) {
    assert(_gl.initialized);
    assert(pLimits);

    *pLimits = _gl.limits;
}

#endif /* defined(VGPU_GL) || defined(VGPU_GLES) || defined(VGPU_WEBGL) */
