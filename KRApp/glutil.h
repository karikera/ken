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
		bool makeCurrent() noexcept;
		void shareContant() noexcept;
		static GLContext* getInstance() noexcept;

		GLint maxTextureSize;

	private:
		EGLContext m_eglContext;
		EGLSurface m_eglSurface;
		EGLDisplay m_eglDisplay;
	};
}

#ifdef NDEBUG
#define glCheck() do {} while(false)
#else 
#define glCheck() do {GLenum error = glGetError(); _assert(error == 0); } while(false)
#endif

