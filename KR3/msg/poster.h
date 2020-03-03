#pragma once

#ifdef WIN32
#include "pump.h"
#endif

namespace kr
{
	class PostTarget;

	class Posted
#ifdef WIN32
		: private TimerEvent
#endif
	{
		friend PostTarget;
	public:
		Posted() noexcept;

	protected:
#ifdef WIN32
		virtual void call() noexcept override = 0;
#elif defined(__EMSCRIPTEN__)
		virtual void call() noexcept = 0;
#endif

	private:
#ifdef __EMSCRIPTEN__
		int m_workerId;
#endif

	public:
		template <typename LAMBDA>
		static Posted * create(LAMBDA &&lambda) noexcept
		{
			struct LambdaWrap :public Posted
			{
				decay_t<LAMBDA> m_lambda;
				LambdaWrap(const decay_t<LAMBDA>&lambda) noexcept
					: m_lambda(lambda)
				{
				}
				LambdaWrap(decay_t<LAMBDA>&&lambda) noexcept
					: m_lambda(move(lambda))
				{
				}
				void call() noexcept override
				{
					m_lambda(this);
				}
			};
			return _new LambdaWrap(forward<LAMBDA>(lambda));
		}
	};

	class PostTarget
	{
	public:
		PostTarget() = delete;
		static PostTarget * getCurrent() noexcept;
		bool attach(Posted * data) noexcept;
		template <typename LAMBDA>
		bool post(LAMBDA &&lambda) noexcept
		{
			return attach(Posted::create(forward<LAMBDA>(lambda)));
		}
	};
}
