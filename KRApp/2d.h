#pragma once

#include "app.h"
#include "image.h"
#include <K2D/image.h>

namespace kr
{
	class WebCanvas2D :public Application
	{
	public:
		WebCanvas2D() noexcept;
		~WebCanvas2D() noexcept;

		void create(int width, int height) noexcept;

		struct Composite
		{
			Composite& operator =(const char * str) noexcept;
		};
		struct StrokeStyle
		{
			StrokeStyle& operator =(uint32_t style) noexcept;
		};
		struct LineWidth
		{
			LineWidth& operator =(float width) noexcept;
		};
		struct FillStyle
		{
			FillStyle& operator =(uint32_t style) noexcept;
		};
		StrokeStyle strokeStyle;
		FillStyle fillStyle;
		LineWidth lineWidth;
		Composite globalCompositeOperation;

		void save() noexcept;
		void restore() noexcept;
		void translate(float x, float y) noexcept;
		void rotate(float radian) noexcept;
		void rect(float x, float y, float width, float height) noexcept;
		void oval(float x, float y, float width, float height) noexcept;
		void clearRect(float x, float y, float width, float height) noexcept;
		void strokeRect(float x, float y, float width, float height) noexcept;
		void fillRect(float x, float y, float width, float height) noexcept;
		void strokeOval(float x, float y, float width, float height) noexcept;
		void fillOval(float x, float y, float width, float height) noexcept;
		void fillText(Text text, float x, float y) noexcept;
		void fillText(Text16 text, float x, float y) noexcept;
		void drawImage(WebImage * image, float x, float y) noexcept;
		void drawImage(WebImage * image, float x, float y, float width, float height) noexcept;
		void drawImage(WebImage * image, float srcX, float srcY, float srcWidth, float srcHeight, float destX, float destY, float destWidth, float destHeight) noexcept;
		bool putImageData(const image::ImageData& image, int x, int y) noexcept;

		void beginPath() noexcept;
		void closePath() noexcept;
		void stroke() noexcept;
		void fill() noexcept;
		void clip() noexcept;
		void moveTo(float x, float y) noexcept;
		void lineTo(float x, float y) noexcept;
		void arc(float x, float y, float radius, float startAngle, float endAngle, bool counterClockwise = false) noexcept;
		void quadraticCurveTo(float cpx, float cpy, float x, float y) noexcept;

		void exRoundRect(float x, float y, float width, float height, float radius) noexcept;

	protected:
		void _flush() noexcept override;
	};
}
