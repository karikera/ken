#ifdef WIN32
#include "texture.h"
#include "internal.h"

#include <K2D/image.h>
#include <K2D/wic/WICTextureLoader.h>

using namespace kr;
using namespace d3d11;

namespace
{
	class CubeImageData
	{
	public:
		image::ImageData m_images[6];
		D3D11_SUBRESOURCE_DATA m_resdata[6];
		image::PixelFormat m_format;
		int m_width;
		int m_height;
		int m_rowPitch;
		size_t m_size;
		Array<char> buffer;

		bool load(pcstr16 names[]) noexcept
		{
			m_size = 0;

			enum Error { None, UnmatchFormat, UnmatchSize };
			Error error = None;

			bool notfound[6] = {};
			bool emptyAny = false;
			for (size_t i = 0; i < 6; i++)
			{
				pcstr16 name = names[i];
				image::ImageData * img = &m_images[i];
				D3D11_SUBRESOURCE_DATA * rd = &m_resdata[i];
				rd->pSysMem = nullptr;

				if (name == nullptr)
				{
					emptyAny = true;
					continue;
				}

				if (FAILED(DirectX::LoadImageFromFile(s_d3d11, wide(name), img)))
				{
					notfound[i] = true;
					emptyAny = true;
					continue;
				}

				if (m_size == 0)
				{
					rd->pSysMem = img->getBits();
					m_format = img->getPixelFormat();
					m_rowPitch = img->getPitch();
					m_width = img->getWidth();
					m_height = img->getHeight();
					m_size = img->getByteSize();
				}
				else
				{
					if (m_format != img->getPixelFormat())
					{
						error = UnmatchFormat;
						notfound[i] = true;
						emptyAny = true;
					}
					else if (m_width != img->getWidth() || m_height != img->getHeight())
					{
						error = UnmatchSize;
						notfound[i] = true;
						emptyAny = true;
					}
					else rd->pSysMem = img->getBits();
				}
			}

			if (m_size == 0) return false;

			if (emptyAny)
			{
				buffer.resize(m_size);
				buffer.fill((char)0xff);

				for (size_t i = 0; i < 6; i++)
				{
					D3D11_SUBRESOURCE_DATA *rd = &m_resdata[i];
					if (rd->pSysMem != nullptr) continue;
					rd->pSysMem = buffer.data();
				}
			}
			for (size_t i = 0; i < 6; i++)
			{
				D3D11_SUBRESOURCE_DATA *rd = &m_resdata[i];
				rd->SysMemPitch = m_rowPitch;
				rd->SysMemSlicePitch = (UINT)m_size;
			}
			return true;
		}
	};
}

void ComMethod<ID3D11Texture2D>::create(D3D11_TEXTURE2D_DESC * desc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode)
{
	hrexcept(s_d3d11->CreateTexture2D(desc, data, &ptr()));
}
void ComMethod<ID3D11Texture2D>::create(uint bind, Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, UINT misc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = multiSampling;
	desc.SampleDesc.Quality = 0;
	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = bind;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = misc;
	create(&desc, data);
}

ComMethod<ID3D11Texture2D>::ComMethod(D3D11_TEXTURE2D_DESC * desc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode)
{
	create(desc, data);
}
ComMethod<ID3D11Texture2D>::ComMethod(uint bind, Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, UINT misc, D3D11_SUBRESOURCE_DATA * data) throws(ErrorCode)
{
	create(bind, usage, format, width, height, multiSampling, misc, data);
}
ComMethod<ID3D11Texture2D>::~ComMethod() noexcept
{
}
d3d11::ShaderResourceView ComMethod<ID3D11Texture2D>::createShaderResourceView() throws(ErrorCode)
{
	D3D11_TEXTURE2D_DESC desc;
	ptr()->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

	d3d11::ShaderResourceView srv;
	hrexcept(s_d3d11->CreateShaderResourceView(ptr(), &srvDesc, &srv));
	return srv;
}

