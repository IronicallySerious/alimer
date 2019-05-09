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

#if defined(VGPU_WEBGL)
#   include <GLES3/gl3.h>
#   include <GLES2/gl2ext.h>
#   include <GL/gl.h>
#   include <GL/glext.h>
#else 
#   include "glad.h"
#endif

typedef struct _vgpu_gl_features {
    bool    compute;
    bool    singlepass;
} _vgpu_gl_features;

typedef struct _vgpu_gl_limits {
    float   pointSizes[2];
    int     textureSize;
    int     textureMSAA;
    float   textureAnisotropy;
    int     blockSize;
    int     blockAlign;
} _vgpu_gl_limits;

typedef struct _vgpu_gl_cache {
    bool        alphaToCoverage;
    uint32_t    primitiveRestart;
} _vgpu_gl_cache;

struct {
    bool initialized;
    _vgpu_gl_features   features;
    _vgpu_gl_limits     limits;
    _vgpu_gl_cache      state;
} _gl = { 0 };

extern void _vgpu_log(vgpu_log_type type, const char *message);

VGpuBackend vgpu_get_backend() {
    return VGPU_BACKEND_OPENGL;
}

void vgpu_initialize(const char* app_name, const VGpuRendererSettings* settings)
{
    if (_gl.initialized) {
        _vgpu_log(vgpu_log_type_error, "vgpu already initialized");
        return;
    }

#ifndef VGPU_WEBGL
    _gl.features.compute = GLAD_GL_ARB_compute_shader;
    _gl.features.singlepass = GLAD_GL_ARB_viewport_array && GLAD_GL_AMD_vertex_shader_viewport_index && GLAD_GL_ARB_fragment_layer_viewport;
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);
    if (settings->swapchain.srgb) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    glGetFloatv(GL_POINT_SIZE_RANGE, _gl.limits.pointSizes);
#else
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, _gl.limits.pointSizes);
#endif

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_gl.limits.textureSize);
    glGetIntegerv(GL_MAX_SAMPLES, &_gl.limits.textureMSAA);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_gl.limits.blockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_gl.limits.blockAlign);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &_gl.limits.textureAnisotropy);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef VGPU_GLES
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#elif defined(VGPU_GL)
    glEnable(GL_PRIMITIVE_RESTART);
    _gl.state.primitiveRestart = 0xffffffff;
    glPrimitiveRestartIndex(_gl.state.primitiveRestart);
#endif

    _gl.state.alphaToCoverage = false;
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    _vgpu_log(vgpu_log_type_debug, "vgpu initialized with success");
    _gl.initialized = true;
}

void vgpu_shutdown()
{
    if (!_gl.initialized) {
        return;
    }

    _gl.initialized = false;
    _vgpu_log(vgpu_log_type_debug, "vgpu shutdown with success");
}


#endif /* defined(VGPU_GL) || defined(VGPU_GLES) || defined(VGPU_WEBGL) */
