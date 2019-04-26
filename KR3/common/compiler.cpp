#include "stdafx.h"
#include "compiler.h"
#include "../util/envvar.h"
#include "../util/process.h"

#ifdef WIN32
#include "../wl/windows.h"

#pragma warning(disable:4800)
//
//extern "C" WINBASEAPI BOOL WINAPI CheckRemoteDebuggerPresent(__in  HANDLE hProcess, __out PBOOL pbDebuggerPresent);
//extern "C" WINBASEAPI BOOL WINAPI TerminateProcess(_In_ HANDLE hProcess, _In_ UINT uExitCode);
//extern "C" WINBASEAPI HANDLE WINAPI GetCurrentProcess(void);

#elif defined(EMSCRIPTEN)
#include <emscripten.h>

staticCode
{
#ifdef NDEBUG
	EM_ASM({
		self['emUtf16ToStr'] = function emUtf16ToStr(ptr, len) {
			ptr >>= 1;
			var end = ptr + len | 0;
			var str = "";
			for (; ptr !== end; ptr = ptr + 1 | 0) {
				str += String.fromCharCode(HEAP16[ptr]);
			}
			return str;
		};
	});	
#else
	EM_ASM({
		if (typeof SAFE_HEAP_LOAD === 'undefined')
		{
			self['emUtf16ToStr'] = function emUtf16ToStr(ptr, len) {
				ptr >>= 1;
				var end = ptr + len | 0;
				var str = "";
				for (; ptr !== end; ptr = ptr + 1 | 0) {
					str += String.fromCharCode(HEAP16[ptr]);
				}
				return str;
			};
		}
		else
		{
			self['emUtf16ToStr'] = function emUtf16ToStr(ptr, len) {
				var end = ptr + (len * 2 | 0) | 0;
				var str = "";
				for (; ptr !== end; ptr = ptr + 2 | 0) {
					var codeUnit = SAFE_HEAP_LOAD(ptr, 2, 0, 0);
					str += String.fromCharCode(codeUnit);
				}
				return str;
			};
		}
	});
#endif
};

void vem__debug_break() noexcept
{
	EM_ASM(debugger;);
}
#else

#endif

bool kr::checkDebugging() noexcept
{
#ifdef WIN32
	return (bool)IsDebuggerPresent();
	//HANDLE process = GetCurrentProcess();
	//BOOL debugged = false;
	//CheckRemoteDebuggerPresent(process, &debugged);
	//return debugged;
#else
	return true;
#endif
}
bool kr::requestDebugger() noexcept
{
#ifdef WIN32
	//%VS140COMNTOOLS%..\IDE\devenv.exe /debugexe yourapp.exe
	// VsJITDebugger.exe -p <pid>
	if (checkDebugging()) return true;

	DWORD exitCode;
	{
		constexpr unsigned int SIZE = 1024;
		wchar_t buffer[1024];
		buffer[0] = L'\"';
		DWORD len = GetEnvironmentVariableW(L"windir", buffer + 1, SIZE - 1);
		if (len == 0) return false;
		len++;
		static const wchar_t CMD_ADD[] = L"\\System32\\VsJITDebugger.exe\" -p ";
		wcscpy_s(buffer + len, SIZE - len, CMD_ADD);
		len += countof(CMD_ADD) - 1;
		_itow_s(GetCurrentProcessId(), buffer + len, SIZE - len, 10);

		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi;
		si.cb = sizeof(si);
		if (!CreateProcessW(nullptr, buffer, nullptr, nullptr, true, 0, nullptr, nullptr, &si, &pi))
			return false;

		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &exitCode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	if (exitCode != 0) return false;
	while (!checkDebugging()) Sleep(100);
	return true;
#else
	debug(); // Not implemented yet
	return false;
#endif
}
ATTR_NORETURN void kr::terminate(int err) noexcept
{
#ifdef WIN32
	TerminateProcess(GetCurrentProcess(), err);
#else
	exit(err);
#endif
}
ATTR_NORETURN void kr::notEnoughMemory() noexcept
{
	// 할당량이 잘못 되었거나,
	// 메모리 부족으로 더 이상 프로그램을 유지할 수 없다.
	debug();
	terminate(ENOMEM);
}

#ifdef _MSC_VER

unsigned long long kr::rdtsc() noexcept
{
	return __rdtsc();
}

#elif defined(__i386__)

unsigned long long kr::rdtsc() noexcept
{
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}

#elif defined(__x86_64__)

unsigned long long kr::rdtsc() noexcept
{
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

unsigned long long kr::rdtsc() noexcept
{
	return (unsigned long long)(EM_ASM_DOUBLE_V(return performance.now();) * 1000000) | 0;
}

#else

#error Need implement

#endif
