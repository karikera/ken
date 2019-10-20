#include "stdafx.h"
#include "bufferqueue.h"

#include <KR3/util/keeper.h>
#include <KR3/mt/criticalsection.h>

using namespace kr;

namespace
{
	AtomicKeeper<BufferBlock, 128> buffers;
}

Buffer BufferBlock::getBuffer() noexcept
{
	return Buffer(buffer, size);
}
WView<byte> BufferBlock::getRemaining() noexcept
{
	return WView<byte>(buffer + size, buffer + BUFFER_BLOCK_SIZE);
}
BufferBlock * BufferBlock::alloc() noexcept
{
	return (BufferBlock*)buffers.rawAlloc();
}
void BufferBlock::free() noexcept
{
	return buffers.rawFree(this);
}
Buffer BufferBlockWithRef::getBuffer() noexcept
{
	if (isReference())
	{
		return Buffer(bufferPtr, size);
	}
	else
	{
		return Buffer(buffer, size);
	}
}
BufferBlockWithRef * BufferBlockWithRef::alloc() noexcept
{
	return (BufferBlockWithRef*)buffers.rawAlloc();
}
void BufferBlockWithRef::free() noexcept
{
	return buffers.rawFree(this);
}

void * BufferBlock::begin() noexcept
{
	return buffer;
}
size_t BufferBlock::capacity() const noexcept
{
	return kr_msize(this) - offsetof(BufferBlock, buffer);
}
bool BufferBlockWithRef::isReference() const noexcept
{
	return (intptr_t)size < 0;
}
const byte * BufferBlockWithRef::begin() const noexcept
{
	return isReference() ? bufferPtr : buffer;
}
size_t BufferBlockWithRef::capacity() const noexcept
{
	return kr_msize(this) - offsetof(BufferBlockWithRef, buffer);
}

