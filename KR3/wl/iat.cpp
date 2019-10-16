#include "stdafx.h"
#include "iat.h"

using namespace kr;
using namespace hook;

#pragma pack(push,1)
#ifdef _WIN64
struct JUMPFUNC
{
	BYTE byOpcodeE9;
	DWORD dwAddress;

	void SetOpcode()
	{
		byOpcodeE9 = 0xE9;
	}
};
#elif defined(_WIN32)
struct JUMPFUNC
{
	BYTE byOpcodeE9;
	DWORD dwAddress;

	void SetOpcode()
	{
		byOpcodeE9 = 0xE9;
	}
};
#endif
#pragma pack(pop)

inline IMAGE_IMPORT_DESCRIPTOR * getIATStart(HMODULE module) noexcept
{
	IMAGE_DOS_HEADER * dos = (IMAGE_DOS_HEADER*)module;
	_assert(dos->e_magic == IMAGE_DOS_SIGNATURE);

	IMAGE_NT_HEADERS * nt = (IMAGE_NT_HEADERS*)((byte*)dos + dos->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR * img_import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(
		(byte*)dos + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	return img_import_desc;
}

void hook::forceFill(void* dest, int value, size_t size) noexcept
{
	Unprotector unpro(dest, size);
	memset(dest, value, size);
}
void hook::forceCopy(void* dest, const void* src, size_t size) noexcept
{
	Unprotector unpro(dest, size);
	memcpy(dest, src, size);
}

DllSearcher::DllSearcher() noexcept
{
	HMODULE module = GetModuleHandle(nullptr);
	this->module = module;
	this->iat = getIATStart(module);
}
pcstr DllSearcher::first() noexcept
{
	return (LPCSTR)((uintptr_t)module + iat->Name);
}
pcstr DllSearcher::next() noexcept
{
	iat++;
	if (iat->Name == 0) return nullptr;
	return (LPCSTR)((uintptr_t)module + iat->Name);
}

Function::Function()
{
	m_pAddress = nullptr;
}
Function::Function(HMODULE hModule, LPCSTR strName)
{
	m_pAddress = GetProcAddress(hModule, strName);
}
Function::Function(LPVOID pAddress)
{
	m_pAddress = pAddress;
}
Function::Function(Function& copy)
{
	m_pAddress = copy.m_pAddress;
}
Function::operator LPVOID()
{
	return m_pAddress;
}

Module::Module(LPCSTR strDLL)
{
	m_hModule = GetModuleHandleA(strDLL);
}
Module::Module(LPCWSTR strDLL)
{
	m_hModule = GetModuleHandleW(strDLL);
}
Module::~Module()
{
}
Function Module::operator [](LPCSTR strName)
{
	return Function(m_hModule, strName);
}
HMODULE Module::getHandle()
{
	return m_hModule;
}

Hooker::Hooker()
{
}
void Hooker::hooking(Function& ori, LPVOID pHook)
{
	m_pAddress = (LPVOID)ori;
	m_pHook = pHook;

	{
		Unprotector unpro(m_pAddress, sizeof(JUMPFUNC));
		memcpy(m_byBackup, m_pAddress, sizeof(JUMPFUNC));
	}
	hook();
}
void Hooker::hook()
{
	Unprotector unpro(m_pAddress, sizeof(JUMPFUNC));
	JUMPFUNC* jmp = (JUMPFUNC*)m_pAddress;
	jmp->SetOpcode();
	jmp->dwAddress = (DWORD)m_pHook - (DWORD)m_pAddress - 5;
}
void Hooker::unhook()
{
	Unprotector unpro(m_pAddress, sizeof(JUMPFUNC));
	memcpy(m_pAddress, m_byBackup, sizeof(JUMPFUNC));
}

CIAT::CIAT(HMODULE hModule, LPCSTR strDLL)
{
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = getIATStart(hModule);

	for (; pImportDesc->Name; pImportDesc++)
	{
		LPCSTR szLibName = (LPCSTR)((DWORD)hModule + pImportDesc->Name);
		if (_stricmp(szLibName, strDLL) != 0) continue;

		m_pThunk = (PIMAGE_THUNK_DATA)((DWORD)hModule + pImportDesc->FirstThunk);
		return;
	}

	m_pThunk = nullptr;
}
PULONG_PTR CIAT::operator [](LPVOID ori)
{
	PIMAGE_THUNK_DATA pThunk = m_pThunk;
	if (pThunk == nullptr) return nullptr;

	for (; pThunk->u1.Function; pThunk++)
	{
		if (pThunk->u1.Function != (DWORD)ori) continue;
		return &pThunk->u1.Function;
	}
	return nullptr;
}

IATHooker::IATHooker()
{
	m_dwBackup = 0;
	m_pTarget = nullptr;
}
IATHooker::IATHooker(PULONG_PTR pTarget, LPVOID pFunc)
{
	m_pTarget = pTarget;

	Unprotector unpro(m_pTarget, 4);
	m_dwBackup = *m_pTarget;
	*m_pTarget = (DWORD)pFunc;
}
void IATHooker::unhook()
{
	if (m_pTarget == nullptr) return;

	Unprotector unpro(m_pTarget, 4);
	*m_pTarget = m_dwBackup;
}

IATModule::IATModule(HMODULE hModule, LPCSTR strDLL) :Module(strDLL), CIAT(hModule, strDLL)
{
}
IATHooker IATModule::hooking(LPCSTR strFunction, LPVOID pHook)
{
	PULONG_PTR pTarget = (*this)[strFunction];
	if (pTarget == nullptr) return IATHooker();
	else return IATHooker(pTarget, pHook);
}
PULONG_PTR IATModule::operator [](LPCSTR strName)
{
	Function func = (*((Module*)this))[strName];
	return (*this)[func];
}

IATHookerList::IATHookerList(HMODULE hModule, LPCSTR strDLL) :IATModule(hModule, strDLL)
{
}
void IATHookerList::hooking(LPCSTR strFunction, LPVOID pHook)
{
	m_vList.create(strFunction, pHook);
}
void IATHookerList::unhook()
{
	auto begin = m_vList.begin();
	auto end = m_vList.end();
	for (auto i = begin; i != end; i++)
	{
		i->unhook();
	}
	m_vList.clear();
}

CodeHooker::CodeHooker()
{
	m_pDest = nullptr;
	m_pFunc = 0;
}
void CodeHooker::LCallHook(void* pCodeDest, void* pHook)
{
	DWORD jmpPos = ((DWORD)pCodeDest + 5);
	m_pDest = (LPDWORD)((DWORD)pCodeDest + 1);
	m_pFunc = ((*m_pDest) + jmpPos);
	{
		Unprotector pro(m_pDest, 4);
		*(DWORD*)pro = (DWORD)pHook - jmpPos;
	}
}

Unprotector::Unprotector(void* pDest, size_t nSize)
{
	m_pDest = pDest;
	m_nSize = nSize;

	VirtualProtect(pDest, nSize, PAGE_EXECUTE_READWRITE, &m_dwOldPage);
}
Unprotector::~Unprotector()
{
	VirtualProtect(m_pDest, m_nSize, m_dwOldPage, &m_dwOldPage);
}
