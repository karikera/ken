#pragma once

#include <iostream>
#include <stdio.h>

class Buffer;
class File;

class Buffer
{
public:
	Buffer() noexcept;
	~Buffer() noexcept;

	size_t size() const noexcept;
	size_t copyTo(void * dest, size_t size) const noexcept;
	void clear() noexcept;
	void append(const void * data, size_t size) noexcept;
	void append(FILE * data, size_t size) noexcept;
	void append(File * data, size_t size) noexcept;

private:
	template <typename T> void _appendTemplate(T* stream, size_t size) noexcept;

	struct BufferData
	{
		BufferData * next;
		size_t size;
		char buffer[1];
	};

	BufferData * m_front;
	BufferData * m_rear;
	size_t m_writed;
	size_t m_size;
};

template <typename T> class BufferT:public Buffer
{
public:
	void append(const T& data) noexcept
	{
		Buffer::append(&data, sizeof(T));
	}
	void append(const T* data, size_t count) noexcept
	{
		Buffer::append(data,count * sizeof(T));
	}
	void append(FILE* data, size_t size) noexcept
	{
		Buffer::append(data, size * sizeof(T));
	}
	
};