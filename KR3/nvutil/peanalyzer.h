#pragma once

#ifndef WIN32
#error is not windows system
#endif


#include <KR3/data/binarray.h>
#include <KR3/wl/windows.h>

namespace kr
{
	class MemoryScanner;

	struct DATA_RANGE
	{
		void * pData;
		size_t nSize;
	};

	class DataSection
	{
		friend MemoryScanner;
	public:
		DataSection();
		DataSection(IMAGE_SECTION_HEADER * sh, void *pData);

	protected:
		IMAGE_SECTION_HEADER * m_sh;
		void * m_pData;
	};

	class MemoryScannerVTable
	{
	public:
		enum Rule
		{
			UnsignedByte,
			UnsignedWord,
			UnsignedDword,
			UnsignedQword,
			SignedByte,
			SignedWord,
			SignedDword,
			SignedQword,
			Float,
			Double
		};

		virtual void searchSame() {};
		virtual void searchDifferent() {};
		virtual void searchAbove() {};
		virtual void searchBellow() {};
		virtual void searchAboveEqual() {};
		virtual void searchBellowEqual() {};

		virtual void searchVSame(void* pValue) {};
		virtual void searchVDifferent(void* pValue) {};
		virtual void searchVAbove(void* pValue) {};
		virtual void searchVBellow(void* pValue) {};
		virtual void searchVAboveEqual(void* pValue) {};
		virtual void searchVBellowEqual(void* pValue) {};

		virtual int getData(size_t i) { return 0; };
		virtual int getPrevious(size_t i) { return 0; };
		virtual LPVOID getAddress(size_t i) { return 0; };
		virtual int sizeOfData() { return 1; };

		static void limit(Rule nRule, long long &value);
		static int getSizeofRule(Rule nRule);
		static void formatValue(LPTSTR strTemp, size_t nSize, ULONG64 nValue, Rule nRule);

	};

	class MemoryScanner
		:public MemoryScannerVTable, private BinArray
	{
	public:
		MemoryScanner();
		MemoryScanner(const DATA_RANGE & dat);
		MemoryScanner(const DataSection &sec);
		MemoryScanner(IMAGE_SECTION_HEADER * sh, void *pData);
		MemoryScanner(void *pData, size_t nSize);
		~MemoryScanner();

		bool scan(Rule nRule);
		void free();
		void enable(size_t i);
		void disable(size_t i);
		void _enable(size_t i);
		void _disable(size_t i);
		using BinArray::findEnabled;
		using BinArray::operator [];

		using MemoryScannerVTable::getData;
		using MemoryScannerVTable::getPrevious;
		void * getData();
		void * getPrevious();
		size_t getSize();
		size_t getCount();

	protected:
		size_t m_nSize;
		void * m_pData;
		void * m_pPrevious;
		size_t m_nAlive;
	};

	class PEData
	{
	public:
		PEData(HMODULE hModule);
		DataSection findSection(LPCSTR strSection);
		DATA_RANGE getWholeData();
		LPVOID rawToVirtual(DWORD dwAddress);

	protected:
		IMAGE_DOS_HEADER * m_dos;
		IMAGE_NT_HEADERS32 * m_nt;
		IMAGE_SECTION_HEADER * m_sh;

		IMAGE_SECTION_HEADER* _findSection(LPCSTR strSection);

	};

	EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#define GetCurrentModule()					((HMODULE)(&__ImageBase))
#define GetCurrentModulePathW(strDest,len)	(GetModuleFileNameW(GetCurrentModule(), strDest, len))
#define GetCurrentModulePathA(strDest,len)	(GetModuleFileNameA(GetCurrentModule(), strDest, len))

	DWORD GetCurrentModuleDirectoryW(LPWSTR strDest, size_t len) noexcept;
	DWORD GetCurrentModuleDirectoryA(LPSTR strDest, size_t len) noexcept;
}

#ifdef _UNICODE
#define GetCurrentModulePath GetCurrentModulePathW
#define GetCurrentModuleDirectory GetCurrentModuleDirectoryW
#else
#define GetCurrentModulePath GetCurrentModulePathA
#define GetCurrentModuleDirectory GetCurrentModuleDirectoryA
#endif
