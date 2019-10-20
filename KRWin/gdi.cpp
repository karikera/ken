#include "stdafx.h"
#include "gdi.h"
#include <KR3/util/bitmap.h>

#include <KR3/wl/windows.h>

#pragma comment(lib, "msimg32.lib")

using namespace kr;
using namespace win;

#pragma warning(disable:4800)

namespace
{
	Font* defaultFont = nullptr;

}

DefaultFont& DefaultFont::operator =(Font * font) noexcept
{
	defaultFont = font;
	return *this;
}
DefaultFont::operator Font*() noexcept
{
	if(defaultFont == nullptr) defaultFont = Font::createSystemFont();
	return defaultFont;
}
Font* DefaultFont::operator ->() noexcept
{
	return defaultFont;
}

void kr::win::_pri_::deleteObject(ptr handle) noexcept
{
	DeleteObject((HGDIOBJ)handle);
}
ptr kr::win::_pri_::getStockObject(int id) noexcept
{
	return GetStockObject(id);
}

RGN* RGN::createRoundRect(const irect & rc,int rx,int ry) noexcept
{
	return (RGN*)::CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,rx,ry);
}
RGN* RGN::createRect(const irect & rc) noexcept
{
	return (RGN*)::CreateRectRgn(rc.left,rc.top,rc.right,rc.bottom);
}

Pen* Pen::create(COLORREF color,int width,int ps)
{
	return (Pen*)CreatePen(ps,width,color);
}

Brush* Brush::create(COLORREF color)
{
	return (Brush*)CreateSolidBrush(color);
}
Brush* Brush::create(byte r,byte g,byte b)
{
	return (Brush*)CreateSolidBrush(RGB(r,g,b));
}
Brush* Brush::fromSystem(int nIndex)
{
	return (Brush*)GetSysColorBrush(nIndex);
}

Font* Font::create(const char * name, int height, int weight) noexcept
{
	return (Font*)CreateFontA(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, name);
}
Font* Font::create(const char16 * name, int height, int weight) noexcept
{
	return (Font*)CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, wide(name));
}
Font* Font::create(const LOGFONTW & font) noexcept
{
	return (Font*)CreateFontIndirectW(&font);
}
Font* Font::create(const LOGFONTA & font) noexcept
{
	return (Font*)CreateFontIndirectA(&font);
}
Font* Font::createSystemFont() noexcept
{
	NONCLIENTMETRICSW met;
	met.cbSize = sizeof(met);
	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(met), &met, 0))
	{
		return Font::getStock(DEVICE_DEFAULT_FONT);
	}
	return Font::create(met.lfMessageFont);
}
Font* Font::createSystemFont(int height) noexcept
{
	NONCLIENTMETRICSW met;
	met.cbSize = sizeof(met);
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(met), &met, 0))
	{
		LOGFONTW & font = met.lfMessageFont;
		font.lfCharSet = DEFAULT_CHARSET;
		font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		font.lfEscapement = 0;
		font.lfHeight = height;
		font.lfItalic = false;
		font.lfOrientation = 0;
		font.lfOutPrecision = OUT_DEFAULT_PRECIS;
		font.lfPitchAndFamily = DEFAULT_PITCH;
		font.lfQuality = DEFAULT_QUALITY;
		font.lfStrikeOut = false;
		font.lfUnderline = false;
		font.lfWeight = FW_NORMAL;
		font.lfWidth = 0;
	}
	met.lfMessageFont.lfHeight = height;
	return Font::create(met.lfMessageFont);
}
bool Font::getObject(LOGFONTA * lf) noexcept
{
	return (bool)GetObjectA(this, sizeof(LOGFONTA), lf);
}
bool Font::getObject(LOGFONTW * lf) noexcept
{
	return (bool)GetObjectW(this, sizeof(LOGFONTW), lf);
}
int Font::getHeight() noexcept
{
	LOGFONTW lf;
	bool getObjectResult = getObject(&lf);
	_assert(getObjectResult);
	return lf.lfHeight;
}
void Font::getName(Writer16 * writer) noexcept
{
	LOGFONTW lf;
	bool getObjectResult = getObject(&lf);
	_assert(getObjectResult);
	*writer << (Text16)(pcstr16)lf.lfFaceName;
}

