#include <KR3/main.h>

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else
#include "header.h"

#include <KR3/io/selfbufferedstream.h>
#include <KR3/util/unaligned.h>
#include <KRUtil/net/socket.h>

uint16_t kr::endian::reverse(uint16_t v) noexcept
{
	return kr::intrinsic<2>::rotl(v, 8);
}
uint32_t kr::endian::reverse(uint32_t v) noexcept
{
	return (v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24);
}
uint64_t kr::endian::reverse(uint64_t v) noexcept
{
	return (v << 56) |
		((v & 0xff00) << 40) |
		((v & 0xff0000) << 24) |
		((v & 0xff000000) << 8) |
		((v >> 8) & 0xff000000) |
		((v >> 24) & 0xff0000) |
		((v >> 40) & 0xff00) |
		(v >> 56);
}

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

size_t kr::WSFrame::getLengthExtend() noexcept
{
	if (length < 126) return 0;
	if (length == 126) return 2;
	if (length == 127) return 8;
	_assert(!"Invalid length");
	return 0;
}
size_t kr::WSFrame::getMaskExtend() noexcept
{
	return (mask ? 4 : 0);
}
size_t kr::WSFrame::getExtendSize() noexcept
{
	return getLengthExtend() + getMaskExtend();
}
size_t kr::WSFrame::getSize() noexcept
{
	return sizeof(WSFrame) + getExtendSize();
}
uint64_t kr::WSFrame::getDataLength() noexcept
{
	if (length < 126) return length;
	if (length == 126) return endian::reverse(*(uint16_t*)(this + 1));
	if (length == 127) return endian::reverse(*(uint64_t*)(this + 1));
	_assert(!"Invalid length");
	return 0;
}
uint32_t kr::WSFrame::getMask() noexcept
{
	return *(Unaligned<uint32_t>*)((char*)(this + 1) + getLengthExtend());
}
void kr::WSFrame::setMask(uint32_t value) noexcept
{
	mask = true;
	*(Unaligned<uint32_t>*)((char*)(this + 1) + getLengthExtend()) = value;
}
void * kr::WSFrame::getData() noexcept
{
	return ((char*)(this + 1) + getExtendSize());
}
void kr::WSFrame::setDataLengthAuto(uint64_t len) noexcept
{
	if (len < 126)
	{
		setDataLength7((uint8_t)len);
	}
	else if (len < 65536)
	{
		setDataLength16((uint16_t)len);
	}
	else
	{
		setDataLength64(len);
	}
}
void kr::WSFrame::setDataLength7(uint8_t len) noexcept
{
	length = len;
}
void kr::WSFrame::setDataLength16(uint16_t len)
{
	length = 126;
	*(uint16_t*)(this + 1) = endian::reverse((uint16_t)len);
}
void kr::WSFrame::setDataLength64(uint64_t len)
{
	length = 127;
	*(uint64_t*)(this + 1) = endian::reverse(len);
}

#endif