#pragma once

#include <kr3/util/time.h>
#include <KR3/util/callable.h>
#include <KR3/util/process.h>
#include <kr3/math/coord.h>
#include <KR3/wl/windows.h>
#include <KR3/wl/handle.h>
#include <KR3/wl/eventhandle.h>
#include <KR3/wl/threadhandle.h>
#include <KR3/util/uts.h>

namespace kr
{
	typedef LRESULT(CT_STDCALL * WndProc)(win::Window * pWnd, uint Msg, WPARAM wParam, LPARAM lParam);
	typedef BOOL(CT_STDCALL * DlgProc)(win::Dialog * dlg, uint msg, WPARAM wParam, LPARAM lParam);

	namespace win
	{
		template <typename T> class GDIObjectBase;
		template <typename T> class GDIObject;
		class ObjectSelector;
		using RGN = GDIObject<HRGN__>;
		using Pen = GDIObject<HPEN__>;
		using Brush = GDIObject<HBRUSH__>;
		using Font = GDIObject<HFONT__>;
		using Bitmap = GDIObject<HBITMAP__>;
		class DrawContext;
	}

	class WindowProgram;

#define OPTIONS_BEGIN(name) class name { using This = name; public: inline name(){}
#define OPTION(type, name) private: type _##name = 0; \
	public: \
	inline type& name(){ return _##name; } \
	inline This& name(const type &value){ _##name = value; return *this; } \
	inline This& name(type&& value) { _##name = move(value); return *this; }
#define OPTIONS_END()	};
	
	namespace win
	{
		OPTIONS_BEGIN(ProcessOptions)
		OPTION(bool, console)
		OPTION(bool, detached)
		OPTION(bool, suspended)
		OPTIONS_END()

		ivec2 getCursorPos() noexcept;

		class ProcessAndModule
		{
		public:
			Process * process;
			Module * winmodule;

			KR_WRITABLE_METHOD(ProcessAndModule, name, true,
				(m_this->getName<C>(dest, m_this->getNameLength<C>() + 1)),
				(m_this->getNameLength<C>()));

			template <typename T> size_t getName(T* dest, size_t capacity) const noexcept;
			template <typename T> size_t getNameLength() const noexcept;
		};

		class Iconable:public Handle<HICON__>
		{
		public:
		};
		class Cursor:public Iconable
		{
		public:
			void operator delete(void * p);
			static Cursor* load(pcstr name);
			static Cursor* load(pcstr16 name);
			static Cursor* load(const wchar_t * name);
			static Cursor* load(uintptr_t id);
			static Cursor* load(Instance* hInstance, uintptr_t id);
			class Current
			{
			public:
			Current& operator =(Cursor * cursor) noexcept;
			operator Cursor*() noexcept;
			};
			static Current current;
		};
		class Icon:public Iconable
		{
		public:
			void operator delete(void * p);
			static Icon* load(pcstr name);
			static Icon* load(pcstr16 name);
			static Icon* load(const wchar_t * name);
			static Icon* load(uintptr_t id);
			static Icon* load(Instance* hInstance, uintptr_t id);
		};
		class Menu:public Handle<HMENU__>
		{
		public:
			static Menu* load(uintptr_t id) noexcept;
			static Menu* loadPick(uintptr_t id, int nPos) noexcept;
			void operator delete(void * p) noexcept;
			BOOL enableItem(uint iItem, uint flags) noexcept;
			DWORD checkItem(UINT uIDCheckItem, UINT uCheck) noexcept;
			BOOL checkRadioItem(UINT first, UINT last, UINT check, UINT flags) noexcept;
			BOOL trackPopup(uint uFlags, ivec2 pt) noexcept;
			Menu* getItem(int nPos) noexcept;
			BOOL removeItem(uint uPos, uint uFlags) noexcept;
			Menu* pickItem(int nPos) noexcept;
		};
		class Accelerator :public Handle<HACCEL__>
		{
		public:
			static Accelerator* load(uintptr_t id) noexcept;
		};
		class Process :public EventHandle
		{
		public:
			struct Pair
			{
				Process* process;
				ThreadHandle* thread;
			};
			using ThreadRoutine = unsigned long (CT_STDCALL *)(void* lpThreadParameter);
			
			static Process * open(ProcessId id) noexcept;
			static Pair execute(pstr command, pcstr currentDirectory = nullptr, ProcessOptions opts = ProcessOptions()) noexcept;
			static Pair execute(pcstr application, pstr command, pcstr currentDirectory, ProcessOptions opts = ProcessOptions()) noexcept;
			static Pair execute(pstr16 command, pcstr16 currentDirectory = nullptr, ProcessOptions opts = ProcessOptions()) noexcept;
			static Pair execute(pcstr16 application, pstr16 command, pcstr16 currentDirectory, ProcessOptions opts = ProcessOptions()) noexcept;

			bool terminate() noexcept;
			Module * getFirstModule() noexcept;
			Module* injectDll(pcstr strDllPath) noexcept;
			Module* injectDll(pcstr16 strDllPath) noexcept;
			dword call(ThreadRoutine pThread, Buffer buffer) noexcept;
			dword call(ThreadRoutine pThread, intptr_t data) noexcept;
			size_t write(void * pDest, const void *pSrc, size_t nSize) noexcept;
			size_t read(void * pDest, const void *pSrc, size_t nSize) noexcept;
		};
		class Monitor :public Handle<HMONITOR__>
		{
		public:
			const irect getWorkArea() noexcept;
			const irect getMonitorArea() noexcept;
		};
		class Window:public Handle<HWND__>
		{
		public:
			static const Window * TOP;
			static const Window * BOTTOM;
			static const Window * TOPMOST;
			static const Window * NOTOPMOST;

			void operator delete(void * p) noexcept;
			static bool adjustRect(irect * rc, int style, bool menu) noexcept;
			bool adjustRect(irect * rc) noexcept;
			bool resizeToCenter(int width, int height) noexcept;
			long setLong(int index, long value) noexcept;
			long getLong(int index) noexcept;
			intptr_t setLongPtr(int index, intptr_t value) noexcept;
			intptr_t getLongPtr(int index) noexcept;
			WndProc setProc(WndProc proc) noexcept;
			WndProc getProc() noexcept;
			long setStyle(long style) noexcept;
			long getStyle() noexcept;
			long addStyle(long style) noexcept;
			long setExStyle(long style) noexcept;
			long addExStyle(long style) noexcept;
			long getExStyle() noexcept;
			long removeStyle(long style) noexcept;
			long removeExStyle(long style) noexcept;
			WindowProgram* setProgram(WindowProgram * pProgram) noexcept;
			WindowProgram* getProgram() noexcept;

			bool clientToScreen(ivec2* lpPoint) noexcept;
			bool screenToClient(ivec2* lpPoint) noexcept;
			bool setPos(ivec2 pos) noexcept;
			bool setSize(ivec2 size) noexcept;
			bool setPos(Window* hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) noexcept;
			bool move(const irectwh &irectwh, bool bRepaint=false) noexcept;
			bool move(int X, int Y, int nWidth, int nHeight, bool bRepaint=false) noexcept;
			bool resize(int width, int height, bool repaint = false) noexcept;
			bool show(int nCmdShow) noexcept;
			bool update() noexcept;
			int setRgn(RGN* rgn, bool bRedraw = false) noexcept;
			UINT_PTR setTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) noexcept;
			bool killTimer(UINT_PTR uIDEvent) noexcept;
			bool invalidateRect(const irect& rect, bool bErase = false) noexcept;
			bool invalidate(bool bErase = false) noexcept;
			bool validateRect(const irect& rect) noexcept;
			bool validate() noexcept;

			irect getRect() noexcept;
			irect getClientRect() noexcept;
			Monitor* getMonitor() noexcept;
			ivec2 getPos() noexcept;

			KR_WRITABLE_METHOD(Window, text, true,
				(m_this->getText<C>(dest, m_this->getTextLength<C>() + 1)),
				(m_this->getTextLength<C>())
			);

			DrawContext* beginPaint(PAINTSTRUCT *ps) noexcept;
			BOOL endPaint(PAINTSTRUCT *ps) noexcept;
			DrawContext* getDC() noexcept;
			int releaseDC(DrawContext * dc) noexcept;

			bool updateLayer(DrawContext* dc, const irectwh & region) noexcept;
			bool updateLayer(DrawContext* dc, int width, int height) noexcept;

			BOOL postMessage(UINT Msg, WPARAM wParam, LPARAM lParam) noexcept;
			BOOL postMessageA(UINT Msg, WPARAM wParam, LPARAM lParam) noexcept;
			LRESULT sendMessageA(UINT Msg, WPARAM wParam, LPARAM lParam) noexcept;
			LRESULT sendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) noexcept;

			void setIcon(Icon* icon) noexcept;
			Icon* setIcon(Icon* icon, bool iconBig) noexcept;
			Icon* getIcon() noexcept;
			Font* setFont(Font* pFont) noexcept;
			Font* getFont() noexcept;
			BOOL enable(bool bEnable) noexcept;

			int msgBox(pcstr strMessage, pcstr strTitle, uint flags) noexcept;
			int msgBox(pcstr16 strMessage, pcstr16 strTitle, uint flags) noexcept;
			int errorBox(pcstr strMessage) noexcept;
			int errorBox(pcstr16 strMessage) noexcept;
			int informationBox(pcstr16 strMessage) noexcept;

			template <typename T> size_t getText(T* dest, size_t capacity) const noexcept;
			template <typename T> size_t getTextLength() const noexcept;
			int setText(pcstr src) noexcept;
			int setText(pcstr16 src) noexcept;

			bool isCaptured() noexcept;
			Window* setCapture() noexcept;
			bool releaseCapture() noexcept;
			bool setMenu(Menu * menu) noexcept;
			Menu * getMenu() noexcept;

			BOOL trackMouseEvent(dword dwFlags) noexcept;
			ivec2 getCursorPos() noexcept;

			int getScrollPos(int nBar) noexcept;
			int getScrollY() noexcept;
			int getScrollX() noexcept;
			int getScrollCtlPos() noexcept;

			bool moveScrollPos(int nBar, int scroll, bool redraw = false) noexcept;
			bool moveScrollY(int scroll, bool redraw = false) noexcept;
			bool moveScrollX(int scroll, bool redraw = false) noexcept;
			bool moveScrollCtlPos(int scroll, bool redraw = false) noexcept;

			int setScrollPos(int nBar, int scroll, bool redraw = false) noexcept;
			int setScrollY(int scroll, bool redraw = false) noexcept;
			int setScrollX(int scroll, bool redraw = false) noexcept;
			int setScrollCtlPos(int scroll, bool redraw = false) noexcept;

			int setScroll(int nBar, int page, int range, bool redraw = false) noexcept;
			int setScrollHorz(int page, int range, bool redraw = false) noexcept;
			int setScrollVert(int page, int range, bool redraw = false) noexcept;
			int setScrollCtl(int page, int range, bool redraw = false) noexcept;

			int getScrollPage(int nBar) noexcept;
			int getScrollYPage() noexcept;
			int getScrollXPage() noexcept;
			int getScrollCtlPage() noexcept;

			int setScrollPage(int nBar, int page, bool redraw = false) noexcept;
			int setScrollYPage(int page, bool redraw = false) noexcept;
			int setScrollXPage(int page, bool redraw = false) noexcept;
			int setScrollCtlPage(int page, bool redraw = false) noexcept;

			int getScrollRange(int nBar) noexcept;
			int getScrollYRange() noexcept;
			int getScrollXRange() noexcept;
			int getScrollCtlRange() noexcept;

			bool setScrollRange(int nBar, int min, int max, bool redraw = false) noexcept;
			bool setScrollYRange(int min, int max, bool redraw = false) noexcept;
			bool setScrollXRange(int min, int max, bool redraw = false) noexcept;
			bool setScrollCtlRange(int min, int max, bool redraw = false) noexcept;

			Window* get(UINT uCmd) noexcept;
			Window* getNext() noexcept;
			Window* getNextTab() noexcept;
			Window* getPrevious() noexcept;
			Window* getFirst() noexcept;
			Window* getLast() noexcept;
			Window* getOwner() noexcept;
			Window* getChild() noexcept;
			Window* getParent() noexcept;
			Window* setParent(Window* pParent) noexcept;
			Window* appendChild(Window* child) noexcept;
			Window* setFocus() noexcept;
			template <typename T> T* getParent() noexcept
			{
				return static_cast<T*>(getParent());
			}

			Static * insertStatic(pcstr16 pszText, irectwh rc) noexcept;
			struct ProcessBag {
				ThreadId threadId;
				ProcessId processId;
			};
			ProcessBag getProcessId() noexcept;
			static Window * find(pcstr16 className, pcstr16 windowName) noexcept;
			static void all(CallableT<bool(Window*)> * call) noexcept;
			template <typename LAMBDA>
			static void all(LAMBDA &&lambda) noexcept
			{
				LambdaCallable<bool(Window*), decay_t<LAMBDA> > call(forward<LAMBDA>(lambda));
				all(&call);
			}
			static Window * getForeground() noexcept;
			static Window* createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, dword width, dword height, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimary(pcstr16 pszClass, pcstr16 pszTitle, dword style, irectwh rect, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, dword width, dword height, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimaryEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword style, irectwh rect, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createPrimaryAsFull(pcstr16 pszClass, pcstr16 pszTitle, dword style, HMENU hMenu = nullptr, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc = { { 0, 0 },{ 0, 0 } }, Window* pParent = nullptr, intptr_t nID = 0, WindowProgram * pProgram = nullptr) noexcept;
			static Window* createEx(dword nExStyle, pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc , Window* pParent, HMENU hMenu, WindowProgram * pProgram = nullptr) noexcept;
			static Window* create(pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc = { {0, 0},{ 0, 0 } }, Window* pParent = nullptr, intptr_t nID = 0, WindowProgram * pProgram = nullptr) noexcept;
			static Window* create(pcstr16 pszClass, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, HMENU hMenu, WindowProgram * pProgram = nullptr) noexcept;
		};

		class Dialog :public Window
		{
		public:
			static Dialog* create(int id, DlgProc proc, Window * parent = nullptr) noexcept;
			static intptr_t modal(int id, DlgProc proc, Window * parent = nullptr) noexcept;
			static intptr_t modal(int id, DlgProc proc, Window * parent, LPARAM param) noexcept;
			static intptr_t modalA(int id, DlgProc proc, Window * parent = nullptr) noexcept;
			static intptr_t modalA(int id, DlgProc proc, Window * parent, LPARAM param) noexcept;
			bool endDialog(intptr_t retcode) noexcept;
			bool setItemText(int id, pcstr text) noexcept;
			bool setItemText(int id, pcstr16 text) noexcept;
			bool checkItemButton(int id, uint check) noexcept;
			bool checkRadioButton(int first, int last, int target) noexcept;
			uint isItemButtonChecked(int id) noexcept;
			Window* getItem(int id) noexcept;
			template <typename T>
			T* getItem(int id) noexcept
			{
				return static_cast<T*>(getItem(id));
			}
		};

		class Button:public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static Button* createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static Button* create(pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;

			bool isChecked() noexcept;
			void setCheck(bool checked) noexcept;
			void setState(int state) noexcept;
			int getState() noexcept;
		};

		class Static:public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static Static* createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent) noexcept;
			static Static* create(pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent) noexcept;
		};

		class EditBox:public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static EditBox* createEx(dword nExStyle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static EditBox* create(dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static EditBox* createEx(dword nExStyle, pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static EditBox* create(pcstr16 pszTitle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;

			void setSelect(size_t from, size_t to) noexcept;
			void setSelect(size_t sel) noexcept;
			void replaceSelect(pcstr16 text, bool canBeUndone = false);
		};

		class ComboBox :public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static ComboBox* createEx(dword nExStyle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static ComboBox* create(dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;

			void addString(pcstr16 text) noexcept;
			void setCursor(int idx) noexcept;
			int getCursor() noexcept;
		};

		class ListBox:public Window
		{
		public:
			static const char16 CLASS_NAME[];
			static ListBox* createEx(dword nExStyle, dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
			static ListBox* create(dword nStyle, irectwh rc, Window* pParent, intptr_t nID = 0) noexcept;
		
			size_t addString(pcstr str) noexcept;
			size_t addString(pcstr16 str) noexcept;
		};

		static class g_cursor_t
		{
		public:
			Cursor* operator =(Cursor * cursor) noexcept;
			operator Cursor*() noexcept;

		} g_cursor;


		extern Window * g_mainWindow;
		extern Instance * g_instance;

	}
	
}
