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
				ULONG_PTR ordinal;
				LPCSTR name;
			};
			class Iterator
			{
			public:
				Iterator(win::Module* module, PIMAGE_IMPORT_DESCRIPTOR desc) noexcept;
				Iterator& operator ++() noexcept;
				FunctionDesc operator *() const noexcept;
				bool isEnd() const noexcept;

			private:
				win::Module* m_module;
				PIMAGE_THUNK_DATA m_ilt;
				PIMAGE_THUNK_DATA m_iat;
			};
			Iterator begin() const noexcept;
			IteratorEnd end() const noexcept;

			IATModule(win::Module* hModule, LPCSTR strDLL) noexcept;
			PULONG_PTR getFunctionStore(LPCSTR name) noexcept;
			PULONG_PTR getFunctionStore(ULONG_PTR ordinal) noexcept;
			IATHooker hooking(LPCSTR functionName, LPVOID pHook) noexcept;
			IATHooker hooking(uintptr_t functionName, LPVOID pHook) noexcept;

		private:
			win::Module* m_module;
			PIMAGE_IMPORT_DESCRIPTOR m_desc;
		};


		class IATHookerList :public IATModule
		{
		public:
			IATHookerList(win::Module* module, LPCSTR dll) noexcept;
			void hooking(LPCSTR func, LPVOID hook) noexcept;
			void hooking(ULONG_PTR func, LPVOID hook) noexcept;
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

		enum Register2
		{
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15
		};

		enum AddressPointerRule
		{
			QwordPtr
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
			void mov(Register r, dword to) noexcept;
			void mov(Register r, qword to) noexcept;
			void mov(Register2 r, qword to) noexcept;
			void jump64(void* to, Register r) noexcept;
			void call64(void* to, Register r) noexcept;
			void jump(Register r) noexcept;
			void call(Register r) noexcept;
			void call(Register2 r) noexcept;
#endif
			void push(Register r) noexcept;
			void pop(Register r) noexcept;
			void mov(Register dest, Register src) noexcept;
			void mov(Register dest, Register2 src) noexcept;
			void mov(Register2 dest, Register src) noexcept;
			void mov(Register2 dest, Register2 src) noexcept;
			void movb(Register2 dest, Register2 src) noexcept;
			void mov(AddressPointerRule address, Register dest, int8_t offset, Register src) noexcept;
			void mov(Register dest, AddressPointerRule address, Register src, int8_t offset) noexcept;
			void mov(Register dest, AddressPointerRule address, Register src, int32_t offset) noexcept;
			void sub(Register dest, char chr) noexcept;
			void add(Register dest, char chr) noexcept;
			void jump(void* to, Register tmp) noexcept;
			void call(void* to, Register tmp) noexcept;
			void ret() noexcept;

		};

		void* createCodeJunction(void* dest, size_t size, void (*func)(), Register temp) noexcept;

	}

}