#include "stdafx.h"
#include "path.h"

using namespace kr;

#ifndef NO_USE_FILESYSTEM

#ifdef WIN32

#include <KR3/wl/windows.h>

template <> bool CurrentDirectory::set<char>(const char * text) const noexcept
{
	return SetCurrentDirectoryA(text) != FALSE;
}
template <> size_t CurrentDirectory::$copyTo<char>(char * dest) const noexcept
{
	return GetCurrentDirectoryA(MAX_PATH, dest);
}
template <> size_t CurrentDirectory::$sizeAs<char>() const noexcept
{
	return GetCurrentDirectoryA(0, nullptr) - 1;
}

template <> bool CurrentDirectory::set<char16>(const char16 * text) const noexcept
{
	return SetCurrentDirectoryW(wide(text)) != FALSE;
}
template <> size_t CurrentDirectory::$copyTo<char16>(char16 * dest) const noexcept
{
	return GetCurrentDirectoryW(MAX_PATH, wide(dest));
}
template <> size_t CurrentDirectory::$sizeAs<char16>() const noexcept
{
	return GetCurrentDirectoryW(0, nullptr) - 1;
}

template <> bool CurrentDirectory::set<wchar_t>(const wchar_t * text) const noexcept
{
	return SetCurrentDirectoryW(text) != FALSE;
}
template <> size_t CurrentDirectory::$copyTo<wchar_t>(wchar_t * dest) const noexcept
{
	return GetCurrentDirectoryW(MAX_PATH, dest);
}
template <> size_t CurrentDirectory::$sizeAs<wchar_t>() const noexcept
{
	return GetCurrentDirectoryW(0, nullptr) - 1;
}
#else

#error Need implement

#endif

#else

namespace
{
	AText s_virtualCurrentPath;
}

template <> bool CurrentDirectory::set<char>(const char * text) const noexcept
{
	s_virtualCurrentPath = (Text)text;
	return true;
}
template <> size_t CurrentDirectory::$copyTo<char>(char * dest) const noexcept
{
	s_virtualCurrentPath = (Text)dest;
	return s_virtualCurrentPath.size();
}
template <> size_t CurrentDirectory::$sizeAs<char>() const noexcept
{
	return s_virtualCurrentPath.size();
}

template <> bool CurrentDirectory::set<char16>(const char16 * text) const noexcept
{
	s_virtualCurrentPath = toAnsi((Text16)text);
	return true;
}
template <> size_t CurrentDirectory::$copyTo<char16>(char16 * dest) const noexcept
{
	return acpToUtf16((Text)"").copyTo(dest);
}
template <> size_t CurrentDirectory::$sizeAs<char16>() const noexcept
{
	return acpToUtf16(s_virtualCurrentPath).size();
}

#endif


Path::Path() noexcept
{
	m_path = m_buffer;
}
Text16 Path::get(Text16 filename) throws(NotEnoughSpaceException)
{
	size_t len = filename.size();
	if (m_buffer + MAX_LEN < m_path + len + 1)
	{
		throw NotEnoughSpaceException();
	}
	memcpy(m_path, filename.begin(), len * sizeof(wchar_t));
	return Text16(m_buffer, m_path + len);
}
pcstr16 Path::getsz(Text16 filename) throws(NotEnoughSpaceException)
{
	get(filename);
	m_path[filename.size()] = '\0';
	return m_buffer;
}
char16 * Path::enter(Text16 name) throws(NotEnoughSpaceException)
{
	char16 * opath = m_path;
	size_t len = name.size();
	if (m_buffer + MAX_LEN < m_path + len + 1)
	{
		throw NotEnoughSpaceException();
	}
	memcpy(m_path, name.begin(), len * sizeof(wchar_t));

	m_path += len;
	*m_path++ = '\\';
	return opath;
}
void Path::leave(char16 * path) noexcept
{
	m_path = path;
}
pcstr16 Path::getCurrentDirectorySz() noexcept
{
	*m_path = '\0';
	return m_buffer;
}

template class kr::path_t<char>;
template class kr::path_t<char16>;
