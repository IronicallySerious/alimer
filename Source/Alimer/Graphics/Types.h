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

#include "../Util/Util.h"
#include "../Math/Color.h"
#include <array>

namespace Alimer
{
	static constexpr uint32_t MaxDescriptorSets = 4u;
	static constexpr uint32_t MaxBindingsPerSet = 16u;
	static constexpr uint32_t MaxVertexAttributes = 16u;
	static constexpr uint32_t MaxVertexBufferBindings = 4u;
	static constexpr uint32_t MaxColorAttachments = 8u;

	/// Enum describing the Graphics backend type.
	enum class GraphicsDeviceType
	{
		/// Best device supported for running platform.
		Default,
		/// Empty/Headless device type.
		Empty,
		/// Vulkan backend.
		Vulkan,
		/// DirectX12 backend.
		Direct3D12
	};

    enum class GpuResourceType : uint32_t
    {
        Unknown,
        Buffer,
        Texture
    };

	/// Enum describing the number of samples.
	enum class SampleCount : uint32_t
	{
		/// 1 sample (no multi-sampling).
		Count1 = 1,
		/// 2 Samples.
		Count2 = 2,
		/// 4 Samples.
		Count4 = 4,
		/// 8 Samples.
		Count8 = 8,
		/// 16 Samples.
		Count16 = 16,
		/// 32 Samples.
		Count32 = 32,
	};

	/// Texture types.
	enum class TextureType : uint32_t
	{
		Type1D = 0,
		Type2D,
		Type3D,
		TypeCube,
	};

	enum class TextureUsage : uint32_t
	{
		Unknown = 0,
		ShaderRead = 1 << 0,
		ShaderWrite = 1 << 1,
		RenderTarget = 1 << 2,
	};
	ALIMER_BITMASK(TextureUsage);

	enum class BufferUsage
	{
		Unknown = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Uniform = 1 << 2,
	};
	ALIMER_BITMASK(BufferUsage);

	/// Shader stage.
	enum class ShaderStage
	{
		Vertex = 0,
		TessControl = 1,
		TessEvaluation = 2,
		Geometry = 3,
		Fragment = 4,
		Compute = 5,
		Count
	};

	/// Primitive topology.
	enum class PrimitiveTopology : uint32_t
	{
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip,
		Count
	};

	/// VertexInputRate
	enum class VertexInputRate
	{
		Vertex,
		Instance
	};

	enum class LoadAction
	{
		DontCare,
		Load,
		Clear
	};

	enum class StoreAction
	{
		DontCare,
		Store
	};

	enum class VertexFormat
	{
		Invalid,
		Float,
		Float2,
		Float3,
		Float4,
		Byte4,
		Byte4N,
		UByte4,
		UByte4N,
		Short2,
		Short2N,
		Short4,
		Short4N,
		Count
	};

	struct VertexElement
	{
		uint32_t binding;
		VertexFormat format;
		uint32_t offset;
	};

	class Texture;
	class RenderPassAttachmentDescriptor
	{
	public:
		Texture * texture = nullptr;
		uint32_t level = 0;
		uint32_t slice = 0;
	};

	class RenderPassColorAttachmentDescriptor : public RenderPassAttachmentDescriptor
	{
	public:
		LoadAction loadAction = LoadAction::Clear;
		StoreAction storeAction = StoreAction::Store;
		Color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class RenderPassDepthAttachmentDescriptor : public RenderPassAttachmentDescriptor
	{
	public:
		LoadAction loadAction = LoadAction::Clear;
		StoreAction storeAction = StoreAction::DontCare;
		float clearDepth = 1.0f;
	};

	class RenderPassStencilAttachmentDescriptor : public RenderPassAttachmentDescriptor
	{
	public:
		LoadAction loadAction = LoadAction::DontCare;
		StoreAction storeAction = StoreAction::DontCare;
		uint8_t clearStencil = 0;
	};

	class RenderPassDescriptor
	{
	public:
		std::array<RenderPassColorAttachmentDescriptor, MaxColorAttachments> colorAttachments;
		RenderPassDepthAttachmentDescriptor depthAttachment;
		RenderPassStencilAttachmentDescriptor stencilAttachment;

		RenderPassDescriptor();
	};

    template <>
    struct EnumNames<ShaderStage>
    {
        constexpr std::array<const char*, 6> operator()() const {
            return { {
                    "vertex",
                    "tesscontrol",
                    "tesseval",
                    "geometry",
                    "fragment",
                    "compute",
                } };
        }
    };
}
