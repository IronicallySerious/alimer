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

#include "Audio.XAudio2.h"
#include "../../Core/Log.h"

#if !ALIMER_PLATFORM_UWP
static const wchar_t* XAUDIO2_DLL_29 = L"xaudio2_9.dll";
static const wchar_t* XAUDIO2_DLL_28 = L"xaudio2_8.dll";
typedef HRESULT(__stdcall *XAudio2CreateProc)(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);


// XAudio 2.7 (June 2010 SDK)
static const CLSID CLSID_XAudio27 = { 0x5a508685, 0xa254,  0x4fba, {  0x9b,  0x82,  0x9a,  0x24,  0xb0,  0x03,  0x06, 0xaf } };
static const CLSID CLSID_XAudio27_Debug = { 0xdb05ea35,  0x0329,  0x4d4b, { 0xa5, 0x3a, 0x6d, 0xea, 0xd0, 0x3d, 0x38, 0x52 } };

// Used in XAUDIO2_DEVICE_DETAILS below to describe the types of applications
// that the user has specified each device as a default for.  0 means that the
// device isn't the default for any role.
typedef enum XAUDIO2_DEVICE_ROLE
{
    NotDefaultDevice = 0x0,
    DefaultConsoleDevice = 0x1,
    DefaultMultimediaDevice = 0x2,
    DefaultCommunicationsDevice = 0x4,
    DefaultGameDevice = 0x8,
    GlobalDefaultDevice = 0xf,
    InvalidDeviceRole = ~GlobalDefaultDevice
} XAUDIO2_DEVICE_ROLE;

// Returned by IXAudio2::GetDeviceDetails
typedef struct XAUDIO2_DEVICE_DETAILS
{
    WCHAR DeviceID[256];                // String identifier for the audio device.
    WCHAR DisplayName[256];             // Friendly name suitable for display to a human.
    XAUDIO2_DEVICE_ROLE Role;           // Roles that the device should be used for.
    WAVEFORMATEXTENSIBLE OutputFormat;  // The device's native PCM audio output format.
} XAUDIO2_DEVICE_DETAILS;


struct IXAudio2_7 : IUnknown
{

    // NAME: IXAudio2::GetDeviceCount
    // DESCRIPTION: Returns the number of audio output devices available.
    //
    // ARGUMENTS:
    //  pCount - Returns the device count.
    //
    STDMETHOD(GetDeviceCount) (THIS_ UINT32* pCount) PURE;

    // NAME: IXAudio2::GetDeviceDetails
    // DESCRIPTION: Returns information about the device with the given index.
    //
    // ARGUMENTS:
    //  Index - Index of the device to be queried.
    //  pDeviceDetails - Returns the device details.
    //
    STDMETHOD(GetDeviceDetails) (THIS_ UINT32 Index, void* pDeviceDetails) PURE;

    // NAME: IXAudio2::Initialize
    // DESCRIPTION: Sets global XAudio2 parameters and prepares it for use.
    //
    // ARGUMENTS:
    //  Flags - Flags specifying the XAudio2 object's behavior.  Currently unused.
    //  XAudio2Processor - An XAUDIO2_PROCESSOR enumeration value that specifies
    //  the hardware thread (Xbox) or processor (Windows) that XAudio2 will use.
    //  The enumeration values are platform-specific; platform-independent code
    //  can use XAUDIO2_DEFAULT_PROCESSOR to use the default on each platform.
    //
    STDMETHOD(Initialize) (THIS_ UINT32 Flags X2DEFAULT(0), UINT32 XAudio2Processor = 0x00000001) PURE;

    // NAME: IXAudio2::RegisterForCallbacks
    // DESCRIPTION: Adds a new client to receive XAudio2's engine callbacks.
    //
    // ARGUMENTS:
    //  pCallback - Callback interface to be called during each processing pass.
    //
    STDMETHOD(RegisterForCallbacks) (_In_ IXAudio2EngineCallback* pCallback) PURE;

    // NAME: IXAudio2::UnregisterForCallbacks
    // DESCRIPTION: Removes an existing receiver of XAudio2 engine callbacks.
    //
    // ARGUMENTS:
    //  pCallback - Previously registered callback interface to be removed.
    //
    STDMETHOD_(void, UnregisterForCallbacks) (_In_ IXAudio2EngineCallback* pCallback) PURE;

