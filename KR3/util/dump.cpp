#include "stdafx.h"
#include "dump.h"

#ifdef WIN32

#include "fs/file.h"
#include <KR3/wl/windows.h>
#pragma warning(disable:4091)
#include <DbgHelp.h>
#include <ctime>
#include <iomanip>
#pragma comment(lib,"Dbghelp.lib")

int kr::_pri_::dump(LPEXCEPTION_POINTERS ex) noexcept
{
	File::createDirectory(u"dump");

	char filename[FILENAME_MAX];
	time_t now = std::time(nullptr);
	tm ts;
	localtime_s(&ts, &now);
	size_t filename_len = strftime(filename, FILENAME_MAX, "dump\\%Y.%m.%d.%I.%M.%S.dmp", &ts);
	File* hFile = File::create(filename);
	if(hFile != nullptr)
	{
		MINIDUMP_EXCEPTION_INFORMATION ExpParam;
		ExpParam.ThreadId = GetCurrentThreadId();
		ExpParam.ExceptionPointers = ex;
		ExpParam.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
			MiniDumpWithDataSegs, &ExpParam, nullptr ,nullptr);
		delete hFile;
	}
	#ifndef NDEBUG
	return EXCEPTION_CONTINUE_SEARCH;
	#else
	return EXCEPTION_EXECUTE_HANDLER;
	#endif
}

void kr::dump_now() noexcept
{
	__try
	{
		*(intptr_t*)0 =0;
	}
	__except(_pri_::dump(GetExceptionInformation()))
	{
	}
}

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

void kr::dump_now() noexcept
{
	EM_ASM({ console.log(new Error('dummy').stack);  });
}

#else

#error Need implement

#endif