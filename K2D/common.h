#pragma once

#include <KR3/util/resloader.h>

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
