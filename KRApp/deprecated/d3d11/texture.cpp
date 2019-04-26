#define KRGL_PRIVATE

#include "texture.h"
#include <KRUtil/wl/com.h>
#include "3d.h"
#include "include_d3d11.h"

#include <new>
#include <cstdint>
#include <utility>

namespace
{
	void mipmapFiltering(const byte* src, int w, int h, byte* dest)
	{
		for(int j = 0; j < (h & ~1); j += 2)
		{
			const byte* psrc = src + (w * j * 4);
			byte*       pdest = dest + ((w >> 1) * (j >> 1) * 4);

			for(int i = 0; i < w >> 1; i++, psrc += 8, pdest += 4)
			{
				pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[w * 4 + 0] + psrc[w * 4 + 4]) >> 2;
				pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[w * 4 + 1] + psrc[w * 4 + 5]) >> 2;
				pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[w * 4 + 2] + psrc[w * 4 + 6]) >> 2;
				pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[w * 4 + 3] + psrc[w * 4 + 7]) >> 2;
			}
		}
	}
	
}

struct kr::gl::Texture::FormatData
{
	int miplevel;
	int bpp;
};

kr::gl::Texture kr::gl::Texture::empty;

kr::gl::HasSampler::HasSampler() noexcept
{
}
kr::gl::HasSampler::~HasSampler() noexcept
{
}
kr::gl::SamplerState kr::gl::HasSampler::getSamplerState() const noexcept
{
	return m_sampler;
}
void kr::gl::HasSampler::setSampleMode(SamplerState sm) const noexcept
{
	m_sampler = sm;
}
void kr::gl::HasSampler::use(int slot) const noexcept
{
	m_sampler.use(slot);
}

kr::gl::HasSampler::HasSampler(const HasSampler & copy) noexcept
{
	m_sampler = copy.m_sampler;
}
kr::gl::HasSampler::HasSampler(HasSampler && move) noexcept
{
	m_sampler = std::move(move.m_sampler);
}
kr::gl::HasSampler & kr::gl::HasSampler::operator=(const HasSampler & tex) noexcept
{
	this->~HasSampler();
	new(this) HasSampler(tex);
	return *this;
}
kr::gl::HasSampler & kr::gl::HasSampler::operator=(HasSampler && tex) noexcept
{
	this->~HasSampler();
	new(this) HasSampler(std::move(tex));
	return *this;
}

kr::gl::Texture::Texture() noexcept
{
}
kr::gl::Texture::Texture(Format cglformat, int width, int height) noexcept
{
	FormatData fmtdata;
	_create(&fmtdata, cglformat, width, height, D3D11_USAGE_DYNAMIC);
}
kr::gl::Texture::Texture(Format cglformat, int width, int height, const void* data) noexcept
{
	FormatData fmtdata;
	_create(&fmtdata, cglformat, width, height, D3D11_USAGE_DEFAULT);

	ID3D11DeviceContext * d3dcontext = wnd->m_ctx;
	d3dcontext->UpdateSubresource(m_Tex, 0, nullptr, data, width * fmtdata.bpp, width * height * fmtdata.bpp);
	if (fmtdata.miplevel > 1)
	{
		Temp<byte> mipmaps(width*height*4);

		int level = 1;
		int mipw = width >> 1;
		if (mipw < 1)
			mipw = 1;
		int miph = height >> 1;
		if (miph < 1)
			miph = 1;

		mipmapFiltering((pcbyte)data, width, height, mipmaps);
		d3dcontext->UpdateSubresource(m_Tex, level, nullptr, mipmaps, mipw * fmtdata.bpp, miph * fmtdata.bpp);
		while(mipw > 1 || miph > 1);
		{
			width = mipw;
			height = miph;
			level++;

			mipw = width >> 1;
			if (mipw < 1) mipw = 1;
			miph = height >> 1;
			if (miph < 1) miph = 1;

			mipmapFiltering(mipmaps, width, height, mipmaps);
			d3dcontext->UpdateSubresource(m_Tex, level, nullptr, mipmaps, mipw * fmtdata.bpp, miph * fmtdata.bpp);
		}
	}
}
kr::gl::Texture::Texture(const Texture & copy) noexcept
	:HasSampler(copy)
{
	m_Tex = copy.m_Tex;
	m_TexSv = copy.m_TexSv;
}
kr::gl::Texture::Texture(Texture && move) noexcept
	:HasSampler((HasSampler&&)(move))
{
	m_Tex = std::move(move.m_Tex);
	m_TexSv = std::move(move.m_TexSv);
}
kr::gl::Texture::~Texture() noexcept
{
}

