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

#include "vaudio/vaudio.h"
#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#   include <malloc.h>
#   undef    alloca
#   define   alloca _malloca
#   define   freea  _freea
#else
#   include <alloca.h>
#endif

#include <stdio.h> /* vsnprintf */

#define VAUDIO_ASSERT(c) assert(c)
#define VAUDIO_ALLOC(type) ((type*) malloc(sizeof(type)))
#define VAUDIO_FREE(ptr) free(ptr)
#define VAUDIO_ALLOC_HANDLE(type) ((type) calloc(1, sizeof(type##_T)))

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4091)   // 'typedef ': ignored on left of '' when no variable is declared
#endif

#include <audioclient.h>
#include <mmdeviceapi.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

HRESULT vaudioWinCoInitialize(void) {
#ifdef __WINRT__
    return S_OK;
#else
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (hr == RPC_E_CHANGED_MODE) {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    }

    /* S_FALSE means success, but someone else already initialized. */
    /* You still need to call CoUninitialize in this case! */
    if (hr == S_FALSE) {
        return S_OK;
    }

    return hr;
#endif
}

void vaudioWinCoUninitialize(void) {
#ifndef __WINRT__
    CoUninitialize();
#endif
}

// Proxy log callback
#define VGPU_MAX_LOG_MESSAGE 4096
static vaudio_log_fn s_logCallback = nullptr;

static void vaudio_log(VAudioLogLevel level, const char* message) {
    if (s_logCallback) {
        s_logCallback(level, "WASAPI", message);
    }
}

static void vaudio_log_format(VAudioLogLevel level, const char* format, ...) {
    char logMessage[VGPU_MAX_LOG_MESSAGE];
    va_list args;
    va_start(args, format);
    vsnprintf(logMessage, VGPU_MAX_LOG_MESSAGE, format, args);
    if (s_logCallback) {
        s_logCallback(level, "WASAPI", logMessage);
    }
    va_end(args);
}

static VAudioResult vaudio_post_error(const char* message, VAudioResult result) {
    vaudio_log(VAUDIO_LOG_LEVEL_ERROR, message);
    return result;
}

class NotificationClient final : public IMMNotificationClient
{
    LONG refCount = 1;

public:
    NotificationClient() {}

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG newRefCount = InterlockedDecrement(&refCount);
        if (!newRefCount) {
            delete this;
        }

