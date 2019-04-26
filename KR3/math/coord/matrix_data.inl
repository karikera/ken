#pragma once

#include "matrix_data.h"

namespace kr
{
	namespace math
	{
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE matrix_data<T, 2, cols, aligned, matrix_data_type::right_bottom>::matrix_data() noexcept
			:from(), to()
		{
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE matrix_data<T, 2, cols, aligned, matrix_data_type::width_height>::matrix_data() noexcept
			: pos(), size()
		{
		}
	}
}

