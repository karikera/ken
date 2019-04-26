#include "stdafx.h"

#ifdef WIN32

#include "threadhandle.h"
#include "windows.h"

#pragma warning(disable:4800)

kr::ThreadId::ThreadId(nullptr_t) noexcept
	:m_id(0)
{
}
kr::ThreadId::ThreadId(dword id) noexcept
	: m_id(id)
{
}
bool kr::ThreadId::quit(int exitCode) noexcept
{
	return PostThreadMessage(m_id, WM_QUIT, exitCode, 0);
}
bool kr::ThreadId::postMessage(int msg, WPARAM wParam, LPARAM lParam) noexcept
{
	return PostThreadMessage(m_id, msg, wParam, lParam);
}
void kr::ThreadId::setName(pcstr name) noexcept
{
#ifndef NDEBUG
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack(pop)
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = m_id;
	info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)
#endif
}
kr::dword kr::ThreadId::value() noexcept
{
	return m_id;
}
kr::ThreadId kr::ThreadId::getCurrent() noexcept
{
	return GetCurrentThreadId();
}

bool kr::ThreadId::operator == (const ThreadId & id) const noexcept
{
	return m_id == id.m_id;
}
bool kr::ThreadId::operator != (const ThreadId & id) const noexcept
{
	return m_id != id.m_id;
}
bool kr::ThreadId::operator == (nullptr_t) const noexcept
{
	return m_id == 0;
}
bool kr::ThreadId::operator != (nullptr_t) const noexcept
{
	return m_id != 0;
}

template <> kr::ThreadHandle* kr::ThreadHandle::createRaw<void>(ThreadProc<void> fn, void * param, ThreadId * id) noexcept
{
	static_assert(sizeof(ThreadId) == sizeof(DWORD), "ThreadId size unmatch");
	ThreadHandle* handle = (ThreadHandle*)CreateThread(nullptr, 0, (DWORD(CALLBACK*)(LPVOID))fn, param, 0, (DWORD*)id);
	_assert(handle != ihv);
	_assert(handle != nullptr);
	return handle;
}
kr::ThreadId kr::ThreadHandle::getId() noexcept
{
	return (ThreadId)GetThreadId(this);
}
void kr::ThreadHandle::suspend() noexcept
{
	SuspendThread(this);
}
void kr::ThreadHandle::terminate() noexcept
{
	TerminateThread(this, -1);
}
void kr::ThreadHandle::detach() noexcept
{
	delete (EventHandle*)this;
}
kr::dword kr::ThreadHandle::join() noexcept
{
	wait();
	DWORD code;
	if (!GetExitCodeThread(this, &code))
	{
		code = -1;
	}
	delete (EventHandle*)this;
	return code;
}
void kr::ThreadHandle::setPriority(ThreadPriority priority) noexcept
{
	switch (priority)
	{
	case kr::ThreadPriority::Idle: SetThreadPriority(this, THREAD_PRIORITY_IDLE); break;
	case kr::ThreadPriority::Lowest: SetThreadPriority(this, THREAD_PRIORITY_LOWEST); break;
	case kr::ThreadPriority::Low: SetThreadPriority(this, THREAD_PRIORITY_BELOW_NORMAL); break;
	case kr::ThreadPriority::Normal: SetThreadPriority(this, THREAD_PRIORITY_NORMAL); break;
	case kr::ThreadPriority::High: SetThreadPriority(this, THREAD_PRIORITY_ABOVE_NORMAL); break;
	case kr::ThreadPriority::Highest: SetThreadPriority(this, THREAD_PRIORITY_HIGHEST); break;
	default: _assert(!"invalid parameter");
	}
}
kr::ThreadHandle * kr::ThreadHandle::getCurrent() noexcept
{
	return (kr::ThreadHandle*)GetCurrentThread();
}

#endif