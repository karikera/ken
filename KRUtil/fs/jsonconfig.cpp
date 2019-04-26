#include "stdafx.h"

#ifndef NO_USE_FILESYSTEM

#include "jsonconfig.h"

using namespace kr;

using kr::JsonParser;
using kr::io::FOStream;


JsonConfig::ValLink::ValLink(Text _name) noexcept
	:name(_name)
{
}
JsonConfig::ValLink::~ValLink() noexcept
{
}

kr::JsonConfig::JsonConfig() noexcept
{
}

kr::JsonConfig::~JsonConfig() noexcept
{
	for (ValLink * link : m_linkList)
	{
		delete link;
	}
}

void JsonConfig::save(pcstr16 filename) throws(Error)
{
	FOStream<char> fos(File::create(filename));


	fos << "{";
	if (!m_linkList.empty())
	{
		fos << "\r\n\t";
		m_linkList.front()->save(&fos);
		for (ValLink * link : m_linkList + 1)
		{
			fos << ",\r\n\t";
			link->save(&fos);
		}
	}
	fos << "\r\n}";
	fos.flush();
}
void JsonConfig::load(pcstr16 filename) throws(EofException, InvalidSourceException, Error)
{
	Must<File> file = File::open(filename);
	JsonParser parser((File*)file);
	parser.object([&](kr::Text name) {
		auto iter = m_nameMap.find(name);
		if (m_nameMap.end() == iter)
		{
			parser.skipValue();
		}
		else
		{
			iter->second->load(&parser);
		}
	});
}

#endif