#pragma once

#include <KR3/main.h>
#include <KR3/util/time.h>
#ifdef WIN32
#include <KR3/msg/pump.h>
#endif
#include <KR3/msg/promise.h>
#include <emscripten.h>
#include <EGL/egl.h>

namespace kr
{
	class Application;

	namespace _pri_
	{
#ifdef __EMSCRIPTEN__
		void emOndrawCallback() noexcept;
#endif
		void setApplication(Application * proc) noexcept;
	}
	
	class Application
	{
	public:
		Application() noexcept;
		~Application() noexcept;
		void create(int width, int height) noexcept;
		int getWidth() noexcept;
		int getHeight() noexcept;

		virtual void swap() noexcept;
		virtual void onDraw() noexcept;
		virtual void onResize(int width, int height) noexcept;
		virtual void onKeyDown(int key, bool repeat) noexcept;
		virtual void onKeyUp(int key) noexcept;
		virtual void onMouseMove(int x, int y) noexcept;
		virtual void onMouseDown(int x, int y, int button) noexcept;
		virtual void onMouseUp(int x, int y, int button) noexcept;

#ifdef __KR_APP_FLUSHER
		virtual void _flush() noexcept = 0;
#else
	protected:
		virtual void _flush() noexcept = 0;
#endif

	protected:

		int m_width, m_height;
		EGLNativeWindowType m_window;
		EGLNativeDisplayType m_display;

#ifdef __KR_APP_FLUSHER
	public:
#endif
		EventPump* m_pump;
	};

	void openDevTools() noexcept;
}

#ifdef __EMSCRIPTEN__

#define main_loop(app) { \
		::kr::_pri_::setApplication(app); \
		emscripten_set_main_loop(::kr::_pri_::emOndrawCallback, 0, true); \
	} while(0,0);

#elif defined(WIN32)

#define main_loop(app) { \
		::kr::_pri_::setApplication(app); \
	} while(0,0);

#endif