#include "gl.h"

#pragma comment(lib, "libEGL.lib")
#pragma comment(lib, "libGLESv2.lib")

using namespace kr;

void WebCanvasGL::_flush() noexcept
{
	m_ctx.swap();
}

WebCanvasGL::WebCanvasGL() noexcept
{
}
WebCanvasGL::~WebCanvasGL() noexcept
{
}
void WebCanvasGL::create(int width, int height) noexcept
{
	Application::create(width, height);
	m_ctx.create(m_display, m_window);
}

void WebCanvasGL::onResize(int width, int height) noexcept
{
	glViewport(0, 0, width, height);
}
