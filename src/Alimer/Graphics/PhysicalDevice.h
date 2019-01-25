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

#include "../Base/String.h"
#include "../Graphics/Backend.h"

namespace alimer
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

	/// Defines a PhysicalDevice.
	class ALIMER_API PhysicalDevice final
	{
        friend class GraphicsDevice;
	public:
        PhysicalDevice() = default;

		/// Destructor.
		~PhysicalDevice() = default;

        /// Get the backend handle.
        PhysicalDeviceHandle GetHandle() const { return _handle; }

        uint32_t GetVendorID() const { return _vendorID; }
        GpuVendor GetVendor() const { return _vendor; }
        uint32_t GetDeviceID() const { return _deviceID; }
        String GetDeviceName() const { return _deviceName; }

#if defined(ALIMER_VULKAN)
        bool IsExtensionSupported(const String& extension)
        {
            return _extensions.Contains(extension);
        }
#endif

    private:
        PhysicalDeviceHandle    _handle = BACKEND_INVALID_HANDLE;
        uint32_t                _vendorID = 0;
        GpuVendor               _vendor = GpuVendor::Unknown;
        uint32_t                _deviceID = 0;
        String                  _deviceName;

#if defined(ALIMER_VULKAN)
        VkPhysicalDeviceProperties          _properties;
        VkPhysicalDeviceMemoryProperties    _memoryProperties;
        VkPhysicalDeviceFeatures            _features;
        Vector<VkQueueFamilyProperties>     _queueFamilyProperties;
        Vector<String>                      _extensions;
#endif
	};
}
