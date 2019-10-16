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


		void clear() noexcept;
		void reset(Socket * socket) noexcept;
		Socket * getSocket() noexcept;
		void connect(Ipv4Address v4addr, int port) throws(SocketException);
		void connect(pcstr16 host, int port) throws(SocketException);
		void requestReceive() noexcept;
		void flush() noexcept;
		void write(Buffer data) noexcept;
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
		CriticalSection m_cs, m_csRead;
		MTClient * m_switchClient;
		BufferQueueWithRef m_writeQueue;
		Operation * m_flushOperation;
		Operation * m_receiveOperation;
		bool m_receiving;
		bool m_flushing;
		atomic<bool> m_closing;
		atomic<bool> m_closed;
		
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
		Socket * m_serverSocket;
		Socket * m_clientSocket;
		Operation * m_acceptOperation;
		bool m_closed;
	};
}

template <typename T, typename ... ARGS>
T* kr::MTClient::switchClient(ARGS && ... args) noexcept
{
	Args spargs;
	spargs.pool = m_pool;
	spargs.socket = m_socket;
	T * newclient = _new T(move(args) ... , &spargs);
	_switchClient(newclient);
	return newclient;
}
