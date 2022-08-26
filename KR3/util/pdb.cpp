#include "stdafx.h"
#include "pdb.h"

#include <KR3/win/windows.h>
#include <KR3/win/dynamic_dbghelp.h>

#include <KR3/fs/file.h>
#include <KR3/data/map.h>


using namespace kr;

inline Text tagToString(ULONG tag) noexcept
{
	switch (tag)
	{
	case SymTagNull: return "Null";
	case SymTagExe: return "Exe";
	case SymTagCompiland: return "Compiland";
	case SymTagCompilandDetails: return "CompilandDetails";
	case SymTagCompilandEnv: return "CompilandEnv";
	case SymTagFunction: return "Function";
	case SymTagBlock: return "Block";
	case SymTagData: return "Data";
	case SymTagAnnotation: return "Annotation";
	case SymTagLabel: return "Label";
	case SymTagPublicSymbol: return "PublicSymbol";
	case SymTagUDT: return "UDT";
	case SymTagEnum: return "Enum";
	case SymTagFunctionType: return "FunctionType";
	case SymTagPointerType: return "PointerType";
	case SymTagArrayType: return "ArrayType";
	case SymTagBaseType: return "BaseType";
	case SymTagTypedef: return "Typedef";
	case SymTagBaseClass: return "BaseClass";
	case SymTagFriend: return "Friend";
	case SymTagFunctionArgType: return "FunctionArgType";
	case SymTagFuncDebugStart: return "FuncDebugStart";
	case SymTagFuncDebugEnd: return "FuncDebugEnd";
	case SymTagUsingNamespace: return "UsingNamespace";
	case SymTagVTableShape: return "VTableShape";
	case SymTagVTable: return "VTable";
	case SymTagCustom: return "Custom";
	case SymTagThunk: return "Thunk";
	case SymTagCustomType: return "CustomType";
	case SymTagManagedType: return "ManagedType";
	case SymTagDimension: return "Dimension";
	default: return "Unknown";
	}
}
inline Text16 symToString(ULONG sym) noexcept
{
	switch (sym)
	{
	case SymNone: return u"None";
	case SymExport: return u"Exports";
	case SymCoff: return u"COFF";
	case SymCv: return u"CodeView";
	case SymSym: return u"SYM";
	case SymVirtual: return u"Virtual";
	case SymPdb: return u"PDB";
	case SymDia: return u"DIA";
	case SymDeferred: return u"Deferred";
	default: return u"Unknown";
	}
}

PdbReader::PdbReader() throws(FunctionError)
{
	m_base = 0;
	m_process = nullptr;
}
PdbReader::PdbReader(PdbReader&& _move) noexcept
{
	m_base = _move.m_base;
	m_process = _move.m_process;
	_move.m_base = 0;
	_move.m_process = nullptr;
}
PdbReader::PdbReader(uint64_t base, pcstr16 programPath) throws(FunctionError)
{
	load(base, programPath);
}
void PdbReader::load() throws(FunctionError)
{
	TSZ16 programPath;
	programPath << CurrentApplicationPath() << nullterm;
	HANDLE handle = GetModuleHandleW(nullptr);
	load((uint64_t)handle, programPath.data());
}
void PdbReader::load(uint64_t base, pcstr16 programPath) throws(FunctionError)
{
	_assert(m_base == 0 && m_process == nullptr);

	DbgHelp* dbghelp = DbgHelp::getInstance();

	m_process = GetCurrentProcess();
	if (!dbghelp->SymInitialize(
		m_process,  // Process handle of the current process 
		NULL,                 // No user-defined search path -> use default 
		FALSE                 // Do not load symbols for modules in the current process 
	))
	{
		int err = GetLastError();
		throw FunctionError("SymInitialize", err);
	}

	try
	{
		Must<File> file = File::open(programPath);
		dword filesize = file->size32();

		m_base = dbghelp->SymLoadModuleExW(
			m_process,
			file,
			wide(programPath),
			wide(path16.basename((Text16)programPath).data()),
			base,
			filesize,
			nullptr,
			0
		);
		if (!m_base)
		{
			int err = GetLastError();
			throw FunctionError("SymLoadModuleEx", err);
		}
	}
	catch (Error&)
	{
		int err = GetLastError();
		throw FunctionError("CreateFile", err);
	}
}
PdbReader::~PdbReader() noexcept
{
	close();
}

