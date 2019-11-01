#pragma once

#include "wsevent.h"
#include <KR3/mt/thread.h>

namespace kr
{
	class Server
	{
	public:
		Server() noexcept;
		Server(Socket * socket) noexcept;
		~Server() noexcept;

		void setSocket(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		void open(word port) throws(SocketException);

	private:
		Socket * m_socket;
	};
}