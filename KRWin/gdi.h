#pragma once

#include "handle.h"
#include <KR3/math/fixed/fixedmatrix.h>
//#include <KR3/draw/bitmap.h>

namespace kr
{
	namespace win
	{
	
		class DefaultFont
		{
		public:
			DefaultFont() = delete;
			DefaultFont& operator =(Font * font) noexcept;
			operator Font*() noexcept;
			Font* operator ->() noexcept;
		};

		static DefaultFont &g_defaultFont = nullref;

		template <typename T> class GDIObjectBase;
		template <typename T> class GDIObject;
		class ObjectSelector;
		using RGN = GDIObject<HRGN__>;
		using Pen = GDIObject<HPEN__>;
		using Brush = GDIObject<HBRUSH__>;
		using Font = GDIObject<HFONT__>;
		using Bitmap = GDIObject<HBITMAP__>;
		class DrawContext;

		struct __JOB;
		using JOB = __JOB*;

		namespace _pri_
		{
			void deleteObject(ptr handle) noexcept;
			ptr getStockObject(int id) noexcept;
		}
	
		template <typename T> class GDIObjectBase:public Handle<T>
		{
		public:
			GDIObjectBase() = delete;
			void operator delete(void * ptr) noexcept
			{
				_pri_::deleteObject(ptr);
			}
			static GDIObject<T>* getStock(int nID) noexcept
			{
				return (GDIObject<T>*)_pri_::getStockObject(nID);
			}
		};

		template <typename T> class GDIObject:public GDIObjectBase<T>
		{
			GDIObject() = delete;
		};

		template <> class GDIObject<HRGN__>:public GDIObjectBase<HRGN__>
		{
		public:
			GDIObject() = delete;
			static RGN* createRoundRect(const irect & rc, int rx, int ry) noexcept;
			static RGN* createRect(const irect & rc) noexcept;
		};

		template <> class GDIObject<HPEN__>:public GDIObjectBase<HPEN__>
		{
		public:
			GDIObject() = delete;
			static Pen* create(COLORREF color,int width=1,int ps=PS_SOLID);

		};
		template <> class GDIObject<HBRUSH__>:public GDIObjectBase<HBRUSH__>
		{
		public:
			GDIObject() = delete;
			static Brush* create(COLORREF color);
			static Brush* create(byte r,byte g,byte b);
			static Brush* fromSystem(int nIndex);
		};
		template <> class GDIObject<HFONT__>:public GDIObjectBase<HFONT__>
		{
		public:
			GDIObject() = delete;
			static Font* create(const char* name, int height, int weight = FW_NORMAL) noexcept;
			static Font* create(const char16* name, int height, int weight = FW_NORMAL) noexcept;
			static Font* create(const LOGFONTA & font) noexcept;
			static Font* create(const LOGFONTW & font) noexcept;
			static Font* createSystemFont() noexcept;
			static Font* createSystemFont(int height) noexcept;
			bool getObject(LOGFONTA * lf) noexcept;
			bool getObject(LOGFONTW * lf) noexcept;
			int getHeight() noexcept;
			void getName(Writer16 * writer) noexcept;

		};
		template <> class GDIObject<HBITMAP__>:public GDIObjectBase<HBITMAP__>
		{
		public:
			GDIObject() = delete;
			static Bitmap * create(HDC hDC,int w,int h) noexcept;
			static Bitmap * load(const char * strFileName, uint fuLoad = LR_LOADFROMFILE) noexcept;
			static Bitmap * load(const char16 * strFileName, uint fuLoad = LR_LOADFROMFILE) noexcept;

			bool getObject(BITMAP *bmp) noexcept;
		};
		
