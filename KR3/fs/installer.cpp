#include "stdafx.h"
#include "installer.h"

#ifndef NO_USE_FILESYSTEM

#include "file.h"
#include <KR3/util/path.h>

using namespace kr;


Wildcard::Wildcard() noexcept
{
	m_firstMatch = u"";
	m_lastMatch = u"";
	m_fullMatch = false;
}
Wildcard::Wildcard(Text16 wildcard) noexcept
{
	if (wildcard.startsWith(u'*'))
	{
		m_firstMatch = u"";
		do
		{
			wildcard++;
		} while (wildcard.startsWith(u'*'));
	}
	else
	{
		m_firstMatch = wildcard.readwith_e(u'*');
		if (wildcard.empty())
		{
			m_fullMatch = true;
			return;
		}
	}
	m_fullMatch = false;
	m_matches = wildcard.splitView(u'*');
	if (!m_matches.empty())
	{
		m_lastMatch = m_matches.popGet();
	}
	else
	{
		m_lastMatch = u"";
	}
}
Wildcard::~Wildcard() noexcept
{
}
bool Wildcard::test(Text16 text) noexcept
{
	if (!text.startsWith(m_firstMatch)) return false;
	text += m_firstMatch.size();
	if (m_fullMatch) return text.empty();

	for (Text16 match : m_matches)
	{
		text = text.subarr(text.find(match));
		if (text == nullptr) return false;
		text += match.size();
	}

	return text.endsWith(m_lastMatch);
}

Installer::Installer(Text16 dest, Text16 src, Text16 wildcard) noexcept
	:m_dest(dest), m_src(src), m_wildcard(wildcard)
{
	_init();
}
Installer::Installer(Text16 dest, Text16 src) noexcept
	:m_dest(dest), m_src(src)
{
	_init();
}
void Installer::_init() noexcept
{
	File::createFullDirectory(m_dest);

	if (!path16.endsWithSeperator(m_dest)) m_dest << path16.sep;
	if (!path16.endsWithSeperator(m_src)) m_src << path16.sep;
	m_srcend = m_src.size();
	m_destend = m_dest.size();
	m_copyCount = 0;
	m_skipCount = 0;
	m_errorCount = 0;
}
Installer::Result Installer::copy(pcstr16 dest, pcstr16 src, Text16 msg) noexcept
{
	time_t stime = File::getLastModifiedTime(src);
	try
	{
		time_t dtime = File::getLastModifiedTime(dest);
		if (stime <= dtime)
		{
			return Result::Skipped;
		}
	}
	catch (Error&)
	{
	}
	if (msg != nullptr)
	{
		ucout << msg << endl;
	}
	if (!File::copy(dest, src))
	{
		return Result::Failed;
	}
	else
	{
		return Result::Copied;
	}
}

pcstr16 Installer::getSrcSz(Text16 name) noexcept
{
	m_src.resize(m_srcend);
	m_src << name << nullterm;
	return m_src.data();
}
Text16 Installer::getSrc(Text16 name) noexcept
{
	m_src.resize(m_srcend);
	m_src << name << nullterm;
	return m_src;
}
pcstr16 Installer::getDestSz(Text16 name) noexcept
{
	m_dest.resize(m_destend);
	m_dest << name << nullterm;
	return m_dest.data();
}
Text16 Installer::getDest(Text16 name) noexcept
{
	m_dest.resize(m_destend);
	m_dest << name << nullterm;
	return m_dest;
}
void Installer::all() noexcept
{
	m_dest.resize(m_destend);
	File::createFullDirectory(m_dest);
	filter(u"*");
}
void Installer::filter(Text16 filterText) noexcept
{
	FindFile find(getSrcSz(filterText));
	for (pcstr16 filename : find)
	{
		if (find.isDirectory())
		{
			dir((Text16)filename);
		}
		else
		{
			Text16 filenametx = (Text16)filename;
			if (m_wildcard.test(filenametx))
			{
				file(filenametx);
			}
		}
	}
}
void Installer::dir(Text16 dirname) noexcept
{
	size_t oldsrc = m_srcend;
	size_t olddest = m_destend;
	m_src.resize(oldsrc);
	m_dest.resize(olddest);
	m_src << dirname << path16.sep;
	m_dest << dirname << nullterm;
	File::createDirectory(m_dest.data());
	m_dest << path16.sep;
	m_srcend = m_src.size();
	m_destend = m_dest.size();
	
	all();

	m_srcend = oldsrc;
	m_destend = olddest;
}
void Installer::file(Text16 filename) noexcept
{
	copy(filename, filename, filename);
}
void Installer::copy(Text16 dest, Text16 src, Text16 msg) noexcept
{
	pcstr16 srcsz = getSrcSz(src);
	pcstr16 destsz = getDestSz(dest);

	Result res = Installer::copy(destsz, srcsz, msg);
	switch (res)
	{
	case Result::Copied:
		m_copyCount++;
		break;
	case Result::Failed:
		ucerr << u"[ERROR]" << ErrorCode::getLast().getMessage<char16>() << endl;
		m_errorCount++;
		break;
	case Result::Skipped:
		m_skipCount++;
		break;
	}
}

#endif