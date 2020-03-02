#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

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

#pragma warning(push)
#pragma warning(disable:26495)
WebSocketSession::WebSocketSession(HttpClient* client) noexcept
	:Super(client)
{
}
#pragma warning(pop)
void WebSocketSession::onRead() throws(...)
{
	Buffer data;
	for (;;)
	{
		try
		{
			data = m_wsf.readFrom(&m_receive);
		}
		catch (TooBigException&)
		{
			onError("Too big data", ERROR_NOT_ENOUGH_MEMORY);
			lock().closeClient();
			throw;
		}
		_assert(m_wsf.opcode != WSOpcode::CONTINUE);
		switch (m_wsf.opcode)
		{
		case WSOpcode::BINARY:
			onBinary(data);
			break;
		case WSOpcode::TEXT:
			onText(data.cast<char>());
			break;
		case WSOpcode::PING:
			sendPong(data);
			break;
		default:
			dout << "OPCODE: " << (int)m_wsf.opcode << endl;
			break;
		}
		
	}
}

void WebSocketSession::onError(Text name, int code) noexcept
{
	dout << "[WebSocket Error] " << name << " (Code: " << code << ')' << endl;
}
void WebSocketSession::onText(Text data) throws(...)
{
}
void WebSocketSession::onBinary(Buffer data) throws(...)
{
}

WebSocketPage::WebSocketPage() noexcept
{
}

void WebSocketPage::process(HttpClient * client)
{
	_handShake(client);
	onAccept(client);
}

void WebSocketPage::_handShake(HttpClient * client)
{
	// client->m_stream.base()->setTimeout(1, 500000);

	HttpRequestHeader header;
	header.set(client->requestHeaders());

	if (header.upgrade != "websocket")
	{
		// 이 페이지는 웹소켓 전용이에요!
		throw HttpStatus::BadRequest;
	}

	if (!connectionIs(header.connection, "Upgrade"))
	{
		// 이 페이지는 웹소켓 전용이에요!
		throw HttpStatus::BadRequest;
	}

	if (header.wsKey == nullptr)
	{
		// No key
		throw HttpStatus::BadRequest;
	}

	/*
	Sec-WebSocket-Location: ws://rua.kr:14128/\r\n\
	Sec-WebSocket-Origin: http://www.rua.kr\r\n\
	Sec-WebSocket-Protocol: /protocol/\r\n\
	*/
	HttpClient::Lock _lock = client->lock();
	_lock.writes({ "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
		"Upgrade: WebSocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: ", makeSecWebSocketAccept(header.wsKey), "\r\n\r\n"
	});
	_lock.flush();
}

#endif