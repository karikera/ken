#pragma once

#include <KR3/util/callable.h>
#include <KR3/util/keeper.h>
#include <KR3/mt/criticalsection.h>

#ifdef WIN32
#include <KR3/win/threadid.h>
#endif

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
		friend TimerEvent;
		friend PromisePump;
	public:
		constexpr static dword MAXIMUM_WAIT = EventHandleMaxWait - 1;

	private:
		class NodeHead
		{
			friend EventPump;
		protected:
			TimerEvent * m_next;
		};
	public:
		void quit(int exitCode) noexcept;
		void waitAll() noexcept;
		void clearTasks() noexcept;

		// node는 내부에서 관리하게 된다.
		// return: 이미 만료 혹은 취소된 이벤트의 경우 false
		bool cancel(TimerEvent* node) noexcept;

		// newnode는 내부에서 관리하게된다.
		void attach(TimerEvent* newnode) noexcept;

		template <typename LAMBDA>
		void post(LAMBDA &&lambda) noexcept
		{
			return attach(TimerEvent::create(timepoint::now(), forward<LAMBDA>(lambda)));
		}

		template <typename LAMBDA>
		void post(timepoint at, LAMBDA &&lambda) noexcept
		{
			return attach(TimerEvent::create(at, forward<LAMBDA>(lambda)));
		}

		template <typename LAMBDA>
		TimerEvent* makePost(timepoint at, LAMBDA &&lambda) noexcept
		{
			TimerEvent * node = TimerEvent::create(at, forward<LAMBDA>(lambda));
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
		TimerEvent* makePost(duration wait, LAMBDA &&lambda) noexcept
		{
			return makePost(TimerEvent::create(timepoint::now() + wait, forward<LAMBDA>(lambda)));
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

		static EventPump * getInstance() noexcept;

	private:
		EventPump() noexcept;
		~EventPump() noexcept;

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
		PromisePump m_prompump;
	};

	namespace _pri_
	{
		template <size_t argn>
		struct CallWithParamIfHas
		{
			template <typename LAMBDA>
			static void call(LAMBDA && lambda, TimerEvent * param) throws(...)
			{
				lambda(param);
			}
		};
		template <>
		struct CallWithParamIfHas<0>
		{
			template <typename LAMBDA>
			static void call(LAMBDA && lambda, TimerEvent * param) throws(...)
			{
				lambda();
			}
		};
	}

	class TimerEvent :public Interface<EventPump::NodeHead>
	{
		friend EventPump;
		friend Referencable<TimerEvent>;
	public:
		TimerEvent() noexcept;
		TimerEvent(timepoint at) noexcept;
		virtual ~TimerEvent() noexcept;
		bool isPosted() noexcept;
		void setTime(timepoint time) noexcept;
		void addTime(duration time) noexcept;
		timepoint getTime() const noexcept;
		void AddRef() noexcept;
		size_t Release() noexcept;
		size_t getRefCount() noexcept;

		template <typename LAMBDA>
		static TimerEvent* create(timepoint at, LAMBDA&& lambda) noexcept;
	protected:
		virtual void call() = 0;

		size_t m_ref;
		TimerEvent* m_prev;
		timepoint m_at;
	};

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

	template <typename LAMBDA>
	static TimerEvent* TimerEvent::create(timepoint at, LAMBDA&& lambda) noexcept
	{
		struct LambdaWrap :public TimerEvent
		{
			decay_t<LAMBDA> m_lambda;
			LambdaWrap(LAMBDA&& lambda, timepoint at) noexcept
				: TimerEvent(at), m_lambda(forward<LAMBDA>(lambda))
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

}
