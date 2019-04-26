#pragma once

namespace kr
{
	namespace meta
	{
		template <typename T, T _value>
		struct cvalue
		{
			static constexpr T value = _value;
		};

		struct bool_true:cvalue<bool, true> {};
		struct bool_false:cvalue<bool, false> {};
		template <int value>
		struct int_value :cvalue<int, value> {};
	}
}