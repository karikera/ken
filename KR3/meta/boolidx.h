#pragma once

namespace kr
{

	namespace meta
	{
		// bool index
		template <bool first, bool... values> struct bool_index;

		template <bool... values> struct bool_index<true, values...>
		{
			constexpr static size_t value = 0;
		};;
		template <bool... values> struct bool_index<false, values...>
		{
			constexpr static size_t value = bool_index<values...>::value + 1;
		};;
	
	}
}