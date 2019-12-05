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
	debug(); // 치명적 프로그램 오류로 프로그램을 유지할 수 없다.
#ifdef WIN32
	MessageBoxA(nullptr, strMessage, "치명적 프로그램 오류", MB_OK | MB_ICONERROR);
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
	debug(); // 프로그램을 계속 진행할 수는 있지만, 발생하지 않게 해야한다.
	getStackWalker()->showCallstack();
}
