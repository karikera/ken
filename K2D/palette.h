#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include <KR3/util/resloader.h>
#include "common.h"

class kr::image::Palette :public kr::Bufferable<Palette, kr::BufferInfo<color, false, false, false, false, krb_image_palette_t> >
{
	using Super = WView<kr::color>;
public:
	static constexpr int PALETTE_SIZE = 256;

	Palette() noexcept = default;

	kr::color * $begin() noexcept;
	const kr::color * $begin() const noexcept;
	kr::color * $end() noexcept;
	const kr::color * $end() const noexcept;
	size_t $size() const noexcept;
	bool $empty() const noexcept;

	void fill(kr::color _color) noexcept;

	int getNearstColor(kr::color _c) const noexcept;

	void setEntireAlpha(byte _alpha) noexcept;

	static thread_local const Palette * defaultPalette;
};
