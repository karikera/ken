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

		template <bool value>
		using bool_value = cvalue<bool, value>;
		using bool_true = bool_value<true>;
		using bool_false = bool_value<false>;
		template <int value>
		using int_value = cvalue<int, value>;
	}
}