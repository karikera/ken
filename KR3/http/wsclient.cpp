#include "stdafx.h"
#include "wsclient.h"

using namespace kr;


template <typename CHR>
struct ParsedUrl
{
	View<CHR> host;
	int port;

	ParsedUrl(View<CHR> url) throws(SocketException)
	{
		static CHR PROTOCOL_SEP[] = {':','/','/','\0'};
		static CHR PROTOCOL_WS[] = { 'w', 's', '\0' };
		static CHR HOST_END[] = { ':', '/', '\0' };

		View<CHR> read = url;
		View<CHR> protocol = read.readwith(PROTOCOL_SEP);
		if (protocol == nullptr) throw SocketException(ERROR_INVALID_DATA);
		if (protocol != PROTOCOL_WS) throw SocketException(ERROR_UNSUPPORTED_TYPE);

		host = read.readto_ye(HOST_END);
		if (!read.empty() && *read == ':')
		{
			port = read.readwith_e('/').to_uint();
		}
		else
		{
			port = 80;
		}
	}

};


#pragma warning(push)
#pragma warning(disable:26495)
WebSocketClient::WebSocketClient() noexcept
{
}
#pragma warning(pop)
WebSocketClient::WebSocketClient(Text16 url) throws(SocketException)
{
	connect(url);
}
void WebSocketClient::connect(Text16 url) throws(SocketException)
{
	m_handShaked = false;

	m_url = toUtf8(url);

	ParsedUrl<char16> parsed(url);
	Client::connect(TSZ16() << parsed.host, parsed.port);
}

void WebSocketClient::onError(Text name, int code) noexcept
{
}
void WebSocketClient::onConnect() noexcept
{
	_assert(m_url != nullptr);

	ParsedUrl<char> parsed(m_url);

	TSZ tsz;
	tsz << "GET " << toUtf8(m_url) << " HTTP/1.1\r\n"
		"Connection: Upgrade\r\n"
		"Origin: about:kenlib \r\n"
		"Host: " << toUtf8(parsed.host) << "\r\n"
		"Upgrade: websocket\r\n\r\n";
	m_url = nullptr;

	write(tsz.cast<void>());
	flush();
}
void WebSocketClient::onConnectFail(int code) noexcept
{
}
void WebSocketClient::onRead() throws(...)
{
	auto * receive = m_receive.retype<char>();
	if (!m_handShaked)
	{
		HashTester<char> needle("\r\n", HashTester<char>::NoReset);
		TText tbuf;
		for (;;)
		{
			needle.reset();
			// "ws://echo.websocket.org"
			Text buf = receive->readwith(needle, &tbuf);
			if (buf == nullptr) return;
			if (buf.empty())
			{
				m_handShaked = true;
				break;
			}
			dout << buf << endl;
			dout.flush();
		}
	}
}
void WebSocketClient::onClose() noexcept
{
}