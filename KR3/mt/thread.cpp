#include "stdafx.h"
#include "thread.h"

#ifdef WIN32
#include <KR3/win/windows.h>
#else
#include <pthread.h>
#endif

using namespace kr;

ThreadObject ThreadObject::getCurrent() noexcept
{
	ThreadObject obj;
#ifdef WIN32
	obj.m_handle = (ThreadHandle*)GetCurrentThread();
#else
	obj.m_handle = pthread_self();
#endif
	return obj;
}
void ThreadObject::createRaw(RawThreadProc fn, void * param
#ifdef WIN32
	, unsigned long * id
#endif
) noexcept
{
#ifdef WIN32
	m_handle = (ThreadHandle*)CreateThread(nullptr, 0, fn, param, 0, id);
	_assert(m_handle != INVALID_HANDLE_VALUE);
	_assert(m_handle != nullptr);
#else
	int pthread_create_res = pthread_create(&m_handle, nullptr, fn, param);
	_assert(pthread_create_res != 0);
#endif
}

Thread::Thread() noexcept
{
}
Thread::~Thread() noexcept
{
}
