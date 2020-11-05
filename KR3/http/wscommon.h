#pragma once

#include <KR3/main.h>
#include <KR3/data/crypt.h>
#include <KR3/util/bufferqueue.h>
#include <KR3/util/serializer.h>

namespace kr
{
	TText makeSecWebSocketAccept(Text key) noexcept;

	struct endian
	{
		static uint16_t reverse(uint16_t v) noexcept;
		static uint32_t reverse(uint32_t v) noexcept;
		static uint64_t reverse(uint64_t v) noexcept;
	};

	enum class WSOpcode :uint8_t
	{
		CONTINUE,     // 0: continuation frame
		TEXT,         // 1: text frame
		BINARY,       // 2: binary frame
					  // 3~7: reserved.
					  CLOSE = 8,      // 8: connection close
					  PING,         // 9: ping
					  PONG,         // A: pong
								  // B~F: reserved.
	};

	struct WSFrame
	{
		WSOpcode opcode : 4;
		bool rsv3 : 1; // Reserved.
		bool rsv2 : 1; // Reserved.
		bool rsv1 : 1; // Reserved.
		bool fin : 1; // Final fragment

		uint8_t length : 7;
		bool mask : 1; // Mask data.
					   // 0-125: Length
					   // 126: After 2bytes Length
					   // 127: After 8bytes Length

		size_t getLengthExtend() noexcept;
		size_t getMaskExtend() noexcept;
		size_t getExtendSize() noexcept;
		size_t getSize() noexcept;
		uint64_t getDataLength() noexcept;
		uint32_t getMask() noexcept;
		void setMask(uint32_t value) noexcept;
		void* getData() noexcept;
		void setDataLengthAuto(uint64_t len) noexcept;
		void setDataLength7(uint8_t len) noexcept;
		void setDataLength16(uint16_t len);
		void setDataLength64(uint64_t len);
	};

	struct WSFrameEx :WSFrame
	{
		uint64_t LengthEx;
		uint32_t MaskEx;
	};

	class WSFrameReader
	{
	public:
		static constexpr size_t DATA_SIZE_LIMIT = 4096;
		WSFrameReader() noexcept;
		Buffer readFrom(BufferQueue* receive) throws(EofException, ThrowAbort, TooBigException);
		WSFrameEx frame;
		WSOpcode opcode;
		ABuffer data;

	private:
		enum class State:byte
		{
			ClearData,
			Frame,
			FrameEx,
			Data,
		};
		State m_state;
	};

	using WSStream = Deserializer<Buffer>;

	template <class Parent> class WSSender:public Parent
	{
	public:
		using typename Parent::Lock;
		using Parent::Parent;
		using Parent::lock;

		void writeBinary(Buffer data) noexcept
		{
			WSFrameEx frame;
			memset(&frame, 0, sizeof(frame));
			frame.opcode = WSOpcode::BINARY;
			frame.fin = true;
			frame.setDataLengthAuto(data.size());

			if (Lock _lock = lock())
			{
				_lock.write({ &frame, frame.getSize() });
				_lock.write(data);
			}
		}
		void writeText(Text data) noexcept
		{
			WSFrameEx frame;
			memset(&frame, 0, sizeof(frame));
			frame.opcode = WSOpcode::TEXT;
			frame.fin = true;
			frame.setDataLengthAuto(data.size());

			if (Lock _lock = lock())
			{
				_lock.write({ &frame, frame.getSize() });
				_lock.write(data);
			}
		}
		void sendPong(Buffer buffer) noexcept
		{
			WSFrameEx frame;
			memset(&frame, 0, sizeof(frame));
			frame.opcode = WSOpcode::PONG;
			frame.fin = true;
			frame.setDataLengthAuto(buffer.size());

			if (Lock _lock = lock())
			{
				_lock.write({ &frame, frame.getSize() });
				_lock.write(buffer);
				_lock.flush();
			}
		}
		void flush() noexcept
		{
			lock().flush();
		}
	};
}

