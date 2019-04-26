#include "stdafx.h"
#include "winx.h"
#include <CommDlg.h>

#pragma warning(disable:4800)

namespace
{
	const kr::char16 WC_WINDOWPROGRAM[] = u"KrWindowProgram";
}

kr::WndProcWrapper::WndProcWrapper()
{
}
kr::WndProcWrapper::WndProcWrapper(WndProc pProc)
{
	m_pNextProc = pProc;
}
void kr::WndProcWrapper::wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam)
{
	throw CallWindowProcW((WNDPROC)m_pNextProc, pWindow, Msg, wParam, lParam);
}

kr::WindowProgram::WindowProgram() noexcept
{
	m_window = nullptr;
	procedures.attach(this);
}
kr::WindowProgram::WindowProgram(win::Window* pWindow) noexcept
{
	_assert(pWindow != nullptr);
	pWindow->setProgram(this);
	m_window = pWindow;
	procedures.attach(this);
}
kr::WindowProgram::~WindowProgram() noexcept
{
	procedures.detach(this);
}
kr::win::Window* kr::WindowProgram::detachWindow() noexcept
{
	win::Window* pwnd = m_window;
	m_window = nullptr;
	return pwnd;
}
kr::win::Window* kr::WindowProgram::getWindow() noexcept
{
	return m_window;
}
kr::win::Window* kr::WindowProgram::createPrimary(pcstr16 title, int style, const irectwh & rc) noexcept
{
	return win::Window::createPrimary(WC_WINDOWPROGRAM, title, style, rc, nullptr, this);
}
kr::win::Window* kr::WindowProgram::createPrimary(pcstr16 title, int style, int width, int height) noexcept
{
	return win::Window::createPrimary(WC_WINDOWPROGRAM, title, style, width, height, nullptr, this);
}
kr::win::Window* kr::WindowProgram::createPrimary(pcstr16 title, int style) noexcept
{
	return win::Window::createPrimary(WC_WINDOWPROGRAM, title, style, nullptr, this);
}
void kr::WindowProgram::create(pcstr16 title, int style, const irectwh & rc) noexcept
{
	win::Window::create(WC_WINDOWPROGRAM, title, style, rc, nullptr, nullptr, this);
}
kr::win::Window* kr::WindowProgram::createPrimaryEx(int exstyle, pcstr16 title, int style, const irectwh & rc) noexcept
{
	return win::Window::createPrimaryEx(exstyle, WC_WINDOWPROGRAM, title, style, rc, nullptr, this);
}
kr::win::Window* kr::WindowProgram::createPrimaryEx(int exstyle, pcstr16 title, int style, int width, int height) noexcept
{
	return win::Window::createPrimaryEx(exstyle, WC_WINDOWPROGRAM, title, style, width, height, nullptr, this);
}
kr::win::Window* kr::WindowProgram::createPrimaryEx(int exstyle, pcstr16 title, int style) noexcept
{
	return win::Window::createPrimaryEx(exstyle, WC_WINDOWPROGRAM, title, style, nullptr, this);
}
void kr::WindowProgram::createEx(int exstyle, pcstr16 title, int style, const irectwh & rc) noexcept
{
	win::Window::createEx(exstyle, WC_WINDOWPROGRAM, title, style, rc, nullptr, nullptr, this);
}
void kr::WindowProgram::destroy() noexcept
{
	delete m_window; // set null by procedure
}
ATOM kr::WindowProgram::registerClass(HICON icon, uint style) noexcept
{
	return registerClass(WC_WINDOWPROGRAM, icon, style);
}
ATOM kr::WindowProgram::registerClass(int icon, uint style) noexcept
{
	return registerClass(WC_WINDOWPROGRAM, icon, style);
}
ATOM kr::WindowProgram::registerClass(pcstr16 className, HICON icon, uint style) noexcept
{
	return WindowClass(className, _wndProc, icon, style).registerClass();
}
ATOM kr::WindowProgram::registerClass(pcstr16 className, int icon, uint style) noexcept
{
	return WindowClass(className, _wndProc, icon, style).registerClass();
}
LRESULT kr::WindowProgram::_wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam) noexcept
{
	WindowProgram* pProgram;
	if (Msg == WM_NCCREATE)
	{
		pProgram = (WindowProgram*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		if (pProgram != nullptr)
		{
			pProgram->m_window = pWindow;
			pWindow->setProgram(pProgram);
		}
		else goto _default;
	}
	else
	{
		pProgram = pWindow->getProgram();
		if (pProgram == nullptr) goto _default;
	}

	try
	{
		for (WndProcedure &proc : pProgram->procedures.reverse())
		{
			proc.wndProc(pWindow, Msg, wParam, lParam);
		}
		goto _default;
	}
	catch (LRESULT lRes)
	{
		if (Msg == WM_NCDESTROY) pProgram->m_window.detach();
		return lRes;
	}
_default:
	return DefWindowProcW(pWindow, Msg, wParam, lParam);
}

bool kr::ExTranslator::translate(const MessageStruct* pMsg)
{
	switch (pMsg->message)
	{
	case WM_CHAR:
		win::Window* pWindow = pMsg->window;
		if (!(pWindow->getStyle() & WS_TABSTOP)) break;

		switch (pMsg->wParam)
		{
		case VK_TAB:
			pWindow->getNextTab()->setFocus();
			return true;
		case VK_RETURN:
			pWindow->getParent()->postMessage(WM_COMMAND, MAKELONG((intptr_t)pWindow->getMenu(), IDN_KCONTROL), (LPARAM)pWindow);
			return true;
		}
		break;
	}
	return false;
}

kr::SubClassingProgram::SubClassingProgram()
{
}
kr::SubClassingProgram::SubClassingProgram(win::Window* pWindow)
{
	m_window = pWindow;

	m_nextProc = (WndProc)pWindow->getLongPtr(GWLP_WNDPROC);
	pWindow->setLongPtr(GWLP_USERDATA, (intptr_t)this);
	pWindow->setLongPtr(GWLP_WNDPROC, (intptr_t)_wndProc);
	procedures.attachFirst(&m_nextProc);
}
kr::SubClassingProgram::~SubClassingProgram()
{
	procedures.detach(&m_nextProc);
}

kr::WindowClass::WindowClass() noexcept
{
	mema::zero(*this);
	m_wc.cbSize = sizeof(m_wc);
	m_wc.hInstance = win::g_instance;
	m_wc.hCursor = win::Cursor::load(IDC_ARROW);
}
kr::WindowClass::WindowClass(pcstr16 pszClassName, WndProc pfnWndProc, HICON hIcon, uint style, int wndExt, int clsExt) noexcept
	:WindowClass()
{
	m_wc.lpszClassName = wide(pszClassName);
	m_wc.lpfnWndProc = (WNDPROC)pfnWndProc;
	m_wc.cbClsExtra = clsExt;
	m_wc.cbWndExtra = wndExt;
	m_wc.hIcon = hIcon;
	m_wc.style = style;
}
kr::WindowClass::WindowClass(pcstr16 pszClassName, WndProc pfnWndProc, int nIcon, uint style, int wndExt, int clsExt) noexcept
	:WindowClass()
{
	m_wc.lpszClassName = wide(pszClassName);
	m_wc.lpfnWndProc = (WNDPROC)pfnWndProc;
	m_wc.cbClsExtra = clsExt;
	m_wc.cbWndExtra = wndExt;
	m_wc.hIcon = win::Icon::load(win::g_instance, nIcon);
	m_wc.style = style;
}
ATOM kr::WindowClass::registerClass() const noexcept
{
	return RegisterClassExW(&m_wc);
}

bool kr::WindowClass::unregister(pcstr16 name) noexcept
{
	return UnregisterClassW(wide(name), win::g_instance);
}

const wchar_t * kr::makeIntResource(int res) noexcept
{
	return MAKEINTRESOURCE(res);
}
kr::irect kr::getMonitorRectFromIndex(dword index) noexcept
{
	DISPLAY_DEVICEW dd;
	dd.cb = sizeof(dd);
	if (!EnumDisplayDevicesW(nullptr, index, &dd, 0))
	{
		return {0, 0, 0, 0};
	}

	struct DataBag
	{
		irect rect;
		LPWSTR name;
	};
	
	DataBag bag = {
		{ 0, 0, 0, 0 },
		dd.DeviceName
	};

	BOOL bRes = EnumDisplayMonitors(
		nullptr, nullptr,
		[](HMONITOR hMonitor, HDC hDC, LPRECT rc, LPARAM data) -> BOOL {
		DataBag* bag = (DataBag*)data;
		MONITORINFOEXW mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfoW(hMonitor, &mi))
		{
			if (wcscmp(bag->name, mi.szDevice) == 0)
			{
				bag->rect = (irect&)mi.rcWork;
				return false;
			}
		}
		return true;
	}, (LPARAM)&bag);
	return bag.rect;
}
kr::irect kr::getMonitorRectFromCursor() noexcept
{
	POINT pt;
	GetCursorPos(&pt);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(hMon, &mi);
	return (irect&)mi.rcWork;
}
kr::irectwh kr::calculateWindowPos(dword style, dword w, dword h)
{
	RECT wndrect = { 0,0,(long)w,(long)h };
	AdjustWindowRect(&wndrect, style, false);
	wndrect.right -= wndrect.left;
	wndrect.bottom -= wndrect.top;

	return calculateWindowPos(wndrect.right, wndrect.bottom);
}
kr::irectwh kr::calculateWindowPos(dword w, dword h)
{
	irect monrc = getMonitorRectFromCursor();
	dword widthmax = (monrc.right - monrc.left);
	dword heightmax = (monrc.bottom - monrc.top);

	irectwh rc;
	rc.x = (monrc.right + monrc.left - (int)tmin(w, widthmax)) / 2;
	rc.y = (monrc.bottom + monrc.top - (int)tmin(h, heightmax)) / 2;
	rc.width = w;
	rc.height = h;
	return rc;
}
void kr::visiblePrimaryWindow(dword style, dword w, dword h)
{
	irectwh rc = calculateWindowPos(style, w, h);
	win::g_mainWindow->setPos(nullptr, rc.x, rc.y, rc.width, rc.height, SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
}
