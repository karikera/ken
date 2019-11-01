#ifdef WIN32

#define C2D_PRIVATE
#pragma comment(lib,"gdiplus.lib")
#include "2d.h"

#include <KR3/util/wide.h>

#define gplog(ex) ([&]()->bool{ Gdiplus::Status s = (ex); if(s == Gdiplus::Ok){ return true; } std::cerr << #ex << ": Status: " << s << std::endl;  return false; })()

kr::gl::Context::Context() noexcept
{
	Gdiplus::GdiplusStartupInput si;
	Gdiplus::GdiplusStartupOutput so;
	gplog(Gdiplus::GdiplusStartup((ULONG_PTR*)&m_token, &si, &so));
}
kr::gl::Context::~Context() noexcept
{
	Gdiplus::GdiplusShutdown(m_token);
}

kr::gl::Image::Image(int width, int height) noexcept
	:m_bitmap(width, height, PixelFormat32bppARGB)
{
}
kr::gl::Image::~Image() noexcept
{
}

void kr::gl::Image::getBits(void * dest) noexcept
{
	Gdiplus::BitmapData data;
	int width = m_bitmap.GetWidth();
	int height = m_bitmap.GetHeight();
	Gdiplus::Rect rc(0,0, width, height);
	if (gplog(m_bitmap.LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data)))
	{
		std::uint32_t * s = (std::uint32_t*)data.Scan0;
		std::uint32_t * d = (std::uint32_t*)dest;

		int srcnext = -width * 2 * sizeof(std::uint32_t);
		int destnext = width  * sizeof(std::uint32_t);
		std::uint32_t * dend = d + width * height;	

		while (d != dend)
		{
			std::uint32_t v = *s++;
			*d++ = (v & 0xff00ff00) | ((v >> 16) & 0xff) | ((v & 0xff) << 16);
		}

		m_bitmap.UnlockBits(&data);
	}
}

kr::gl::ImageCanvas::ImageCanvas(int width, int height) noexcept
	:Image(width, height), m_g(&m_bitmap)
	, m_pen(Gdiplus::Color::Black)
	, m_brush(Gdiplus::Color::White)
{
	m_font = new Gdiplus::Font(L"¸¼Àº °íµñ", 12.f);
}
kr::gl::ImageCanvas::~ImageCanvas() noexcept
{
	delete m_font;
}

void kr::gl::ImageCanvas::setFont(Text name, float size) noexcept
{
	delete m_font;
	m_font = new Gdiplus::Font(wide((pcstr16)(TSZ16)(AnsiToUtf16)name), size);
}
void kr::gl::ImageCanvas::setComposite(Composite comp) noexcept
{
	switch (comp)
	{
	case Composite::Copy:
		m_g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
		break;
	case Composite::Over:
		m_g.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
		break;
	}
}
void kr::gl::ImageCanvas::setStrokeColor(std::uint32_t color) noexcept
{
	m_pen.SetColor(color);
}
void kr::gl::ImageCanvas::setLineWidth(float width) noexcept
{
	m_pen.SetWidth(width);
}
void kr::gl::ImageCanvas::setFillColor(std::uint32_t color) noexcept
{
	m_brush.SetColor(color);
}
void kr::gl::ImageCanvas::fillRect(const frectwh &rect) noexcept
{
	m_g.FillRectangle(&m_brush, (Gdiplus::RectF&)rect);
}
void kr::gl::ImageCanvas::strokeRect(const frectwh &rect) noexcept
{
	m_g.DrawRectangle(&m_pen, (Gdiplus::RectF&)rect);
}
void kr::gl::ImageCanvas::fillOval(const frectwh &rect) noexcept
{
	m_g.FillEllipse(&m_brush, (Gdiplus::RectF&)rect);
}
void kr::gl::ImageCanvas::strokeOval(const frectwh &rect) noexcept
{
	m_g.DrawEllipse(&m_pen, (Gdiplus::RectF&)rect);
}
void kr::gl::ImageCanvas::fillText(Text text, vec2 pt) noexcept
{
	fillText((TText16)(AnsiToUtf16)text, pt);
}
void kr::gl::ImageCanvas::fillText(Text16 text, vec2 pt) noexcept
{
	m_g.DrawString(wide(text.begin()), intact<int>(text.size()), m_font, (Gdiplus::PointF&)pt, &m_brush);
}
void kr::gl::ImageCanvas::fillText(pcstr text, vec2 pt) noexcept
{
	fillText((Text)text, pt);
}
void kr::gl::ImageCanvas::fillText(pcstr16 text, vec2 pt) noexcept
{
	m_g.DrawString(wide(text), -1, m_font, (Gdiplus::PointF&)pt, &m_brush);
}
void kr::gl::ImageCanvas::fillPoly(const vec2 * poly, std::size_t count) noexcept
{
	_assert(count <= 0x7fffffff);
	m_g.FillPolygon(&m_brush, (Gdiplus::PointF*)poly, (int)count);
}

#else

#include <KR3/main.h>
EMPTY_SOURCE

#endif