void* PdbReader::base() noexcept
{
	return (void*)m_base;
}
void* PdbReader::process() noexcept
{
	return m_process;
}
void PdbReader::close() noexcept
{
	if (m_process == nullptr && m_base == 0) return;
	DbgHelp* dbghelp = DbgHelp::getInstance();
	dbghelp->SymUnloadModule64(m_process, m_base);
	m_process = nullptr;
	m_base = 0;
}
PdbReader::SymbolInfo PdbReader::getInfo() throws(FunctionError)
{
	DbgHelp* dbghelp = DbgHelp::getInstance();

	SymbolInfo info;
	static_assert(sizeof(IMAGEHLP_MODULEW64) == sizeof(SymbolInfo::buffer_for_IMAGEHLP_MODULEW64), "buffer size unmatch");

	IMAGEHLP_MODULEW64* ModuleInfo = (IMAGEHLP_MODULEW64*)info.buffer_for_IMAGEHLP_MODULEW64;
	memset(ModuleInfo, 0, sizeof(IMAGEHLP_MODULEW64));
	ModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);

	if (!dbghelp->SymGetModuleInfoW64(m_process, m_base, ModuleInfo))
	{
		int err = GetLastError();
		throw FunctionError("SymGetModuleInfo64", err);
	}

	info.type = symToString(ModuleInfo->SymType);
	info.imageName = (Text16)unwide(ModuleInfo->ImageName);
	info.loadedImageName = (Text16)unwide(ModuleInfo->LoadedImageName);
	info.loadedPdbName = (Text16)unwide(ModuleInfo->LoadedPdbName);

	// Is debug information unmatched ? 
	// (It can only happen if the debug information is contained 
	// in a separate file (.DBG or .PDB) 
	info.unmatched = ModuleInfo->PdbUnmatched || ModuleInfo->DbgUnmatched;

	// Contents 
	info.lineNumbers = ModuleInfo->LineNumbers;
	info.globalSymbols = ModuleInfo->GlobalSymbols;
	info.typeInfo = ModuleInfo->TypeInfo;
	info.sourceIndexed = ModuleInfo->SourceIndexed;
	info.publics = ModuleInfo->Publics;
	return info;
}
AText PdbReader::getTypeName(uint32_t typeId) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();

	//switch (typeId)
	//{
	//case 0: return "[no type]";
	//case 1: return "void";
	//case 2: return "char";
	//case 3: return "wchar_t";
	//case 6: return "int";
	//case 7: return "unsigned int";
	//case 8: return "float";
	//case 9: return "[bdc]";
	//case 10: return "bool";
	//case 13: return "long";
	//case 14: return "unsigned long";
	//case 25: return "[currency]";
	//case 26: return "[date]";
	//case 27: return "[variant]";
	//case 28: return "[complex]";
	//case 29: return "[bit]";
	//case 30: return "[BSTR]";
	//case 31: return "[HRESULT]";
	//};

	WCHAR* name = nullptr;
	if (!dbghelp->SymGetTypeInfo(m_process, m_base, typeId, TI_GET_SYMNAME, &name))
	{
		return AText() << "[invalid " << typeId << ']';
	}
	_assert(name != nullptr);
	AText out = (Utf16ToUtf8)(Text16)unwide(name);
	LocalFree(name);

	return out;
}
uint32_t PdbReader::setOptions(uint32_t options) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->SymSetOptions(options);
}
uint32_t PdbReader::getOptions() noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->SymGetOptions();
}
bool PdbReader::search(pcstr filter, SearchCallback callback) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->SymEnumSymbols(
		m_process,
		m_base,
		filter,
		[](SYMBOL_INFO* symInfo, ULONG SymbolSize, void* callback)->BOOL {
			return (*(SearchCallback*)callback)(Text(symInfo->Name, symInfo->NameLen), (autoptr64)symInfo->Address, symInfo->TypeIndex);
		},
		(PVOID)&callback);
}
bool PdbReader::getAllEx(GetAllExCallback callback) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();

	return dbghelp->SymEnumSymbols(
		m_process,
		m_base,
		nullptr,
		[](SYMBOL_INFO* symInfo, ULONG SymbolSize, void* callback)->BOOL {
			GetAllExCallback* cb = (GetAllExCallback*)callback;
			Text name = Text(symInfo->Name, symInfo->NameLen);
			return (*cb)(name, symInfo);
		},
		(PVOID)&callback);
}
bool PdbReader::getAll(SearchCallback callback) noexcept
{
	return search(nullptr, move(callback));
}
template <>
size_t PdbReader::undecorate<char>::$copyTo(char* out) const noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->UnDecorateSymbolName(m_decorated, out, maximum, m_flags);
}
template <>
size_t PdbReader::undecorate<char16_t>::$copyTo(char16_t* out) const noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->UnDecorateSymbolNameW(wide(m_decorated), wide(out), maximum, m_flags);
}
bool PdbReader::search16(pcstr16 filter, SearchCallback16 callback) noexcept
{
	const char* filterAptr;
	AText filterA;
	if (filter != nullptr) {
		filterA << (Utf16ToAnsi)(Text16)filter << nullterm;
		filterAptr = filterA.data();
	}
	else {
		filterAptr = nullptr;
	}

	DbgHelp* dbghelp = DbgHelp::getInstance();
	return dbghelp->SymEnumSymbols(
		m_process,
		m_base,
		filterAptr,
		[](SYMBOL_INFO* symInfo, ULONG SymbolSize, void* callback)->BOOL {
			AText16 name16 = (AnsiToUtf16)Text(symInfo->Name, symInfo->NameLen);
			return (*(SearchCallback16*)callback)(name16, (autoptr64)symInfo->Address, symInfo->TypeIndex);
		},
		(PVOID)&callback);
}
bool PdbReader::getAllEx16(GetAllExCallback16 callback) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();

	return dbghelp->SymEnumSymbols(
		m_process,
		m_base,
		nullptr,
		[](SYMBOL_INFO* symInfo, ULONG SymbolSize, void* callback)->BOOL {
			AText16 name16 = (AnsiToUtf16)Text(symInfo->Name, symInfo->NameLen);
			GetAllExCallback16* cb = (GetAllExCallback16*)callback;
			return (*cb)(name16, symInfo);
		},
		(PVOID)&callback);
}
bool PdbReader::getAll16(SearchCallback16 callback) noexcept
{
	return search16(nullptr, move(callback));
}
autoptr64 PdbReader::getFunctionAddress(const char* name) noexcept
{
	DbgHelp* dbghelp = DbgHelp::getInstance();

	byte buffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME];
	IMAGEHLP_SYMBOL64& sym = *(IMAGEHLP_SYMBOL64*)buffer;
	sym.SizeOfStruct = sizeof(sym);
	sym.MaxNameLength = MAX_SYM_NAME;
	if (!dbghelp->SymGetSymFromName64(m_process, name, &sym))
	{
		return nullptr;
	}
	return (autoptr64)sym.Address;
}

