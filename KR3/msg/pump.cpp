#include "stdafx.h"

#ifdef WIN32

#include <KR3/win/eventhandle.h>
#include <KR3/win/threadhandle.h>

#include "pump.h"
#include "msgloop.h"

#pragma warning(disable:4800)

using namespace kr;

constexpr int PROCESS_MESSAGE_COUNT_PER_TRY = 10;

EventPump::EventPump() noexcept
	:m_reference(0)
{
	m_start.m_next = nullptr;

	m_msgevent = EventHandle::create(false, false);
	m_threadId = ThreadId::getCurrent();
}
EventPump::~EventPump() noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	waitAll();

	_assert(m_start.m_next == nullptr); // all tasks must complete
	delete m_msgevent;
}

void EventPump::quit(int exitCode) noexcept
{
	m_threadId.quit(exitCode);
}
void EventPump::waitAll() noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	for (;;)
	{
		m_prompump.process();
		processOnce();
		if (m_reference != 0)
		{
			DWORD sleep = _processTimer(INFINITE);
			dword index = MsgWaitForMultipleObjectsEx(1, (HANDLE*)&m_msgevent, sleep, QS_ALLINPUT, MWMO_ALERTABLE);
			_assert(index != WAIT_FAILED);
			if (index == WAIT_OBJECT_0 + 1) _processMessage();
		}
		else
		{
			if (m_prompump.empty()) break;
		}
	}
	
	clearTasks();
}
void EventPump::clearTasks() noexcept
{
	TimerEvent* node = m_start.m_next;
	if (node != nullptr)
	{
		m_start.m_next = nullptr;

		do
		{
			TimerEvent* next = node->m_next;
			node->Release();
			node = next;
		} while (node != nullptr);
		
	}

	m_prompump.clear();
}
bool EventPump::cancel(TimerEvent* node) noexcept
{
	m_timercs.enter();
	TimerEvent* p = node->m_prev;
	if (p == nullptr)
	{
		m_timercs.leave();
		return false;
	}

	TimerEvent* n = node->m_next;
	p->m_next = n;
	if (n != nullptr) n->m_prev = p;
	m_timercs.leave();
	node->m_prev = nullptr;
	return true;
}
void EventPump::attach(TimerEvent* newnode) noexcept
{
	// reline_new(newnode); // if timer is secondary parent, it make error

	newnode->AddRef();
	assert(m_threadId != nullptr);

	timepoint newtime = newnode->m_at;

	{
		CsLock locked = m_timercs;
		TimerEvent* pnode = static_cast<TimerEvent*>(&m_start);
		TimerEvent* node;
		for (;;)
		{
			node = pnode->m_next;
			if (node == nullptr) break;
			if (newtime < node->m_at)
			{
				node->m_prev = newnode;
				break;
			}
			pnode = node;
		}
		pnode->m_next = newnode;
		newnode->m_prev = pnode;
		newnode->m_next = node;
	}
	m_msgevent->set();
}

