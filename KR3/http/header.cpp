#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else
#include "header.h"

#include <KR3/io/selfbufferedstream.h>
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
void kr::HttpResponseHeader::set(const HeaderView* data) noexcept
{
	//"Server","Chahe-Control","Expires","Keep-Alive","Content-Type","Connection",
	data->ifGet("Content-Length", [this](Text value) { contentLength = value.to_uint(); });
	data->ifGet("Location", [this](Text value) { location = (kr::AText)value; });
	data->ifGet("ETag", [this](Text value) { eTag = (kr::AText)value; });
	data->ifGet("Last-Modified", [this](Text value) { lastModified = value; });
	data->ifGet("Date", [this](Text value) { date = value; });
}

kr::HttpRequestHeader::HttpRequestHeader() noexcept
{
	wsVersion = 0;
}
void kr::HttpRequestHeader::set(const HeaderView* data) noexcept
{
	data->ifGet("Upgrade", [this](Text value) { upgrade = (AText)value; });
	data->ifGet("Connection", [this](Text value) { connection = (AText)value; });
	data->ifGet("Host", [this](Text value) { host = (AText)value; });
	data->ifGet("Origin", [this](Text value) { origin = (AText)value; });
	data->ifGet("Sec-WebSocket-Key", [this](Text value) { wsKey = (AText)value; });
	data->ifGet("Sec-WebSocket-Version", [this](Text value) { wsVersion = value.to_uint(); });
	data->ifGet("Sec-WebSocket-Protocol", [this](Text value) { wsProtocol = (AText)value; });
}

kr::HeaderView::HeaderView() noexcept
{
}
kr::HeaderView::~HeaderView() noexcept
{
}
void kr::HeaderView::setAll(Text header) noexcept
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
void kr::HeaderView::clear() noexcept
{
	m_map.clear();
}
kr::Text kr::HeaderView::operator [](Text name) const noexcept
{
	return get(name);
}
kr::Text kr::HeaderView::get(Text name) const noexcept
{
	return m_map.get(name, nullptr);
}

kr::HeaderStore::HeaderStore() noexcept
{
}
kr::HeaderStore::HeaderStore(HttpStatus status, Text statusName) noexcept
{
	setStatus(status, statusName);
}
kr::HeaderStore::~HeaderStore() noexcept
{
}
kr::Text kr::HeaderStore::getStatusLine() noexcept
{
	return m_status == nullptr ? "HTTP/1.0 200 OK\r\n"_tx : (Text)m_status;
}
void kr::HeaderStore::setStatus(HttpStatus status, Text statusName) noexcept
{
	m_status = AText::concat("HTTP/1.0 "_tx, (uint)status, ' ', statusName, "\r\n"_tx);
}
void kr::HeaderStore::clear() noexcept
{
	m_headers.clear();
	m_headerValues.clear();
	m_status = nullptr;
}
kr::Text kr::HeaderStore::operator [](Text name) const noexcept
{
	return get(name);
}
kr::Text kr::HeaderStore::get(Text name) const noexcept
{
	auto iter = m_headers.find(name);
	if (iter == m_headers.end()) return nullptr;
	return m_headerValues.get(iter->second);
}
void kr::HeaderStore::setAll(Text header) noexcept
{
	for (;;)
	{
		Text line = header.readwith_e("\r\n");
		if (line == nullptr || line.empty()) break;
		Text name = line.readwith(":");
		if (name == nullptr) continue;
		line.readIf(' ');
		set(name, line);
	}
}

void kr::HeaderStore::set(Text name, Text value) noexcept
{
	size_t idx = m_headerValues.put(value);
	m_headers[name] = idx;
}
void kr::HeaderStore::setIfNotSetted(Text name, Text value) noexcept
{
	auto res = m_headers.insert(name, 0);
	if (res.second)
	{
		size_t idx = m_headerValues.put(value);
		res.first->second = idx;
	}
}
void kr::HeaderStore::inherit(const HeaderStore* headers) noexcept
{
	for (auto& iter : headers->m_headers)
	{
		auto res = m_headers.insert(iter.first, 0);
		if (res.second)
		{
			size_t idx = m_headerValues.put(headers->m_headerValues.get(iter.second));
			res.first->second = idx;
		}
	}
}
kr::HeaderStore::Iterator kr::HeaderStore::begin() noexcept
{
	return { this, m_headers.begin() };
}
kr::HeaderStore::Iterator kr::HeaderStore::end() noexcept
{
	return { this, m_headers.end() };
}


kr::HeaderStore::Iterator::Iterator() noexcept
	:m_store(nullptr)
{
}
kr::HeaderStore::Iterator::Iterator(HeaderStore* store, Map<Text, size_t>::iterator iter) noexcept
	:m_store(store), m_iter(iter)
{
}
kr::HeaderStore::Iterator& kr::HeaderStore::Iterator::operator ++() noexcept
{
	++m_iter;
	return *this;
}
const kr::HeaderStore::Iterator kr::HeaderStore::Iterator::operator ++(int) noexcept
{
	return { m_store, m_iter++ };
}
bool kr::HeaderStore::Iterator::operator ==(const Iterator& other) const noexcept
{
	return m_iter == other.m_iter;
}
bool kr::HeaderStore::Iterator::operator !=(const Iterator& other) const noexcept
{
	return m_iter != other.m_iter;
}
std::pair<kr::Text, kr::Text> kr::HeaderStore::Iterator::operator *() const noexcept
{
	return { key(), value() };
}
kr::Text kr::HeaderStore::Iterator::key() const noexcept
{
	return m_iter->first;
}
kr::Text kr::HeaderStore::Iterator::value() const noexcept
{
	return m_store->m_headerValues.get(m_iter->second);
}

#endif