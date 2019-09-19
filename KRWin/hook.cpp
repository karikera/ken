#include "stdafx.h"
#include "hook.h"
#include "handle.h"

using namespace kr;
using namespace hook;

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

IATHooker::IATHooker() noexcept
{
	m_backup = 0;
	m_target = nullptr;
}
IATHooker::IATHooker(PULONG_PTR target, LPVOID func) noexcept
{
	m_target = target;

	Unprotector unpro(m_target, 4);
	m_backup = *m_target;
	*m_target = (uintptr_t)func;
}
void IATHooker::unhook() noexcept
{
	if (m_target == nullptr) return;

	Unprotector unpro(m_target, 4);
	*m_target = m_backup;
}

IATModule::IATModule(win::Module* module, LPCSTR dll) noexcept
	: m_module(module)
{
	PIMAGE_IMPORT_DESCRIPTOR importdesc = getIATStart(module);
	for (; importdesc->Name; importdesc++)
	{
		LPCSTR szLibName = (LPCSTR)((uintptr_t)module + importdesc->Name);
		if (_stricmp(szLibName, dll) != 0) continue;

		m_desc = importdesc;
		return;
	}
	m_desc = nullptr;
}
IATHooker IATModule::hooking(LPCSTR functionName, LPVOID pHook) noexcept
{
	PULONG_PTR pTarget = getFunctionStore(functionName);
	if (pTarget == nullptr) return IATHooker();
	else return IATHooker(pTarget, pHook);
}

IATModule::Iterator::Iterator(win::Module * module, PIMAGE_IMPORT_DESCRIPTOR desc) noexcept
{
	m_module = module;
	m_ilt = (PIMAGE_THUNK_DATA)((uintptr_t)m_module + desc->OriginalFirstThunk);
	m_iat = (PIMAGE_THUNK_DATA)((uintptr_t)m_module + desc->FirstThunk);
}
IATModule::FunctionDesc IATModule::Iterator::operator *() const noexcept
{
	_assert(m_ilt != nullptr);

	PIMAGE_IMPORT_BY_NAME namedata = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)m_module + m_ilt->u1.AddressOfData);
	FunctionDesc desc;
	desc.name = namedata->Name;
	desc.store = &m_iat->u1.Function;
	return desc;
}
bool IATModule::Iterator::operator != (const IteratorEnd&) const noexcept
{
	return m_ilt != nullptr;
}
bool IATModule::Iterator::operator == (const IteratorEnd&) const noexcept
{
	return m_ilt != nullptr;
}
IATModule::Iterator& IATModule::Iterator::operator ++() noexcept
{
	m_ilt++;
	if (m_ilt->u1.AddressOfData == 0)
	{
		m_ilt = nullptr;
		return *this;
	}
	m_iat++;
	return *this;
}

IATModule::Iterator IATModule::begin() const noexcept
{
	return Iterator(m_module, m_desc);
}
IteratorEnd IATModule::end() const noexcept
{
	return IteratorEnd();
}

PULONG_PTR IATModule::getFunctionStore(LPCSTR name) noexcept
{
	for (FunctionDesc desc : *this)
	{
		if (strcmp(desc.name, name) == 0)
		{
			return desc.store;
		}
	}
	return nullptr;
}


IATHookerList::IATHookerList(win::Module* module, LPCSTR dll) noexcept :IATModule(module, dll)
{
}
void IATHookerList::hooking(LPCSTR func, LPVOID hook) noexcept
{
	m_list.create(IATModule::hooking(func, hook));
}
void IATHookerList::unhook() noexcept
{
	for (Node<IATHooker> & hook : m_list)
	{
		hook.data().unhook();
	}
	m_list.clear();
}//

//CodeHooker::CodeHooker()
//{
//	m_dest = nullptr;
//	m_func = 0;
//}
//void CodeHooker::LCallHook(void* codeDest, void* hook)
//{
//	uintptr_t jmpPos = ((uintptr_t)codeDest + 5);
//	m_dest = (LPDWORD)((uintptr_t)hook + 1);
//	m_func = ((*m_dest) + jmpPos);
//	{
//		Unprotector pro(m_dest, sizeof(uintptr_t));
//		*(uintptr_t*)pro = (uintptr_t)hook - jmpPos;
//	}
//}

Unprotector::Unprotector(void* pDest, size_t nSize)
{
	m_dest = pDest;
	m_nSize = nSize;

	VirtualProtect(pDest, nSize, PAGE_EXECUTE_READWRITE, &m_dwOldPage);
}
Unprotector::~Unprotector()
{
	VirtualProtect(m_dest, m_nSize, m_dwOldPage, &m_dwOldPage);
}