    // NAME: IXAudio2::CreateSourceVoice
    // DESCRIPTION: Creates and configures a source voice.
    //
    // ARGUMENTS:
    //  ppSourceVoice - Returns the new object's IXAudio2SourceVoice interface.
    //  pSourceFormat - Format of the audio that will be fed to the voice.
    //  Flags - XAUDIO2_VOICE flags specifying the source voice's behavior.
    //  MaxFrequencyRatio - Maximum SetFrequencyRatio argument to be allowed.
    //  pCallback - Optional pointer to a client-provided callback interface.
    //  pSendList - Optional list of voices this voice should send audio to.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateSourceVoice) (THIS_ _Outptr_ IXAudio2SourceVoice** ppSourceVoice,
        _In_ const WAVEFORMATEX* pSourceFormat,
        UINT32 Flags X2DEFAULT(0),
        float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO),
        _In_opt_ IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL),
        _In_opt_ const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
        _In_opt_ const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

    // NAME: IXAudio2::CreateSubmixVoice
    // DESCRIPTION: Creates and configures a submix voice.
    //
    // ARGUMENTS:
    //  ppSubmixVoice - Returns the new object's IXAudio2SubmixVoice interface.
    //  InputChannels - Number of channels in this voice's input audio data.
    //  InputSampleRate - Sample rate of this voice's input audio data.
    //  Flags - XAUDIO2_VOICE flags specifying the submix voice's behavior.
    //  ProcessingStage - Arbitrary number that determines the processing order.
    //  pSendList - Optional list of voices this voice should send audio to.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateSubmixVoice) (THIS_ _Outptr_ IXAudio2Voice** ppSubmixVoice,
        UINT32 InputChannels, UINT32 InputSampleRate,
        UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0),
        _In_opt_ const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
        _In_opt_ const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;


    // NAME: IXAudio2::CreateMasteringVoice
    // DESCRIPTION: Creates and configures a mastering voice.
    //
    // ARGUMENTS:
    //  ppMasteringVoice - Returns the new object's IXAudio2MasteringVoice interface.
    //  InputChannels - Number of channels in this voice's input audio data.
    //  InputSampleRate - Sample rate of this voice's input audio data.
    //  Flags - XAUDIO2_VOICE flags specifying the mastering voice's behavior.
    //  DeviceIndex - Identifier of the device to receive the output audio.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateMasteringVoice) (THIS_ IXAudio2MasteringVoice** ppMasteringVoice,
        UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
        UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
        UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0),
        const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

    // NAME: IXAudio2::StartEngine
    // DESCRIPTION: Creates and starts the audio processing thread.
    //
    STDMETHOD(StartEngine) (THIS) PURE;

    // NAME: IXAudio2::StopEngine
    // DESCRIPTION: Stops and destroys the audio processing thread.
    //
    STDMETHOD_(void, StopEngine) (THIS) PURE;

    // NAME: IXAudio2::CommitChanges
    // DESCRIPTION: Atomically applies a set of operations previously tagged
    //              with a given identifier.
    //
    // ARGUMENTS:
    //  OperationSet - Identifier of the set of operations to be applied.
    //
    STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet) PURE;

    // NAME: IXAudio2::GetPerformanceData
    // DESCRIPTION: Returns current resource usage details: memory, CPU, etc.
    //
    // ARGUMENTS:
    //  pPerfData - Returns the performance data structure.
    //
    STDMETHOD_(void, GetPerformanceData) (THIS_ _Out_ XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;

    // NAME: IXAudio2::SetDebugConfiguration
    // DESCRIPTION: Configures XAudio2's debug output (in debug builds only).
    //
    // ARGUMENTS:
    //  pDebugConfiguration - Structure describing the debug output behavior.
    //  pReserved - Optional parameter; must be NULL.
    //
    STDMETHOD_(void, SetDebugConfiguration) (THIS_ _In_opt_ const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
        _Reserved_ void* pReserved X2DEFAULT(NULL)) PURE;
};
HRESULT XAudio27CreateProc(IXAudio2_7** ppXAudio2, UINT32 Flags, UINT32 XAudio2Processor)
{
    // Instantiate the appropriate XAudio2 engine
    IXAudio2_7* pXAudio2;

    IID iid;
    IIDFromString(L"{8bcf1f58-9fe7-4583-8ac6-e2adc465c8bb}", &iid);

    HRESULT hr = CoCreateInstance((Flags & XAUDIO2_DEBUG_ENGINE) ? CLSID_XAudio27_Debug : CLSID_XAudio27,
        NULL, CLSCTX_INPROC_SERVER, iid, (void**)&pXAudio2);
    if (SUCCEEDED(hr))
    {
        hr = pXAudio2->Initialize(Flags, XAudio2Processor);

        if (SUCCEEDED(hr))
        {
            *ppXAudio2 = pXAudio2;
        }
        else
        {
            pXAudio2->Release();
        }
    }
    return hr;
}
#endif

