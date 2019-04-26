#pragma once

#include <KR3/main.h>
#include <KRUtil/wl/com.h>

#include <dxgiformat.h>

#include "../gi.h"

struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;

namespace kr
{
	namespace d3d11
	{
		enum class BlendMode
		{
			None,
			Normal,
			Add,
			Subtract,
			Multiply,
			Count,
		};

		using DeviceContext = Com<ID3D11DeviceContext>;
		using RenderTargetView = Com<ID3D11RenderTargetView>;
		using DepthStencilView = Com<ID3D11DepthStencilView>;

		using Texture2D = Com<ID3D11Texture2D>;
		using ShaderResourceView = Com<ID3D11ShaderResourceView>;
		using Buffer = Com<ID3D11Buffer>;
		using InputLayout = Com<ID3D11InputLayout>;
		using VertexShader = Com<ID3D11VertexShader>;
		using PixelShader = Com<ID3D11PixelShader>;
		using GeometryShader = Com<ID3D11GeometryShader>;

		using SamplerState = ID3D11SamplerState*;
		using BlendState = ID3D11BlendState*;
		using RasterizerState = ID3D11RasterizerState*;
		using DepthStencilState = ID3D11DepthStencilState*;

		enum class TextureAddressMode
		{
			WRAP = 1,
			MIRROR = 2,
			CLAMP = 3,
			BORDER = 4,
			MIRROR_ONCE = 5
		};
		enum class Filter
		{
			MIN_MAG_MIP_POINT = 0,
			MIN_MAG_POINT_MIP_LINEAR = 0x1,
			MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
			MIN_POINT_MAG_MIP_LINEAR = 0x5,
			MIN_LINEAR_MAG_MIP_POINT = 0x10,
			MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
			MIN_MAG_LINEAR_MIP_POINT = 0x14,
			MIN_MAG_MIP_LINEAR = 0x15,
			ANISOTROPIC = 0x55,
		};
		enum class CullMode
		{
			NONE = 1,
			FRONT = 2,
			BACK = 3
		};
		enum class FillMode
		{
			WIREFRAME = 2,
			SOLID = 3
		};
		enum class ComparationFunc
		{
			NEVER = 1,
			LESS = 2,
			EQUAL = 3,
			LESS_EQUAL = 4,
			GREATER = 5,
			NOT_EQUAL = 6,
			GREATER_EQUAL = 7,
			ALWAYS = 8
		};
		enum class DepthWriteMask
		{
			ZERO = 0,
			ALL = 1
		};

		template <typename T> struct StateConst;
		template <> struct StateConst<TextureAddressMode> { enum { count = 5, offset = 1 }; };
		template <> struct StateConst<Filter> { enum { count = 9, offset = -1 }; };
		template <> struct StateConst<CullMode> { enum { count = 3, offset = 1 }; };
		template <> struct StateConst<FillMode> { enum { count = 2, offset = 2 }; };
		template <> struct StateConst<ComparationFunc> { enum { count = 8, offset = 1 }; };
		template <> struct StateConst<DepthWriteMask> { enum { count = 2, offset = 0 }; };
		template <> struct StateConst<bool> { enum { count = 2, offset = 0 }; };

		enum class Usage
		{
			Default = 0,
			Immutable = 1,
			Dynamic = 2,
			Staging = 3
		};
		enum class CpuAccess
		{
			Write = 0x10000L,
			Read = 0x20000L
		};
	}

	COM_CLASS_DECLARE(ID3D11DeviceContext);
	COM_CLASS_DECLARE(ID3D11RenderTargetView);
	COM_CLASS_DECLARE(ID3D11DepthStencilView);

}