TextureCube::TextureCube(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data)
{
	create(bind, usage, format, width, height, multiSampling, data);
}
TextureCube::TextureCube(pcstr16 names[6])
{
	create(names);
}
TextureCube::TextureCube(pcstr16 back, pcstr16 front, pcstr16 left, pcstr16 right, pcstr16 top, pcstr16 bottom)
{
	pcstr16 names[] = {back, front, left, right, top, bottom};
	create(names);
}
d3d11::ShaderResourceView TextureCube::createShaderResourceView()
{
	D3D11_TEXTURE2D_DESC desc;
	ptr()->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = desc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	d3d11::ShaderResourceView srv;
	hrexcept(s_d3d11->CreateShaderResourceView(ptr(), &srvDesc, &srv));
	return srv;
}
void TextureCube::create(uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data)
{
	remove();

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = format;
	desc.SampleDesc.Count = multiSampling;
	desc.SampleDesc.Quality = 0;
	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = bind;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	ComMethod<ID3D11Texture2D>::create(&desc, data);
}
void TextureCube::create(pcstr16 names[6])
{
	remove();

	CubeImageData data;
	if(!data.load(names)) return;
	create(D3D11_BIND_SHADER_RESOURCE, Usage::Default, 
		(DXGI_FORMAT)image::getFormatInfo(data.m_format)->dxgiFormat,
		data.m_width, data.m_height, 1, data.m_resdata);
}

TextureCubeMap::TextureCubeMap()
{
}
TextureCubeMap::TextureCubeMap(ID3D11DeviceContext * ctx, uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data)
{
	create(ctx, bind, usage, format, width, height, multiSampling, data);
}
TextureCubeMap::TextureCubeMap(ID3D11DeviceContext * ctx, pcstr16 names[6])
{
	create(ctx, names);
}
TextureCubeMap::TextureCubeMap(ID3D11DeviceContext * ctx, pcstr16 right, pcstr16 left, pcstr16 top, pcstr16 bottom, pcstr16 front, pcstr16 back)
{
	pcstr16 names[] = { right, left, top, bottom, front, back };
	create(ctx, names);
}
void TextureCubeMap::create(ID3D11DeviceContext * ctx, uint bind, d3d11::Usage usage, DXGI_FORMAT format, int width, int height, int multiSampling, D3D11_SUBRESOURCE_DATA * data)
{
	remove();

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 6;
	desc.Format = format;
	desc.SampleDesc.Count = multiSampling;
	desc.SampleDesc.Quality = 0;
	desc.Usage = (D3D11_USAGE)usage;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = bind;
	bool mipable = isSupportMipAutogen(format);
	if (mipable)
	{
		desc.MipLevels = 0;
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE |D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else
	{
		desc.MipLevels = 1;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	}

	Texture2D tex;
	hrexcept(s_d3d11->CreateTexture2D(&desc, nullptr, &tex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = mipable ? -1 : 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	hrexcept(s_d3d11->CreateShaderResourceView(tex, &srvDesc, &ptr()));

	for (int i = 0;i < 6;i++)
	{
		D3D11_SUBRESOURCE_DATA *sd = &data[i];
		ctx->UpdateSubresource(tex, i, nullptr, sd->pSysMem, sd->SysMemPitch, sd->SysMemSlicePitch);
	}
	if (mipable)
	{
		ctx->GenerateMips(ptr());
	}
}
void TextureCubeMap::create(ID3D11DeviceContext * ctx, pcstr16 names[6])
{
	remove();

	CubeImageData data;
	if(!data.load(names)) return;
	create(ctx, D3D11_BIND_SHADER_RESOURCE, Usage::Default, (DXGI_FORMAT)image::getFormatInfo(data.m_format)->dxgiFormat, data.m_width, data.m_height, 1, data.m_resdata);
}

#endif