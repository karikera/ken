#include "stdafx.h"
#include "logger.h"
#include <stdarg.h>
#include <KR3/mt/criticalsection.h>

kr::LogManager kr::g_log;

static kr::CriticalSection s_logLock;

#pragma warning(disable:4996)

template <typename S>
void formatout(S * stream, const char * format, va_list vl)
{
#ifdef _MSC_VER
	size_t len = _vscprintf(format, vl);
#else
	size_t len = vsnprintf(NULL, 0, format, vl);
#endif
	char * dest = stream->padding(len + 1);
	vsprintf(dest, format, vl);
	stream->commit(len);
}

void kr::LogManager::write(kr::Text text) noexcept
{
	kr::CsLock _lock = s_logLock;
	for (io::VOStream<char> & out : *this)
		out.write(text.begin(), text.size());
}
void kr::LogManager::printf(pcstr agent, pcstr format, ...) noexcept
{
	kr::CsLock _lock = s_logLock;
	va_list vl;
	va_start(vl, format);

	TSZ buffer;
	buffer << '[' << agent << "] ";
	formatout(&buffer, format, vl);
	buffer << "\r\n";
	write(buffer);
}
void kr::LogManager::puts(pcstr agent, pcstr str) noexcept
{
	TSZ buffer;
	buffer << '[' << agent << "] " << str << "\r\n";
	write(buffer);
}
void kr::LogManager::putch(char chr) noexcept
{
	kr::CsLock _lock = s_logLock;
	for (io::VOStream<char> & out : *this)
		out.write(&chr, 1);
}
kr::Logger* kr::LogManager::pushStdOut() noexcept
{
	kr::CsLock _lock = s_logLock;
	return create(&kr::cout);
}
kr::Logger* kr::LogManager::pushErrOut() noexcept
{
	kr::CsLock _lock = s_logLock;
	return create(&kr::cerr);
}