void EventPump::processOnce() throws(QuitException)
{
	_tryProcess(&m_msgevent, 1);
}
dword EventPump::processOnce(View<EventHandle *> events) throws(QuitException)
{
	TmpArray<EventHandle*> newevents = _makeEventArray(events);
	dword count = intact<DWORD>(newevents.size());
	return _tryProcess(newevents.begin(), count);
}
void EventPump::processOnce(View<EventProcedure> proc) throws(QuitException)
{
	static const duration_detail ms = duration_detail::second / 1000;
	timepoint_detail start = timepoint_detail::now();

	_assert(m_threadId == ThreadId::getCurrent());
	m_prompump.process();
	TmpArray<EventHandle*> events = _makeEventArray(proc);

	do {
		_processTimer(0);
		DWORD cnt = intact<DWORD>(events.size());

		dword index = MsgWaitForMultipleObjectsEx(cnt, (HANDLE*)events.data(), 0, QS_ALLINPUT, MWMO_ALERTABLE);
		if (index == WAIT_TIMEOUT) return;
		_assert(index != WAIT_FAILED);
		if (index == cnt)
		{
			_processMessage();
		}
		else if (index < cnt - 1)
		{
			const EventProcedure& p = proc[index];
			p.callback(p.param);
			m_prompump.process();
		}
	} while (timepoint_detail::now() - start < ms);
}
void EventPump::processOnceWithoutMessage() throws(QuitException)
{
	m_prompump.process();
	_processTimer(0);
}
void EventPump::processOnceWithoutMessage(View<EventProcedure> proc) throws(QuitException)
{
	_assert(m_threadId == ThreadId::getCurrent());
	m_prompump.process();
	_processTimer(0);
	TmpArray<EventHandle*> events = _makeEventArray(proc);

	for (int i = 0; i < 3; i++)
	{
		DWORD sleep = _processTimer(0);
		DWORD cnt = intact<DWORD>(events.size());
		dword index = WaitForMultipleObjects(cnt, (HANDLE*)events.data(), false, 0);
		_assert(index != WAIT_FAILED);
		if (index == WAIT_TIMEOUT) return;
		if (index < cnt - 1)
		{
			const EventProcedure& p = proc[index];
			p.callback(p.param);
			m_prompump.process();
		}
	}
}
Promise<void>* EventPump::promise(duration time) noexcept
{
	return promiseTo(timepoint::now() + time);
}
Promise<void>* EventPump::promiseTo(timepoint at) noexcept
{
	class Prom :public Promise<void>, public TimerEvent
	{
	public:
		Prom(timepoint at) noexcept
			:TimerEvent(at)
		{
			AddRef();
		}
		~Prom() noexcept override
		{
		}

		void call() noexcept override
		{
			_resolve();
		}
	};
	Prom* prom = _new Prom(at);
	attach(prom);
	return prom;
}
void EventPump::sleep(duration dura) throws(QuitException)
{
	wait(zerolen, dura);
}
void EventPump::sleepTo(timepoint time) throws(QuitException)
{
	waitTo(View<EventHandle*>(nullptr, nullptr), time);
}
void EventPump::wait(EventHandle* event) throws(QuitException)
{
	wait(View<EventHandle*>(&event, 1));
}
void EventPump::wait(EventHandle* event, duration time) throws(QuitException)
{
	wait(View<EventHandle*>(&event, 1), time);
}
void EventPump::waitTo(EventHandle* event, timepoint time) throws(QuitException)
{
	waitTo(View<EventHandle*>(&event, 1), time);
}
void EventPump::AddRef() noexcept
{
	m_reference++;
}
void EventPump::Release() noexcept
{
	size_t ref = --m_reference;
	_assert(ref >= 0);
	if (ref == 0)
	{
		m_msgevent->set();
	}
}
dword EventPump::wait(View<EventHandle*> events) throws(QuitException)
{
	m_prompump.process();
	TmpArray<EventHandle*> newevents = _makeEventArray(events);
	dword count = intact<DWORD>(newevents.size());
	for (;;)
	{
		DWORD sleep = _processTimer(INFINITE);
		dword index = MsgWaitForMultipleObjectsEx(count, (HANDLE*)newevents.data(), sleep, QS_ALLINPUT, MWMO_ALERTABLE);
		_assert(index != WAIT_FAILED);
		if (index == WAIT_IO_COMPLETION) continue;
		if (index != WAIT_OBJECT_0 + count - 1)
		{
			if (index == WAIT_OBJECT_0 + count)
			{
				_processMessage();
			}
			else
			{
				return index - WAIT_OBJECT_0;
			}
		}
	}
}
dword EventPump::wait(View<EventHandle*> events, duration time) throws(QuitException)
{
	return waitTo(events, timepoint::now() + time);
}
dword EventPump::waitTo(View<EventHandle*> events, timepoint timeto) throws(QuitException)
{
	_assert(events.size() < MAXIMUM_WAIT);

	m_prompump.process();
	TmpArray<EventHandle*> newevents = _makeEventArray(events);
	dword count = intact<DWORD>(newevents.size());

	duration time = timeto - timepoint::now();
	if (time <= (duration)0)
	{
		return _tryProcess(newevents.begin(), count);
	}

	for (;;)
	{
		DWORD sleep = _processTimer(time.value());
		dword index = MsgWaitForMultipleObjectsEx(count, (HANDLE*)newevents.data(), sleep, QS_ALLINPUT, MWMO_ALERTABLE);
		_assert(index != WAIT_FAILED);
		if (index != WAIT_IO_COMPLETION && index != WAIT_OBJECT_0 + count - 1)
		{
			if (index == WAIT_OBJECT_0 + count)
			{
				_processMessage();
			}
			else
			{
				return index - WAIT_OBJECT_0;
			}
		}
		time = timeto - timepoint::now();
		if (time <= (duration)0)
			return WAIT_TIMEOUT;
	}
}
int EventPump::messageLoop() noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	m_prompump.process();
	try
	{
		for (;;)
		{
			for (;;)
			{
				DWORD sleep = _processTimer(INFINITE);
				dword index = MsgWaitForMultipleObjectsEx(1, (HANDLE*)&m_msgevent, sleep, QS_ALLINPUT, MWMO_ALERTABLE);
				_assert(index != WAIT_FAILED);
				if (index == WAIT_OBJECT_0 + 1) _processMessage();
			}
		}
	}
	catch (QuitException& quit)
	{
		return quit.exitCode;
	}
}
int EventPump::messageLoopWith(View<EventProcedure> proc) noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	m_prompump.process();
	try
	{
		TmpArray<EventHandle*> events = _makeEventArray(proc);

		for (;;)
		{
			DWORD sleep = _processTimer(INFINITE);
			DWORD cnt = intact<DWORD>(events.size());
			dword index = MsgWaitForMultipleObjectsEx(cnt, (HANDLE*)events.data(), sleep, QS_ALLINPUT, MWMO_ALERTABLE);
			_assert(index != WAIT_FAILED);
			if (index == WAIT_IO_COMPLETION)
			{
				continue;
			}
			else if (index == WAIT_OBJECT_0 + cnt)
			{
				_processMessage();
			}
			else if (index < WAIT_OBJECT_0 + cnt - 1)
			{
				const EventProcedure& p = proc[index];
				p.callback(p.param);
				m_prompump.process();
			}
		}
	}
	catch (QuitException& quit)
	{
		return quit.exitCode;
	}
}

