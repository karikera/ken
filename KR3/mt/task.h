#pragma once

#include <KR3/main.h>
#include <KR3/util/callable.h>
#include <KR3/data/linkedlist.h>

namespace kr
{
	namespace _pri_
	{
		class TaskImpl :public Callable
		{
		public:
			virtual void cancel() noexcept;
		};
	}

	template <typename This, typename Super = Empty>
	class TaskLambdaPost :public Super
	{
	public:
		template <typename LAMBDA>
		void post(LAMBDA &&lambda)
		{
			struct Data :Task
			{
				decay_t<LAMBDA> m_lambda;

				Data(LAMBDA lamb)
					:m_lambda(forward<LAMBDA>(lamb))
				{
				}
				~Data() noexcept override
				{
				}
				void operator ()() override
				{
					m_lambda();
				}
			};
			static_cast<This*>(this)->attach(_new Data(forward<LAMBDA>(lambda)));
		}

		template <typename LAMBDA, typename CANCEL>
		void post(LAMBDA &&lambda, CANCEL oncancel)
		{
			struct Data :Task
			{
				decay_t<LAMBDA> m_lambda;
				CANCEL m_oncancel;

				Data(LAMBDA lamb, CANCEL oncancel)
					:m_lambda(forward<LAMBDA>(lamb)), m_oncancel(forward<CANCEL>(oncancel))
				{
				}
				~Data() noexcept override
				{
				}
				void operator ()() override
				{
					m_lambda();
				}
				void cancel() noexcept override
				{
					m_oncancel();
				}
			};
			static_cast<This*>(this)->attach(_new Data(forward<LAMBDA>(lambda), forward<CANCEL>(oncancel)));
		}

	};

}