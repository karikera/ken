#pragma once

namespace kr
{
	
	namespace meta
	{
		constexpr size_t ilog2(size_t v)
		{
			return v == 0 ? -1 : v == 1 ? 0 : ilog2(v >> 1) + 1;
		}

		constexpr size_t pow2ceil(size_t number) noexcept
		{
			return (size_t)1 << (ilog2(number-1) + 1);
		}

		constexpr size_t pow2floor(size_t number) noexcept
		{
			return (size_t)1 << ilog2(number);
		}
	}

}