#pragma once

#include <KR3/main.h>
#include <KRUtil/uts.h>
#include <KRUtil/net/socket.h>
#include <KR3/data/map.h>

namespace kr
{
	class HttpHeader;
	class AHttpHeader;

	struct endian
	{
		static uint16_t reverse(uint16_t v) noexcept;
		static uint32_t reverse(uint32_t v) noexcept;
		static uint64_t reverse(uint64_t v) noexcept;
	};

	struct CHAHECONTROL
	{
		dword maxage;
	};
	struct KEEPALIVE
	{
		dword timeout;
	};


	struct HttpResponseHeader
	{
		UnixTimeStamp date;
		AText server;
		CHAHECONTROL cacheControl;
		dword expires;
		UnixTimeStamp lastModified;
		AText eTag;
		dword contentLength;
		KEEPALIVE keepAlive;
		AText connection;
		AText contentType;
		AText location;

		HttpResponseHeader() noexcept;
		void set(const HttpHeader & data) noexcept;
	};

	struct HttpRequestHeader
	{
		AText upgrade;
		AText connection;
		AText host;
		AText origin;
		AText wsKey;
		AText wsProtocol;
		uint wsVersion;

		HttpRequestHeader() noexcept;
		void set(const HttpHeader &data) noexcept;
	};

	class HttpHeader
	{
	public:
		HttpHeader() noexcept;
		void set(Text header) noexcept;
		void clear() noexcept;
		Text operator [](Text name) const noexcept;
		template <typename LAMBDA>
		bool ifGet(Text name, const LAMBDA& call) const noexcept
		{
			return m_map.ifGet(name, call);
		}

	private:
		ReferenceMap<Text, Text> m_map;
	};

	class AHttpHeader : public HttpHeader
	{
	public:
		AHttpHeader() noexcept;
		~AHttpHeader() noexcept;
		void set(AText header) noexcept;

	private:
		AText m_buffer;
	};

	struct HttpConnectionRequest
	{
		Text contentType;
		Text contentData;
		Text extraHeaders;

		HttpConnectionRequest() noexcept;
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
		void * getData() noexcept;
		void setDataLengthAuto(uint64_t len) noexcept;
		void setDataLength7(uint8_t len) noexcept;
		void setDataLength16(uint16_t len);
		void setDataLength64(uint64_t len);
	};

	struct WSFrameEx:WSFrame
	{
		uint64_t LengthEx;
		uint32_t MaskEx;
	};

}
