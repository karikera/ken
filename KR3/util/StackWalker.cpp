#include "stdafx.h"
#include "StackWalker.h"

#include <stdio.h>

#ifdef WIN32

#include <tchar.h>

#include "../wl/windows.h"

#pragma comment(lib, "dbghelp.lib")

#pragma warning(disable:4091)
#include <dbghelp.h>
#include <TlHelp32.h>

enum { STACKWALK_MAX_NAMELEN = 1024 };
#define USED_CONTEXT_FLAGS CONTEXT_FULL
#undef MODULEENTRY32
#undef Module32First
#undef Module32Next

#if defined(_M_IX86)
// The following should be enough for walking the callstack...
#define GET_CURRENT_CONTEXT(c, contextFlags) \
  do { \
    memset(&c, 0, sizeof(CONTEXT)); \
    c.ContextFlags = contextFlags; \
    __asm    call x \
    __asm x: pop eax \
    __asm    mov c.Eip, eax \
    __asm    mov c.Ebp, ebp \
    __asm    mov c.Esp, esp \
      } while(0);
#else

// The following is defined for x86 (XP and higher), x64 and IA64:
#define GET_CURRENT_CONTEXT(c, contextFlags) \
  do { \
    memset(&c, 0, sizeof(CONTEXT)); \
    c.ContextFlags = contextFlags; \
    RtlCaptureContext(&c); \
    } while(0);
#endif

namespace
{
	void getOSInfomation()
	{
		// Also display the OS-version
		OSVERSIONINFOEXA ver;
		ZeroMemory(&ver, sizeof(OSVERSIONINFOEXA));
		ver.dwOSVersionInfoSize = sizeof(ver);
#pragma warning(push)
#pragma warning(disable:4996)
		if (GetVersionExA((OSVERSIONINFOA*)&ver) != FALSE)
#pragma warning(pop)
		{
			char temp[1024];
			sprintf_s(temp, "OS-Version: %d.%d.%d (%s) 0x%x-0x%x\n",
				ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber,
				ver.szCSDVersion, ver.wSuiteMask, ver.wProductType);
		}
	}
	const char * getSymTypeName(SYM_TYPE type)
	{
		static const char * names[] =
		{
			"-nosymbols-",
			"COFF",
			"CV",
			"PDB",
			"-exported-",
			"-deferred-",
			"SYM",
			"Virtual",
			"DIA"
		};
		if (type > SymDia) return "-unknown-";
		return names[type];
	}
	char * getSymBuildPath(const char * strDefaultSymPath)
	{
		constexpr size_t nSymPathLen = 4096;
		char *szSymPath = _new char[nSymPathLen];
		szSymPath[0] = 0;

		// Now first add the (optional) provided sympath:
		if (strDefaultSymPath != nullptr)
		{
			strcat_s(szSymPath, nSymPathLen, strDefaultSymPath);
			strcat_s(szSymPath, nSymPathLen, ";");
		}

		strcat_s(szSymPath, nSymPathLen, ".;");

		const size_t nTempLen = 1024;
		char szTemp[nTempLen];
		// Now add the current directory:
		if (GetCurrentDirectoryA(nTempLen, szTemp) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, ";");
		}

		// Now add the path for the main-module:
		if (GetModuleFileNameA(nullptr, szTemp, nTempLen) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			for (char *p = (szTemp + strlen(szTemp) - 1); p >= szTemp; --p)
			{
				// locate the rightmost path separator
				if ((*p == '\\') || (*p == '/') || (*p == ':'))
				{
					*p = 0;
					break;
				}
			}  // for (search for path separator...)
			if (strlen(szTemp) > 0)
			{
				strcat_s(szSymPath, nSymPathLen, szTemp);
				strcat_s(szSymPath, nSymPathLen, ";");
			}
		}
		if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", szTemp, nTempLen) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, ";");
		}
		if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", szTemp, nTempLen) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, ";");
		}
		if (GetEnvironmentVariableA("SYSTEMROOT", szTemp, nTempLen) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, ";");
			// also add the "system32"-directory:
			strcat_s(szTemp, nTempLen, "\\system32");
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, ";");
		}

		if (GetEnvironmentVariableA("SYSTEMDRIVE", szTemp, nTempLen) > 0)
		{
			szTemp[nTempLen - 1] = 0;
			strcat_s(szSymPath, nSymPathLen, "SRV*");
			strcat_s(szSymPath, nSymPathLen, szTemp);
			strcat_s(szSymPath, nSymPathLen, "\\websymbols");
			strcat_s(szSymPath, nSymPathLen, "*http://msdl.microsoft.com/download/symbols;");
		}
		else
			strcat_s(szSymPath, nSymPathLen, "SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;");
		return szSymPath;
	}

