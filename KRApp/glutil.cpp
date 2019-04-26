#include "glutil.h"

using namespace kr;

GLContext::GLContext() noexcept
{
}
GLContext::~GLContext() noexcept
{
}
void GLContext::create(EGLNativeDisplayType display, EGLNativeWindowType window) noexcept
{
	static EGLint attribList[] =
	{
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		EGL_DEPTH_SIZE,     24,
		EGL_STENCIL_SIZE,   8,
		EGL_NONE
	};

	// Get Display
	m_eglDisplay = eglGetDisplay(display);
	if (m_eglDisplay == EGL_NO_DISPLAY)
	{
		cerr << "eglGetDisplay failed" << endl;
		return;
	}

	// Initialize EGL
	EGLint majorVersion;
	EGLint minorVersion;
	if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion))
	{
		cerr << "eglInitialize failed" << endl;
		return;
	}

	// Get configs
	EGLint numConfigs;
	if (!eglGetConfigs(m_eglDisplay, nullptr, 0, &numConfigs))
	{
		cerr << "eglGetConfigs failed" << endl;
		return;
	}

	// Choose config
	EGLConfig config;
	if (!eglChooseConfig(m_eglDisplay, attribList, &config, 1, &numConfigs))
	{
		cerr << "eglChooseConfig failed" << endl;
		return;
	}

	if (numConfigs == 0)
	{
		cerr << "no available config" << endl;
		return;
	}

	// Create a surface
	m_eglSurface = eglCreateWindowSurface(m_eglDisplay, config, window, nullptr);
	if (m_eglSurface == EGL_NO_SURFACE)
	{
		cerr << "eglCreateWindowSurface failed" << endl;
		return;
	}

	// Create a GL context
	static EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
	if (m_eglContext == EGL_NO_CONTEXT)
	{
		cerr << "eglCreateContext failed" << endl;
		return;
	}

	// Make the context current
	if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext))
	{
		cerr << "eglMakeCurrent failed" << endl;
		return;
	}
}
void GLContext::swap() noexcept
{
	eglSwapBuffers(m_eglDisplay, m_eglSurface);
}
