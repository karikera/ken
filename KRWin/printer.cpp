#include "stdafx.h"
#include "printer.h"
#include "gdi.h"

#include "winx.h"

#include <commdlg.h>

const float INCH2PT = 72.f;
const float PT2INCH = 1.f/INCH2PT;

const float MM2INCH = 1.f/MM2INCH;
const float INCH2MM = 25.4f;

const float MM2PT = MM2INCH * INCH2PT;
const float PT2MM = PT2INCH * INCH2MM;

kr::GDIReUnit::GDIReUnit() noexcept
{
}
kr::GDIReUnit::~GDIReUnit() noexcept
{
	if (m_pDC != nullptr)
	{
		m_pDC->select(m_fontOld);
	}
}
kr::win::Window* kr::GDIReUnit::createPreview(dword style, int x,int y, vec2 paper, int height, PreviewCallback ondraw, void * param) noexcept
{
	struct ExtraData
	{
		PreviewCallback cb;
		void * param;
		float dpmm;
	};

	staticCode {
		WindowClass(u"Preview", [](win::Window* wnd, UINT Msg, WPARAM wParam, LPARAM lParam)->LRESULT {
			switch (Msg)
			{
			case WM_PAINT:
				PAINTSTRUCT ps;
				GDIReUnit dc;
				dword dpmm = wnd->getLong(offsetof(ExtraData , dpmm));
				PreviewCallback cb = (PreviewCallback)wnd->getLongPtr(offsetof(ExtraData, cb));
				void * param = (void*)wnd->getLongPtr(offsetof(ExtraData, param));

				dc.setDCWithDPMM(wnd->beginPaint(&ps), {(float&)dpmm, (float&)dpmm});
				cb(&dc, param);
				wnd->endPaint(&ps);
				break;
			}
			return DefWindowProcW(wnd, Msg, wParam, lParam);
		}, 0, CS_HREDRAW | CS_VREDRAW, sizeof(ExtraData)).registerClass();
	};

	irect rc;
	rc.left = x;
	rc.top = y;
	rc.right = x + (int)(paper.x / paper.y * height);
	rc.bottom = y + height;
	float dpmm = height / paper.y;

	AdjustWindowRect((LPRECT)&rc, style, false);
	win::Window * wnd =  win::Window::create(u"Preview", u"Preview", style, (irectwh)rc, nullptr, nullptr);
	wnd->setLongPtr(offsetof(ExtraData, cb), (intptr_t)ondraw);
	wnd->setLongPtr(offsetof(ExtraData, param), (intptr_t)param);
	wnd->setLong(offsetof(ExtraData, dpmm), (long&)dpmm);
	return wnd;
}
void kr::GDIReUnit::clearState() noexcept
{
	if (m_pDC != nullptr)
	{
		if (m_fontNow != nullptr) m_pDC->select(m_fontOld);
		if (m_toolNow != nullptr) m_pDC->select(m_toolOld);
	}
	m_fontNow = nullptr;
	m_toolNow = nullptr;

	m_offset.x = m_offset.y = 0.f;
	m_fontOld = nullptr;
	m_nCursorX = 0;
	m_fLineHeight = 1.f;
	m_align = Align::Left;
	m_mode = Mode::Spacing;
	m_width = 0;
	m_recolor = true;
	m_rewidth = true;
	m_color = RGB(0, 0, 0);
	m_fillMode = false;

	//m_logfont.lfHeight;
	m_logfont.lfWidth = 0;
	m_logfont.lfEscapement = 0;
	m_logfont.lfOrientation = 0;
	//m_logfont.lfWeight;
	m_logfont.lfItalic = false;
	m_logfont.lfUnderline = false;
	m_logfont.lfStrikeOut = false;
	m_logfont.lfCharSet = DEFAULT_CHARSET;
	m_logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_logfont.lfQuality = ANTIALIASED_QUALITY;
	m_logfont.lfPitchAndFamily = DEFAULT_PITCH;
	//m_logfont.lfFaceName;
}
void kr::GDIReUnit::setFont(Text16 strName, float pt, int weight) noexcept
{
	m_logfont.lfHeight = (int)(pt*PT2MM*m_vDPMM.y);
	m_logfont.lfWeight = weight;
	Writer16(unwide(m_logfont.lfFaceName), countof(m_logfont.lfFaceName)) << strName << nullterm;
	m_pDC->select(m_fontOld);
	m_fontNow = win::Font::create(m_logfont);
	m_fontOld = m_pDC->select((win::Font*)m_fontNow);
}
float kr::GDIReUnit::setWidth(float width, Mode mode) noexcept
{
	float old = (float)m_width / m_vDPMM.x;
	m_width = (int)(width * m_vDPMM.x);
	m_mode = mode;
	return old;
}
float kr::GDIReUnit::setLineHeight(float rate) noexcept
{
	float old = m_fLineHeight;
	m_fLineHeight = rate;
	return old;
}
void kr::GDIReUnit::text(Text16 text) noexcept
{
	switch (m_mode)
	{
	case Mode::Spacing:
		m_pDC->setSBL(m_width);
		_textLoop(text, [this](Text16 str, ivec2 pt) {
			int x;
			int width = m_pDC->getTextWidth(str);
			switch (m_align)
			{
			case Align::Left:	x = pt.x; break;
			case Align::Center:	x = pt.x - width / 2; break;
			case Align::Right:	x = pt.x - width; break;
			}
			m_pDC->textOut<char16>(str, { x, pt.y });
			return x + width;
		});
		break;
	case Mode::Fill:
		_textLoop(text, [this](Text16 str, ivec2 pt) {
			int x;
			switch (m_align)
			{
			case Align::Left:	x = pt.x; break;
			case Align::Center:	x = pt.x - m_width / 2; break;
			case Align::Right:	x = pt.x - m_width; break;
			}
			m_pDC->setSBL(0);
			int width = m_pDC->getTextWidth(str);
			m_pDC->setSBL((int)((m_width - width) / (intptr_t)(str.size() - 1)));
			m_pDC->textOut<char16>(str, { x, pt.y });
			return x + m_width;
		});
		break;
	case Mode::Limit:
		_textLoop(text, [this](Text16 str, ivec2 pt) {
			int x;
			switch (m_align)
			{
			case Align::Left:	x = pt.x; break;
			case Align::Center:	x = pt.x - m_width / 2; break;
			case Align::Right:	x = pt.x - m_width; break;
			}
			m_pDC->setSBL(0);
			int width = m_pDC->getTextWidth(str);
			int sbl;
			if (m_width >= width) sbl = 0;
			else sbl = (int)((m_width - width) / (intptr_t)(str.size() - 1));
			m_pDC->setSBL(sbl);
			m_pDC->textOut<char16>(str, { x, pt.y });
			return x + m_width;
		});
		break;
	}
}
template <typename LAMBDA>
void kr::GDIReUnit::_textLoop(Text16 text, LAMBDA &&lambda) noexcept
{
	int lineHeight = (int)(m_pDC->getTextHeight()*m_fLineHeight);
	Text16 findtx = text;
	ivec2 pt = (ivec2)(m_offset * m_vDPMM);
	pt.x += m_nCursorX;
	
	pcstr16 find = findtx.find(u'\n');
	if (find != nullptr)
	{
		lambda(findtx.cut(find), pt);
		pt.y += (int)lineHeight;
		findtx.subarr_self(find + 1);
		pt.x -= m_nCursorX;
		m_nCursorX = 0;

		while ((find = findtx.find(u'\n')) != nullptr)
		{
			lambda(findtx.cut(find), pt);
			pt.y += (int)lineHeight;
			findtx.subarr_self(find + 1);
		}
	}
	int lastX = lambda(findtx, pt);
	pt.x -= m_nCursorX;

	m_nCursorX = lastX - pt.x;
	m_offset = (vec2)pt / m_vDPMM;
}
kr::vec2 kr::GDIReUnit::getPosition() noexcept
{
	return (vec2)m_pDC->getOutputOffset() / m_vDPMM;
}
kr::vec2 kr::GDIReUnit::moveTo(vec2 point) noexcept
{
	vec2 old = m_offset;
	m_offset = point;
	m_pDC->moveTo((ivec2)(m_offset * m_vDPMM));
	m_nCursorX = 0;
	return old;
}
kr::vec2 kr::GDIReUnit::moveBy(vec2 point) noexcept
{
	vec2 old = m_offset;
	m_offset += point;
	m_pDC->moveTo((ivec2)(m_offset * m_vDPMM));
	m_nCursorX = 0;
	return old;
}
void kr::GDIReUnit::lineTo(vec2 point) noexcept
{
	_setTool(false);
	point *= m_vDPMM;
	m_pDC->lineTo((ivec2)point);
}
void kr::GDIReUnit::strokeRect(frect rc) noexcept
{
	_setTool(false);
	rc.from *= m_vDPMM;
	rc.to *= m_vDPMM;
	m_pDC->rectangle((irect)rc);
}
void kr::GDIReUnit::fillRect(frect rc) noexcept
{
	_setTool(true);
	rc.from *= m_vDPMM;
	rc.to *= m_vDPMM;
	m_pDC->rectangle((irect)rc);
}
void kr::GDIReUnit::setAlign(Align align) noexcept
{
	m_align = align;
}
COLORREF kr::GDIReUnit::setColor(COLORREF color) noexcept
{
	COLORREF old = m_color;
	m_color = color;
	m_recolor = true;
	return old;
}
float kr::GDIReUnit::setStrokeWidth(float width) noexcept
{
	float old = m_strokeWidth / m_vDPMM.x;
	m_strokeWidth = (int)(width * m_vDPMM.x);
	if (m_strokeWidth == 0 && width > 0)
	{
		m_strokeWidth = 1;
	}
	m_rewidth = true;
	return old;
}
void kr::GDIReUnit::drawImage(win::Bitmap* bmp, float width, float height) noexcept
{
	win::Bitmap* pOld = m_pMDC->select(bmp);
	BITMAP bm;
	bool bmpGetObjectResult = bmp->getObject(&bm);
	_assert(bmpGetObjectResult);
	m_pDC->stretchBlt(m_pMDC
		, { {0, 0} , {bm.bmWidth, bm.bmHeight} }
		, { (ivec2)(m_offset*m_vDPMM)+ivec2(m_nCursorX, 0), {(int)(width*m_vDPMM.x), (int)(height*m_vDPMM.y)} }
	);
	m_pMDC->select(pOld);
}

