#include "stdafx.h"
#include "bitmap.h"

kr::ptr kr::BitmapInfo::getBits() const noexcept
{
	return (byte*)this+biSize;
}
kr::dword kr::BitmapInfo::getLineBytes() const noexcept
{
	return (biWidth * biBitCount + 31) >> 5 << 2;
}
kr::color * kr::BitmapInfo::getPalette() const noexcept
{
	return (color*)(((byte*)this) + sizeof(BitmapInfo));
}
