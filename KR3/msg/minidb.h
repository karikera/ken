#pragma once

#include <KR3/main.h>
#include <KR3/data/map.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/msg/promise.h>

namespace kr
{
	class MiniDB
	{
	public:
		class Table;

		class Item:public Bufferable<Item, BufferInfo<void, false, false, false, false> >
		{
			friend Table;
		public:
			Text getName() const noexcept;
			size_t $size() const noexcept;
			byte * $begin() noexcept;
			byte * $end() noexcept;
			const byte * $begin() const noexcept;
			const byte * $end() const noexcept;
			
		private:
			static Item* _allocate(Text name, size_t size) noexcept;

			Item() = delete;
			Promise<Item*> * m_loading;

			Text m_name;
			size_t m_size;
		};
		class Table
		{
		public:
			Table(AText16 path, size_t size) noexcept;
			Promise<Item*>* get(AText key) noexcept;

		private:
			AText16 m_path;
			size_t m_tableSize;
			Map<Text, Item*, true> m_map;
			CriticalSection m_cs;
		};

		// path: It references text memory directly, need to keep memory
		MiniDB(Text16 path) noexcept;
		~MiniDB() noexcept;

		Table* table(Text key, size_t tableSize) noexcept;

	private:
		atomic<uintptr_t> m_references;
		Text16 m_path;
		Map<Text, Table*, true> m_tables;
	};
}

