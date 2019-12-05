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

kr::SEHException::SEHException(PEXCEPTION_POINTERS exception) noexcept
	:exception(exception)
{
}
kr::Text16 kr::SEHException::getErrorText() noexcept
{
	switch (getErrorCode())
	{
	case STATUS_ACCESS_VIOLATION: return u"STATUS_ACCESS_VIOLATION";
	case STATUS_BREAKPOINT: return u"STATUS_BREAKPOINT";
	case STATUS_DATATYPE_MISALIGNMENT: return u"STATUS_DATATYPE_MISALIGNMENT";
	case STATUS_FLOAT_DIVIDE_BY_ZERO: return u"STATUS_FLOAT_DIVIDE_BY_ZERO";
	case STATUS_FLOAT_OVERFLOW: return u"STATUS_FLOAT_OVERFLOW";
	case STATUS_FLOAT_UNDERFLOW: return u"STATUS_FLOAT_UNDERFLOW";
	// case STATUS_FLOATING_RESEVERED_OPERAND: return u"STATUS_FLOATING_RESEVERED_OPERAND";
	case STATUS_ILLEGAL_INSTRUCTION: return u"STATUS_ILLEGAL_INSTRUCTION";
	case STATUS_PRIVILEGED_INSTRUCTION: return u"STATUS_PRIVILEGED_INSTRUCTION";
	case STATUS_INTEGER_DIVIDE_BY_ZERO: return u"STATUS_INTEGER_DIVIDE_BY_ZERO";
	case STATUS_INTEGER_OVERFLOW: return u"STATUS_INTEGER_OVERFLOW";
	case STATUS_SINGLE_STEP: return u"STATUS_SINGLE_STEP";
	default: return u"UNKNOWN";
	}
}
uint32_t kr::SEHException::getErrorCode() noexcept
{
	return exception->ExceptionRecord->ExceptionCode;
}

kr::SEHCatcher::SEHCatcher() noexcept
{
	m_func = _set_se_translator([](unsigned int code, EXCEPTION_POINTERS* ptr) { throw SEHException(ptr); });
}
kr::SEHCatcher::~SEHCatcher() noexcept
{
	_set_se_translator((_se_translator_function)m_func);
}

int kr::dump(const SEHException& ex) noexcept
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
		ExpParam.ExceptionPointers = ex.exception;
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
	__except(dump(GetExceptionInformation()))
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