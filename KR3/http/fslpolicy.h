#pragma once

#include <KR3/net/socket.h>
#include <KR3/net/mtnet.h>
#include <KR3/main.h>

namespace kr
{
	class FSLPolicyClient: public MTClient
	{
	public:
		using MTClient::MTClient;

		void onRead() noexcept override;
		void onError(Text func, int code) noexcept override;
	};
	class FSLPolicyServer:public MTServer
	{
	public:
		FSLPolicyServer() noexcept;

		MTClient* onAccept(Socket * socket) noexcept override;
		void onError(Text func, int code) noexcept override;
	};
}
