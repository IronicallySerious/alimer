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

#include "../GraphicsDevice.h"
#include "D3D11Cache.h"

namespace alimer
{
    class SwapChainD3D11;

    /// D3D11 graphics implementation.
    class GraphicsDeviceD3D11 final : public GraphicsDevice
    {
    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        GraphicsDeviceD3D11(const GraphicsDeviceDescriptor* descriptor);

        /// Destructor.
        ~GraphicsDeviceD3D11() override;

        bool InitializeImpl(const SwapChainDescriptor* descriptor) override;
        void Tick() override;

        Texture* CreateTextureImpl(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData);
        Framebuffer* CreateFramebufferImpl(const FramebufferDescriptor* descriptor);
        Buffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* pInitData);
        Sampler* CreateSamplerImpl(const SamplerDescriptor* descriptor);
        Shader* CreateShaderImpl(const ShaderDescriptor* descriptor);

        void HandleDeviceLost();

        // Getters
        inline IDXGIFactory1*   GetFactory() const { return _factory.Get(); }
        inline IDXGIAdapter1*   GetAdapter() const { return _adapter.Get(); }

        ID3D11Device*           GetD3DDevice() const { return _d3dDevice.Get(); }
        ID3D11Device1*          GetD3DDevice1() const { return _d3dDevice1.Get(); }
        ID3D11DeviceContext*    GetD3DDeviceContext() const { return _d3dContext; }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return _d3dFeatureLevel; }
        
        uint32_t                GetShaderModerMajor() const { return _shaderModelMajor; }
        uint32_t                GetShaderModerMinor() const { return _shaderModelMinor; }
        bool                    AllowTearing() const { return _allowTearing; }

        D3D11Cache&             GetCache();

    private:
        void InitializeCaps();
        //void GenerateScreenshot(const std::string& fileName);


        Microsoft::WRL::ComPtr<IDXGIFactory1>               _factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1>               _adapter;
        Microsoft::WRL::ComPtr<ID3D11Device>                _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11Device1>               _d3dDevice1;

        D3D_FEATURE_LEVEL _d3dFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1;
        ID3D11DeviceContext* _d3dContext = nullptr;

        uint32_t                                            _shaderModelMajor = 4;
        uint32_t                                            _shaderModelMinor = 0;
        bool                                                _allowTearing = false;
        D3D11Cache                                          _cache;
    };
}