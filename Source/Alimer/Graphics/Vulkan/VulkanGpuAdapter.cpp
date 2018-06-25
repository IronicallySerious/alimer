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

#include "VulkanGpuAdapter.h"

namespace Alimer
{
    VulkanGpuAdapter::VulkanGpuAdapter(VkPhysicalDevice handle)
        : _vkHandle(handle)
    {
        vkGetPhysicalDeviceProperties(handle, &_properties);
        vkGetPhysicalDeviceFeatures(handle, &_features);
        vkGetPhysicalDeviceMemoryProperties(handle, &_memoryProperties);

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, nullptr);
        _queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, _queueFamilyProperties.data());

        // Find vendor
        switch (_properties.vendorID)
        {
        case 0x13B5:
            _vendor = GpuVendor::Arm;
            break;
        case 0x10DE:
            _vendor = GpuVendor::Nvidia;
            break;
        case 0x1002:
        case 0x1022:
            _vendor = GpuVendor::Amd;
            break;
        case 0x8086:
            _vendor = GpuVendor::Intel;
            break;
        default:
            _vendor = GpuVendor::Unknown;
            break;
        }

        _vendorID = _properties.vendorID;
        _deviceID = _properties.deviceID;
        _deviceName = _properties.deviceName;
        //uint32_t apiMajor = VK_VERSION_MAJOR(_properties.apiVersion);
        //uint32_t apiMinor = VK_VERSION_MINOR(_properties.apiVersion);
        //uint32_t apiPatch = VK_VERSION_PATCH(_properties.apiVersion);

        //uint32_t driverMajor = VK_VERSION_MAJOR(_properties.driverVersion);
        //uint32_t driverMinor = VK_VERSION_MINOR(_properties.driverVersion);
        //uint32_t driverPatch = VK_VERSION_PATCH(_properties.driverVersion);
    }

    VulkanGpuAdapter::~VulkanGpuAdapter()
    {

    }
}
