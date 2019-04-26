#pragma once

#include <KR3/wl/eventhandle.h>
#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/util/callable.h>
#include <KR3/data/linkedlist.h>
#include "pump.h"
#include "promise.h"

namespace kr
{
	class EventDispatcher;

	class DispatchedEvent:protected Referencable<DispatchedEvent>
	{
		friend EventDispatcher;

	public:
		DispatchedEvent(EventHandle * event) noexcept;
		virtual ~DispatchedEvent() noexcept;
		EventHandle * handle() noexcept;
		void remove() noexcept;
		EventHandle * detach() noexcept;
		virtual void call() noexcept = 0;

	protected:
		EventPump* m_pump;

	private:
		EventDispatcher * m_dispatcher;
		EventHandle * m_event;
	};

	template <typename LAMBDA>
	class DispatchedEventLambda:public DispatchedEvent
	{
	private:
		LAMBDA m_lambda;
		
	public:
		DispatchedEventLambda(EventHandle * event, LAMBDA lambda) noexcept
			:DispatchedEvent(event), m_lambda(move(lambda))
		{	
		}

		void call() noexcept override
		{
			m_lambda(this);
		}
	};

	template <typename LAMBDA>
	class DispatchedEventLambdaCurrentThread :public DispatchedEvent
	{
	private:
		LAMBDA m_lambda;
		EventPump* m_pump;

	public:
		DispatchedEventLambdaCurrentThread(EventHandle* event, LAMBDA lambda) noexcept
			:DispatchedEvent(event), m_lambda(move(lambda)), m_pump(EventPump::getInstance())
		{
		}

		void call() noexcept override
		{
			AddRef();
			m_pump->postL([this](TimerEvent*) {
				if (handle() != nullptr)
				{
					m_lambda(this);
				}
				Release();
			});
		}
	};

	class EventDispatcher : private Threadable<EventDispatcher>, public Node<EventDispatcher, true>
	{
		friend DispatchedEvent;

	public:
		template <typename LAMBDA>
		static DispatchedEvent * regist(EventHandle * event, LAMBDA lambda) noexcept
		{
			return _new DispatchedEventLambdaCurrentThread<LAMBDA>(event, move(lambda));
		}
		template <typename LAMBDA>
		static DispatchedEvent* registThreaded(EventHandle* event, LAMBDA lambda) noexcept
		{
			return _new DispatchedEventLambda<LAMBDA>(event, move(lambda));
		}
		static Promise<void> * promise(EventHandle * event) noexcept;
		static Promise<void> * promiseAndRemove(EventHandle * event) noexcept;
		int thread() noexcept;
		~EventDispatcher() noexcept;

	private:
		EventDispatcher() noexcept;

		void _remove(DispatchedEvent * event) noexcept;
		bool _add(DispatchedEvent * event) noexcept;

		BArray<EventHandle*, EventHandle::MAXIMUM_WAIT> m_events;
		CriticalSection m_insert;
		DispatchedEvent * m_dispatched[EventHandle::MAXIMUM_WAIT-1];
		atomic<int> m_eventCounter;
		uint m_references[EventHandle::MAXIMUM_WAIT - 1];
	};
}
