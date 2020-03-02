#pragma once

#include <KR3/main.h>
#include <KR3/net/wsevent.h>
#include <KR3/io/protocol.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/data/crypt.h>
#include <KR3/net/socket.h>
#include <KR3/http/httpstatus.h>

#include "header.h"


namespace kr
{

	class HttpHeaderBuilder
	{
	public:
		HttpHeaderBuilder() noexcept;
		HttpHeaderBuilder(HttpHeaderBuilder && data) noexcept;

		void clear() noexcept;
		void appendETag(Text etag) noexcept;
		void appendIfModifiedSince(UnixTimeStamp time) noexcept;
		template <class T>
		void append(Text name, const T &value)
		{
			m_strHeaders << name << ": " << value << "\r\n";
		}
		Text data() const noexcept;

	private:
		AText m_strHeaders;
	};

	class HttpDataBuilder
	{
	public:
		HttpDataBuilder() noexcept;
		HttpDataBuilder(HttpDataBuilder && data) noexcept;

		void clear() noexcept;
		void append(Text line) noexcept;
		void appendUriEncoded(Text line) noexcept;
		io::UriOStream<AText> * getDataOutputStream(Text name) noexcept;
		Text data() const noexcept;

		template <class T> void append(Text name, const T &value)
		{
			value.writeTo(&getDataOutputStream(name));
		}
	private:
		AText m_strData;
	};
	class HttpConnection :public InStream<HttpConnection, char>
	{
	public:

		HttpConnection() noexcept;
		~HttpConnection() noexcept;

		HttpStatus open(Text url, const HttpConnectionRequest * request, HeaderStore * response); // SocketException
		void close() noexcept;
		Text get() noexcept;
		bool next() throws(SocketException);
		void download(pcstr16 filename) throws(Error);

		size_t $read(char * dest, size_t size);

	private:
		io::SelfBufferedIStream<io::SocketStream<char>> m_socket;
	
	};

	//class HTTPProtocol:public Protocol
	//{
	//public:
	//	HTTPProtocol();
	//	~HTTPProtocol();
	//	virtual size_t readFirst(ptr data, dword nSize) override;
	//	virtual size_t readNext(ptr data, dword nSize) override;
	//
	//private:
	//	HTTPConnection m_webfile;
	//};

	namespace contentType
	{
		const Text URL_ENCODED = "application/x-www-form-urlencoded";
		const Text JSON = "application/json; charset=utf-8";
	}
}
