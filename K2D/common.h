#pragma once

#include <KRUtil/resloader.h>

namespace kr
{
	namespace image
	{
		using PixelFormat = kr_pixelformat_t;
		class Palette;
		class ImageData;
		template <PixelFormat pf>
		class MemCanvas;
		template <PixelFormat pf>
		class MemBitmap;
	}
}
