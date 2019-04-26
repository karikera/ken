#pragma once

#include "texture.h"
#include <functional>

namespace kr
{
	namespace gl
	{
		struct Resolution
		{
			int w, h, ms;
			bool operator ==(const Resolution & res) const noexcept;
			bool operator !=(const Resolution & res) const noexcept;
		};
	}
}

template <> struct std::hash<kr::gl::Resolution>
{
	std::size_t operator()(const kr::gl::Resolution& _Keyval) const;
};
namespace kr
{

	namespace gl
	{
		class TempDepthTexture :public DepthTexture
		{
		public:
			static TempDepthTexture* create(int w, int h, int ms) noexcept;
			void AddRef() noexcept;
			void Release() noexcept;

		private:
			using DepthTexture::DepthTexture;
			TempDepthTexture(const Resolution & resol) noexcept;
			~TempDepthTexture() noexcept;

			int m_ref;
			Resolution m_resol;
		};
	}
}