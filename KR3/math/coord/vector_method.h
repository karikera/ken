#pragma once

#include "vector_data.h"

namespace kr
{
	namespace math
	{
		namespace _pri_
		{
			template <typename T,size_t fullcount, size_t count, bool aligned, typename order>
			class vector_method:public vector_method<T, fullcount, 4, aligned, order>
			{
			public:
				using vector_method<T, fullcount, 4, aligned, order>::vector_method;
				using vector_method<T, fullcount, 4, aligned, order>::m_data;
			};

			template <typename T, size_t fullcount, bool aligned, typename order>
			class vector_method<T, fullcount, 1, aligned, order>:public vector_constructor<T, fullcount, aligned, order>
			{
			public:
				using vector_constructor<T, fullcount, aligned, order>::vector_constructor;
				using vector_constructor<T, fullcount, aligned, order>::m_data;
			};
			template <typename T, size_t fullcount, bool aligned, typename order>
			class vector_method<T, fullcount, 2, aligned, order> :public vector_method<T, fullcount, 1, aligned, order>
			{
			public:
				using vector_method<T, fullcount, 1, aligned, order>::vector_method;
				using vector_method<T, fullcount, 1, aligned, order>::m_data;

				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_xy() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_yx() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_z(T rad) const noexcept;
				ATTR_INLINE static vector<T, fullcount, aligned, order> direction_z(T angle) noexcept;
			};
			template <typename T, size_t fullcount, bool aligned, typename order>
			class vector_method<T, fullcount, 3, aligned, order> :public vector_method<T, fullcount, 2, aligned, order>
			{
			public:
				using vector_method<T, fullcount, 2, aligned, order>::vector_method;
				using vector_method<T, fullcount, 2, aligned, order>::m_data;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_xz() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_zx() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_yz() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_zy() const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_x(T rad) const noexcept;
				ATTR_INLINE const vector<T, fullcount, aligned, order> rotate_y(T rad) const noexcept;
			};
			template <typename T, size_t fullcount, bool aligned, typename order>
			class vector_method<T, fullcount, 4, aligned, order> :public vector_method<T, fullcount, 3, aligned, order>
			{
			public:
				using vector_method<T, fullcount, 3, aligned, order>::vector_method;
				using vector_method<T, fullcount, 3, aligned, order>::m_data;
			};

		}
	}
}