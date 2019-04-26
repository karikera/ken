#pragma once

#include <KR3/main.h>

#include "handle.h"

namespace kr
{
	class ProcessMemory
	{
	public:
		ProcessMemory(win::Process* hProcess);
		ProcessMemory(win::Process* hProcess, size_t nSize, dword dwFlags);
		~ProcessMemory();

		bool alloc(size_t nSize, dword dwFlags);
		size_t write(LPCVOID pData, size_t nSize);
		size_t read(LPVOID pData, size_t nSize);
		LPVOID getAddress();

	protected:
		win::Process* m_hProcess;
		LPVOID m_pAddress;
		size_t m_nSize;
	};

}
