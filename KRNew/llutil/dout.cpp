#include "stdafx.h"
#include "dout.h"

#ifdef WIN32

#include <KR3/wl/windows.h>

kr::LLDebugOutput::LLDebugOutput() noexcept
{
	m_dest = m_temp;
}
kr::LLDebugOutput::~LLDebugOutput() noexcept
{
	flush();
}
void kr::LLDebugOutput::putSourceLine(const char * src, int line) noexcept
{
	*this << src << '(' << line << ")\r\n";
}

kr::LLDebugOutput& kr::LLDebugOutput::operator <<(char chr) noexcept
{
	if (_destend() == m_dest)
		flush();
	*m_dest++ = chr;
	return *this;
}
kr::LLDebugOutput& kr::LLDebugOutput::operator <<(const char * src) noexcept
{
	write(src, strlen(src));
	return *this;
}
kr::LLDebugOutput& kr::LLDebugOutput::operator <<(int n) noexcept
{
	char temp[10];
	char * ptr = temp + 10;

	do
	{
		*--ptr = (char)((n % 10) + '0');
		n /= 10;
	}
	while (n != 0);
	write(ptr, temp + 10 - ptr);
	return *this;
}

size_t kr::LLDebugOutput::write(const char * src, size_t len) noexcept
{
	char * destend = _destend();
	size_t left = destend - m_dest;
	if(left < len)
	{
		memcpy(m_dest, src, left);
		m_dest = destend;
		flush();

		const char * srcend = src + len;
		src += left;
		while (BUFFERSIZE < srcend - src)
		{
			memcpy(m_temp, src, BUFFERSIZE);
			m_dest = destend;
			flush();
			src += BUFFERSIZE;
		}
		len = srcend - src;
	}
	memcpy(m_dest, src, len);
	m_dest += len;
	return len;
}
void kr::LLDebugOutput::flush() noexcept
{
	*m_dest = '\0';
	OutputDebugStringA(m_temp);
	m_dest = m_temp;
}
char * kr::LLDebugOutput::_destend() noexcept
{
	return m_temp + BUFFERSIZE;
}

#else

#include <iostream>

using std::cout;

kr::LLDebugOutput::LLDebugOutput() noexcept
{
}
kr::LLDebugOutput::~LLDebugOutput() noexcept
{
	flush();
}
void kr::LLDebugOutput::putSourceLine(const char * src, int line) noexcept
{
	*this << src << '(' << line << ")\r\n";
}

kr::LLDebugOutput& kr::LLDebugOutput::operator <<(char chr) noexcept
{
	cout << chr;
	return *this;
}
kr::LLDebugOutput& kr::LLDebugOutput::operator <<(const char * src) noexcept
{
	write(src, strlen(src));
	return *this;
}
kr::LLDebugOutput& kr::LLDebugOutput::operator <<(int n) noexcept
{
	char temp[10];
	char * ptr = temp + 10;

	do
	{
		*--ptr = (char)((n % 10) + '0');
		n /= 10;
	}
	while (n != 0);
	write(ptr, temp + 10 - ptr);
	return *this;
}

size_t kr::LLDebugOutput::write(const char * src, size_t len) noexcept
{
	cout.write(src, len);
	return len;
}
void kr::LLDebugOutput::flush() noexcept
{
	cout.flush();
}
#endif