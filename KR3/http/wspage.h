#pragma once

#include <KR3/main.h>
#include "httpd.h"
#include "wscommon.h"

namespace kr
{
	class WebSocketPage;

	class WebSocketSession : private WSSender<MTClient>
	{
		friend WebSocketPage;
		using Super = WSSender<MTClient>;
	public:
		WebSocketSession(HttpClient * client) noexcept;
		using Super::writeBinary;
		using Super::writeText;
		using Super::flush;

	protected:
		void onError(Text name, int code) noexcept override;
		void onRead() throws(...) override final;

		virtual void onText(Text data) throws(...);
		virtual void onBinary(Buffer data) throws(...);

	private:
		WSFrameReader m_wsf;
	};
	class WebSocketPage :public Page
	{
		friend class CWebSocketPath;
	public:
		WebSocketPage() noexcept;
		void process(HttpClient * client) override;
		virtual WebSocketSession* onAccept(HttpClient* args) = 0;
		
	private:
		void _handShake(HttpClient * client);
	};
}
