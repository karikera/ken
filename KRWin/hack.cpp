#include "stdafx.h"
#include "hack.h"

kr::ProcessMemory::ProcessMemory(win::Process* process) noexcept
{
	m_hProcess = process;
	m_nSize = 0;
	m_pAddress = nullptr;
}
kr::ProcessMemory::ProcessMemory(win::Process* process, size_t size) noexcept
{
	m_hProcess = process;
	alloc(size);
}
kr::ProcessMemory::ProcessMemory(win::Process* process, Buffer buffer) noexcept
{
	m_hProcess = process;
	alloc(buffer);
}
kr::ProcessMemory::~ProcessMemory() noexcept
{
	if (m_pAddress != nullptr) VirtualFreeEx(m_hProcess, m_pAddress, 0, MEM_RELEASE);
}
bool kr::ProcessMemory::alloc(size_t size) noexcept
{
	m_nSize = size;
	m_pAddress = VirtualAllocEx(m_hProcess, nullptr, size, MEM_COMMIT, PAGE_READWRITE);
	return (m_pAddress != nullptr);
}
size_t kr::ProcessMemory::alloc(Buffer buffer) noexcept
{
	if (!alloc(buffer.size())) return 0;
	return write(buffer);
}
size_t kr::ProcessMemory::write(Buffer data) noexcept
{
	SIZE_T size = data.size();
	WriteProcessMemory(m_hProcess, m_pAddress, data.data(), data.size(), &size);
	return size;
}
size_t kr::ProcessMemory::read(void* data, size_t size) noexcept
{
	static_assert(sizeof(SIZE_T) == sizeof(size_t), "unmatch size");
	ReadProcessMemory(m_hProcess, m_pAddress, data, size, (SIZE_T*)&size);
	return size;
}
LPVOID kr::ProcessMemory::getAddress() noexcept
{
	return m_pAddress;
}
