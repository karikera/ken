#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else
#include "server.h"
#include <KR3/net/socket.h>

#include <KRMessage/msgloop.h>
#include <winsock2.h>
#include <ws2tcpip.h>

kr::Server::Server() noexcept
{
	m_socket = nullptr;
}
kr::Server::Server(Socket * socket) noexcept
{
	m_socket = socket;
}
kr::Server::~Server() noexcept
{
	delete m_socket;
}
void kr::Server::setSocket(Socket * socket) noexcept
{
	m_socket = socket;
}
kr::Socket * kr::Server::getSocket() noexcept
{
	return m_socket;
}
void kr::Server::open(word port) throws(SocketException)
{
	m_socket->open(port);
}

#endif