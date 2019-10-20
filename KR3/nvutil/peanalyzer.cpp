#include "stdafx.h"

#ifdef WIN32
#include "peanalyzer.h"

#include <stdlib.h>
#include <tchar.h>
#include <TlHelp32.h>

using namespace kr;

template <typename T> class TScanner :public MemoryScannerVTable
{
public:
	void searchSame();
	void searchDifferent();
	void searchAbove();
	void searchBellow();
	void searchAboveEqual();
	void searchBellowEqual();

	void searchVSame(void* pValue);
	void searchVDifferent(void* pValue);
	void searchVAbove(void* pValue);
	void searchVBellow(void* pValue);
	void searchVAboveEqual(void* pValue);
	void searchVBellowEqual(void* pValue);

	int getData(size_t i);
	int getPrevious(size_t i);
	LPVOID getAddress(size_t i);
	int sizeOfData();
	static int _sizeOfData();
};


void MemoryScannerVTable::limit(Rule nRule,long long &value)
{
	switch(nRule)
	{
	case UnsignedByte:
		if(value < 0) value = 0;
		else if(value > 0xff) value = 0xff;
		break;
	case UnsignedWord:
		if(value < 0) value = 0;
		else if(value > 0xffff) value = 0xffff;
		break;
	case UnsignedDword:
		if(value < 0) value = 0;
		else if(value > 0xffffffff) value = 0xffffffff;
		break;
	case UnsignedQword: break;
	case SignedByte:
		if(value < 0x80) value = 0x80;
		else if(value > 0x7f) value = 0x7f;
		break;
	case SignedWord:
		if(value < 0x8000) value = 0x8000;
		else if(value > 0x7fff) value = 0x7fff;
		break;
	case SignedDword:
		if(value < 0x80000000) value = 0x80000000;
		else if(value > 0x7fffffff) value = 0x7fffffff;
		break;
	case SignedQword: break;
	case Float:
		value = (long long)(float)value;
		break;
	case Double:
		value = (long long)(double)value;
		break;
	}
}
int MemoryScannerVTable::getSizeofRule(Rule nRule)
{
	return (nRule%4)+1;
}
void MemoryScannerVTable::formatValue(LPTSTR strTemp,size_t nSize,ULONG64 nValue,Rule nRule)
{
	switch(nRule)
	{
	case UnsignedByte: _stprintf_s(strTemp,nSize,TEXT("%u(%02X)"), (uint)nValue, (uint)nValue); break;
	case UnsignedWord: _stprintf_s(strTemp,nSize,TEXT("%u(%04X)"), (uint)nValue, (uint)nValue); break;
	case UnsignedDword: _stprintf_s(strTemp,nSize,TEXT("%u(%08X)"), (uint)nValue, (uint)nValue); break;
	case UnsignedQword: _stprintf_s(strTemp,nSize,TEXT("%llu(%016llX)"),nValue,nValue); break;
	case SignedByte: _stprintf_s(strTemp,nSize,TEXT("%d(%02X)"),(int)nValue,(int)nValue); break;
	case SignedWord: _stprintf_s(strTemp,nSize,TEXT("%d(%04X)"), (int)nValue, (int)nValue); break;
	case SignedDword: _stprintf_s(strTemp,nSize,TEXT("%u(%08X)"), (int)nValue, (int)nValue); break;
	case SignedQword: _stprintf_s(strTemp,nSize,TEXT("%llu(%016llX)"),nValue,nValue); break;
	case Float: _stprintf_s(strTemp,nSize,TEXT("%f"),(float&)nValue); break;
	case Double: _stprintf_s(strTemp,nSize,TEXT("%lf"),(double&)nValue); break;
	}
};

DataSection::DataSection()
{
	m_pData = nullptr;
	m_sh = nullptr;
}
DataSection::DataSection(IMAGE_SECTION_HEADER * sh,void *pData)
{
	m_sh = sh;
	m_pData = pData;
}

MemoryScanner::MemoryScanner()
{
	m_pPrevious = nullptr;
	m_nAlive = 0;
	m_nSize = 0;
}
MemoryScanner::MemoryScanner(const DATA_RANGE & dat)
{
	m_pData = dat.pData;
	m_nSize = dat.nSize;
	m_pPrevious = nullptr;
	m_nAlive = 0;
}
MemoryScanner::MemoryScanner(const DataSection &sec)
{
	m_pData = sec.m_pData;
	m_nSize = sec.m_sh->SizeOfRawData;
	m_pPrevious = nullptr;
	m_nAlive = 0;
}
MemoryScanner::MemoryScanner(IMAGE_SECTION_HEADER * sh,void *pData)
{
	m_pData = pData;
	m_nSize = sh->SizeOfRawData;
	m_pPrevious = nullptr;
	m_nAlive = 0;
}
MemoryScanner::MemoryScanner(void *pData,size_t nSize)
{
	m_pData = pData;
	m_nSize = nSize;
	m_pPrevious = nullptr;
	m_nAlive = 0;
}
MemoryScanner::~MemoryScanner()
{
	free();
}

