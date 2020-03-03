#include "stdafx.h"
#include "process.h"

#ifdef WIN32

#include "envvar.h"
#include <KR3/win/windows.h>
#include <KR3/win/eventhandle.h>
#include <shellapi.h>
#include <TlHelp32.h>

#pragma warning(disable:4703)

using namespace kr;

StdStream::StdStream() noexcept
{
	m_stream_read = nullptr;
}
StdStream::StdStream(StdStream&& _move) noexcept
{
	m_stream_read = _move.m_stream_read;
	_move.m_stream_read = nullptr;
}
StdStream::~StdStream() noexcept
{
	close();
}
size_t StdStream::$read(char* dest, size_t sz) throws(EofException)
{
	DWORD willRead = sz > (DWORD)-1 ? (DWORD)-1 : (DWORD)sz;
	DWORD readed;
	if (!ReadFile(m_stream_read, dest, willRead, &readed, nullptr))
	{
		throw EofException();
	}
	return readed;
}
void StdStream::close() noexcept
{
	CloseHandle(m_stream_read);
	m_stream_read = nullptr;
}

Process::Process() noexcept
{
	m_process = nullptr;
}
Process::Process(Process&& _move) noexcept
{
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
Process::Process(pcstr16 fileName, pstr16 parameter, pcstr16 curdir, StdStream* out, StdStream* err) noexcept
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
	CloseHandle(m_process);
	m_process = nullptr;
}
void Process::cmd(pstr16 parameter, pcstr16 curdir, StdStream* out, StdStream* err) throws(Error)
{
	static AText16 s_comspec;
	staticCode
	{
		s_comspec = EnviromentVariable16(u"comspec");
		s_comspec.c_str();
	};

	exec(s_comspec.data(), parameter, curdir, out, err);
}
void Process::shell(Text16 command, pcstr16 curdir, StdStream* out, StdStream* err) throws(Error)
{
	return cmd(TSZ16() << u"/c " << command, curdir, out, err);
}
void Process::exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir, StdStream* out, StdStream* err) throws(Error)
{
	_assert(out == nullptr || out->m_stream_read == nullptr);
	_assert(err == nullptr || err->m_stream_read == nullptr);

	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = true; 
	saAttr.lpSecurityDescriptor = nullptr; 
   
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);

	HANDLE stdout_write;
	HANDLE stderr_write;

	if (out != nullptr)
	{
		BOOL resCreatePipe = CreatePipe(&out->m_stream_read, &stdout_write, &saAttr, 0);
		_assert(resCreatePipe);

		BOOL resSetHandleInformation = SetHandleInformation(out->m_stream_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		_assert(resSetHandleInformation);
		si.hStdError = si.hStdOutput = stdout_write;
		si.dwFlags = STARTF_USESTDHANDLES;
	}
	if (err != nullptr)
	{
		BOOL resCreatePipe = CreatePipe(&err->m_stream_read, &stderr_write, &saAttr, 0);
		_assert(resCreatePipe);

		BOOL resSetHandleInformation = SetHandleInformation(err->m_stream_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		_assert(resSetHandleInformation);
		si.hStdError = stderr_write;
	}
	if (!CreateProcessW(wide(fileName), wide(parameter), nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, wide(curdir), &si, &pi))
	{
		throw Error();
	}
	if (out != nullptr)
	{
		CloseHandle(stdout_write);
	}
	if (err != nullptr)
	{
		CloseHandle(stderr_write);
	}
	CloseHandle(pi.hThread);

	m_process = pi.hProcess;
}
void Process::exec(pstr16 commandLine) throws(Error)
{
	exec(nullptr, commandLine, nullptr);
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

ProcessStream::ProcessStream() noexcept
{
}
ProcessStream::ProcessStream(ProcessStream&& _move) noexcept
	:process(move(_move.process)), stream(move(_move.stream))
{

}
ProcessStream::~ProcessStream() noexcept
{
}

size_t ProcessStream::$read(char* dest, size_t sz) throws(EofException)
{
	return stream.$read(dest, sz);
}
void ProcessStream::close() noexcept
{
	process.close();
	stream.close();
}
#ifdef WIN32
void ProcessStream::cmd(pstr16 parameter, pcstr16 curdir) throws(Error)
{
	process.cmd(parameter, curdir, &stream);
}
#endif
void ProcessStream::shell(Text16 command, pcstr16 curdir) throws(Error)
{
	process.shell(command, curdir, &stream);
}
void ProcessStream::exec(pcstr16 fileName, pstr16 parameter, pcstr16 curdir) throws(Error)
{
	process.exec(fileName, parameter, curdir, &stream);
}
void ProcessStream::exec(pstr16 commandLine) throws(Error)
{
	process.exec(nullptr, commandLine, nullptr, &stream);
}

StreamBuffer<char, ProcessStream> kr::shell(Text16 command, pcstr16 curdir) throws(Error)
{
	ProcessStream ps;
	ps.shell(command, curdir);
	ps.process.wait();
	return StreamBuffer<char, ProcessStream>(move(ps));
}
void kr::shellPiped(Text16 command, pcstr16 curdir) throws(Error)
{
	ProcessStream ps;
	ps.shell(command, curdir);
	passThrough(&cout, &ps.stream);
	ps.process.wait();
}
StreamBuffer<char, ProcessStream> kr::exec(pcstr16 file, pstr16 parameter, pcstr16 curdir) noexcept
{
	ProcessStream ps;
	ps.exec(file, parameter, curdir);
	ps.process.wait();
	return StreamBuffer<char, ProcessStream>(move(ps));
}
StreamBuffer<char, ProcessStream> kr::exec(Text16 command) noexcept
{
	ProcessStream ps;
	ps.exec(TSZ16() << command);
	return StreamBuffer<char, ProcessStream>(move(ps));
}
StreamBuffer<char, ProcessStream> kr::exec(pstr16 command) noexcept
{
	ProcessStream ps;
	ps.exec(command);
	return StreamBuffer<char, ProcessStream>(move(ps));
}
StreamBuffer<char, ProcessStream> kr::exec(pstr16 path, EventHandle* canceler) throws(ThrowAbort, Error)
{
	ProcessStream ps;
	ps.exec(path);
	if (!ps.process.waitWith(canceler)) throw ThrowAbort();
	return StreamBuffer<char, ProcessStream>(move(ps));
}
void kr::execPiped(pstr16 path) throws(ThrowAbort, Error)
{
	ProcessStream proc;
	proc.exec(path);
	passThrough(&cout, &proc.stream);
	proc.process.wait();
}
void kr::execOpen(pcstr16 path) noexcept
{
	ShellExecuteW(nullptr, L"open", wide(path), L"", L"", SW_SHOW);
}
int kr::execNoOutput(pstr16 pszstr) noexcept
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
int kr::execDetached(pstr16 pszstr) noexcept
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
