#pragma once

#include <KR3/data/linkedlist.h>
#include <KR3/win/windows.h>

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
				Iterator(win::Module* winmodule, PIMAGE_IMPORT_DESCRIPTOR desc) noexcept;
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
			IATHookerList(win::Module* winmodule, LPCSTR dll) noexcept;
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
			HMODULE winmodule;

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
			void* alloc(size_t size, size_t alignment) noexcept;
			void shrink(void* end) noexcept;

			static ExecutableAllocator* getInstance() noexcept;

		private:
			ondebug(byte* m_alloc_begin);
			byte* m_page;
			byte* m_page_end;

		};

		enum RegisterLow
		{
			AL,
			DL,
			CL,
			BL,
			AH,
			DH,
			CH,
			BH,

			R8B,
			R9B,
			R10B,
			R11B,
			R12B,
			R13B,
			R14B,
			R15B
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
			RDI,

			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15
		};

		enum FloatRegister
		{
			XMM0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,

			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15,
		};

		enum AddressPointerRule
		{
			BytePtr,
			DwordPtr,
			QwordPtr
		};

		enum class RegSize
		{
			Byte,
			Word,
			Dword,
			Qword,
		};

		enum class AccessType
		{
			Register,
			Write,
			Read,
			Lea,
			WriteConst,
			PutConst,
		};

		enum class Operator
		{
			ADD,
			OR,
			ADC,
			SBB,
			AND,
			SUB,
			XOR,
			CMP,
		};

		struct CodeDiff :TmpArray<pair<size_t, size_t>>
		{
			bool succeeded() noexcept;
		};

		class CodeWriter :public ArrayWriter<byte>
		{
		public:
			CodeWriter(void* dest, size_t size) noexcept;
			CodeWriter(void* dest, void* dest_end) noexcept;
			~CodeWriter() noexcept;

			void fillNop() noexcept;
			void fillDebugBreak() noexcept;
			void rjump(int32_t rpos) throws(NotEnoughSpaceException);
			void rcall(int32_t rpos) throws(NotEnoughSpaceException);
			void mov_gs(Register dest, AddressPointerRule, int32_t offset) throws(NotEnoughSpaceException);
#ifdef _M_X64
			void mov(Register r, dword to) throws(NotEnoughSpaceException);
			void mov(Register r, qword to) throws(NotEnoughSpaceException);
			void jump64ex(void* to, Register r, bool isCall) throws(NotEnoughSpaceException);
			void jump64(void* to, Register r) throws(NotEnoughSpaceException);
			void call64(void* to, Register r) throws(NotEnoughSpaceException);
			void jumpex(Register r, bool isCall) throws(NotEnoughSpaceException);
			void jump(Register r) throws(NotEnoughSpaceException);
			void call(Register r) throws(NotEnoughSpaceException);
#endif
			void jumpex(AddressPointerRule address, Register r, int32_t offset, bool isCall) throws(NotEnoughSpaceException);
			void jump(AddressPointerRule address, Register r, int32_t offset) throws(NotEnoughSpaceException);
			void call(AddressPointerRule address, Register r, int32_t offset) throws(NotEnoughSpaceException);
			void push(Register r) throws(NotEnoughSpaceException);
			void pop(Register r) throws(NotEnoughSpaceException);
			void push(int32_t value) throws(NotEnoughSpaceException);
			void mov(Register dest, Register src) throws(NotEnoughSpaceException);
			void movb(Register dest, Register src) throws(NotEnoughSpaceException);
			void movsxd(Register dest, AddressPointerRule address, Register src, int32_t offset) throws(NotEnoughSpaceException);
			void movzx(Register dest, AddressPointerRule address, Register src, int32_t offset) throws(NotEnoughSpaceException);
			void movex(RegSize bittype, Register reg1, int32_t reg2_or_constvalue, AccessType atype, int32_t offset) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, int32_t value) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, int32_t offset, int32_t value) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, RegisterLow src) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, int32_t offset, RegisterLow src) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, Register src) throws(NotEnoughSpaceException);
			void mov(AddressPointerRule address, Register dest, int32_t offset, Register src) throws(NotEnoughSpaceException);
			void mov(Register dest, AddressPointerRule address, Register src, int32_t offset = 0) throws(NotEnoughSpaceException);
			void lea(Register dest, Register src, int32_t offset = 0) throws(NotEnoughSpaceException);
			void operex(Operator oper, Register dest, int32_t offset, int32_t reg2_or_const, AccessType atype) throws(NotEnoughSpaceException);
			void test(Register dest, Register src) throws(NotEnoughSpaceException);
			void test_b(Register dest, Register src) throws(NotEnoughSpaceException);

			void cmp(Register dest, Register src) throws(NotEnoughSpaceException);
			void sub(Register dest, Register src) throws(NotEnoughSpaceException);
			void add(Register dest, Register src) throws(NotEnoughSpaceException);
			void sbb(Register dest, Register src) throws(NotEnoughSpaceException);
			void adc(Register dest, Register src) throws(NotEnoughSpaceException);
			void xor_(Register dest, Register src) throws(NotEnoughSpaceException);
			void or_(Register dest, Register src) throws(NotEnoughSpaceException);
			void and_(Register dest, Register src) throws(NotEnoughSpaceException);

			void cmp(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void sub(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void add(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void sbb(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void adc(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void xor_(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void or_(Register dest, int32_t chr) throws(NotEnoughSpaceException);
			void and_(Register dest, int32_t chr) throws(NotEnoughSpaceException);

			void cmp(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void sub(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void add(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void sbb(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void adc(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void xor_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void or_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);
			void and_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) throws(NotEnoughSpaceException);

			void jump(void* to, Register tmp) throws(NotEnoughSpaceException);
			void jumpWithoutTemp(void* to) throws(NotEnoughSpaceException);
			void call(void* to, Register tmp) throws(NotEnoughSpaceException);
			void jz(int32_t offset) throws(NotEnoughSpaceException);
			void jnz(int32_t offset) throws(NotEnoughSpaceException);
			void ret() throws(NotEnoughSpaceException);
			void debugBreak() throws(NotEnoughSpaceException);
			void movss(AddressPointerRule atype, Register dest, int32_t offset, FloatRegister r) throws(NotEnoughSpaceException);
			void movsd(AddressPointerRule atype, Register dest, int32_t offset, FloatRegister r) throws(NotEnoughSpaceException);
			void movss(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset = 0) throws(NotEnoughSpaceException);
			void movsd(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset = 0) throws(NotEnoughSpaceException);
			void movsd(FloatRegister dest, FloatRegister src) throws(NotEnoughSpaceException);
			void movss(FloatRegister dest, FloatRegister src) throws(NotEnoughSpaceException);
			void cvttsd2ss(FloatRegister dest, Register r) throws(NotEnoughSpaceException);
			void cvttsi2sd(FloatRegister dest, Register r) throws(NotEnoughSpaceException);
			void cvttsd2ss(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset = 0) throws(NotEnoughSpaceException);
			void cvttsi2sd(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset = 0) throws(NotEnoughSpaceException);
			void cvttsd2si(Register dest, AddressPointerRule atype, int32_t value) throws(NotEnoughSpaceException);
			void cvttsd2si(Register dest, AddressPointerRule atype, Register r, int32_t offset = 0) throws(NotEnoughSpaceException);
			void cvttsd2si(Register dest, FloatRegister xmm) throws(NotEnoughSpaceException);
			void cmovz(Register a, Register b) throws(NotEnoughSpaceException);
			void cmovnz(Register a, Register b) throws(NotEnoughSpaceException);

			static CodeDiff check(void* code, Buffer originalCode, View<pair<size_t, size_t> > skip = nullptr) noexcept;
			static CodeDiff hook(void* from, void* to, View<uint8_t> originalCode, View<pair<size_t, size_t>> skip = nullptr) noexcept;
			static CodeDiff nopping(void* base, View<uint8_t> originalCode, View<pair<size_t, size_t>> skip = nullptr) noexcept;

		private:
			void _writeRegEx(int r, int r2, RegSize size) throws(NotEnoughSpaceException);
			void _writeOffset(uint8_t opcode, Register r, int32_t offset, bool registerOperation) throws(NotEnoughSpaceException);
		};

		class JitFunction: public CodeWriter
		{
		public:
			JitFunction(size_t size) noexcept;
			JitFunction(ExecutableAllocator* alloc, size_t size) noexcept;
			~JitFunction() noexcept;
			bool shrinked() noexcept;
			void shrink() noexcept;

			void* pointer() noexcept;
			CodeDiff patchTo(void* base, Buffer originalCode, Register tempregister, bool jump, View<std::pair<size_t, size_t>> skip = nullptr) noexcept;
			CodeDiff patchTo_jz(void* base, Register testregister, void* jumpPoint, Buffer originalCode, Register tempregister) noexcept;

		private:
			ExecutableAllocator* m_alloc;
			void* m_ptr;
		};

		void* createCodeJunction(void* dest, size_t size, void (*func)(), Register temp) noexcept;

		CodeDiff memdiff(const void* _src, const void* _dst, size_t size) noexcept;
		bool memdiff_contains(View<pair<size_t, size_t>> larger, View<pair<size_t, size_t>> smaller) noexcept;

	}

}