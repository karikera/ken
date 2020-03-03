#include "stdafx.h"
#ifdef WIN32
#include "devent.h"

#include <KR3/win/windows.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/mt/thread.h>
#include <KR3/msg/promise.h>

using namespace kr;

namespace
{
	CriticalSection s_csDispatcher;
	LinkedList<DispatchedEventKrImpl::Manager> s_dispatchers;
}

class DispatchedEventKrImpl::Manager : private Threadable<Manager>, public Node<Manager, true>
{
	friend Threadable<Manager>;
	friend DispatchedEventKrImpl;

public:
	int thread() noexcept;
	~Manager() noexcept;

private:
	Manager() noexcept;

	void _remove(DispatchedEventKrImpl* event) noexcept;
	bool _add(DispatchedEventKrImpl* event) noexcept;

	BArray<EventHandle*, EventHandleMaxWait> m_events;
	CriticalSection m_insert;
	DispatchedEventKrImpl* m_dispatched[EventHandleMaxWait - 1];
	atomic<int> m_eventCounter;
};

DispatchedEventCommon::DispatchedEventCommon(EventHandle* event) noexcept
	:m_pump(EventPump::getInstance()), m_event(event)
{
}
DispatchedEventCommon::~DispatchedEventCommon() noexcept
{
}
EventHandle* DispatchedEventCommon::handle() const noexcept
{
	return m_event;
}

DispatchedEventWinImpl::DispatchedEventWinImpl(EventHandle* event) noexcept
	:DispatchedEventCommon(event)
{
	AddRef();
}
DispatchedEventWinImpl::~DispatchedEventWinImpl() noexcept
{
}
void DispatchedEventWinImpl::cancel() noexcept
{
	UnregisterWait(m_waiter);
	m_waiter = nullptr;
	Release();
}
bool DispatchedEventWinImpl::canceled() noexcept
{
	return m_waiter == nullptr;
}
void DispatchedEventWinImpl::_regist() noexcept
{
	BOOL RegisterWaitForSingleObject_res = RegisterWaitForSingleObject(&m_waiter, m_event, [](void* context, BOOLEAN signal) {
		static_cast<DispatchedEventWinImpl*>(context)->call();
		}, this, INFINITE, 0);
	_assert(RegisterWaitForSingleObject_res);
}

DispatchedEventKrImpl::DispatchedEventKrImpl(EventHandle * event) noexcept
	: DispatchedEventCommon(event)
{
	AddRef();
}
DispatchedEventKrImpl::~DispatchedEventKrImpl() noexcept
{
}
void DispatchedEventKrImpl::cancel() noexcept
{
	m_dispatcher->_remove(this);
	Release();
	m_dispatcher = nullptr;
}
bool DispatchedEventKrImpl::canceled() noexcept
{
	return m_dispatcher == nullptr;
}
void DispatchedEventKrImpl::_regist() noexcept
{
	DispatchedEventKrImpl::Manager* dispatcher;

	s_csDispatcher.enter();
	if (s_dispatchers.empty())
	{
		dispatcher = _new DispatchedEventKrImpl::Manager;
		dispatcher->m_dispatched[0] = this;
		dispatcher->m_events.push(m_event);
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

DispatchedEventKrImpl::Manager::Manager() noexcept
	:m_eventCounter(0)
{
	m_events.push(EventHandle::create(false, false));
}
DispatchedEventKrImpl::Manager::~Manager() noexcept
{
	delete m_events.front();
	join();
}
void DispatchedEventKrImpl::Manager::_remove(DispatchedEventKrImpl* event) noexcept
{
	m_eventCounter++;
	m_events.front()->set();
	m_insert.enter();
	m_eventCounter--;

	size_t count = m_events.size();
	if (count == EventHandleMaxWait)
	{
		s_csDispatcher.enter();
		s_dispatchers.attach(this);
		s_csDispatcher.leave();
	}

	size_t idx = memptr::pos(m_dispatched, event, count - 1);
	if (idx != (size_t)-1)
	{
		DispatchedEventKrImpl*& dptr = m_dispatched[idx];
		DispatchedEventKrImpl* old = dptr;
		dptr = m_dispatched[count - 2];
		m_events.pick(idx + 1);
	}

	m_insert.leave();
}
bool DispatchedEventKrImpl::Manager::_add(DispatchedEventKrImpl* event) noexcept
{
	m_eventCounter++;
	m_events.front()->set();
	m_insert.enter();
	m_eventCounter--;

	size_t idx = m_events.size();
	m_dispatched[idx - 1] = event;
	m_events.push(event->m_event);
	bool isFull = m_events.remaining() == 0;

	m_insert.leave();

	return isFull;
}
int DispatchedEventKrImpl::Manager::thread() noexcept
{
	m_insert.enter();
	for (;;)
	{
		DWORD res = WaitForMultipleObjects(intact<DWORD>(m_events.size()), (HANDLE*)m_events.data(), false, INFINITE);
		_assert(res != WAIT_FAILED);
		if (res >= WAIT_ABANDONED_0)
		{
			res -= WAIT_ABANDONED_0;
			if (res == 0) return 0;
			size_t cnt = m_events.size();
			if (cnt == EventHandleMaxWait)
			{
				s_csDispatcher.enter();
				s_dispatchers.attach(this);
				s_csDispatcher.leave();
			}

			DispatchedEventKrImpl*& dptr = m_dispatched[res - 1];
			DispatchedEventKrImpl* old = dptr;
			dptr = m_dispatched[cnt - 1];
			m_events.pick(res);

			old->m_pump->post([old] { old->Release(); });
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
			DispatchedEventKrImpl* event = m_dispatched[res - 1];
			event->call();
		}
	}
	m_insert.leave();
}

struct Object
{
public:
	void regist(EventHandle* event) noexcept;
	void cancel() noexcept;
	virtual void call() noexcept;

private:
	void* m_handle;
};

#endif