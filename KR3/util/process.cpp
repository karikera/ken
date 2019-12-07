#include "stdafx.h"
#include "process.h"

#ifdef WIN32

#include "envvar.h"
#include "../wl/windows.h"
#include "../wl/eventhandle.h"
#include <shellapi.h>
#include <TlHelp32.h>

using namespace kr;

Process::Process() noexcept
{
	m_stdout_read = nullptr;
	m_process = nullptr;
}
Process::Process(Process&& _move) noexcept
{
	m_stdout_read = _move.m_stdout_read;
	_move.m_stdout_read = nullptr;
	m_process = _move.m_process;
	_move.m_process = nullptr;
}
Process::Process(Shell_t, Text16 cmd) noexcept
	:Process()
{
	shell(cmd);
}
Process::Process(pstr16 command) noexcept
{
	exec(command);
}
Process::Process(pcstr16 fileName, pstr16 parameter, pcstr16 curdir) noexcept
	: Process()
{
	exec(fileName, parameter, curdir);
}
Process::~Process() noexcept
{
	close();
}

void Process::close() noexcept
{
	CloseHandle(m_stdout_read);
	CloseHandle(m_process);
	m_stdout_read = nullptr;
	m_process = nullptr;
}
void Process::cmd(pstr16 parameter, pcstr16 curdir) throws(Error)
{
	static AText16 s_comspec;
	staticCode
	{
		s_comspec = EnviromentVariable16(u"comspec");
		s_comspec.c_str();
	};

	exec(s_comspec.data(), parameter, curdir);
}
void Process::shell(Text16 command, pcstr16 curdir) throws(Error)
{
	return cmd(TSZ16() << u"/c " << command, curdir);
}
void Process::exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir) throws(Error)
{
	_assert(m_stdout_read == nullptr);

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = true; 
	saAttr.lpSecurityDescriptor = nullptr; 
   
	HANDLE stdout_write;
	BOOL resCreatePipe = CreatePipe(&m_stdout_read, &stdout_write, &saAttr, 0);
	_assert(resCreatePipe);

	BOOL resSetHandleInformation = SetHandleInformation(m_stdout_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	_assert(resSetHandleInformation);

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	si.hStdError = si.hStdOutput = stdout_write;
	si.dwFlags = STARTF_USESTDHANDLES;
	if (!CreateProcessW(wide(fileName), wide(parameter), nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, wide(curdir), &si, &pi))
	{
		throw Error();
	}

	CloseHandle(stdout_write);
	CloseHandle(pi.hThread);

	m_process = pi.hProcess;
}
void Process::exec(pstr16 commandLine) throws(Error)
{
	exec(nullptr, commandLine, nullptr);
}
size_t Process::$read(char * dest, size_t sz) throws(EofException)
{
	DWORD willRead = sz > (DWORD)-1 ? (DWORD)-1 : (DWORD)sz;
	DWORD readed;
	if (!ReadFile(m_stdout_read, dest, willRead, &readed, nullptr))
	{
		throw EofException();
	}
	return readed;
}
void Process::wait() noexcept
{
	WaitForSingleObject(m_process, INFINITE);
}
bool Process::wait(int millis) noexcept
{
	return WaitForSingleObject(m_process, millis) == WAIT_TIMEOUT;
}
bool Process::waitWith(EventHandle * canceler) noexcept
{
	return getEventHandle()->waitWith(canceler);
}
EventHandle * Process::getEventHandle() noexcept
{
	return ((EventHandle*)m_process);
}
int Process::getExitCode() noexcept
{
	DWORD dwExit;
	if (!GetExitCodeProcess(m_process, &dwExit))
	{
		return -1;
	}
	return (int)dwExit;
}
void * Process::getCloseEventHandle() noexcept
{
	return m_process;
}

void kr::Process::executeOpen(kr::pcstr16 path) noexcept
{
	ShellExecuteW(nullptr, L"open", wide(path), L"", L"", SW_SHOW);
}
AText kr::Process::call(pstr16 path) throws(ThrowAbort, Error)
{
	Process process;
	process.exec(path);
	return process.readAll();
}
AText kr::Process::call(pstr16 path, EventHandle * canceler) throws(ThrowAbort, Error)
{
	Process process;
	process.exec(path);
	if (!process.waitWith(canceler)) throw ThrowAbort();
	return process.readAll();
}
int kr::Process::execute(pstr16 pszstr) noexcept
{
	DWORD exitCode;
	PROCESS_INFORMATION procinfo;
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);

	if (!CreateProcessW(nullptr, wide(pszstr), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &procinfo))
	{
		return GetLastError();
	}

	WaitForSingleObject(procinfo.hProcess, INFINITE);
	GetExitCodeProcess(procinfo.hProcess, &exitCode);
	CloseHandle(procinfo.hThread);
	CloseHandle(procinfo.hProcess);
	return exitCode;
}
int kr::Process::detachedExecute(pstr16 pszstr) noexcept
{
	PROCESS_INFORMATION procinfo;
	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcessW(nullptr, wide(pszstr), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &procinfo))
	{
		return GetLastError();
	}

	CloseHandle(procinfo.hThread);
	CloseHandle(procinfo.hProcess);
	return 0;
}

