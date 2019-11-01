#include "palette.h"
#include "palette.h"
#include "palette.h"
#include "palette.h"

thread_local const kr::image::Palette * kr::image::Palette::defaultPalette;

kr::color * kr::image::Palette::$begin() noexcept
{
	return (kr::color*)color;
}

const kr::color * kr::image::Palette::$begin() const noexcept
{
	return (kr::color*)color;
}

kr::color * kr::image::Palette::$end() noexcept
{
	return (kr::color*)color + size();
}

const kr::color * kr::image::Palette::$end() const noexcept
{
	return (kr::color*)color + size();
}

size_t kr::image::Palette::$size() const noexcept
{
	return countof(color);
}

bool kr::image::Palette::$empty() const noexcept
{
	return false;
}

void kr::image::Palette::fill(kr::color _color) noexcept
{
	for (kr::color &c : *this)
		c = _color;
}

int kr::image::Palette::getNearstColor(kr::color _c) const noexcept
{
	ivec4 _axis = (ivec4)_c;
	int minIdx = 0;
	int minDist = ((ivec4)(*this)[0] - _axis).length_sq();
	for (int i = 1; i < (int)size(); i++)
	{
		int dist = ((ivec4)(*this)[i] - _axis).length_sq();
		if (dist >= minDist)
			continue;
		if (dist == 0)
			return i;
		minIdx = i;
	}
	return minIdx;
}

void kr::image::Palette::setEntireAlpha(byte _alpha) noexcept
{
	for (kr::color &c : *this)
		c.a = _alpha;
}
