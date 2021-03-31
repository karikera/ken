#pragma once

#include <KR3/win/windows.h>
#pragma warning(disable:4091)
#define _NO_CVCONST_H
#include <DbgHelp.h>


#define FUNCDECL(name) decltype(::name)* name  = (decltype(name))GetProcAddress(dll, #name)

struct DbgHelp
{
    HMODULE dll = LoadLibraryW(L"dbghelp.dll");
    FUNCDECL(MiniDumpWriteDump);
    FUNCDECL(SymSetOptions);
    FUNCDECL(SymInitialize);
    FUNCDECL(SymLoadModuleExW);
    FUNCDECL(SymUnloadModule64);
    FUNCDECL(SymGetModuleInfoW64);
    FUNCDECL(SymGetTypeInfo);
    FUNCDECL(SymEnumSymbols);
    FUNCDECL(SymEnumSymbolsExW);
    FUNCDECL(UnDecorateSymbolName);
    FUNCDECL(UnDecorateSymbolNameW);
    FUNCDECL(SymEnumSymbolsForAddrW);
    FUNCDECL(SymGetSymFromName64);
    FUNCDECL(StackWalk64);
    FUNCDECL(SymFunctionTableAccess64);
    FUNCDECL(SymGetModuleBase64);
    FUNCDECL(SymCleanup);
    FUNCDECL(SymGetOptions);
    FUNCDECL(SymGetSymFromAddr64);
    FUNCDECL(SymGetLineFromAddrW64);
    FUNCDECL(SymNext);
    FUNCDECL(SymNextW);
    FUNCDECL(SymEnumTypesW);

    static DbgHelp* getInstance() noexcept;

private:
    DbgHelp() noexcept;
};

#undef FUNCLINK