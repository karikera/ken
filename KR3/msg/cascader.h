#pragma once

#include "pool.h"
#include "poster.h"

namespace kr
{
	class ThreadCascader
	{
	private:
		Promise<void> * m_promise;

		class PromiseImpl : public Promise<void>
		{
		private:
			PostTarget * m_target;
			ThreadCascader * m_that;

		public:
			PromiseImpl(ThreadCascader * that) noexcept;

			void resolve() noexcept;
			void reject(std::exception_ptr v) noexcept;
		};

	public:
		ThreadCascader() noexcept;

		template <typename LAMBDA>
		Promise<void> * operator ()(LAMBDA lambda) noexcept
		{
			PromiseImpl * prom = _new PromiseImpl(this);
			if (!m_promise)
			{
				m_promise = prom;
				threadingVoid([prom, lambda = move(lambda)]() mutable{
					try
					{
						lambda();
						prom->resolve();
					}
					catch (...)
					{
						prom->reject(std::current_exception());
					}
				});
				return prom;
			}
			else
			{
				return m_promise = m_promise->then([prom, lambda = move(lambda)]() {
					threadingVoid([prom, lambda = move(lambda)]() mutable{
						try
						{
							lambda();
							prom->resolve();
						}
						catch (...)
						{
							prom->reject(std::current_exception());
						}
					});
					return static_cast<Promise<void>*>(prom);
				});
			}
		}
	};
}
