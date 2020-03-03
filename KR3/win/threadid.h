#pragma once

#include "minidef.h"

namespace kr
{

	class ThreadId
	{
		friend MessageThreadId;
		friend ThreadHandle;
	public:
		ThreadId() = default;
		ThreadId(nullptr_t) noexcept;
		ThreadId(dword id) noexcept;
		bool quit(int exitCode) noexcept;
		bool postMessage(int msg, WPARAM wParam, LPARAM lParam) noexcept;
		void setName(pcstr name) noexcept;
		dword value() noexcept;
		static ThreadId getCurrent() noexcept;

		bool operator == (const ThreadId& id) const noexcept;
		bool operator != (const ThreadId& id) const noexcept;
		bool operator == (nullptr_t) const noexcept;
		bool operator != (nullptr_t) const noexcept;
	private:
		dword m_id;
	};

}