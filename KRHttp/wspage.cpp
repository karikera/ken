#include <KR3/main.h>

#include "wspage.h"

#include <KR3/data/crypt.h>

using namespace kr;

static bool connectionIs(Text list, Text type) noexcept
{
	for (Text conn : list.splitIterable(','))
	{
		if (conn.trim() == type) return true;
	}
	return false;
}

WebSocketClient::WebSocketClient(Socket * socket) noexcept
	:MTClient(socket)
{
	m_frameReady = false;
}
void WebSocketClient::onRead()
{
	for (;;)
	{
		if (!m_frameReady)
		{
			if (m_receive.size() < sizeof(WSFrame)) break;
			m_receive.peek((char*)&m_wsf, sizeof(WSFrame));
			size_t needex = m_wsf.getExtendSize();
			if (m_receive.size() < sizeof(WSFrame) + needex) break;
			m_receive.skip(sizeof(WSFrame));
			m_receive.read((char*)&m_wsf + sizeof(WSFrame), needex);
			m_frameReady = true;
		}

		switch (m_wsf.opcode)
		{
		case WSOpcode::CONTINUE: break;
		case WSOpcode::CLOSE:
			close();
			return;
		case WSOpcode::PING: break;
		case WSOpcode::PONG: break;
		case WSOpcode::BINARY:
			break;
		case WSOpcode::TEXT:
			break;
		}

		constexpr size_t MAX = 4096;

		uint64_t datalen64 = m_wsf.getDataLength();
		if (datalen64 >= MAX)
		{
			close();
			throw TooBigException();
		}
		size_t datalen = (size_t)datalen64;

		if (m_receive.size() < datalen) break;
		m_frameReady = false;

		TBuffer temp;
		Buffer data = m_receive.read(datalen, &temp);
		if (m_wsf.mask)
		{
			mem::xor_copy((byte*)data.data(), data.data(), datalen, m_wsf.mask);
		}
		WSStream wsstream(&data);
		onData(wsstream);
	}
}

WebSocketPage::WebSocketPage() noexcept
{
}

void WebSocketPage::process(HttpClient * client, Text query, BufferQueue * stream)
{
	_handShake(client, stream);
	Socket * socket = client->getSocket();
	WebSocketClient * newclient = onAccept(socket);
	client->switchClient(newclient);
}

void WebSocketPage::_handShake(HttpClient * client, BufferQueue * stream)
{
	// client->m_stream.base()->setTimeout(1, 500000);

	HttpRequestHeader header;
	header.set(client->getHeader());

	if (header.upgrade != "websocket")
	{
		// 이 페이지는 웹소켓 전용이에요!
		throw HttpBadRequest;
	}

	if (!connectionIs(header.connection, "Upgrade"))
	{
		// 이 페이지는 웹소켓 전용이에요!
		throw HttpBadRequest;
	}

	if (header.wsKey == nullptr)
	{
		// No key
		throw HttpBadRequest;
	}

	BText<128> keys;
	keys << header.wsKey;
	keys << "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	BText<32> acceptKey;
	acceptKey << (encoder::Base64)(BText<20>)(encoder::Sha1)keys;

	/*
	Sec-WebSocket-Location: ws://rua.kr:14128/\r\n\
	Sec-WebSocket-Origin: http://www.rua.kr\r\n\
	Sec-WebSocket-Protocol: /protocol/\r\n\
	*/
	client->write((TSZ() << "\
HTTP/1.1 101 Web Socket Protocol Handshake\r\n\
Upgrade: WebSocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: " << acceptKey << "\r\n\
\r\n").cast<void>());
	client->flush();
}