void * kr::gl::Texture::lock() noexcept
{
	D3D11_MAPPED_SUBRESOURCE ms;
	if(hrshouldbe(wnd->m_ctx->Map(m_Tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
		return ms.pData;
	return nullptr;
}
void kr::gl::Texture::unlock() noexcept
{
	wnd->m_ctx->Unmap(m_Tex, 0);
}

void kr::gl::Texture::use(int slot) const noexcept
{
	ID3D11ShaderResourceView * srv;
	if (m_TexSv == nullptr)
	{
		SamplerState(SamplerState::Nearest | SamplerState::Repeat).use(0);
		srv = Texture::empty.m_TexSv;
	}
	else
	{
		HasSampler::use(slot);
		srv = m_TexSv;
	}
	wnd->m_ctx->PSSetShaderResources(slot, 1, &srv);
}
kr::gl::Texture & kr::gl::Texture::operator=(const Texture & tex) noexcept
{
	this->~Texture();
	new(this) Texture(tex);
	return *this;
}
kr::gl::Texture & kr::gl::Texture::operator=(Texture && tex) noexcept
{
	this->~Texture();
	new(this) Texture(std::move(tex));
	return *this;
}

void kr::gl::Texture::_create(FormatData* fmtdata, Format cglformat, int width, int height, D3D11_USAGE usage) noexcept
{
	DXGI_FORMAT format;
	switch(cglformat & Format::TypeMask)
	{
	case RGBA:
		fmtdata->bpp = 4;
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case Depth32f:
		fmtdata->bpp = 0;
		format = DXGI_FORMAT_D32_FLOAT;
		break;
	case Depth16:
		fmtdata->bpp = 0;
		format = DXGI_FORMAT_D16_UNORM;
		break;
	default:
		assert(!"Invalid texture format");
	}
	
	int samples = cglformat & SamplesMask;
	if (samples < 1) samples = 1;
	
	int	miplevel;
	if ((cglformat == (RGBA | GenMipmaps)))
	{
		int wmip = math::ilog2(width);
		int hmip = math::ilog2(height);
		miplevel = math::max(wmip, hmip);
	}
	else miplevel = 1;
	fmtdata->miplevel = miplevel;
	
	//
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width     = width;
	dsDesc.Height    = height;

	dsDesc.MipLevels = miplevel;
	dsDesc.ArraySize = 1;
	dsDesc.Format    = format;
	dsDesc.SampleDesc.Count = samples;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage     = usage;
	dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dsDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
	dsDesc.MiscFlags      = 0;
	
	ID3D11Device * d3ddevice = wnd->m_device;
	if (hrshouldbe(d3ddevice->CreateTexture2D(&dsDesc, nullptr, &m_Tex)))
	{
		hrshouldbe(d3ddevice->CreateShaderResourceView(m_Tex, nullptr, &m_TexSv));
	}
}

kr::gl::DepthTexture::DepthTexture() noexcept
{
}
kr::gl::DepthTexture::~DepthTexture() noexcept
{
}
kr::gl::DepthTexture::DepthTexture(int width, int height, int samples) noexcept
{
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width     = width;
	dsDesc.Height    = height;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format    = DXGI_FORMAT_D32_FLOAT;
	dsDesc.SampleDesc.Count = samples;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage     = D3D11_USAGE_DEFAULT;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags      = 0;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Device * d3ddevice = wnd->m_device;
	if (hrshouldbe(d3ddevice->CreateTexture2D(&dsDesc, nullptr, &m_tex)))
	{
		hrshouldbe(d3ddevice->CreateDepthStencilView(m_tex, nullptr, &m_dsv));
	}
}
kr::gl::DepthTexture::DepthTexture(const DepthTexture & copy) noexcept
{
	m_dsv = copy.m_dsv;
	m_tex = copy.m_tex;
}
kr::gl::DepthTexture::DepthTexture(DepthTexture && move) noexcept
{
	m_dsv = std::move(move.m_dsv);
	m_tex = std::move(move.m_tex);
}
kr::gl::DepthTexture & kr::gl::DepthTexture::operator=(const DepthTexture & tex) noexcept
{
	this->~DepthTexture();
	new(this) DepthTexture(tex);
	return *this;
}
kr::gl::DepthTexture & kr::gl::DepthTexture::operator=(DepthTexture && tex) noexcept
{
	this->~DepthTexture();
	new(this) DepthTexture(std::move(tex));
	return *this;
}

kr::gl::Texture::Format operator |(kr::gl::Texture::Format a, kr::gl::Texture::Format b) noexcept
{
	return (kr::gl::Texture::Format)(((int)a) | ((int)b));
}

void kr::gl::copy(void * dest, const void * src, std::size_t count, Texture::Format df, Texture::Format sf)
{
	static const std::size_t sizes[]=
	{
		4, // RGBA
		2, // Depth16
		4, // Depth32f
	};
	std::size_t fmtsize = sizes[(sf & Texture::TypeMask) >> 8];
	if (sf == df)
	{
		memcpy(dest, src, count * fmtsize);
		return;
	}
	if (sf == Texture::Depth16 &&  df == Texture::RGBA)
	{
		const std::uint16_t * color = (const std::uint16_t*)src;
		const std::uint16_t * end = color + count;
		std::uint32_t * fdest = (std::uint32_t*)dest;
		while (color != end)
		{
			int depthes = *color >> 8;
			depthes |= (depthes << 8);
			depthes |= (depthes << 16);
			*fdest = depthes;
			fdest++;
			color++;
		}
	}
	else
	{
		std::cerr << "Unsupported format" << std::endl;
	}
}