bool MemoryScanner::scan(Rule nRule)
{
	free();
	switch(nRule)
	{
	case UnsignedByte: *(LPVOID*)this = (LPVOID&)TScanner<BYTE>(); break;
	case UnsignedWord: *(LPVOID*)this = (LPVOID&)TScanner<WORD>(); break;
	case UnsignedDword: *(LPVOID*)this = (LPVOID&)TScanner<DWORD>(); break;
	case UnsignedQword: *(LPVOID*)this = (LPVOID&)TScanner<DWORD64>(); break;
	case SignedByte: *(LPVOID*)this = (LPVOID&)TScanner<char>(); break;
	case SignedWord: *(LPVOID*)this = (LPVOID&)TScanner<short>(); break;
	case SignedDword: *(LPVOID*)this = (LPVOID&)TScanner<long>(); break;
	case SignedQword: *(LPVOID*)this = (LPVOID&)TScanner<__int64>(); break;
	case Float: *(LPVOID*)this = (LPVOID&)TScanner<float>(); break;
	case Double: *(LPVOID*)this = (LPVOID&)TScanner<double>(); break;
	default: return false;
	}

	int nDataSize = sizeOfData();
	m_pPrevious = VirtualAlloc(nullptr,m_nSize,MEM_COMMIT,PAGE_READWRITE);
	if(m_pPrevious == nullptr) return false;
	alloc(m_nSize / nDataSize);
	disableAll();
	
	m_nAlive = 0;

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	LPBYTE pPrevious = (LPBYTE)m_pPrevious;
	LPBYTE pData = (LPBYTE)m_pData;
	LPBYTE pEnd = pData + m_nSize;

	while(pData<pEnd)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(pData,&mbi,sizeof(mbi));
		
		size_t nSize = (LPBYTE)mbi.BaseAddress + mbi.RegionSize - pData;
		if(pData + nSize > pEnd) nSize = pEnd - pData;

		if((mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ)) && (mbi.State & MEM_COMMIT))
		{
			if(mbi.Protect & PAGE_GUARD)
			{
				DWORD dwOld;
				VirtualProtect(mbi.BaseAddress,mbi.RegionSize,mbi.Protect & (~PAGE_GUARD),&dwOld);
			}

			intptr_t nOffset = (pPrevious - (LPBYTE)m_pPrevious);
			intptr_t nBegin = (nOffset+nDataSize-1) / nDataSize;
			intptr_t nCount = (nOffset+nSize) / nDataSize - nBegin;
			m_nAlive += nCount;
			enableRange(nBegin,nCount);
		}
		pData+=nSize;
		pPrevious+=nSize;
	}
	return true;
}
void MemoryScanner::free()
{
	disableAll();
	m_nAlive = 0;
	if(m_pPrevious != nullptr)
	{
		VirtualFree(m_pPrevious,m_nSize,MEM_FREE);
		m_pPrevious = nullptr;
	}
}
void MemoryScanner::enable(size_t i)
{
	if(!(*this)[i])
	{
		m_nAlive++;
		BinArray::enable(i);
	}
}
void MemoryScanner::disable(size_t i)
{
	if((*this)[i])
	{
		m_nAlive--;
		BinArray::disable(i);
	}
}
void MemoryScanner::_enable(size_t i)
{
	m_nAlive++;
	BinArray::enable(i);
}
void MemoryScanner::_disable(size_t i)
{
	m_nAlive--;
	BinArray::disable(i);
}

void * MemoryScanner::getData()
{
	return m_pData;
}
void * MemoryScanner::getPrevious()
{
	return m_pPrevious;
}
size_t MemoryScanner::getSize()
{
	return m_nSize;
}
size_t MemoryScanner::getCount()
{
	return m_nAlive;
}

