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

#pragma once

#include "../../Graphics/GraphicsDevice.h"
#include "D3D11Prerequisites.h"
#include <array>
#include <thread>

namespace Alimer
{
    class D3DPlatformFunctions;
    class D3D11Texture;
    class D3D11SwapChain;

    /// D3D11 graphics device implementation.
    class D3D11GraphicsDevice final : public GraphicsDevice
    {
    public:
        /// Constructor.
        D3D11GraphicsDevice(bool validation);

        /// Destructor.
        ~D3D11GraphicsDevice() override;

        bool Initialize(const RenderingSettings& settings) override;
        void Shutdown() override;
        bool WaitIdle() override;

        void PresentImpl() override;

        Framebuffer* GetSwapchainFramebuffer() const override;

        //RenderPass* CreateRenderPassImpl(const RenderPassDescription* descriptor) override;
        GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) override;
        //VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) override;
        //ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) override;
        //ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) override;

        Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) override;


        void HandleDeviceLost();

        ID3D11InputLayout* GetInputLayout(const InputLayoutDesc& desc);
        void StoreInputLayout(const InputLayoutDesc& desc, ID3D11InputLayout* layout);

        // Getters
        D3D_FEATURE_LEVEL GetFeatureLevel() const { return _d3dFeatureLevel; }
        ID3D11Device1* GetD3DDevice() const { return _d3dDevice; }
        ID3D11DeviceContext1* GetD3DDeviceContext1() const { return _d3dImmediateContext; }
        uint32_t GetShaderModerMajor() const { return _shaderModelMajor; }
        uint32_t GetShaderModerMinor() const { return _shaderModelMinor; }

        const D3DPlatformFunctions* GetFunctions() { return _functions; }

    private:
        void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
        bool InitializeCaps();

        void GenerateScreenshot(const std::string& fileName);

        D3DPlatformFunctions*       _functions = nullptr;
        D3D_FEATURE_LEVEL           _d3dMinFeatureLevel = D3D_FEATURE_LEVEL_10_0;
        D3D_FEATURE_LEVEL           _d3dFeatureLevel;

        Microsoft::WRL::ComPtr<ID3D11Device>            _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11Device1>           _d3dDevice1;

        Microsoft::WRL::ComPtr<ID3D11DeviceContext>     _d3dContext;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    _d3dContext1;
        ID3D11DeviceContext1*       _d3dImmediateContext = nullptr;
        ID3DUserDefinedAnnotation*  _d3dAnnotation = nullptr;

        D3D11SwapChain* _swapChain = nullptr;

        uint32_t _shaderModelMajor = 4;
        uint32_t _shaderModelMinor = 0;

        /// Input layouts.
        InputLayoutMap _inputLayouts;
    };
}
