#include "buffer.h"
#include "fileio.h"

constexpr size_t MINIMUM_SIZE = 4096;

class FileStream
{
private:
	FILE * m_src;

public:
	FileStream(FILE * src) noexcept
	{
		m_src = src;
	}
	void read(void * dest, size_t size) noexcept
	{
		size_t readed = fread(dest, 1, size, m_src);
		if (readed != size)
		{
			std::cerr << "file not enough size" << std::endl;
		}
	}
};

class MemStream
{
private:
	const char * m_src;

public:
	MemStream(const void * src) noexcept
	{
		m_src = (char*)src;
	}
	void read(void * dest, size_t size) noexcept
	{
		memcpy(dest, m_src, size);
		m_src += size;
	}
};

Buffer::Buffer() noexcept
{
	m_front = m_rear = (BufferData*)new char[offsetof(BufferData,buffer) + MINIMUM_SIZE];
	m_rear->size = MINIMUM_SIZE;
	m_writed = 0;
}
Buffer::~Buffer() noexcept
{
	clear();
}

template <typename T>
inline void Buffer::_appendTemplate(T* stream, size_t size) noexcept
{
	m_size += size;

	if (m_rear->size != m_writed)
	{
		size_t left = m_rear->size - m_writed;
		if (size <= left)
		{
			stream->read(m_rear->buffer + m_writed, size);
			m_writed += size;
			return;
		}
		stream->read(m_rear->buffer + m_writed, left);
		size -= left;
	}

	size_t bufsize = std::max(size, MINIMUM_SIZE);
	BufferData * nbuf = (BufferData*)new char[offsetof(BufferData,buffer) + bufsize];
	m_rear->next = nbuf;
	m_rear = nbuf;
	nbuf->size = bufsize;
	stream->read(nbuf->buffer, size);
	m_writed = size;
}

size_t Buffer::size() const noexcept
{
	return m_size;
}
size_t Buffer::copyTo(void * dest, size_t size) const noexcept
{
	char * ptr = (char*)dest;
	BufferData * data = m_front;
	for (;;)
	{
		size_t dsize = data->size;
		if (size <= dsize)
		{
			memcpy(ptr, data->buffer, size);
			ptr += size;
			break;
		}
		else
		{
			memcpy(ptr, data->buffer, dsize);
			ptr += dsize;
			size -= dsize;
			if (data == m_rear) break;
			data = data->next;
		}
	}
	return ptr - (char*)dest;
}
void Buffer::clear() noexcept
{
	BufferData* f = m_front;
	while (f != m_rear)
	{
		BufferData* d = f;
		f = f->next;
		delete d;
	}
	m_front = m_rear;
	m_writed = 0;
}
void Buffer::append(const void * data, size_t size) noexcept
{
	MemStream ms = data;
	_appendTemplate(&ms, size);
}
void Buffer::append(FILE * data, size_t size) noexcept
{
	FileStream fs = data;
	_appendTemplate(&fs, size);
}
void Buffer::append(File * data, size_t size) noexcept
{
	_appendTemplate(data, size);
}
