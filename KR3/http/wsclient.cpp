#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

#include "wsclient.h"
#include "wscommon.h"

#include <KR3/data/crypt.h>

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
WebSocketClient::WebSocketClient(Text16 url, View<Text> protocols) throws(SocketException)
{
	connect(url, protocols);
}
void WebSocketClient::connect(Text16 url, View<Text> protocols) throws(SocketException)
{
	ParsedUrl<char16> parsed(url);
	Client::connect(TSZ16() << parsed.host, parsed.port);
	_sendRequest(url, protocols);
}
void WebSocketClient::writeBinary(Buffer data) noexcept
{
	WSFrameEx frame;
	memset(&frame, 0, sizeof(frame));
	frame.opcode = WSOpcode::BINARY;
	frame.fin = true;
	frame.setDataLengthAuto(data.size());
	write({ &frame, frame.getSize() });
	write(data);
}
void WebSocketClient::writeText(Text data) noexcept
{
	WSFrameEx frame;
	memset(&frame, 0, sizeof(frame));
	frame.opcode = WSOpcode::TEXT;
	frame.fin = true;
	frame.setDataLengthAuto(data.size());
	write({ &frame, frame.getSize() });
	write(data.cast<void>());
}

void WebSocketClient::onError(Text name, int code) noexcept
{
	dout << "[WebSocket Error] " << name << " (Code: " << code << ')' << endl;
}
void WebSocketClient::onConnect() noexcept
{
}
void WebSocketClient::onConnectFail(int code) noexcept
{
}
void WebSocketClient::onHandshaked() noexcept
{
}
void WebSocketClient::onClose() noexcept
{
}
void WebSocketClient::onRead() throws(...)
{
	for (;;)
	{
		m_state.call([this](auto& obj) { 
			return onReadWith(obj); 
		});
	}
}
void WebSocketClient::onText(Text data) noexcept
{
}
void WebSocketClient::onBinary(Buffer data) noexcept
{
}

void WebSocketClient::_sendPong(Buffer buffer) noexcept
{
	WSFrameEx frame;
	memset(&frame, 0, sizeof(frame));
	frame.opcode = WSOpcode::PONG;
	frame.fin = true;
	frame.setDataLengthAuto(buffer.size());
	write({ &frame, frame.getSize() });
	write(buffer);
	flush();
}
void WebSocketClient::_sendRequest(Text16 url, View<Text> protocols) noexcept
{
	Connecting& conn = m_state.reset<Connecting>();
	TText url8 = toUtf8(url);
	ParsedUrl<char> parsed(url8);
	
	constexpr size_t SecWebSocketKeySize = 16;
	byte randombuf[SecWebSocketKeySize];
	g_random.fill(randombuf);
	conn.key = (encoder::Base64)Buffer(randombuf);

	TSZ tsz;
	tsz << "GET " << toUtf8(url) << " HTTP/1.1\r\n"
		"Connection: Upgrade\r\n"
		"Origin: about:kenlib \r\n"
		"Host: " << toUtf8(parsed.host) << "\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Key: " << conn.key << "\r\n"
		"User-Agent: KenLib/1.0" << "\r\n";


	if (protocols != nullptr)
	{
		tsz << "Sec-WebSocket-Protocol: " << join(protocols, ", ") << "\r\n";
	}

	tsz << "Sec-WebSocket-Version: 13\r\n"
		"\r\n";

	tsz.clear();
	tsz <<
		"GET ws://echo.websocket.org/ HTTP/1.1\r\n"
		"Host: echo.websocket.org\r\n"
		"Connection: Upgrade\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.87 Safari/537.36\r\n"
		"Upgrade: websocket\r\n"
		"Origin: file://\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Accept-Language: ko,en-US;q=0.9,en;q=0.8,ja;q=0.7,ko-KR;q=0.6\r\n"
		"Sec-WebSocket-Key: " << conn.key << "\r\n\r\n";
	write(tsz.cast<void>());
	flush();
}
void WebSocketClient::onReadWith(Connecting& obj) throws(...)
{
	TText tbuf;
	auto* receive = m_receive.retype<char>();
	HashTester<char> needle("\r\n");
	Text firstline = receive->readwith(needle, &tbuf);

	Text version = firstline.readwith(' ');
	Text statenum = firstline.readwith(' ');
	Text statetxt = firstline;
	AText key = move(obj.key);

	Headers& nobj = m_state.reset<Headers>();
	nobj.upgraded = false;
	nobj.upgradeToWebSocket = false;
	nobj.keyChecked = false;
	nobj.key = move(key);
}
void WebSocketClient::onReadWith(Headers& obj) throws(...)
{
	TText tbuf;
	TBufferQueue<char>* receive = m_receive.retype<char>();
	HashTester<char> needle("\r\n", HashTester<char>::NoReset);

	for (;;)
	{
		needle.reset();
		Text buf = receive->readwith(needle, &tbuf);
		if (buf == nullptr) throw EofException();
		if (buf.empty())
		{
			if (!obj.upgraded)
			{
				onError("No upgrade header", ERROR_INVALID_SERVER_STATE);
				throw ThrowAbort();
			}
			if (!obj.keyChecked)
			{
				onError("No Sec-WebSocket-Accept header", ERROR_INVALID_SERVER_STATE);
				throw ThrowAbort();
			}
			m_state.reset<HandShaked>();
			onHandshaked();
			break;
		}

		Text prop = buf.readwith(':');
		buf.readIf(' ');
		if (prop == "Connection")
		{
			if (buf != "Upgrade")
			{
				onError("Connection must be upgrade", ERROR_INVALID_SERVER_STATE);
				throw ThrowAbort();
			}
			obj.upgraded = true;
		}
		else if (prop == "Upgrade")
		{
			if (!buf.equals_i("websocket"))
			{
				onError("Upgrade must be websocket", ERROR_INVALID_SERVER_STATE);
				throw ThrowAbort();
			}
			obj.upgradeToWebSocket = true;
		}
		else if (prop == "Sec-WebSocket-Accept")
		{
			if (!buf.equals(makeSecWebSocketAccept(obj.key)))
			{
				onError("Sec-WebSocket-Accept not matched", ERROR_INVALID_SERVER_STATE);
				// throw ThrowAbort();
			}
			obj.keyChecked = true;
		}
		else
		{
			// dout << prop << ": " << buf << endl;
		}
	}
}
void WebSocketClient::onReadWith(HandShaked& obj) throws(...)
{
	Buffer data;
	for (;;)
	{
		try
		{
			data = obj.wsf.readFrom(&m_receive);
		}
		catch (TooBigException&)
		{
			onError("Too big data", ERROR_NOT_ENOUGH_MEMORY);
			throw ThrowAbort();
		}
		switch (obj.wsf.frame.opcode)
		{
		case WSOpcode::TEXT:
			onText(data.cast<char>());
			break;
		case WSOpcode::BINARY:
			onBinary(data);
			break;
		case WSOpcode::PING:
			_sendPong(data);
			break;
		default:
			dout << "OPCODE: " << (int)obj.wsf.frame.opcode << endl;
			break;
		}
	}
}

#endif