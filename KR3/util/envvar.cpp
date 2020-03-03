#include "stdafx.h"
#include "envvar.h"

using namespace kr;

#ifdef WIN32

#include <KR3/win/windows.h>

#pragma warning(disable:4800)

EnviromentVariableT<char>::EnviromentVariableT(const char * szname) noexcept
	:m_name(szname)
{
	m_size = GetEnvironmentVariableA(m_name, nullptr, 0) - 1;
	if (m_size == -1)
		m_size = 0;
}
EnviromentVariableT<char16>::EnviromentVariableT(const char16 * szname) noexcept
	: m_name(szname)
{
	m_size = GetEnvironmentVariableW(wide(m_name), nullptr, 0) - 1;
	if (m_size == -1)
		m_size = 0;
}
bool EnviromentVariableT<char>::set(const char * dest)  noexcept
{
	return SetEnvironmentVariableA(m_name, dest);
}
bool EnviromentVariableT<char16>::set(const char16 * dest)  noexcept
{
	return SetEnvironmentVariableW(wide(m_name), wide(dest));
}

size_t EnviromentVariableT<char>::$copyTo(char * dest) const  noexcept
{
	GetEnvironmentVariableA(m_name, dest, m_size + 1);
	return m_size;
}
size_t EnviromentVariableT<char16>::$copyTo(char16 * dest) const  noexcept
{
	GetEnvironmentVariableW(wide(m_name), wide(dest), m_size + 1);
	return m_size;
}

#else

template <>
EnviromentVariableT<char>::EnviromentVariableT(const char * szname) noexcept
	:m_name(szname)
{
	char * var = getenv(m_name);
	if (var == nullptr)
		m_size = 0;
	else
		m_size = strlen(var);
}
template <>
bool EnviromentVariableT<char>::set(const char * dest)  noexcept
{
	return putenv(TSZ() << m_name << '=' << dest) != 0;
}
template <>
size_t EnviromentVariableT<char>::$copyTo(char * dest) const  noexcept
{
	memcpy(dest, getenv(m_name), m_size + 1);
	return m_size;
}

#endif