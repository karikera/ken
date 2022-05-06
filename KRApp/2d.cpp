#ifdef WIN32

#include <windows.h>
#include <GdiPlus.h>

#include "2d.h"

#include <KR3/initializer.h>
#include <KRWin/gdi.h>
#include <KR3/msg/msgloop.h>
#include <KR3/util/wide.h>

#include <assert.h>
#include <iostream>

#include <d2d1.h>

using namespace kr;
using namespace win;

using GPPen = Gdiplus::Pen;
using GPBrush = Gdiplus::SolidBrush;
using GPFont = Gdiplus::Font;
using GPFontFamily = Gdiplus::FontFamily;
using GPBitmap = Gdiplus::Bitmap;
using GPColor = Gdiplus::Color;
using GPMatrix = Gdiplus::Matrix;

namespace
{
	struct State:Node<State, true>
	{
		dword penColor;
		float penWidth;
		GPPen pen;

		dword brushColor;
		GPBrush brush;

		AText16 fontName;
		GPFontFamily fontFamily;
		float fontAscent;
		float fontHeight;
		GPFont font;

		GPMatrix matrix;

		State() noexcept;
		~State() noexcept;
		explicit State(const State & clone) noexcept;
		State & operator = (const State & clone) noexcept;
		void setPenColor(dword color) noexcept;
		void setPenWidth(float width) noexcept;
		void setBrushColor(dword color) noexcept;
		float getFontAscent() noexcept;
	};

	enum class PathCommand
	{
		MoveTo,
		LineTo
	};
	
	struct GraphicContext
	{
		struct BitmapAndDrawContext
		{
			int width, height;
			kr::win::Bitmap * bitmap;
			DrawContext * dc;

			BitmapAndDrawContext(DrawContext * dc, Window * wnd) noexcept
			{
				irect rc = wnd->getClientRect();
				width = rc.width();
				height = rc.height();
				bitmap = dc->createCompatibleBitmap(width, height);
				this->dc = dc->createCompatibleDC();
				this->dc->select(bitmap);
			}
		};
		Window * wnd;
		DrawContext * dc;
		BitmapAndDrawContext back;
		Gdiplus::Graphics g;

		State * state;
		LinkedList<State> states;
		Gdiplus::GraphicsPath path;
		float pathStartX, pathStartY;
		float pathX, pathY;

		GPBrush clearColor;

		bool needFlush;
		timepoint waitTo;

		GraphicContext(Window* wnd) noexcept
			:clearColor(GPColor::White)
			, needFlush(true)
			, dc(wnd->getDC())
			, back(dc, wnd)
			, g(back.dc)
		{
			state = states.create();
			waitTo = timepoint::now() + 16_ms;

			g.FillRectangle(&clearColor, 0, 0, back.width, back.height);
			g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		}
		~GraphicContext() noexcept
		{
			delete back.bitmap;
			delete back.dc;
			wnd->releaseDC(dc);
		}

		float getFontAscent() noexcept
		{
			return state->getFontAscent();
		}
		void flush() noexcept
		{
			timepoint now = timepoint::now();
			if (waitTo - now <= -20_ms) {
				waitTo = now + 16_ms;
				EventPump::getInstance()->processOnce();
			}
			else {
				EventPump::getInstance()->sleepTo(waitTo);
				waitTo += 16_ms;
			}

			if (!needFlush) return;
			needFlush = false;
			dc->bitBlt(back.dc, {0, 0, back.width, back.height}, { 0, 0 });
		}
	};

	inline dword argbToBgr(dword argb) noexcept
	{
		dword rgb = (argb >> 16) & 0xff;
		rgb |= argb & 0xff00;
		rgb |= (argb & 0xff) << 16;
		return rgb;
	}
	static Manual<GraphicContext> s_g;

