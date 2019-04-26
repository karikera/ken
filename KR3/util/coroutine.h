#pragma once

#include "../main.h"

namespace kr
{
	class Coroutine;
	class Yield;

	class Yield
	{
		friend Coroutine;
	public:
		void operator ()() noexcept;

	private:
		byte * m_esp;
	};
	class Coroutine
	{
	public:
		Coroutine(void (CT_CDECL *func)(Yield &), size_t stackSize = 8192) noexcept;
		~Coroutine() noexcept;
		void call() noexcept;

	private:
		Yield m_yield;
		const size_t m_stackSize;
	};
}
