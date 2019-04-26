#pragma once

namespace kr
{
	namespace meta
	{
		template <typename T, typename NT>
		struct retype;
		template <typename T, typename NT>
		using retype_t = typename retype<T, NT>::type;

		template <typename T, typename NT>
		struct retype
		{
			using type = NT;
			using from = T;
		};
		template <typename T, typename NT>
		struct retype<T&, NT>
		{
			using type = typename retype<T, NT>::type&;
			using from = typename retype<T, NT>::from;
		};
		template <typename T, typename NT>
		struct retype<T*, NT>
		{
			using type = typename retype<T, NT>::type*;
			using from = typename retype<T, NT>::from;
		};
		template <typename T, typename NT>
		struct retype<T&&, NT>
		{
			using type = typename retype<T, NT>::type&&;
			using from = typename retype<T, NT>::from;
		};
		template <typename T, typename NT>
		struct retype<const T, NT>
		{
			using type = const typename retype<T, NT>::type;
			using from = typename retype<T, NT>::from;
		};
		template <typename T, typename NT, size_t arr>
		struct retype<T[arr], NT>
		{
			using type = typename retype<T, NT>::type[arr];
			using from = typename retype<T, NT>::from;
		};
	}
}