EventPump* EventPump::getInstance() noexcept
{
	static thread_local EventPump pump;
	return &pump;
}

dword EventPump::_tryProcess(EventHandle* const* events, dword count) throws(QuitException)
{
	m_msgevent->reset();
	_processTimer(0);
	dword index;
	for (int i = 0; i < 4; i++)
	{
		index = MsgWaitForMultipleObjectsEx(count, (HANDLE*)events, 0, QS_ALLINPUT, MWMO_ALERTABLE);
		_assert(index != WAIT_FAILED);
		if (index == WAIT_IO_COMPLETION) continue;
		if (index != count - 1)
		{
			if (index == count)
			{
				_processMessage();
			}
			else
			{
				return index;
			}
		}
		break;
	}
	return WAIT_TIMEOUT;
}
TmpArray<EventHandle*> EventPump::_makeEventArray(View<EventHandle*> events) noexcept
{
	size_t sz = events.size();
	TmpArray<EventHandle*> newevents;
	newevents.resize(sz + 1);
	newevents.subcopy(events.begin(), sz, 0);
	newevents[sz] = m_msgevent;
	return newevents;
}
TmpArray<EventHandle*> EventPump::_makeEventArray(View<EventProcedure> proc) noexcept
{
	size_t sz = proc.size();
	TmpArray<EventHandle*> events;
	events.resize(sz + 1);
	{
		EventHandle** dest = events.begin();
		for (const EventProcedure& p : proc)
		{
			*dest++ = p.event;
		}
	}
	events[sz] = m_msgevent;
	return events;
}
void EventPump::_processMessage() throws(QuitException)
{
	MessageLoop* msgloop = MessageLoop::getInstance();
	// 윈도우 메세지
	for (int i = 0; i < PROCESS_MESSAGE_COUNT_PER_TRY; i++)
	{
		if (!msgloop->tryGet()) break;
		msgloop->dispatch();
	}
	m_prompump.process();
}
dword EventPump::_processTimer(dword maxSleep) throws(QuitException)
{
	if (m_start.m_next == nullptr) return maxSleep;

	timepoint startTime = timepoint::now();
	timepoint now = startTime;
	timepoint callTime;
	int tryCount = 0;
	for (;;)
	{
		TimerEvent* node;
		{
			CsLock locked = m_timercs;
			node = m_start.m_next;
			if (node == nullptr) break;
			callTime = node->m_at;
			if (callTime > now)
			{
				maxSleep = mint((dword)(callTime - now).value(), maxSleep);
				break;
			}
			m_start.m_next = node->m_next;
		}

		node->m_prev = nullptr;
		{
			finally { node->Release(); };
			node->call();
		}

		if (m_start.m_next == nullptr) break;

		tryCount++;
		if (tryCount >= PROCESS_MESSAGE_COUNT_PER_TRY)
		{
			now = timepoint::now();
			if (now > startTime + 10_ms)
			{
				maxSleep = 0;
				break;
			}
			tryCount = 0;
		}
	}
	m_prompump.process();
	return maxSleep;
}

