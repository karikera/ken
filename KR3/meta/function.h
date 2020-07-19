#pragma once

#include "types.h"

namespace kr
{
	namespace meta
	{
		// varaidic  template index
		namespace _pri_
		{
			template <size_t index, typename T, typename ... ARGS> struct variadic_index_of_impl:variadic_index_of_impl<index-1, ARGS ...>
			{
			};;
			template <typename T, typename ... ARGS> class variadic_index_of_impl<0, T, ARGS ...>
			{
				using type = T;
			};;
			struct function_clean
			{
				static constexpr bool is_class = false;
				static constexpr bool has_class = false;
				static constexpr bool is_const = false;
			};
		}
		template <size_t index, typename ... args> using variadic_index_of = typename _pri_::variadic_index_of_impl<index, args ...>::type;
	
		// function
		template <typename t> struct function:function<decltype(&t::operator())>
		{
			static constexpr bool is_class = true;
			using lambda_t = t;
		};
		template <typename ret, typename ... ARGS> struct function<ret(ARGS ...)> :_pri_::function_clean
		{
			using return_t = ret;
			using lambda_t = ret(*)(ARGS ...);
			using args_t = types<ARGS ...>;
		};;
		template <typename ret, typename ... ARGS> struct function<ret(*)(ARGS ...)>:function<ret(ARGS ...)>
		{
		};;
		template <typename cls, typename ret, typename ... ARGS> struct function<ret(cls::*)(ARGS ...)> :_pri_::function_clean
		{
			using return_t = ret;
			using class_t = cls;
			using lambda_t = ret(cls::*)(ARGS ...);
			using args_t = types<ARGS ...>;
			static constexpr bool has_class = true;
		};;
		template <typename cls, typename ret, typename ... ARGS> struct function<ret(cls::*)(ARGS ...) const>: function<ret(cls::*)(ARGS ...)>
		{
			using lambda_t = ret(cls::*)(ARGS ...) const;
			static constexpr bool is_const = true;
		};
		template <typename t> struct function<const t> :function<t>
		{
		};
		template <typename t> struct function<t&> :function<t>
		{
		};
		template <typename t> struct function<t&&> :function<t>
		{
		};


		template <typename func, size_t index>
		using parameterAt = typeAt<typename meta::function<func>::args_t, index>;
	}

}