#pragma once

#include <type_traits>

namespace kr
{
	namespace meta
	{
		namespace _pri_
		{
			template <typename T, bool is_const> struct voidptr_impl 
			{
				using type = const void*;
			};
			template <typename T> struct voidptr_impl <T, false>
			{
				using type = void*;
			};;
			template <typename T> struct voidptr:voidptr_impl<T, std::is_const<T>::value>
			{
			};;
		}
		template <typename T> using voidptr = typename _pri_::voidptr<T>::type;
	}
}
