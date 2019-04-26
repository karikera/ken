#pragma once

#include "tdepth.h"

namespace kr
{	
	namespace gl
	{
		class Window3D;

		class RenderTarget :public HasSampler
		{
		public:
			RenderTarget() noexcept;
			~RenderTarget() noexcept;
			RenderTarget(const RenderTarget& copy) noexcept;
			RenderTarget(RenderTarget&& move) noexcept;
			RenderTarget(int width, int height, int samples = 1) noexcept;

			void createFromSwapChain() noexcept;
			void use() noexcept;
			void useWithoutDepth() noexcept;
			void setViewport() noexcept;
			int getWidth() noexcept;
			int getHeight() noexcept;
			TempDepthTexture * getDepth() noexcept;
			RenderTarget & operator=(const RenderTarget & tex) noexcept;
			RenderTarget & operator=(RenderTarget && tex) noexcept;

#ifndef KRGL_PRIVATE
		private:
#endif

#ifdef KRGL_USE_D3D11
			Keep<ID3D11Texture2D>				m_tex;
			Keep<ID3D11RenderTargetView>		m_rtv;
			Keep<ID3D11ShaderResourceView>		m_srv;
#endif

			Keep<TempDepthTexture>				m_depth;
			int m_width, m_height;
		};
	}
}