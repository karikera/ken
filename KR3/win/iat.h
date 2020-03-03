#pragma once

#include <KR3/data/linkedlist.h>
#include <KR3/wl/windows.h>

namespace kr
{
	namespace hook
	{
		class Function
		{
		public:
			Function();
			Function(HMODULE hModule, LPCSTR strName);
			Function(LPVOID pAddress);
			Function(Function& copy);
			operator LPVOID();

		protected:
			LPVOID m_address;
		};

		class Module
		{
		public:
			Module(LPCSTR strDLL);
			Module(LPCWSTR strDLL);
			~Module();
			Function operator [](LPCSTR strName);
			HMODULE getHandle();

		protected:
			HMODULE m_module;

		};

		class IAT
		{
		public:
			IAT(HMODULE module, LPCSTR dll);
			PULONG_PTR operator [](LPVOID ori);

		protected:
			PIMAGE_THUNK_DATA m_thunk;
		};

		class IATHooker
		{
		public:
			IATHooker();
			IATHooker(PULONG_PTR pTarget, LPVOID pFunc);
			void unhook();

		protected:
			PULONG_PTR m_pTarget;
			ULONG_PTR m_dwBackup;
		};

		class IATModule :public Module, public IAT
		{
		public:
			IATModule(HMODULE hModule, LPCSTR strDLL);
			PULONG_PTR operator [](LPCSTR strName);
			using IAT::operator [];
			IATHooker hooking(LPCSTR strFunction, LPVOID pHook);
		};


		class IATHookerList :public IATModule
		{
		public:
			IATHookerList(HMODULE hModule, LPCSTR strDLL);
			void hooking(LPCSTR strFunction, LPVOID pHook);
			void unhook();

		protected:
			LinkedList<IATHooker> m_vList;
		};

		class Unprotector
		{
		public:
			Unprotector(void* pDest, size_t nSize);
			~Unprotector();
			template <typename T> operator T* ()
			{
				return (T*)m_pDest;
			}

		protected:
			void* m_pDest;
			size_t m_nSize;
			DWORD m_dwOldPage;
		};

		class DllSearcher
		{
		public:
			IMAGE_IMPORT_DESCRIPTOR * iat;
			HMODULE module;

			DllSearcher() noexcept;
			pcstr first() noexcept;
			pcstr next() noexcept;
		};

		void forceFill(void* dest, int value, size_t size) noexcept;
		void forceCopy(void* dest, const void* src, size_t size) noexcept;
		template <typename T> void forceSet(T& dest, const T& src) noexcept
		{
			forceCopy(&dest, &src, sizeof(T));
		}

	}
}