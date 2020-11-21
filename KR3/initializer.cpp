#include "stdafx.h"
#include "initializer.h"

#ifdef WIN32

#include <windows.h>
#include <CommCtrl.h>
#include <float.h>
#include <GdiPlus.h>

kr::COM::Init::Init() noexcept
{
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) error("COM 초기화 실패: %p", hr);
}
kr::COM::Init::~Init() noexcept
{
	CoUninitialize();
}
kr::OLE::Init::Init() noexcept
{
	HRESULT hr = OleInitialize(nullptr);
	if (FAILED(hr)) error("OLE 초기화 실패: %p", hr);
}
kr::OLE::Init::~Init() noexcept
{
	OleUninitialize();
}
kr::CommonControls::Init::Init() noexcept
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	if (!InitCommonControlsEx(&icc))
	{
		error("Common Control 초기화 실패: %p", GetLastError());
	}
}
kr::CommonControls::Init::~Init() noexcept
{
}
kr::GdiPlus::Init::Init() noexcept
{
	static_assert(sizeof(token) == sizeof(ULONG_PTR), "token size unmatch");
	Gdiplus::GdiplusStartupOutput output;
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup((ULONG_PTR*)&token, &input, &output);
}
kr::GdiPlus::Init::~Init() noexcept
{
	Gdiplus::GdiplusShutdown(token);
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#endif

#ifdef __EMSCRIPTEN__

extern "C" int __cxa_thread_atexit(int, int, int)
{
	return 0;
}

#endif