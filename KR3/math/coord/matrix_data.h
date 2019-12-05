#pragma once

#include "vector.h"
#include "../../meta/types.h"

namespace kr
{
	namespace math
	{
		namespace _pri_
		{
			enum class ConvertMode { Copy, Loop };
			template <typename T, typename T2, size_t rows, size_t cols, bool aligned, bool aligned2, matrix_data_type to, matrix_data_type from, ConvertMode mode, typename vs, typename ss>
			struct MatrixConvertImpl;

			template <typename T, typename T2, size_t rows, size_t cols, bool aligned, bool aligned2, matrix_data_type to, matrix_data_type from, typename vs, typename ss>
			struct MatrixConvertImpl<T, T2, rows, cols, aligned, aligned2, to, from, ConvertMode::Copy, vs, ss>
			{
				ATTR_INLINE static void convert(matrix_constructor<T, rows, cols, aligned, to, vs, ss> *_to, const matrix<T2, rows, cols, aligned2, from> *_from)
				{
					static_assert(sizeof(*_to) == sizeof(*_from), "Matrix size unmatch");
					memcpy(_to, _from, sizeof(*_to));
				}
			};
			template <typename T, typename T2, size_t rows, size_t cols, bool aligned, bool aligned2, matrix_data_type to, matrix_data_type from, typename vs, typename ss>
			struct MatrixConvertImpl<T, T2, rows, cols, aligned, aligned2, to, from, ConvertMode::Loop, vs, ss>
			{
				ATTR_INLINE static void convert(matrix_constructor<T, rows, cols, aligned, to, vs, ss> *_to, const matrix<T2, rows, cols, aligned2, from> *_from)
				{
					T2 * src = (T2*)_from;
					T * dst = (T*)_to;
					T * dstend = (T*)(_to + 1);
					while (dst != dstend)
					{
						*dst++ = (T)(*src++);
					}
				}
			};

			template <typename T, typename T2, size_t rows, size_t cols, bool aligned, bool aligned2, matrix_data_type to, matrix_data_type from, typename vs, typename ss>
			struct MatrixConvert:MatrixConvertImpl<T, T2, rows, cols, aligned, aligned2, to, from, std::is_same<T,T2>::value ? ConvertMode::Copy : ConvertMode::Loop, vs, ss>
			{
			};
			template <typename T, typename T2, size_t cols, bool aligned, bool aligned2, typename vs, typename ss>
			struct MatrixConvert<T, T2, 2, cols, aligned, aligned2, matrix_data_type::right_bottom, matrix_data_type::width_height, vs, ss>
			{
				ATTR_INLINE static void convert(matrix_constructor<T, 2, cols, aligned, matrix_data_type::right_bottom, vs, ss> *_to, const matrix<T2, 2, cols, aligned2, matrix_data_type::width_height> *_from)
				{
					using dstvec = vector<T, cols, aligned>;
					_to->v[0] = (dstvec)(_from->v[0]);
					_to->v[1] = (dstvec)(_from->v[0] + _from->v[1]);
				}
			};
			template <typename T, typename T2, size_t cols, bool aligned, bool aligned2, typename vs, typename ss>
			struct MatrixConvert<T, T2, 2, cols, aligned, aligned2, matrix_data_type::width_height, matrix_data_type::right_bottom, vs, ss>
			{
				ATTR_INLINE static void convert(matrix_constructor<T, 2, cols, aligned, matrix_data_type::width_height, vs, ss> *_to, const matrix<T2, 2, cols, aligned2, matrix_data_type::right_bottom> *_from)
				{
					using dstvec = vector<T, cols, aligned>;
					_to->v[0] = (dstvec)(_from->v[0]);
					_to->v[1] = (dstvec)(_from->v[1] - _from->v[0]);
				}
			};


		}

		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		class matrix_data
		{
		public:
			vector<T, cols, aligned> v[rows];
		};

