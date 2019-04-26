#pragma once

namespace kr
{
	namespace meta
	{
		// array with zero
		template <typename T>
		struct zeroarray
		{
			operator T*() noexcept
			{
				return (T*)this;
			}
		};
		template <typename T>
		struct array_countof<zeroarray<T>>
		{
			static constexpr size_t value = 0;
		};

		namespace _pri_
		{
			template <typename T, size_t size>
			struct array_with_zero
			{
				using type = T[size];
			};
			template <typename T>
			struct array_with_zero<T,0>
			{
				using type = zeroarray<T>;
			};;
		}


		template <typename T,size_t size> using array =typename _pri_::array_with_zero<T,size>::type;

	}
}