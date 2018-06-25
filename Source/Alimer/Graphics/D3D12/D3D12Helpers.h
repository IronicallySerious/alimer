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


#include "D3D12Prerequisites.h"

#if !ALIMER_PLATFORM_UWP
extern decltype(CreateDXGIFactory2)* CreateDXGIFactory2Fn;
extern PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterfaceFn;
extern PFN_D3D12_CREATE_DEVICE D3D12CreateDeviceFn;
extern PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignatureFn;
extern PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignatureFn;
HRESULT D3D12LoadLibraries();
#endif

HRESULT privateD3D12GetDebugInterface(_In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug);
HRESULT privateCreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);
HRESULT privateD3D12CreateDevice(_In_opt_ IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, _In_ REFIID riid, _COM_Outptr_opt_ void** ppDevice);
HRESULT privateD3D12SerializeRootSignature(
    _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
    _In_ D3D_ROOT_SIGNATURE_VERSION Version,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob);

HRESULT privateD3D12SerializeVersionedRootSignature(
    _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob);

//------------------------------------------------------------------------------------------------
// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
// To help enable root signature 1.1 features when they are available and not require maintaining
// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
// 1.1 is not supported.
// Copied to use dynamic dll version on Windows.
inline HRESULT AlimerD3DX12SerializeVersionedRootSignature(
    _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
    D3D_ROOT_SIGNATURE_VERSION MaxVersion,
    _Outptr_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
    if (ppErrorBlob != NULL)
    {
        *ppErrorBlob = NULL;
    }

    switch (MaxVersion)
    {
    case D3D_ROOT_SIGNATURE_VERSION_1_0:
        switch (pRootSignatureDesc->Version)
        {
        case D3D_ROOT_SIGNATURE_VERSION_1_0:
            return privateD3D12SerializeRootSignature(&pRootSignatureDesc->Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);

        case D3D_ROOT_SIGNATURE_VERSION_1_1:
        {
            HRESULT hr = S_OK;
            const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

            const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
            void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : NULL;
            if (ParametersSize > 0 && pParameters == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            D3D12_ROOT_PARAMETER* pParameters_1_0 = reinterpret_cast<D3D12_ROOT_PARAMETER*>(pParameters);

            if (SUCCEEDED(hr))
            {
                for (UINT n = 0; n < desc_1_1.NumParameters; n++)
                {
                    __analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
                    pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
                    pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

                    switch (desc_1_1.pParameters[n].ParameterType)
                    {
                    case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                        pParameters_1_0[n].Constants.Num32BitValues = desc_1_1.pParameters[n].Constants.Num32BitValues;
                        pParameters_1_0[n].Constants.RegisterSpace = desc_1_1.pParameters[n].Constants.RegisterSpace;
                        pParameters_1_0[n].Constants.ShaderRegister = desc_1_1.pParameters[n].Constants.ShaderRegister;
                        break;

                    case D3D12_ROOT_PARAMETER_TYPE_CBV:
                    case D3D12_ROOT_PARAMETER_TYPE_SRV:
                    case D3D12_ROOT_PARAMETER_TYPE_UAV:
                        pParameters_1_0[n].Descriptor.RegisterSpace = desc_1_1.pParameters[n].Descriptor.RegisterSpace;
                        pParameters_1_0[n].Descriptor.ShaderRegister = desc_1_1.pParameters[n].Descriptor.ShaderRegister;
                        break;

                    case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                        const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 = desc_1_1.pParameters[n].DescriptorTable;

                        const SIZE_T DescriptorRangesSize = sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
                        void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr)) ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize) : NULL;
                        if (DescriptorRangesSize > 0 && pDescriptorRanges == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        D3D12_DESCRIPTOR_RANGE* pDescriptorRanges_1_0 = reinterpret_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

                        if (SUCCEEDED(hr))
                        {
                            for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++)
                            {
                                __analysis_assume(DescriptorRangesSize == sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
                                pDescriptorRanges_1_0[x].BaseShaderRegister = table_1_1.pDescriptorRanges[x].BaseShaderRegister;
                                pDescriptorRanges_1_0[x].NumDescriptors = table_1_1.pDescriptorRanges[x].NumDescriptors;
                                pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart = table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
                                pDescriptorRanges_1_0[x].RangeType = table_1_1.pDescriptorRanges[x].RangeType;
                                pDescriptorRanges_1_0[x].RegisterSpace = table_1_1.pDescriptorRanges[x].RegisterSpace;
                            }
                        }

                        D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
                        table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
                        table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                CD3DX12_ROOT_SIGNATURE_DESC desc_1_0(desc_1_1.NumParameters, pParameters_1_0, desc_1_1.NumStaticSamplers, desc_1_1.pStaticSamplers, desc_1_1.Flags);
                hr = privateD3D12SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
            }

            if (pParameters)
            {
                for (UINT n = 0; n < desc_1_1.NumParameters; n++)
                {
                    if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                    {
                        HeapFree(GetProcessHeap(), 0, reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(pParameters_1_0[n].DescriptorTable.pDescriptorRanges)));
                    }
                }
                HeapFree(GetProcessHeap(), 0, pParameters);
            }
            return hr;
        }
        }
        break;

    case D3D_ROOT_SIGNATURE_VERSION_1_1:
        return privateD3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
    }

    return E_INVALIDARG;
}

namespace Alimer
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICAL("HRESULT of 0x%08X", static_cast<UINT>(hr));
        }
    }

	inline void D3D12SetObjectName(ID3D12Object* object, _In_z_  LPCWSTR name)
	{
#if defined(ALIMER_DEV)
		object->SetName(name);
#endif
	}

    inline int GetRefCount(IUnknown* _interface)
    {
        _interface->AddRef();
        return _interface->Release();
    }

	class D3D12Resource
	{
	public:
		ID3D12Resource * operator->() { return _resource.Get(); }
		const ID3D12Resource* operator->() const { return _resource.Get(); }

		ID3D12Resource* GetResource() { return _resource.Get(); }
		const ID3D12Resource* GetResource() const { return _resource.Get(); }

		D3D12_RESOURCE_STATES GetUsageState() const { return _usageState; }
		void SetUsageState(D3D12_RESOURCE_STATES state) { _usageState = state; }

		D3D12_RESOURCE_STATES GetTransitioningState() const { return _transitioningState; }
		void SetTransitioningState(D3D12_RESOURCE_STATES state) { _transitioningState = state; }

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return _gpuVirtualAddress; }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
		D3D12_RESOURCE_STATES _usageState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES _transitioningState = (D3D12_RESOURCE_STATES)-1;
		D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	};

	namespace d3d12
	{
		static inline DXGI_FORMAT Convert(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::R8UNorm:
					return DXGI_FORMAT_R8_UNORM;
				case PixelFormat::RG8UNorm:
					return DXGI_FORMAT_R8G8_UNORM;
				case PixelFormat::RGBA8UNorm:
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				case PixelFormat::BGRA8UNorm:
					return DXGI_FORMAT_B8G8R8A8_UNORM;

				case PixelFormat::Undefined:
				default:
					return DXGI_FORMAT_UNKNOWN;
			}
		}

		static inline D3D12_PRIMITIVE_TOPOLOGY Convert(PrimitiveTopology topology)
		{
			switch (topology)
			{
				case PrimitiveTopology::Points:
					return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
				case PrimitiveTopology::Lines:
					return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
				case PrimitiveTopology::LineStrip:
					return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

				case PrimitiveTopology::Triangles:
					return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				case PrimitiveTopology::TriangleStrip:
					return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

				default:
					return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}
	}
}
