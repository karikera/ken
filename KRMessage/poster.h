#pragma once

#ifdef WIN32
#include "pump.h"
#endif

namespace kr
{
	class PostTarget;

	class Posted
#ifdef WIN32
		: private EventPump::Timer
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
				LAMBDA m_lambda;
				LambdaWrap(LAMBDA &&lambda) noexcept
					: m_lambda(forward<LAMBDA>(lambda))
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
		bool post(Posted * data) noexcept;
		template <typename LAMBDA>
		bool postL(LAMBDA && lambda) noexcept
		{
			return post(Posted::create(forward<LAMBDA>(lambda)));
		}
	};
}
