#pragma once

#include <KR3/util/time.h>
#ifdef WIN32
#include "pump.h"
#endif

namespace kr
{
	class Timeout
#ifdef WIN32
		: private EventPump::Timer
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
		static Timeout * create(LAMBDA lambda) noexcept
		{
			struct LambdaWrap :public Timeout
			{
				LAMBDA m_lambda;
				LambdaWrap(LAMBDA lambda) noexcept
					: m_lambda(move(lambda))
				{
				}
				void call() noexcept override
				{
					m_lambda(this);
				}
			};
			return _new LambdaWrap(move(lambda));
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
		class Wrapper : public EventPump::Timer
		{
		public:
			using EventPump::Timer::Timer;
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
		static Interval * create(LAMBDA lambda) noexcept
		{
			struct LambdaWrap :public Interval
			{
				LAMBDA m_lambda;
				LambdaWrap(LAMBDA lambda) noexcept
					: m_lambda(move(lambda))
				{
				}
				void call() noexcept override
				{
#ifdef WIN32
					_repost();
					m_lambda(this);
#elif defined(__EMSCRIPTEN__)
					m_lambda(this);
#endif
				}
			};
			return _new LambdaWrap(move(lambda));
		}
	};

	template <typename LAMBDA>
	Interval* setInterval(LAMBDA lambda, int interval) noexcept
	{
		return setInterval(move(lambda), (duration)interval);
	}

	template <typename LAMBDA>
	Interval * setInterval(LAMBDA lambda, duration interval) noexcept
	{
		Interval * obj = Interval::create(move(lambda));
		obj->start(interval);
		return obj;
	}

	template <typename LAMBDA>
	Timeout* setTimeout(LAMBDA lambda, int dura) noexcept
	{
		return setTimeout(move(lambda), (duration)dura);
	}

	template <typename LAMBDA>
	Timeout* setTimeout(LAMBDA lambda, duration dura) noexcept
	{
		Timeout * obj = Timeout::create(move(lambda));
		obj->post(dura);
		return obj;
	}
}
