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

#if ALIMER_D3D12
#include "Graphics/D3D12/D3D12Graphics.h"
#endif

using namespace std;

namespace Alimer
{
    static Graphics* __graphicsInstance = nullptr;

    Graphics::Graphics(GraphicsDeviceType deviceType, bool validation)
        : _deviceType(deviceType)
        , _validation(validation)
        , _adapter(nullptr)
    {
        __graphicsInstance = this;
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

        __graphicsInstance = nullptr;
    }

    Graphics* Graphics::GetInstance()
    {
        return __graphicsInstance;
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

        case GraphicsDeviceType::Default:
            break;

        case GraphicsDeviceType::Empty:
        default:
            break;
        }

        return graphics;
    }

    bool Graphics::Initialize(GpuAdapter* adapter, const SharedPtr<Window>& window)
    {
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
        return BackendInitialize();
    }

    SharedPtr<Shader> Graphics::CreateShader(const string& vertexShaderFile, const std::string& fragmentShaderFile)
    {
        auto vertexShaderStream = gResources()->Open(vertexShaderFile + ".glsl");
        auto fragmentShaderStream = gResources()->Open(fragmentShaderFile + ".glsl");

        // Lookup for GLSL shader.
        if (!vertexShaderStream)
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", vertexShaderFile.c_str());
            return nullptr;
        }

        if (!fragmentShaderStream)
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", fragmentShaderFile.c_str());
            return nullptr;
        }

        // Compile GLSL.
        string vertexShader = vertexShaderStream->ReadAllText();
        string fragmentShader = fragmentShaderStream->ReadAllText();
        string errorLog;
        vector<uint32_t> vertexByteCode = ShaderCompiler::Compile(vertexShader, vertexShaderStream->GetName(), ShaderStage::Vertex, errorLog);
        vector<uint32_t> fragmentByteCode = ShaderCompiler::Compile(fragmentShader, fragmentShaderStream->GetName(), ShaderStage::Fragment, errorLog);

        ShaderStageDescription vertex = {};
        vertex.code = vertexByteCode;
        vertex.entryPoint = "main";

        ShaderStageDescription fragment = {};
        fragment.code = fragmentByteCode;
        fragment.entryPoint = "main";

        return CreateShader(vertex, fragment);
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

    Graphics& gGraphics()
    {
        return *__graphicsInstance;
    }
}
