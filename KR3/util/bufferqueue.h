#pragma once

#include <KR3/main.h>
#include <KR3/util/hashtester.h>

namespace kr
{
	class BufferQueue;
	class BufferQueuePointer;
	struct BufferBlock;

	template <typename C>
	class TBufferQueue;

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


	class BufferQueue:public MakeIterable<BufferQueue, AddBufferable<BufferQueue, BufferInfo<void, method::CopyTo, false, true,
		OutStream<BufferQueue, void>
	> >>
	{
		friend BufferQueue;
		friend BufferQueuePointer;
	public:
		class Iterator:public MakeIterableIterator<Iterator, Buffer>
		{
		public:
			Iterator(BufferQueue* queue) noexcept;
			Buffer value() const noexcept;
			void next() noexcept;
			bool isEnd() const noexcept;

		private:
			size_t m_offset;
			BufferBlock* m_buffer;
		};
		struct ReadResult
		{
			BufferQueue* const queue;
			TmpArray<Buffer> buffers;
			size_t totalSize;
			bool ended;

			ReadResult(BufferQueue * queue) noexcept;
			ReadResult(ReadResult&& _move) noexcept;
			void push(const Buffer& buffer) noexcept;
			void remove(size_t size) noexcept;
			void readAllTo(io::VOStream<void> out) throws(ThrowRetry);
			void readAllTo(ABuffer * buffer, size_t limit) throws(ThrowRetry, NotEnoughSpaceException);
		};
		
		BufferQueue() noexcept;
		~BufferQueue() noexcept;
		BufferQueue(BufferQueue&& _move) noexcept;
		BufferQueue& operator = (BufferQueue&& _move) noexcept;
		WView<void> prepare() noexcept;
		void commit(size_t size) noexcept;
		void write(const void * data, size_t size) noexcept;
		void write(Buffer data) noexcept;
		void peek(void * dest, size_t size) throws(EofException);
		void read(void * dest, size_t size) throws(EofException);
		ReadResult read(size_t size) throws(EofException);
		ReadResult readto(byte needle) noexcept;
		ReadResult readto(HashTester<void>& needle) noexcept;
		Buffer read(size_t size, TBuffer* temp) throws(EofException);
		Buffer readwith(byte needle, TBuffer* temp) throws(EofException);
		Buffer readwith(HashTester<void>& needle, TBuffer* temp) throws(EofException);

		bool empty() noexcept;
		void clear() noexcept;
		Buffer getFirstBlock() noexcept;
		void skip(size_t size) noexcept;
		size_t size() noexcept;
		
		static void clearKeeped() noexcept;

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

		template <typename T>
		TBufferQueue<T>* retype() noexcept
		{
			return static_cast<TBufferQueue<T>*>(this);
		}
	private:
		void _skipZero() noexcept;
		BufferBlock * _makeBuffer() noexcept;
		void _removeFrontNode() noexcept;
		BufferBlock * _axis() noexcept;

		BufferBlock *m_first;
		BufferBlock *m_last;
		size_t m_count;
		size_t m_readed;
		size_t m_totalSize;
	};
	template <typename C>
	class TBufferQueue:public BufferQueue
	{
		static_assert(sizeof(C) == sizeof(byte), "Type size must be 1 byte");
	public:
		WView<C> prepare() noexcept
		{
			return (WView<C>&)BufferQueue::prepare();
		}
		void write(const C* data, size_t size) noexcept
		{
			BufferQueue::write(data, size);
		}
		void write(View<C> data) noexcept
		{
			BufferQueue::write(data.cast<void>());
		}
		void peek(C* dest, size_t size) throws(EofException)
		{
			BufferQueue::peek(dest, size);
		}
		void read(C* dest, size_t size) throws(EofException)
		{
			BufferQueue::read(dest, size);
		}
		ReadResult read(size_t size) throws(EofException)
		{
			return BufferQueue::read(size);
		}
		ReadResult readwith(C needle) throws(EofException)
		{
			return BufferQueue::readwith((byte)needle);
		}
		ReadResult readwith(View<C> needle) throws(EofException)
		{
			return BufferQueue::readwith(needle.cast<void>());
		}
		View<C> read(size_t size, TmpArray<C>* temp) throws(EofException)
		{
			return BufferQueue::read(size, (TmpArray<void>*)temp).cast<C>();
		}
		View<C> readwith(C needle, TmpArray<C>* temp) throws(EofException)
		{
			return BufferQueue::readwith((byte)needle, (TmpArray<void>*)temp).cast<C>();
		}
		View<C> readwith(HashTester<C>& needle, TmpArray<C>* temp) throws(EofException)
		{
			return BufferQueue::readwith((HashTester<void>&)needle, (TmpArray<void>*)temp).cast<C>();
		}
		void readwith(byte needle, io::VOStream<void> out, size_t limit) throws(EofException)
		{
			return BufferQueue::readwith(needle, out, limit);
		}
		void readwith(View<C> needle, io::VOStream<C> out, size_t limit) throws(EofException)
		{
			return BufferQueue::readwith(needle.cast<void>(), (io::VOStream<void>&)out, limit);
		}
		View<C> getFirstBlock() noexcept
		{
			return BufferQueue::getFirstBlock().cast<C>();
		}
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
}