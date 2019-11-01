#pragma once

#include <KR3/main.h>
NEED_SOCKET

#include "ipaddr.h"

namespace kr
{
	class SocketException
	{
	public:
		SocketException() noexcept;
		SocketException(int err) noexcept;
		operator int() const noexcept;

	private:
		const int m_error;
	};
	class SocketWriteFailException:public SocketException
	{
	public:
		SocketWriteFailException(ABuffer data) noexcept;

		ABuffer unsended;
	};

	class Socket:public io::Streamable<Socket>
	{
	public:
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
		Socket() noexcept = delete;
		static Socket* create() noexcept;
		static void operator delete(ptr socket) noexcept;
		void shutdown() noexcept;

		void connect(Ipv4Address add,word port) throws(SocketException);
		void connect(pcstr url, word port) throws(SocketException);
		void connect(pcstr16 url, word port) throws(SocketException);
		void connectAsync(Ipv4Address add, word port) throws(SocketException);
		void setOption(int name, const void * data, int size) noexcept;
		template <typename T>
		void setOption(int name, T data) noexcept
		{
			setOption(name, &data, sizeof(data));
		}
		void setLinger(bool enabled, short sec) noexcept;
		void setTimeout(long sec,long usec) noexcept;
		void open(word port, Ipv4Address v4addr = nullptr) throws(SocketException);
		Ipv4Address getIpAddress() noexcept;
		Socket* accept() throws(SocketException);
		void $write(cptr binary, size_t len) throws(SocketException);
		size_t $read(ptr binary, size_t len) throws(SocketException, EofException);

		template <typename CHR>
		static Ipv4Address findIp(const CHR * url) throws(SocketException);
		static Ipv4Address getCurrentIpAddress() throws(SocketException);
	};

	namespace io
	{
		template <typename C>
		using SocketStream = StreamableStream<Socket, C>;
	}

	using SBISocketStream = io::SelfBufferedIStream<io::StreamableStream<Socket, char>>;
}
