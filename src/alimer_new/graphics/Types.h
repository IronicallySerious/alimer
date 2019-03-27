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

#include "engine/window_handle.h"
#include "graphics/PixelFormat.h"
#include <string>

namespace alimer
{
    /// Defines the graphics backend.
    enum class GraphicsBackend
    {
        Default = 0,
        Null,
        Vulkan,
        Direct3D12,
        Direct3D11,
    };

    /// Defines the gpu device power preference.
    enum class GpuPowerPreference
    {
        Default = 0,
        LowPower = 1,
        HighPerformance = 2,
    };

    enum class ShaderStage : uint32_t
    {
        Vertex = 0,
        TessellationControl = 1,
        TessellationEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5,
        Count
    };

    /// GraphicsDevice information .
    struct GraphicsDeviceInfo
    {
        /// Backend type.
        GraphicsBackend backend;

        /// Rendering API name.
        std::string backendName;

        /// The hardware gpu device vendor name.
        std::string vendorName;

        /// The hardware gpu device vendor id.
        uint32_t vendorId;

        /// The hardware gpu device name.
        std::string deviceName;
    };

    /// Describes features supported by given instance of GpuDevice.
    struct GraphicsDeviceFeatures
    {
        bool    instancing = false;
        bool    alphaToCoverage = false;
        bool    independentBlend = false;
        bool    computeShader = false;
        bool    geometryShader = false;
        bool    tessellationShader = false;
        bool    sampleRateShading = false;
        bool    dualSrcBlend = false;
        bool    logicOp = false;
        bool    multiViewport = false;
        bool    indexUInt32 = false;
        bool    drawIndirect = false;
        bool    alphaToOne = false;
        bool    fillModeNonSolid = false;
        bool    samplerAnisotropy = false;
        bool    textureCompressionBC = false;
        bool    textureCompressionPVRTC = false;
        bool    textureCompressionETC2 = false;
        bool    textureCompressionATC = false;
        bool    textureCompressionASTC = false;
        bool    pipelineStatisticsQuery = false;
        /// Specifies whether 1D textures are supported.
        bool    texture1D = false;
        /// Specifies whether 3D textures are supported.
        bool    texture3D = false;

        /// Specifies whether 2D array textures are supported.
        bool    texture2DArray = false;

        /// Specifies whether cube array textures are supported.
        bool    textureCubeArray = false;
    };

    struct GraphicsDeviceLimits
    {
        uint32_t        maxTextureDimension1D;
        uint32_t        maxTextureDimension2D;
        uint32_t        maxTextureDimension3D;
        uint32_t        maxTextureDimensionCube;
        uint32_t        maxTextureArrayLayers;
        uint32_t        maxColorAttachments;
        uint32_t        maxUniformBufferSize;
        uint64_t        minUniformBufferOffsetAlignment;
        uint32_t        maxStorageBufferSize;
        uint64_t        minStorageBufferOffsetAlignment;
        uint32_t        maxSamplerAnisotropy;
        uint32_t        maxViewports;
        uint32_t        maxViewportDimensions[2];
        uint32_t        maxPatchVertices;
        float           pointSizeRange[2];
        float           lineWidthRange[2];
        uint32_t        maxComputeSharedMemorySize;
        uint32_t        maxComputeWorkGroupCount[3];
        uint32_t        maxComputeWorkGroupInvocations;
        uint32_t        maxComputeWorkGroupSize[3];
    };

    /// Describes caps of graphics device.
    struct GraphicsDeviceCapabilities
    {
        /// Specifies all supported hardware features.
        GraphicsDeviceFeatures          features;

        /// Specifies all rendering limitations.
        GraphicsDeviceLimits            limits;
    };

    struct SwapChainSurface
    {
#if defined(_WIN64) || defined(_WIN32)
        void *hInstance;
        void *window;
#elif defined(__ANDROID__)
        void *window;
#elif defined(__linux__)
        const void **display;
        uint64_t window;
#elif defined(__APPLE__)
        void *layer;
#endif
    };

    struct SwapChainDescriptor
    {
        uint32_t width;
        uint32_t height;
        /// The color buffer format srgb.
        bool srgb = true;
        /// The depth buffer format
        PixelFormat depthFormat = PixelFormat::Depth32Float;

        bool vSyncEnabled = false;
    };
}
