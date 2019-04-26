#pragma once


namespace kr
{
	namespace meta
	{
		// align of
		template <size_t SIZE> struct align_of
		{
			static constexpr size_t value = ((SIZE % 8) == 0) * 4 + ((SIZE % 4) == 0) * 2 + ((SIZE % 2) == 0) * 1 + 1;
		};
	}
}