		template <typename T, bool aligned>
		class matrix_data<T, 2, 2, aligned, matrix_data_type::none>
		{
		public:
			union
			{
				vector<T, 2, aligned> v[2];
				struct
				{
					T _11, _12;
					T _21, _22;
				};
			};
		};

		template <typename T, bool aligned>
		class matrix_data<T, 3, 2, aligned, matrix_data_type::none>
		{
		public:
			union
			{
				vector<T, 2, aligned> v[3];
				struct
				{
					T _11, _12;
					T _21, _22;
					T _31, _32;
				};
			};
		};

		template <typename T, bool aligned>
		class matrix_data<T, 3, 3, aligned, matrix_data_type::none>
		{
		public:
			union
			{
				vector<T, 3, aligned> v[3];
				struct
				{
					T _11, _12, _13;
					T _21, _22, _23;
					T _31, _32, _33;
				};
			};
		};
		template <typename T, bool aligned>
		class matrix_data<T, 4, 4, aligned, matrix_data_type::none>
		{
		public:
			union
			{
				vector<T, 4, aligned> v[4];
				struct
				{
					T _11, _12, _13, _14;
					T _21, _22, _23, _24;
					T _31, _32, _33, _34;
					T _41, _42, _43, _44;
				};
			};
		};

		template <typename T, size_t cols, bool aligned>
		class matrix_data<T, 2, cols, aligned, matrix_data_type::right_bottom>
		{
		public:
			union
			{
				vector<T, cols, aligned> v[2];
				struct { T left, top, right, bottom; };
				struct { vector<T, cols, aligned> from, to; };
			};

			ATTR_INLINE matrix_data() noexcept;

			ATTR_INLINE T width() const noexcept { return right - left; };
			ATTR_INLINE T height() const noexcept { return bottom - top; };
		};

		template <typename T, size_t cols, bool aligned>
		class matrix_data<T, 2, cols, aligned, matrix_data_type::width_height>
		{
		public:
			union
			{
				vector<T, cols, aligned> v[2];
				struct { T x, y, width, height; };
				struct { vector<T, cols, aligned> pos, size; };
			};

			ATTR_INLINE matrix_data() noexcept;

			ATTR_INLINE T right() const noexcept { return x + width; };
			ATTR_INLINE T bottom() const noexcept { return y + height; };
		};

		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type, typename ... vectors, typename ... scalars>
		class matrix_constructor<T, rows, cols, aligned, type, meta::types<vectors ...>, meta::types<scalars ...>>
			:public matrix_data<T, rows, cols, aligned, type>
		{
		public:
			using matrix_data<T, rows, cols, aligned, type>::v;
			using constructor_t = matrix_constructor;

			matrix_constructor() = default;
			ATTR_INLINE matrix_constructor(const vectors & ... args) noexcept
			{
				vector<T, cols, aligned> buffer[] = { args ... };
				*this = *(matrix_constructor*)buffer;
			}
			ATTR_INLINE matrix_constructor(scalars ... args) noexcept
			{
				T buffer[] = { args ... };
				*this = *(matrix_constructor*)buffer;
			}
			template <typename T2, bool aligned2, matrix_data_type type2>
			ATTR_INLINE explicit matrix_constructor(const matrix<T2, rows, cols, aligned2, type2>& o) noexcept;

		};
	}
}


template <typename T, size_t rows, size_t cols, bool aligned, kr::math::matrix_data_type type, typename ... vectors, typename ... scalars>
template <typename T2, bool aligned2, kr::math::matrix_data_type type2>
ATTR_INLINE kr::math::matrix_constructor<T, rows, cols, aligned, type, kr::meta::types<vectors ...>, kr::meta::types<scalars ...>>::matrix_constructor(const matrix<T2, rows, cols, aligned2, type2>& o) noexcept
{
	_pri_::MatrixConvert<T, T2, rows, cols, aligned, aligned2, type, type2, kr::meta::types<vectors ...>, kr::meta::types<scalars ...>>::convert(this, &o);
}