TimerEvent::TimerEvent() noexcept
{
	m_ref = 0;
	m_prev = nullptr;
}
TimerEvent::TimerEvent(timepoint at) noexcept
{
	m_at = at;
	m_ref = 0;
	m_prev = nullptr;
}
TimerEvent::~TimerEvent() noexcept
{
}
bool TimerEvent::isPosted() noexcept
{
	return m_prev != nullptr;
}
void TimerEvent::setTime(timepoint time) noexcept
{
	m_at = time;
}
void TimerEvent::addTime(duration time) noexcept
{
	m_at += time;
}
timepoint TimerEvent::getTime() const noexcept
{
	return m_at;
}
void TimerEvent::AddRef() noexcept
{
	m_ref++;
}
size_t TimerEvent::Release() noexcept
{
	size_t ret = --m_ref;
	if (ret == 0)
	{
		delete this;
	}
	return ret;
}
size_t TimerEvent::getRefCount() noexcept
{
	return m_ref;
}

EventThread::EventThread() noexcept
{
	create();
}
EventThread::~EventThread() noexcept
{
	quit(0);
}

void EventThread::create() noexcept
{
	struct Info
	{
		EventThread * thread;
		EventHandle * waiter;
	};
	Info info;
	info.thread = this;
	info.waiter = EventHandle::create(false, false);
	m_thread = ThreadHandle::createRaw((ThreadHandle::ThreadProc<Info>)[](Info * info) {
		MessageLoop::getInstance()->peek();
		EventPump * pump = EventPump::getInstance();
		info->thread->m_pump = pump;
		info->waiter->set();
		return pump->messageLoop();
	}, &info);
	info.waiter->wait();
	delete info.waiter;
}
void EventThread::quit(int exitCode) noexcept
{
	if (!m_pump) return;
	m_pump->post([](void*) { throw QuitException(0); });
	m_pump->quit(exitCode);
	m_thread->wait();
	m_pump = nullptr;
	m_thread = nullptr;
}
EventPump * EventThread::operator ->() noexcept
{
	return m_pump;
}

#else

EMPTY_SOURCE

#endif