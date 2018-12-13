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

#include "../Audio/Types.h"
#include <string>
#include <atomic>

namespace Alimer
{
	/// Defines Audio module class.
	class ALIMER_API Audio 
	{
	protected:
		/// Constructor.
		Audio(AudioBackend backend);

	public:
        /// Destructor.
        virtual ~Audio();

        /// Create the audio module.
        static Audio* Create(AudioBackend prefferedBackend = AudioBackend::Default, bool validation = false);

        /// Shutdown the audio module.
        static void Shutdown();

        /// Get the best platform backend.
        static AudioBackend GetPlatformDefaultBackend();

        /// Return the single instance of the Audio.
        static Audio& GetInstance();

        /// Get the backend.
        AudioBackend GetBackend() const { return _backend; }

        /// Initialize the audio engine.
        AudioResult Initialize();

        /// Pause the audio.
        void Pause();

        /// Resume the audio.
        void Resume();

        float GetMasterVolume() const { return _masterVolume; }
        void SetMasterVolume(float volume);

	private:
        virtual AudioResult InitializeImpl() = 0;
        virtual void SetMasterVolumeImpl(float volume) = 0;
        virtual void SetPaused(bool paused) = 0;

        static Audio *_instance;
        AudioBackend _backend = AudioBackend::Empty;
        bool _initialized = false;
        float _masterVolume = 1.0f;
        bool _paused = false;

		DISALLOW_COPY_MOVE_AND_ASSIGN(Audio);
	};

    /// Singleton access for Audio. 
    ALIMER_API Audio& gAudio();
}
