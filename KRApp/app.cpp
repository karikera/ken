#define __KR_APP_FLUSHER
#include "app.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <GLES2/gl2.h>


using namespace kr;

namespace
{
	Application * s_main;
#ifndef __EMSCRIPTEN__
	class DrawInterval:public TimerEvent
	{
	public:
		DrawInterval() noexcept
			:TimerEvent(timepoint())
		{
			AddRef();
		}
		void start() noexcept
		{
			m_at = timepoint::now();
			s_main->m_pump->attach(this);
		}
		void post() noexcept
		{
			m_at += 16_ms;
			timepoint now = timepoint::now();
			if (m_at < now) m_at = now;
			EventPump::getInstance()->attach(this);
		}
		void call() noexcept override
		{
			s_main->onDraw();
			s_main->_flush();
			post();
		}
	};
	DrawInterval s_drawInterval;
#endif
}

#ifdef __EMSCRIPTEN__
void ::kr::_pri_::emOndrawCallback() noexcept
{
	s_main->onDraw();
	PromisePump::getInstance()->process();
}
#endif

void ::kr::_pri_::setApplication(Application * proc) noexcept
{
	s_main = proc;

#ifdef __EMSCRIPTEN__
#elif defined(WIN32)
	s_drawInterval.start();
	s_main->m_pump->messageLoop();
#else
#error Need implement
#endif
}

#ifdef WIN32
#include <KRWin/handle.h>
#include <KRWin/winx.h>
#include <KRWin/gdi.h>
#endif

#ifdef __EMSCRIPTEN__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif
using namespace kr;


namespace
{
#ifdef __EMSCRIPTEN__
	int getScreenWidth() noexcept
	{
		return EM_ASM_INT_V({ return window.innerWidth });
	}
	int getScreenHeight() noexcept
	{
		return EM_ASM_INT_V({ return window.innerHeight });
	}
#endif
}

Application::Application() noexcept
	:
#ifdef __EMSCRIPTEN__
	m_pump(PromisePump::getInstance())
#else
	m_pump(EventPump::getInstance())
