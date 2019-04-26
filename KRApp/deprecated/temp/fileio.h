#pragma once

#include <stdio.h>

void myfread(void * dest, size_t size, FILE * file);

class File
{
public:
	File(const char * filename) noexcept;
	~File() noexcept;
	void read(void * dest,size_t size) noexcept;
	size_t getString(char * dest, size_t limit) noexcept;
	
	template <typename T> void read(T * dest) noexcept
	{
		read(dest ,sizeof(T));
	}
	void skip(size_t size) noexcept;

private:
	FILE * m_file;
	size_t m_needsize;
	size_t m_readsize;
	size_t m_filesize;
};