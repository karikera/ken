#pragma once

#include <KR3/main.h>

namespace kr
{
	struct BitmapInfo
	{
	public:
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

		ptr getBits() const noexcept;
		dword getLineBytes() const noexcept;
		color * getPalette() const noexcept;

	};
}
