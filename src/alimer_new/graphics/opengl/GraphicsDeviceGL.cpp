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

#include "GraphicsDeviceGL.h"
#include "foundation/Utils.h"
#if defined(ALIMER_GLFW)
#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>
#endif /* ALIMER_GLFW */

GLenum __gl_error_code = GL_NO_ERROR;

namespace alimer
{
    bool GraphicsDeviceGL::IsSupported()
    {
        return true;
    }

    GraphicsDeviceGL::GraphicsDeviceGL(bool validation)
    {
        ALIMER_ASSERT_MSG(IsSupported(), "OpenGL backend is not supported");

#if defined(ALIMER_GLFW)
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            //ALIMER_LOGERROR("Failed to initialize GLAD");
            return;
        }
#endif

        _validation = validation;
        InitializeCaps();
    }

    GraphicsDeviceGL::~GraphicsDeviceGL()
    {
    }

    SwapChain* GraphicsDeviceGL::CreateSwapChainImpl(SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
    {
        return nullptr;
    }

    static std::string GLGetString(GLenum name)
    {
        auto bytes = glGetString(name);
        return (bytes != nullptr ? std::string(reinterpret_cast<const char*>(bytes)) : "");
    }

    static uint32_t GLGetUInt(GLenum param) {
        GLint attr = 0;
        glGetIntegerv(param, &attr);
        return static_cast<uint32_t>(attr);
    }

    static float GLGetFloat(GLenum param) {
        GLfloat attr = 0.0f;
        glGetFloatv(param, &attr);
        return attr;
    }

    static uint32_t GLGetUIntIndexed(GLenum param, GLuint index) {
        GLint attr = 0;
        //if (GLAD_GL_EXT_draw_buffers2))
        glGetIntegeri_v(param, index, &attr);
        return static_cast<uint32_t>(attr);
    }

    void GraphicsDeviceGL::InitializeCaps()
    {
        _info.backend = GraphicsBackend::OpenGL;
        _info.backendName = "OpenGL " + GLGetString(GL_VERSION);
        _info.deviceName = GLGetString(GL_RENDERER);
        _info.vendorName = GLGetString(GL_VENDOR);
        //_info.shadingLanguageName = "GLSL " + GLGetString(GL_SHADING_LANGUAGE_VERSION);

        //vgpuLogFormat(VGPU_LOG_LEVEL_INFO, "GL vendor: %s", vendor);
        //vgpuLogFormat(VGPU_LOG_LEVEL_INFO, "GL version: %s", version);
        //vgpuLogFormat(VGPU_LOG_LEVEL_INFO, "GL renderer: %s", renderer);
        //vgpuLogFormat(VGPU_LOG_LEVEL_INFO, "GLSL version: %s", glslVersion);

        float pointSizes[2];
        float lineWidthRanges[2];

#if !ALIMER_WEBGL
        _versionMajor = 3;
        _versionMinor = 3;
        /* On desktop we run minimum 3.3+ context */
        _caps.features.instancing = true;
        _caps.features.alphaToCoverage = true;
        _caps.features.independentBlend = false; /* TODO: */
        _caps.features.computeShader = GLAD_GL_ARB_compute_shader;
        _caps.features.geometryShader = true;

        if (_versionMajor >= 4) {
            _caps.features.independentBlend = true;
            _caps.features.tessellationShader = true;
        }
        _caps.features.sampleRateShading = false;
        _caps.features.dualSrcBlend = false;
        _caps.features.logicOp = false;
        _caps.features.multiViewport = false;
        _caps.features.indexUInt32 = true;
        _caps.features.drawIndirect = false;
        _caps.features.alphaToOne = true;
        _caps.features.fillModeNonSolid = true;
        _caps.features.samplerAnisotropy = true;
        _caps.features.textureCompressionBC = true;
        _caps.features.textureCompressionPVRTC = false;
        _caps.features.textureCompressionETC2 = false;
        _caps.features.textureCompressionATC = false;
        _caps.features.textureCompressionASTC = false;
        _caps.features.pipelineStatisticsQuery = true;
        _caps.features.texture1D = true;
        _caps.features.texture3D = true;
        _caps.features.texture2DArray = true;
        _caps.features.textureCubeArray = true;

        _singlePass = GLAD_GL_ARB_viewport_array && GLAD_GL_AMD_vertex_shader_viewport_index && GLAD_GL_ARB_fragment_layer_viewport;
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_PROGRAM_POINT_SIZE);
        /*if (descriptor->swapchain->srgb) {
            glEnable(GL_FRAMEBUFFER_SRGB);
        }
        else {
            glDisable(GL_FRAMEBUFFER_SRGB);
        }*/
        glGetFloatv(GL_POINT_SIZE_RANGE, pointSizes);
        glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidthRanges);