	State::~State() noexcept
	{
	}
	State::State() noexcept
		:penColor(GPColor::Black),
		penWidth(1.f),
		pen(penColor, penWidth),
		brushColor(GPColor::Black),
		brush(brushColor),
		fontName(AText16::concat(u"sans-serif", nullterm)),
		fontFamily(wide(fontName.data())),
		fontHeight(10.f),
		fontAscent(-1.f),
		font(&fontFamily, fontHeight)
	{
	}
	State::State(const State & clone) noexcept
		:penColor(clone.penColor),
		penWidth(clone.penWidth),
		pen(clone.penColor, clone.penWidth),
		brushColor(clone.brushColor),
		brush(clone.brushColor),
		fontName(AText16::concat(clone.fontName, nullterm)),
		fontFamily(wide(clone.fontName.data())),
		fontHeight(clone.fontHeight),
		fontAscent(clone.fontAscent),
		font(&fontFamily, fontHeight)
	{
		pen.SetStartCap(clone.pen.GetStartCap());
		pen.SetEndCap(clone.pen.GetEndCap());
		float m[6];
		clone.matrix.GetElements(m);
		matrix.SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
	}
	State & State::operator = (const State & clone) noexcept
	{
		this->~State();
		new(this) State(clone);
		return *this;
	}
	void State::setPenColor(dword color) noexcept
	{
		if (penColor == color) return;
		penColor = color;
		pen.SetColor(color);
	}
	void State::setPenWidth(float width) noexcept
	{
		if (penWidth == width) return;
		penWidth = width;
		pen.SetWidth(width);
	}
	void State::setBrushColor(dword color) noexcept
	{
		if (brushColor == color) return;
		brushColor = color;
		brush.SetColor(color);
	}
	float State::getFontAscent() noexcept
	{
		if (fontAscent == -1.f)
		{
			uint ascent = fontFamily.GetCellAscent(Gdiplus::FontStyleRegular);
			fontAscent = fontHeight * ascent / fontFamily.GetEmHeight(Gdiplus::FontStyleRegular);
		}
		return fontAscent;
	}
}

#elif defined(__EMSCRIPTEN__)
#include "2d.h"
#include <emscripten.h>
using namespace kr;
#endif

void WebCanvas2D::_flush() noexcept
{
#ifdef __EMSCRIPTEN__
#elif defined(WIN32)
	s_g->flush();
#endif
}

WebCanvas2D::WebCanvas2D() noexcept
{
}
WebCanvas2D::WebCanvas2D(int width, int height) noexcept
{
	create(width, height);
}
WebCanvas2D::~WebCanvas2D() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM({
		delete window.ctx;
		Module.canvas = null;
	});
#elif defined(WIN32)
	s_g.remove();
#endif
}

void WebCanvas2D::create(int width, int height) noexcept
{
	Application::create(width, height);

#ifdef __EMSCRIPTEN__
	int succeeded = EM_ASM_INT({
		if ("ctx" in window) return 0;
		document.body.appendChild(Module.canvas);

		window.ctx = Module.canvas.getContext("2d");
		return 1;
		}, 0);
	_assert(succeeded);
#elif defined(WIN32)
	static kr::Initializer<kr::GdiPlus> _init_gdip;
	staticCode
	{
		MessageLoop::getInstance()->attachBasicTranslator();
	};

	s_g.create((Window*)m_window);
#endif
}

WebCanvas2D::Composite& WebCanvas2D::Composite::operator =(const char * str) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_( ctx.globalCompositeOperation = UTF8ToString($0); ,str);
#elif defined(WIN32)

#endif
	return *this;
}
WebCanvas2D::StrokeStyle& WebCanvas2D::StrokeStyle::operator =(uint32_t style) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(
		ctx.strokeStyle = "rgba("+ $0+ "," + $1 + "," + $2 + "," + $3 +")";,
		((style >> 16) & 0xff),
		((style >> 8) & 0xff),
		((style >> 0) & 0xff),
		((style >> 24) & 0xff) / 255);
#elif defined(WIN32)
	s_g->state->setPenColor(style);
#endif
	return *this;
}
WebCanvas2D::LineWidth& WebCanvas2D::LineWidth::operator =(float width) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(
		ctx.lineWidth = $0;,
		width);
#elif defined(WIN32)
	s_g->state->setPenWidth(width);
#endif
	return *this;
}
WebCanvas2D::FillStyle& WebCanvas2D::FillStyle::operator =(uint32_t style) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(
		ctx.fillStyle = "rgba(" + $0 + "," + $1 + "," + $2 + "," + $3 + ")";,
		((style >> 16) & 0xff),
		((style >> 8) & 0xff),
		((style >> 0) & 0xff),
		((style >> 24) & 0xff) / 255);
#elif defined(WIN32)
	s_g->state->setBrushColor(style);
#endif
	return *this;
}

void WebCanvas2D::save() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.save(); );
#elif defined(WIN32)
	s_g->state = s_g->states.create(*s_g->state);
#endif
}
void WebCanvas2D::restore() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.restore(); );
#elif defined(WIN32)
	s_g->states.pop();
	s_g->state = s_g->states.back();
	s_g->g.SetTransform(&s_g->state->matrix);
