#include "stdafx.h"

#ifdef WIN32

#include "handle.h"
#include "windows.h"

#include <KR3/util/wide.h>

using namespace kr;
using namespace win;

Library* Library::load(pcstr16 str) noexcept
{
	return (Library*)LoadLibraryW(wide(str));
}
Module* Library::byName(pcstr16 str) noexcept
{
	return (Module*)GetModuleHandleW(wide(str));
}
Module* Library::current() noexcept
{
	return byName(nullptr);
}
void Library::operator delete(void* library) noexcept
{
	FreeLibrary((HMODULE)library);
}
const autovar<sizeof(ptr)> Library::get(pcstr str) noexcept
{
	FARPROC t = GetProcAddress(this, str);
	return t;
}
template <> size_t Library::getFileName<char>(char* dest, size_t capacity) const noexcept
{
	return GetModuleFileNameA(const_cast<Library*>(this), dest, (DWORD)mint(capacity, Path::MAX_LEN));
}
template <> size_t Library::getFileName<char16_t>(char16_t* dest, size_t capacity) const noexcept
{
	return GetModuleFileNameW(const_cast<Library*>(this), wide(dest), (DWORD)mint(capacity, Path::MAX_LEN));
}

void kr::_pri_::closeHandle(void * handle) noexcept
{
	::CloseHandle((HANDLE)handle);
}

#endif