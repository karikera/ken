#include "stdafx.h"

#ifdef WIN32

#include "pump.h"
#include "msgloop.h"

#pragma warning(disable:4800)

using namespace kr;

constexpr int PROCESS_MESSAGE_COUNT_PER_TRY = 4;

EventPump::EventPump() noexcept
{
	m_start.m_next = nullptr;

	m_msgevent = EventHandle::create(false, false);
	m_threadId = ThreadId::getCurrent();
}
EventPump::~EventPump() noexcept
{
	delete m_msgevent;

	terminate();
}

void EventPump::quit(int exitCode) noexcept
{
	m_threadId.quit(exitCode);
}
void EventPump::clear() noexcept
{
	m_timercs.enter();
	Timer * node = m_start.m_next;
	while (node != nullptr)
	{
		Timer * next = node->m_next;
		node->Release();
		node = next;
	}
	m_start.m_next = nullptr;
	m_timercs.leave();
}
void EventPump::terminate() noexcept
{
	Timer * p = m_start.m_next;
	while (p != nullptr)
	{
		Timer * next = p->m_next;
		p->Release();
		p = next;
	}
	m_start.m_next = nullptr;

	PromiseManager::getInstance()->finish();
}
bool EventPump::cancel(Timer * node) noexcept
{
	m_timercs.enter();
	Timer * p = node->m_prev;
	if (p == nullptr)
	{
		m_timercs.leave();
		return false;
	}

	Timer * n = node->m_next;
	p->m_next = n;
	if (n != nullptr) n->m_prev = p;
	m_timercs.leave();
	node->m_prev = nullptr;
	return true;
}
void EventPump::post(Timer * newnode) noexcept
{
	// reline_new(newnode); // if timer is secondary parent, it make error

	newnode->AddRef();
	assert(m_threadId != nullptr);

	timepoint newtime = newnode->m_at;

	{
		CsLock locked = m_timercs;
		Timer * pnode = static_cast<Timer*>(&m_start);
		Timer * node;
		for (;;)
		{
			node = pnode->m_next;
			if (node == nullptr) break;
			if (node->m_at >= newtime)
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
	_assert(m_threadId == ThreadId::getCurrent());
	_fireAfterProcess();
	_processTimer(0);
	TmpArray<EventHandle*> events = _makeEventArray(proc);

	for (int i = 0; i<3; i++)
	{
		DWORD sleep = _processTimer(0);
		DWORD cnt = intact<DWORD>(events.size());

		dword index = MsgWaitForMultipleObjects(cnt, (HANDLE*)events.data(), false, 0, QS_ALLINPUT);
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
			_fireAfterProcess();
		}
	}
}
void EventPump::processOnceWithoutMessage() throws(QuitException)
{
	_fireAfterProcess();
	_processTimer(0);
}
void EventPump::processOnceWithoutMessage(View<EventProcedure> proc) throws(QuitException)
{
	_assert(m_threadId == ThreadId::getCurrent());
	_fireAfterProcess();
	_processTimer(0);
	TmpArray<EventHandle*> events = _makeEventArray(proc);

	for (int i=0;i<3;i++)
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
			_fireAfterProcess();
		}
	}
}
Promise<void> * EventPump::promise(duration time) noexcept
{
	return promiseTo(timepoint::now() + time);
}
Promise<void> * EventPump::promiseTo(timepoint at) noexcept
{
	class Prom :public Promise<void>, public Timer
	{
	public:
		Prom(timepoint at) noexcept
			:Timer(at)
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
	Prom * prom = _new Prom(at);
	post(prom);
	return prom;
}
void EventPump::sleep(duration dura) throws(QuitException)
{
	wait(View<EventHandle*>(nullptr, nullptr), dura);
}
void EventPump::sleepTo(timepoint time) throws(QuitException)
{
	waitTo(View<EventHandle*>(nullptr, nullptr), time);
}
void EventPump::wait(EventHandle * event) throws(QuitException)
{
	wait(View<EventHandle*>(&event, 1));
}
void EventPump::wait(EventHandle * event, duration time) throws(QuitException)
{
	wait(View<EventHandle*>(&event, 1), time);
}
void EventPump::waitTo(EventHandle * event, timepoint time) throws(QuitException)
{
	waitTo(View<EventHandle*>(&event, 1), time);
}
dword EventPump::wait(View<EventHandle *> events) throws(QuitException)
{
	_fireAfterProcess();
	TmpArray<EventHandle*> newevents = _makeEventArray(events);
	dword count = intact<DWORD>(newevents.size());
	for (;;)
	{
		DWORD sleep = _processTimer(INFINITE);
		dword index = MsgWaitForMultipleObjects(count, (HANDLE*)newevents.data(), false, sleep, QS_ALLINPUT);
		_assert(index != WAIT_FAILED);
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
	}
}
dword EventPump::wait(View<EventHandle *> events, duration time) throws(QuitException)
{
	_assert(events.size() < MAXIMUM_WAIT);

	TmpArray<EventHandle*> newevents = _makeEventArray(events);
	dword count = intact<DWORD>(newevents.size());
	if (time <= (duration)0)
	{
		return _tryProcess(newevents.begin(), count);
	}

	timepoint timeto = timepoint::now() + time;

	for (;;)
	{
		DWORD sleep = _processTimer(time.value());
		dword index = MsgWaitForMultipleObjects(count, (HANDLE*)newevents.data(), false, sleep, QS_ALLINPUT);
		_assert(index != WAIT_FAILED);
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
		time = timeto - timepoint::now();
		if (time <= (duration)0)
			return WAIT_TIMEOUT;
	}
}
dword EventPump::waitTo(View<EventHandle *> events, timepoint timeto) throws(QuitException)
{
	_assert(events.size() < MAXIMUM_WAIT);

	_fireAfterProcess();
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
		dword index = MsgWaitForMultipleObjects(count, (HANDLE*)newevents.data(), false, sleep, QS_ALLINPUT);
		_assert(index != WAIT_FAILED);
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
		time = timeto - timepoint::now();
		if (time <= (duration)0)
			return WAIT_TIMEOUT;
	}
}
int EventPump::messageLoop() noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	_fireAfterProcess();
	try
	{
		for (;;)
		{
			for (;;)
			{
				DWORD sleep = _processTimer(INFINITE);
				dword index = MsgWaitForMultipleObjects(1, (HANDLE*)&m_msgevent, false, sleep, QS_ALLINPUT);
				_assert(index != WAIT_FAILED);
				if (index == 1) _processMessage();
			}
		}
	}
	catch (QuitException& quit)
	{
		terminate();
		return quit.exitCode;
	}
}
int EventPump::messageLoopWith(View<EventProcedure> proc) noexcept
{
	_assert(m_threadId == ThreadId::getCurrent());
	_fireAfterProcess();
	try
	{
		TmpArray<EventHandle*> events = _makeEventArray(proc);

		for (;;)
		{
			DWORD sleep = _processTimer(INFINITE);
			DWORD cnt = intact<DWORD>(events.size());
			dword index = MsgWaitForMultipleObjects(cnt, (HANDLE*)events.data(), false, sleep, QS_ALLINPUT);
			_assert(index != WAIT_FAILED);
			if (index == cnt)
			{
				_processMessage();
			}
			else if (index < cnt - 1)
			{
				const EventProcedure& p = proc[index];
				p.callback(p.param);
				_fireAfterProcess();
			}
		}
	}
	catch (QuitException& quit)
	{
		terminate();
		return quit.exitCode;
	}
}

