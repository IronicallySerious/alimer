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
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Core/Log.h"

#if ALIMER_VULKAN
#include "Graphics/Vulkan/VulkanGraphics.h"
#endif

#if ALIMER_D3D12
#include "Graphics/D3D12/D3D12Graphics.h"
#endif

namespace Alimer
{
    Alimer::Graphics* graphics = nullptr;

    Graphics::Graphics(GraphicsDeviceType deviceType)
        : _deviceType(deviceType)
        , _window(nullptr)
    {
        graphics = this;
    }

    Graphics::~Graphics()
    {
        Finalize();
        graphics = nullptr;
    }

    void Graphics::Finalize()
    {

    }

    std::set<GraphicsDeviceType> Graphics::GetAvailableBackends()
    {
        static std::set<GraphicsDeviceType> availableBackends;

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
#if ALIMER_D3D12
                ALIMER_LOGINFO("Using Vulkan graphics backend");
                graphics = new VulkanGraphics(validation, applicationName);
#else
                ALIMER_LOGERROR("Vulkan graphics backend not supported");
#endif
                break;

            case GraphicsDeviceType::Direct3D12:
#if ALIMER_D3D12
                ALIMER_LOGINFO("Using Direct3D 12 graphics backend");
                graphics = new D3D12Graphics();
#else
                ALIMER_LOGERROR("Direct3D 12 graphics backend not supported");
#endif
                break;

            case GraphicsDeviceType::Default:
                break;

            case GraphicsDeviceType::Empty:
            default:
                break;
        }

        return graphics;
    }

    bool Graphics::Initialize(const SharedPtr<Window>& window)
    {
        _window = window;
        return true;
    }

    SharedPtr<Shader> Graphics::CreateShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
    {
        std::string baseShaderUrl = "assets://shaders/";

        // Lookup for GLSL shader.
        if (!FileSystem::Get().FileExists(baseShaderUrl + vertexShaderFile + ".glsl"))
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", vertexShaderFile.c_str());
            return nullptr;
        }

        if (!FileSystem::Get().FileExists(baseShaderUrl + fragmentShaderFile + ".glsl"))
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", fragmentShaderFile.c_str());
            return nullptr;
        }

        // Compile GLSL.
        std::string errorLog;
        std::vector<uint32_t> vertexByteCode = ShaderCompiler::Compile(baseShaderUrl + vertexShaderFile + ".glsl", errorLog);
        std::vector<uint32_t> fragmentByteCode = ShaderCompiler::Compile(baseShaderUrl + fragmentShaderFile + ".glsl", errorLog);

        ShaderStageDescription vertex = {};
        vertex.byteCode = vertexByteCode.data();
        vertex.byteCodeSize = vertexByteCode.size();
        vertex.entryPoint = "main";

        ShaderStageDescription fragment = {};
        fragment.byteCode = fragmentByteCode.data();
        fragment.byteCodeSize = fragmentByteCode.size();
        fragment.entryPoint = "main";

        return CreateShader(vertex, fragment);
    }
}
