#pragma once

#include <KR3/data/linkedlist.h>
#include <KR3/wl/windows.h>

namespace kr
{
	namespace hook
	{
		//class Hooker :public Function
		//{
		//public:
		//	Hooker();
		//	void hooking(Function& ori, LPVOID pHook);

		//	void hook();
		//	void unhook();
		//	using Function::operator LPVOID;

		//protected:
		//	BYTE m_backup[5];
		//	LPVOID m_hook;
		//};

		class IATHooker
		{
		public:
			IATHooker() noexcept;
			IATHooker(PULONG_PTR target, LPVOID func) noexcept;
			void unhook() noexcept;

		protected:
			PULONG_PTR m_target;
			ULONG_PTR m_backup;
		};

		class IATModule
		{
		public:
			struct FunctionDesc
			{
				PULONG_PTR store;
				LPCSTR name;
			};
			class Iterator
			{
			public:
				Iterator(win::Module* module, PIMAGE_IMPORT_DESCRIPTOR desc) noexcept;
				Iterator& operator ++() noexcept;
				FunctionDesc operator *() const noexcept;
				bool operator != (const IteratorEnd&) const noexcept;
				bool operator == (const IteratorEnd&) const noexcept;

			private:
				win::Module* m_module;
				PIMAGE_THUNK_DATA m_ilt;
				PIMAGE_THUNK_DATA m_iat;
			};
			Iterator begin() const noexcept;
			IteratorEnd end() const noexcept;

			IATModule(win::Module* hModule, LPCSTR strDLL) noexcept;
			PULONG_PTR getFunctionStore(LPCSTR name) noexcept;
			FunctionDesc getFunctionDesc(void* func) noexcept;
			IATHooker hooking(LPCSTR functionName, LPVOID pHook) noexcept;

		private:
			win::Module* m_module;
			PIMAGE_IMPORT_DESCRIPTOR m_desc;
		};


		class IATHookerList :public IATModule
		{
		public:
			IATHookerList(win::Module* module, LPCSTR dll) noexcept;
			void hooking(LPCSTR func, LPVOID hook) noexcept;
			void unhook() noexcept;

		protected:
			LinkedList<Node<IATHooker>> m_list;
		};

		class Unprotector
		{
		public:
			Unprotector(void* pDest, size_t nSize);
			~Unprotector();
			template <typename T> operator T* ()
			{
				return (T*)m_dest;
			}

		protected:
			void* m_dest;
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

		class ExecutableAllocator
		{
		public:
			ExecutableAllocator() noexcept;
			void* alloc(size_t size) noexcept;

			static ExecutableAllocator* getInstance() noexcept;

		private:
			byte* m_page;
			byte* m_page_end;

		};

		enum Register
		{
			RAX,
			RCX,
			RDX,
			RBX,
			RSP,
			RBP,
			RSI,
			RDI
		};

		class CodeWriter :public ArrayWriter<byte>
		{
		public:
			CodeWriter(ExecutableAllocator* alloc, size_t size) noexcept;
			CodeWriter(void* dest, size_t size) noexcept;

			void fillNop() noexcept;
			void rjump(int32_t rpos) noexcept;
			void rcall(int32_t rpos) noexcept;
#ifdef _M_X64
			void store32_to_64(dword to) noexcept;
			void store64(qword to) noexcept;
			void jump64(void* to) noexcept;
			void call64(void* to) noexcept;
#endif
			void pushRsp() noexcept;
			void mov(Register dest, Register src) noexcept;
			void sub(Register dest, char chr) noexcept;
			void add(Register dest, char chr) noexcept;
			void jump(void* to) noexcept;
			void call(void* to) noexcept;
			void ret() noexcept;

		};

		void* createCodeJunction(void* dest, size_t size, void (*func)()) noexcept;

	}

}