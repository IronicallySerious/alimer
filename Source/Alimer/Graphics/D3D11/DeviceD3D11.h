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

#include "../../Graphics/GPUDevice.h"
#include "D3D11Cache.h"

namespace Alimer
{
    class D3D11CommandContext;

    /// D3D11 graphics implementation.
    class DeviceD3D11 final : public GPUDevice
    {
    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        DeviceD3D11(bool validation);

        /// Destructor.
        ~DeviceD3D11() override;

        bool Initialize(const RenderWindowDescriptor* mainWindowDescriptor) override;
        bool WaitIdle() override;

        Buffer* CreateBufferCore(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData, const std::string& name) override;
        Texture* CreateTextureCore(TextureType type, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format,
            TextureUsage usage, SampleCount samples, const void* initialData)  override;
        Sampler* CreateSamplerCore(const SamplerDescriptor* descriptor) override;
        Framebuffer* CreateFramebufferCore(uint32_t colorAttachmentsCount, const FramebufferAttachment* colorAttachments, const FramebufferAttachment* depthStencilAttachment) override;

        void HandleDeviceLost();

        // Getters
        inline IDXGIFactory1*   GetFactory() const { return _factory.Get(); }
        inline IDXGIAdapter1*   GetAdapter() const { return _adapter.Get(); }

        ID3D11Device*           GetD3DDevice() const { return _d3dDevice.Get(); }
        ID3D11Device1*          GetD3DDevice1() const { return _d3dDevice1.Get(); }
        ID3D11DeviceContext*    GetD3DDeviceContext() const { return _d3dContext.Get(); }
        ID3D11DeviceContext1*   GetD3DDeviceContext1() const { return _d3dContext1.Get(); }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return _d3dFeatureLevel; }
        
        uint32_t                GetShaderModerMajor() const { return _shaderModelMajor; }
        uint32_t                GetShaderModerMinor() const { return _shaderModelMinor; }
        bool                    AllowTearing() const { return _allowTearing; }

        D3D11Cache&             GetCache();

    private:
        void InitializeCaps();
        void GenerateScreenshot(const std::string& fileName);

        D3D_FEATURE_LEVEL                                   _d3dFeatureLevel;

        Microsoft::WRL::ComPtr<IDXGIFactory4>               _factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1>               _adapter;
        Microsoft::WRL::ComPtr<ID3D11Device>                _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11Device1>               _d3dDevice1;

        Microsoft::WRL::ComPtr<ID3D11DeviceContext>         _d3dContext;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>        _d3dContext1;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>   _d3dAnnotation;

        uint32_t                                            _shaderModelMajor = 4;
        uint32_t                                            _shaderModelMinor = 0;
        bool                                                _allowTearing = false;

        D3D11Cache                                          _cache;
    };
}
