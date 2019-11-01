#include "stdafx.h"
#include "handle.h"
#include "stdafx.h"
#include "gdi.h"
#include "winx.h"
#include "hack.h"

#include <process.h>
#include <KR3/wl/windows.h>
#include <Psapi.h>

#pragma comment(lib,"psapi.lib")

using namespace kr;
using namespace win;

Window * kr::win::g_mainWindow;
Instance* kr::win::g_instance = Instance::getModule(nullptr);
Cursor::Current Cursor::current;

#pragma warning(disable:4800)

ivec2 win::getCursorPos() noexcept
{
	ivec2 pt;
	GetCursorPos((POINT*)&pt);
	return pt;
}

Library* Library::load(pcstr16 str) noexcept
{
	return (Library*)LoadLibraryW(wide(str));
}
Module * Library::getModule(pcstr16 str) noexcept
{
	return (Module*)GetModuleHandleW(wide(str));
}
void Library::operator delete(void * library) noexcept
{
	FreeLibrary((HMODULE)library);
}
const autovar<sizeof(ptr)> Library::get(pcstr str) noexcept
{
	FARPROC t = GetProcAddress(this, str);
	return t;
}
template <> size_t ProcessAndModule::getName<char>(char* dest, size_t capacity) const noexcept
{
	return GetModuleBaseNameA(process, module, dest, intact<DWORD>(capacity));
}
template <> size_t ProcessAndModule::getNameLength<char>() const noexcept
{
	return GetModuleBaseNameA(process, module, nullptr, 0);
}
template <> size_t ProcessAndModule::getName<char16>(char16* dest, size_t capacity) const noexcept
{
	return GetModuleBaseNameW(process, module, wide(dest), intact<DWORD>(capacity));
}
template <> size_t ProcessAndModule::getNameLength<char16>() const noexcept
{
	return GetModuleBaseNameW(process, module, nullptr, 0);
}