void kr::GDIReUnit::setDC(win::DrawContext * pDC) noexcept
{
	setDCWithDPI(pDC, { (float)pDC->getDPIX() , (float)pDC->getDPIY() });
}
void kr::GDIReUnit::setDCWithDPMM(win::DrawContext * pDC, vec2 dpmm) noexcept
{
	clearState();
	m_pDC = pDC;
	m_brushOld = m_pDC->select(win::Brush::getStock(NULL_BRUSH));
	m_penOld = m_pDC->select(win::Pen::getStock(NULL_PEN));
	m_pDC->setBkMode(TRANSPARENT);
	m_pMDC = pDC->createCompatibleDC();
	m_vDPMM = dpmm;
	m_strokeWidth = (int)m_vDPMM.x;
}
void kr::GDIReUnit::setDCWithDPI(win::DrawContext * pDC, vec2 dpi) noexcept
{
	setDCWithDPMM(pDC, dpi / INCH2MM);
}

void kr::GDIReUnit::_setTool(bool fill) noexcept
{
	if (m_rewidth)
	{
		m_rewidth = false;
		if (!fill) goto _remake;
	}
	if (fill != m_fillMode) goto _remake;
	if(m_recolor) goto _remake;
	return;
_remake:
	m_fillMode = fill;
	m_recolor = false;

	if (m_toolNow != nullptr)
	{
		m_pDC->select(m_toolOld);
		m_toolNow = nullptr;
	}
	if (fill)
	{
		m_toolNow = (win::GDIObject<void>*)win::Brush::create(m_color);
	}
	else
	{
		m_toolNow = (win::GDIObject<void>*)win::Pen::create(m_color, m_strokeWidth);
	}
	m_pDC->select((win::GDIObject<void>*)m_toolNow);
}