BufferQueue::BufferQueue() noexcept
{
	m_first = nullptr;
	m_last = _axis();
	m_readed = 0;
	m_count = 0;
	m_totalSize = 0;
}
BufferQueue::~BufferQueue() noexcept
{
	clear();
}
BufferQueue::BufferQueue(BufferQueue&& _move) noexcept
{
	m_first = _move.m_first;
	if (_move.m_last == _move._axis()) m_last = _axis();
	else
	{
		m_last = _move.m_last;
		_move.m_last = _move._axis();
	}
	m_readed = _move.m_readed;
	m_count = _move.m_count;
	m_totalSize = _move.m_totalSize;
	_move.m_first = nullptr;
	_move.m_readed = 0;
	_move.m_count = 0;
	_move.m_totalSize = 0;
}
BufferQueue& BufferQueue::operator = (BufferQueue&& _move) noexcept
{
	this->~BufferQueue();
	new(this) BufferQueue(move(_move));
	return *this;
}
WView<void> BufferQueue::prepare() noexcept
{
	if (m_last != _axis())
	{
		BufferBlock* buff = m_last;
		size_t left = buff->capacity() - buff->size;
		if (left != 0)
		{
			return { buff->buffer + buff->size, left };
		}
	}
	BufferBlock * buff = BufferBlock::alloc();
	m_last->next = buff;
	m_count++;
	
	size_t cap = buff->capacity();
	buff->size = 0;
	buff->next = nullptr;
	m_last = buff;

	return { buff->buffer, cap };
}
void BufferQueue::commit(size_t size) noexcept
{
	_assert(size <= m_last->capacity() - m_last->size);
	m_totalSize += size;
	m_last->size += size;
}
void BufferQueue::write(const void * data, size_t size) noexcept
{
	m_totalSize += size;

	if (m_last != _axis())
	{
		BufferBlock* buff = m_last;
		size_t left = buff->capacity() - buff->size;
		if ( size <= left)
		{
			memcpy(buff->buffer + buff->size, data, size);
			buff->size += size;
			return;
		}
		else
		{
			memcpy(buff->buffer + buff->size, data, left);
			(const byte*&)data += left;
			buff->size += left;
			size -= left;
		}
	}
	BufferBlock * buff = BufferBlock::alloc();
	m_last->next = buff;
	m_count++;

_refill:

	size_t cap = buff->capacity();
	if (size > cap)
	{
		memcpy(buff->buffer,data, cap);
		buff->size = cap;

		buff = buff->next = BufferBlock::alloc();
		(const byte*&)data += cap;
		size -= cap;
		m_count++;
		goto _refill;
	}
	else
	{
		memcpy(buff->buffer, data, size);
		buff->size = size;
		buff->next = nullptr;
	}
	m_last = buff;
}
void BufferQueue::write(Buffer data) noexcept
{
	write(data.data(), data.size());
}
void BufferQueue::peek(void * dest, size_t size) noexcept
{
	_assert(size <= m_totalSize);
	size_t offset = m_readed;
	BufferBlock * block = m_first;
	for (;;)
	{
		byte* srcbuffer = (byte*)block->buffer + offset;
		size_t srcsize = block->size - offset;
		if (srcsize > size)
		{
			memcpy(dest, srcbuffer, size);
			return;
		}
		else
		{
			memcpy(dest, srcbuffer, srcsize);
			(byte*&)dest += srcsize;
			size -= srcsize;
			offset = 0;
			block = block->next;
			if (block == nullptr) return;
		}
	}
}
void BufferQueue::read(void * dest, size_t size) noexcept
{
	_assert(size <= m_totalSize);
	if (size == 0) return;
	m_totalSize -= size;

	for (;;)
	{
		byte* srcbuffer = (byte*)m_first->buffer + m_readed;
		size_t srcsize = m_first->size - m_readed;
		if (srcsize > size)
		{
			memcpy(dest, srcbuffer, size);
			m_readed += size;
			return;
		}
		else
		{
			memcpy(dest, srcbuffer, srcsize);
			(byte*&)dest += srcsize;
			size -= srcsize;
			m_readed = 0;
			BufferBlock * next = m_first->next;
			m_first->free();
			m_first = next;
			if (next == nullptr)
			{
				_assert(size == 0);
				_assert(m_count == 1);
				_assert(m_totalSize == 0);
				m_last = _axis();
				m_count = 0;
				return;
			}
			m_count--;
		}
	}
}
Buffer BufferQueue::read(size_t size, TBuffer * temp) noexcept
{
	_assert(size <= m_totalSize);
	if (size == 0) return Buffer(nullptr, nullptr);

	m_totalSize -= size;


	size_t srcsize = m_first->size - m_readed;
	if (srcsize == 0)
	{
		BufferBlock * next = m_first->next;
		m_first->free();
		m_first = next;
		srcsize = next->size;
		m_readed = 0;
		m_count--;
	}

	byte* srcbuffer = (byte*)m_first->buffer + m_readed;
	if (srcsize >= size)
	{
		m_readed += size;
		return { srcbuffer, size };
	}
	else
	{
		temp->write(srcbuffer, srcsize);
		size -= srcsize;
		BufferBlock * next = m_first->next;
		m_first->free();
		m_first = next;
		m_count--;
	}

	for (;;)
	{
		byte* srcbuffer = (byte*)m_first->buffer;
		size_t srcsize = m_first->size;
		if (srcsize > size)
		{
			temp->write(srcbuffer, size);
			m_readed = size;
			return *temp;
		}
		else
		{
			temp->write(srcbuffer, srcsize);
			size -= srcsize;
			BufferBlock * next = m_first->next;
			m_first->free();
			m_first = next;
			if (next == nullptr)
			{
				_assert(size == 0);
				_assert(m_count == 1);
				_assert(m_totalSize == 0);
				m_last = _axis();
				m_count = 0;
				m_readed = 0;
				return *temp;
			}
			m_count--;
		}
	}
}
Text BufferQueue::readwith(char chr, TText * temp) noexcept
{
	temp->clear();
	auto iter = begin();
	auto iter_end = end();
	if (iter == iter_end) return nullptr;

	{
		Text text = (*iter).cast<char>();
		Text readed = text.readwith(chr);
		if (readed != nullptr)
		{
			size_t size = readed.size() + 1;
			m_readed += size;
			m_totalSize -= size;
			return readed;
		}
		*temp << text;
		iter++;
	}

	for (;iter != iter_end;iter++)
	{
		Text text = (*iter).cast<char>();
		Text readed = text.readwith(chr);
		if (readed != nullptr)
		{
			*temp << readed;
			skip(temp->size() + 1);
			return *temp;
		}
		*temp << text;
	}
	return nullptr;
}
Text BufferQueue::readwith(Text chr, TText * temp) noexcept
{
	temp->clear();
	auto iter = begin();
	auto iter_end = end();

	if (iter == iter_end) return false;

	{
		Text text = (*iter).cast<char>();
		Text finded = text.readto(chr);
		if (finded != nullptr)
		{
			m_readed += temp->size() + chr.size();
			return finded;
		}
		*temp << text;
		iter++;
	}
	size_t offset = temp->size() - chr.size() + 1;

	for (; iter != iter_end; iter++)
	{
		Text text = (*iter).cast<char>();
		*temp << text;
		
		const char * finded = (*temp + offset).find(chr).begin();
		if (finded != nullptr)
		{
			temp->cut(finded);
			skip(temp->size() + chr.size());
			return *temp;
		}
	}
	return nullptr;
}
AText BufferQueue::readAll() noexcept
{
	AText data;
	for (Buffer buf : *this)
	{
		data << buf.cast<char>();
	}
	return data;
}
bool BufferQueue::empty() noexcept
{
	return m_first == nullptr;
}
void BufferQueue::clear() noexcept
{
	BufferBlock * set = m_first;
	while (set != nullptr)
	{
		BufferBlock * next = set->next;
		set->free();
		set = next;
	}
	m_first = nullptr;
	m_last = _axis();
	m_readed = 0;
	m_count = 0;
	m_totalSize = 0;
}
Buffer BufferQueue::getFirstBlock() noexcept
{
	return m_first->getBuffer() + m_readed;
}
void BufferQueue::skip(size_t size) noexcept
{
	BufferBlock * buff = m_first;
	if (buff == nullptr) return;
	size_t skipto = size + m_readed;
	do
	{
		if (buff->size > skipto)
		{
			m_readed = skipto;
			m_totalSize -= size;
			return;
		}
		skipto -= buff->size;
		BufferBlock * next = buff->next;
		buff->free();
		buff = next;
		m_first = buff;
		m_count--;
	}
	while (buff != nullptr);
	m_last = _axis();
	m_readed = 0;
	m_count = 0;
	m_totalSize = 0;
}
size_t BufferQueue::size() noexcept
{
	return m_totalSize;
}
BufferBlock * BufferQueue::_makeBuffer() noexcept
{
	BufferBlock * buff = BufferBlock::alloc();
	buff->next = nullptr;
	m_last->next = buff;
	m_last = buff;
	m_count++;
	return buff;
}
void BufferQueue::_removeFrontNode() noexcept
{
	m_readed = 0;
	BufferBlock * p = m_first;
	m_first = p->next;
	if (m_first == nullptr)
	{
		m_last = _axis();
		_assert(m_totalSize == 0);
	}
	p->free();
	m_count--;
}
BufferBlock * BufferQueue::_axis() noexcept
{
	return (BufferBlock*)&m_first;
}
void BufferQueue::clearKeeped() noexcept
{
	buffers.clear();
}

