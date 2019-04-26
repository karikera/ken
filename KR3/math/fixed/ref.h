#pragma once

namespace kr
{
	// fixed
	template <typename T, size_t size> class TFixedBase;
	template <typename T, size_t size=2> class TFixed;
	template <typename T, size_t size=2> class TFixedPoint;
	using fixed = TFixed<short, 2>;

}
