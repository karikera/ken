#pragma once

#include <KR3/main.h>

namespace kr
{
	namespace ws
	{
		enum WSOpcode
		{
			Text
		};
		struct WS_FRAME_EX
		{
			bool fin;
			WSOpcode opcode;
			
			inline void SetDataLengthAuto(size_t sz) noexcept
			{
			}
			inline size_t size() noexcept
			{
				return 0;
			}
		};

		const int WBYTE_SIZE = 0x80;
		const int WWORD_SIZE = 0x4000;
		const int WTRIB_SIZE = 0x200000;
		const int WDWORD_SIZE = 0x10000000;

		const int WBYTE_MASK = 0x7f;
		const int WWORD_MASK = 0x3fff;
		const int WTRIB_MASK = 0x1fffff;
		const int WDWORD_MASK = 0xfffffff;

		const int WBYTE_SHIFT = 7;
		const int WWORD_SHIFT = 14;
		const int WTRIB_SHIFT = 21;
		const int WDWORD_SHIFT = 28;

		class WSIStream;
		class WSOStream;
		
		class WSStreamBase
		{
		public:
			char * getCursor() noexcept;
			void setCursor(char * str) noexcept;

		protected:
			char * m_data;
		};

		class WSIStream : public WSStreamBase
		{
			friend class WSOStream;
		public:
			WSIStream(const char * data, size_t len) noexcept;
			bool isEmpty() noexcept;
			bool read(void * dest, size_t len) noexcept;
			unsigned readInt(size_t len) noexcept;

			inline unsigned readByte() noexcept { return readInt(1); }
			inline unsigned readWord() noexcept { return readInt(2); }
			inline unsigned readTrib() noexcept { return readInt(3); }
			inline unsigned readDword() noexcept { return readInt(4); }
			inline unsigned readRealInt() noexcept { return readInt(5); }
			float readFloatLP() noexcept;
			float readFloat() noexcept;
			double readDouble() noexcept;
			unsigned readExtendInt() noexcept;

			size_t readStringNoNull(char * dest, size_t limit) noexcept;
			size_t readString(char * dest, size_t limit) noexcept;

		protected:
			char * m_end;
		};

		class WSOStream : public WSStreamBase
		{
		public:
			friend class WSIStream;
			WSOStream() noexcept;

			void reset() noexcept;
			bool write(const void * buff, size_t len) noexcept;
			bool writeInt(unsigned n, size_t len) noexcept;
			bool writeString(const char * str) noexcept;
			size_t writeStream(WSIStream& stream) noexcept;
			inline bool writeByte(unsigned v) noexcept { return writeInt(v, 1); }
			inline bool writeWord(unsigned v) noexcept { return writeInt(v, 2); }
			inline bool writeTrib(unsigned v) noexcept { return writeInt(v, 3); }
			inline bool writeDword(unsigned v) noexcept { return writeInt(v, 4); }
			inline bool writeRealInt(int v) noexcept { return writeInt(v, 5); }
			bool writeFloatLP(float v) noexcept;
			bool writeFloat(float v) noexcept;
			bool writeDouble(double v) noexcept;
			bool writeExtendInt(dword v) noexcept;

			size_t size() noexcept;
			Buffer data() noexcept;
			char * begin() noexcept;
			char * end() noexcept;

		protected:
			char m_buff[sizeof(WS_FRAME_EX) + 1024];
		};
	}
}

