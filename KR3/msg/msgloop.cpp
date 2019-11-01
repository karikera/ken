#include "stdafx.h"

#ifdef WIN32

#include "msgloop.h"
#include <KR3/util/callable.h>
#include <KR3/mt/thread.h>

#pragma warning(disable:4800)

using namespace kr;

static_assert(WAIT_OBJECT_0 == 0, "WAIT_OBJECT_0 is not 0");

LPMSG MessageStruct::getMessageStruct() noexcept
{
	return (LPMSG)this;
}
ivec2 MessageStruct::getPointFromParam() const noexcept
{
	return ::getPointFromParam(lParam);
}
bool MessageStruct::isDlgMessageA(HWND wnd) noexcept
{
	return ::IsDialogMessageA(wnd, (LPMSG)this);
}
bool MessageStruct::isDlgMessage(HWND wnd) noexcept
{
	return ::IsDialogMessageW(wnd, (LPMSG)this);
}

bool MessageStruct::get() noexcept
{
	return GetMessageW((LPMSG)this, nullptr, 0, 0);
}
bool MessageStruct::peek() noexcept
{
	return PeekMessageW((LPMSG)this, nullptr, 0, 0, 0);
}
bool MessageStruct::tryGet() noexcept
{
	return PeekMessageW((LPMSG)this, nullptr, 0, 0, PM_REMOVE);
}
bool MessageStruct::translate() const noexcept
{
	return TranslateMessage((LPMSG)this);
}
LRESULT MessageStruct::dispatch() const noexcept
{
	return DispatchMessageW((LPMSG)this);
}

MessageLoop * MessageLoop::getInstance() noexcept
{
	static thread_local MessageLoop pump;
	return &pump;
}

MessageLoop::MessageLoop() noexcept
{
}
MessageLoop::~MessageLoop() noexcept
{
}

void MessageLoop::attachAccelerator(HWND hWnd, HACCEL hAccel) noexcept
{
	attach(_new AcceleratorTranslator(hWnd, hAccel));
}
void MessageLoop::attachBasicTranslator() noexcept
{
	attach(_new BasicTranslator);
}
bool MessageLoop::get() noexcept
{
	return m_msg.get();
}
bool MessageLoop::tryGet() noexcept
{
	return m_msg.tryGet();
}
bool MessageLoop::peek() noexcept
{
	return m_msg.peek();
}
void MessageLoop::dispatch() throws(QuitException)
{
	for (Translator& trans : reverse())
	{
		if (trans.translate(&m_msg))
			return;
	}

	m_msg.dispatch();

	if(m_msg.message == WM_QUIT)
		throw QuitException((int)m_msg.wParam);
}

int MessageLoop::messageLoop() noexcept
{
	try
	{
		while (get())
			dispatch();

		return (int)m_msg.wParam;
	}
	catch (QuitException& e)
	{
		return e.exitCode;
	}
}
uint MessageLoop::getLastMessage() noexcept
{
	return m_msg.message;
}

bool AcceleratorTranslator::translate(const MessageStruct* pMsg) noexcept
{
	if (::TranslateAcceleratorW(m_hWnd, m_hAccTable, (LPMSG)pMsg)) return true;
	return false;
}
AcceleratorTranslator::AcceleratorTranslator(HWND hWnd, HACCEL hAccTable) noexcept
{
	m_hWnd = hWnd;
	m_hAccTable = hAccTable;
}
bool BasicTranslator::translate(const MessageStruct* pMsg) noexcept
{
	::TranslateMessage((LPMSG)pMsg);
	return false;
}

ivec2 kr::getPointFromParam(LPARAM lParam) noexcept
{
	return (ivec2)(svec2&)lParam;
}

#else

EMPTY_SOURCE

#endif