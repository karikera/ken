#pragma once

#include <KRMessage/net/client.h>

namespace kr
{
	class WebSocketClient: public Client
	{
	public:
		WebSocketClient() noexcept;
		WebSocketClient(Text16 url) throws(SocketException);

		void connect(Text16 url) throws(SocketException);

	protected:
		void onError(Text name, int code) noexcept override final;
		void onConnect() noexcept override final;
		void onConnectFail(int code) noexcept override final;
		void onRead() throws(...) override final;
		void onClose() noexcept override final;

	private:
		AText m_url;
		bool m_handShaked;
	};
}
