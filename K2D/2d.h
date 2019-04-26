#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include "include_gdiplus.h"
#include <cstdint>
#include <cstddef>
#include <string>

namespace Gdiplus
{
	class Bitmap;
}

namespace kr
{
	namespace gl
	{
		class Context
		{
		public:
			Context() noexcept;
			~Context() noexcept;

		private:
			std::uintptr_t m_token;
		};
		class Image
		{
		public:
			Image(int width, int height) noexcept;
			~Image() noexcept;

			void getBits(void * dest) noexcept;

#ifndef C2D_PRIVATE
		private:
#endif
			Gdiplus::Bitmap m_bitmap;

		};

		enum class Composite
		{
			Copy,
			Over
		};

		class ImageCanvas :public Image
		{
		public:
			ImageCanvas(int width, int height) noexcept;
			~ImageCanvas() noexcept;

			void setFont(Text name, float size) noexcept;
			void setComposite(Composite comp) noexcept;
			void setStrokeColor(std::uint32_t color) noexcept;
			void setLineWidth(float width) noexcept;
			void setFillColor(std::uint32_t color) noexcept;
			void fillRect(const frectwh &rect) noexcept;
			void strokeRect(const frectwh &rect) noexcept;
			void fillOval(const frectwh &rect) noexcept;
			void strokeOval(const frectwh &rect) noexcept;
			void fillText(Text text, vec2 pt) noexcept;
			void fillText(Text16 text, vec2 pt) noexcept;
			void fillText(pcstr text, vec2 pt) noexcept;
			void fillText(pcstr16 text, vec2 pt) noexcept;
			void fillPoly(const vec2 * poly, std::size_t count) noexcept;

#ifndef C2D_PRIVATE
		private:
#endif
			Gdiplus::Graphics	m_g;
			Gdiplus::Pen		m_pen;
			Gdiplus::SolidBrush	m_brush;
			Gdiplus::Font		*m_font;
		};
	}
}