#pragma once

#include "common.h"
#include <stddef.h>

namespace kr
{
	namespace d3d11
	{
		struct VertexFormatInfo
		{
			enum Type
			{
				Typeless,
				UnsignedNormal,
				SignedNormal,
				Float,
				UnsignedInt,
				SignedInt,
				Mixed,
			};
			enum Special
			{
				Normal,
				S_1,
				S_24_8,
				S_32_8_24,
				S_10_10_10_2,
				S_11_11_10,
			};

			unsigned int elementSize;
			unsigned int elementCount;
			Type elementType;
			Special special;
			unsigned int byteSize;

			VertexFormatInfo() = default;
			VertexFormatInfo(DXGI_FORMAT format) noexcept;
		};
		struct VertexLayout
		{
			enum Semantic
			{
				BiNormal,
				BlendIndices,
				BlendWeight,
				Color,
				Normal,
				Position,
				PositionT,
				PSize,
				Tangent,
				TexCoord,
			};

			Semantic semantic;
			DXGI_FORMAT format;
			unsigned int slot;

			VertexLayout() = default;
			VertexLayout(Semantic semantic, DXGI_FORMAT format, unsigned int slot = 0) noexcept;
			unsigned int getSize() const noexcept;
			static unsigned int getFullSize(View<VertexLayout> vl) noexcept;
		};
	}
}
