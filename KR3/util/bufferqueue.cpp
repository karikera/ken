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

BufferQueue::ReadResult::ReadResult(BufferQueue* queue) noexcept
	:queue(queue)
{
	totalSize = 0;
}
BufferQueue::ReadResult::ReadResult(ReadResult&& _move) noexcept
	:queue(_move.queue)
{
	buffers = move(_move.buffers);
	totalSize = _move.totalSize;
	ended = _move.ended;
}
void BufferQueue::ReadResult::push(const Buffer& buffer) noexcept
{
	buffers.push(buffer);
	totalSize += buffer.size();
}
void BufferQueue::ReadResult::remove(size_t size) noexcept
{
	_assert(size <= totalSize);

	while (size != 0)
	{
		Buffer& buffer = buffers.back();
		size_t bufsize = buffer.size();
		if (bufsize <= size)
		{
			size -= bufsize;
			buffers.pop();
		}
		else
		{
			buffer.cut_self(bufsize - size);
			return;
		}
	}
}
void BufferQueue::ReadResult::readAllTo(io::VOStream<void> out) throws(ThrowRetry)
{
	for (Buffer buf : buffers)
	{
		out << buf;
	}
	queue->skip(totalSize);
	if (!ended) throw ThrowRetry();
}
void BufferQueue::ReadResult::readAllTo(ABuffer* buffer, size_t limit) throws(ThrowRetry, NotEnoughSpaceException)
{
	if (buffer->size() + totalSize > limit) throw NotEnoughSpaceException();
	readAllTo(buffer);
	if (!ended) throw ThrowRetry();
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
BufferQueue::ReadResult BufferQueue::read(size_t size) noexcept
{
	_assert(size <= m_totalSize);

	ReadResult result(this);
	result.ended = true;

	if (size == 0)
	{
		return result;
	}

	size_t offset = m_readed;

	BufferBlock* block = m_first;

	byte* srcbuffer = (byte*)block->buffer + offset;
	size_t srcsize = block->size - offset;
	if (srcsize > size)
	{
		result.push(Buffer(srcbuffer, size));
		return result;
	}
	else
	{
		result.push(Buffer(srcbuffer, srcsize));
		size -= srcsize;
		block = block->next;
		if (block == nullptr)
		{
			return result;
		}
	}

	for (;;)
	{
		byte* srcbuffer = (byte*)block->buffer;
		size_t srcsize = block->size;
		if (srcsize > size)
		{
			result.push(Buffer(srcbuffer, size));
			return result;
		}
		else
		{
			result.push(Buffer(srcbuffer, srcsize));
			size -= srcsize;
			block = block->next;
			if (block == nullptr)
			{
				result.ended = true;
				return result;
			}
		}
	}
	return result;
}
BufferQueue::ReadResult BufferQueue::readto(byte needle) noexcept
{
	ReadResult result(this);

	auto iter = begin();
	auto iter_end = end();

	if (iter == iter_end)
	{
		result.ended = false;
		return result;
	}

	Buffer text = *iter;
	{
		Buffer finded = text.readto(needle);
		if (finded != nullptr)
		{
			result.push(finded);
			result.ended = true;
			return result;
		}
		result.push(text);
		iter++;
	}
	for (; iter != iter_end; iter++)
	{
		Buffer text = *iter;
		Buffer finded = text.readto(needle);
		if (finded != nullptr)
		{
			result.push(finded);
			result.ended = true;
			return result;
		}
		result.push(text);
	}
	result.ended = false;
	return result;
}
BufferQueue::ReadResult BufferQueue::readto(HashTester<void>& needle) noexcept
{
	ReadResult result(this);

	auto iter = begin();
	auto iter_end = end();
	size_t needle_size = needle.size();

	if (iter == iter_end)
	{
		result.ended = false;
		result.totalSize = 0;
		return result;
	}

	{
		Buffer text = *iter;
		cptr finded = needle.puts(text);
		if (finded != nullptr)
		{
			size_t srcsize = (byte*)finded - (byte*)text.data() - needle_size;
			if (srcsize != 0)
			{
				result.push(text.cut(srcsize));
			}
			result.ended = true;
			return result;
		}
		result.push(text);
		iter++;
	}

	for (; iter != iter_end; iter++)
	{
		Buffer text = *iter;
		cptr finded = needle.puts(text);
		if (finded != nullptr)
		{
			intptr_t endsize = (byte*)finded - (byte*)text.data() - needle_size;
			if (endsize >= 0)
			{
				if (endsize != 0)
				{
					result.push(text.cut(endsize));
				}
			}
			else
			{
				result.remove(-endsize);
			}
			result.ended = true;
			return result;
		}
		result.push(text);
	}

	result.ended = false;
	if (result.totalSize < needle_size)
	{
		result.buffers.clear();
	}
	else
	{
		result.remove(needle_size - 1);
	}
	return result;
}
Buffer BufferQueue::read(size_t size, TBuffer* temp) noexcept
{
	_assert(size <= m_totalSize);
	if (size == 0) return Buffer(nullptr, nullptr);

	m_totalSize -= size;


	size_t srcsize = m_first->size - m_readed;
	if (srcsize == 0)
	{
		BufferBlock* next = m_first->next;
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
		BufferBlock* next = m_first->next;
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
			BufferBlock* next = m_first->next;
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
Buffer BufferQueue::readwith(byte needle, TBuffer* temp) noexcept
{
	temp->clear();
	auto iter = begin();
	auto iter_end = end();
	if (iter == iter_end) return nullptr;

	{
		Buffer text = *iter;
		Buffer readed = text.readwith(needle);
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

	for (; iter != iter_end; iter++)
	{
		Buffer text = *iter;
		Buffer readed = text.readwith(needle);
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
Buffer BufferQueue::readwith(HashTester<void>& needle, TBuffer* temp) noexcept
{
	size_t chr_size = needle.size();

	temp->clear();
	auto iter = begin();
	auto iter_end = end();

	if (iter == iter_end) return false;

	{
		Buffer text = *iter;
		cptr finded = needle.puts(text);
		if (finded != nullptr)
		{
			m_readed = (byte*)finded - (byte*)text.data();
			text.cut_self((byte*)finded - chr_size);
			return text;
		}
		*temp << text;
		iter++;
	}
	for (; iter != iter_end; iter++)
	{
		Buffer text = *iter;

		cptr finded = needle.puts(text);
		if (finded != nullptr)
		{
			intptr_t endsize = (byte*)finded - (byte*)text.data() - needle.size();
			if (endsize >= 0)
			{
				if (endsize != 0)
				{
					*temp << text.cut(endsize);
				}
			}
			else
			{
				temp->resize(temp->size() + endsize);
			}
			skip(temp->size() + chr_size);
			return *temp;
		}
		*temp << text;
	}
	return nullptr;
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
void BufferQueue::_skipZero() noexcept
{
	if (m_first->size == m_readed)
	{
		m_first = m_first->next;
		m_readed = 0;
		if (m_first == nullptr)
		{
			m_last = _axis();
		}
	}
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

size_t BufferQueue::bufferCount() noexcept
{
	return m_count;
}
void BufferQueue::checkBufferCorrution() noexcept
{
	BufferBlock * block = m_first;
	if (block == nullptr)
	{
		_assert(m_last == _axis());
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

BufferQueue::Iterator::Iterator(BufferQueue * queue) noexcept
	:m_buffer(queue->m_first), m_offset(queue->m_readed)
{
}
kr::Buffer BufferQueue::Iterator::value() const noexcept
{
	return kr::Buffer(m_buffer->buffer + m_offset, m_buffer->size - m_offset);
}
void BufferQueue::Iterator::next() noexcept
{
	m_buffer = m_buffer->next;
	m_offset = 0;
}
bool BufferQueue::Iterator::isEnd() const noexcept
{
	return m_buffer == nullptr;
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