void Cursor::operator delete(void * p)
{
	DestroyCursor((HCURSOR)p);
}
Cursor* Cursor::load(pcstr name)
{
	if (IS_INTRESOURCE(name))
	{
		return load((uintptr_t)name);
	}
	return (Cursor*)LoadImageA(nullptr, name, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}
Cursor* Cursor::load(pcstr16 name)
{
	return load(wide(name));
}
Cursor* Cursor::load(const wchar_t * name)
{
	if (IS_INTRESOURCE(name))
	{
		return load((uintptr_t)name);
	}
	return (Cursor*)LoadImageW(nullptr, name, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}
Cursor* Cursor::load(uintptr_t id)
{
	return (Cursor*)LoadCursorW(nullptr, MAKEINTRESOURCE(id));
}
Cursor* Cursor::load(Instance* hInstance, uintptr_t id)
{
	return (Cursor*)LoadCursorW(hInstance, MAKEINTRESOURCE(id));
}
Cursor::Current& Cursor::Current::operator =(Cursor * cursor) noexcept
{
	SetCursor(cursor);
	return *this;
}
Cursor::Current::operator Cursor*() noexcept
{
	return (Cursor*)GetCursor();
}

Icon* Icon::load(pcstr name)
{
	return (Icon*)LoadImageA(nullptr, name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}
Icon* Icon::load(pcstr16 name)
{
	return load(wide(name));
}
Icon* Icon::load(const wchar_t * name)
{
	return (Icon*)LoadImageW(nullptr, name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}
Icon* Icon::load(uintptr_t id)
{
	return (Icon*)LoadIconW(nullptr, MAKEINTRESOURCE(id));
}
Icon* Icon::load(Instance * hInstance, uintptr_t id)
{
	return (Icon*)LoadIconW(hInstance, MAKEINTRESOURCE(id));
}

Menu* Menu::load(uintptr_t id) noexcept
{
	return (Menu*)LoadMenuW(nullptr, MAKEINTRESOURCE(id));
}
Menu* Menu::loadPick(uintptr_t id, int nPos) noexcept
{
	Menu* pMenu = load(id);
	Menu* pItem = pMenu->pickItem(nPos);
	delete pMenu;
	return pItem;
}
void Menu::operator delete(void * p) noexcept
{
	DestroyMenu((HMENU)p);
}
BOOL Menu::enableItem(uint iItem, uint flags) noexcept
{
	return EnableMenuItem(this, iItem, flags);
}
DWORD Menu::checkItem(UINT uIDCheckItem, UINT uCheck) noexcept
{
	return CheckMenuItem(this, uIDCheckItem, uCheck);
}
BOOL kr::win::Menu::checkRadioItem(UINT first, UINT last, UINT check, UINT flags) noexcept
{
	return CheckMenuRadioItem(this, first, last, check, flags);
}
BOOL Menu::trackPopup(uint uFlags,ivec2 pt) noexcept
{
	return TrackPopupMenu(this, uFlags, pt.x, pt.y, 0, g_mainWindow, nullptr);
}
Menu* Menu::getItem(int nPos) noexcept
{
	return (Menu*)GetSubMenu(this, nPos);
}
BOOL Menu::removeItem(uint uPos, uint uFlags) noexcept
{
	return RemoveMenu(this, uPos, uFlags);
}
Menu* Menu::pickItem(int nPos) noexcept
{
	Menu* pMenu = getItem(nPos);
	removeItem(nPos, MF_BYPOSITION);
	return pMenu;
}

Accelerator* Accelerator::load(uintptr_t id) noexcept
{
	return (Accelerator*)LoadAccelerators(g_instance, MAKEINTRESOURCE(id));
}

const Window * Window::TOP = ((Window*)0);
const Window * Window::BOTTOM = ((Window*)1);
const Window * Window::TOPMOST = ((Window*)-1);
const Window * Window::NOTOPMOST = ((Window*)-2);

void Window::operator delete(void * p) noexcept
{
	DestroyWindow((HWND)p);
}
bool Window::adjustRect(irect * rc, int style, bool menu) noexcept
{
	return AdjustWindowRect((LPRECT)rc, style, menu);
}
bool Window::adjustRect(irect * rc) noexcept
{
	return adjustRect(rc, getStyle(), getMenu() != nullptr);
}
bool kr::win::Window::resizeToCenter(int width, int height) noexcept
{
	irect wndrect = {0, 0, width, height};
	if (!adjustRect(&wndrect))
		return false;

	irect screen = getMonitor()->getMonitorArea();
	screen.right -= screen.left;
	screen.bottom -= screen.top;
	wndrect.right -= wndrect.left;
	wndrect.bottom -= wndrect.top;

	return move(
		(screen.right - wndrect.right) / 2 + screen.left,
		(screen.bottom - wndrect.bottom) / 2 + screen.top,
		wndrect.right, wndrect.bottom, false);
}
long Window::setLong(int index,long value) noexcept
{
	return SetWindowLongW(this,index,value);
}
long Window::getLong(int index) noexcept
{
	return GetWindowLongW(this,index);
}
intptr_t Window::setLongPtr(int index,intptr_t value) noexcept
{
	return SetWindowLongPtrW(this,index,value);
}
intptr_t Window::getLongPtr(int index) noexcept
{
	return GetWindowLongPtrW(this,index);
}
WndProc Window::setProc(WndProc proc) noexcept
{
	return (WndProc)setLongPtr(GWLP_WNDPROC, (intptr_t)proc);
}
WndProc Window::getProc() noexcept
{
	return (WndProc)getLongPtr(GWLP_WNDPROC);
}
long Window::setStyle(long style) noexcept
{
	return setLong(GWL_STYLE,style);
}
long Window::getStyle() noexcept
{
	return getLong(GWL_STYLE);
}
long Window::addStyle(long style) noexcept
{
	return setStyle(getStyle() | style);
}
long Window::setExStyle(long style) noexcept
{
	return setLong(GWL_EXSTYLE,style);
}
long Window::getExStyle() noexcept
{
	return getLong(GWL_EXSTYLE);
}
long Window::addExStyle(long style) noexcept
{
	return setExStyle(getExStyle() | style);
}
long Window::removeStyle(long style) noexcept
{
	return setStyle(getStyle() & ~style);
}
long Window::removeExStyle(long style) noexcept
{
	return setExStyle(getStyle() & ~style);
}
WindowProgram* Window::setProgram(WindowProgram * pProgram) noexcept
{
	return (WindowProgram*)setLongPtr(GWLP_USERDATA,(intptr_t)pProgram);
}
WindowProgram* Window::getProgram() noexcept
{
	return (WindowProgram*)getLongPtr(GWLP_USERDATA);
}
bool Window::clientToScreen(ivec2* lpPoint) noexcept
{
	return ::ClientToScreen(this,(LPPOINT)lpPoint);
}
bool Window::screenToClient(ivec2* lpPoint) noexcept
{
	return ::ScreenToClient(this,(LPPOINT)lpPoint);
}
bool Window::setPos(ivec2 pos) noexcept
{
	return ::SetWindowPos(this, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
bool Window::setSize(ivec2 size) noexcept
{
	return ::SetWindowPos(this, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER);
}
bool Window::setPos(Window* hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) noexcept
{
	return ::SetWindowPos(this,hWndInsertAfter,X,Y,cx,cy,uFlags);
}
bool Window::move(const irectwh &irectwh,bool bRepaint) noexcept
{
	return ::MoveWindow(this,irectwh.x,irectwh.y,irectwh.width,irectwh.height,bRepaint);
}
bool Window::move(int X,int Y,int nWidth,int nHeight,bool bRepaint) noexcept
{
	return ::MoveWindow(this,X,Y,nWidth,nHeight,bRepaint);
}
bool kr::win::Window::resize(int width, int height, bool repaint) noexcept
{
	UINT flags = SWP_NOZORDER | SWP_NOMOVE;
	if (!repaint)
		flags |= SWP_NOREDRAW;
	return setPos(nullptr, 0, 0, width, height, flags);
}
bool Window::show(int nCmdShow) noexcept
{
	return ::ShowWindow(this, nCmdShow);
}
bool Window::update() noexcept
{
	return ::UpdateWindow(this);
}
int Window::setRgn(RGN* rgn,bool bRedraw) noexcept
{
	return ::SetWindowRgn(this,(HRGN)rgn,bRedraw);
}
UINT_PTR Window::setTimer(UINT_PTR nIDEvent,UINT uElapse,TIMERPROC lpTimerFunc) noexcept
{
	return ::SetTimer(this,nIDEvent,uElapse,lpTimerFunc);
}
bool Window::killTimer(UINT_PTR uIDEvent) noexcept
{
	return ::KillTimer(this,uIDEvent);
}
bool Window::invalidateRect(const irect& rect,bool bErase) noexcept
{
	return ::InvalidateRect(this,(const RECT*)&rect,bErase);
}
bool Window::invalidate(bool bErase) noexcept
{
	return ::InvalidateRect(this,nullptr,bErase);
}
bool Window::validateRect(const irect& rect) noexcept
{
	return ::ValidateRect(this,(const RECT*)&rect);
}
bool Window::validate() noexcept
{
	return ::ValidateRect(this, nullptr);
}
irect Window::getRect() noexcept
{
	RECT rect;
	BOOL res = ::GetWindowRect(this, &rect);
	_assert(res);
	return (irect&)rect;
}
irect Window::getClientRect() noexcept
{
	RECT rect;
	BOOL res = ::GetClientRect(this, &rect);
	_assert(res);
	return (irect&)rect;
}
Monitor* Window::getMonitor() noexcept
{
	Monitor* monitor = (Monitor*)MonitorFromWindow(g_mainWindow, MONITOR_DEFAULTTONEAREST);
	_assert(monitor != nullptr);
	return monitor;
}
ivec2 Window::getPos() noexcept
{
	ivec2 pos = getRect().from;
	Window * parent = getParent();
	if (parent == nullptr) return pos;
	bool res_screenToClient = parent->screenToClient(&pos);
	_assert(res_screenToClient);
	return pos;
}
DrawContext* Window::beginPaint(PAINTSTRUCT *ps) noexcept
{
	return (DrawContext*)::BeginPaint(this,ps);
}
BOOL Window::endPaint(PAINTSTRUCT *ps) noexcept
{
	return ::EndPaint(this,ps);
}
DrawContext* Window::getDC() noexcept
{
	return (DrawContext*)::GetDC(this);
}
int Window::releaseDC(DrawContext * dc) noexcept
{
	return ::ReleaseDC(this,(HDC)dc);
}
bool Window::updateLayer(DrawContext* dc, const irectwh & region) noexcept
{
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = AC_SRC_ALPHA;
	POINT srcPoint = { region.x, region.y };
	SIZE size = { region.width, region.height };
	return UpdateLayeredWindow(this, nullptr, nullptr, &size, dc, &srcPoint, 0, &bf, ULW_ALPHA);
}
bool Window::updateLayer(DrawContext* dc, int width, int height) noexcept
{
	return updateLayer(dc, { 0, 0, width, height });
}
BOOL Window::postMessage(UINT Msg,WPARAM wParam,LPARAM lParam) noexcept
{
	return ::PostMessageW(this,Msg,wParam,lParam);
}
BOOL Window::postMessageA(UINT Msg,WPARAM wParam,LPARAM lParam) noexcept
{
	return ::PostMessageA(this,Msg,wParam,lParam);
}
LRESULT Window::sendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
	return ::SendMessageW(this, Msg, wParam, lParam);
}
LRESULT Window::sendMessageA(UINT Msg,WPARAM wParam,LPARAM lParam) noexcept
{
	return ::SendMessageA(this,Msg,wParam,lParam);
}
void Window::setIcon(Icon* icon) noexcept
{
	setIcon(icon, false);
	setIcon(icon, true);
}
Icon* Window::setIcon(Icon* icon, bool iconBig) noexcept
{
	return (Icon*)sendMessage(WM_SETICON, (WPARAM)iconBig, (LPARAM)icon);
}
Icon* Window::getIcon() noexcept
{
	return (Icon*)sendMessage(WM_GETICON, 0, 0);
}
Font* Window::setFont(Font* pFont) noexcept
{
	return (Font*)sendMessage(WM_SETFONT,(WPARAM)pFont,0);
}
Font* Window::getFont() noexcept
{
	return (Font*)sendMessage(WM_GETFONT, 0, 0);
}
BOOL Window::enable(bool bEnable) noexcept
{
	return EnableWindow(this,bEnable);
}
int Window::msgBox(pcstr strMessage, pcstr strTitle, uint flags) noexcept
{
	return ::MessageBoxA(this, strMessage, strTitle, flags);
}
int Window::msgBox(pcstr16 strMessage, pcstr16 strTitle, uint flags) noexcept
{
	return ::MessageBoxW(this, wide(strMessage), wide(strTitle), flags);
}
int Window::errorBox(pcstr strMessage) noexcept
{
	return msgBox(strMessage, nullptr, MB_OK | MB_ICONERROR);
}
int Window::errorBox(pcstr16 strMessage) noexcept
{
	return msgBox(strMessage,nullptr,MB_OK | MB_ICONERROR);
}
int Window::informationBox(pcstr16 strMessage) noexcept
{
	return msgBox(strMessage, nullptr, MB_OK | MB_ICONINFORMATION);
}
template <> size_t Window::getText<char>(char * dest,size_t capacity) const noexcept
{
	return GetWindowTextA((Window*)this, dest, intact<int>(capacity));
}
template <> size_t Window::getText<char16>(char16 * dest,size_t capacity) const noexcept
{
	return GetWindowTextW((Window*)this, wide(dest), intact<int>(capacity));
}
template <> size_t Window::getTextLength<char>() const noexcept
{
	return GetWindowTextLengthA((Window*)this);
}
template <> size_t Window::getTextLength<char16>() const noexcept
{
	return GetWindowTextLengthW((Window*)this);
}
int Window::setText(pcstr src) noexcept
{
	return SetWindowTextA(this,src);
}
int Window::setText(pcstr16 src) noexcept
{
	return SetWindowTextW(this, wide(src));
}
bool Window::isCaptured() noexcept
{
	return (GetCapture() == this);
}
Window* Window::setCapture() noexcept
{
	return (Window*)::SetCapture(this);
}
bool Window::releaseCapture() noexcept
{
	if(!isCaptured()) return false;
	::ReleaseCapture();
	return true;
}
bool Window::setMenu(Menu * menu) noexcept
{
	return ::SetMenu(this, menu);
}
Menu* Window::getMenu() noexcept
{
	return (Menu*)::GetMenu(this);
}
BOOL Window::trackMouseEvent(dword dwFlags) noexcept
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = dwFlags;
	tme.dwHoverTime = 20;
	tme.hwndTrack = this;
	return ::TrackMouseEvent(&tme);
}
ivec2 Window::getCursorPos() noexcept
{
	ivec2 pos = win::getCursorPos();
	screenToClient(&pos);
	return pos;
}
int Window::getScrollPos(int nBar) noexcept
{
	return GetScrollPos(this, nBar);
}
int Window::getScrollY() noexcept
{
	return GetScrollPos(this, SB_VERT);
}
int Window::getScrollX() noexcept
{
	return GetScrollPos(this, SB_HORZ);
}
int Window::getScrollCtlPos() noexcept
{
	return GetScrollPos(this, SB_CTL);
}
bool Window::moveScrollPos(int nBar, int scroll, bool redraw) noexcept
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	BOOL res = GetScrollInfo(this, nBar, &si);
	_assert(res);

	if(si.nPos == si.nMin && scroll < 0) return false;
	if(si.nPos + (int)si.nPage >= si.nMax && scroll > 0) return false;
	setScrollPos(nBar, si.nPos + scroll, redraw);
	return true;
}
bool Window::moveScrollY(int scroll, bool redraw) noexcept
{
	return moveScrollPos(SB_VERT, scroll, redraw);
}
bool Window::moveScrollX(int scroll, bool redraw) noexcept
{
	return moveScrollPos(SB_HORZ, scroll, redraw);
}
bool Window::moveScrollCtlPos(int scroll, bool redraw) noexcept
{
	return moveScrollPos(SB_CTL, scroll, redraw);
}
int Window::setScrollPos(int nBar, int scroll, bool redraw) noexcept
{
	return SetScrollPos(this, nBar, scroll, redraw);
}
int Window::setScrollY(int scroll, bool redraw) noexcept
{
	return SetScrollPos(this, SB_VERT, scroll, redraw);
}
int Window::setScrollX(int scroll, bool redraw) noexcept
{
	return SetScrollPos(this, SB_HORZ, scroll, redraw);
}
int Window::setScrollCtlPos(int scroll, bool redraw) noexcept
{
	return SetScrollPos(this, SB_CTL, scroll, redraw);
}
int Window::setScroll(int nBar, int page, int range, bool redraw) noexcept
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;
	si.nMax = range;
	si.nPage = page;
	return SetScrollInfo(this, nBar, &si, redraw);
}
int Window::setScrollHorz(int page, int range, bool redraw) noexcept
{
	return setScroll(SB_HORZ, page, range, redraw);
}
int Window::setScrollVert(int page, int range, bool redraw) noexcept
{
	return setScroll(SB_VERT, page, range, redraw);
}
int Window::setScrollCtl(int page, int range, bool redraw) noexcept
{
	return setScroll(SB_CTL, page, range, redraw);
}
int Window::getScrollPage(int nBar) noexcept
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE;
	BOOL res = GetScrollInfo(this,nBar,&si);
	_assert(res);
	return si.nPage;
}
int Window::getScrollYPage() noexcept
{
	return getScrollPage(SB_VERT);
}
int Window::getScrollXPage() noexcept
{
	return getScrollPage(SB_HORZ);
}
int Window::getScrollCtlPage() noexcept
{
	return getScrollPage(SB_CTL);
}
int Window::setScrollPage(int nBar, int page, bool redraw) noexcept
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE;
	si.nPage = page;
	return SetScrollInfo(this, nBar, &si, redraw);
}
int Window::setScrollYPage(int page, bool redraw) noexcept
{
	return setScrollPage(SB_VERT, page, redraw);
}
int Window::setScrollXPage(int page, bool redraw) noexcept
{
	return setScrollPage(SB_HORZ, page, redraw);
}
int Window::setScrollCtlPage(int page, bool redraw) noexcept
{
	return setScrollPage(SB_CTL, page, redraw);
}
int Window::getScrollRange(int nBar) noexcept
{
	int min,max;
	BOOL res = GetScrollRange(this, nBar, &min, &max);
	_assert(res);
	return max;
}
int Window::getScrollYRange() noexcept
{
	return getScrollRange(SB_VERT);
}
int Window::getScrollXRange() noexcept
{
	return getScrollRange(SB_HORZ);
}
int Window::getScrollCtlRange() noexcept
{
	return getScrollRange(SB_CTL);
}
bool Window::setScrollRange(int nBar, int min, int max, bool redraw) noexcept
{
	return SetScrollRange(this, nBar, min, max, redraw);
}
bool Window::setScrollYRange(int min, int max, bool redraw) noexcept
{
	return SetScrollRange(this, SB_VERT, min, max, redraw);
}
bool Window::setScrollXRange(int min, int max, bool redraw) noexcept
{
	return SetScrollRange(this, SB_HORZ, min, max, redraw);
}
bool Window::setScrollCtlRange(int min, int max, bool redraw) noexcept
{
	return SetScrollRange(this, SB_CTL, min, max, redraw);
}

