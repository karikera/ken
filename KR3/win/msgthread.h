#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include "threadhandle.h"
#include "windows.h"

namespace kr
{

	class MessageThreadId :public ThreadId
	{
	public:
		MessageThreadId() = default;
		MessageThreadId(const ThreadId &threadid) noexcept;
		bool postMessage(uint msg, WPARAM wParam, LPARAM lParam) noexcept;
		bool postMessageA(uint msg, WPARAM wParam, LPARAM lParam) noexcept;
		bool quit(int exitCode) noexcept;
	};

}