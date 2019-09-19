#pragma once

#include <KR3/main.h>

#include "handle.h"

namespace kr
{
	class ProcessMemory
	{
	public:
		ProcessMemory(win::Process* process) noexcept;
		ProcessMemory(win::Process* process, size_t size) noexcept;
		ProcessMemory(win::Process* process, Buffer buffer) noexcept;
		~ProcessMemory() noexcept;

		bool alloc(size_t size) noexcept;
		size_t alloc(Buffer buffer) noexcept;
		size_t write(Buffer buffer) noexcept;
		size_t read(void* data, size_t size) noexcept;
		void* getAddress() noexcept;

	protected:
		win::Process* m_hProcess;
		void* m_pAddress;
		size_t m_nSize;
	};

}