Window* Window::get(UINT uCmd) noexcept
{
	return (Window*)GetWindow(this,uCmd);
}
Window* Window::getNext() noexcept
{
	return get(GW_HWNDNEXT);
}
Window* Window::getNextTab() noexcept
{
	Window* p = this;
	do
	{
		p = p->getNext();
		if(p == nullptr) p = this->getFirst();
		if(p == this) return p;
	}
	while(!(p->getStyle() & WS_TABSTOP));
	return p;
}
Window* Window::getPrevious() noexcept
{
	return get(GW_HWNDPREV);
}
Window* Window::getFirst() noexcept
{
	return get(GW_HWNDFIRST);
}
Window* Window::getLast() noexcept
{
	return get(GW_HWNDLAST);
}
Window* Window::getOwner() noexcept
{
	return get(GW_OWNER);
}
Window* Window::getChild() noexcept
{
	return get(GW_CHILD);
}
Window* Window::getParent() noexcept
{
	return (Window*)::GetParent(this);
}
Window* Window::setParent(Window* pParent) noexcept
{
	return (Window*)::SetParent(this,pParent);
}
Window* Window::appendChild(Window* child) noexcept
{
	return (Window*)::SetParent(child, this);
}

Window* Window::setFocus() noexcept
{
	return (Window*)::SetFocus(this);
}

