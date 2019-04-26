#pragma once

#include "common.h"
#include "buffer.h"

struct D3D11_TEXTURE2D_DESC;
struct D3D11_SUBRESOURCE_DATA;

namespace kr
{
	COM_CLASS_METHODS(ID3D11Texture2D)
	{
	public:
		ComMethod() = default;

		// 텍스처 생성
		ComMethod(D3D11_TEXTURE2D_DESC * desc, D3D11_SUBRESOURCE_DATA * data = nullptr) throws(ErrorCode);

		// 텍스처 생성
		ComMethod(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, uint misc = 0, D3D11_SUBRESOURCE_DATA * data = nullptr) throws(ErrorCode);

		~ComMethod() noexcept;

		// 텍스처에서 셰이더 리소스 뷰를 생성한다
		d3d11::ShaderResourceView createShaderResourceView();

		void create(D3D11_TEXTURE2D_DESC * desc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode);
		void create(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, uint misc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode);

	};

	namespace d3d11
	{
		class TextureCube : public Texture2D
		{
		public:
			TextureCube() = default;
			TextureCube(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data);
			TextureCube(pcstr16 names[6]);
			TextureCube(pcstr16 back, pcstr16 front, pcstr16 left, pcstr16 right, pcstr16 top, pcstr16 bottom);

			// 텍스처에서 셰이더 리소스 뷰를 생성한다
			d3d11::ShaderResourceView createShaderResourceView();

			void create(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data);
			void create(pcstr16 names[6]);
		};

		class TextureCubeMap : public Com<ID3D11ShaderResourceView>
		{
		public:
			TextureCubeMap();
			TextureCubeMap(ID3D11DeviceContext * ctx, uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data);
			
			// right, left, top, bottom, front, back
			TextureCubeMap(ID3D11DeviceContext * ctx, pcstr16 name[]);
			TextureCubeMap(ID3D11DeviceContext * ctx, pcstr16 right, pcstr16 left, pcstr16 top, pcstr16 bottom, pcstr16 front, pcstr16 back);

			void create(ID3D11DeviceContext * ctx, uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data);
			void create(ID3D11DeviceContext * ctx, pcstr16 name[6]);

		};
	}
}