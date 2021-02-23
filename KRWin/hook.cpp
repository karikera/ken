
#include "stdafx.h"
#include "hook.h"
#include "handle.h"
#include <KR3/util/unaligned.h>

using namespace kr;
using namespace hook;

struct PEStructure
{
	IMAGE_DEBUG_DIRECTORY* img_debug_dir;
	IMAGE_IMPORT_DESCRIPTOR* img_import_desc;

	PEStructure(HMODULE winmodule) noexcept
	{
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)winmodule;
		_assert(dos->e_magic == IMAGE_DOS_SIGNATURE);

		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((byte*)dos + dos->e_lfanew);

		img_debug_dir = (IMAGE_DEBUG_DIRECTORY*)(
			(byte*)dos + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);
		img_debug_dir->PointerToRawData;

		img_import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(
			(byte*)dos + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	}
};

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
	HMODULE winmodule = GetModuleHandle(nullptr);
	this->winmodule = winmodule;
	this->iat = PEStructure(winmodule).img_import_desc;
}
pcstr DllSearcher::first() noexcept
{
	return (LPCSTR)((uintptr_t)winmodule + iat->Name);
}
pcstr DllSearcher::next() noexcept
{
	iat++;
	if (iat->Name == 0) return nullptr;
	return (LPCSTR)((uintptr_t)winmodule + iat->Name);
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

IATModule::IATModule(win::Module* winmodule, LPCSTR dll) noexcept
	: m_module(winmodule)
{
	IMAGE_IMPORT_DESCRIPTOR* importdesc = PEStructure(winmodule).img_import_desc;
	for (; importdesc->Name; importdesc++)
	{
		LPCSTR szLibName = (LPCSTR)((uintptr_t)winmodule + importdesc->Name);
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
IATHooker IATModule::hooking(uintptr_t functionName, LPVOID pHook) noexcept
{
	PULONG_PTR pTarget = getFunctionStore(functionName);
	if (pTarget == nullptr) return IATHooker();
	else return IATHooker(pTarget, pHook);
}

IATModule::Iterator::Iterator(win::Module * winmodule, PIMAGE_IMPORT_DESCRIPTOR desc) noexcept
{
	m_module = winmodule;
	m_ilt = (PIMAGE_THUNK_DATA)((uintptr_t)m_module + desc->OriginalFirstThunk);
	m_iat = (PIMAGE_THUNK_DATA)((uintptr_t)m_module + desc->FirstThunk);
}
IATModule::FunctionDesc IATModule::Iterator::operator *() const noexcept
{
	_assert(m_ilt != nullptr);
	FunctionDesc desc;
	desc.store = &m_iat->u1.Function;

	if (m_ilt->u1.AddressOfData & IMAGE_ORDINAL_FLAG)
	{
		desc.ordinal = IMAGE_ORDINAL(m_ilt->u1.AddressOfData);
		desc.name = nullptr;
	}
	else
	{
		PIMAGE_IMPORT_BY_NAME namedata = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)m_module + m_ilt->u1.AddressOfData);
		desc.ordinal = 0;
		desc.name = namedata->Name;
	}
	return desc;
}
bool IATModule::Iterator::isEnd() const noexcept
{
	return m_ilt == nullptr;
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
PULONG_PTR IATModule::getFunctionStore(ULONG_PTR ordinal) noexcept
{
	for (FunctionDesc desc : *this)
	{
		if (desc.ordinal == ordinal)
		{
			return desc.store;
		}
	}
	return nullptr;
}

IATHookerList::IATHookerList(win::Module* winmodule, LPCSTR dll) noexcept :IATModule(winmodule, dll)
{
}
void IATHookerList::hooking(LPCSTR func, LPVOID hook) noexcept
{
	m_list.create(IATModule::hooking(func, hook));
}
void IATHookerList::hooking(ULONG_PTR func, LPVOID hook) noexcept
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

// 48 b8 35 08 40 00 00 00 00 00   mov rax, 0x0000000000400835
// ff e0                           jmp rax

ExecutableAllocator::ExecutableAllocator() noexcept
{
	m_page = nullptr;
	m_page_end = nullptr;
}
void* ExecutableAllocator::alloc(size_t size) noexcept
{
	size_t remaining = m_page_end - m_page;
	if (remaining <= size)
	{
		dword pageSize = getAllocationGranularity();
		size_t needsize = (size + pageSize - 1 - remaining) & ~(size_t)(pageSize - 1);
		void* next = VirtualAlloc(m_page_end, needsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (next != nullptr)
		{
			if (m_page_end == nullptr) m_page = (byte*)next;
			m_page_end = (byte*)next + needsize;
		}
		else
		{
			if (m_page_end == nullptr) notEnoughMemory();
			needsize = (size + pageSize - 1) & ~(size_t)(pageSize - 1);
			m_page = (byte*)VirtualAlloc(nullptr, needsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (m_page == nullptr) notEnoughMemory();
			m_page = m_page + needsize;
		}
	}
	void* out = m_page;
	ondebug(m_alloc_begin = m_page);
	m_page += size;
	return out;
}
void* ExecutableAllocator::alloc(size_t size, size_t alignment) noexcept
{
	uintptr_t missed = ((uintptr_t)m_page) % alignment;
	missed = (alignment - missed) % alignment;
	m_page += missed;
	return alloc(size);
}
void ExecutableAllocator::shrink(void* end) noexcept
{
	ondebug(_assert(m_alloc_begin <= (byte*)end));
	_assert((byte*)end <= m_page);
	m_page = (byte*)end;
}
ExecutableAllocator* ExecutableAllocator::getInstance() noexcept
{
	static ExecutableAllocator alloc;
	return &alloc;
}


namespace
{
	bool regex(int r) noexcept
	{
		return r >= 8;
	}
	bool regex(Register r) noexcept
	{
		return r >= R8;
	}
	bool regex(FloatRegister r) noexcept
	{
		return r >= XMM8;
	}
	byte regidx(Register r) noexcept
	{
		return r % R8;
	}
	RegSize ptr_to_size(AddressPointerRule rule) noexcept
	{
		if (rule == QwordPtr) return RegSize::Qword;
		else if (rule == DwordPtr) return RegSize::Dword;
		else if (rule == BytePtr) return RegSize::Byte;
		else unreachable();
	}
}

CodeWriter::CodeWriter(void* dest, void* dest_end) noexcept
	:ArrayWriter<byte>((byte*)dest, (byte*)dest_end)
{
}
CodeWriter::CodeWriter(void* dest, size_t size) noexcept
	:ArrayWriter<byte>((byte*)dest, size)
{
}
CodeWriter::~CodeWriter() noexcept
{
}

void CodeWriter::_writeRegEx(int r1, int r2, RegSize size) noexcept
{
	if (size == RegSize::Word) write(0x66);

	byte rex = 0x40;
	if (size == RegSize::Qword) rex |= 0x08;
	if (regex(r1)) rex |= 0x01;
	if (regex(r2)) rex |= 0x04;
	if (rex != 0x40) write(rex);
}

void CodeWriter::fillNop() noexcept
{
	writeFill(0x90, remaining());
}
void CodeWriter::fillDebugBreak() noexcept
{
	writeFill(0xcc, remaining());
}
void CodeWriter::rjump(int32_t rpos) noexcept
{
	write(0xe9);
	writeas(rpos);
}
void CodeWriter::rcall(int32_t rpos) noexcept
{
	write(0xe8);
	writeas(rpos);
}
void CodeWriter::mov_gs(Register dest, AddressPointerRule, int32_t offset) noexcept
{
	write(0x65);
	write(0x48);
	write(0x8b);
	write(0x04 | (regidx(dest) << 3));
	write(0x25);
	writeas<int32_t>(offset);
}
#ifdef _M_X64
void CodeWriter::mov(Register r, dword to) noexcept
{
	_writeRegEx(r, 0, RegSize::Qword);
	write(0xc7);
	write(0xC0 | regidx(r));
	writeas(to);
}
void CodeWriter::mov(Register r, qword to) noexcept
{
	if ((dword)to == to) return mov(r, (dword)to);
	_writeRegEx(r, 0, RegSize::Qword);
	write(0xb8 | regidx(r));
	writeas(to);
}
void CodeWriter::jump64ex(void* to, Register r, bool isCall) noexcept
{
	mov(r, (uintptr_t)to);
	jumpex(r, isCall);
}
void CodeWriter::jump64(void* to, Register r) noexcept
{
	jump64ex(to, r, false);
}
void CodeWriter::call64(void* to, Register r) noexcept
{
	jump64ex(to, r, true);
}
void CodeWriter::jumpex(Register r, bool isCall) noexcept
{
	if (regex(r)) write(0x41);
	write(0xff);
	write((isCall ? 0xd0 : 0xe0) | regidx(r));
}
void CodeWriter::jump(Register r) noexcept
{
	jumpex(r, false);
}
void CodeWriter::call(Register r) noexcept
{
	jumpex(r, true);
}
#endif
void CodeWriter::jumpex(AddressPointerRule address, Register r, int32_t offset, bool isCall) noexcept
{
	if (regex(r)) write(0x41);
	write(0xff);
	_writeOffset((isCall ? 0x10 : 0x20) | regidx(r), r, offset, false);
}
void CodeWriter::jump(AddressPointerRule address, Register r, int32_t offset) noexcept
{
	jumpex(address, r, offset, false);
}
void CodeWriter::call(AddressPointerRule address, Register r, int32_t offset) noexcept
{
	jumpex(address, r, offset, true);
}
void CodeWriter::push(int32_t value) noexcept
{
	if (value == (int8_t)value)
	{
		write(0x6A);
		writeas<int8_t>(value);
	}
	else
	{
		write(0x68);
		writeas<int32_t>(value);
	}
}
void CodeWriter::push(Register r) noexcept
{
	if (regex(r)) write(0x41);
	write(0x50 | regidx(r));
}
void CodeWriter::pop(Register r) noexcept
{
	if (regex(r)) write(0x41);
	write(0x58 | regidx(r));
}
void CodeWriter::mov(Register dest, Register src) noexcept
{
	movex(RegSize::Qword, dest, src, AccessType::Register, 0);
}
void CodeWriter::movb(Register dest, Register src) noexcept
{
	movex(RegSize::Byte, dest, src, AccessType::Register, 0);
}
void CodeWriter::movsxd(Register dest, AddressPointerRule address, Register src, int32_t offset) noexcept
{
	_assert(address == DwordPtr);
	_writeRegEx(src, dest, RegSize::Qword);
	write(0x63);
	_writeOffset((dest << 3) | src, src, offset, false);
}
void CodeWriter::movzx(Register dest, AddressPointerRule address, Register src, int32_t offset) noexcept
{
	_assert(address == BytePtr);
	_writeRegEx(src, dest, RegSize::Qword);
	write(0x0f);
	write(0xb6);
	_writeOffset((dest << 3) | src, src, offset, false);
}
void CodeWriter::movex(RegSize bittype, Register reg1, int32_t reg2_or_constvalue, AccessType atype, int32_t offset) noexcept
{
	// reg1 is memory address

	Register reg2 = (Register)reg2_or_constvalue;
	bool reg2_is_constvalue = atype == AccessType::WriteConst;

	_writeRegEx(reg1, reg2_is_constvalue ? 0 : reg2, bittype);


	if (atype == AccessType::WriteConst)
	{
		if (bittype == RegSize::Byte)
		{
			write(0xc6);
		}
		else
		{
			_assert(bittype == RegSize::Word || bittype == RegSize::Dword || bittype == RegSize::Qword);
			write(0xc7);
		}
	}
	else
	{
		_assert(atype != AccessType::Lea || (bittype == RegSize::Dword || bittype == RegSize::Qword));

		byte memorytype = 0x88;
		if (atype == AccessType::Lea) memorytype |= 0x05;
		else if (atype == AccessType::Read) memorytype |= 0x02;
		if (bittype != RegSize::Byte) memorytype |= 0x01;
		write(memorytype);
	}

	byte opcode = regidx(reg1);
	if (!reg2_is_constvalue) opcode |= (regidx(reg2) << 3);

	_writeOffset(opcode, reg1, offset, atype == AccessType::Register);

	if (reg2_is_constvalue)
	{
		if (bittype == RegSize::Byte)
		{
			writeas<int8_t>((int8_t)reg2_or_constvalue);
		}
		else if (bittype == RegSize::Word)
		{
			writeas<int16_t>((int16_t)reg2_or_constvalue);
		}
		else
		{
			_assert(bittype == RegSize::Dword || bittype == RegSize::Qword);
			writeas<int32_t>(reg2_or_constvalue);
		}
	}
}
void CodeWriter::mov(AddressPointerRule address, Register dest, int32_t value) noexcept
{
	movex(ptr_to_size(address), dest, value, AccessType::WriteConst, 0);
}
void CodeWriter::mov(AddressPointerRule address, Register dest, int32_t offset, int32_t value) noexcept
{
	movex(ptr_to_size(address), dest, value, AccessType::WriteConst, offset);
}
void CodeWriter::mov(AddressPointerRule address, Register dest, RegisterLow src) noexcept
{
	mov(address, dest, 0, src);
}
void CodeWriter::mov(AddressPointerRule address, Register dest, int32_t offset, RegisterLow src) noexcept
{
	_assert(address == BytePtr);
	movex(ptr_to_size(address), dest, src, AccessType::Write, offset);
}
void CodeWriter::mov(AddressPointerRule address, Register dest, Register src) noexcept
{
	mov(address, dest, 0, src);
}
void CodeWriter::mov(AddressPointerRule address, Register dest, int32_t offset, Register src) noexcept
{
	movex(ptr_to_size(address), dest, src, AccessType::Write, offset);
}
void CodeWriter::mov(Register dest, AddressPointerRule address, Register src, int32_t offset) noexcept
{
	movex(ptr_to_size(address), src, dest, AccessType::Read, offset);
}
void CodeWriter::lea(Register dest, Register src, int32_t offset) noexcept
{
	if (offset == 0) mov(dest, src);
	else movex(RegSize::Qword, src, dest, AccessType::Lea, offset);
}
void CodeWriter::operex(Operator oper, Register dest, int32_t offset, int32_t reg2_or_const, AccessType atype) noexcept
{
	_writeRegEx(dest, 0, RegSize::Qword);

	if (atype == AccessType::Register)
	{
		Register src = (Register)reg2_or_const;
		write(0x01 | ((int)oper << 3));
		_writeOffset(regidx(dest) | (regidx(src) << 3), dest, offset, true);
	}
	else
	{
		int32_t chr = reg2_or_const;

		int is32bits = (int8_t)chr != chr;
		if (is32bits && dest == RAX)
		{
			write(0x05 | ((int)oper << 3));
			writeas<int32_t>(chr);
		}
		else
		{
			write(0x83 ^ (is32bits << 1));
			_writeOffset(((int)oper << 3) | regidx(dest), dest, offset, atype == AccessType::Register || atype == AccessType::PutConst);

			if (is32bits) writeas<int32_t>(chr);
			else write((int8_t)chr);
		}
	}
}
void CodeWriter::test(Register dest, Register src) noexcept
{
	_writeRegEx(src, dest, RegSize::Qword);
	write(0x85);
	write(0xC0 | (src << 3) | dest);
}
void CodeWriter::test_b(Register dest, Register src) noexcept
{
	_writeRegEx(src, dest, RegSize::Qword);
	write(0x84);
	write(0xC0 | (src << 3) | dest);
}

void CodeWriter::cmp(Register dest, Register src) noexcept
{
	operex(Operator::CMP, dest, 0, src, AccessType::Register);
}
void CodeWriter::sub(Register dest, Register src) noexcept
{
	operex(Operator::SUB, dest, 0, src, AccessType::Register);
}
void CodeWriter::add(Register dest, Register src) noexcept
{
	operex(Operator::ADD, dest, 0, src, AccessType::Register);
}
void CodeWriter::sbb(Register dest, Register src) noexcept
{
	operex(Operator::SBB, dest, 0, src, AccessType::Register);
}
void CodeWriter::adc(Register dest, Register src) noexcept
{
	operex(Operator::ADC, dest, 0, src, AccessType::Register);
}
void CodeWriter::xor_(Register dest, Register src) noexcept
{
	operex(Operator::XOR, dest, 0, src, AccessType::Register);
}
void CodeWriter::or_(Register dest, Register src) noexcept
{
	operex(Operator::OR, dest, 0, src, AccessType::Register);
}
void CodeWriter::and_(Register dest, Register src) noexcept
{
	operex(Operator::AND, dest, 0, src, AccessType::Register);
}

void CodeWriter::cmp(Register dest, int32_t chr) noexcept
{
	operex(Operator::CMP, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::sub(Register dest, int32_t chr) noexcept
{
	operex(Operator::SUB, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::add(Register dest, int32_t chr) noexcept
{
	operex(Operator::ADD, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::sbb(Register dest, int32_t chr) noexcept
{
	operex(Operator::SBB, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::adc(Register dest, int32_t chr) noexcept
{
	operex(Operator::ADC, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::xor_(Register dest, int32_t chr) noexcept
{
	operex(Operator::XOR, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::or_(Register dest, int32_t chr) noexcept
{
	operex(Operator::OR, dest, 0, chr, AccessType::PutConst);
}
void CodeWriter::and_(Register dest, int32_t chr) noexcept
{
	operex(Operator::AND, dest, 0, chr, AccessType::PutConst);
}

void CodeWriter::cmp(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::CMP, dest, offset, chr, AccessType::Write);
}
void CodeWriter::sub(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::SUB, dest, offset, chr, AccessType::Write);
}
void CodeWriter::add(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::ADD, dest, offset, chr, AccessType::Write);
}
void CodeWriter::sbb(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::SBB, dest, offset, chr, AccessType::Write);
}
void CodeWriter::adc(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::ADC, dest, offset, chr, AccessType::Write);
}
void CodeWriter::xor_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::XOR, dest, offset, chr, AccessType::Write);
}
void CodeWriter::or_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::OR, dest, offset, chr, AccessType::Write);
}
void CodeWriter::and_(AddressPointerRule address, Register dest, int32_t offset, int32_t chr) noexcept
{
	operex(Operator::AND, dest, offset, chr, AccessType::Write);
}

void CodeWriter::jump(void* to, Register tmp) noexcept
{
	intptr_t rjumppos = (intptr_t)((byte*)to - end() - 5);
#ifdef _M_X64
	if ((int)rjumppos != rjumppos)
	{
		jump64(to, tmp);
	}
	else
	{
		rjump((int)rjumppos);
	}
#else
	rjump((int)rjumppos);
#endif
}
void CodeWriter::jumpWithoutTemp(void* to) noexcept
{
#ifdef _M_X64
	push(RCX);
	mov(DwordPtr, RSP, 4, (uint32_t)(((uint64_t)to) >> 32));
	mov(DwordPtr, RSP, 0, (uint32_t)((uint64_t)to));
	ret();
#else
	intptr_t rjumppos = (intptr_t)((byte*)to - end() - 5);
	rjump((int)rjumppos);
#endif
}
void CodeWriter::call(void* to, Register tmp) noexcept
{
	intptr_t rjumppos = (intptr_t)((byte*)to - end() - 5);
#ifdef _M_X64
	if ((int)rjumppos != rjumppos)
	{
		call64(to, tmp);
	}
	else
	{
		rcall((int)rjumppos);
	}
#else
	rcall((int)rjumppos);
#endif
}
void CodeWriter::jz(int32_t offset) noexcept
{
	if ((int8_t)offset == offset)
	{
		write(0x74);
		write((int8_t)offset);
	}
	else
	{
		write(0x0f);
		write(0x84);
		writeas(offset);
	}
}
void CodeWriter::jnz(int32_t offset) noexcept
{
	if ((int8_t)offset == offset)
	{
		write(0x75);
		write((int8_t)offset);
	}
	else
	{
		write(0x0f);
		write(0x85);
		writeas(offset);
	}
}
void CodeWriter::ret() noexcept
{
	write(0xc3);
}
void CodeWriter::debugBreak() noexcept
{
	write(0xcc);
}

void CodeWriter::_writeOffset(uint8_t opcode, Register r, int32_t offset, bool registerOperation) noexcept
{
	if (registerOperation)
	{
		_assert(offset == 0);
		write(opcode | 0xc0);
		return;
	}
	if (offset == 0 && r != RBP) {}
	else if (offset == (int8_t)offset)
	{
		opcode |= 0x40;
	}
	else
	{
		opcode |= 0x80;
	}
	write(opcode);

	if (r == RSP) write(0x24);
	if (opcode & 0x40) write((int8_t)offset);
	else if (opcode & 0x80) writeas<int32_t>(offset);
}
void CodeWriter::movss(AddressPointerRule atype, Register dest, int32_t offset, FloatRegister r) noexcept
{
	write(0xf3);
	write(0x0f);
	write(0x11);
	_writeOffset(0x04 | (r << 3) | dest, dest, offset, false);
}
void CodeWriter::movsd(AddressPointerRule atype, Register dest, int32_t offset, FloatRegister r) noexcept
{
	write(0xf2);
	write(0x0f);
	write(0x11);
	_writeOffset(0x04 | (r << 3) | dest, dest, offset, false);
}
void CodeWriter::movss(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset) noexcept
{
	write(0xf3);
	write(0x0f);
	write(0x10);
	_writeOffset(0x04 | (dest << 3) | r, r, offset, false);
}
void CodeWriter::movsd(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset) noexcept
{
	write(0xf2);
	write(0x0f);
	write(0x10);
	_writeOffset(0x04 | (dest << 3) | r, r, offset, false);
}
void CodeWriter::movsd(FloatRegister dest, FloatRegister src) noexcept
{
	write(0xf2);
	if (regex(src) || regex(dest)) write(0x48 | (regex(src) ? 1 : 0) | (regex(dest) ? 4 : 0));
	write(0x0f);
	write(0x10);
	write(0xc0 | (dest << 3) | src);
}
void CodeWriter::movss(FloatRegister dest, FloatRegister src) noexcept
{
	write(0xf3);
	if (regex(src) || regex(dest)) write(0x40 | (regex(src) ? 1 : 0) | (regex(dest) ? 4 : 0));
	write(0x0f);
	write(0x10);
	write(0xc0 | (dest << 3) | src);
}
void CodeWriter::cvttsd2ss(FloatRegister dest, Register r) noexcept
{
	write(0xf3);
	_writeRegEx(r, dest, RegSize::Qword);
	write(0x0f);
	write(0x2a);
	write(0xc0 | r | (dest << 3));
}
void CodeWriter::cvttsi2sd(FloatRegister dest, Register r) noexcept
{
	write(0xf2);
	_writeRegEx(r, dest, RegSize::Qword);
	write(0x0f);
	write(0x2a);
	write(0xc0 | r | (dest << 3));
}
void CodeWriter::cvttsd2ss(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset) noexcept
{
	write(0xf3);
	if (atype == QwordPtr || regex(r) || regex(dest)) _writeRegEx(r, dest, RegSize::Qword);
	write(0x0f);
	write(0x2a);
	_writeOffset(r | (dest << 3), r, offset, false);
}
void CodeWriter::cvttsi2sd(FloatRegister dest, AddressPointerRule atype, Register r, int32_t offset) noexcept
{
	write(0xf2);
	if (atype == QwordPtr || regex(r) || regex(dest)) _writeRegEx(r, dest, RegSize::Qword);
	write(0x0f);
	write(0x2a);
	_writeOffset(r | (dest << 3), r, offset, false);
}
void CodeWriter::cvttsd2si(Register dest, AddressPointerRule atype, int32_t value) noexcept
{
	write(0xf2);
	if (atype == QwordPtr || regex(dest)) _writeRegEx(0, dest, RegSize::Qword);
	write(0x0f);
	write(0x2c);

	write(0x04 | (dest << 3));
	write(0x25);
	writeas<int32_t>(value);
}
void CodeWriter::cvttsd2si(Register dest, AddressPointerRule atype, Register r, int32_t offset) noexcept
{
	write(0xf2);
	if (atype == QwordPtr || regex(dest)) _writeRegEx(r, dest, RegSize::Qword);
	write(0x0f);
	write(0x2c);
	_writeOffset((dest << 3) | r, r, offset, false);
}
void CodeWriter::cvttsd2si(Register dest, FloatRegister xmm) noexcept
{
	write(0xf2);
	_writeRegEx(xmm, dest, RegSize::Qword);
	write(0x0f);
	write(0x2c);
	write(0xc0 | (dest << 3) | xmm);
}
void CodeWriter::cmovz(Register a, Register b) noexcept
{
	_writeRegEx(b, a, RegSize::Qword);
	write(0x0f);
	write(0x44);
	write(0xc0 | (a << 3) | b);
}
void CodeWriter::cmovnz(Register a, Register b) noexcept
{
	_writeRegEx(b, a, RegSize::Qword);
	write(0x0f);
	write(0x45);
	write(0xc0 | (a << 3) | b);
}

bool kr::hook::CodeDiff::succeeded() noexcept
{
	return empty();
}

CodeDiff CodeWriter::check(void* code, Buffer originalCode, View<pair<size_t, size_t>> skip) noexcept
{
	CodeDiff diff = memdiff(code, originalCode.data(), originalCode.size());
	if (skip != nullptr)
	{
		if (memdiff_contains(skip, diff)) diff.truncate();
	}
	return diff;
}
CodeDiff CodeWriter::hook(void* from, void* to, kr::View<uint8_t> originalCode, View<std::pair<size_t, size_t>> skip) noexcept
{
	size_t size = originalCode.size();
	Unprotector unpro(from, size);
	CodeDiff diff = check(from, originalCode, skip);
	if (diff.succeeded())
	{
		JitFunction hooker(64 + size);
		void* code = hooker.end();
		hooker.push(RCX);
		hooker.push(RDX);
		hooker.push(R8);
		hooker.push(R9);
		hooker.sub(RSP, 0x28);
		hooker.call(to, RAX);
		hooker.add(RSP, 0x28);
		hooker.pop(R9);
		hooker.pop(R8);
		hooker.pop(RDX);
		hooker.pop(RCX);
		hooker.write(originalCode.cast<byte>());
		hooker.jumpWithoutTemp((byte*)from + size);

		{
			CodeWriter writer((void*)unpro, size);
			writer.jump(code, RAX);
			writer.fillDebugBreak();
		}
	}
	return diff;
}
CodeDiff CodeWriter::nopping(void* base, kr::View<uint8_t> originalCode, kr::View<std::pair<size_t, size_t>> skip) noexcept
{
	size_t codeSize = originalCode.size();
	Unprotector unpro((byte*)base, codeSize);
	CodeDiff diff = check(base, originalCode, skip);
	if (diff.succeeded())
	{
		CodeWriter code((void*)unpro, codeSize);
		code.fillNop();
	}
	return diff;
}

JitFunction::JitFunction(size_t size) noexcept
	:JitFunction(ExecutableAllocator::getInstance(), size)
{
	m_ptr = end();
}
JitFunction::JitFunction(ExecutableAllocator* alloc, size_t size) noexcept
	:CodeWriter((byte*)alloc->alloc(size), size), m_alloc(alloc)
{
}
JitFunction::~JitFunction() noexcept
{
	if (m_alloc == nullptr) return;
	m_alloc->shrink(end());
}
bool JitFunction::shrinked() noexcept
{
	return m_alloc == nullptr;
}
void JitFunction::shrink() noexcept
{
	_assert(!shrinked());
	m_alloc->shrink(end());
	m_alloc = nullptr;
}
void* JitFunction::pointer() noexcept
{
	return m_ptr;
}

CodeDiff JitFunction::patchTo(void* base, Buffer originalCode, kr::hook::Register tempregister, bool jump, View<pair<size_t, size_t>> skip) noexcept
{
	size_t size = originalCode.size();
	Unprotector unpro(base, size);
	CodeDiff diff = check(base, originalCode, skip);
	if (diff.succeeded())
	{
		CodeWriter writer((void*)unpro, size);
		if (jump) writer.jump(m_ptr, tempregister);
		else writer.call(m_ptr, tempregister);
		writer.fillNop();
	}
	return diff;
}
CodeDiff JitFunction::patchTo_jz(void* base, kr::hook::Register testregister, void* jumpPoint, Buffer originalCode, kr::hook::Register tempregister) noexcept
{
	size_t size = originalCode.size();
	Unprotector unpro(base, size);
	CodeDiff diff = check(base, originalCode);
	if (diff.succeeded())
	{
		CodeWriter writer((void*)unpro, size);
		writer.call(m_ptr, tempregister);
		writer.test(testregister, testregister);
		writer.jz(intact<int32_t>((byte*)jumpPoint - (byte*)writer.end() - 6));
		writer.fillNop();
	}
	return diff;
}

void * kr::hook::createCodeJunction(void* dest, size_t size, void (*func)(), Register temp) noexcept
{
	ExecutableAllocator * alloc = ExecutableAllocator::getInstance();
	void * code = alloc->alloc(size + 12 + 12);

	Unprotector unpro(dest, size);
	{
		CodeWriter junction(code, size + 12 + 12);
		junction.call(func, temp);
		junction.write(unpro, size);
		junction.jump(dest, RAX);
	}

	{
		CodeWriter writer((void*)unpro, size);
		writer.jump(code, RAX);
		writer.fillNop();
	}

	return code;
}

CodeDiff kr::hook::memdiff(const void* _src, const void* _dst, size_t size) noexcept
{
	byte* src = (byte*)_src;
	byte* src_end = (byte*)_src + size;
	byte* dst = (byte*)_dst;

	CodeDiff diff;
	pair<size_t, size_t>* last = nullptr;

	for (; src != src_end; src++, dst++)
	{
		if (*src == *dst)
		{
			if (last == nullptr) continue;
			last->second = src - (byte*)_src;
			last = nullptr;
		}
		else
		{
			if (last != nullptr) continue;
			last = diff.prepare(1);
			last->first = src - (byte*)_src;
		}
	}
	if (last != nullptr) last->second = size;
	return diff;
}
bool kr::hook::memdiff_contains(View<pair<size_t, size_t>> larger, View<pair<size_t, size_t>> smaller) noexcept
{
	auto* small = smaller.begin();
	auto* small_end = smaller.end();

	for (auto large : larger)
	{
		for (;;)
		{
			if (small == small_end) return true;

			if (small->first < large.first) return false;
			if (small->first > large.second) break;
			if (small->first == large.second) return false;
			if (small->second > large.second) return false;
			if (small->second == large.second)
			{
				small++;
				break;
			}
			small++;
		}
	}
	return true;
}