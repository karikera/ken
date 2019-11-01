#pragma once

#include <KR3/main.h>
#include <KR3/mt/thread.h>
#include <KR3/mt/criticalsection.h>
#include <KR3/net/socket.h>

#include "wsevent.h"
#include "client.h"
#include "server.h"

namespace kr
{
	struct Operation;
	class MTClient;

	class MTClient:public AtomicReferencable<MTClient>
	{
	public:
		MTClient() noexcept;
		MTClient(Socket * socket) noexcept;
		virtual ~MTClient() noexcept;

		void reset(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		void connect(Ipv4Address v4addr, int port) throws(SocketException);
		void connect(pcstr16 host, int port) throws(SocketException);
		void requestReceive() noexcept;
		void flush() noexcept;
		void write(Buffer data) noexcept;
		void writes(View<Buffer> datas) noexcept;
		void writeLock() noexcept;
		void writeWithoutLock(Buffer data) noexcept;
		void writeUnlock() noexcept;
		void close() noexcept;
		bool isClosed() noexcept;

		void readSetBuffer(BufferQueue receiver) noexcept;
		void readCommit(size_t size) noexcept;
		void writeCommit(size_t size) noexcept;
		void callOnRead() noexcept;

		virtual void onConnect() noexcept;
		virtual void onRead() = 0;
		virtual void onError(Text funcname, int code) noexcept = 0;
		virtual void onSendDone() noexcept;
		virtual void onClose() noexcept;

		void switchClient(MTClient * client) noexcept;
		template <typename T, typename ... ARGS>
		T* switchClient(ARGS && ... args) noexcept;

	protected:
		BufferQueue m_receive;

	private:
		void _close() noexcept;
		void _callOnRead() noexcept;

		Socket * m_socket;
		CriticalSection m_cs;
		CriticalSection m_csRead; // for m_receive buffer
		MTClient * m_switchClient;
		BufferQueue m_writeQueue;
		Operation * m_flushOperation;
		Operation * m_receiveOperation;
		enum class RState :byte
		{
			Closed,
			Receiving,
			Preparing,
		};
		RState m_receiving;
		bool m_flushing;
		bool m_starting;
		atomic<bool> m_closing;
		
	};

	class MTServer
	{
	public:
		MTServer() throws(FunctionError);
		~MTServer() noexcept;
		void open(int port) throws(SocketException);

		virtual MTClient* onAccept(Socket * socket) noexcept = 0;
		virtual void onError(Text message, int errcode) noexcept = 0;

		static int CT_STDCALL iocpWorker(void*) noexcept;

	private:
		Socket * getClientSocket() noexcept;
		void requestAccept() noexcept;
		void acceptCommit(size_t size) noexcept;

	protected:
		BufferQueue m_receive;

	private:
		AText m_htmlPath;
		Socket * m_serverSocket;
		Socket * m_clientSocket;
		Operation * m_acceptOperation;
		bool m_closed;
	};
}

template <typename T, typename ... ARGS>
T* kr::MTClient::switchClient(ARGS && ... args) noexcept
{
	T * newclient = _new T(move(args) ... , m_socket);
	_switchClient(newclient);
	return newclient;
}
