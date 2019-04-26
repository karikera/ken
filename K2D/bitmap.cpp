#include "bitmap.h"

kr::ptr kr::BITMAP_FILE::getBits() const noexcept
{
	return (byte*)this+biSize;
}
kr::dword kr::BITMAP_FILE::getLineBytes() const noexcept
{
	return (biWidth * biBitCount + 31) >> 5 << 2;
}
const kr::image::Palette * kr::BITMAP_FILE::getPalette() const noexcept
{
	return (image::Palette*)((byte*)this + sizeof(BITMAP_FILE));
}
