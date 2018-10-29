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

#include "Graphics/Texture.h"
#include "D3D11Prerequisites.h"
#include <unordered_map>

namespace Alimer
{
    class D3D11GraphicsDevice;

    /// D3D11 Texture implementation.
    class D3D11Texture final : public Texture
    {
    public:
        /// Constructor.
        D3D11Texture(D3D11GraphicsDevice* device, const TextureDescriptor* descriptor, const ImageLevel* initialData, ID3D11Texture2D* nativeTexture);

        /// Destructor.
        ~D3D11Texture() override;

        void Destroy() override;

        void InvalidateViews();

        ID3D11Device*  GetD3DDevice() const { return _d3dDevice; }
        ID3D11Resource* GetResource() const { return _resource; }
        DXGI_FORMAT GetDXGIFormat() const { return _dxgiFormat; }

        ID3D11ShaderResourceView* GetSRV(uint32_t mostDetailedMip = 0, uint32_t mipCount = RemainingMipLevels, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingMipLevels) const;
        ID3D11UnorderedAccessView* GetUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingMipLevels) const;
        ID3D11RenderTargetView* GetRTV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingMipLevels) const;
        ID3D11DepthStencilView* GetDSV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingMipLevels) const;

    private:
        ID3D11Device* _d3dDevice;
        DXGI_FORMAT _dxgiFormat;

        union {
            ID3D11Resource* _resource;
            ID3D11Texture1D* _texture1D;
            ID3D11Texture2D* _texture2D;
            ID3D11Texture3D* _texture3D;
        };

       
        struct ViewInfoHashFunc
        {
            std::size_t operator()(const ResourceViewInfo& v) const
            {
                return ((std::hash<uint32_t>()(v.firstArraySlice)
                    ^ (std::hash<uint32_t>()(v.arraySize) << 1)) >> 1)
                    ^ (std::hash<uint32_t>()(v.mipCount) << 1)
                    ^ (std::hash<uint32_t>()(v.mostDetailedMip) << 3);
            }
        };

        mutable std::unordered_map<ResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, ViewInfoHashFunc> _srvs;
        mutable std::unordered_map<ResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>, ViewInfoHashFunc> _uavs;
        mutable std::unordered_map<ResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, ViewInfoHashFunc> _rtvs;
        mutable std::unordered_map<ResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, ViewInfoHashFunc> _dsvs;
    };
}