Bitmap* Bitmap::create(HDC hDC,int w,int h) noexcept
{
	return (Bitmap*)CreateCompatibleBitmap(hDC,w,h);
}
Bitmap * Bitmap::load(const char* strFileName,uint fuLoad) noexcept
{
	return (Bitmap*)LoadImageA(nullptr, strFileName, IMAGE_BITMAP, 0,0,fuLoad);
}
Bitmap * Bitmap::load(const char16* strFileName,uint fuLoad) noexcept
{
	return (Bitmap*)LoadImageW(nullptr, wide(strFileName), IMAGE_BITMAP, 0,0,fuLoad);
}

bool Bitmap::getObject(BITMAP *bmp) noexcept
{
	return (bool)::GetObjectW(this, sizeof(BITMAP), bmp);
}


DrawContext * DrawContext::get(win::Window* wnd) noexcept
{
	return (DrawContext*)::GetDC(wnd);
}
DrawContext * DrawContext::create() noexcept
{
	return (DrawContext*)::CreateDC(L"DISPLAY", nullptr, nullptr, nullptr);
}
void DrawContext::operator delete(void * p) noexcept
{
	DeleteDC((HDC)p);
}
DrawContext * DrawContext::createCompatibleDC() noexcept
{
	return (DrawContext*)::CreateCompatibleDC(this);
}
Bitmap * DrawContext::createCompatibleBitmap(int nWidth,int nHeight) noexcept
{
	return (Bitmap*)::CreateCompatibleBitmap(this,nWidth,nHeight);
}
Bitmap * DrawContext::createDIBitmap(CONST BITMAPINFOHEADER *pbmih, DWORD flInit, CONST VOID *pjBits, CONST BitmapInfo *pbmi, UINT iUsage) noexcept
{
	return (Bitmap*)::CreateDIBitmap(this,pbmih,flInit,pjBits,(BITMAPINFO*)pbmi,iUsage);
}
Bitmap * DrawContext::createDIBitmap(CONST BitmapInfo *pbmi) noexcept
{
	return createDIBitmap((BITMAPINFOHEADER*)pbmi,CBM_INIT,(byte*)pbmi + pbmi->biSize,(BitmapInfo*)pbmi,DIB_RGB_COLORS);
}
ObjectSelector DrawContext::use(HGDIOBJ t) noexcept
{
	return ObjectSelector(this, t);
}
int DrawContext::release(win::Window* wnd) noexcept
{
	return ::ReleaseDC(wnd,this);
}
int DrawContext::fillRect(const irect &rect,Brush* brush) noexcept
{
	return ::FillRect(this,(RECT*)&rect,brush);
}

