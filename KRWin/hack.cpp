#include "stdafx.h"
#include "hack.h"

kr::ProcessMemory::ProcessMemory(win::Process* hProcess)
{
	m_hProcess = hProcess;
	m_nSize = 0;
	m_pAddress = nullptr;
}
kr::ProcessMemory::ProcessMemory(win::Process* hProcess, size_t nSize, kr::dword dwFlags)
{
	m_hProcess = hProcess;
	alloc(nSize, dwFlags);
}
kr::ProcessMemory::~ProcessMemory()
{
	if (m_pAddress != nullptr) VirtualFreeEx(m_hProcess, m_pAddress, m_nSize, MEM_RELEASE);
}

bool kr::ProcessMemory::alloc(size_t nSize, kr::dword dwFlags)
{
	m_nSize = nSize;
	m_pAddress = VirtualAllocEx(m_hProcess, nullptr, nSize, MEM_COMMIT, dwFlags);
	return (m_pAddress != nullptr);
}
size_t kr::ProcessMemory::write(LPCVOID pData, size_t nSize)
{
	WriteProcessMemory(m_hProcess, m_pAddress, pData, nSize, (SIZE_T*)&nSize);
	return nSize;
}
size_t kr::ProcessMemory::read(LPVOID pData, size_t nSize)
{
	ReadProcessMemory(m_hProcess, m_pAddress, pData, nSize, (SIZE_T*)&nSize);
	return nSize;
}
LPVOID kr::ProcessMemory::getAddress()
{
	return m_pAddress;
}