#pragma region get module info
	// show module info (SymGetModuleInfo64())
	//IMAGEHLP_MODULE64 Module;
	//memset(&Module, 0, sizeof(Module));
	//Module.SizeOfStruct = sizeof(Module);
	//if (SymGetModuleInfo64(this->m_hProcess, s.AddrPC.Offset, &Module) != FALSE)
	//{
	//	csEntry.symTypeString = getSymTypeName(Module.SymType);
	//	strcpy_s(csEntry.moduleName, Module.ModuleName);
	//	csEntry.baseOfImage = Module.BaseOfImage;
	//	strcpy_s(csEntry.loadedImageName, Module.LoadedImageName);
	//} // got module info OK
	//else
	//{
	//	this->OnDbgHelpErr("SymGetModuleInfo64", GetLastError(), s.AddrPC.Offset);
	//}
#pragma endregion
#pragma region get thread context
	//if (hThread != GetCurrentThread())
	//{
	//	SuspendThread(hThread);
	//	memset(&c, 0, sizeof(CONTEXT));
	//	c.ContextFlags = USED_CONTEXT_FLAGS;
	//	if (GetThreadContext(hThread, &c) == FALSE)
	//	{
	//		ResumeThread(hThread);
	//		return FALSE;
	//	}
	//}
#pragma endregion
#pragma region get module info
	//info.name = img; // path
	//info.szSymType = "-unknown-";
	//IMAGEHLP_MODULE64 Module;
	//fileVersion = getFileVersion(szImg);
	//if (SymGetModuleInfo64(hProcess, baseAddr, &Module) != FALSE)
	//{
	//	info.szSymType = getSymTypeName(Module.SymType);
	//	info.pdbName = Module.LoadedImageName;
	//}

#pragma endregion
}

kr::StackWalker::StackWalker() noexcept
{
	m_modulesLoaded = false;
	m_hProcess = GetCurrentProcess();
	m_dwProcessId = GetCurrentProcessId();
}
kr::StackWalker::~StackWalker() noexcept
{
	SymCleanup(m_hProcess);
}