int DrawContext::fillRect(const irect &rect,HBRUSH brush) noexcept
{
	return ::FillRect(this,(RECT*)&rect,brush);
}
bool DrawContext::rectangle(const irect &rect) noexcept
{
	return ::Rectangle(this,rect.left,rect.top,rect.right,rect.bottom);
}
bool DrawContext::ellipse(const irect &rect) noexcept
{
	return ::Ellipse(this,rect.left,rect.top,rect.right,rect.bottom);
}
bool DrawContext::moveTo(ivec2 pt) noexcept
{
	return moveTo(pt.x, pt.y);
}
bool DrawContext::lineTo(ivec2 pt) noexcept
{
	return lineTo(pt.x, pt.y);
}
bool DrawContext::moveTo(int x, int y) noexcept
{
	return ::MoveToEx(this, x, y, nullptr);
}
bool DrawContext::lineTo(int x, int y) noexcept
{
	return ::LineTo(this, x, y);
}
bool DrawContext::polygon(View<ivec2> pts) noexcept
{
	return Polygon(this, (POINT*)pts.begin(), intact<int>(pts.size()));
}
inline void setTriVertex(TRIVERTEX * out, ivec2 pos, bvec4 color)
{
	out->x = pos.x;
	out->y = pos.y;
	out->Red = color.x * 65535/255;
	out->Green = color.y * 65535 / 255;
	out->Blue = color.z * 65535 / 255;
	out->Alpha = color.w * 65535 / 255;
}
bool DrawContext::gradientRectH(ivec2 from, ivec2 to, COLORREF color1, COLORREF color2) noexcept
{
	GRADIENT_RECT rc = { 0, 1 };
	TRIVERTEX triv[2];
	setTriVertex(&triv[0], from, (bvec4&)color1);
	setTriVertex(&triv[1], to, (bvec4&)color2);
	return GradientFill(this, triv, 2, &rc, 1, GRADIENT_FILL_RECT_H);
}
bool DrawContext::gradientRectV(ivec2 from, ivec2 to, COLORREF color1, COLORREF color2) noexcept
{
	GRADIENT_RECT rc = {0, 1};
	TRIVERTEX triv[2];
	setTriVertex(&triv[0], from, (bvec4&)color1);
	setTriVertex(&triv[1], to, (bvec4&)color2);
	return GradientFill(this, triv, 2, &rc, 1, GRADIENT_FILL_RECT_V);
}
bool DrawContext::bitBlt(HDC dc2,const irectwh &From,ivec2 To) noexcept
{
	return ::BitBlt(this,From.x,From.y,From.width,From.height,dc2,To.x,To.y,SRCCOPY);
}
bool DrawContext::stretchBlt(HDC dc2, const irectwh &From, const irectwh &To) noexcept
{
	return ::StretchBlt(this, To.x, To.y, To.width, To.height, dc2, From.x, From.y, From.width, From.height, SRCCOPY);
}
int DrawContext::clip(RGN * rgn) noexcept
{
	return SelectClipRgn(this, rgn);
}
int DrawContext::getTextWidth(pcstr str, size_t len) noexcept
{
	SIZE sz;
	::GetTextExtentPoint32A(this,str,intact<int>(len),&sz);
	return sz.cx;
}
int DrawContext::getTextWidth(pcstr16 str,size_t len) noexcept
{
	SIZE sz;
	::GetTextExtentPoint32W(this, wide(str), intact<int>(len),&sz);
	return sz.cx;
}
int DrawContext::getTextWidth(Text str) noexcept
{
	return getTextWidth(str.begin(),str.size());
}
int DrawContext::getTextWidth(Text16 str) noexcept
{
	return getTextWidth(str.begin(),str.size());
}
bool DrawContext::getTextMetrics(TEXTMETRICA * metric) noexcept
{
	return ::GetTextMetricsA(this, metric);
}
bool DrawContext::getTextMetrics(TEXTMETRICW * metric) noexcept
{
	return ::GetTextMetricsW(this, metric);
}
int DrawContext::getTextHeight() noexcept
{
	TEXTMETRICW tm;
	bool getTextMetricsResult = getTextMetrics(&tm);
	_assert(getTextMetricsResult);
	return tm.tmHeight;
}
int DrawContext::getTextAscent() noexcept
{
	TEXTMETRICW metric;
	bool getTextMetricsResult = getTextMetrics(&metric);
	_assert(getTextMetricsResult);
	return metric.tmAscent;
}
int DrawContext::getTextDescent() noexcept
{
	TEXTMETRICW metric;
	bool getTextMetricsResult = getTextMetrics(&metric);
	_assert(getTextMetricsResult);
	return metric.tmDescent;
}
int DrawContext::getTextPos(Text str,int x) noexcept
{
	int s,p=0;
	if(x<0) return 0;
	do
	{
		if(!str.size()) return p;
		x-=(s=getTextWidth(str.begin(),1));
		p++;
		str++;
	}
	while(x>0);
	if(x<-(s>>1)) return p-1;
	else return p;
}
int DrawContext::setSBL(int extra) noexcept
{
	return ::SetTextCharacterExtra(this, extra);
}
int DrawContext::getSBL() noexcept
{
	return ::GetTextCharacterExtra(this);
}

template <> void DrawContext::textOut<char>(const char * str, size_t len, ivec2 pt) noexcept
{
	::TextOutA(this, pt.x, pt.y, str, intact<int>(len));
}
template <> void DrawContext::textOut<char16>(const char16 * str, size_t len, ivec2 pt) noexcept
{
	::TextOutW(this, pt.x, pt.y, wide(str), intact<int>(len));
}