Static * Window::insertStatic(pcstr16 pszText, irectwh irect) noexcept
{
	return Static::create(pszText, WS_VISIBLE | WS_CHILD, irect, this);
}
Window::ProcessBag Window::getProcessId() noexcept
{
	ProcessBag bag;
	bag.threadId = GetWindowThreadProcessId(this, (LPDWORD)&bag.processId);
	return bag;
}
Window * Window::find(pcstr16 className, pcstr16 windowName) noexcept
{
	return (Window*)FindWindowW(wide(className), wide(windowName));
}
void Window::all(CallableT<bool(Window*)> * call) noexcept
{
	EnumWindows([](HWND hwnd, LPARAM call)->BOOL {
		return ((CallableT<bool(Window*)>*)call)->call((Window*)hwnd);
	}, (LPARAM)call);
}
Window * Window::getForeground() noexcept
{
	return (Window*)GetForegroundWindow();
}
Window* Window::createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return createPrimaryEx(0, pszClass, pszTitle, style, hMenu, pProgram);
}
Window* Window::createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, dword width, dword height, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return createPrimaryEx(0, pszClass, pszTitle, style, width, height, hMenu, pProgram);
}
Window * kr::win::Window::createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, irectwh rect, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return createPrimaryEx(0, pszClass, pszTitle, style, rect, hMenu, pProgram);
}
Window* Window::createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return g_mainWindow = (Window*)createEx(nExStyle, pszClass, pszTitle, style, { { CW_USEDEFAULT,CW_USEDEFAULT },{ CW_USEDEFAULT,CW_USEDEFAULT } }, nullptr, hMenu, pProgram);
}
Window* Window::createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, dword width, dword height, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	irectwh irect = calculateWindowPos(style, width, height);
	return g_mainWindow = (Window*)createEx(nExStyle, pszClass, pszTitle, style, irect, nullptr, hMenu, pProgram);
}
Window * kr::win::Window::createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, irectwh rect, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return g_mainWindow = (Window*)createEx(nExStyle, pszClass, pszTitle, style, rect, nullptr, hMenu, pProgram);
}
Window* Window::createPrimaryAsFull(pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	irectwh irect = (irectwh)getMonitorRectFromCursor();
	return g_mainWindow = (Window*)create(pszClass, pszTitle, style, irect, nullptr, hMenu, pProgram);
}
Window* Window::createEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID, WindowProgram * pProgram) noexcept
{
	return (Window*)CreateWindowExW(nExStyle, wide(pszClass), wide(pszTitle), nStyle, rc.x, rc.y, rc.width, rc.height, pParent, (HMENU)nID, g_instance, pProgram);
}
Window* Window::createEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return (Window*)CreateWindowExW(nExStyle, wide(pszClass), wide(pszTitle), nStyle, rc.x, rc.y, rc.width, rc.height, pParent, hMenu, g_instance, pProgram);
}
Window* Window::create(pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID, WindowProgram * pProgram) noexcept
{
	return createEx(0, pszClass, pszTitle, nStyle, irect, pParent, nID, pProgram);
}
Window* Window::create(pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, HMENU hMenu, WindowProgram * pProgram) noexcept
{
	return createEx(0, pszClass, pszTitle, nStyle, irect, pParent, hMenu, pProgram);
}

