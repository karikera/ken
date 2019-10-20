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
	using Task = Node<_pri_::TaskImpl>;

	template <typename This, typename Super = Empty>
	class TaskLambdaPost :public Super
	{
	public:
		template <typename LAMBDA>
		void post(LAMBDA lambda)
		{
			struct Data :Task
			{
				LAMBDA m_lambda;

				Data(LAMBDA lamb)
					:m_lambda(move(lamb))
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
			static_cast<This*>(this)->attach(_new Data(move(lambda)));
		}

		template <typename LAMBDA, typename CANCEL>
		void post(LAMBDA lambda, CANCEL oncancel)
		{
			struct Data :Task
			{
				LAMBDA m_lambda;
				CANCEL m_oncancel;

				Data(LAMBDA lamb, CANCEL oncancel)
					:m_lambda(move(lamb)), m_oncancel(move(oncancel))
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
			static_cast<This*>(this)->attach(_new Data(move(lambda), move(oncancel)));
		}

	};

}