#endif
}
void WebCanvas2D::translate(float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.translate($0, $1), x, y);
#elif defined(WIN32)
	s_g->state->matrix.Translate(x, y, Gdiplus::MatrixOrderPrepend);
	s_g->g.SetTransform(&s_g->state->matrix);
#endif
}
void WebCanvas2D::rotate(float radian) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.rotate($0) , radian);
#else
	s_g->state->matrix.Rotate(radian, Gdiplus::MatrixOrderPrepend);
	s_g->g.SetTransform(&s_g->state->matrix);
#endif
}
void WebCanvas2D::rect(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.rect($0, $1, $2, $3) , x, y, width, height);
#elif defined(WIN32)
#endif
}
void WebCanvas2D::oval(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.oval($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
#endif
}
void WebCanvas2D::clearRect(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.clearRect($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
	s_g->needFlush = true;
	// s_g->g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	s_g->g.FillRectangle(&s_g->clearColor, x, y, width, height);
	// s_g->g.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
#endif
}
void WebCanvas2D::strokeRect(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.strokeRect($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
	s_g->needFlush = true;
	s_g->g.DrawRectangle(&s_g->state->pen, x, y, width, height);
#endif
}
void WebCanvas2D::fillRect(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.fillRect($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
	s_g->needFlush = true;
	s_g->g.FillRectangle(&s_g->state->brush, x, y, width, height);
#endif
}
void WebCanvas2D::strokeOval(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.strokeOval($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
	s_g->needFlush = true;
	s_g->g.DrawEllipse(&s_g->state->pen, x, y, width, height);
#endif
}
void WebCanvas2D::fillOval(float x, float y, float width, float height) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.fillOval($0,$1,$2,$3), x, y, width, height);
#elif defined(WIN32)
	s_g->needFlush = true;
	s_g->g.FillEllipse(&s_g->state->brush, x, y, width, height);
#endif
}
void WebCanvas2D::fillText(Text text, float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.fillText(Pointer_stringify($0, $1),$2,$3), text.data(), text.size(), x, y);
#elif defined(WIN32)
	s_g->needFlush = true;
	TText16 text16;
	text16 << utf8ToUtf16(text);
	s_g->g.DrawString(wide(text16.data()), intact<int>(text16.size()), &s_g->state->font, {x, y}, &s_g->state->brush);
#endif
}
void WebCanvas2D::fillText(Text16 text, float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_(ctx.fillText(emUtf16ToStr($0, $1), $2, $3), text.data(), text.size(), x, y);
#elif defined(WIN32)
	s_g->needFlush = true;
	s_g->g.DrawString(wide(text.data()), intact<int>(text.size()), &s_g->state->font, { x, y }, &s_g->state->brush);
#endif
}
void WebCanvas2D::drawImage(WebImage * image, float x, float y) noexcept
{
}
void WebCanvas2D::drawImage(WebImage * image, float x, float y, float width, float height) noexcept
{
}
void WebCanvas2D::drawImage(WebImage * image, float srcX, float srcY, float srcWidth, float srcHeight, float destX, float destY, float destWidth, float destHeight) noexcept
{
}
bool WebCanvas2D::putImageData(const image::ImageData& image, int x, int y) noexcept
{
	if (image.getPixelFormat() != PixelFormatABGR8) return false;
	int width = image.getWidth();
	int height = image.getHeight();
	int pitch = image.getPitch();
#ifdef __EMSCRIPTEN__
	if (width * 4 != pitch)
	{
		EM_ASM_(
			var buffer = new Uint8ClampedArray(Module.HEAPU8.buffer, $0, numBytes);
			var imgdata = new ImageData(buffer, $1, $2);
			ctx.putImageData(imgdata, $3, $4);
		, image.getBits(), width, height, x, y);
	}
	else
	{
		if (pitch % 4 != 0) return false;
		EM_ASM_(
			var buffer = new Uint8ClampedArray(Module.HEAPU8.buffer, $0, numBytes);
			var imgdata = new ImageData(buffer, $1, $2);
			ctx.putImageData(imgdata, $3, $4, 0, 0, $5, $2);
		, image.getBits(), pitch >> 2, height, x, y, width);
	}
#elif defined(WIN32)
	if (pitch % 4 != 0) return false;
	s_g->needFlush = true;
	TmpArray<dword> data;
	data.resize(width * height);
	
	dword * dest = data.data();
	dword * dest_end = data.end();
	const dword * src = (const dword *)image.getBits();
	size_t destpitch = width * 4;
	size_t srcpitch = image.getPitch();
	size_t src_nextline = srcpitch + width * 4;

	src = (dword*)((byte*)src + (height - 1) * srcpitch);

	while (dest != dest_end)
	{
		dword * dest_nextline = (dword*)((byte*)dest + destpitch);
		while (dest != dest_nextline)
		{
			dword v = *src++;
			*dest++ = ((v & 0xff) << 16) | ((v & 0xff0000) >> 16) | (v & 0xff00ff00);
		}
		src = (dword*)((byte*)src - src_nextline);
	}
	BITMAPINFO info;
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	SetDIBitsToDevice(s_g->back.dc, x, y, width, height, 0, 0, 0, height, data.data(), &info, DIB_RGB_COLORS);
#endif
	return true;
}

void WebCanvas2D::beginPath() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.beginPath(); );
#elif defined(WIN32)
	s_g->path.Reset();
#endif
}
void WebCanvas2D::closePath() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.closePath(); );
#elif defined(WIN32)
	lineTo(s_g->pathStartX, s_g->pathStartY);
