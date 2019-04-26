#pragma once

#include <KR3/math/coord.h>
#include <KR3/wl/handle.h>
#include <KRWin/gdi.h>
#include <functional>

namespace kr
{
	namespace paper
	{
		constexpr float A4_WIDTH = 210.f;
		constexpr float A4_HEIGHT = 297.f;
	}

	class GDIReUnit
	{
	public:
		GDIReUnit() noexcept;
		~GDIReUnit() noexcept;

		using PreviewCallback = void(*)(GDIReUnit * dc, void * param);
		
		enum class Align
		{
			Left,
			Center,
			Right,
		};

		enum class Mode
		{
			Spacing,
			Fill,
			Limit
		};

		static win::Window* createPreview(dword style, int x,int y, vec2 paper, int width, PreviewCallback ondraw, void * param = nullptr) noexcept;
		void clearState() noexcept;
		void setFont(Text16 strName, float pt, int weight = FW_NORMAL) noexcept;
		float setWidth(float width, Mode mode) noexcept;
		float setLineHeight(float rate) noexcept;
		void text(Text16 text) noexcept;
		vec2 getPosition() noexcept;
		vec2 moveTo(vec2 point) noexcept;
		vec2 moveBy(vec2 point) noexcept;
		void lineTo(vec2 point) noexcept;
		void strokeRect(frect rc) noexcept;
		void fillRect(frect rc) noexcept;
		void setAlign(Align align) noexcept;
		COLORREF setColor(COLORREF color) noexcept;
		float setStrokeWidth(float width) noexcept;
		void drawImage(win::Bitmap* bmp,float width,float height) noexcept;

		void setDC(win::DrawContext * pDC) noexcept;
		void setDCWithDPMM(win::DrawContext * pDC, vec2 dpmm) noexcept;
		void setDCWithDPI(win::DrawContext * pDC, vec2 dpi) noexcept;

	protected:
		Keep<win::DrawContext> m_pDC;

	private:
		template <typename LAMBDA> void _textLoop(Text16 text, LAMBDA &lambda) noexcept;
		void _setTool(bool fill) noexcept;

		win::Font * m_fontOld;
		win::Brush * m_brushOld;
		win::Pen * m_penOld;
		Keep<win::Font> m_fontNow;
		win::GDIObject<void> * m_toolOld;
		Keep<win::GDIObject<void>> m_toolNow;
		Keep<win::DrawContext> m_pMDC;
		LOGFONTW m_logfont;
	
		vec2 m_offset;
		vec2 m_vDPMM;
		int m_nCursorX;
		float m_fLineHeight;
		int m_width;
		int m_strokeWidth;
		Align m_align;
		Mode m_mode;
		COLORREF m_color;
		bool m_fillMode;
		bool m_recolor;
		bool m_rewidth;
	};

	class GDIPrinter:public GDIReUnit
	{
	public:
		GDIPrinter(const GDIPrinter&) = delete;
		GDIPrinter& operator =(const GDIPrinter&) = delete;
		GDIPrinter();
		~GDIPrinter();

		bool beginPrint(pcstr16 strDocName);
		void endPrint();

		bool startPage();
		bool endPage();

	protected:
		using GDIReUnit::setDC;
	};


}
