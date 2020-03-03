#include "stdafx.h"
#include "threadid.h"

#include <KR3/win/windows.h>

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

bool kr::ThreadId::operator == (const ThreadId& id) const noexcept
{
	return m_id == id.m_id;
}
bool kr::ThreadId::operator != (const ThreadId& id) const noexcept
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
