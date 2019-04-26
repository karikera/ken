#pragma once

#include <K2D/2d.h>
#include "../texture.h"

namespace kr
{
	namespace gl
	{

		class TextureCanvas :public ImageCanvas
		{
		public:
			TextureCanvas(int width, int height) noexcept;
			void use(int slot) noexcept;

		private:
			kr::gl::Texture m_texture;
			int m_width, m_height;
		};

	}
}