#include "stdafx.h"
#include "criticalsection.h"


#ifdef WIN32
#include <KR3/win/windows.h>
#include <KR3/win/eventhandle.h>
#else
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#endif


kr::CsLock::CsLock(CriticalSection & cs) noexcept
{
	m_cs = &cs;
	m_cs->enter();
}
kr::CsLock::~CsLock() noexcept
{
	m_cs->leave();
}

kr::CsLocks<2>::CsLocks(CriticalSection& cs1, CriticalSection& cs2) noexcept
{
	if (&cs1 < &cs2)
	{
		m_cs1 = &cs1;
		m_cs2 = &cs2;
	}
	else
	{
		m_cs1 = &cs2;
		m_cs2 = &cs1;
	}
	m_cs1->enter();
	m_cs2->enter();
}
kr::CsLocks<2>::~CsLocks() noexcept
{
	m_cs1->leave();
	m_cs2->leave();
}

kr::CriticalSection::CriticalSection() noexcept
{
#ifdef WIN32
	static_assert(sizeof(CRITICAL_SECTION_DATA) == sizeof(CRITICAL_SECTION), "Critical section data size unmatch");
	InitializeCriticalSection((CRITICAL_SECTION*)&m_cs);
#else
	// Use the mutex attribute to create the mutex
	pthread_mutex_init(&m_mutex, nullptr);
#endif
}
kr::CriticalSection::~CriticalSection() noexcept
{
#ifdef WIN32
	DeleteCriticalSection((CRITICAL_SECTION*)&m_cs);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}

bool kr::CriticalSection::tryEnter() noexcept
{
#ifdef WIN32
	return (bool)TryEnterCriticalSection((CRITICAL_SECTION*)&m_cs);
#else
	return pthread_mutex_trylock(&m_mutex) == 0;
#endif
}
void kr::CriticalSection::enter() noexcept
{
#ifdef WIN32
	EnterCriticalSection((CRITICAL_SECTION*)&m_cs);
#else
	pthread_mutex_lock(&m_mutex);
#endif
}
void kr::CriticalSection::leave() noexcept
{
#ifdef WIN32
	LeaveCriticalSection((CRITICAL_SECTION*)&m_cs);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}

kr::Cond::Cond() noexcept
{
#ifdef WIN32
	m_handle = EventHandle::create(false, false);
#else
	pthread_cond_init(&m_cond, nullptr);
	pthread_mutex_init(&m_mutex, nullptr);
#endif
}
kr::Cond::~Cond() noexcept
{
#ifdef WIN32
	delete m_handle;
#else
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
#endif
}

void kr::Cond::set() noexcept
{
#ifdef WIN32
	m_handle->set();
#else
	pthread_mutex_lock(&m_mutex);
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
#endif
}
void kr::Cond::reset() noexcept
{
#ifdef WIN32
	m_handle->reset();
#else
	pthread_mutex_lock(&m_mutex);
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	pthread_cond_timedwait(&m_cond, &m_mutex, &now);
	pthread_mutex_unlock(&m_mutex);
#endif
}
void kr::Cond::wait() noexcept
{
#ifdef WIN32
	m_handle->wait();
#else
	pthread_mutex_lock(&m_mutex);
	pthread_cond_wait(&m_cond, &m_mutex);
	pthread_mutex_unlock(&m_mutex);
#endif
}
bool kr::Cond::wait(duration time) noexcept
{
#ifdef WIN32
	return m_handle->wait(time);
#else
	pthread_mutex_lock(&m_mutex);
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	uint sec = duration::second.value();
	uint waitv = time.value();

	now.tv_sec += waitv / sec;
	now.tv_nsec += (waitv % sec) * (1000000000 / sec);
	if (now.tv_nsec < 0)
	{
		now.tv_nsec += 1000000000;
		now.tv_sec--;
	}
	else if (now.tv_nsec >= 1000000000)
	{
		now.tv_nsec -= 1000000000;
		now.tv_sec++;
	}

	int err = pthread_cond_timedwait(&m_cond, &m_mutex, &now);
	pthread_mutex_unlock(&m_mutex);
	if (err != ETIMEDOUT) return true;
	_assert(err == 0);
	return false;
#endif
}

kr::RWLock::RWLock() noexcept
	:m_reading(0)
{
}
void kr::RWLock::enterRead() noexcept
{
	int v;
	for (;;)
	{
		v = m_reading;
		if (v == -1)
		{
			m_cond.wait();
			continue;
		}
		if (m_reading.compare_exchange_weak(v, v + 1, std::memory_order_acquire)) break;
	}
}
void kr::RWLock::leaveRead() noexcept
{
	if (--m_reading == 0)
	{
		m_cond.set();
	}
}
void kr::RWLock::changeToWrite() noexcept
{
	int v;
	for (;;)
	{
		v = m_reading;
		if (v != 1)
		{
			m_cond.wait();
			continue;
		}
		if (m_reading.compare_exchange_weak(v, -1)) break;
	}
}
void kr::RWLock::enterWrite() noexcept
{
	int v;
	for (;;)
	{
		v = m_reading;
		if (v != 0)
		{
			m_cond.wait();
			continue;
		}
		if (m_reading.compare_exchange_weak(v, -1)) break;
	}
}
void kr::RWLock::leaveWrite() noexcept
{
	m_reading.store(0, std::memory_order_release);
	m_cond.set();
}
void kr::RWLock::leaveAnyway() noexcept
{
	int expected = -1;
	if (m_reading.compare_exchange_strong(expected, 0))
	{
		m_cond.set();
	}
	else
	{
		if (--m_reading == 0)
		{
			m_cond.set();
		}
	}
}

kr::Semaphore::Semaphore(int init) noexcept
{
#ifdef WIN32
	m_semaphore = CreateSemaphoreW(
		nullptr,           // default security attributes
		init,  // initial count
		INT_MAX,  // maximum count
		nullptr);          // unnamed semaphore
#else
	sem_init(&m_semaphore, 0, init);
#endif
}
void kr::Semaphore::release(int count) noexcept
{
#ifdef WIN32
	ReleaseSemaphore(m_semaphore, count, nullptr);
#else
	sem_destroy(&m_semaphore);
#endif
}
void kr::Semaphore::wait() noexcept
{
#ifdef WIN32
	WaitForSingleObject(m_semaphore, INFINITE);
#else
	sem_wait(&m_semaphore);
#endif
}