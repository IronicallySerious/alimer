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

#include "AudioWASAPI.h"
#include "../../Core/Log.h"
#include <functiondiscoverykeys_devpkey.h>

namespace Alimer
{
	class AudioNotificationClient final : public IMMNotificationClient
	{
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvInterface) override
		{
			if (IID_IUnknown == riid)
			{
				AddRef();
				*ppvInterface = (IUnknown *)this;
			}
			else if (__uuidof(IMMNotificationClient) == riid)
			{
				AddRef();
				*ppvInterface = (IMMNotificationClient *)this;
			}
			else
			{
				*ppvInterface = nullptr;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		ULONG STDMETHODCALLTYPE AddRef() override
		{
			return InterlockedIncrement(&_refs);
		}

		ULONG STDMETHODCALLTYPE Release() override
		{
			ULONG newRef = InterlockedDecrement(&_refs);
			if (0 == newRef)
			{
				delete this;
			}

			return newRef;
		}

		// IMMNotificationClient
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow InFlow, ERole InRole, LPCWSTR pwstrDeviceId) override
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override
		{
			return S_OK;
		}

	private:
		ULONG _refs{ 1 };
	};

	AudioWASAPI::AudioWASAPI()
		: _deviceEnumerator(nullptr)
		, _notificationClient(nullptr)
		, _renderAudioClient(nullptr)
		, _audioRender(nullptr)
	{
		HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator),
			(void**)&_deviceEnumerator);
		if (FAILED(hr))
		{
			ALIMER_LOGERROR("[WASAPI] - CoCreateInstance(MMDeviceEnumerator) failed, %08lx", hr);
		}

		_notificationClient = new AudioNotificationClient();
		_deviceEnumerator->RegisterEndpointNotificationCallback(_notificationClient);

		IMMDeviceCollection* renderDevices;
		hr = _deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &renderDevices);
		if (FAILED(hr))
		{
			_deviceEnumerator->Release();
			ALIMER_LOGERROR("[WASAPI] - Failed to enumerate audio endpoints.");
			return;
		}

		UINT renderDeviceCount;
		hr = renderDevices->GetCount(&renderDeviceCount);
		for (UINT i = 0; i < renderDeviceCount; ++i)
		{
			IMMDevice* pDevice;
			hr = renderDevices->Item(i, &pDevice);

			LPWSTR id;
			hr = pDevice->GetId(&id);
			IPropertyStore *devicePropStore;
			hr = pDevice->OpenPropertyStore(STGM_READ, &devicePropStore);

			PROPVARIANT deviceNameProp;
			PropVariantInit(&deviceNameProp);
			hr = devicePropStore->GetValue(PKEY_Device_FriendlyName, &deviceNameProp);
			std::wstring str(deviceNameProp.pwszVal);
			PropVariantClear(&deviceNameProp);
		}
	}


	AudioWASAPI::~AudioWASAPI()
	{
		if (_deviceEnumerator)
		{
			_deviceEnumerator->UnregisterEndpointNotificationCallback(_notificationClient);
			SafeRelease(_deviceEnumerator);
		}

		SafeRelease(_audioRender);
		SafeRelease(_renderAudioClient);
	}
}
