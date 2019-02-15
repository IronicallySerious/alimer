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

#include "../Texture.h"
#include "BackendD3D11.h"
#include <unordered_map>

namespace alimer
{
    /// D3D11 Texture implementation.
    class TextureD3D11 final : public Texture
    {
    public:
        /// Constructor.
        TextureD3D11(DeviceD3D11* device, const TextureDescriptor* descriptor, void* nativeTexture, const void* initialData);

        /// Destructor.
        ~TextureD3D11() override;

        /// Destroy
        void Destroy() override;

        void InvalidateViews();

        ID3D11Device*       GetD3DDevice() const { return _d3dDevice; }
        ID3D11Resource*     GetResource() const { return _resource; }
        ID3D11Texture2D*    GetD3DTexture2D() const { return _texture2D; }
        DXGI_FORMAT         GetDXGIFormat() const { return _dxgiFormat; }

        //ID3D11RenderTargetView* getRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t arraySize = MaxPossible);
        ID3D11ShaderResourceView* GetSRV(uint32_t mostDetailedMip = 0, uint32_t mipCount = RemainingMipLevels, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingArrayLayers) const;
        ID3D11UnorderedAccessView* GetUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingArrayLayers) const;

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
            std::size_t operator()(const D3DResourceViewInfo& v) const
            {
                return ((std::hash<uint32_t>()(v.firstArraySlice)
                    ^ (std::hash<uint32_t>()(v.arraySize) << 1)) >> 1)
                    ^ (std::hash<uint32_t>()(v.mipCount) << 1)
                    ^ (std::hash<uint32_t>()(v.mostDetailedMip) << 3);
            }
        };

        mutable std::unordered_map<D3DResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, ViewInfoHashFunc> _srvs;
        mutable std::unordered_map<D3DResourceViewInfo, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>, ViewInfoHashFunc> _uavs;
    };
}
