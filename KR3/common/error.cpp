#include "stdafx.h"
#include "error.h"

using namespace kr;

ErrorCode::ErrorCode(HRESULT error) noexcept
{
	m_error = error;
}
ErrorCode::operator HRESULT() const noexcept
{
	return m_error;
}
ErrorMessage::ErrorMessage(const char * error) noexcept
	:m_error(error)
{
}
const char * ErrorMessage::getMessage() noexcept
{
	return m_error;
}


#ifdef WIN32
#include <KR3/win/windows.h>

namespace
{
	constexpr int ERRORBOX_WIDTH = 400;
	constexpr int ERRORBOX_HEIGHT = 300;
	constexpr int WM_ERRORQUIT = WM_USER + 20;
	const char16 s_errorBoxClassName[] = u"KR3_ErrorBox";
	const char16 s_errorBoxTitle[] = u"¿À·ù";
	LRESULT CALLBACK errorProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostMessageW(nullptr, WM_ERRORQUIT, 0, 0);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDOK: DestroyWindow(hWnd); break;
			}
			break;
		default: 
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}
}

template <>
TSZ ErrorCode::getMessage<char>() const noexcept
{
	TSZ dest;
	getMessageTo(&dest);
	return dest;
}
template <>
TSZ16 ErrorCode::getMessage<char16>() const noexcept
{
	TSZ16 dest;
	getMessageTo(&dest);
	return dest;
}

template <>
void ErrorCode::getMessageTo<char>(TSZ* dest) const noexcept
{
	size_t cap = 256;
	dest->padding(cap);
	for (;;)
	{
		DWORD res = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, m_error, 0, dest->end(), intact<DWORD>(dest->remaining()), nullptr);
		if (res == 0)
		{
			DWORD err = GetLastError();
			if (err != ERROR_INSUFFICIENT_BUFFER)
			{
				_assert(err == ERROR_MR_MID_NOT_FOUND);
				*dest << "Unknown Error";
				break;
			}
			cap *= 2;
			dest->padding(cap);
			continue;
		}
		dest->prepare(res - 2);
		break;
	}
}
template <>
void ErrorCode::getMessageTo<char16>(TSZ16* dest) const noexcept
{
	size_t cap = 256;
	dest->padding(cap);
	for (;;)
	{
		DWORD res = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, m_error, 0, wide(dest->end()), intact<DWORD>(dest->remaining()), nullptr);
		if (res == 0)
		{
			DWORD err = GetLastError();
			if (err != ERROR_INSUFFICIENT_BUFFER)
			{
				_assert(err == ERROR_MR_MID_NOT_FOUND);
				*dest << u"Unknown Error";
				break;
			}
			cap *= 2;
			dest->padding(cap);
			continue;
		}
		dest->prepare(res - 2);
		break;
	}
}

void kr::errorBox(pcstr16 str) noexcept
{
	HINSTANCE instanceHandle = GetModuleHandle(nullptr);

	WNDCLASSEXW wc;
	mema::zero(wc);
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = instanceHandle;
	wc.lpfnWndProc = errorProc;
	wc.lpszClassName = (LPCWSTR)s_errorBoxClassName;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	if (RegisterClassExW(&wc) == 0)
	{
		MessageBoxW(nullptr, (LPCWSTR)str, nullptr, MB_OK | MB_ICONERROR);
		return;
	}

	HWND window;
	HFONT font;

	finally{
		UnregisterClassW(wide(s_errorBoxClassName), instanceHandle);
		DestroyWindow(window);
		DeleteObject(font);
	};

	int style = WS_CAPTION | WS_SYSMENU;
	RECT rc = { 0, 0, ERRORBOX_WIDTH, ERRORBOX_HEIGHT };
	AdjustWindowRect(&rc, style, FALSE);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	window = CreateWindowEx(0, wide(s_errorBoxClassName), wide(s_errorBoxTitle), style, x, y, width, height, nullptr, nullptr, instanceHandle, nullptr);
	if (window == nullptr)
	{
		MessageBoxW(nullptr, wide(str), nullptr, MB_OK | MB_ICONERROR);
		return;
	}
	HWND edit = CreateWindowExW(WS_EX_CLIENTEDGE | WS_EX_TOPMOST, L"edit", L"", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_MULTILINE | WS_VSCROLL, 0, 0, ERRORBOX_WIDTH, ERRORBOX_HEIGHT - 45, window, nullptr, instanceHandle, nullptr);
	HWND button = CreateWindowExW(0, L"button", L"OTL", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, ERRORBOX_WIDTH - 70, ERRORBOX_HEIGHT - 35, 60, 25, window, (HMENU)IDOK, instanceHandle, nullptr);
	font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"¸¼Àº °íµñ");
	SendMessageW(button, WM_SETFONT, 0, (LPARAM)font);
	SetWindowTextW(edit, wide(str));
	ShowWindow(window, SW_SHOW);

	MSG msg;
	while(GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		if (msg.message == WM_ERRORQUIT)
			break;
	}
}
ErrorCode ErrorCode::getLast() noexcept
{
	return GetLastError();
}

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

template <>
TSZ ErrorCode::getMessage<char>() const noexcept
{
	TSZ dest((size_t)0, 1024);
	dest << "ERR: (" << m_error << ')';
	return dest;
}
template <>
TSZ16 ErrorCode::getMessage<char16>() const noexcept
{
	TSZ16 dest((size_t)0, 1024);
	dest << u"ERR: (" << m_error << u')';
	return dest;
}

void kr::errorBox(pcstr16 str) noexcept
{
	EM_ASM_("alert(UTF16ToString(x))", str);
}
ErrorCode ErrorCode::getLast() noexcept
{
	return (HRESULT)errno;
}

#else


#endif

void ErrorCode::print() noexcept
{
	ucerr << getMessage<char16>() << u'(' << m_error << u')' << endl;
	udout << getMessage<char16>() << u'(' << m_error << u')' << endl;
}
FunctionError::FunctionError(const char * funcname, HRESULT error) noexcept
	:ErrorCode(error), m_funcname(funcname)
{
}
const char * FunctionError::getFunctionName() const noexcept
{
	return m_funcname;
}

