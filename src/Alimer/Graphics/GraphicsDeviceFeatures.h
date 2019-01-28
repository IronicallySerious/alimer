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
#include "../Graphics/Types.h"

namespace alimer
{
    enum class GpuVendor : uint32_t
    {
        Unknown = 0,
        NVIDIA = 1,
        AMD = 2,
        INTEL = 3,
        ARM = 4,
        WARP = 5,
        Count = 6
    };

	/// Describes features supported by given instance of GpuDevice.
	class ALIMER_API GraphicsDeviceFeatures final
	{
    public:
		/// Constructor.
        GraphicsDeviceFeatures();

		/// Destructor.
		~GraphicsDeviceFeatures() = default;

        GraphicsBackend GetBackend() const { 
            return _backend;
        }

        void SetBackend(GraphicsBackend backend) {
            _backend = backend; 
        }

        uint32_t GetVendorId() const { 
            return _vendorId; 
        }
        void SetVendorId(uint32_t vendorID) {
            _vendorId = vendorID;
        }
        GpuVendor GetVendor() const { 
            return _vendor;
        }
        void SetVendor(GpuVendor vendor) {
            _vendor = vendor;
        }
        uint32_t GetDeviceId() const { 
            return _deviceId; 
        }
        void SetDeviceId(uint32_t deviceId) {
            _deviceId = deviceId;
        }

        ///	Gets the current physical device name.
        String GetDeviceName() const { 
            return _deviceName; 
        }
        ///	Sets the current GPU device name.
        void SetDeviceName(const String& name) {
            _deviceName = name;
        }

        bool GetMultithreading() const { 
            return _multithreading; 
        }
        void SetMultithreading(bool value) {
            _multithreading = value; 
        }

        uint32_t GetMaxColorAttachments() const {
            return _maxColorAttachments;
        }
        void SetMaxColorAttachments(uint32_t value) {
            _maxColorAttachments = value;
        }

        uint32_t GetMaxBindGroups() const {
            return _maxBindGroups;
        }
        void SetMaxBindGroups(uint32_t value) {
            _maxBindGroups = value;
        }

        uint32_t GetMinUniformBufferOffsetAlignment() const {
            return _minUniformBufferOffsetAlignment;
        }
        void SetMinUniformBufferOffsetAlignment(uint32_t value) {
            _minUniformBufferOffsetAlignment = value;
        }

    private:
        GraphicsBackend _backend = GraphicsBackend::Null;
        uint32_t        _vendorId = 0;
        GpuVendor       _vendor = GpuVendor::Unknown;
        uint32_t        _deviceId = 0;
        String          _deviceName;
        bool            _multithreading = false;

        uint32_t        _maxColorAttachments = 0;
        uint32_t        _maxBindGroups = 0;
        uint32_t        _minUniformBufferOffsetAlignment = 0;
	};
}