		class DrawContext:public Handle<HDC__>
		{
		public:
			DrawContext() = delete;
			static DrawContext * get(win::Window* wnd) noexcept;
			static DrawContext * create() noexcept;
			void operator delete(void * p) noexcept;
			DrawContext * createCompatibleDC() noexcept;
			Bitmap * createCompatibleBitmap(int nWidth,int nHeight) noexcept;
			Bitmap * createDIBitmap(CONST BITMAPINFOHEADER *pbmih, DWORD flInit, CONST VOID *pjBits, CONST BitmapInfo *pbmi, UINT iUsage) noexcept;
			Bitmap * createDIBitmap(CONST BitmapInfo *pbmi) noexcept;
			template <typename T> T* select(T* t) noexcept
			{
				return (T*)SelectObject(this,(HGDIOBJ)t);
			}
			ObjectSelector use(HGDIOBJ t) noexcept;
			int release(win::Window* wnd) noexcept;
			int fillRect(const irect &rect,Brush* brush) noexcept;
			int fillRect(const irect &rect,HBRUSH brush) noexcept;
			bool rectangle(const irect &rect) noexcept;
			bool ellipse(const irect &rect) noexcept;
			bool moveTo(ivec2 pt) noexcept;
			bool lineTo(ivec2 pt) noexcept;
			bool moveTo(int x, int y) noexcept;
			bool lineTo(int x, int y) noexcept;
			bool polygon(View<ivec2> pts) noexcept;
			bool gradientRectH(ivec2 from, ivec2 to, COLORREF color1, COLORREF color2) noexcept;
			bool gradientRectV(ivec2 from, ivec2 to, COLORREF color1, COLORREF color2) noexcept;
			bool bitBlt(HDC dc2,const irectwh &From,ivec2 To) noexcept;
			bool stretchBlt(HDC dc2, const irectwh &From, const irectwh &To) noexcept;

			int clip(RGN * rgn) noexcept;

			int getTextWidth(pcstr str,size_t len) noexcept;
			int getTextWidth(pcstr16 str,size_t len) noexcept;
			int getTextWidth(Text str) noexcept;
			int getTextWidth(Text16 str) noexcept;
			bool getTextMetrics(TEXTMETRICA * metric) noexcept;
			bool getTextMetrics(TEXTMETRICW * metric) noexcept;
			int getTextHeight() noexcept;
			int getTextAscent() noexcept;
			int getTextDescent() noexcept;
			int getTextPos(Text str,int x) noexcept;
			int setSBL(int extra) noexcept; // Space between letter
			int getSBL() noexcept; // Space between letter
	
			template <typename T> void textOut(const T * str, size_t len, ivec2 pt) noexcept;
			template <typename T> void textOut(View<T> str, ivec2 pt) noexcept;
			template <typename T> void textWrite(View<T> str, ivec2* pt) noexcept;
			template <typename T> void textWrite(View<T> str, ivec2* pt, irectwh* rc, bool nodraw = false) noexcept;
			int drawText(Text text, irect* lprc, uint format) noexcept;
			int drawText(Text16 text, irect* lprc, uint format) noexcept;
			dword getGlyphOutlineA(
				uint uChar,
				uint fuFormat,
				LPGLYPHMETRICS lpgm,
				dword cjBuffer,
				void * pvBuffer,
				const fixedmatrix2 *lpmat2
				);
			dword getGlyphOutline(
				uint uChar,
				uint fuFormat,
				LPGLYPHMETRICS lpgm,
				dword cjBuffer,
				void * pvBuffer,
				const fixedmatrix2 *lpmat2
				);
			TBuffer getGlyphOutlineA(
				uint uChar,
				uint fuFormat,
				LPGLYPHMETRICS lpgm,
				const fixedmatrix2 *lpmat2
			);
			TBuffer getGlyphOutline(
				uint uChar,
				uint fuFormat,
				LPGLYPHMETRICS lpgm,
				const fixedmatrix2 *lpmat2
			);
			bool drawFocusRect(const irect& rc) noexcept;

