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

#include "../../Audio/Audio.h"
#include "../../Core/Platform.h"

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable:4091)   // 'typedef ': ignored on left of '' when no variable is declared
#endif
#	include <mmdeviceapi.h>
#   include <xaudio2.h>
#   include <xaudio2fx.h>
#   include <x3daudio.h>
#   include <xapofx.h>
#   include <atomic>
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

#if !ALIMER_PLATFORM_UWP
struct IXAudio2_7;
#endif

namespace Alimer
{
	/// XAudio2.7 Audio implementation.
	class AudioXAudio2 final : public Audio
	{
	public:
		/// Constructor.
        AudioXAudio2(bool validation);

		/// Destructor.
		~AudioXAudio2() override;

        void SetMasterVolumeImpl(float volume) override;
        void SetPaused(bool paused) override;

	private:
#if !ALIMER_PLATFORM_UWP
        HMODULE _xAudio2Module = nullptr;
        HMODULE _x3DAudioModule = nullptr;
        IXAudio2_7* _xaudio27 = nullptr;
        IMMDeviceEnumerator*    _deviceEnumerator = nullptr;
        IMMNotificationClient*  _notificationClient = nullptr;
#endif
        IXAudio2*               _xaudio2 = nullptr;
        IXAudio2MasteringVoice* _masteringVoice = nullptr;
        IXAudio2SubmixVoice*    _reverbVoice = nullptr;

        uint32_t                _apiMajorVersion = 2;
        uint32_t                _apiMinorVersion;
        uint32_t                _masterChannelMask = 0;
        uint32_t                _masterChannels = 0;
        uint32_t                _masterRate = 0;

        X3DAUDIO_HANDLE        _X3DAudio;
	};
}
