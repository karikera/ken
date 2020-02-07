#include "stdafx.h"
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
WebSocketSession::WebSocketSession(Socket * socket) noexcept
	:MTClient(socket)
{
}
#pragma warning(pop)
void WebSocketSession::onRead() throws(...)
{
	TBuffer tbuf;
	Buffer data;
	for (;;)
	{
		try
		{
			data = m_wsf.readFrom(&m_receive, &tbuf);
		}
		catch (TooBigException&)
		{
			onError("Too big data", ERROR_NOT_ENOUGH_MEMORY);
			close();
			throw;
		}
		onData(data);
	}
}

WebSocketPage::WebSocketPage() noexcept
{
}

void WebSocketPage::process(HttpClient * client)
{
	_handShake(client);
	Socket * socket = client->getSocket();
	WebSocketSession * newclient = onAccept(socket);
	client->switchClient(newclient);
}

void WebSocketPage::_handShake(HttpClient * client)
{
	// client->m_stream.base()->setTimeout(1, 500000);

	HttpRequestHeader header;
	header.set(client->getHeader());

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
	client->writes({
		"HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
		"Upgrade: WebSocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: ", makeSecWebSocketAccept(header.wsKey), "\r\n\r\n"
	});
	client->flush();
}
