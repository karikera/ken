#include "stdafx.h"

#ifdef WIN32
#include "asyncfile.h"

#include <KR3/fs/file.h>

using namespace kr;

AsyncFile::AsyncFile() noexcept
	:m_file(nullptr)
{
	AddRef();
}
AsyncFile::AsyncFile(AText16 filename) noexcept
{
	create(move(filename));
}
AsyncFile::~AsyncFile() noexcept
{
}
void AsyncFile::$write(const void * data, size_t size) noexcept
{
	AddRef();
	m_cascader([this, data = ABuffer(data, size)] {
		m_file->$write(data.data(), data.size());
		Release();
	});
}
void AsyncFile::create(AText16 filename) noexcept
{
	filename.c_str();

	AddRef();
	m_cascader([this, filename = move(filename)] {
		delete m_file;
		m_file = File::create(filename.data());
		Release();
	});
}
void AsyncFile::close() noexcept
{
	AddRef();
	m_cascader([this]{
		delete m_file;
		m_file = nullptr;
		Release();
	});
}

#else

EMPTY_SOURCE

#endif