Dialog * kr::win::Dialog::create(int id, DlgProc proc, Window * parent) noexcept
{
	return (Dialog*)CreateDialog(g_instance, MAKEINTRESOURCE(id), parent, (DLGPROC)proc);
}
intptr_t kr::win::Dialog::modal(int id, DlgProc proc, Window * parent) noexcept
{
	return DialogBoxW(g_instance, MAKEINTRESOURCEW(id), parent, (DLGPROC)proc);
}
intptr_t kr::win::Dialog::modal(int id, DlgProc proc, Window * parent, LPARAM param) noexcept
{
	return DialogBoxParamW(g_instance, MAKEINTRESOURCEW(id), parent, (DLGPROC)proc, param);
}
intptr_t kr::win::Dialog::modalA(int id, DlgProc proc, Window * parent) noexcept
{
	return DialogBoxA(g_instance, MAKEINTRESOURCEA(id), parent, (DLGPROC)proc);
}
intptr_t kr::win::Dialog::modalA(int id, DlgProc proc, Window * parent, LPARAM param) noexcept
{
	return DialogBoxParamA(g_instance, MAKEINTRESOURCEA(id), parent, (DLGPROC)proc, param);
}
bool kr::win::Dialog::endDialog(intptr_t retcode) noexcept
{
	return EndDialog(this, retcode);
}
bool kr::win::Dialog::setItemText(int id, pcstr text) noexcept
{
	return SetDlgItemTextA(this, id, text);
}
bool kr::win::Dialog::setItemText(int id, pcstr16 text) noexcept
{
	return SetDlgItemTextW(this, id, wide(text));
}
bool kr::win::Dialog::checkItemButton(int id, uint check) noexcept
{
	return CheckDlgButton(this, id, check);
}
bool kr::win::Dialog::checkRadioButton(int first, int last, int target) noexcept
{
	return CheckRadioButton(this, first, last, target);
}
uint kr::win::Dialog::isItemButtonChecked(int id) noexcept
{
	return IsDlgButtonChecked(this, id);
}
Window * kr::win::Dialog::getItem(int id) noexcept
{
	return (Window*)GetDlgItem(this, id);
}

