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

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#pragma warning(push)
#pragma warning(disable : 4467)
#include <wrl.h>
#pragma warning(pop)

#include <vector>

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}


class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

inline void D3D12SetObjectName(ID3D12Object* object, _In_z_  LPCWSTR name)
{
#if defined(ALIMER_DEV)
		object->SetName(name);
#endif
}

#include "Graphics/Graphics.h"
using namespace Microsoft::WRL;

namespace Alimer
{
	/// D3D12 Low-level 3D graphics API class.
	class D3D12Graphics final : public Graphics
	{
	public:
		/// Constructor.
		D3D12Graphics();

		/// Destructor.
		virtual ~D3D12Graphics() override;

		bool Initialize(std::shared_ptr<Window> window) override;
		bool WaitIdle() override;
		std::shared_ptr<Texture> BeginFrame() override;
		bool Present() override;

		inline IDXGIFactory4* GetDXGIFactory() const { return _factory.Get(); }

	private:
		bool InitializeCaps();
		void CreateCommandQueues();
		void CreateSwapchain(std::shared_ptr<Window> window);
		bool MoveToNextFrame();

		static constexpr UINT FrameCount = 2;

		ComPtr<IDXGIFactory4> _factory;
		ComPtr<IDXGIAdapter1> _adapter;
		ComPtr<ID3D12Device> _d3dDevice;
		ComPtr<ID3D12CommandQueue> _d3dDirectQueue;
		ComPtr<ID3D12CommandQueue> _d3dAsyncComputeQueue;
		ComPtr<IDXGISwapChain3> _swapChain;
		ComPtr<ID3D12Resource> _renderTargets[FrameCount];
		std::vector<std::shared_ptr<Texture>> _textures;

		bool _useWarpDevice{ false };

		// Synchronization objects.
		uint32_t _frameIndex{};
		HANDLE _fenceEvent;
		ComPtr<ID3D12Fence> _d3dFence;
		UINT64 _fenceValues[FrameCount] = {};
	};
}
