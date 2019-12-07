#include "stdafx.h"
#include "windows.h"

#include <KR3/util/wide.h>

using namespace kr;

ModuleName<char>::ModuleName(const char* name) noexcept
	:m_module(GetModuleHandleA(name))
{
}
ModuleName<char16>::ModuleName(const char16* name) noexcept
	:m_module(GetModuleHandleW(wide(name)))
{
}

size_t ModuleName<char>::$copyTo(char* dest) const noexcept
{
	return GetModuleFileNameA((HMODULE)m_module, dest, MAX_PATH);
}
size_t ModuleName<char16>::$copyTo(char16* dest) const noexcept
{
	return GetModuleFileNameW((HMODULE)m_module, wide(dest), MAX_PATH);
}