const char16 Button::CLASS_NAME[] = u"button";
Button* Button::createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return (Button*)Window::createEx(nExStyle, CLASS_NAME, pszTitle, nStyle, irect, pParent, nID);
}
Button* Button::create(pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return createEx(0, pszTitle, nStyle, irect, pParent, nID);
}
bool Button::isChecked() noexcept
{
	return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED;
}
void Button::setCheck(bool checked) noexcept
{
	sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}
void Button::setState(int state) noexcept
{
	sendMessage(BM_SETSTATE, state, 0);
}
int Button::getState() noexcept
{
	return (int)sendMessage(BM_GETSTATE, 0, 0);
}

const char16 Static::CLASS_NAME[] = u"static";
Static* Static::createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent) noexcept
{
	Static * pStatic = (Static*)Window::createEx(nExStyle, CLASS_NAME, pszTitle, nStyle, irect, pParent);
	pStatic->setFont(g_defaultFont);
	return pStatic;
}
Static* Static::create(pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent) noexcept
{
	return createEx(0, pszTitle, nStyle, irect, pParent);
}

const char16 EditBox::CLASS_NAME[] = u"edit";
EditBox* EditBox::createEx(dword nExStyle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	EditBox* pEdit = (EditBox*)Window::createEx(nExStyle, CLASS_NAME, zeroptr, nStyle, irect, pParent, nID);
	pEdit->setFont(g_defaultFont);
	return pEdit;
}
EditBox* EditBox::create(dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return createEx(0, nStyle, irect, pParent, nID);
}
EditBox* EditBox::createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	EditBox* pEdit = (EditBox*)Window::createEx(nExStyle, CLASS_NAME, pszTitle, nStyle, irect, pParent, nID);
	pEdit->setFont(g_defaultFont);
	return pEdit;
}
EditBox* EditBox::create(pcstr16 pszTitle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return createEx(0, pszTitle, nStyle, irect, pParent, nID);
}

