#include "glutil.h"

using namespace kr;

namespace {
	thread_local GLContext* s_context = nullptr;
}

GLContext::GLContext() noexcept
{
	m_eglContext = nullptr;
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

	makeCurrent();

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
}
void GLContext::swap() noexcept
{
	eglSwapBuffers(m_eglDisplay, m_eglSurface);
}
bool GLContext::makeCurrent() noexcept {
	// Make the context current
	if (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
		s_context = this;
		return true;
	}
	else {
		cerr << "eglMakeCurrent failed" << endl;
		return false;
	}
}
void GLContext::shareContant() noexcept {
	_assert(m_eglContext != nullptr);
	s_context = this;
}

GLContext* GLContext::getInstance() noexcept {
	_assert(s_context != nullptr && s_context->m_eglContext != nullptr);
	return s_context;
}
