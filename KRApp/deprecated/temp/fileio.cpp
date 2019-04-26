#include "fileio.h"
#include <iostream>

using std::cerr;
using std::endl;

namespace
{
	size_t notenoughsize;
}

File::File(const char * filename) noexcept
{
	m_file = fopen(filename, "rb");
	if (m_file == nullptr)
	{
		cerr << "file not founded: " << filename << endl;
	}
	m_readsize = 0;
	m_needsize = 0;

	fseek(m_file, 0, SEEK_END);
	m_filesize = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
}
File::~File() noexcept
{
	fclose(m_file);
	
	if (m_readsize != m_needsize)
	{
		cerr << "Not enough file size (need = " << m_needsize << ", read = " << m_readsize << ")" << endl;
	}
}
void File::read(void * dest, size_t size) noexcept
{
	size_t readed = fread(dest, 1, size, m_file);
	m_readsize += readed;
	m_needsize += size;
}
size_t File::getString(char * dest, size_t limit) noexcept
{
	char * beg = dest;
	char * end = beg + limit;
	do
	{
		if (dest == end)
		{
			cerr << "string too long, cutted" << endl;

			char chr;
			do
			{
				read(&chr);
				dest++;
			}
			while (chr != '\0');
			break;
		}
		read(dest);
	}
	while (*dest++ != '\0');
	return dest - beg;
}
void File::skip(size_t size) noexcept
{
	size_t left = m_filesize - m_needsize;
	size_t skip;
	if (size > left) skip = left;
	else skip = size;
	m_needsize += size;
	m_readsize += skip;
	if (fseek(m_file, skip, SEEK_CUR) != 0)
	{
		cerr << "seek failed" << endl;
	}
}