void EditBox::setSelect(size_t from, size_t to) noexcept
{
	sendMessage(EM_SETSEL, (WPARAM)from, (LPARAM)to);
}
void EditBox::setSelect(size_t sel) noexcept
{
	sendMessage(EM_SETSEL, (WPARAM)sel, (LPARAM)sel);
}
void EditBox::replaceSelect(pcstr16 text, bool canBeUndone)
{
	sendMessage(EM_REPLACESEL, canBeUndone, (WPARAM)text);
}

const char16 ComboBox::CLASS_NAME[] = u"combobox";
ComboBox* ComboBox::createEx(dword nExStyle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	ComboBox* pEdit = (ComboBox*)Window::createEx(nExStyle, CLASS_NAME, zeroptr, nStyle, irect, pParent, nID);
	pEdit->setFont(g_defaultFont);
	return pEdit;
}
ComboBox* ComboBox::create(dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return createEx(0, nStyle, irect, pParent, nID);
}

void ComboBox::addString(pcstr16 text) noexcept
{
	sendMessage(CB_ADDSTRING, 0, (LPARAM)text);
}

void kr::win::ComboBox::setCursor(int idx) noexcept
{
	sendMessage(CB_SETCURSEL, idx, 0);
}
int kr::win::ComboBox::getCursor() noexcept
{
	return (int)sendMessage(CB_GETCURSEL, 0, 0);
}

const char16 ListBox::CLASS_NAME[] = u"listbox";
ListBox* ListBox::createEx(dword nExStyle, dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	ListBox * pList = (ListBox*)Window::create(CLASS_NAME, zeroptr, nStyle, irect, pParent, nID);
	pList->setFont(g_defaultFont);
	return pList;
}
ListBox* ListBox::create(dword nStyle, irectwh irect, Window* pParent, intptr_t nID) noexcept
{
	return createEx(0, nStyle, irect, pParent, nID);
}
size_t ListBox::addString(pcstr str) noexcept
{
	return sendMessageA(LB_ADDSTRING, 0, (LPARAM)str);
}
size_t ListBox::addString(pcstr16 str) noexcept
{
	return sendMessage(LB_ADDSTRING, 0, (LPARAM)str);
}

