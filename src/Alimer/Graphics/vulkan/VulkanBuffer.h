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

#include "../Buffer.h"
#include "../Backend.h"
#include <vector>

namespace alimer
{
    class VulkanGraphicsDevice;

	/// Vulkan Buffer.
	class VulkanBuffer final : public Buffer
	{
	public:
        VulkanBuffer(VulkanGraphicsDevice* device, const BufferDescriptor* descriptor, const void* initialData);
        ~VulkanBuffer() override;
        void Destroy() override;

        //bool SetSubDataImpl(uint32_t offset, uint32_t size, const void* pData) override;

		VkBuffer GetHandle() const { return _handle; }
        VmaAllocation GetAllocation() const { return _allocation; }
	private:
        VkBuffer _handle = VK_NULL_HANDLE;
        VkDevice _logicalDevice;
        VmaAllocator _allocator;
        VmaAllocation _allocation = VK_NULL_HANDLE;
	};
}