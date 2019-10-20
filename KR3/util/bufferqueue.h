#pragma once

#include <KR3/main.h>

namespace kr
{
	class BufferQueue;
	class BufferQueuePointer;
	class BufferQueueWithRef;
	struct BufferBlock;

	constexpr size_t BUFFER_BLOCK_SIZE = 4096;

	struct BufferBlock
	{
		BufferBlock * next;
		size_t size;
		byte buffer[BUFFER_BLOCK_SIZE];

		void * begin() noexcept;
		size_t capacity() const noexcept;

		Buffer getBuffer() noexcept;
		WView<byte> getRemaining() noexcept;
		static BufferBlock * alloc() noexcept;
		void free() noexcept;

		BufferBlock() = delete;
		~BufferBlock() = delete;
	};

	struct BufferBlockWithRef
	{
		static constexpr size_t SIZE_MASK = (size_t)-1 >> 1;
		static constexpr size_t REFERENCE_BIT = ~SIZE_MASK;
		BufferBlockWithRef * next;
		size_t size;
		union
		{
			const byte * bufferPtr;
			struct
			{
				byte buffer[BUFFER_BLOCK_SIZE];
			};
		};

		bool isReference() const noexcept;

		const byte * begin() const noexcept;
		size_t capacity() const noexcept;

		Buffer getBuffer() noexcept;
		static BufferBlockWithRef * alloc() noexcept;
		void free() noexcept;

		BufferBlockWithRef() = delete;
		~BufferBlockWithRef() = delete;
	};

	class BufferQueue:public AddBufferable<BufferQueue, BufferInfo<void, false, false, true, 
		OutStream<BufferQueue, void>
	> >
	{
		friend BufferQueue;
		friend BufferQueuePointer;
		friend BufferQueueWithRef;
	public:	
		class Iterator;

		BufferQueue() noexcept;
		~BufferQueue() noexcept;
		BufferQueue(BufferQueue&& _move) noexcept;
		BufferQueue& operator = (BufferQueue&& _move) noexcept;
		WView<void> prepare() noexcept;
		void commit(size_t size) noexcept;
		void write(const void * data, size_t size) noexcept;
		void write(Buffer data) noexcept;
		void peek(void * dest, size_t size) noexcept;
		void read(void * dest, size_t size) noexcept;
		Buffer read(size_t size, TBuffer * temp) noexcept;
		Text readwith(char chr, TText * temp) noexcept;
		Text readwith(Text chr, TText * temp) noexcept;
		AText readAll() noexcept;
		bool empty() noexcept;
		void clear() noexcept;
		Buffer getFirstBlock() noexcept;
		void skip(size_t size) noexcept;
		size_t size() noexcept;
		
		static void clearKeeped() noexcept;

		Iterator begin() noexcept;
		Iterator end() noexcept;
		size_t bufferCount() noexcept;
		void checkBufferCorrution() noexcept;

		template <typename T>
		T readas() noexcept
		{
			T data;
			read(&data, sizeof(T));
			return data;
		}

		template <typename T>
		T peekas() noexcept
		{
			T data;
			peek(&data, sizeof(T));
			return data;
		}

	private:
		BufferBlock * _makeBuffer() noexcept;
		void _removeFrontNode() noexcept;
		BufferBlock * _axis() noexcept;

		BufferBlock *m_first;
		BufferBlock *m_last;
		size_t m_count;
		size_t m_readed;
		size_t m_totalSize;
	};
	class BufferQueuePointer:public InStream<BufferQueuePointer, void>
	{
	public:
		BufferQueuePointer(const BufferQueue & buf) noexcept;
		byte read() throws(EofException);
		void read(void * dest, size_t size) throws(EofException);
		template <typename T> T read() throws(EofException)
		{
			T v;
			read(&v, sizeof(v));
			return v;
		}
		void skip(size_t size) throws(EofException);
		size_t getReadSize() noexcept;
		void clearSize() noexcept;

	private:
		BufferBlock *m_buffer;
		size_t m_readed;
		size_t m_fullreaded;
	};
	class BufferQueueWithRef:public BufferQueue
	{
	public:
		class Iterator;

		Buffer getFirstBlock() noexcept;
		void write(const void * data, size_t size) noexcept;
		void writeRef(const void * data, size_t size) noexcept;
		void read(void * dest, size_t size) noexcept;

		Iterator begin() noexcept;
		Iterator end() noexcept;
	};

	class BufferQueue::Iterator
	{
	public:
		Iterator(BufferBlock * buffer, size_t offset) noexcept;
		Buffer operator *() const noexcept;
		Iterator & operator ++() noexcept;
		const Iterator operator ++(int) noexcept;
		bool operator ==(const Iterator & other) const noexcept;
		bool operator !=(const Iterator & other) const noexcept;

	private:
		size_t m_offset;
		BufferBlock * m_buffer;
	};
	class BufferQueueWithRef::Iterator
	{
	public:
		Iterator(BufferBlockWithRef * buffer, size_t offset) noexcept;
		Buffer operator *() noexcept;
		Iterator & operator ++() noexcept;
		const Iterator operator ++(int) noexcept;
		bool operator ==(const Iterator & other) noexcept;
		bool operator !=(const Iterator & other) noexcept;

	private:
		size_t m_offset;
		BufferBlockWithRef * m_buffer;
	};
}