        return newRefCount;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface) override
    {
        if (riid == IID_IUnknown)
        {
            AddRef();
            *ppvInterface = this;
        }
        else if (riid == __uuidof(IMMNotificationClient))
        {
            AddRef();
            *ppvInterface = this;
        }
        else
        {
            *ppvInterface = nullptr;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override
    {
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override
    {
        if (role == eConsole)
        {
            if (flow == eRender)
            {
                // TODO: implement
            }
            else if (flow == eCapture)
            {
                // TODO: implement
            }
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override
    {
        return S_OK;
    }
};

/* Global objects */
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

struct {
    IMMDeviceEnumerator*    enumerator = nullptr;
    IMMNotificationClient*  notificationClient = nullptr;
} _wasapi;

/* Handle declaration */
typedef struct VAudioDevice_T {
    IAudioClient*           audioClient = nullptr;
    VAudioSampleFormat      sampleFormat = VAUDIO_SAMPLE_FORMAT_UNKNOWN;
    uint32_t                sampleSize = 0;
    UINT32                  bufferFrameCount = 0;
    uint32_t                bufferSize = 0;
    IAudioRenderClient*     renderClient = nullptr;
    HANDLE                  notifyEvent = nullptr;
    HANDLE                  thread = nullptr;
    bool                    started = false;
    bool                    running = false;
} VAudioDevice_T;

static DWORD WINAPI vaudio_worker_thread(LPVOID param) {
    VAudioDevice device = (VAudioDevice)param;

    while (device->running)
    {
        WaitForSingleObject(device->notifyEvent, INFINITE);

        if (!device->running) {
            break;
        }

        UINT32 padding = 0;
        if (FAILED(device->audioClient->GetCurrentPadding(&padding))) {
            continue;
        }

        UINT32 frameCount = device->bufferFrameCount - padding;
        BYTE* renderBuffer;
        if (FAILED(device->renderClient->GetBuffer(frameCount, &renderBuffer))) {
            return 0;
        }

        device->renderClient->ReleaseBuffer(frameCount, 0);
    }

    return 0;
}

VAudioBackend vaudioGetBackend() {
    return VAUDIO_BACKEND_WASAPI;
}

VAudioResult vaudioDeviceInitialize(VAudioDeviceDescriptor* descriptor, VAudioDevice* pDevice) {
    VAUDIO_ASSERT(descriptor);
    VAUDIO_ASSERT(pDevice);

    HRESULT hr = S_OK;
    if (_wasapi.enumerator == nullptr) {
        vaudioWinCoInitialize();

        hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, reinterpret_cast<LPVOID*>(&_wasapi.enumerator));
        if (FAILED(hr)) {
            return vaudio_post_error("Failed to create IMMDeviceEnumerator.", VAUDIO_ERROR_INITIALIZATION_FAILED);
        }

        _wasapi.notificationClient = new NotificationClient();
        if (FAILED(hr = _wasapi.enumerator->RegisterEndpointNotificationCallback(_wasapi.notificationClient))) {
            return vaudio_post_error("Failed to get audio endpoint", VAUDIO_ERROR_INITIALIZATION_FAILED);
        }
    }

    VAudioDevice context = VAUDIO_ALLOC_HANDLE(VAudioDevice);

    IMMDevice* device = nullptr;
    EDataFlow deviceType = eRender;
    if (descriptor->deviceType == VAUDIO_DEVICE_TYPE_CAPTURE) {
        deviceType = eCapture;
    }

    if (FAILED(hr = _wasapi.enumerator->GetDefaultAudioEndpoint(deviceType, eConsole, &device))) {
        return vaudio_post_error("Failed to get audio endpoint", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    if (FAILED(hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&context->audioClient)))) {
        return vaudio_post_error("Failed to activate audio device", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    WAVEFORMATEX* audioClientWaveFormat;

    if (FAILED(hr = context->audioClient->GetMixFormat(&audioClientWaveFormat))) {
        return vaudio_post_error("Failed to get audio mix format", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    WAVEFORMATEX waveFormat;
    memset(&waveFormat, 0, sizeof(waveFormat));
    waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveFormat.nChannels = descriptor->channels > 0 ? descriptor->channels : audioClientWaveFormat->nChannels;
    waveFormat.nSamplesPerSec = descriptor->sampleRate > 0 ? descriptor->sampleRate : 44100;
    waveFormat.wBitsPerSample = 32;
    waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

    DWORD streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

    if (waveFormat.nSamplesPerSec != audioClientWaveFormat->nSamplesPerSec) {
        streamFlags |= AUDCLNT_STREAMFLAGS_RATEADJUST;
    }

    CoTaskMemFree(audioClientWaveFormat);

    if (FAILED(hr = context->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, 0, 0, &waveFormat, nullptr)))
    {
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.wBitsPerSample = 16;

        if (FAILED(hr = context->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, 0, 0, &waveFormat, nullptr))) {
            return vaudio_post_error("Failed to initialize audio client", VAUDIO_ERROR_INITIALIZATION_FAILED);
        }

        context->sampleFormat = VAUDIO_SAMPLE_FORMAT_SINT16;
        context->sampleSize = sizeof(int16_t);
    }
    else
    {
        context->sampleFormat = VAUDIO_SAMPLE_FORMAT_FLOAT32;
        context->sampleSize = sizeof(float);
    }

    // init output device
    if (FAILED(hr = context->audioClient->GetBufferSize(&context->bufferFrameCount))) {
        return vaudio_post_error("Failed to get audio buffer size", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    vaudio_log_format(VAUDIO_LOG_LEVEL_INFO, "Detected %u channels", waveFormat.nChannels);
    vaudio_log_format(VAUDIO_LOG_LEVEL_INFO, "Audio buffer frames: %u", context->bufferFrameCount);

    context->bufferSize = context->bufferFrameCount * waveFormat.nChannels;

    if (FAILED(hr = context->audioClient->GetService(IID_IAudioRenderClient, reinterpret_cast<void**>(&context->renderClient)))) {
        return vaudio_post_error("Failed to get render client service", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    context->notifyEvent = CreateEventW(nullptr, false, false, nullptr);
    if (!context->notifyEvent) {
        return vaudio_post_error("Failed to create event", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    if (FAILED(hr = context->audioClient->SetEventHandle(context->notifyEvent)))
        return vaudio_post_error("Failed to set event handle", VAUDIO_ERROR_INITIALIZATION_FAILED);

    if (FAILED(hr = context->audioClient->Start())) {
        return vaudio_post_error("Failed to start audio", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    context->started = true;
    context->running = true;
    device->Release();

    /* create streaming thread */
    context->thread = CreateThread(NULL, 0, vaudio_worker_thread, context, 0, 0);
    if (!context->thread) {
        return vaudio_post_error("CreateThread failed", VAUDIO_ERROR_INITIALIZATION_FAILED);
    }

    *pDevice = context;
    return VAUDIO_SUCCESS;
}

void vaudioDeviceShutdown(VAudioDevice device) {
    if (!device->started) {
        return;
    }

    device->running = false;
    if (device->notifyEvent) {
        SetEvent(device->notifyEvent);
        WaitForSingleObject(device->thread, INFINITE);
        CloseHandle(device->thread);
        CloseHandle(device->notifyEvent);

        device->notifyEvent = nullptr;
        device->thread = nullptr;
    }

    if (device->renderClient) {
        device->renderClient->Release();
    }

    if (device->audioClient)
    {
        if (device->started) {
            device->audioClient->Stop();
        }

        device->audioClient->Release();
    }

    if (_wasapi.enumerator) {
        _wasapi.enumerator->UnregisterEndpointNotificationCallback(_wasapi.notificationClient);
        _wasapi.notificationClient->Release();
        _wasapi.enumerator->Release();
        _wasapi.enumerator = nullptr;
    }

    device->started = false;
    vaudioWinCoUninitialize();
}
