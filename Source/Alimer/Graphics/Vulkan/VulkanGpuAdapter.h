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

#include "../GpuAdapter.h"
#include "VulkanPrerequisites.h"

namespace Alimer
{
	class VulkanGpuAdapter final : public GpuAdapter
	{
    public:
		/// Constructor.
        VulkanGpuAdapter(VkPhysicalDevice handle);

		/// Destructor.
		~VulkanGpuAdapter() override;

        VkPhysicalDevice GetVkHandle() const { return _vkHandle; }
        inline const VkPhysicalDeviceProperties& GetProperties() const { return _properties; }
        inline const VkPhysicalDeviceLimits& GetLimits() const { return _properties.limits; }
        inline const VkPhysicalDeviceFeatures& GetFeatures() const { return _features; }

    protected:
        VkPhysicalDevice _vkHandle;
        VkPhysicalDeviceProperties _properties;
        VkPhysicalDeviceFeatures _features;
        VkPhysicalDeviceMemoryProperties _memoryProperties;
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
	};
}