#else

#include <stdlib.h>

using namespace kr;

kr::Process::Process() noexcept
{
}
Process::Process(Shell_t, Text16 cmd) noexcept
	:Process()
{
	notImplementedYet();
}
Process::Process(pcstr16 fileName, pstr16 parameter) noexcept
	: Process()
{
	notImplementedYet();
}
Process::~Process() noexcept
{
	notImplementedYet();
}

void Process::close() noexcept
{
	notImplementedYet();
}
void Process::shell(Text16 command, pcstr16 curdir)
{
	notImplementedYet();
}
void Process::exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir)
{
	notImplementedYet();
}
size_t Process::$read(char * dest, size_t sz)
{
	notImplementedYet();
}
int Process::getExitCode() noexcept
{
	notImplementedYet();
}

void kr::Process::executeOpen(pcstr path) noexcept
{
	system(path);
}
int kr::Process::execute(pstr16 pszstr) noexcept
{
	notImplementedYet();
}
int kr::Process::detachedExecute(pstr16 pszstr) noexcept
{
	notImplementedYet();
}

#endif


ProcessId::ProcessId(dword id) noexcept
	:m_id(id)
{
}
ProcessId ProcessId::findByName(Text16 name) noexcept
{
#ifdef WIN32
	PROCESSENTRY32W ps = { sizeof(PROCESSENTRY32W), };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32FirstW(hSnapshot, &ps))
	{
		do
		{
			if ((Text16)unwide(ps.szExeFile) == name)
			{
				CloseHandle(hSnapshot);
				return ps.th32ProcessID;
			}
		} while (Process32NextW(hSnapshot, &ps));
	}
	CloseHandle(hSnapshot);
	return 0;
#else
	notImplementedYet();
#endif
}
TmpArray<ProcessId> ProcessId::findsByName(Text16 name) noexcept
{
#ifdef WIN32
	TmpArray<ProcessId> list;

	PROCESSENTRY32W ps = { sizeof(PROCESSENTRY32W), };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32FirstW(hSnapshot, &ps))
	{
		do
		{
			if ((Text16)unwide(ps.szExeFile) == name)
			{
				list.push((ProcessId)ps.th32ProcessID);
			}
		} while (Process32NextW(hSnapshot, &ps));
	}
	CloseHandle(hSnapshot);

	return list;
#else
	notImplementedYet();
#endif
}
dword ProcessId::value() noexcept
{
	return m_id;
}

StreamBuffer<char, Process> kr::shell(Text16 command, pcstr16 curdir) throws(Error)
{
	Process process;
	process.shell(command, curdir);
	process.wait();
	return StreamBuffer<char, Process>(move(process));
}
StreamBuffer<char, Process> kr::exec(pcstr16 file, pstr16 parameter, pcstr16 curdir) noexcept
{
	Process process;
	process.exec(file, parameter, curdir);
	process.wait();
	return StreamBuffer<char, Process>(move(process));
}
StreamBuffer<char, Process> kr::exec(Text16 command) noexcept
{
	Process process;
	process.exec(TSZ16() << command);
	return StreamBuffer<char, Process>(move(process));
}
StreamBuffer<char, Process> kr::exec(pstr16 command) noexcept
{
	Process process;
	process.exec(command);
	return StreamBuffer<char, Process>(move(process));
}