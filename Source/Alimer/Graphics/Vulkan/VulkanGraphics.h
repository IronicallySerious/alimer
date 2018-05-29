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

#include "../Graphics.h"
#include "../../Util/HashMap.h"
#include "VulkanPrerequisites.h"

namespace Alimer
{
	/// Vulkan graphics backend.
	class VulkanGraphics final : public Graphics
	{
	public:
		/// Is backend supported?
		static bool IsSupported();

		/// Construct. Set parent shader and defines but do not compile yet.
		VulkanGraphics(bool validation, const std::string& applicationName);
		/// Destruct.
		~VulkanGraphics() override;

		bool WaitIdle() override;
		bool Initialize(std::shared_ptr<Window> window) override;
		std::shared_ptr<Texture> AcquireNextImage() override;
		bool Present() override;

		CommandBufferPtr CreateCommandBuffer() override;

		GpuBufferPtr CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData) override;
		PipelineLayoutPtr CreatePipelineLayout() override;
		std::shared_ptr<Shader> CreateShader(const std::string& name) override;
		std::shared_ptr<Shader> CreateShader(const ShaderBytecode& vertex, const ShaderBytecode& fragment) override;
		PipelineStatePtr CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;
		
		VkDevice GetVkDevice() const { return _device; }

	private:
		void Finalize() override;

		bool PrepareDraw(PrimitiveTopology topology);

		VkInstance _vkInstance = VK_NULL_HANDLE;
		VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT _vkDebugCallback = VK_NULL_HANDLE;
		VkDevice _device = VK_NULL_HANDLE;
	};
}