#endif
}
void WebCanvas2D::stroke() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.stroke(); );
#elif defined(WIN32)
	s_g->g.DrawPath(&s_g->state->pen, &s_g->path);
	s_g->needFlush = true;
#endif
}
void WebCanvas2D::fill() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.fill(); );
#elif defined(WIN32)
	s_g->g.FillPath(&s_g->state->brush, &s_g->path);
	s_g->needFlush = true;
#endif
}
void WebCanvas2D::clip() noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM(ctx.clip(); );
#elif defined(WIN32)
	// s_g->g.SetClip(&s_g->path, Gdiplus::CombineModeUnion);
#endif
}
void WebCanvas2D::moveTo(float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_({ ctx.moveTo($0,$1); }, x, y);
#elif defined(WIN32)
	s_g->pathStartX = x;
	s_g->pathStartY = y;
	s_g->pathX = x;
	s_g->pathY = y;
#endif
}
void WebCanvas2D::lineTo(float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_({ ctx.lineTo($0,$1); }, x, y);
#elif defined(WIN32)
	s_g->path.AddLine(s_g->pathStartX, s_g->pathStartY, x, y);
	s_g->pathX = x;
	s_g->pathY = y;
#endif
}
void WebCanvas2D::arc(float x, float y, float radius, float startAngle, float endAngle, bool counterClockwise) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_({ ctx.src($0,$1,$2,$3,$4,$5); }
		, x, y, radius, startAngle, endAngle, counterClockwise);
#elif defined(WIN32)
	float sx = cos(startAngle) * radius + x;
	float sy = sin(startAngle) * radius + y;
	float ex = cos(endAngle) * radius + x;
	float ey = sin(endAngle) * radius + y;
	s_g->path.AddLine(s_g->pathStartX, s_g->pathStartY, sx, sy);
	float radius2 = radius * 2.f;
	if (counterClockwise)
	{
	}
	s_g->path.AddArc(x - radius, y - radius, radius2, radius2, startAngle, endAngle - startAngle);
	s_g->pathX = ex;
	s_g->pathY = ey;
#endif
}
void WebCanvas2D::quadraticCurveTo(float cpx, float cpy, float x, float y) noexcept
{
#ifdef __EMSCRIPTEN__
	EM_ASM_( ctx.quadraticCurveTo($0,$1,$2,$3); , cpx, cpy, x, y);
#elif defined(WIN32)
	cpx *= 2 / 3.f;
	cpy *= 2 / 3.f;
	float sx = s_g->pathX;
	float sy = s_g->pathY;
	s_g->path.AddBezier(
		sx, sy,
		cpx + sx / 3.f, cpy + sy / 3.f, 
		cpx + x / 3.f, cpy + y / 3.f, 
		x, y);
	s_g->pathX = x;
	s_g->pathY = y;
#endif
}
void WebCanvas2D::exRoundRect(float x, float y, float width, float height, float radius) noexcept
{
	beginPath();
	moveTo(x + radius, y);
	lineTo(x + width - radius, y);
	quadraticCurveTo(x + width, y, x + width, y + radius);
	lineTo(x + width, y + height - radius);
	quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
	lineTo(x + radius, y + height);
	quadraticCurveTo(x, y + height, x, y + height - radius);
	lineTo(x, y + radius);
	quadraticCurveTo(x, y, x + radius, y);
	closePath();
}
