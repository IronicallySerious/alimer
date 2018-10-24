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

#include "../AlimerConfig.h"
#include <string>

namespace Alimer
{
    enum class GpuVendor : uint8_t
    {
        Unknown,
        Arm,
        Nvidia,
        Amd,
        Intel,
        Warp,
        Count
    };

	/// Describes features supported by given instance of GpuDevice.
	class ALIMER_API GraphicsDeviceFeatures final
	{
    public:
		/// Constructor.
        GraphicsDeviceFeatures();

		/// Destructor.
		~GraphicsDeviceFeatures() = default;

        /// Reset features to default.
        void Reset();

        bool GetMultithreading() const { return _multithreading; }
        void SetMultithreading(bool value) { _multithreading = value; }

        uint32_t GetMaxColorAttachments() const { return _maxColorAttachments; }
        void SetMaxColorAttachments(uint32_t value) { _maxColorAttachments = value; }

        void SetVendorId(uint32_t vendorID);
        void SetDeviceId(uint32_t deviceID);
        void SetDeviceName(const std::string& deviceName);

    protected:
        uint32_t _vendorID;
        GpuVendor _vendor;
        uint32_t _deviceID;
        std::string _deviceName;
        bool _multithreading;
        uint32_t _maxColorAttachments;
	};
}
