#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

#include "http.h"

#include <KR3/win/windows.h>
#include <KR3/fs/file.h>
#include <KR3/net/socket.h>

#pragma comment(lib,"shell32.lib")

kr::HttpHeaderBuilder::HttpHeaderBuilder() noexcept
	:m_strHeaders((size_t)0, 1024)
{
}
kr::HttpHeaderBuilder::HttpHeaderBuilder(HttpHeaderBuilder && data) noexcept
{
	m_strHeaders = move(data.m_strHeaders);
}
void kr::HttpHeaderBuilder::clear() noexcept
{
	m_strHeaders.clear();
}
void kr::HttpHeaderBuilder::appendETag(Text etag) noexcept
{
	append("If-None-Match",etag);
}
void kr::HttpHeaderBuilder::appendIfModifiedSince(UnixTimeStamp uts) noexcept
{
	append("If-Modified-Since",uts.getInfo());
}
kr::Text kr::HttpHeaderBuilder::data() const noexcept
{
	return m_strHeaders;
}

kr::HttpDataBuilder::HttpDataBuilder() noexcept
	:m_strData((size_t)0, 1024)
{

}
kr::HttpDataBuilder::HttpDataBuilder(HttpDataBuilder && data) noexcept
{
	m_strData = move(data.m_strData);
}
void kr::HttpDataBuilder::clear() noexcept
{
	m_strData.clear();
}
void kr::HttpDataBuilder::append(Text line) noexcept
{
	m_strData.write(line);
}
void kr::HttpDataBuilder::appendUriEncoded(Text line) noexcept
{
	*encoder::Uri::cast(&m_strData) << line;
}
kr::io::UriOStream<kr::AText> * kr::HttpDataBuilder::getDataOutputStream(Text name) noexcept
{
	if (!m_strData.empty())
	{
		m_strData.write('&');
	}
	m_strData.write(name);
	m_strData.write('=');
	return encoder::Uri::cast(&m_strData);
}
kr::Text kr::HttpDataBuilder::data() const noexcept
{
	return m_strData;
}

kr::HttpConnectionRequest::HttpConnectionRequest() noexcept
{
	contentType = nullptr;
	contentData = nullptr;
	extraHeaders = nullptr;
}

kr::HttpConnection::HttpConnection() noexcept:
	m_socket(nullptr)
{
}
kr::HttpConnection::~HttpConnection() noexcept
{
	delete m_socket.base();
}
kr::HttpStatus kr::HttpConnection::open(Text url, const HttpConnectionRequest * request, HeaderStore* response) // SocketException
{
	close();

	Text host = url.subarr(url.find("://"));
	_assert(host != nullptr);
	host += 3;

	pcstr end = host.find('/');
	if (end != nullptr) host = host.cut(end);
	
	end = host.find(':');
	word port;
	if(end != nullptr)
	{
		port = (word)host.subarr(end+1).to_int();
		host = host.cut(end);
	}
	else port = 80;

	m_socket.resetIStream(Socket::create());
	m_socket->connect(Socket::findIp(pstr(TSZ() << host)), port);

	TText sendbuffer((size_t)0, 1024);
	if (request->contentData != nullptr)
	{
		sendbuffer << "POST"_tx;
	}
	else
	{
		sendbuffer << "GET"_tx;
	}
	sendbuffer << ' ' << url;
	sendbuffer << " HTTP/1.0\r\n";
	if (request->extraHeaders != nullptr)
	{
		sendbuffer << request->extraHeaders;
	}
	if(request->contentData != nullptr)
	{
		sendbuffer << "Content-Type: " << request->contentType;
		sendbuffer << "\r\n"
			"Content-Length: " << request->contentData.size() << "\r\n\r\n";
		sendbuffer << request->contentData;
	}
	else
	{
		sendbuffer << "\r\n";
	}
	m_socket->write(sendbuffer);

	HttpStatus statusCode = HttpStatus::Undefined;

	try
	{
		{
			Text line = m_socket.readwith("\r\n");
			Text httpVersion = line.readwith(' ');
			if (httpVersion == nullptr) throw InvalidSourceException();
			Text httpCode = line.readwith(' ');
			if (httpCode == nullptr) throw InvalidSourceException();

			statusCode = (HttpStatus)httpCode.to_uint();
		}

		for (;;)
		{
			Text line = m_socket.readwith("\r\n");
			if (line.empty()) break;
			Text name = line.readwith(':');
			if (name == nullptr) continue;
			line.readIf(' ');
			response->set(name, line);
		}
	}
	catch (EofException&)
	{
	}
	return statusCode;
}
void kr::HttpConnection::close() noexcept
{
	delete m_socket.base();
	m_socket.resetIStream(nullptr);
}
kr::Text kr::HttpConnection::get() noexcept
{
	return m_socket.text();
}
bool kr::HttpConnection::next() throws(SocketException)
{
	try
	{
		m_socket.clearBuffer();
		m_socket.request();
		return true;
	}
	catch (EofException&)
	{
		return false;
	}
}
void kr::HttpConnection::download(pcstr16 filename) throws(Error)
{
	Must<File> file = File::create(filename);
	do
	{
		Buffer buffer = get().cast<void>();
		file->$write(buffer.data(), buffer.size());
	}
	while (next());
}
size_t kr::HttpConnection::$read(char * dest, size_t size)
{
	auto text = m_socket.text();
	if (!text.empty())
	{
		return m_socket.read(dest, size);
	}
	return m_socket.base()->read(dest, size);
}

//
//kr::HTTPProtocol::HTTPProtocol()
//{
//	BText<30> strETag;
//	TText16 ETagFile(MAX_PATH);
//
//	strFile += 7;
//	m_szName = strFile.clone_sz();
//	~ETagFile<<m_szName<<_T(".etag")<<nullterm;
//	CreateDirectoryAll(m_szName);
//
//	{
//		TStringA temp;
//		temp.convert(m_szName);
//
//		File * ETag = File::Create(ETagFile.string, GENERIC_READ, OPEN_EXISTING);
//		AutoDelete(ETag);
//		if(ETag)
//		{
//			ETag->ReadString(strETag);
//
//			HttpHeaderBuilder data;
//			data.AppendETag(strETag);
//			webfile.Open(temp, &data, true);
//		}
//		else
//		{
//			webfile.Open(temp, nullptr, true);
//		}
//	}
//}
//kr::HTTPProtocol::~HTTPProtocol()
//{
//	m_webfile.Close();
//}
//size_t kr::HTTPProtocol::readFirst(ptr data, dword nSize)
//{
//	debug;
//	return 0;
//}
//size_t kr::HTTPProtocol::readNext(ptr data, dword nSize)
//{
//	if(webfile.header.nStatusCode == HttpStatus::NotModified)
//	{
//		m_fp = File::Create(m_szName.string, GENERIC_READ, OPEN_EXISTING);
//	}
//	else
//	{
//		if(webfile.header.ETag != nullptr)
//		{
//			File * ETag = File::Create(ETagFile.string, GENERIC_WRITE, CREATE_ALWAYS);
//			AutoDelete(ETag);
//			ETag->WriteStringWait(webfile.header.ETag);
//		}
//
//		m_fp = File::Create(m_szName.string, GENERIC_WRITE | GENERIC_READ, CREATE_ALWAYS);
//		webfile.BeginStreamingTo(m_fp);
//		while(webfile.StreamingTo(m_fp));
//	}
//	return 0;
//}

#endif