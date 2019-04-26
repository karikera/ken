#pragma once

#include "../main.h"

namespace kr
{
	namespace _pri_
	{
		template <char ... > struct BIN;
		template <char first, char ... lefts> struct BIN<first, lefts...>
		{
			static_assert(first == '0' || first == '1', "Need 0 or 1");
			static constexpr uintptr_t value = ((first-'0') << (sizeof ... (lefts))) | BIN<lefts...>::value;
		};
		template <> struct BIN<>
		{
			static constexpr uintptr_t value = 0;
		};
	}

}

template <char ... strs> int operator ""_bin()
{
	return (int)kr::_pri_::BIN<strs...>::value;
}