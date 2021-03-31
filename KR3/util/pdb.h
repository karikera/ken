#pragma once

#include <KR3/main.h>
#include <KR3/util/callable.h>
typedef struct _SYMBOL_INFO SYMBOL_INFO;
typedef struct _SYMBOL_INFOW SYMBOL_INFOW;
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
		void* process() noexcept;
		void close() noexcept;
		SymbolInfo getInfo() throws(FunctionError);
		AText getTypeName(uint32_t typeId) noexcept;

		static uint32_t setOptions(uint32_t options) noexcept;
		static uint32_t getOptions() noexcept;
		template <typename C>
		class undecorate :public Bufferable<undecorate<C>, BufferInfo<C, method::OnlyCopyTo, true, true, HasOnlyCopyToInfo<4095> > > {
		private:
			const C* m_decorated;
			uint32_t m_flags;

		public:
			undecorate(const C* decorated, uint32_t flags) noexcept
				:m_decorated(decorated), m_flags(flags) {
			}
			size_t $copyTo(C* decorated) const noexcept;
		};

		using SearchCallback = Lambda<sizeof(void*) * 3, bool(Text name, autoptr64 address, uint32_t typeId)>;
		bool search(pcstr filter, SearchCallback callback) noexcept;
		bool getAll(SearchCallback callback) noexcept;

		using GetAllExCallback = Lambda<sizeof(void*) * 3, bool(Text name, SYMBOL_INFO* info)>;
		bool getAllEx(GetAllExCallback callback) noexcept;

		using SearchCallback16 = Lambda<sizeof(void*) * 3, bool(Text16 name, autoptr64 address, uint32_t typeId)>;
		bool search16(pcstr16 filter, SearchCallback16 callback) noexcept;
		bool getAll16(SearchCallback16 callback) noexcept;
		
		using GetAllExCallback16 = Lambda<sizeof(void*) * 3, bool(Text16 name, SYMBOL_INFOW* info)>;
		bool getAllEx16(GetAllExCallback16 callback) noexcept;

		autoptr64 getFunctionAddress(const char* name) noexcept;

	private:

		void* m_process;
		uint64_t m_base;
	};

}