#else
        /* GLES 3.0+ */
        if (_versionMajor >= 3) {
            _caps.features.alphaToCoverage = true;
        }
        glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizes);
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRanges);
#endif
        // Limits
        _caps.limits.maxTextureDimension1D = GLGetUInt(GL_MAX_TEXTURE_SIZE);
        _caps.limits.maxTextureDimension2D = GLGetUInt(GL_MAX_TEXTURE_SIZE);
#if GL_MAX_3D_TEXTURE_SIZE
        _caps.limits.maxTextureDimension3D = GLGetUInt(GL_MAX_3D_TEXTURE_SIZE);
#else
        _caps.limits.maxTextureDimension3D = GLGetUInt(GL_MAX_TEXTURE_SIZE);
#endif
        _caps.limits.maxTextureDimensionCube = GLGetUInt(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
#if GL_MAX_ARRAY_TEXTURE_LAYERS
        _caps.limits.maxTextureArrayLayers = GLGetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
#else
        _caps.limits.maxTextureArrayLayers = 1;
#endif
        _caps.limits.maxColorAttachments = GLGetUInt(GL_MAX_DRAW_BUFFERS);
        _caps.limits.maxUniformBufferSize = GLGetUInt(GL_MAX_UNIFORM_BLOCK_SIZE);
        _caps.limits.minUniformBufferOffsetAlignment = GLGetUInt(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
        _caps.limits.maxStorageBufferSize = GLGetUInt(GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
        _caps.limits.minStorageBufferOffsetAlignment = GLGetUInt(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
        _caps.limits.maxSamplerAnisotropy = static_cast<uint32_t>(GLGetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));
        _caps.limits.maxViewports = GLGetUInt(GL_MAX_VIEWPORTS);

        GLint maxViewportDims[2];
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);

        _caps.limits.maxViewportDimensions[0] = maxViewportDims[0];
        _caps.limits.maxViewportDimensions[1] = maxViewportDims[0];
        _caps.limits.maxPatchVertices = GLGetUInt(GL_MAX_PATCH_VERTICES);
        _caps.limits.pointSizeRange[0] = pointSizes[0];
        _caps.limits.pointSizeRange[1] = pointSizes[1];
        _caps.limits.lineWidthRange[0] = lineWidthRanges[0];
        _caps.limits.lineWidthRange[1] = lineWidthRanges[0];
        _caps.limits.maxComputeSharedMemorySize = GLGetUInt(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
        _caps.limits.maxComputeWorkGroupCount[0] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
        _caps.limits.maxComputeWorkGroupCount[1] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
        _caps.limits.maxComputeWorkGroupCount[2] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);
        _caps.limits.maxComputeWorkGroupInvocations = GLGetUInt(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
        _caps.limits.maxComputeWorkGroupSize[0] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
        _caps.limits.maxComputeWorkGroupSize[1] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
        _caps.limits.maxComputeWorkGroupSize[2] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GL_ASSERT(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&_defaultFramebuffer));

        // Generate default vao
        if (!_gles2) {
            glGenVertexArrays(1, &_vertexArrayId);
            glBindVertexArray(_vertexArrayId);
        }

        //_insideRenderPass = false;
        //_currentPassWidth = descriptor->swapchain->width;
        //_currentPassHeight = descriptor->swapchain->height;
        //ResetStateCache(&context->cache, context->gles2);
    }
}
