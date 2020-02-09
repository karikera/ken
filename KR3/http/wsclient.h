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
		void writeBinary(Buffer data) noexcept;
		void writeText(Text data) noexcept;
		using Client::makeProcedure;
		using Client::flush;

	protected:
		void onError(Text name, int code) noexcept override;
		void onRead() throws(...) override final;

		virtual void onConnect() noexcept override;
		virtual void onConnectFail(int code) noexcept override;
		virtual void onClose() noexcept override;
		virtual void onHandshaked() noexcept;
		virtual void onText(Text data) noexcept;
		virtual void onBinary(Buffer data) noexcept;

	private:
		struct Connecting
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

		void _sendPong(Buffer buffer) noexcept;
		void _sendRequest(Text16 url, View<Text> protocols) noexcept;
		void onReadWith(Connecting& obj) throws(...);
		void onReadWith(Headers& obj) throws(...);
		void onReadWith(HandShaked& obj) throws(...);

		Mixed<Connecting, Headers, HandShaked> m_state;
	};
}
