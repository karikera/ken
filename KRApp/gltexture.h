#pragma once

#include <KR3/msg/promise.h>
#include <K2D/image.h>

#include "globject.h"

namespace kr
{
	namespace gl
	{
		class Texture:public Object
		{
		public:
			Texture() noexcept = default;
			Texture(nullptr_t) noexcept;
			void generate() noexcept;
			void remove() noexcept;
			Promise<void>* load(Text16 filename) noexcept;
		};

		void krTexImage2D(GLenum target, const image::ImageData * img) noexcept;

		template <GLenum type>
		class TextureTarget
		{
		public:
			Texture operator =(Texture texture) noexcept
			{
				glBindTexture(type, texture.getId());
				return texture;
			}

			void image2D(const image::ImageData * img) noexcept
			{
				krTexImage2D(type, img);
			}

			void image2DColor(uint color) noexcept
			{
				glTexImage2D(type, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
			}

			void parameteri(GLenum name, GLenum value) noexcept
			{
				glTexParameteri(type, name, value);
			}

			void minFilter(GLenum value) noexcept
			{
				parameteri(GL_TEXTURE_MIN_FILTER, value);
			}

			void magFilter(GLenum value) noexcept
			{
				parameteri(GL_TEXTURE_MAG_FILTER, value);
			}
		};

		static TextureTarget<GL_TEXTURE_2D> &texture2D = nullref;
	}
}
