#include "stdafx.h"
#include "stdio.h"

#include "../mt/criticalsection.h"
#include "../mt/thread.h"

#include <iostream>
#include <codecvt>

#ifdef WIN32
#include <KR3/win/windows.h>
#endif
#include "../util/wide.h"

#pragma warning(disable:4074)
#pragma init_seg(compiler)

using namespace kr;

namespace
{
#ifdef WIN32
#ifndef NDEBUG
	BText16<1024> s_buffer;
	CriticalSection s_cs;
	ThreadHandle * s_thread;
	EventHandle * s_quitThread;
#endif
#endif
}

StandardOutput &kr::cout = StandardOutput::out;
StandardOutput16 &kr::ucout = StandardOutput16::out;
StandardErrorOutput &kr::cerr = StandardErrorOutput::out;
StandardErrorOutput16 &kr::ucerr = StandardErrorOutput16::out;
DebugOutput &kr::dout = DebugOutput::out;
DebugOutput16 &kr::udout = DebugOutput16::out;

void kr::dumpMemory(void * addr, size_t size) noexcept
{
	byte* p = (byte*)addr;
	if (size == 0) return;
	size--;
	for (size_t i = 0; i < size; i++)
	{
		byte d = *p++;
		cout << hexf(d, 2) << ' ';
		if (i % 16 == 15) cout << endl;
		else if (i % 8 == 7) cout << ' ';
	}
	cout << hexf(*p, 2) << endl;
}

template <>
void StandardOutput::flush() noexcept
{
	std::cout.flush();
}
template <>
void StandardOutput::$write(const char *chr, size_t sz) noexcept
{
	std::cout.write(chr, sz);
}
template <>
void StandardErrorOutput::flush() noexcept
{
	std::cerr.flush();
}
template <>
void StandardErrorOutput::$write(const char *chr, size_t sz) noexcept
{
	std::cerr.write(chr, sz);
}
template <>
void StandardOutput16::flush() noexcept
{
	std::wcout.flush();
}
template <>
void StandardOutput16::$write(const char16 *chr, size_t sz) noexcept
{
	if (sizeof(wchar_t) == sizeof(char16_t))
	{
		auto tmp = wide_tmp(chr, sz);
		std::wcout.write(tmp.data(), tmp.size());
	}
	else
	{
		TText buf = toAnsi(Text16(chr, sz));
		std::cout.write(buf.data(), buf.size());
	}
}
template <>
void StandardErrorOutput16::flush() noexcept
{
	std::wcerr.flush();
}
template <>
void StandardErrorOutput16::$write(const char16 *chr, size_t sz) noexcept
{
	if (sizeof(wchar_t) == sizeof(char16_t))
	{
		auto tmp = wide_tmp(chr, sz);
		std::wcerr.write(tmp.data(), tmp.size());
	}
	else
	{
		TText buf = toAnsi(Text16(chr, sz));
		std::cerr.write(buf.data(), buf.size());
	}
}

#ifdef WIN32

ConsoleOutputStream<ConsoleType::Debug, char16> ConsoleOutputStream<ConsoleType::Debug, char16>::out;

template <>
void DebugOutput::$write(const char *chr, size_t sz) noexcept
{
	udout << ansiToUtf16(Text(chr, sz));
}

DebugOutput16::ConsoleOutputStream() noexcept
{
#ifndef NDEBUG
	s_thread = nullptr;
#endif
}
DebugOutput16::~ConsoleOutputStream() noexcept
{
#ifndef NDEBUG
	if (s_thread != nullptr)
	{
		s_quitThread->set();
		s_thread->join();
	}
#endif
}
void DebugOutput16::$write(const char16 * chr, size_t sz) noexcept
{
#ifndef NDEBUG
	s_cs.enter();
	size_t remaining = s_buffer.remaining() - 1;
	size_t left = sz;
	if (left <= remaining)
	{
		s_buffer.write(chr, left);
	}
	else
	{
		left -= remaining;
		s_buffer.write(chr, remaining);
		remaining = s_buffer.capacity() - 1;
		for (;;)
		{
			flush();
			if (left <= remaining)
			{
				s_buffer.write(chr, left);
				break;
			}
			s_buffer.write(chr, remaining);
			left -= remaining;
		}
	}
	if (s_thread == nullptr)
	{
		s_quitThread = EventHandle::create(false, false);
		ThreadId threadid;
		s_thread = ThreadHandle::create<DebugOutput16, &DebugOutput16::_thread>(this, &threadid);
		threadid.setName("DebugOutputThread");
	}
	s_cs.leave();
#endif
}
void DebugOutput16::putSourceLine(pcstr16 src, int line) noexcept
{
	*this << src << u'(' << line << u")\r\n";
}
void DebugOutput16::flush() noexcept
{
#ifndef NDEBUG
	s_cs.enter();
	if (!s_buffer.empty())
	{
		s_buffer << nullterm;
		s_buffer.change('\0', ' ');

		OutputDebugStringW((LPCWSTR)s_buffer.data());

		s_buffer.clear();
	}
	s_cs.leave();
#endif
}

#ifndef NDEBUG

int DebugOutput16::_thread() noexcept
{
	while (!s_quitThread->wait((duration)20))
	{
		flush();
	}
	return 0;
}

#endif

#else

template <>
void DebugOutput::$write(const char *chr, size_t sz) noexcept
{
	cout.write(chr, sz);
}
template <>
void DebugOutput::putSourceLine(pcstr src, int line) noexcept
{
	*this << src << '(' << line << ")\r\n";
}
template <>
void DebugOutput::flush() noexcept
{
	cout.flush();
}

#endif