template <typename T> void DrawContext::textOut(View<T> str, ivec2 pt) noexcept
{
	textOut(str.begin(), str.size(), pt);
}
template void DrawContext::textOut<char>(View<char> str, ivec2 pt) noexcept;
template void DrawContext::textOut<char16>(View<char16> str, ivec2 pt) noexcept;

template <typename T> void DrawContext::textWrite(View<T> str, ivec2* pt) noexcept
{
	int txHeight=getTextHeight();
	View<T> Find, pFind=str;
	while((Find=pFind.find('\n')) != nullptr)
	{
		textOut(pFind.begin(), Find - pFind, *pt);
		pFind=Find+1;
		pt->y += txHeight;
	}
	textOut<T>(pFind, *pt);
	pt->x = getTextWidth(pFind)+pt->x;
}
template void DrawContext::textWrite<char>(View<char> str, ivec2* pt) noexcept;
template void DrawContext::textWrite<char16>(View<char16> str, ivec2* pt) noexcept;

template <typename T> void DrawContext::textWrite(View<T> str, ivec2* pt, irectwh* rc, bool nodraw) noexcept
{
	if(rc==nullptr) return textWrite<T>(str, pt);
	int txHeight=getTextHeight();
	
	int px=pt->x, x=pt->x, y=pt->y;
	int rcr=rc->x+rc->width;

	View<T> s = str;
	View<T> t = s;
	for(; t.begin() != t.end();)
	{
		if(*t==(T)'\n')
		{
			t++;
		}
		else
		{
			x += getTextWidth(t.begin(), 1);
			if(x>rcr)
			{
				if (!nodraw) textOut(s.begin(), t - s ,{ px, y });
			}
			else
			{
				t++;
				continue;
			}
		}
		y += txHeight;
		s = t;
		px = x = rc->x;
	}
	pt->x = x;
	pt->y = y;
	rc->height = y+txHeight-rc->y;
	textOut<T>(s, { px, y });
}
template void DrawContext::textWrite<char>(View<char> str, ivec2* pt, irectwh* rc, bool nodraw) noexcept;
template void DrawContext::textWrite<char16>(View<char16> str, ivec2* pt, irectwh* rc, bool nodraw) noexcept;

int DrawContext::drawText(Text text, irect* lprc, uint format) noexcept
{
	return DrawTextA(this, text.begin(), intact<int>(text.size()), (LPRECT)lprc, format);
}
int DrawContext::drawText(Text16 text, irect* lprc, uint format) noexcept
{
	return DrawTextW(this, wide(text.begin()), intact<int>(text.size()), (LPRECT)lprc, format);
}
dword DrawContext::getGlyphOutlineA(
	uint uChar,
	uint fuFormat,
	LPGLYPHMETRICS lpgm,
	dword cjBuffer,
	void * pvBuffer,
	const fixedmatrix2 *lpmat2
	)
{
	return GetGlyphOutlineA(this, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, (MAT2*)lpmat2);
}
dword DrawContext::getGlyphOutline(
	uint uChar,
	uint fuFormat,
	LPGLYPHMETRICS lpgm,
	dword cjBuffer,
	void * pvBuffer,
	const fixedmatrix2 *lpmat2
	)
{
	return GetGlyphOutlineW(this, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, (MAT2*)lpmat2);
}
TBuffer DrawContext::getGlyphOutlineA(
	uint uChar,
	uint fuFormat,
	LPGLYPHMETRICS lpgm,
	const fixedmatrix2 *lpmat2
)
{
	dword size = getGlyphOutlineA(uChar, fuFormat, lpgm, 0, nullptr, lpmat2);
	TBuffer buffer(size);
	getGlyphOutlineA(uChar, fuFormat, lpgm, size, buffer.begin(), lpmat2);
	return buffer;
}
TBuffer DrawContext::getGlyphOutline(
	uint uChar,
	uint fuFormat,
	LPGLYPHMETRICS lpgm,
	const fixedmatrix2 *lpmat2
)
{
	dword size = getGlyphOutline(uChar, fuFormat, lpgm, 0, nullptr, lpmat2);
	TBuffer buffer(size);
	getGlyphOutline(uChar, fuFormat, lpgm, size, buffer.begin(), lpmat2);
	return buffer;
}
bool DrawContext::drawFocusRect(const irect& rc) noexcept
{
	return ::DrawFocusRect(this, (LPRECT)&rc);
}