BufferQueue::Iterator BufferQueue::begin() noexcept
{
	return Iterator(m_first, m_readed);
}
BufferQueue::Iterator BufferQueue::end() noexcept
{
	return Iterator(nullptr, 0);
}
size_t BufferQueue::bufferCount() noexcept
{
	return m_count;
}
void BufferQueue::checkBufferCorrution() noexcept
{
	BufferBlock * block = m_first;
	if (block == nullptr)
	{
		_assert(m_last == nullptr);
		_assert(m_count == 0);
		_assert(m_totalSize == 0);
		_assert(m_readed == 0);
		return;
	}

	_assert(m_readed <= block->size);
	size_t total = block->size - m_readed;

	for (;;)
	{
		BufferBlock * next = block->next;
		if (next == nullptr)
		{
			_assert(block == m_last);
			_assert(total == m_totalSize);
			return;
		}
		total += next->size;
		block = next;
	}
}

BufferQueuePointer::BufferQueuePointer(const BufferQueue & buf) noexcept
{
	m_buffer = buf.m_first;
	m_readed = buf.m_readed;
	m_fullreaded = 0-m_readed;
}
byte BufferQueuePointer::read() throws(EofException)
{
	if (m_buffer == nullptr) throw EofException();
	byte* srcbuffer = (byte*)m_buffer->buffer + m_readed;
	if (m_buffer->size > m_readed+1)
	{
		m_readed++;
		return *srcbuffer;
	}
	else
	{
		m_fullreaded += m_readed + 1;
		m_readed = 0;
		m_buffer = m_buffer->next;
		return *srcbuffer;
	}
}
void BufferQueuePointer::read(void * dest, size_t size) throws(EofException)
{
	for (;;)
	{
		if (size == 0) return;
		if (m_buffer == nullptr) throw EofException();
		byte* srcbuffer = (byte*)m_buffer->buffer + m_readed;
		size_t srcsize = m_buffer->size - m_readed;
		if (srcsize > size)
		{
			memcpy(dest, srcbuffer, size);
			m_readed += size;
			return;
		}
		else
		{
			memcpy(dest, srcbuffer, srcsize);
			(byte*&)dest += srcsize;
			size -= srcsize;
			m_fullreaded += m_readed + srcsize;
			m_readed = 0;
			m_buffer = m_buffer->next;
		}
	}
}
void BufferQueuePointer::skip(size_t size) throws(EofException)
{
	size += m_readed;
	for (;;)
	{
		if (size == 0)
		{
			m_readed = 0;
			return;
		}
		if (m_buffer == nullptr)
		{
			m_readed = 0;
			throw EofException();
		}
		if (m_buffer->size > size)
		{
			m_readed = size;
			return;
		}
		m_fullreaded += m_buffer->size;
		size -= m_buffer->size;
		m_buffer = m_buffer->next;
	}
}
size_t BufferQueuePointer::getReadSize() noexcept
{
	return m_fullreaded + m_readed;
}
void BufferQueuePointer::clearSize() noexcept
{
	m_fullreaded = 0-m_readed;
}

