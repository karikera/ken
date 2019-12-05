#pragma once

#include <type_traits>

namespace kr
{
	namespace meta
	{
		template <bool b, typename A, typename B> struct type_if;
		template <typename A, typename B> struct type_if<false, A, B>
		{
			using type = B;
		};;
		template <typename A, typename B> struct type_if<true, A, B>
		{
			using type = A;
		};;
		template <bool b, typename A, typename B> using if_t = typename type_if<b,A,B>::type;
		template <typename A, typename B, typename RA, typename RB> using if_same_t = if_t<std::is_same<A,B>::value, RA, RB>;
		template <typename Base, typename Der, typename RA, typename RB> using if_base_of_t = if_t<std::is_base_of<Base, Der>::value, RA, RB>;
	}
}