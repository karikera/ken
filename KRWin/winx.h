#pragma once

#include <KR3/win/handle.h>
#include <KR3/msg/msgloop.h>
#include <KR3/data/linkedlist.h>
#include <KR3/math/coord.h>
#include "handle.h"

namespace kr
{
	constexpr int IDN_KCONTROL = 3;

	class WndProcedure:public Node<WndProcedure, true>
	{
	public:
		virtual void wndProc(win::Window* wnd, uint Msg, WPARAM wParam, LPARAM lParam) throws(LRESULT) =0;

	};

	class WndProcWrapper:public WndProcedure
	{
	public:
		WndProcWrapper();
		WndProcWrapper(WndProc pProc);

		virtual void wndProc(win::Window* wnd, uint Msg, WPARAM wParam, LPARAM lParam) throws(LRESULT) override;

	private:
		WndProc m_pNextProc;
	};

	class WindowProgram:private WndProcedure
	{
	public:
		Chain<WndProcedure> procedures;

		WindowProgram() noexcept;
		WindowProgram(win::Window* pWindow) noexcept;
		WindowProgram(const WindowProgram&) = delete;
		WindowProgram& operator =(const WindowProgram&) = delete;
		virtual ~WindowProgram() noexcept;
		win::Window* detachWindow() noexcept;
		win::Window* getWindow() noexcept;
		win::Window* createPrimary(pcstr16 title, int style, const irectwh & rc) noexcept;
		win::Window* createPrimary(pcstr16 title, int style, int width, int height) noexcept;
		win::Window* createPrimary(pcstr16 title, int style) noexcept;
		void create(pcstr16 title, int style, const irectwh & rc) noexcept;
		win::Window* createPrimaryEx(int exstyle, pcstr16 title, int style, const irectwh & rc) noexcept;
		win::Window* createPrimaryEx(int exstyle, pcstr16 title, int style, int width, int height) noexcept;
		win::Window* createPrimaryEx(int exstyle, pcstr16 title, int style) noexcept;
		void createEx(int exstyle, pcstr16 title, int style, const irectwh & rc) noexcept;
		void destroy() noexcept;

		static ATOM registerClass(HICON icon, uint style = CS_HREDRAW | CS_VREDRAW) noexcept;
		static ATOM registerClass(int icon, uint style = CS_HREDRAW | CS_VREDRAW) noexcept;
		static ATOM registerClass(pcstr16 className, HICON icon, uint style = CS_HREDRAW | CS_VREDRAW) noexcept;
		static ATOM registerClass(pcstr16 className, int icon, uint style = CS_HREDRAW | CS_VREDRAW) noexcept;
		template <typename T, typename... ARGS> T* changeTo(ARGS... args) noexcept
		{
			MUST_BASE_OF(T, WindowProgram);
			win::Window* pWindow = detachWindow();
			delete this;
			return _new T(pWindow, args...);
		}

	protected:
		Keep<win::Window> m_window;

		virtual void wndProc(win::Window* wnd, uint Msg, WPARAM wParam, LPARAM lParam) throws(LRESULT) override;
		static LRESULT CALLBACK _wndProc(win::Window* pWindow, uint Msg, WPARAM wParam, LPARAM lParam) noexcept;
	};

	class ExTranslator :public Translator
	{
		virtual bool translate(const MessageStruct* pMsg) override;
	};

	class SubClassingProgram:public WindowProgram
	{
	public:
		SubClassingProgram();
		SubClassingProgram(win::Window* pWindow);
		SubClassingProgram(const SubClassingProgram&) = delete;
		SubClassingProgram& operator =(const SubClassingProgram&) = delete;
		~SubClassingProgram();

	private:
		WndProcWrapper m_nextProc;
	};

	class WindowClass
	{
	public:
		WindowClass() noexcept;
		WindowClass(pcstr16 pszClassName, WndProc pfnWndProc, HICON hIcon, uint style = CS_HREDRAW | CS_VREDRAW, int wndExt = 0, int clsExt = 0) noexcept;
		WindowClass(pcstr16 pszClassName, WndProc pfnWndProc, int nIcon, uint style = CS_HREDRAW | CS_VREDRAW, int wndExt = 0, int clsExt = 0) noexcept;

		ATOM registerClass() const noexcept;

		static bool unregister(pcstr16 name) noexcept;

	private:
		WNDCLASSEXW m_wc;
	};

	const wchar_t * makeIntResource(int res) noexcept;

	irect getMonitorRectFromIndex(dword index) noexcept;
	irect getMonitorRectFromCursor() noexcept;
	irectwh calculateWindowPos(dword style, dword w, dword h);
	irectwh calculateWindowPos(dword w, dword h);
	void visiblePrimaryWindow(dword style, dword w, dword h);
}