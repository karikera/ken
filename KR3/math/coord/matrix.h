#pragma once

#include "matrix_method.h"
#include <iostream>

namespace kr
{
	namespace math
	{
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		class matrix
			:public matrix_method<T, rows, cols, aligned, type>
		{
		public:
			using matrix_method<T, rows, cols, aligned, type>::matrix_method;
			using matrix_method<T, rows, cols, aligned, type>::v;
		};
	}
}
