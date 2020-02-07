#pragma once

#include <KR3/main.h>
#include "httpd.h"
#include "wscommon.h"

namespace kr
{
	class WebSocketPage;

	class WebSocketSession : public MTClient
	{
	public:
		WebSocketSession(Socket * socket) noexcept;
		void onRead() throws(...) override final;
		virtual void onData(Buffer data) = 0;

	private:
		WSFrameReader m_wsf;
	};
	class WebSocketPage :public Page
	{
		friend class CWebSocketPath;
	public:
		WebSocketPage() noexcept;
		void process(HttpClient * client) override;
		virtual WebSocketSession* onAccept(Socket * args) = 0;
		
	private:
		void _handShake(HttpClient * client);
	};
}
