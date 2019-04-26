#include "stdafx.h"
#ifdef WIN32
#include "eventdispatcher.h"
#include <KR3/wl/windows.h>

using namespace kr;

namespace
{
	CriticalSection s_csDispatcher;
	LinkedList<EventDispatcher> s_dispatchers;

	class PromisedEvent:public Promise<void>, public DispatchedEvent
	{
	public:
		PromisedEvent(EventHandle * handle) noexcept
			:DispatchedEvent(handle)
		{
			AddRef();
		}
		~PromisedEvent() noexcept override
		{
		}
		void call() noexcept override
		{
			detach();
			_resolve();
		}
	};

	class PromisedEventAndRemove :public PromisedEvent
	{
	public:
		using PromisedEvent::PromisedEvent;

		void call() noexcept override
		{
			remove();
			_resolve();
		}
	};
}

DispatchedEvent::DispatchedEvent(EventHandle * event) noexcept
	: m_event(event), m_pump(EventPump::getInstance())
{
	AddRef();

	EventDispatcher * dispatcher;

	s_csDispatcher.enter();
	if (s_dispatchers.empty())
	{
		dispatcher = _new EventDispatcher;
		dispatcher->m_dispatched[0] = this;
		dispatcher->m_events.push(event);
		dispatcher->start();
		s_dispatchers.attach(dispatcher);
	}
	else
	{
		dispatcher = s_dispatchers.back();
		if (dispatcher->_add(this))
		{
			s_dispatchers.detachLast();
		}
	}
	s_csDispatcher.leave();

	m_dispatcher = dispatcher;
}
DispatchedEvent::~DispatchedEvent() noexcept
{
	delete m_event;
}
EventHandle * DispatchedEvent::handle() noexcept
{
	return m_event;
}
void DispatchedEvent::remove() noexcept
{
	m_dispatcher->_remove(this);
	
	delete m_event;
	m_event = nullptr;

	Release();
}
EventHandle * DispatchedEvent::detach() noexcept
{
	m_dispatcher->_remove(this);

	EventHandle * ret = m_event;
	m_event = nullptr;
	
	Release();

	return ret;
}

EventDispatcher::EventDispatcher() noexcept
	:m_eventCounter(0)
{
	m_events.push(EventHandle::create(false, false));
}
EventDispatcher::~EventDispatcher() noexcept
{
	delete m_events.front();
}
Promise<void> * EventDispatcher::promise(EventHandle * event) noexcept
{
	return _new PromisedEvent(event);
}
Promise<void> * EventDispatcher::promiseAndRemove(EventHandle * event) noexcept
{
	return new PromisedEventAndRemove(event);
}
void EventDispatcher::_remove(DispatchedEvent * event) noexcept
{
	m_eventCounter++;
	m_events.front()->set();
	m_insert.enter();
	m_eventCounter--;

	size_t count = m_events.size();
	if (count == EventHandle::MAXIMUM_WAIT)
	{
		s_csDispatcher.enter();
		s_dispatchers.attach(this);
		s_csDispatcher.leave();
	}

	size_t idx = memptr::pos(m_dispatched, event, count - 1);
	if (idx != (size_t)-1)
	{
		DispatchedEvent *& dptr = m_dispatched[idx];
		DispatchedEvent * old = dptr;
		dptr = m_dispatched[count - 1];
		m_events.pick(idx);
	}

	m_insert.leave();
}
bool EventDispatcher::_add(DispatchedEvent * event) noexcept
{
	event->AddRef();

	m_eventCounter++;
	m_events.front()->set();
	m_insert.enter();
	m_eventCounter--;

	size_t idx = m_events.size();
	m_dispatched[idx - 1] = event;
	m_events.push(event->m_event);
	m_references[0] = 0;
	bool isEnd = m_events.left() == 0;

	m_insert.leave();

	return isEnd;
}
int EventDispatcher::thread() noexcept
{
	m_insert.enter();
	for (;;)
	{
		DWORD res = WaitForMultipleObjects(intact<DWORD>(m_events.size()), (HANDLE*)m_events.data(), false, INFINITE);
		_assert(res != WAIT_FAILED);
		if (res > WAIT_ABANDONED_0)
		{
			res -= WAIT_ABANDONED_0;
			size_t cnt = m_events.size();
			if (cnt == EventHandle::MAXIMUM_WAIT)
			{
				s_csDispatcher.enter();
				s_dispatchers.attach(this);
				s_csDispatcher.leave();
			}

			DispatchedEvent*& dptr = m_dispatched[res - 1];
			DispatchedEvent* old = dptr;
			dptr = m_dispatched[cnt - 1];
			m_events.pick(res);

			old->m_pump->postL([old] { old->Release(); });
		}
		else if (res == 0)
		{
			while (m_eventCounter != 0)
			{
				m_insert.leave();				
				m_insert.enter();
			}
		}
		else
		{
			DispatchedEvent * event = m_dispatched[res-1];
			event->call();
		}
	}
	m_insert.leave();
}

#endif