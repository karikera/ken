#include "stdafx.h"
#include "exception.h"
#include "../util/StackWalker.h"
#include <stdarg.h>

#ifdef WIN32
#include "../wl/windows.h"
#endif

using namespace kr;

namespace
{
	class Walker:public StackWalker
	{
	public:
		Walker() noexcept;
		virtual void onOutput(Text16 text) noexcept override;
	};
	Walker::Walker() noexcept
	{
		if (!loadModules())
		{
			udout << u"[StackWalker]Cannot load modules" << endl;
		};
	}
	void Walker::onOutput(Text16 text) noexcept
	{
		udout << text << endl;
	}
	Walker * getStackWalker() noexcept
	{
		static Walker stackWalker;
		return &stackWalker;
	}
}


ATTR_NORETURN void kr::quit(int exitCode) throws(QuitException)
{
	throw QuitException(exitCode);
}
ATTR_NORETURN void kr::error(const char * strMessage, ...) noexcept
{
	va_list vl;
	char temp[1024];
	va_start(vl, strMessage);
	size_t len = vsnprintf(temp, countof(temp) - 1, strMessage, vl);
	_assert(len < countof(temp));
	temp[len] = '\0';
	dout << temp << endl;
	dout.flush();
	debug(); // ġ���� ���α׷� ������ ���α׷��� ������ �� ����.
#ifdef WIN32
	MessageBoxA(nullptr, strMessage, "ġ���� ���α׷� ����", MB_OK | MB_ICONERROR);
#endif
	getStackWalker()->showCallstack();
	terminate(-1);
}
void kr::warning(const char * strMessage, ...) noexcept
{
	va_list vl;
	char temp[1024];
	va_start(vl, strMessage);
	size_t len = vsnprintf(temp, countof(temp) - 1, strMessage, vl);
	_assert(len < countof(temp));
	temp[len] = '\0';
	dout << temp << endl;
	dout.flush();
	debug(); // ���α׷��� ��� ������ ���� ������, �߻����� �ʰ� �ؾ��Ѵ�.
	getStackWalker()->showCallstack();
}
