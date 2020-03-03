#include "stdafx.h"

#ifdef WIN32

#include "threadhandle.h"
#include "windows.h"

#pragma warning(disable:4800)

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
void kr::ThreadHandle::resume() noexcept
{
	ResumeThread(this);
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
void kr::ThreadHandle::attach(Task * task) noexcept
{
	QueueUserAPC([](ULONG_PTR data) {
		auto * task = (Task*)data;
		task->call();
		}, this, (ULONG_PTR)task);
}
kr::ThreadHandle * kr::ThreadHandle::getCurrent() noexcept
{
	return (kr::ThreadHandle*)GetCurrentThread();
}

#endif