#pragma once

#include <KR3/main.h>
#include <KRUtil/serializer.h>
#include "httpd.h"

namespace kr
{
	using WSStream = Deserializer<Buffer>;
	class WebSocketPage;

	class WebSocketSession : public MTClient
	{
	public:
		WebSocketSession(Socket * socket) noexcept;
		void onRead() throws(...) override final;
		virtual void onData(WSStream & data) = 0;

	private:
		WSFrameEx m_wsf;
		bool m_frameReady;
	};
	class WebSocketPage :public Page
	{
		friend class CWebSocketPath;
	public:
		WebSocketPage() noexcept;
		void process(HttpClient * client, Text query, BufferQueue * stream) override;
		virtual WebSocketSession* onAccept(Socket * args) = 0;
		
	private:
		void _handShake(HttpClient * client, BufferQueue * stream);
	};
}