#endif
{
}
Application::~Application() noexcept
{
#ifdef __EMSCRIPTEN__
#else
	win::Window * wnd = (win::Window*)(m_window);
	wnd->releaseDC((win::DrawContext*)(m_display));
	delete wnd;
#endif
}
void Application::create(int width, int height) noexcept
{
#ifdef __EMSCRIPTEN__
	m_width = width = getScreenWidth();
	m_height = height = getScreenHeight();
	emscripten_set_canvas_element_size("#canvas", width, height);
	Display * display = (Display*)((_XPrivDisplay *)0 + 1); // XOpenDisplay(nullptr);
	_assert(display != nullptr);

	Window root = DefaultRootWindow(display);

	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

	Window win = XCreateWindow(
		display, root,
		0, 0, width, height, 0,
		CopyFromParent, InputOutput,
		CopyFromParent, CWEventMask,
		&swa);

	XSetWindowAttributes  xattr;
	xattr.override_redirect = false;
	XChangeWindowAttributes(display, win, CWOverrideRedirect, &xattr);

	XWMHints hints;
	hints.input = true;
	hints.flags = InputHint;
	XSetWMHints(display, win, &hints);

	// make the window visible on the screen
	XMapWindow(display, win);
	XStoreName(display, win, "test");

	// f identifiers for the provided atom name strings
	Atom wm_state = XInternAtom(display, "_NET_WM_STATE", false);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = false;
	XSendEvent(
		display,
		DefaultRootWindow(display),
		false,
		SubstructureNotifyMask,
		&xev);

	m_display = (EGLNativeDisplayType)display;
	m_window = (EGLNativeWindowType)win;

#ifdef __EMSCRIPTEN__
	emscripten_set_keydown_callback("#window", this, false, [](int eventType, const EmscriptenKeyboardEvent *keyEvent, void *webcanvas)->EM_BOOL {
		Application* app = (Application*)webcanvas;
		app->onKeyDown(keyEvent->keyCode, keyEvent->repeat);
		app->m_pump->process();
		return true;
	});
	emscripten_set_keyup_callback("#window", this, false, [](int eventType, const EmscriptenKeyboardEvent *keyEvent, void *webcanvas)->EM_BOOL {
		Application* app = (Application*)webcanvas;
		app->onKeyUp(keyEvent->keyCode);
		app->m_pump->process();
		return true;
	});
	emscripten_set_mousedown_callback("#window", this, false, [](int eventType, const EmscriptenMouseEvent *keyEvent, void *webcanvas)->EM_BOOL {
		Application* app = (Application*)webcanvas;
		app->onMouseDown(keyEvent->canvasX, keyEvent->canvasY, keyEvent->button);
		app->m_pump->process();
		return true;
	});
	emscripten_set_mouseup_callback("#window", this, false, [](int eventType, const EmscriptenMouseEvent *keyEvent, void *webcanvas)->EM_BOOL {
		Application* app = (Application*)webcanvas;
		app->onMouseMove(keyEvent->canvasX, keyEvent->canvasY);
		app->m_pump->process();
		return true;
	});
	emscripten_set_mousemove_callback("#window", this, false, [](int eventType, const EmscriptenMouseEvent *keyEvent, void *webcanvas)->EM_BOOL {
		Application* app = (Application*)webcanvas;
		app->onMouseUp(keyEvent->canvasX, keyEvent->canvasY, keyEvent->button);
		app->m_pump->process();
		return true;
	});
	emscripten_set_resize_callback("#window", this, false,
		[](int eventType, const EmscriptenUiEvent *uiEvent, void *webcanvas)->EM_BOOL {
		int width = uiEvent->windowInnerWidth;
		int height = uiEvent->windowInnerHeight;
		emscripten_set_canvas_element_size("#canvas", width, height);
		Application * app = ((Application*)webcanvas);
		app->m_width = width;
		app->m_height = height;
		app->onResize(width, height);
		app->m_pump->process();
		return true;
	});
#endif

#elif defined(WIN32)

	static const char16 WC_CANVAS[] = u"KRCANVAS";
	WindowClass wndcls(WC_CANVAS, [](win::Window* wnd, UINT msg, WPARAM wParam, LPARAM lParam)->LRESULT {
		switch (msg)
		{
		case WM_NCCREATE:
			wnd->setLongPtr(GWLP_USERDATA, (intptr_t)((CREATESTRUCTW*)lParam)->lpCreateParams);
			return DefWindowProcW(wnd, msg, wParam, lParam);
		case WM_KEYDOWN: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onKeyDown((int)wParam, (HIWORD(lParam) & KF_REPEAT) != 0);
			break;
		}
		case WM_KEYUP: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onKeyUp((int)wParam);
			break;
		}
		case WM_MOUSEMOVE: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onMouseMove((short)LOWORD(lParam), (short)HIWORD(lParam));
			break;
		}
		case WM_LBUTTONDOWN: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			wnd->setCapture();
			app->onMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam), 0);
			break;
		}
		case WM_MBUTTONDOWN: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam), 1);
			break;
		}
		case WM_RBUTTONDOWN: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam), 2);
			break;
		}
		case WM_LBUTTONUP: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			wnd->releaseCapture();
			app->onMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam), 0);
			break;
		}
		case WM_MBUTTONUP: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam), 1);
			break;
		}
		case WM_RBUTTONUP: {
			Application* app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->onMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam), 2);
			break;
		}
		case WM_ENTERSIZEMOVE:
			//wnd->setTimer(1, __emscripten_timer, [](HWND, UINT, UINT_PTR, DWORD) {
			//	__emscripten_main();
			//});
			break;
		case WM_EXITSIZEMOVE:
			//wnd->killTimer(1);
			break;

		case WM_DESTROY: 
			s_main->m_pump->cancel(&s_drawInterval);
			PostQuitMessage(0);
			break;
		case WM_SIZE: {
			Application * app = ((Application*)wnd->getLongPtr(GWLP_USERDATA));
			app->m_width = LOWORD(lParam);
			app->m_height = HIWORD(lParam);
			app->onResize(LOWORD(lParam), HIWORD(lParam));
			break;
		}
		default: return DefWindowProcW(wnd, msg, wParam, lParam);
		}
		return 0;
	}, (HICON)nullptr);
	wndcls.registerClass();

	win::Window * wnd = win::Window::createPrimary(WC_CANVAS, u"", 
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_DLGFRAME, nullptr, this);
	wnd->resizeToCenter(width, height);
	wnd->show(SW_SHOW);

	m_width = width;
	m_height = height;

	m_window = wnd;
	m_display = wnd->getDC();

#endif
}
int Application::getWidth() noexcept
{
	return m_width;
}
int Application::getHeight() noexcept
{
	return m_height;
}

void Application::swap() noexcept
{
}
void Application::onDraw() noexcept
{
}
void Application::onResize(int width, int height) noexcept
{
}
void Application::onKeyDown(int key, bool repeat) noexcept
{
}
void Application::onKeyUp(int key) noexcept
{
}
void Application::onMouseMove(int x, int y) noexcept
{
}
void Application::onMouseDown(int x, int y, int button) noexcept
{
}
void Application::onMouseUp(int x, int y, int button) noexcept
{
}


void kr::openDevTools() noexcept
{
	#ifdef __EMSCRIPTEN__
	EM_ASM({
		if (typeof require != = 'undefined')
		{
			require("nw.gui").Window.get().showDevTools();
		}
	});
	#endif
}