Cursor* g_cursor_t::operator =(Cursor * cursor) noexcept
{
	SetClassLongPtrW(g_mainWindow,GCLP_HCURSOR,(LONG_PTR)cursor);
	return cursor;
}
g_cursor_t::operator Cursor*() noexcept
{
	return (Cursor*)GetClassLongPtr(g_mainWindow,GCLP_HCURSOR);
}

win::Process * win::Process::open(ProcessId id) noexcept
{
	return (win::Process*)OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION, FALSE, id.value());
}
win::Process::Pair win::Process::execute(pstr strCommand, pcstr strPath, ProcessOptions opts) noexcept
{
	STARTUPINFOA si = { sizeof(STARTUPINFOA), };
	PROCESS_INFORMATION pi;
	DWORD flags = 0;
	if (opts.suspended()) flags |= CREATE_SUSPENDED;
	if (opts.console()) flags |= CREATE_NEW_CONSOLE;
	if (opts.detached()) flags |= DETACHED_PROCESS;
	if (!CreateProcessA(nullptr, strCommand, nullptr, nullptr, false, flags, nullptr, strPath, &si, &pi))
	{
		return { nullptr, nullptr };
	}
	return { (Process*)pi.hProcess, (ThreadHandle*)pi.hThread };
}
win::Process::Pair win::Process::execute(pstr16 strCommand, pcstr16 strPath, ProcessOptions opts) noexcept
{
	STARTUPINFOW si = { sizeof(STARTUPINFOW), };
	PROCESS_INFORMATION pi;
	DWORD flags = 0;
	if (opts.suspended()) flags |= CREATE_SUSPENDED;
	if (opts.console()) flags |= CREATE_NEW_CONSOLE;
	if (opts.detached()) flags |= DETACHED_PROCESS;
	if (!CreateProcessW(nullptr, wide(strCommand), nullptr, nullptr, false, flags, nullptr, wide(strPath), &si, &pi))
	{
		return { nullptr, nullptr };
	}
	return { (Process*)pi.hProcess, (ThreadHandle*)pi.hThread };
}

bool win::Process::terminate() noexcept
{
	return TerminateProcess(this, -1);
}
Module * win::Process::getFirstModule() noexcept
{
	HMODULE module;
	DWORD junk;
	if (!EnumProcessModules(this, &module, sizeof(module), &junk)) return nullptr;
	return (Module*)module;
}
Module* win::Process::injectDll(pcstr strDllPath) noexcept
{
	return (Module*)(DWORD_PTR)call((LPTHREAD_START_ROUTINE)LoadLibraryA, Buffer(strDllPath, mem::find(strDllPath, '\0') + 1));
}
Module* win::Process::injectDll(pcstr16 strDllPath) noexcept
{
	return (Module*)(DWORD_PTR)call((LPTHREAD_START_ROUTINE)LoadLibraryW, Buffer(strDllPath, mem16::find(strDllPath, u'\0') + 1));
}
dword win::Process::call(ThreadRoutine pThread, Buffer buffer) noexcept
{
	ProcessMemory memory(this, buffer);
	return call(pThread, (intptr_t)memory.getAddress());
}
dword win::Process::call(ThreadRoutine pThread, intptr_t data) noexcept
{
	static_assert(sizeof(dword) == sizeof(DWORD), "dword size unmatch");

	HANDLE hThread = CreateRemoteThread(this, nullptr, 0, pThread, (LPVOID)data, 0, nullptr);
	if (hThread == nullptr) return -1;

	DWORD dwResult;
	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, &dwResult);
	CloseHandle(hThread);
	return dwResult;
}
size_t win::Process::write(void * pDest, const void *pSrc, size_t nSize) noexcept
{
	SIZE_T nWrite;
	WriteProcessMemory(this, pDest, pSrc, nSize, &nWrite);
	return nWrite;
}
size_t win::Process::read(void * pDest, const void *pSrc, size_t nSize) noexcept
{
	SIZE_T nRead;
	ReadProcessMemory(this, pSrc, pDest, nSize, &nRead);
	return nRead;
}

const irect Monitor::getWorkArea() noexcept
{
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	BOOL GetMonitorInfoResult = GetMonitorInfo(this, &info);
	_assert(GetMonitorInfoResult);
	return (irect&)info.rcWork;
}
const irect Monitor::getMonitorArea() noexcept
{
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	BOOL GetMonitorInfoResult = GetMonitorInfo(this, &info);
	_assert(GetMonitorInfoResult);
	return (irect&)info.rcMonitor;
}
