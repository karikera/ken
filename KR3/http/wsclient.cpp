#include "stdafx.h"
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
void WebSocketClient::send(Buffer data) noexcept
{
	WSFrameEx frame;
	memset(&frame, 0, sizeof(frame));
	frame.setDataLengthAuto(data.size());
	frame.opcode = WSOpcode::BINARY;
	write({ &frame, frame.getSize() });
	m_receive.write(data);
}
void WebSocketClient::sendText(Text data) noexcept
{
	WSFrameEx frame;
	memset(&frame, 0, sizeof(frame));
	frame.setDataLengthAuto(data.size());
	frame.opcode = WSOpcode::TEXT;
	write({ &frame, frame.getSize() });
	m_receive.write(data.cast<void>());
}

void WebSocketClient::onError(Text name, int code) noexcept
{
	dout << "[WebSocket Error] " << name << " (Code: " << code << ')' << endl;
}
void WebSocketClient::onConnect() noexcept
{
	Connecting& conn = m_state.get<Connecting>();
	_assert(conn.key != nullptr);
	AText key = move(conn.key);
	HeadLine& headline = m_state.reset<HeadLine>();
	headline.key = move(key);
}
void WebSocketClient::onConnectFail(int code) noexcept
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
		"Upgrade: WebSocket\r\n"
		"Sec-WebSocket-Key: " << conn.key << "\r\n";


	if (protocols != nullptr)
	{
		tsz << "Sec-WebSocket-Protocol: " << join(protocols, ", ") << "\r\n";
	}

	tsz << "Sec-WebSocket-Version: 13\r\n"
		"\r\n";

	write(tsz.cast<void>());
	flush();
}
void WebSocketClient::onReadWith(Connecting& obj) throws(...)
{
	unreachable();
}
void WebSocketClient::onReadWith(HeadLine& obj) throws(...)
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
				close();
				throw EofException();
			}
			if (!obj.keyChecked)
			{
				onError("No Sec-WebSocket-Accept header", ERROR_INVALID_SERVER_STATE);
				close();
				throw EofException();
			}
			m_state.reset<HandShaked>();
			break;
		}

		Text prop = buf.readwith(':');
		buf.readIf(' ');
		if (prop == "Connection")
		{
			if (buf != "Upgrade")
			{
				onError("Connection must be upgrade", ERROR_INVALID_SERVER_STATE);
				close();
				throw EofException();
			}
			obj.upgraded = true;
		}
		else if (prop == "Upgrade")
		{
			if (!buf.equals_i("WebSocket"))
			{
				onError("Upgrade must be WebSocket", ERROR_INVALID_SERVER_STATE);
				close();
				throw EofException();
			}
			obj.upgradeToWebSocket = true;
		}
		else if (prop == "Sec-WebSocket-Accept")
		{
			if (!buf.equals(makeSecWebSocketAccept(obj.key)))
			{
				onError("Sec-WebSocket-Accept not matched", ERROR_INVALID_SERVER_STATE);
				close();
				throw EofException();
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
	TBuffer tbuf;
	Buffer data;
	for (;;)
	{
		try
		{
			data = obj.wsf.readFrom(&m_receive, &tbuf);
			dout << "OPCODE: " << (int)obj.wsf.frame.opcode << endl;
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
void WebSocketClient::onClose() noexcept
{
}