namespace alimer
{
    template <typename T>
    void SafeDestroyVoice(T *&voice)
    {
        if (voice)
        {
            voice->DestroyVoice();
            voice = nullptr;
        }
    }

    const PROPERTYKEY ALIMER_PKEY_Device_FriendlyName = { {0xA45C254E, 0xDF1C, 0x4EFD, {0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0}}, 14 };
    const PROPERTYKEY ALIMER_PKEY_AudioEngine_DeviceFormat = { {0xF19F064D, 0x82C,  0x4E27, {0xBC, 0x73, 0x68, 0x82, 0xA1, 0xBB, 0x8E, 0x4C}},  0 };

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
            ALIMER_UNUSED(pwstrDeviceId);
            ALIMER_UNUSED(dwNewState);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override
        {
            ALIMER_UNUSED(pwstrDeviceId);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override
        {
            ALIMER_UNUSED(pwstrDeviceId);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow InFlow, ERole InRole, LPCWSTR pwstrDeviceId) override
        {
            ALIMER_UNUSED(InFlow);
            ALIMER_UNUSED(InRole);
            ALIMER_UNUSED(pwstrDeviceId);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override
        {
            ALIMER_UNUSED(pwstrDeviceId);
            ALIMER_UNUSED(key);
            return S_OK;
        }

    private:
        ULONG _refs{ 1 };
    };

    AudioXAudio2::AudioXAudio2(bool validation)
        : Audio(AudioBackend::XAudio2)
    {
        memset(&_X3DAudio, 0, X3DAUDIO_HANDLE_BYTESIZE);

#if !ALIMER_PLATFORM_UWP
        if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == RPC_E_CHANGED_MODE)
        {
            CoInitializeEx(NULL, COINIT_MULTITHREADED);
        }

        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IMMDeviceEnumerator),
            (void**)&_deviceEnumerator);
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("[XAudio2] - CoCreateInstance(MMDeviceEnumerator) failed, {}", hr);
        }

        _notificationClient = new AudioNotificationClient();
        _deviceEnumerator->RegisterEndpointNotificationCallback(_notificationClient);

        IMMDeviceCollection* renderDevices;
        hr = _deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &renderDevices);
        if (FAILED(hr))
        {
            _deviceEnumerator->Release();
            ALIMER_LOGERROR("[XAudio2] - Failed to enumerate audio endpoints.");
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
            hr = devicePropStore->GetValue(ALIMER_PKEY_Device_FriendlyName, &deviceNameProp);
            std::wstring str(deviceNameProp.pwszVal);
            PropVariantClear(&deviceNameProp);

            PROPVARIANT deviceFormatProp;
            PropVariantInit(&deviceFormatProp);
            hr = devicePropStore->GetValue(ALIMER_PKEY_AudioEngine_DeviceFormat, &deviceFormatProp);
            WAVEFORMATEX* pWF = (WAVEFORMATEX*)deviceFormatProp.blob.pBlobData;
            ALIMER_UNUSED(pWF);
        }

        // First try to load 2.9
        _apiMinorVersion = 9;
        _xAudio2Module = LoadLibraryW(XAUDIO2_DLL_29);

        // Fallback to 2.8
        if (!_xAudio2Module)
        {
            ALIMER_LOGTRACE("XAudio 2.9 not found, fallback to 2.8.");
            _xAudio2Module = LoadLibraryW(XAUDIO2_DLL_28);
            _apiMinorVersion = 8;
        }

        if (!_xAudio2Module)
        {
            ALIMER_LOGTRACE("XAudio 2.8 not found, fallback to 2.7.");

            UINT32 flags = 0;
            if (validation)
            {
                _xAudio2Module = LoadLibraryExW(L"XAudioD2_7.DLL", nullptr, 0x00000800 /* LOAD_LIBRARY_SEARCH_SYSTEM32 */);
                if (!_xAudio2Module)
                {
                    ALIMER_LOGWARN("XAudio 2.7 debug version not installed on system (install the DirectX SDK Developer Runtime)");
                }
                else
                {
                    flags |= XAUDIO2_DEBUG_ENGINE;
                }
            }

            if (!_xAudio2Module)
            {
                _xAudio2Module = LoadLibraryW(L"XAudio2_7.DLL");
                if (!_xAudio2Module)
                {
                    ALIMER_LOGERROR("XAudio 2.7 not installed on system (install the DirectX End-user Runtimes (June 2010))");
                    return;
                }
            }

            _x3DAudioModule = LoadLibraryExW(L"X3DAudio1_7", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            _apiMinorVersion = 7;
            hr = XAudio27CreateProc(&_xaudio27, flags, XAUDIO2_DEFAULT_PROCESSOR);
        }
        else
        {
            XAudio2CreateProc XAudio2CreateFunc = (XAudio2CreateProc)GetProcAddress(_xAudio2Module, "XAudio2Create");
            if (!XAudio2CreateFunc)
            {
                ALIMER_LOGCRITICAL("Failed to get address of XAudio2Create");
            }

            hr = XAudio2CreateFunc(&_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
            if (FAILED(hr))
            {
                ALIMER_LOGCRITICAL("Failed to initialize XAudio2");
            }
        }
#else
        HRESULT hr = XAudio2Create(&_xAudio2, eflags);
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICAL("Failed to initialize XAudio2");
        }
#endif
        ALIMER_LOGINFO("XAudio {}.{} backend created.", _apiMajorVersion, _apiMinorVersion);

        if (validation
            && _apiMinorVersion > 7)
        {
            XAUDIO2_DEBUG_CONFIGURATION debugConfiguration;
            debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_DETAIL;
            debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
            debugConfiguration.LogThreadID = FALSE;
            debugConfiguration.LogFileline = FALSE;
            debugConfiguration.LogFunctionName = FALSE;
            debugConfiguration.LogTiming = FALSE;
            _xaudio2->SetDebugConfiguration(&debugConfiguration);
            ALIMER_LOGDEBUG("XAudio {}.{} debugging enabled.", _apiMajorVersion, _apiMinorVersion);
        }
    }

    AudioXAudio2::~AudioXAudio2()
    {
        SafeDestroyVoice(_reverbVoice);
        SafeDestroyVoice(_masteringVoice);
        if (_apiMinorVersion == 7)
        {
            _xaudio27->Release();
            _xaudio27 = nullptr;
        }
        else
        {
            _xaudio2->Release();
            _xaudio2 = nullptr;
        }
        memset(&_X3DAudio, 0, X3DAUDIO_HANDLE_BYTESIZE);

#if !ALIMER_PLATFORM_UWP
        if (_deviceEnumerator)
        {
            _deviceEnumerator->UnregisterEndpointNotificationCallback(_notificationClient);
            _deviceEnumerator->Release();
            _deviceEnumerator = nullptr;
        }

        if (_apiMinorVersion == 7)
        {
            FreeLibrary(_x3DAudioModule);
        }

        FreeLibrary(_xAudio2Module);
#endif
    }

    AudioResult AudioXAudio2::InitializeImpl()
    {
        HRESULT hr = S_OK;
        if (_apiMajorVersion == 2 && _apiMinorVersion == 7)
        {
            UINT32 count = 0;
            hr = _xaudio27->GetDeviceCount(&count);
            if (FAILED(hr)
                || !count)
            {
                _xaudio27->Release();
                return AudioResult::NoDeviceError;
            }

            const wchar_t* deviceId = nullptr;
            UINT32 devIndex = 0;
            if (deviceId)
            {
                // Translate device ID back into device index
                devIndex = UINT32(-1);
                for (UINT32 j = 0; j < count; ++j)
                {
                    XAUDIO2_DEVICE_DETAILS details;
                    hr = _xaudio27->GetDeviceDetails(j, &details);
                    if (SUCCEEDED(hr))
                    {
                        if (wcsncmp(deviceId, details.DeviceID, 256) == 0)
                        {
                            devIndex = j;
                            _masterChannelMask = details.OutputFormat.dwChannelMask;
                            break;
                        }
                    }
                }

                if (devIndex == UINT32(-1))
                {
                    _xaudio27->Release();
                    return AudioResult::NoDeviceError;
                }
            }
            else
            {
                // No search needed
                XAUDIO2_DEVICE_DETAILS details;
                hr = _xaudio27->GetDeviceDetails(0, &details);
                if (FAILED(hr))
                {
                    _xaudio27->Release();
                    return AudioResult::NoDeviceError;
                }

                _masterChannelMask = details.OutputFormat.dwChannelMask;
            }

            hr = _xaudio27->CreateMasteringVoice(
                &_masteringVoice,
                XAUDIO2_DEFAULT_CHANNELS,
                XAUDIO2_DEFAULT_SAMPLERATE,
                0,
                devIndex,
                nullptr);

            if (FAILED(hr))
            {
                _xaudio27->Release();
                ALIMER_LOGCRITICAL("Failed to create XAudio2 mastering voice");
            }

            if (FAILED(_xaudio27->StartEngine()))
            {

                _xaudio27->Release();
                ALIMER_LOGCRITICAL("Failed to starn XAudio2 engine.");
            }

            XAUDIO2_VOICE_DETAILS details;
            _masteringVoice->GetVoiceDetails(&details);

            _masterChannels = details.InputChannels;
            _masterRate = details.InputSampleRate;
        }
        else
        {
            hr = _xaudio2->CreateMasteringVoice(
                &_masteringVoice,
                XAUDIO2_DEFAULT_CHANNELS,
                XAUDIO2_DEFAULT_SAMPLERATE,
                0,
                nullptr,
                nullptr,
                AudioCategory_GameEffects);

            if (FAILED(hr))
            {
                _xaudio2->Release();
                return AudioResult::NoDeviceError;
            }

            if (FAILED(_xaudio2->StartEngine()))
            {

                _xaudio2->Release();
                ALIMER_LOGCRITICAL("Failed to starn XAudio2 engine.");
            }

            DWORD dwChannelMask;
            hr = _masteringVoice->GetChannelMask(&dwChannelMask);
            if (FAILED(hr))
            {
                _masteringVoice->DestroyVoice();
                _masteringVoice = nullptr;
                _xaudio2->Release();
                return AudioResult::Error;
            }

            XAUDIO2_VOICE_DETAILS details;
            _masteringVoice->GetVoiceDetails(&details);

            _masterChannelMask = dwChannelMask;
            _masterChannels = details.InputChannels;
            _masterRate = details.InputSampleRate;
        }

        ALIMER_LOGDEBUG("[XAudio2] - Mastering voice has {} channels, {} sample rate, {} channel mask", _masterChannels, _masterRate, _masterChannelMask);

        // Setup 3D audio
        const float SPEEDOFSOUND = X3DAUDIO_SPEED_OF_SOUND;

        if (_apiMinorVersion == 7)
        {
            typedef void(_cdecl * PFN_X3DAudioInitialize27)(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, __out X3DAUDIO_HANDLE Instance);

            PFN_X3DAudioInitialize27 X3DAudioInitializeFunc27 = (PFN_X3DAudioInitialize27)GetProcAddress(_x3DAudioModule, "X3DAudioInitialize");
            if (!X3DAudioInitializeFunc27)
                return AudioResult::Error;

            X3DAudioInitializeFunc27(_masterChannelMask, SPEEDOFSOUND, _X3DAudio);
        }
        else
        {
#if !ALIMER_PLATFORM_UWP
            typedef HRESULT(_cdecl * PFN_X3DAudioInitialize)(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, _Out_writes_bytes_(X3DAUDIO_HANDLE_BYTESIZE) X3DAUDIO_HANDLE Instance);

            PFN_X3DAudioInitialize X3DAudioInitializeFunc = (PFN_X3DAudioInitialize)GetProcAddress(_xAudio2Module, "X3DAudioInitialize");
            if (!X3DAudioInitializeFunc)
                return AudioResult::Error;

            hr = X3DAudioInitializeFunc(_masterChannelMask, SPEEDOFSOUND, _X3DAudio);
#else
            hr = X3DAudioInitialize(_masterChannelMask, SPEEDOFSOUND, _X3DAudio);
#endif
            if (FAILED(hr))
            {
                SafeDestroyVoice(_reverbVoice);
                SafeDestroyVoice(_masteringVoice);
                //_reverbEffect.Reset();
                //_volumeLimiter.Reset();
                _xaudio2->Release();
            }
        }

        return AudioResult::OK;
    }

    void AudioXAudio2::SetMasterVolumeImpl(float volume)
    {
        assert(volume >= -XAUDIO2_MAX_VOLUME_LEVEL && volume <= XAUDIO2_MAX_VOLUME_LEVEL);

        if (_masteringVoice)
        {
            HRESULT hr = _masteringVoice->SetVolume(volume);
            if (FAILED(hr))
            {
                ALIMER_LOGDEBUG("[XAudio2] - Failed to set master voice volume, COM error: {}", hr);
            }
        }
    }

    void AudioXAudio2::SetPaused(bool paused)
    {
        if (paused)
        {
            if (_apiMinorVersion == 7)
                _xaudio27->StopEngine();
            else
                _xaudio2->StopEngine();
        }
        else
        {
            HRESULT hr;
            if (_apiMinorVersion == 7)
                hr = _xaudio27->StartEngine();
            else
                hr = _xaudio2->StartEngine();

            if (FAILED(hr))
            {
                ALIMER_LOGERROR(
                    "XAudio2 resume: Failure with HRESULT of {}",
                    static_cast<unsigned int>(hr)
                );
            }
        }
    }
}
