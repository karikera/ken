#include "stdafx.h"

#ifndef NO_USE_SOCKET

#ifdef __GNUC__

#include <arpa/inet.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)
using SOCKET = uintptr_t;

#elif defined(_MSC_VER)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <KR3/util/wide.h>

#endif

#include "socket.h"

using namespace kr;

namespace
{
	template <typename LAMBDA>
	Ipv4Address urlLoop(pcstr url, const LAMBDA &lambda) throws(SocketException)
	{
		ADDRINFOA *info;
		int res = getaddrinfo(url, nullptr, nullptr, &info);
		if (res != 0) throw SocketException(res);
		
		for (ADDRINFOA * res = info; res != nullptr; res = res->ai_next)
		{
			sockaddr_in * addr = (sockaddr_in *)res->ai_addr;
			Ipv4Address ip = (Ipv4Address&)addr->sin_addr.s_addr;
			if(raw(ip) == 0) ip = Ipv4Address(127,0,0,1);
			try
			{
				lambda(ip);
				return ip;
			}
			catch(SocketException&)
			{
			}
		}
		throw SocketException();
	}
	template <typename LAMBDA>
	Ipv4Address urlLoop(pcstr16 url, const LAMBDA &lambda) throws(SocketException)
	{
		ADDRINFOW *info;
		int res = GetAddrInfoW(wide(url), nullptr, nullptr, &info);
		if (res != 0) throw SocketException(res);

		for (ADDRINFOW * res = info; res != nullptr; res = res->ai_next)
		{
			sockaddr_in * addr = (sockaddr_in *)res->ai_addr;
			Ipv4Address ip = (Ipv4Address&)addr->sin_addr.s_addr;
			if (raw(ip) == 0) ip = Ipv4Address(127, 0, 0, 1);
			try
			{
				lambda(ip);
				return ip;
			}
			catch (SocketException&)
			{
			}
		}
		throw SocketException();
	}
}
SocketException::SocketException() noexcept
	:m_error(WSAGetLastError())
{
}
SocketException::SocketException(int err) noexcept
	:m_error(err)
{
}
SocketException::operator int() const noexcept
{
	return m_error;
}
Socket::Init::Init() noexcept
{
	WSADATA wsadata;
	int res = WSAStartup(0x202, &wsadata);
	if(res != S_OK) error("소켓 초기화 실패: %p", res);
}
Socket::Init::~Init() noexcept
{
	WSACleanup();
}
Socket* Socket::create() noexcept
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	_assert(socket != nullptr);
	return (Socket*)sock;
}
void Socket::operator delete(ptr socket) noexcept
{
	//if (socket == nullptr) return;
#ifdef __GNUC__
	close((int)socket);
#elif defined(_MSC_VER)
	closesocket((SOCKET)socket);
#endif
}
void Socket::shutdown() noexcept
{
	::shutdown((SOCKET)this, SD_BOTH);
}
void Socket::connect(Ipv4Address add,word port) throws(SocketException)
{
	sockaddr_in addr;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr = (in_addr&)add;
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	if(::connect((SOCKET)this,(sockaddr*)&addr,sizeof(addr)) == SOCKET_ERROR)
	{
		throw SocketException();
	}
}
void Socket::connect(pcstr url, word port) throws(SocketException)
{
	urlLoop(url, [&](Ipv4Address ip){ connect(ip, port); });
}
void Socket::connect(pcstr16 url, word port) throws(SocketException)
{
	urlLoop(url, [&](Ipv4Address ip) { connect(ip, port); });
}
void Socket::connectAsync(Ipv4Address add, word port) throws(SocketException)
{
	sockaddr_in addr;
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr = (in_addr&)add;
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int connectResult = ::connect((SOCKET)this, (sockaddr*)&addr, sizeof(addr));
	_assert(connectResult == SOCKET_ERROR);
	int err = WSAGetLastError();
	if (err != WSAEWOULDBLOCK) throw SocketException(err);
}
void Socket::setOption(int name, const void * data, int size) noexcept
{
	if (setsockopt((SOCKET)this, SOL_SOCKET, name, (char*)data, size) == SOCKET_ERROR)
	{
		error("Set timeout failed. %p", WSAGetLastError());
	}
}
void Socket::setLinger(bool enabled, short sec) noexcept
{
	LINGER  lingerStruct;
	lingerStruct.l_onoff = enabled;
	lingerStruct.l_linger = sec;
	setOption(SO_LINGER, lingerStruct);
}
void Socket::setTimeout(long sec,long usec) noexcept
{
	timeval tv = {sec,usec};
	setOption(SO_RCVTIMEO, tv);
}
void Socket::open(word port, Ipv4Address v4addr) throws(SocketException)
{
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	(Ipv4Address&)addr.sin_addr.s_addr = v4addr;
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	if (bind((SOCKET)this, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		throw SocketException();
	}
	if (listen((SOCKET)this, SOMAXCONN) == SOCKET_ERROR)
	{
		throw SocketException();
	}
}
Ipv4Address Socket::getIpAddress() noexcept
{
	sockaddr_in addr;
	int size = sizeof(addr);
	int getpeernameResult = getpeername((SOCKET)this, (sockaddr*)&addr, &size);
	_assert(getpeernameResult != SOCKET_ERROR);
	return (Ipv4Address&)addr.sin_addr.s_addr;
}
Socket* Socket::accept() throws(SocketException)
{
	socklen_t size = sizeof(sockaddr_in);
	sockaddr_in temp = {0};
	SOCKET tsock = ::accept((SOCKET)this,(sockaddr*)&temp,&size);
	if (tsock == INVALID_SOCKET) throw SocketException();
	//*ip = addr.sin_addr.s_addr;
	return (Socket*)tsock;
}
void Socket::$write(cptr binary, size_t len) throws(SocketException)
{
	int sended = send((SOCKET)this, (pcstr)binary, (int)mint(len, (size_t)0x7fffffff), 0);
	if (sended == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			throw ThrowAbort();
		throw SocketException(err);
	}
	_assert(sended == len);
}
size_t Socket::$read(ptr binary, size_t len) throws(SocketException, EofException)
{
	int nReceiveLen = recv((SOCKET)this, (char *)binary, intact<int>(len), 0);
	if (nReceiveLen == 0) throw EofException();
	if (nReceiveLen == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK) return 0;
		throw SocketException(err);
	}
	return nReceiveLen;
}

template <>
Ipv4Address Socket::findIp<char>(const char * url) throws(SocketException)
{
	return urlLoop(url, [&](Ipv4Address ip){});
}
template <>
Ipv4Address Socket::findIp<char16>(const char16 * url) throws(SocketException)
{
	return urlLoop(url, [&](Ipv4Address ip) {});
}
Ipv4Address Socket::getCurrentIpAddress() throws(SocketException)
{
	char name[MAX_PATH];
	if(gethostname(name,MAX_PATH)) return nullptr;
	return urlLoop(name, [](Ipv4Address ip) {});
}

SocketWriteFailException::SocketWriteFailException(ABuffer data) noexcept
	:unsended(move(data))
{
}

#endif