dword DrawContext::setTextColor(dword b) noexcept
{
	return ::SetTextColor(this,b);
}
dword DrawContext::setTextColor(byte r, byte g, byte b) noexcept
{
	return ::SetTextColor(this, RGB(r, g, b));
}
dword DrawContext::getTextColor() noexcept
{
	return ::GetTextColor(this);
}
dword DrawContext::setBkColor(dword b) noexcept
{
	return ::SetBkColor(this,b);
}
dword DrawContext::setBkColor(byte r, byte g, byte b) noexcept
{
	return ::SetBkColor(this,RGB(r,g,b));
}
dword DrawContext::getBkColor() noexcept
{
	return ::GetBkColor(this);
}
int DrawContext::setBkMode(int nMode) noexcept
{
	return ::SetBkMode(this, nMode);
}
int DrawContext::getBkMode() noexcept
{
	return ::GetBkMode(this);
}
int DrawContext::setROP2(int rop) noexcept
{
	return ::SetROP2(this,rop);
}
int DrawContext::getROP2() noexcept
{
	return ::GetROP2(this);
}
int DrawContext::setMapMode(int nMode) noexcept
{
	return ::SetMapMode(this, nMode);
}
int DrawContext::getMapMode() noexcept
{
	return ::GetMapMode(this);
}
bool DrawContext::setInputRange(ivec2 pt, ivec2 * pPrevious) noexcept
{
	return ::SetWindowExtEx(this, pt.x, pt.y, (LPSIZE)pPrevious);
}
bool DrawContext::setInputOffset(ivec2 pt, ivec2 * pPrevious) noexcept
{
	return ::SetWindowOrgEx(this, pt.x, pt.y, (LPPOINT)pPrevious);
}
bool DrawContext::setOutputRange(ivec2 pt, ivec2 * pPrevious) noexcept
{
	return ::SetViewportExtEx(this, pt.x, pt.y, (LPSIZE)pPrevious);
}
bool DrawContext::setOutputOffset(ivec2 pt, ivec2 * pPrevious) noexcept
{
	return ::SetViewportOrgEx(this, pt.x, pt.y, (LPPOINT)pPrevious);
}
ivec2 DrawContext::getInputRange() noexcept
{
	SIZE size;
	::GetViewportExtEx(this, &size);
	return (ivec2&)size;
}
ivec2 DrawContext::getInputOffset() noexcept
{
	POINT pt;
	::GetViewportOrgEx(this, &pt);
	return (ivec2&)pt;
}
ivec2 DrawContext::getOutputRange() noexcept
{
	SIZE size;
	::GetWindowExtEx(this, &size);
	return (ivec2&)size;
}
ivec2 DrawContext::getOutputOffset() noexcept
{
	POINT pt;
	::GetViewportOrgEx(this, &pt);
	return (ivec2&)pt;
}
bool DrawContext::setInputRect(const irectwh &rc) noexcept
{
	return setInputOffset(rc.pos) && setInputRange(rc.size);
}
bool DrawContext::setOutputRect(const irectwh &rc) noexcept
{
	return setOutputOffset(rc.pos) && setOutputRange(rc.size);
}

