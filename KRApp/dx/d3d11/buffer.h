#pragma once

#include "common.h"
#include <new>

namespace kr
{
	COM_CLASS_METHODS(ID3D11Buffer)
	{
		ComMethod() = default;
		ComMethod(uint bindflags, size_t size);
		ComMethod(uint bindflags, kr::Buffer initdata);

		void create(d3d11::Usage usage, d3d11::CpuAccess cpuflags, uint bindflags, size_t size);
		void create(d3d11::Usage usage, d3d11::CpuAccess cpuflags, uint bindflags, kr::Buffer initdata);
	};

	namespace d3d11
	{
		enum Bind
		{
			BindVertexBuffer = 0x1L,
			BindIndexBuffer = 0x2L,
			BindConstantBuffer = 0x4L,
			BindShaderResource = 0x8L,
			BindStreamOutput = 0x10L,
			BindRenderTarget = 0x20L,
			BindDepthStencil = 0x40L,
			BindUnorderedAccess = 0x80L,
			BindDecoder = 0x200L,
			BindVideoEncoder = 0x400L
		};

		class DynamicBuffer : public Buffer
		{
		public:
			DynamicBuffer() = default;
			DynamicBuffer(uint bindflags, size_t size);
			DynamicBuffer(uint bindflags, kr::Buffer initdata);

			using Buffer::create;
			void create(uint bindflags, size_t size);
			void create(uint bindflags, kr::Buffer initdata);

			void* map(ID3D11DeviceContext * dc);
			void unmap(ID3D11DeviceContext * dc) noexcept;

			using Buffer::operator &;
			using Buffer::operator ID3D11Buffer *;
			using Buffer::operator ->;
		};

		class VertexBuffer :public Buffer
		{
		public:
			VertexBuffer() = default;
			VertexBuffer(kr::Buffer initdata);
		};

		class IndexBuffer :public Buffer
		{
		public:
			IndexBuffer() = default;
			IndexBuffer(kr::Buffer initdata);
		};

		class ConstBuffer :public Buffer
		{
		public:
			ConstBuffer() = default;
			ConstBuffer(size_t sz);
		};
	}
}