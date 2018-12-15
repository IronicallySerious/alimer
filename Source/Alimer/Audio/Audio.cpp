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

#include "../Audio/Audio.h"
#include "../Core/Log.h"
#if defined(_MSC_VER)
#   include "../Audio/XAudio2/Audio.XAudio2.h"
#endif

namespace Alimer
{
    Audio *Audio::_instance;

	Audio::Audio(AudioBackend backend)
        : _backend(backend)
	{
        _instance = this;
	}

    Audio::~Audio()
    {
        _instance = nullptr;
    }

    Audio& Audio::GetInstance()
    {
        return *_instance;
    }

    Audio* Audio::Create(AudioBackend prefferedBackend, bool validation)
    {
        if (prefferedBackend == AudioBackend::Default)
        {
            prefferedBackend = GetPlatformDefaultBackend();
        }

        Audio* audio = nullptr;
        switch (prefferedBackend)
        {
        case AudioBackend::XAudio2:
#if defined(_MSC_VER)
            audio = new AudioXAudio2(validation);
#else
            ALIMER_LOGWARN("XAudio2 is not supported");
#endif
            break;

        default:
            break;
        }

        return audio;
    }

    void Audio::Shutdown()
    {
        SafeDelete(_instance);
    }

    AudioBackend Audio::GetPlatformDefaultBackend()
    {
#if defined(_MSC_VER)
        return AudioBackend::XAudio2;
#endif
    }
    
    AudioResult Audio::Initialize()
    {
        if (_initialized)
            return AudioResult::OK;

        auto result = InitializeImpl();
        _initialized = result == AudioResult::OK;
        if (result != AudioResult::OK)
        {
            switch (result)
            {
            case AudioResult::NoDeviceError:
                ALIMER_LOGERROR("AudioEngine found no default audio device.");
                break;
            default:
                ALIMER_LOGERROR("AudioEngine failed to initialize.");
                break;
            }
        }

        return result;
    }

    void Audio::Pause()
    {
        if (_paused)
            return;

        SetPaused(true);
        _paused = true;
    }

    void Audio::Resume()
    {
        if (!_paused)
            return;

        SetPaused(false);
        _paused = false;
    }

    void Audio::SetMasterVolume(float volume)
    {
        _masterVolume = volume;
        SetMasterVolumeImpl(volume);
    }


    Audio& gAudio()
    {
        return Audio::GetInstance();
    }
}