kr::GDIPrinter::GDIPrinter()
{
}
kr::GDIPrinter::~GDIPrinter()
{
	endPrint();
}
bool kr::GDIPrinter::beginPrint(pcstr16 strDocName)
{
	static HGLOBAL devname;
	endPrint();

	PRINTDLGW pd;
	mema::zero(pd);
	pd.lStructSize = sizeof(pd);
	pd.hwndOwner = win::g_mainWindow;
	pd.hDevNames = devname;
	pd.Flags       = PD_RETURNDC | PD_NOSELECTION | PD_NOPAGENUMS | PD_USEDEVMODECOPIESANDCOLLATE;
	if(!PrintDlgW(&pd)) return false;
	GlobalFree(pd.hDevMode);
	devname = pd.hDevNames;

	win::DrawContext* pDC = (win::DrawContext*)pd.hDC;

	DOCINFOW info;
	mema::zero(info);
	info.cbSize = sizeof(info);
	info.lpszDocName = wide(strDocName);
	if(pDC->startDoc(&info) == nullptr)
	{
		delete pDC;
		return false;
	}

	setDC(pDC);
	return true; 
}
void kr::GDIPrinter::endPrint()
{
	if(m_pDC == nullptr) return;
	m_pDC->endDoc();
	m_pDC = nullptr;
}
bool kr::GDIPrinter::startPage()
{
	return m_pDC->startPage();
}
bool kr::GDIPrinter::endPage()
{
	return m_pDC->endPage();
}

//dm=(DEVMODE *)GlobalLock(pd.hDevMode);
//dm->dmFields=DM_ORIENTATION;
//dm->dmOrientation=Orient;
//ResetDC(hPrtdc, dm);
//GlobalUnlock(pd.hDevMode);
