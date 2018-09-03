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

#include "Graphics.h"
#include "ShaderCompiler.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Log.h"

#if ALIMER_VULKAN
#include "Graphics/Vulkan/VulkanGraphics.h"
#endif

#if ALIMER_D3D11
#include "Graphics/D3D11/D3D11Graphics.h"
#endif

#if ALIMER_D3D12
#include "Graphics/D3D12/D3D12Graphics.h"
#endif

using namespace std;

#if defined(_WIN32)
// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    Graphics::Graphics(GraphicsDeviceType deviceType, bool validation)
        : _deviceType(deviceType)
        , _validation(validation)
        , _initialized(false)
        , _adapter(nullptr)
        , _features{}
    {
    }

    Graphics::~Graphics()
    {
        Finalize();

        // Clear adapters.
        for (auto &adapter : _adapters)
        {
            SafeDelete(adapter);
        }
        _adapters.clear();
    }

    void Graphics::Finalize()
    {
        // Destroy undestroyed resources.
        if (_gpuResources.size())
        {
            lock_guard<mutex> lock(_gpuResourceMutex);

            // Release all GPU objects that still exist
            std::sort(_gpuResources.begin(), _gpuResources.end(),
                [](const GpuResource* x, const GpuResource* y)
            {
                return x->GetResourceType() < y->GetResourceType();
            });

            for (size_t i = 0; i < _gpuResources.size(); ++i)
            {
                GpuResource* resource = _gpuResources.at(i);
                ALIMER_ASSERT(resource);
                resource->Destroy();
            }

            _gpuResources.clear();
        }
    }

    set<GraphicsDeviceType> Graphics::GetAvailableBackends()
    {
        static set<GraphicsDeviceType> availableBackends;

        if (availableBackends.empty())
        {
            availableBackends.insert(GraphicsDeviceType::Empty);

#if ALIMER_VULKAN
            if (VulkanGraphics::IsSupported())
            {
                availableBackends.insert(GraphicsDeviceType::Vulkan);
            }
#endif

#if ALIMER_D3D12
            if (D3D12Graphics::IsSupported())
            {
                availableBackends.insert(GraphicsDeviceType::Direct3D12);
            }
#endif

#if ALIMER_D3D11
            if (D3D11Graphics::IsSupported())
            {
                availableBackends.insert(GraphicsDeviceType::Direct3D11);
            }
#endif

        }

        return availableBackends;
    }

    Graphics* Graphics::Create(GraphicsDeviceType deviceType, bool validation, const std::string& applicationName)
    {
        if (deviceType == GraphicsDeviceType::Default)
        {
            auto availableDrivers = Graphics::GetAvailableBackends();

            if (availableDrivers.find(GraphicsDeviceType::Vulkan) != availableDrivers.end())
            {
                deviceType = GraphicsDeviceType::Vulkan;
            }
            else if (availableDrivers.find(GraphicsDeviceType::Direct3D12) != availableDrivers.end())
            {
                deviceType = GraphicsDeviceType::Direct3D12;
            }
            else
            {
                deviceType = GraphicsDeviceType::Empty;
            }
        }

        Graphics* graphics = nullptr;
        switch (deviceType)
        {
        case GraphicsDeviceType::Vulkan:
#if ALIMER_VULKAN
            if (VulkanGraphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Vulkan graphics backend");
                graphics = new VulkanGraphics(validation, applicationName);
            }
            else
#endif
            {
                ALIMER_LOGERROR("Vulkan graphics backend not supported");
            }

            break;

        case GraphicsDeviceType::Direct3D12:
#if ALIMER_D3D12
            if (D3D12Graphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Direct3D 12 graphics backend");
                graphics = new D3D12Graphics(validation);
            }
            else
#endif
            {
                ALIMER_LOGERROR("Direct3D 12 graphics backend not supported");
            }

            break;

        case GraphicsDeviceType::Direct3D11:
#if ALIMER_D3D11
            if (D3D11Graphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Direct3D 11 graphics backend");
                graphics = new D3D11Graphics(validation);
            }
            else
#endif
            {
                ALIMER_LOGERROR("Direct3D 11 graphics backend not supported");
            }

            break;

        case GraphicsDeviceType::Default:
            break;

        case GraphicsDeviceType::Empty:
        default:
            break;
        }

        return graphics;
    }

    bool Graphics::Initialize(GpuAdapter* adapter, WindowPtr window)
    {
        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize Graphics if already initialized");
            return false;
        }

        if (!adapter)
        {
            _adapter = GetDefaultAdapter();
        }
        else
        {
            _adapter = adapter;
        }

        ALIMER_ASSERT_MSG(window.Get(), "Invalid window for graphics creation");
        _window = window;
        _initialized = BackendInitialize();
        return _initialized;
    }

    GpuBuffer* Graphics::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (!descriptor->usage)
        {
            ALIMER_LOGCRITICAL("Invalid buffer usage");
        }

        if (!descriptor->size)
        {
            ALIMER_LOGCRITICAL("Cannot create empty buffer");
        }

        if (initialData && !(descriptor->usage & BufferUsage::TransferDest))
        {
            ALIMER_LOGCRITICAL("Buffer needs the transfer dest usage when creating with initial data");
        }

        return CreateBufferImpl(descriptor, initialData);
    }

    Shader* Graphics::CreateShader(const string& vertexShaderFile, const std::string& fragmentShaderFile)
    {
        // Load from spv bytecode.
        vector<uint8_t> vertexByteCode;
        vector<uint8_t> fragmentByteCode;

        /*if (gResources()->Exists(vertexShaderFile + ".spv")
            && gResources()->Exists(fragmentShaderFile + ".spv"))
        {
            auto vertexShaderStream = FileSystem::Get().Open(vertexShaderFile + ".spv");
            //auto vertexShaderStream = gResources()->Open(vertexShaderFile + ".spv");
            auto fragmentShaderStream = gResources()->Open(fragmentShaderFile + ".spv");

            // Lookup for SPIR-V compiled shader.
            if (!vertexShaderStream)
            {
                ALIMER_LOGERROR("GLSL shader does not exists '{}'", vertexShaderFile.c_str());
                return nullptr;
            }

            if (!fragmentShaderStream)
            {
                ALIMER_LOGERROR("GLSL shader does not exists '{}'", fragmentShaderFile.c_str());
                return nullptr;
            }

            vertexByteCode = vertexShaderStream->ReadBytes();
            fragmentByteCode = fragmentShaderStream->ReadBytes();
        }
        else*/
        {
#if defined(ALIMER_DISABLE_SHADER_COMPILER)
            ALIMER_LOGCRITICAL("Shader compilation is not allowed");
            return nullptr;
#else
            auto vertexShaderStream = FileSystem::Get().Open(vertexShaderFile);
            auto fragmentShaderStream = FileSystem::Get().Open(fragmentShaderFile);

            // Compile from source GLSL.
            string vertexShader = vertexShaderStream->ReadAllText();
            string fragmentShader = fragmentShaderStream->ReadAllText();
            string errorLog;
            vertexByteCode = ShaderCompiler::Compile(vertexShader, vertexShaderStream->GetName(), ShaderStage::Vertex, errorLog);
            if (!errorLog.empty())
            {
                ALIMER_LOGCRITICAL("Vertex shader compilation failed: \n {}", errorLog);
            }

            fragmentByteCode = ShaderCompiler::Compile(fragmentShader, fragmentShaderStream->GetName(), ShaderStage::Fragment, errorLog);
            if (!errorLog.empty())
            {
                ALIMER_LOGCRITICAL("Fragment shader compilation failed: \n {}", errorLog);
            }
#endif
        }

        return CreateShader(
            vertexByteCode.data(), vertexByteCode.size(),
            fragmentByteCode.data(), fragmentByteCode.size()
        );
    }

    void Graphics::AddGpuResource(GpuResource* resource)
    {
        lock_guard<mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void Graphics::RemoveGpuResource(GpuResource* resource)
    {
        lock_guard<mutex> lock(_gpuResourceMutex);
        auto it = std::find(_gpuResources.begin(), _gpuResources.end(), resource);
        if (it != _gpuResources.end())
        {
            _gpuResources.erase(it);
        }
    }
}
