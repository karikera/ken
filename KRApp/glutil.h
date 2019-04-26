#pragma once

#include <KR3/main.h>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace kr
{
	class GLContext
	{
	public:
		GLContext() noexcept;
		~GLContext() noexcept;
		void create(EGLNativeDisplayType display, EGLNativeWindowType window) noexcept;
		void swap() noexcept;

	private:
		EGLContext m_eglContext;
		EGLSurface m_eglSurface;
		EGLDisplay m_eglDisplay;
	};
}
