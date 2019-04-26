#pragma once

#include <KR3/main.h>
#include "samplerstate.h"

#include <cstddef>

#ifdef KRGL_USE_D3D11

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
enum D3D11_USAGE;

#else 

#error Need implement

#endif

namespace kr
{

	namespace gl
	{
		class RenderTarget;

		class HasSampler
		{
		public:
			HasSampler() noexcept;
			~HasSampler() noexcept;
			SamplerState getSamplerState() const noexcept;
			void setSampleMode(SamplerState sm) const noexcept;
			void use(int slot) const noexcept;
			HasSampler(const HasSampler& copy) noexcept;
			HasSampler(HasSampler&& move) noexcept;
			HasSampler& operator =(const HasSampler& tex) noexcept;
			HasSampler& operator =(HasSampler&& tex) noexcept;


		private:
			mutable SamplerState				m_sampler;

		};
		class DepthTexture :public HasSampler
		{
		public:
			DepthTexture() noexcept;
			~DepthTexture() noexcept;
			DepthTexture(int width, int height, int samples = 1) noexcept;
			DepthTexture(const DepthTexture& copy) noexcept;
			DepthTexture(DepthTexture&& move) noexcept;
			DepthTexture & operator=(const DepthTexture & tex) noexcept;
			DepthTexture& operator =(DepthTexture&& tex) noexcept;

#ifndef KRGL_PRIVATE
		private:
#endif

#ifdef KRGL_USE_D3D11
			Keep<ID3D11Texture2D>				m_tex;
			Keep<ID3D11DepthStencilView>		m_dsv;

#endif
		};
		class Texture :public HasSampler
		{
		public:
			enum Format
			{
				RGBA = 0x0100,
				Depth16 = 0x0200,
				Depth32f = 0x0300,
				TypeMask = 0xff00,
				SamplesMask = 0x00ff,
				RenderTarget = 0x10000,
				GenMipmaps = 0x20000,
			};

			Texture() noexcept;
			Texture(Format cglformat, int width, int height) noexcept;
			Texture(Format cglformat, int width, int height, const void* data) noexcept;
			Texture(const Texture& copy) noexcept;
			Texture(Texture&& move) noexcept;
			~Texture() noexcept;

			void* lock() noexcept;
			void unlock() noexcept;
			void use(int slot) const noexcept;

			Texture& operator =(const Texture& tex) noexcept;
			Texture& operator =(Texture&& tex) noexcept;

			struct FormatData;
			void _create(FormatData* fmtdata, Format cglformat, int width, int height, D3D11_USAGE usage) noexcept;

#ifndef KRGL_PRIVATE
		private:
#endif
#ifdef KRGL_USE_D3D11
			Keep<ID3D11Texture2D>				m_Tex;
			Keep<ID3D11ShaderResourceView>		m_TexSv;
#endif

			static Texture empty;
		};

		void copy(void * dest, const void * src, std::size_t count, Texture::Format df, Texture::Format sf);
	}

}

kr::gl::Texture::Format operator |(kr::gl::Texture::Format a, kr::gl::Texture::Format b) noexcept;
