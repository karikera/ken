#pragma once

#include <KR3/net/client.h>
#include <KR3/data/mixed.h>
#include "wscommon.h"

namespace kr
{
	class WebSocketClient: private Client
	{
	public:
		WebSocketClient() noexcept;
		WebSocketClient(Text16 url, View<Text> protocols = nullptr) throws(SocketException);

		void connect(Text16 url, View<Text> protocols = nullptr) throws(SocketException);
		void send(Buffer data) noexcept;
		void sendText(Text data) noexcept;
		using Client::makeProcedure;

	protected:
		void onError(Text name, int code) noexcept override;
		void onConnect() noexcept override final;
		void onConnectFail(int code) noexcept override final;
		void onRead() throws(...) override final;
		void onClose() noexcept override final;
		virtual void onData(Buffer data) noexcept = 0;

	private:
		struct Connecting
		{
			AText key;
		};
		struct HeadLine
		{
			AText key;
		};
		struct Headers
		{
			AText key;
			bool upgraded : 1;
			bool upgradeToWebSocket : 1;
			bool keyChecked : 1;
		};
		struct HandShaked
		{
			WSFrameReader wsf;
		};

		void _sendRequest(Text16 url, View<Text> protocols) noexcept;
		void onReadWith(Connecting& obj) throws(...);
		void onReadWith(HeadLine& obj) throws(...);
		void onReadWith(Headers& obj) throws(...);
		void onReadWith(HandShaked& obj) throws(...);

		Mixed<Connecting, HeadLine, Headers, HandShaked> m_state;
	};
}
