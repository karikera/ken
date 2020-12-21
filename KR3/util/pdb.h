#pragma once

#include <KR3/main.h>
#include <KR3/util/callable.h>
typedef struct _SYMBOL_INFO SYMBOL_INFO;
namespace kr
{

	class PdbReader
	{
	public:

		struct SymbolInfo
		{
			byte buffer_for_IMAGEHLP_MODULEW64[3264];

			Text16 type;
			Text16 imageName;
			Text16 loadedImageName;
			Text16 loadedPdbName;
			bool unmatched;
			bool lineNumbers;
			bool globalSymbols;
			bool typeInfo;
			bool sourceIndexed;
			bool publics;
		};

		PdbReader() throws(FunctionError);
		PdbReader(PdbReader&& _move) noexcept;
		PdbReader(uint64_t base, pcstr16 programPath) throws(FunctionError);
		~PdbReader() noexcept;
		void load() throws(FunctionError);
		void load(uint64_t base, pcstr16 programPath) throws(FunctionError);
		void* base() noexcept;
		void close() noexcept;
		SymbolInfo getInfo() throws(FunctionError);
		AText getTypeName(uint32_t typeId) noexcept;

		using SearchCallback = Lambda<sizeof(void*) * 4, bool(Text name, autoptr64 address, uint32_t typeId)>;
		bool search(const char* filter, SearchCallback callback) noexcept;

		using GetAllExCallback = Lambda<sizeof(void*) * 4, bool(Text name, SYMBOL_INFO* info)>;
		bool getAllEx(GetAllExCallback callback) noexcept;

		using GetAllCallback = Lambda<sizeof(void*) * 4, bool(Text name, autoptr64 address)>;
		bool getAll(GetAllCallback callback) noexcept;

		autoptr64 getFunctionAddress(const char* name) noexcept;

	private:

		void* m_process;
		uint64_t m_base;
	};

}
