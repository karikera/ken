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
			Texture() = default;
			Texture(nullptr_t) noexcept;
			void generate() noexcept;
			void remove() noexcept;
			Promise<void>* load(Text16 filename) noexcept;
		};

		void krTexImage2D(GLenum target, const image::ImageData * img) noexcept;

		template <GLenum type>
		class TextureTarget final
		{
		public:
			Texture operator =(Texture texture) const noexcept
			{
				glBindTexture(type, texture.getId());
				return texture;
			}

			void image2D(const image::ImageData * img) const noexcept
			{
				krTexImage2D(type, img);
			}

			void image2DColor(uint color) const noexcept
			{
				glTexImage2D(type, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
			}

			void parameteri(GLenum name, GLenum value) const noexcept
			{
				glTexParameteri(type, name, value);
			}

			void minFilter(GLenum value) const noexcept
			{
				parameteri(GL_TEXTURE_MIN_FILTER, value);
			}

			void magFilter(GLenum value) const noexcept
			{
				parameteri(GL_TEXTURE_MAG_FILTER, value);
			}
		};

		static constexpr const TextureTarget<GL_TEXTURE_2D> texture2D = TextureTarget<GL_TEXTURE_2D>();
	}
}
