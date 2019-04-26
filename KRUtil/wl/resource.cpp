#include "stdafx.h"

#ifdef WIN32
#include "resource.h"
#include <KR3/util/process.h>
#include <KR3/wl/windows.h>

int kr::ResourceFile::m_nTempNo=0;


kr::Resource<void>::Resource(HINSTANCE hModule, int id, int type) noexcept
{
	m_rsrc = nullptr;
	m_hGlobal = nullptr;

	m_rsrc = FindResourceW(hModule, MAKEINTRESOURCEW(id), MAKEINTRESOURCEW(type));
	_assert(m_rsrc != nullptr);
	m_size = SizeofResource(hModule, (HRSRC)m_rsrc);
	m_hGlobal = LoadResource(hModule, (HRSRC)m_rsrc);
	_assert(m_hGlobal != nullptr);
	m_resource = LockResource(m_hGlobal);
	_assert(m_resource != nullptr);
}
kr::Resource<void>::~Resource() noexcept
{
	if (m_resource) UnlockResource(m_resource);
	if (m_rsrc) FreeResource(m_rsrc);
}
void * kr::Resource<void>::begin() noexcept
{
	return m_resource;
}
size_t kr::Resource<void>::size() noexcept
{
	return m_size;
}

kr::ResourceFile::ResourceFile(HINSTANCE hModule, int id, int type) noexcept
	:Resource(hModule, id, type)
{
}
kr::ResourceFile::~ResourceFile() noexcept
{
	if(m_isTemp)
	{
		File::remove(m_strFileName.begin());
	}
}
bool kr::ResourceFile::toFile(pcstr16 str, bool temp) throws(Error)
{
	size_t size = m_size;

	Must<File> file = File::create(str);
	m_strFileName << Text16(str) <<nullterm;
	byte* data = (byte*)m_resource;
	file->writeImpl(data, size);
	m_isTemp=temp;
	return true;
}
int kr::ResourceFile::execute(Text16 param) noexcept try
{
	if(!m_strFileName.empty())
	{
		m_strFileName << u"temp" << m_nTempNo << u".exe" << nullterm;
		toFile(m_strFileName.begin(), true);
		m_nTempNo++;
	}
	return Process::execute(TSZ16() << m_strFileName << u' ' << param);
}
catch (NotEnoughSpaceException&)
{
	return -1;
}

#endif