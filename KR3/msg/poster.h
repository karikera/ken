#pragma once

#ifndef __EMSCRIPTEN__
#include "pump.h"
#endif

namespace kr
{
	class PostTarget;

	class Posted
#ifndef __EMSCRIPTEN__
		: private TimerEvent
#endif
	{
		friend PostTarget;
	public:
		Posted() noexcept;

	protected:
#ifndef __EMSCRIPTEN__
		virtual void call() noexcept override = 0;
#else
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
