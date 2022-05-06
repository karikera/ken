#pragma once

#include <KR3/msg/promise.h>
#include <K2D/image.h>

#include "globject.h"

namespace kr {
	namespace gl {
		class TextureData;

		class Texture:public Object {
		public:
			Texture() = default;
			Texture(nullptr_t) noexcept;
			void generate() noexcept;
			void remove() noexcept;
			Promise<void>* load(Text16 filename, bool potResize) noexcept;
			void load(const image::ImageData & image) noexcept;
			void load(const TextureData & image) noexcept;
		};

		class TextureData: private image::ImageData {
		public:
			TextureData() noexcept;
			TextureData(const image::ImageData& image, bool potResize) noexcept;
			TextureData(const image::ImageData& image, bool useAlpha, bool potResize) noexcept;
			TextureData(image::ImageData&& image, bool resize) noexcept;
			TextureData(TextureData&& data) noexcept;
			~TextureData() noexcept;

			void image2D(GLenum target) const noexcept;
			void subImage2D(GLenum target, int xoffset, int yoffset) const noexcept;
			void free() noexcept;
			TextureData subimage(int _x, int _y, int _w, int _h) const noexcept;

			static Promise<TextureData>* load(Text16 filename, bool potResize) noexcept;

			using ImageData::operator =;
			using ImageData::getWidth;
			using ImageData::getHeight;
			using ImageData::getBits;

			GLint format;

		private:
			bool m_allocated;
		};

		template <GLenum target>
		class TextureTarget final {
		public:
			Texture operator =(Texture texture) const noexcept {
				glCheck();
				glBindTexture(target, texture.getId());
				glCheck();
				return texture;
			}

			void image2D(const image::ImageData & img) const noexcept {
				image2D(TextureData(img, true));
			}

			void subImage2D(const image::ImageData& img, int xoffset, int yoffset) const noexcept {
				subImage2D(TextureData(img, false), xoffset, yoffset);
			}

			void image2D(const TextureData& img) const noexcept {
				img.image2D(target);
			}

			void subImage2D(const TextureData& img, int xoffset, int yoffset) const noexcept {
				img.subImage2D(target, xoffset, yoffset);
			}

			void subImage2DPixel(colorr color, int x, int y) const noexcept {
				glCheck();
				glTexSubImage2D(target, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
				glCheck();
			}

			void image2D(GLsizei width, GLsizei height, bool alpha) const noexcept {
				GLint format;
				if (alpha) {
					format = GL_RGBA;
				}
				else {
					format = GL_RGB;
				}

				glCheck();
				glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				glCheck();
			}

			void image2DColor(colorr color) const noexcept {
				glCheck();
				glTexImage2D(target, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
				glCheck();
			}

			void parameteri(GLenum name, GLenum value) const noexcept {
				glCheck();
				glTexParameteri(target, name, value);
				glCheck();
			}
			void wrapX(GLenum value) const noexcept {
				glCheck();
				glTexParameteri(target, GL_TEXTURE_WRAP_S, value);
				glCheck();
			}
			void wrapY(GLenum value) const noexcept {
				glCheck();
				glTexParameteri(target, GL_TEXTURE_WRAP_T, value);
				glCheck();
			}

			void minFilter(GLenum value) const noexcept {
				glCheck();
				parameteri(GL_TEXTURE_MIN_FILTER, value);
				glCheck();
			}

			void magFilter(GLenum value) const noexcept {
				glCheck();
				parameteri(GL_TEXTURE_MAG_FILTER, value);
				glCheck();
			}
		};

		static constexpr const TextureTarget<GL_TEXTURE_2D> texture2D = TextureTarget<GL_TEXTURE_2D>();
	}
}
