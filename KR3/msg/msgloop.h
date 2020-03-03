#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/win/windows.h>
#include <KR3/math/coord.h>
#include <KR3/data/linkedlist.h>

namespace kr
{
	class MessageStruct
	{
	public:
		win::Window*	window;
		uint	message;
		WPARAM	wParam;
		LPARAM	lParam;
		DWORD	time;
		ivec2	point;

#ifdef _MAC
		DWORD       lPrivate;
#endif
		LPMSG getMessageStruct() noexcept;

		ivec2 getPointFromParam() const noexcept;
		bool isDlgMessageA(HWND wnd) noexcept;
		bool isDlgMessage(HWND wnd) noexcept;

		bool get() noexcept;
		bool peek() noexcept;
		bool tryGet() noexcept;
		bool translate() const noexcept;
		LRESULT dispatch() const noexcept;
	};

	class Translator:public Node<Translator, true>
	{
	public:
		virtual bool translate(const MessageStruct* pMsg) throws(QuitException) = 0;

	};
	
	class AcceleratorTranslator :public Translator
	{
		virtual bool translate(const MessageStruct* pMsg) noexcept override;

	public:
		AcceleratorTranslator(HWND hWnd, HACCEL hAccTable) noexcept;

		HWND m_hWnd;
		HACCEL m_hAccTable;
	};

	class BasicTranslator :public Translator
	{
		virtual bool translate(const MessageStruct* pMsg) noexcept override;
	};

	class Message;

	class MessageLoop :public Chain<Translator>
	{
	public:
		static MessageLoop * getInstance() noexcept;

		MessageLoop() noexcept;
		~MessageLoop() noexcept;

		void attachAccelerator(HWND hWnd, HACCEL hAccel) noexcept;
		void attachBasicTranslator() noexcept;
		bool get() noexcept;
		bool tryGet() noexcept;
		bool peek() noexcept;

		void dispatch() throws(QuitException);

		int messageLoop() noexcept;
		uint getLastMessage() noexcept;

	private:
		MessageStruct m_msg;
		
	};
	
	ivec2 getPointFromParam(LPARAM lParam) noexcept;
}