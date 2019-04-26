#pragma once

#include "common.h"
#include "vertexlayout.h"
#include "device.h"

struct D3D11_INPUT_ELEMENT_DESC;

namespace kr
{
	COM_CLASS_METHODS(ID3D11InputLayout)
	{
		ComMethod() = default;
		ComMethod(Buffer data, View<d3d11::VertexLayout> vl);

		void _create(Buffer data, View<D3D11_INPUT_ELEMENT_DESC> desc);
		void create(Buffer data, View<d3d11::VertexLayout> vl);
	};

	COM_CLASS_METHODS(ID3D11VertexShader)
	{
		ComMethod() = default;
		ComMethod(Buffer data);

		void create(Buffer data);
	};

	COM_CLASS_METHODS(ID3D11PixelShader)
	{
		ComMethod() = default;

		ComMethod(Buffer data);
		void create(Buffer data);
	};
}