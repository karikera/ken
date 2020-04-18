#include "stdafx.h"
#include "minidb.h"
#include "pool.h"

#ifndef NO_USE_FILESYSTEM

#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <KR3/msg/pump.h>

using namespace kr;

MiniDB::MiniDB(Text16 path) noexcept
	:m_path(path), m_references(0)
{
}
MiniDB::~MiniDB() noexcept
{
}
MiniDB::Table* MiniDB::table(Text key, size_t tableSize) noexcept
{
	AText16 path;
	path << m_path;
	if (!path16.endsWithSeperator(path)) path << path16.sep;
	path << utf8ToUtf16(key);
	
	Table * tbl = _new Table(move(path), tableSize);
	return tbl;
}

Text MiniDB::Item::getName() const noexcept
{
	return m_name;
}
size_t MiniDB::Item::$size() const noexcept
{
	return m_size;
}
byte* MiniDB::Item::$begin() noexcept
{
	return (byte*)(this + 1);
}
byte* MiniDB::Item::$end() noexcept
{
	return $begin() + m_size;
}

const byte* MiniDB::Item::$begin() const noexcept
{
	return (byte*)(this+1);
}
const byte* MiniDB::Item::$end() const noexcept
{
	return $begin() + m_size;
}
MiniDB::Item* MiniDB::Item::_allocate(Text name, size_t size) noexcept
{
	Item* item = (Item*)_new byte[sizeof(Item) + size + name.size()];
	char * nameptr = (char*)(item->begin() + size);
	name.copyTo(nameptr);
	item->m_name = Text(nameptr, name.size());
	return item;
}

MiniDB::Table::Table(AText16 path, size_t size) noexcept
	:m_path(move(path)), m_tableSize(size)
{
}
Promise<MiniDB::Item*>* MiniDB::Table::get(AText key) noexcept
{
	key << nullterm;

	DeferredPromise<Item*> * prom = _new DeferredPromise<Item*>();

	threadingVoid([this, key = move(key), prom]{
		try
		{
			Item* item;
			{
				CsLock __lock = m_cs;
				item = Item::_allocate(key, m_tableSize);
				auto res = m_map.insert(key, item);
				if (!res.second)
				{
					Item * item = res.first->second;
					if (item->m_loading)
					{
						item->m_loading->connect(prom);
					}
					else
					{
						prom->_resolve(item);
					}
					return;
				}
				item->m_loading = prom;
			}

			TSZ16 path;
			path << m_path << utf8ToUtf16(key);
			File* file = File::open(path.c_str());
			file->read(item->data(), m_tableSize);
			prom->resolve(item);
		}
		catch (...)
		{
			prom->reject();
		}
	});

	return prom;
}

#endif