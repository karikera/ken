#pragma once

#include <KR3/main.h>
#include "palette.h"

namespace kr
{
	struct BITMAP_FILE
	{
		dword      biSize;
		long       biWidth;
		long       biHeight;
		word       biPlanes;
		word       biBitCount;
		dword      biCompression;
		dword      biSizeImage;
		long       biXPelsPerMeter;
		long       biYPelsPerMeter;
		dword      biClrUsed;
		dword      biClrImportant;

		ptr		getBits() const noexcept;
		dword	getLineBytes() const noexcept;
		const image::Palette* getPalette() const noexcept;
	};

}