EventPump * EventPump::getInstance() noexcept
{
	static thread_local EventPump pump;
	return &pump;
}

dword EventPump::_tryProcess(EventHandle * const * events, dword count) throws(QuitException)
{
	m_msgevent->reset();
	_processTimer(0);
	dword index = MsgWaitForMultipleObjects(count, (HANDLE*)events, false, 0, QS_ALLINPUT);
	_assert(index != WAIT_FAILED);
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
	return WAIT_TIMEOUT;
}
TmpArray<EventHandle*> EventPump::_makeEventArray(View<EventHandle*> events) noexcept
{
	size_t sz = events.size();
	TmpArray<EventHandle*> newevents(sz + 1);
	newevents.subcopy(events.begin(), sz, 0);
	newevents[sz] = m_msgevent;
	return newevents;
}
TmpArray<EventHandle *> EventPump::_makeEventArray(View<EventProcedure> proc) noexcept
{
	size_t sz = proc.size();
	TmpArray<EventHandle*> events(sz + 1);
	{
		EventHandle ** dest = events.begin();
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
	MessageLoop * msgloop = MessageLoop::getInstance();

	// 윈도우 메세지
	for (int i = 0; i < PROCESS_MESSAGE_COUNT_PER_TRY; i++)
	{
		if (!msgloop->tryGet()) break;
		msgloop->dispatch();
	}

	_fireAfterProcess();
}
dword EventPump::_processTimer(dword maxSleep)
{
	if (m_start.m_next == nullptr) return maxSleep;

	timepoint startTime = timepoint::now();
	timepoint now = startTime;
	timepoint callTime;
	int tryCount = 0;
	for (;;)
	{
		Timer * node;
		{
			CsLock locked = m_timercs;
			node = m_start.m_next;
			if (node == nullptr)
			{
				_fireAfterProcess();
				return maxSleep;
			}
			callTime = node->m_at;
			if (callTime > now)
			{
				_fireAfterProcess();
				return tmin((dword)(callTime - now).value(), maxSleep);
			}
			m_start.m_next = node->m_next;
		}

		node->m_prev = nullptr;
		{
			finally{ node->Release(); };
			node->call();
		}

		if (m_start.m_next == nullptr)
		{
			_fireAfterProcess();
			return maxSleep;
		}

		tryCount++;
		if (tryCount >= PROCESS_MESSAGE_COUNT_PER_TRY)
		{
			now = timepoint::now();
			if (now > startTime + 10_ms)
			{
				_fireAfterProcess();
				return 0;
			}
			tryCount = 0;
		}
	}
}
void EventPump::_fireAfterProcess() noexcept
{
	PromiseManager::getInstance()->process();
}

EventPump::Timer::Timer() noexcept
{
	m_ref = 0;
	m_prev = nullptr;
}
EventPump::Timer::Timer(timepoint at) noexcept
{
	m_at = at;
	m_ref = 0;
	m_prev = nullptr;
}
EventPump::Timer::~Timer() noexcept
{
}
bool EventPump::Timer::isPosted() noexcept
{
	return m_prev != nullptr;
}
void EventPump::Timer::setTime(timepoint time) noexcept
{
	m_at = time;
}
void EventPump::Timer::addTime(duration time) noexcept
{
	m_at += time;
}
timepoint EventPump::Timer::getTime() const noexcept
{
	return m_at;
}
void EventPump::Timer::AddRef() noexcept
{
	m_ref++;
}
size_t EventPump::Timer::Release() noexcept
{
	size_t ret = --m_ref;
	if (ret == 0)
	{
		delete this;
	}
	return ret;
}
size_t EventPump::Timer::getRefCount() noexcept
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
	m_pump->postL([](void*) { throw QuitException(0); });
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