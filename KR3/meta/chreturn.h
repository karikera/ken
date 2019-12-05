#pragma once

#include "../main.h"
#include "function.h"
#include <type_traits>

namespace kr
{
	
	namespace meta
	{
		// chretrun
		namespace _pri_
		{
			template <typename lambda_t> struct LambdaContainer
			{
				lambda_t m_lambda;

				LambdaContainer(lambda_t &&lambda)
					:m_lambda(move(lambda))
				{
				}
				LambdaContainer(const lambda_t &lambda)
					:m_lambda(lambda)
				{
				}
			};
			template <typename cast_t, typename lambda_t> struct ChReturn
			{
				template <typename ret_t, bool is_class, typename ... args_t> struct CastWrapReturn
					:LambdaContainer<lambda_t>
				{
					using LambdaContainer<lambda_t>::LambdaContainer;
					cast_t operator()(args_t ... args)
					{
						return (cast_t)m_lambda(args ...);
					}
					cast_t operator()(args_t ... args) const
					{
						return (cast_t)m_lambda(args ...);
					}
				};
				template <typename ... args_t> struct CastWrapReturn<void, true, args_t ...>
					:LambdaContainer<lambda_t>
				{
					using LambdaContainer<lambda_t>::LambdaContainer;
					cast_t operator()(args_t ... args)
					{
						m_lambda(args ...);
						return cast_t();
					}
					cast_t operator()(args_t ... args) const
					{
						return (cast_t)m_lambda(args ...);
					}
				};
				template <typename ... args_t> struct CastWrapReturn<void, false, args_t ...>
					:LambdaContainer<lambda_t>
				{
					using LambdaContainer<lambda_t>::LambdaContainer;
					cast_t operator()(args_t ... args)
					{
						m_lambda(args ...);
						return zerovar;
					}
					cast_t operator()(args_t ... args) const
					{
						m_lambda(args ...);
						return zerovar;
					}
				};;
				template <typename ... args_t> struct types
				{
					using type = CastWrapReturn<typename function<lambda_t>::return_t, std::is_class<lambda_t>::value, args_t ...>;
				};
			};
		}

		template <typename cast_t, typename lambda_t>
		using ChReturn = typename meta::typesExpand<typename function<lambda_t>::args_t, _pri_::ChReturn<cast_t, lambda_t>::template types>::type;

		template <typename cast_t, typename lambda_t> auto chreturn(lambda_t lambda)
		{
			return (ChReturn<cast_t, lambda_t>)(lambda);
		};;
		template <typename lambda_t> auto returnBool(lambda_t lambda)
		{
			return chreturn<bool>(lambda);
		};;
	}

}