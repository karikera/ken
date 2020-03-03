#include "stdafx.h"

#ifdef WIN32

#include "msgthread.h"
#pragma warning(disable:4800)

kr::MessageThreadId::MessageThreadId(const ThreadId &threadid) noexcept
{
	m_id = threadid.m_id;
}
bool kr::MessageThreadId::postMessage(uint msg, WPARAM wParam, LPARAM lParam) noexcept
{
	return PostThreadMessageW(m_id, msg, wParam, lParam);
}
bool kr::MessageThreadId::postMessageA(uint msg, WPARAM wParam, LPARAM lParam) noexcept
{
	return PostThreadMessageA(m_id, msg, wParam, lParam);
}
bool kr::MessageThreadId::quit(int exitCode) noexcept
{
	return postMessage(WM_QUIT, exitCode, 0);
}

#endif