			dword setTextColor(dword b) noexcept;
			dword setTextColor(byte r, byte g, byte b) noexcept;
			dword getTextColor() noexcept;
			dword setBkColor(dword b) noexcept;
			dword setBkColor(byte r, byte g, byte b) noexcept;
			dword getBkColor() noexcept;
			int setBkMode(int nMode) noexcept;
			int getBkMode() noexcept;
			int setROP2(int rop) noexcept;
			int getROP2() noexcept;
			int setMapMode(int nMode) noexcept;
			int getMapMode() noexcept;
			void setWorldTransform() noexcept;
			bool setInputRange(ivec2 pt, ivec2 * pPrevious = nullptr) noexcept;
			bool setInputOffset(ivec2 pt, ivec2 * pPrevious = nullptr) noexcept;
			bool setOutputRange(ivec2 pt, ivec2 * pPrevious = nullptr) noexcept;
			bool setOutputOffset(ivec2 pt,ivec2 * pPrevious = nullptr) noexcept;
			ivec2 getInputRange() noexcept;
			ivec2 getInputOffset() noexcept;
			ivec2 getOutputRange() noexcept;
			ivec2 getOutputOffset() noexcept;
			bool setInputRect(const irectwh &rc) noexcept;
			bool setOutputRect(const irectwh &rc) noexcept;
	
			void drawOutline(ivec2 wpt, char16 chr, const fixedmatrix2 &mat=fixedmatrix2(1, 0, 0, 1)) noexcept;
			JOB startDoc(const DOCINFOW* pInfo) noexcept;
			bool startPage() noexcept;
			bool endPage() noexcept;
			bool endDoc() noexcept;
			int getDeviceCaps(int index) noexcept;

			int getDPIX() noexcept;
			int getDPIY() noexcept;
			int getPixelWidth() noexcept;
			int getPixelHeight() noexcept;
			int getMillimeterWidth() noexcept;
			int getMillimeterHeight() noexcept;

		};

		class PaintStruct : public PAINTSTRUCT
		{
		public:
			PaintStruct(win::Window * wnd) noexcept;
			~PaintStruct() noexcept;
			DrawContext* operator ->() noexcept;
			operator DrawContext*() noexcept;
			operator HDC() noexcept;

			win::Window * const window;
		};


		class DoubleBufferedDC
		{
		public:
			DoubleBufferedDC(win::Window* wnd) noexcept;
			~DoubleBufferedDC() noexcept;
			DrawContext* operator ->() noexcept;
			void clear(HBRUSH brush) noexcept;
			void present() noexcept;
			irectwh getRect() noexcept;

		private:
			Bitmap * m_oldbm;
			DrawContext * m_frontDc;
			DrawContext * m_backDc;
			irectwh m_client;
		};

		extern template void DrawContext::textOut<char>(const char * str, size_t len, ivec2 pt) noexcept;
		extern template void DrawContext::textOut<char>(View<char> str, ivec2 pt) noexcept;
		extern template void DrawContext::textWrite<char>(View<char> str, ivec2* pt) noexcept;
		extern template void DrawContext::textWrite<char>(View<char> str, ivec2* pt, irectwh* rc, bool nodraw) noexcept;
		extern template void DrawContext::textOut<char16>(const char16 * str, size_t len, ivec2 pt) noexcept;
		extern template void DrawContext::textOut<char16>(View<char16> str, ivec2 pt) noexcept;
		extern template void DrawContext::textWrite<char16>(View<char16> str, ivec2* pt) noexcept;
		extern template void DrawContext::textWrite<char16>(View<char16> str, ivec2* pt, irectwh* rc, bool nodraw) noexcept;

		class ObjectSelector
		{
		public:
			ObjectSelector(DrawContext * pDC, HGDIOBJ pObj)
			{
				m_pDC = pDC;
				m_pOld = m_pDC->select(pObj);
			}
			ObjectSelector(const ObjectSelector&) = delete;
			ObjectSelector(ObjectSelector&& mov)
			{
				m_pDC = mov.m_pDC;
				m_pOld = mov.m_pOld;
				mov.m_pOld = nullptr;
			}
			~ObjectSelector()
			{
				if(m_pOld == nullptr) return;
				m_pDC->select(m_pOld);
			}

		private:
			DrawContext * m_pDC;
			HGDIOBJ m_pOld;
		};

	}
}