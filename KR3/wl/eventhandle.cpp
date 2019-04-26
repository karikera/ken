#include "stdafx.h"

#ifdef WIN32

#include "eventhandle.h"
#include "windows.h"

using namespace kr;

static_assert(EventHandle::MAXIMUM_WAIT == MAXIMUM_WAIT_OBJECTS, "connection count unmatch");


SemaphoreHandle * SemaphoreHandle::create(long initsem, long maxsem) noexcept
{
	return (SemaphoreHandle*)CreateSemaphore(nullptr, initsem, maxsem, nullptr);
}
bool SemaphoreHandle::release(long count) noexcept
{
	return ReleaseSemaphore(this, count, nullptr) != FALSE;
}

EventHandle * EventHandle::create(bool status, bool manual) noexcept
{
	return (EventHandle*)CreateEvent(nullptr, manual, status, nullptr);
}
void EventHandle::operator delete(ptr event) noexcept
{
	//if (event == nullptr) return;
	CloseHandle(event);
}
void EventHandle::wait() noexcept
{
	dword WaitForSingleObjectResult = WaitForSingleObject(this, INFINITE);
	_assert(WaitForSingleObjectResult != WAIT_FAILED && WaitForSingleObjectResult != WAIT_ABANDONED);
}
bool EventHandle::wait(duration timer) noexcept
{
	dword WaitForSingleObjectResult = WaitForSingleObject(this, timer.value());
	_assert(WaitForSingleObjectResult != WAIT_FAILED && WaitForSingleObjectResult != WAIT_ABANDONED);
	return WaitForSingleObjectResult != WAIT_TIMEOUT;
}
bool EventHandle::waitWith(EventHandle * ev) noexcept
{
	EventList<2> list = { ev, this };
	dword idx = list.wait();
	return idx != 0;
}
int EventHandle::waitWith(EventHandle * ev, duration ms) noexcept
{
	EventList<2> list = { ev, this };
	return list.wait();
}
void EventHandle::set() noexcept
{
	SetEvent(this);
}
void EventHandle::reset() noexcept
{
	ResetEvent(this);
}

Event::Event(EventHandle * handle) noexcept
{
	m_event = handle;
}
Event::Event(bool bStatus, bool manual) noexcept
{
	m_event = EventHandle::create(bStatus, manual);
}
Event::Event(Event && move) noexcept
{
	m_event = move.m_event;
	move.m_event = nullptr;
}
Event::~Event() noexcept
{
	if (m_event != nullptr)
		delete m_event;
}
EventHandle * Event::operator ->() noexcept
{
	return m_event;
}
EventHandle *& Event::operator &() noexcept
{
	return m_event;
}
Event & Event::operator = (Event && move) noexcept
{
	this->~Event();
	m_event = move.m_event;
	move.m_event = nullptr;
	return *this;
}
Event & Event::operator = (nullptr_t) noexcept
{
	this->~Event();
	m_event = m_event = nullptr;
	return *this;
}
bool Event::operator == (nullptr_t) noexcept
{
	return m_event == nullptr;
}
bool Event::operator == (const Event & _other) noexcept
{
	return m_event == _other.m_event;
}
bool Event::operator != (nullptr_t) noexcept
{
	return m_event == nullptr;
}
bool Event::operator != (const Event & _other) noexcept
{
	return m_event == _other.m_event;
}

_pri_::EventListImpl::EventListImpl() noexcept
{
	m_size = 0;
}
dword _pri_::EventListImpl::wait() noexcept
{
	DWORD res = WaitForMultipleObjects(intact<DWORD>(m_size), (HANDLE*)_ptr(), false, INFINITE);
	ondebug
	(
		if (res == WAIT_FAILED)
		{
			DWORD err = GetLastError();
			_assert(!"WaitForMultipleObject error");
		}
	);
	_assert(WAIT_OBJECT_0 == 0);
	return res;
}
dword _pri_::EventListImpl::wait(dword time) noexcept
{
	DWORD res = WaitForMultipleObjects(intact<DWORD>(m_size), (HANDLE*)_ptr(), false, time);
	_assert(res != WAIT_FAILED);
	_assert(WAIT_OBJECT_0 == 0);
	return res;
}
EventHandle * _pri_::EventListImpl::pickOut(size_t idx) noexcept
{
	m_size--;
	if (idx == m_size) return _ptr()[idx];
	EventHandle * out = _ptr()[idx];
	_ptr()[idx] = _ptr()[m_size];
	return out;
}
void _pri_::EventListImpl::pop() noexcept
{
	_assert(m_size != 0);
	m_size--;
}
EventHandle * const * _pri_::EventListImpl::begin() const noexcept
{
	return _ptr();
}
EventHandle * const * _pri_::EventListImpl::end() const noexcept
{
	return _ptr() + size();
}
size_t _pri_::EventListImpl::size() const noexcept
{
	return m_size;
}
void _pri_::EventListImpl::clear() noexcept
{
	m_size = 0;
}
EventHandle ** _pri_::EventListImpl::_ptr() noexcept
{
	return (EventHandle**)(this + 1);
}
EventHandle * const * _pri_::EventListImpl::_ptr() const noexcept
{
	return (EventHandle**)(this+1);
}

EventWaiter::EventWaiter(bool bStatus) noexcept :Event(bStatus)
{
}
EventWaiter::~EventWaiter() noexcept
{
	(*this)->wait();
}

#endif