bool kr::StackWalker::loadModules() noexcept
{
	// SymInitialize
	if (SymInitialize(m_hProcess, nullptr, FALSE) == FALSE)
	{
		onDbgHelpErr("SymInitialize", GetLastError(), 0);
		SetLastError(ERROR_DLL_INIT_FAILED);
		return false;
	}

	// SymGetOptions
	dword symOptions = SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
	//symOptions |= SYMOPT_NO_PROMPTS;
	symOptions = SymSetOptions(symOptions);

	// load module
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_dwProcessId);
	if (hSnap == INVALID_HANDLE_VALUE) return false;

	ModuleInfo info;
	MODULEENTRY32 me;
	me.dwSize = sizeof(me);
	if (Module32First(hSnap, &me)) do
	{
		if (SymLoadModule64(m_hProcess, 0, me.szExePath, me.szModule, (qword)me.modBaseAddr, me.modBaseSize) == 0) break;
		info.name = me.szModule;
		onLoadModule(&info);
	}
	while (Module32Next(hSnap, &me));
	CloseHandle(hSnap);
	m_modulesLoaded = true;
	return true;
}
bool kr::StackWalker::showCallstack() noexcept
{
	CONTEXT c;;
	if (m_modulesLoaded == false) loadModules();  // ignore the result...
	GET_CURRENT_CONTEXT(c, USED_CONTEXT_FLAGS);

	HANDLE hThread = GetCurrentThread();

	STACKFRAME64 s;
	memset(&s, 0, sizeof(s));
	dword imageType;
#ifdef _M_IX86
	// normally, call ImageNtHeader() and use machine info from PE header
	imageType = IMAGE_FILE_MACHINE_I386;
	s.AddrPC.Offset = c.Eip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = c.Ebp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.Esp;
	s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	s.AddrPC.Offset = c.Rip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = c.Rsp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.Rsp;
	s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	imageType = IMAGE_FILE_MACHINE_IA64;
	s.AddrPC.Offset = c.StIIP;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = c.IntSp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrBStore.Offset = c.RsBSP;
	s.AddrBStore.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.IntSp;
	s.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

	struct SYM :IMAGEHLP_SYMBOL64
	{
		char __buffer[STACKWALK_MAX_NAMELEN];
	} sym;
	memset(&sym, 0, sizeof(sym));
	sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	sym.MaxNameLength = STACKWALK_MAX_NAMELEN;

	StackInfo csEntry;
	memset(&csEntry, 0, sizeof(csEntry));

	IMAGEHLP_LINE64 Line;
	memset(&Line, 0, sizeof(Line));
	Line.SizeOfStruct = sizeof(Line);

	for (;;)
	{
		if (!StackWalk64(imageType, this->m_hProcess, hThread, &s, &c,
			[](HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
		{
			SIZE_T size;
			BOOL res = ReadProcessMemory(hProcess, (LPVOID)qwBaseAddress, lpBuffer, nSize, &size);
			static_assert(sizeof(dword) == sizeof(DWORD), "dword size unmatch");
			*lpNumberOfBytesRead = (dword)size;
			return res;
		}, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
		{
			onDbgHelpErr("StackWalk64", GetLastError(), s.AddrPC.Offset);
			break;
		}

		if (s.AddrPC.Offset == 0) continue;
		if (s.AddrPC.Offset == s.AddrReturn.Offset)
		{
			onDbgHelpErr("StackWalk64-Endless-Callstack!", 0, s.AddrPC.Offset);
			break;
		}

		csEntry.address = (void*)s.AddrPC.Offset;

		qword offsetFromSymbol;
		if (SymGetSymFromAddr64(this->m_hProcess, s.AddrPC.Offset, &offsetFromSymbol, &sym) != FALSE)
		{
			csEntry.function = sym.Name;
			//UnDecorateSymbolName(sym.Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY);
			//UnDecorateSymbolName(sym.Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE);
		}
		else
		{
			onDbgHelpErr("SymGetSymFromAddr64", GetLastError(), s.AddrPC.Offset);
		}

		DWORD offsetFromLine;
		if (SymGetLineFromAddr64(this->m_hProcess, s.AddrPC.Offset, &offsetFromLine, &Line) != FALSE)
		{
			csEntry.line = Line.LineNumber;
			csEntry.filename = Line.FileName;
		}
		else
		{
			if (GetLastError() == ERROR_INVALID_ADDRESS)
			{
				csEntry.line = 0;
				csEntry.filename = nullptr;
			}
			else
			{
				onDbgHelpErr("SymGetLineFromAddr64", GetLastError(), s.AddrPC.Offset);
			}
		}

		this->onStack(&csEntry);
		if (s.AddrReturn.Offset == 0)
		{
			SetLastError(ERROR_SUCCESS);
			break;
		}
	}

	return TRUE;
}

#else


kr::StackWalker::StackWalker() noexcept
{
	notImplementedYet();
}
kr::StackWalker::~StackWalker() noexcept
{
	notImplementedYet();
}

bool kr::StackWalker::loadModules() noexcept
{
	notImplementedYet();
}
bool kr::StackWalker::showCallstack() noexcept
{
	notImplementedYet();
}

#endif

void kr::StackWalker::onLoadModule(ModuleInfo * info) noexcept
{
}
void kr::StackWalker::onStack(StackInfo *entry) noexcept
{
	if (entry->filename == nullptr) return;

	const char * filename = strrchr(entry->filename, '\\');
	if (filename == nullptr) filename = entry->filename;
	else filename++;

	if (strcmp(entry->function, "kr::criticalError") == 0) return;
	if (strcmp(entry->function, "kr::StackWalker::ShowCallstack") == 0) return;
	if (strcmp(entry->function, "invoke_main") == 0) return;
	if (strcmp(entry->function, "__scrt_common_main_seh") == 0) return;
	if (strcmp(entry->function, "__scrt_common_main") == 0) return;
	if (strcmp(entry->function, "mainCRTStartup") == 0) return;

	TSZ buf;
	buf << entry->filename << '(' << entry->line << "): " << entry->function << '\n';
	onOutput(buf);
}
void kr::StackWalker::onDbgHelpErr(pcstr szFuncName, dword gle, qword addr) noexcept
{
	TSZ buf;
	buf << "ERROR: " << ", GetLastError: " << gle << " (Address: " << (void*)(uintptr_t)addr << ")\n";
	onOutput(buf);
}
