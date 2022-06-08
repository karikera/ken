#pragma once

#include <KR3/util/time.h>
#ifdef WIN32
#include "pump.h"
#endif

namespace kr
{
	class Timeout
#ifdef WIN32
		: private TimerEvent
#endif
	{
	public:
		Timeout() noexcept;
		bool post(duration to) noexcept;
		bool cancel() noexcept;

	protected:
#ifdef WIN32
		virtual void call() noexcept override = 0;
#elif defined(__EMSCRIPTEN__)
		virtual void call() noexcept = 0;
#endif

	private:
#ifdef __EMSCRIPTEN__
		int m_timeoutId;
#endif

	public:
		template <typename LAMBDA>
		static Timeout * create(LAMBDA &&lambda) noexcept
		{
			struct LambdaWrap :public Timeout
			{
				decay_t<LAMBDA> m_lambda;
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

	class Interval
	{
	public:
		Interval() noexcept;
		bool start(duration interval) noexcept;
		bool stop() noexcept;

	protected:
		virtual void call() noexcept = 0;

	private:
#ifdef WIN32
		class Wrapper : public TimerEvent
		{
		public:
			using TimerEvent::TimerEvent;
			bool start(duration interval) noexcept;

		protected:
			void call() noexcept override;

		private:
			duration m_interval;
		};
		Wrapper m_wrapper;
#endif
#ifdef __EMSCRIPTEN__
		int m_intervalId;
#endif

	public:
		template <typename LAMBDA>
		static Interval * create(LAMBDA &&lambda) noexcept
		{
			struct LambdaWrap :public Interval
			{
				decay_t<LAMBDA> m_lambda;
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

	template <typename LAMBDA>
	Interval* setInterval(LAMBDA &&lambda, int interval) noexcept
	{
		return setInterval(forward<LAMBDA>(lambda), (duration)interval);
	}

	template <typename LAMBDA>
	Interval * setInterval(LAMBDA &&lambda, duration interval) noexcept
	{
		Interval * obj = Interval::create(forward<LAMBDA>(lambda));
		obj->start(interval);
		return obj;
	}

	template <typename LAMBDA>
	Timeout* setTimeout(LAMBDA &&lambda, int dura) noexcept
	{
		return setTimeout(forward<LAMBDA>(lambda), (duration)dura);
	}

	template <typename LAMBDA>
	Timeout* setTimeout(LAMBDA &&lambda, duration dura) noexcept
	{
		Timeout * obj = Timeout::create(forward<LAMBDA>(lambda));
		obj->post(dura);
		return obj;
	}
}