void DrawContext::drawOutline(ivec2 wpt,char16 chr,const fixedmatrix2 &mat) noexcept
{
	GLYPHMETRICS gm;
	int fullsize;
	fullsize = GetGlyphOutline(this, chr, GGO_BEZIER, &gm, 0, nullptr, (MAT2*)&mat);

	TmpArray<byte> buff(fullsize);
	
	GetGlyphOutline(this, chr, GGO_BEZIER, &gm, fullsize, buff.begin(), (MAT2*)&mat);

	int height=gm.gmBlackBoxY;
	byte *read, *next, *last;

	read=buff.begin();
	last=buff.end();
	do
	{
		ivec2 spt;
		LPTTPOLYGONHEADER ph=(LPTTPOLYGONHEADER)read;

		spt.x=ph->pfxStart.x.value+wpt.x;
		spt.y=height-ph->pfxStart.y.value+wpt.y;
		moveTo(spt);

		next=read+ph->cb;
		read+=sizeof(TTPOLYGONHEADER);
		do
		{
			LPTTPOLYCURVE pc=(LPTTPOLYCURVE)read;
			LPPOINT pt=(LPPOINT)&(pc->apfx[0]);
			int count=pc->cpfx;

			for(int i=0;i<count;i++)
			{
				pt[i].x=pc->apfx[i].x.value+wpt.x;
				pt[i].y=height-pc->apfx[i].y.value+wpt.y;
			}
			switch(pc->wType)
			{
			case TT_PRIM_LINE: PolylineTo(this,pt,count); break;
			case TT_PRIM_QSPLINE: PolyBezierTo(this,pt,count); break;
			case TT_PRIM_CSPLINE: PolyBezierTo(this,pt,count); break;
			default: debug(); // 잘못된 타입이다.
			}
			
			read+=(count-1)*sizeof(POINTFX)+sizeof(TTPOLYCURVE);
		}
		while(read<next);
		lineTo(spt);
	}
	while(read<last);
}
JOB DrawContext::startDoc(const DOCINFOW* pInfo) noexcept
{
	int job = ::StartDocW(this, pInfo);
	if(job <= 0) return nullptr;
	return (JOB)(size_t)job;
}
bool DrawContext::startPage() noexcept
{
	return ::StartPage(this) > 0;
}
bool DrawContext::endPage() noexcept
{
	return ::EndPage(this) > 0;
}
bool DrawContext::endDoc() noexcept
{
	return ::EndDoc(this) > 0;
}
int DrawContext::getDeviceCaps(int index) noexcept
{
	return ::GetDeviceCaps(this, index);
}

int DrawContext::getDPIX() noexcept { return getDeviceCaps(LOGPIXELSX); }
int DrawContext::getDPIY() noexcept { return getDeviceCaps(LOGPIXELSY); }
int DrawContext::getPixelWidth() noexcept { return getDeviceCaps(HORZRES); }
int DrawContext::getPixelHeight() noexcept { return getDeviceCaps(VERTRES); }
int DrawContext::getMillimeterWidth() noexcept { return getDeviceCaps(HORZSIZE); }
int DrawContext::getMillimeterHeight() noexcept { return getDeviceCaps(VERTSIZE); }

PaintStruct::PaintStruct(win::Window * wnd) noexcept
	:window(wnd)
{
	window->beginPaint(this);
}
PaintStruct::~PaintStruct() noexcept
{
	window->endPaint(this);
}
DrawContext* PaintStruct::operator ->() noexcept
{
	return (DrawContext*)hdc;
}
PaintStruct::operator DrawContext*() noexcept
{
	return (DrawContext*)hdc;
}
PaintStruct::operator HDC() noexcept
{
	return hdc;
}

DoubleBufferedDC::DoubleBufferedDC(win::Window* wnd) noexcept
{
	m_client = (irectwh)wnd->getClientRect();
	m_frontDc = wnd->getDC();
	m_backDc = m_frontDc->createCompatibleDC();
	Bitmap * bm = m_frontDc->createCompatibleBitmap(m_client.width, m_client.height);
	m_oldbm = m_backDc->select(bm);
}
DoubleBufferedDC::~DoubleBufferedDC() noexcept
{
	Bitmap * bm = m_backDc->select(m_oldbm);
	delete bm;
}
DrawContext* DoubleBufferedDC::operator ->() noexcept
{
	return m_backDc;
}
void DoubleBufferedDC::clear(HBRUSH brush) noexcept
{
	m_backDc->fillRect((irect)m_client, brush);
}
void DoubleBufferedDC::present() noexcept
{
	m_frontDc->bitBlt(m_backDc, m_client, ivec2(0, 0));
}
irectwh DoubleBufferedDC::getRect() noexcept
{
	return m_client;
}
