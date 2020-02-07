#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else
#include "header.h"

#include <KR3/io/selfbufferedstream.h>
#include <KR3/util/unaligned.h>
#include <KR3/net/socket.h>

kr::HttpResponseHeader::HttpResponseHeader() noexcept
{
	date = (time_t)0;
	lastModified = (time_t)0;

	cacheControl.maxage = 0;
	expires = 0;
	contentLength = 0;
	keepAlive.timeout = 0;
}
void kr::HttpResponseHeader::set(const HttpHeader& data) noexcept
{
	//"Server","Chahe-Control","Expires","Keep-Alive","Content-Type","Connection",
	data.ifGet("Content-Length", [this](Text value) { contentLength = value.to_uint(); });
	data.ifGet("Location", [this](Text value) { location = (kr::AText)value; });
	data.ifGet("ETag", [this](Text value) { eTag = (kr::AText)value; });
	data.ifGet("Last-Modified", [this](Text value) { lastModified = value; });
	data.ifGet("Date", [this](Text value) { date = value; });
}

kr::HttpRequestHeader::HttpRequestHeader() noexcept
{
	wsVersion = 0;
}

void kr::HttpRequestHeader::set(const HttpHeader& data) noexcept
{
	data.ifGet("Upgrade", [this](Text value) { upgrade = (AText)value; });
	data.ifGet("Connection", [this](Text value) { connection = (AText)value; });
	data.ifGet("Host", [this](Text value) { host = (AText)value; });
	data.ifGet("Origin", [this](Text value) { origin = (AText)value; });
	data.ifGet("Sec-WebSocket-Key", [this](Text value) { wsKey = (AText)value; });
	data.ifGet("Sec-WebSocket-Version", [this](Text value) { wsVersion = value.to_uint(); });
	data.ifGet("Sec-WebSocket-Protocol", [this](Text value) { wsProtocol = (AText)value; });
}

kr::HttpHeader::HttpHeader() noexcept
{
}
void kr::HttpHeader::set(Text header) noexcept
{
	for (;;)
	{
		Text line = header.readwith_e("\r\n");
		if (line == nullptr || line.empty()) break;
		Text name = line.readwith(":");
		if (name == nullptr) continue;
		if (line.startsWith(' ')) line++;
		m_map[name] = line;
	}
}
void kr::HttpHeader::clear() noexcept
{
	m_map.clear();
}
kr::Text kr::HttpHeader::operator [](Text name) const noexcept
{
	return m_map.get(name, nullptr);
}

kr::AHttpHeader::AHttpHeader() noexcept
{
}
kr::AHttpHeader::~AHttpHeader() noexcept
{
	HttpHeader::clear();
}
void kr::AHttpHeader::set(AText header) noexcept
{
	m_buffer = move(header);
	HttpHeader::set(m_buffer);
}

#endif