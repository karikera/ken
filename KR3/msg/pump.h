#pragma once

#include <KR3/util/callable.h>
#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/util/keeper.h>
#include "promise.h"

namespace kr
{
	struct EventProcedure
	{
		EventHandle * event;
		void * param;
		void(*callback)(void*);
	};
	class EventPump:public Interface<Empty>
	{
		friend void PromiseRaw::_setState(State state) noexcept;
	public:
		constexpr static dword MAXIMUM_WAIT = EventHandle::MAXIMUM_WAIT - 1;
		class Timer;

	private:
		class NodeHead
		{
			friend EventPump;
		protected:
			Timer * m_next;
		};
	public:
		class Timer :public Interface<NodeHead>
		{
			friend EventPump;
			friend Referencable<Timer>;
		public:
			Timer() noexcept;
			Timer(timepoint at) noexcept;
			virtual ~Timer() noexcept;
			bool isPosted() noexcept;
			void setTime(timepoint time) noexcept;
			void addTime(duration time) noexcept;
			timepoint getTime() const noexcept;
			void AddRef() noexcept;
			size_t Release() noexcept;
			size_t getRefCount() noexcept;

			template <typename LAMBDA>
			static Timer * create(timepoint at, LAMBDA &&lambda) noexcept;
		protected:
			virtual void call() = 0;

			size_t m_ref;
			Timer * m_prev;
			timepoint m_at;
		};
	public:
		void quit(int exitCode) noexcept;
		void waitAll() noexcept;
		ATTR_DEPRECATED("all tasks must complete") void clearTasks() noexcept;

		// node는 내부에서 관리하게 된다.
		// return: 이미 만료 혹은 취소된 이벤트의 경우 false
		bool cancel(Timer * node) noexcept;

		// newnode는 내부에서 관리하게된다.
		void attach(Timer * newnode) noexcept;

		template <typename LAMBDA>
		void post(LAMBDA &&lambda) noexcept
		{
			return attach(Timer::create(timepoint::now(), forward<LAMBDA>(lambda)));
		}

		template <typename LAMBDA>
		void post(timepoint at, LAMBDA &&lambda) noexcept
		{
			return attach(Timer::create(at, forward<LAMBDA>(lambda)));
		}

		template <typename LAMBDA>
		Timer* makePost(timepoint at, LAMBDA &&lambda) noexcept
		{
			Timer * node = Timer::create(at, forward<LAMBDA>(lambda));
			node->AddRef();
			if (attach(node))
			{
				return node;
			}
			node->Release();
			return nullptr;
		}

		template <typename LAMBDA>
		void post(duration wait, LAMBDA &&lambda) noexcept
		{
			return post(timepoint::now() + wait, forward<LAMBDA>(lambda));
		}

		template <typename LAMBDA>
		Timer* makePost(duration wait, LAMBDA &&lambda) noexcept
		{
			return makePost(Timer::create(timepoint::now() + wait, forward<LAMBDA>(lambda)));
		}

		// events를 대기한다.
		// return: 셋된 이벤트의 인덱스
		dword wait(View<EventHandle *> events) throws(QuitException);

		// events를 time동안 대기한다.
		// return: 셋된 이벤트의 인덱스
		dword wait(View<EventHandle *> events, duration time) throws(QuitException);

		// events를 timeto까지 대기한다.
		// return: 셋된 이벤트의 인덱스
		dword waitTo(View<EventHandle *> events, timepoint timeto) throws(QuitException);

		void processOnce() throws(QuitException);
		dword processOnce(View<EventHandle *> events) throws(QuitException);
		void processOnce(View<EventProcedure> proc) throws(QuitException);
		void processOnceWithoutMessage() throws(QuitException);
		void processOnceWithoutMessage(View<EventProcedure> proc) throws(QuitException);
		void wait(EventHandle * event) throws(QuitException);
		void wait(EventHandle * event, duration time) throws(QuitException);
		void waitTo(EventHandle * event, timepoint time) throws(QuitException);
		void AddRef() noexcept;
		void Release() noexcept;

		Promise<void> * promise(duration time) noexcept;
		Promise<void> * promiseTo(timepoint at) noexcept;

		void sleep(duration dura) throws(QuitException);
		void sleepTo(timepoint time) throws(QuitException);

		int messageLoop() noexcept;
		int messageLoopWith(View<EventProcedure> proc) noexcept;

		// use to fix webasm
		size_t getPromiseCount() noexcept;

		static EventPump * getInstance() noexcept;

	private:
		EventPump() noexcept;
		~EventPump() noexcept;

		void _processPromise() noexcept;
		dword _tryProcess(EventHandle * const * events, dword count) throws(QuitException);
		TmpArray<EventHandle *> _makeEventArray(View<EventHandle *> events) noexcept;
		TmpArray<EventHandle *> _makeEventArray(View<EventProcedure> proc) noexcept;
		void _processMessage() throws(QuitException);
		dword _processTimer(dword maxSleep) throws(QuitException);

		CriticalSection m_timercs;
		EventHandle* m_msgevent;
		NodeHead m_start;
		ThreadId m_threadId;
		atomic<size_t> m_reference;
		
	private:
		PromiseRaw* m_process;
		PromiseRaw** m_pprocess;
	};

	namespace _pri_
	{
		template <size_t argn>
		struct CallWithParamIfHas
		{
			template <typename LAMBDA>
			static void call(LAMBDA && lambda, EventPump::Timer * param) throws(...)
			{
				lambda(param);
			}
		};
		template <>
		struct CallWithParamIfHas<0>
		{
			template <typename LAMBDA>
			static void call(LAMBDA && lambda, EventPump::Timer * param) throws(...)
			{
				lambda();
			}
		};
	}

	template <typename LAMBDA>
	static EventPump::Timer * EventPump::Timer::create(timepoint at, LAMBDA &&lambda) noexcept
	{
		struct LambdaWrap :public Timer
		{
			decay_t<LAMBDA> m_lambda;
			LambdaWrap(LAMBDA &&lambda, timepoint at) noexcept
				: Timer(at), m_lambda(forward<LAMBDA>(lambda))
			{
			}
			void call() throws(...) override
			{
				constexpr size_t argn = meta::function<LAMBDA>::args_t::size;
				_pri_::CallWithParamIfHas<argn>::call(m_lambda, this);
			}
		};
		return _new LambdaWrap(forward<LAMBDA>(lambda), at);
	}

	class EventThread
	{
	public:
		EventThread() noexcept;
		~EventThread() noexcept;

		void create() noexcept;
		void quit(int exitCode) noexcept;
		EventPump * operator ->() noexcept;

	private:
		ThreadHandle * m_thread;
		EventPump * m_pump;
	};

	using TimerEvent = EventPump::Timer;
}