Buffer BufferQueueWithRef::getFirstBlock() noexcept
{
	return ((BufferBlockWithRef*)m_first)->getBuffer() + m_readed;
}
void BufferQueueWithRef::write(const void * data, size_t size) noexcept
{
	m_totalSize += size;

	if (m_last != _axis())
	{
		BufferBlockWithRef* buff = (BufferBlockWithRef*)m_last;
		size_t buff_size = buff->size & BufferBlockWithRef::SIZE_MASK;
		size_t left = buff->capacity() - buff_size;
		if (size <= left)
		{
			memcpy(buff->buffer + buff_size, data, size);
			buff->size += size;
			return;
		}
		else
		{
			memcpy(buff->buffer + buff_size, data, left);
			(const byte*&)data += left;
			buff->size += left;
			size -= left;
		}
	}
	m_count++;
	BufferBlockWithRef * buff = BufferBlockWithRef::alloc();
	m_last->next = (BufferBlock*)buff;

	for (;;)
	{
		size_t cap = buff->capacity();
		if (size > cap)
		{
			memcpy(buff->buffer, data, cap);
			buff->size = cap;

			m_count++;
			buff = buff->next = BufferBlockWithRef::alloc();
			(const byte*&)data += cap;
			size -= cap;
		}
		else
		{
			memcpy(buff->buffer, data, size);
			buff->size = size;
			buff->next = nullptr;
			break;
		}
	}
	m_last = (BufferBlock*)buff;
}
void BufferQueueWithRef::writeRef(const void * data, size_t size) noexcept
{
	BufferBlockWithRef * buff = BufferBlockWithRef::alloc();
	buff->size = size | BufferBlockWithRef::REFERENCE_BIT;
	buff->bufferPtr = (byte*)data;
	buff->next = nullptr;

	m_last->next = (BufferBlock*)buff;
	m_last = (BufferBlock*)buff;
	m_totalSize += size;
	m_count++;
}
void BufferQueueWithRef::read(void * dest, size_t size) noexcept
{
	if (size == 0) return;
	m_totalSize -= size;

	for (;;)
	{
		BufferBlockWithRef* buff = (BufferBlockWithRef*)m_first;

		const byte* srcbuffer = buff->begin() + m_readed;
		size_t srcsize = (buff->size & BufferBlockWithRef::SIZE_MASK) - m_readed;
		if (srcsize > size)
		{
			memcpy(dest, srcbuffer, size);
			m_readed += size;
			return;
		}
		else
		{
			memcpy(dest, srcbuffer, srcsize);
			(byte*&)dest += srcsize;
			size -= srcsize;
			m_readed = 0;
			BufferBlockWithRef * next = (BufferBlockWithRef*)buff->next;
			buff->free();
			buff = next;
			m_first = (BufferBlock*)next;
			if (next == nullptr)
			{
				_assert(size == 0);
				_assert(m_count == 1);
				m_count = 0;
				m_last = _axis();
				_assert(m_totalSize == 0);
				return;
			}
			m_count--;
		}
	}
}
BufferQueueWithRef::Iterator BufferQueueWithRef::begin() noexcept
{
	return Iterator((BufferBlockWithRef*)m_first, m_readed);
}
BufferQueueWithRef::Iterator BufferQueueWithRef::end() noexcept
{
	return Iterator(nullptr, 0);
}
BufferQueue::Iterator::Iterator(BufferBlock * buffer, size_t offset) noexcept
	:m_buffer(buffer), m_offset(offset)
{
}
kr::Buffer BufferQueue::Iterator::operator *() const noexcept
{
	return kr::Buffer(m_buffer->buffer + m_offset, m_buffer->size - m_offset);
}
BufferQueue::Iterator & BufferQueue::Iterator::operator ++() noexcept
{
	m_buffer = m_buffer->next;
	m_offset = 0;
	return *this;
}
const BufferQueue::Iterator BufferQueue::Iterator::operator ++(int) noexcept
{
	Iterator old = *this;
	++*this;
	return old;
}
bool BufferQueue::Iterator::operator ==(const Iterator & other) const noexcept
{
	return m_buffer == other.m_buffer && m_offset == other.m_offset;
}
bool BufferQueue::Iterator::operator !=(const Iterator & other) const noexcept
{
	return m_buffer != other.m_buffer || m_offset != other.m_offset;
}

BufferQueueWithRef::Iterator::Iterator(BufferBlockWithRef * buffer, size_t offset) noexcept
	:m_buffer(buffer), m_offset(offset)
{
}
Buffer BufferQueueWithRef::Iterator::operator *() noexcept
{
	return m_buffer->getBuffer() + m_offset;
}
BufferQueueWithRef::Iterator & BufferQueueWithRef::Iterator::operator ++() noexcept
{
	m_buffer = m_buffer->next;
	m_offset = 0;
	return *this;
}
const BufferQueueWithRef::Iterator BufferQueueWithRef::Iterator::operator ++(int) noexcept
{
	Iterator old = *this;
	++*this;
	return old;
}
bool BufferQueueWithRef::Iterator::operator ==(const Iterator & other) noexcept
{
	return m_buffer == other.m_buffer;
}
bool BufferQueueWithRef::Iterator::operator !=(const Iterator & other) noexcept
{
	return m_buffer != other.m_buffer;
}
