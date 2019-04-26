#pragma once

#include <type_traits>

namespace kr
{
	namespace meta
	{
		namespace _pri_
		{
			template <bool b,typename A, typename B> struct if_type_impl;
			template <typename A, typename B> struct if_type_impl<false, A, B>
			{
				using type = B;
			};;
			template <typename A, typename B> struct if_type_impl<true, A, B>
			{
				using type = A;
			};;
		}

		template <bool b, typename A, typename B> using if_t = typename _pri_::if_type_impl<b,A,B>::type;
		template <typename A, typename B, typename RA, typename RB> using if_same_t = if_t<is_same<A,B>::value, RA, RB>;
		template <typename Base, typename Der, typename RA, typename RB> using if_base_of_t = if_t<is_base_of<Base, Der>::value, RA, RB>;
	}
}