PEData::PEData(HMODULE hModule)
{
	m_dos = (IMAGE_DOS_HEADER*)hModule;
	m_nt = (IMAGE_NT_HEADERS32*)((LPBYTE)m_dos + m_dos->e_lfanew);
	m_sh = (IMAGE_SECTION_HEADER*)((LPBYTE)(&m_nt->OptionalHeader)+ m_nt->FileHeader.SizeOfOptionalHeader);
}
DataSection PEData::findSection(LPCSTR strSection)
{
	IMAGE_SECTION_HEADER *sh = _findSection(strSection);
	if(sh == nullptr) return DataSection(nullptr,nullptr);
	return DataSection(sh,(LPBYTE)m_dos + sh->VirtualAddress);
}
DATA_RANGE PEData::getWholeData()
{
	DATA_RANGE dat;

	LPBYTE pStart = (LPBYTE)m_dos + _findSection(".text")[1].VirtualAddress;
	LPBYTE pEnd = (LPBYTE)m_dos + m_nt->OptionalHeader.SizeOfImage +
		m_nt->OptionalHeader.SizeOfHeapReserve + m_nt->OptionalHeader.SizeOfStackReserve;

	dat.pData = pStart;
	dat.nSize = pEnd - pStart;
	return dat;
}
IMAGE_SECTION_HEADER* PEData::_findSection(LPCSTR strSection)
{
	IMAGE_SECTION_HEADER *sh = m_sh;
	int nSection = m_nt->FileHeader.NumberOfSections;
	for(int i=0;i<nSection;i++)
	{
		if(strcmp((char*)sh->Name,strSection) == 0) return sh;
		sh++;
	}
	return nullptr;
}
LPVOID PEData::rawToVirtual(DWORD dwAddress)
{
	IMAGE_SECTION_HEADER * sec = m_sh;
	IMAGE_SECTION_HEADER * sec_end = sec + m_nt->FileHeader.NumberOfSections;

	if(dwAddress < sec->PointerToRawData) return (LPBYTE)m_dos+dwAddress;
	for(;sec != sec_end;sec++)
	{
		DWORD rawaddr = sec->PointerToRawData;
		if(dwAddress < rawaddr + sec->SizeOfRawData)
		{
			return (LPBYTE)m_dos + dwAddress - rawaddr + sec->VirtualAddress;
		}
	}
	return nullptr;
}

template <typename T> void TScanner<T>::searchSame()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] != pData[i]) pThis->_disable(i);
	}
}
template <typename T> void TScanner<T>::searchDifferent()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] == pData[i]) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchAbove()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] >= pData[i]) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchBellow()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] <= pData[i]) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchAboveEqual()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] > pData[i]) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchBellowEqual()
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pPrevious[i] < pData[i]) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}

template <typename T> void TScanner<T>::searchVSame(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] != value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchVDifferent(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] == value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchVAbove(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] >= value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchVBellow(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] >= value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchVAboveEqual(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] < value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}
template <typename T> void TScanner<T>::searchVBellowEqual(void* pValue)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	T* pData = (T*)pThis->getData();
	size_t nMax = pThis->getSize() / sizeof(T);
	T value = *(T*)pValue;

	for (size_t i = 0; (i = pThis->findEnabled(i)) < nMax; i++)
	{
		if (pData[i] > value) pThis->disable(i);
		else pPrevious[i] = pData[i];
	}
}

template <typename T> int TScanner<T>::getData(size_t i)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pData = (T*)pThis->getData();
	return (int)pData[i];
}
template <typename T> int TScanner<T>::getPrevious(size_t i)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pPrevious = (T*)pThis->getPrevious();
	return (int)pPrevious[i];
}
template <typename T> LPVOID TScanner<T>::getAddress(size_t i)
{
	MemoryScanner * pThis = (MemoryScanner*)(MemoryScannerVTable*)this;
	T* pData = (T*)pThis->getData();
	return &pData[i];
}
template <typename T> int TScanner<T>::sizeOfData()
{
	return sizeof(T);
}
template <typename T> int TScanner<T>::_sizeOfData()
{
	return sizeof(T);
}

DWORD kr::GetCurrentModuleDirectoryW(LPWSTR strDest, size_t len) noexcept
{
	DWORD dwLen = GetCurrentModulePathW(strDest, kr::intact<DWORD>(len));
	if (dwLen == 0) return 0;

	LPWSTR strReverse = wcsrchr(strDest, L'\\');
	if (strReverse != nullptr) strReverse[1] = L'\0';
	else strDest[0] = L'\0';
	return dwLen;
}
DWORD kr::GetCurrentModuleDirectoryA(LPSTR strDest, size_t len) noexcept
{
	DWORD dwLen = GetCurrentModulePathA(strDest, kr::intact<DWORD>(len));
	if (dwLen == 0) return 0;

	LPSTR strReverse = strrchr(strDest, '\\');
	if (strReverse != nullptr)
	{
		strReverse++;
		*strReverse = '\0';
		return kr::intact<DWORD>(strReverse - strDest);
	}
	else
	{
		*strDest = '\0';
		return 0;
	}
}
#endif